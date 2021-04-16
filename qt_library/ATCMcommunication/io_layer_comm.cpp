/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI-FCM communication class
 */
#include <errno.h>
#include <unistd.h>

#include "app_logprint.h"
#include "common.h"
#include "io_layer_comm.h"
#include "cross_table_utility.h"

#include "hmi_plc.h"

#include <QDebug>
#include <QSettings>
#include <QProcess>

#define TIMEOUT_MS 100

// ---------------------------------------------------------------------------

extern HmiPlcBlock plcBlock;
extern HmiPlcBlock hmiBlock;

extern uint16_t writingCount;
extern uint16_t writingList[DimCrossTable]; // NB: zero based

pthread_cond_t theWritingCondvar;
pthread_mutex_t theWritingMutex;

// ---------------------------------------------------------------------------

static pthread_t theThread;
static pthread_cond_t condvar;
static pthread_mutex_t mutex;
static void *automation_thread(void *arg);

extern int update_all(void);
extern void setup(void);
extern void loop(void);

void *automation_thread(void *arg)
{
    pthread_mutex_lock(&mutex);
    {
        pthread_cond_wait(&condvar, &mutex);
        setup();
        do
        {
            pthread_cond_wait(&condvar, &mutex);
            loop();
        }
        while (1);
    }
    pthread_mutex_unlock(&mutex);
    return arg;
}

// ---------------------------------------------------------------------------

io_layer_comm::io_layer_comm(pthread_mutex_t *send_mutex, pthread_mutex_t *recv_mutex) :
    the_send_mutex(send_mutex),
    the_recv_mutex(recv_mutex)
{
    QSettings *options = new QSettings(HMI_INI_FILE, QSettings::IniFormat);
    QString plc_host = "";

    if (options) {
        plc_host = options->value("plc_host", "").toString();
    }

    if (not plc_host.isEmpty() and plc_host != "127.0.0.1") {
        qDebug() << QString("connecting to remote plc engine at '%1'").arg(plc_host);
        hmiClient = newHmiClient(plc_host.toLatin1().data());

    } else {
        if (system("PID=`pidof fcrts` && test $PID != '' && test \"`grep -c zombie /proc/$PID/status`\" -eq 0")) {
            QProcess *myProcess = new QProcess();

            qDebug() << QString("starting local plc engine");
            myProcess->start("fcrts.sh");
        }
        qDebug() << QString("connecting to local plc engine");
        hmiClient = newHmiClient(NULL);
    }
}

io_layer_comm::~io_layer_comm()
{
    finalize();
}

bool io_layer_comm::finalize(void)
{
    deleteHmiClient(hmiClient);
    hmiClient = NULL;
    return true;
}

// ---------------------------------------------------------------------------

void io_layer_comm::run()
{
    if (not hmiClient) {
        LOG_PRINT(error_e, "plc is not connected\n");
        return;
    }

    /* set the absolute time */
    bool recompute_abstime = true;
    struct timespec abstime;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condvar, NULL);
    pthread_create(&theThread, NULL, automation_thread, NULL);

    while (1)
    {
        if (recompute_abstime) {
            recompute_abstime = false;
            clock_gettime(CLOCK_MONOTONIC, &abstime); // pthread_cond_timedwait + pthread_condattr_setclock
            abstime.tv_sec += IOLAYER_PERIOD_ms / 1000;
            abstime.tv_nsec += (IOLAYER_PERIOD_ms % 1000) * 1000 * 1000; // ms -> ns
            if (abstime.tv_nsec >= (1000*1000*1000)) {
                abstime.tv_sec += abstime.tv_nsec / (1000*1000*1000);
                abstime.tv_nsec = abstime.tv_nsec % (1000*1000*1000);
            }
        }

        // wait for either period 100ms or write occurrence
        pthread_mutex_lock(the_send_mutex);
        {
            int rc = pthread_cond_timedwait(&theWritingCondvar, the_send_mutex, &abstime);
            if (rc == 0) {
                // run from signal
            } else if (rc == ETIMEDOUT) {
                // run from timeout
                recompute_abstime = true;
            } else {
                // error
                recompute_abstime = true;
                LOG_PRINT(error_e, "pthread_cond_timedwait %d\n", rc);
            }

            pthread_mutex_lock(the_recv_mutex);
            {
                // set/reset the DO_READ for active and visible "H" variables
                // is already done by activateVar() and deactivateVar()
                // except when there was a write, see below

                // increment the sequence number, it's ok to overflow
                ++hmiBlock.seqnum;

                // send and receive blocks
                if (hmiClientPoll(hmiClient, &hmiBlock, &plcBlock, TIMEOUT_MS) <= 0) {
                    LOG_PRINT(error_e, "communication error with plc\n");
                    // skip resetting, i.e. keep the writingList filled

                } else {
                    // reset the DO_WRITE, checking for "H" variables too
                    clearHmiBlock(&hmiBlock);
                    for (int n = 0; n < writingCount; ++n) {
                        int addr = writingList[n];

                        if (varNameArray[addr].active == TAG_ONDEMAND and varNameArray[addr].visible) {
                            // there was a write on an "H" variable, that is still visible
                            changeStatusHmiBlock(&hmiBlock, addr, varStatus_DO_READ);
                        } else {
                            changeStatusHmiBlock(&hmiBlock, addr, varStatus_NOP);
                        }
                        writingList[n] = 0;
                    }
                    writingCount = 0;
                }

                // update local variables
                update_all(); // readFromDb
            }
            pthread_mutex_unlock(the_recv_mutex);
        }
        pthread_mutex_unlock(the_send_mutex);

        // call the setup() and loop()
        if (recompute_abstime) {
            pthread_cond_signal(&condvar);
        }
    }
}
