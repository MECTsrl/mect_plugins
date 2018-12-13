/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief This class read the errors/event table in order to understand:
 * - wich variable is an error/event,
 * - if it is an error or an event,
 * - wich desriprion is associated,
 * - if you need to log,
 * - which level it have.
 * The other info are not used in the HMI.
 */

#ifndef _ISOC99_SOURCE
#define _ISOC99_SOURCE
#endif
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/statvfs.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ftw.h>
#include <pthread.h>

#include "hmi_logger.h"
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "alarms.h"
#include "common.h"

#include <QCoreApplication>

Logger * logger = NULL;
sem_t theLoggingSem;

pthread_mutex_t alarmevents_list_mutex = PTHREAD_MUTEX_INITIALIZER;
int removeTheOldestLog(const char * dir, FILE *current);

#if 0
/**
 * @brief this table will filled with the color to be used for each error level:
 * rgb sintax is used (example white is "rgb(255,255,255);")
 * tag INVISIBLE is used to hide the item
 */
char LevelColorTable[nb_of_level_e][DESCR_LEN] =
{
    "rgb(0,0,0);", /* level 0 reserved */
    "rgb(0,0,0);", /* level 1 */
    "rgb(0,0,0);", /* level 2 */
    "rgb(0,0,0);", /* level 3 */
    "rgb(0,0,0);", /* level 4 */
    "rgb(0,0,0);", /* level 5 */
    "rgb(0,0,0);", /* level 6 */
    "rgb(0,0,0);", /* level 7 */
    "rgb(0,0,0);", /* level 8 */
    "rgb(0,0,0);", /* level 9 */
    "rgb(0,0,0);", /* level 10 */
    "rgb(0,0,0);", /* level 11 */
    "rgb(0,0,0);", /* level 12 */
    "rgb(0,0,0);", /* level 13 */
    "rgb(0,0,0);", /* level 14 */
    "rgb(0,0,0);", /* level 15 */
    "rgb(0,0,0);", /* level 16 */
    "rgb(0,0,0);", /* level 17 */
    "rgb(0,0,0);", /* level 18 */
    "rgb(0,0,0);", /* level 19 */
    "rgb(0,0,0);", /* level 20 */
    "rgb(0,0,0);", /* level 21 */
    "rgb(0,0,0);", /* level 22 */
    "rgb(0,0,0);", /* level 23 */
    "rgb(0,0,0);", /* level 24 */
    "rgb(0,0,0);", /* level 25 */
    "rgb(0,0,0);", /* level 26 */
    "rgb(0,0,0);", /* level 27 */
    "rgb(0,0,0);", /* level 28 */
    "rgb(0,0,0);", /* level 29 */
    "rgb(0,0,0);", /* level 30 */
    "rgb(0,0,0);", /* level 31 */
    "rgb(0,0,0);", /* level 32 */
};
#endif

/**
 * @brief this table will filled with the color to be used for each error staatus:
 * rgb sintax is used (example white is "rgb(255,255,255);")
 * tag INVISIBLE is used to hide the item
 */
char StatusColorTable[nb_of_alarm_status_e][DESCR_LEN] =
{
    "rgb(255,  0,  0);", /* active alarm latched not acknowledged */
    "rgb(255,  0,  0);", /* active alarm not latched not acknowledged */
    "rgb(  0,  0,255);", /* active alarm latched acknowledged */
    "rgb(  0,  0,255);", /* active alarm not latched acknowledged */
    "rgb(122,122,122);", /* not active alarm latched not acknowledged */
    INVISIBLE,         /* not active alarm not latched not acknowledged */
    INVISIBLE,         /* not active alarm latched acknowledged */
    INVISIBLE,         /* not active alarm not latched acknowledged */
};

char StatusBannerColorTable[nb_of_alarm_status_e][DESCR_LEN] =
{
    "rgb(255,  0,  0);", /* active alarm latched not acknowledged */
    "rgb(255,  0,  0);", /* active alarm not latched not acknowledged */
    "rgb(  0,  0,255);", /* active alarm latched acknowledged */
    INVISIBLE,         /* active alarm not latched acknowledged */
    "rgb(122,122,122);", /* not active alarm latched not acknowledged */
    INVISIBLE,         /* not active alarm not latched not acknowledged */
    INVISIBLE,         /* not active alarm latched acknowledged */
    INVISIBLE,         /* not active alarm not latched acknowledged */
};

#ifdef ENABLE_ALARMS
QHash<QString, event_t *> EventHash;
#endif

unsigned long int totalBytes = 0;

Logger::Logger(const char * alarms_dir, const char * store_dir, int period_msec, QObject *parent) :
    QThread(parent)
{
    alarmsfp = NULL;
    storefp = NULL;
    _period_msec = period_msec;
    _period_msec = ALARMS_PERIOD_MS;

    qRegisterMetaType<trend_msg_t>();
    
    LOG_PRINT(verbose_e,"log period is %d\n", _period_msec);

    if (alarms_dir != NULL)
    {
        strcpy(AlarmsDir, alarms_dir);
    }
    else
    {
        strcpy(AlarmsDir, ALARMS_DIR);
    }
    LOG_PRINT(verbose_e, "Alarm dir: '%s'.\n", AlarmsDir);
    
    if (store_dir != NULL)
    {
        strcpy(StorageDir, store_dir);
    }
    else
    {
        strcpy(StorageDir, STORE_DIR);
    }
    LOG_PRINT(verbose_e, "Store dir: '%s'.\n", StorageDir);
    
    time(&Now);
    memcpy(&CurrentTimeInfo, localtime(&Now),  sizeof(struct tm));

    for ( int i = 0; StoreArrayV[i].tag[0] != '\0'; i++)
    {
        StoreArrayV[i].value[0] = '\0';
    }

#ifdef ENABLE_ALARMS
    if (loadAlarmsTable() <= 0)
    {
        LOG_PRINT(warning_e, "No event/error found.\n");
    }
#endif	
    /* check the available space */
    if (MaxLogUsageMb == 0)
    {
        LOG_PRINT(warning_e, "No space available for logs [%d]. No logs will be recorded.\n", MaxLogUsageMb);
    }
    else
    {
        int capacity_mb = getCapacityDir(LOCAL_DATA_DIR) / 1024;
        if ( MaxLogUsageMb > capacity_mb )
        {
            LOG_PRINT(warning_e, "'%s' is too big [%d]. Use the maximum value %d\n",
                      MAX_SPACE_AVAILABLE_TAG,
                      MaxLogUsageMb,
                      capacity_mb);
            MaxLogUsageMb = capacity_mb;
        }

        /* if necessary delete the oldest store and alarm logfiles */
        while (checkSpace() == 1)
        {
            if (removeTheOldestLog(StorageDir, NULL))
            {
                LOG_PRINT(error_e, "cannot remove the oldest store log\n");
                break;
            }
        }
        while (checkSpace() == 1)
        {
            if (removeTheOldestLog(AlarmsDir, NULL))
            {
                LOG_PRINT(error_e, "cannot remove the oldest alarm log\n");
                break;
            }
        }
    }
#ifdef ENABLE_STORE
    counterS = 0;
    counterF = 0;
#endif
}

FILE * Logger::openFile(bool daily, int * newfile, const char * basedir, const char * subdir)
{
    /* create the log name in function of the current date */
    char logFileName[FILENAME_MAX];
    char CurrentDate[DESCR_LEN];

    time(&Now);
    memcpy(&CurrentTimeInfo, localtime(&Now),  sizeof(struct tm));

    if (daily)
    {
        strftime (CurrentDate, 32, "%Y_%m_%d", &CurrentTimeInfo);
    }
    else
    {
        strftime (CurrentDate, 32, "%Y_%m_%d_%H_%M_%S", &CurrentTimeInfo);
    }

    /* if necessary, create the subdir */
    if (subdir != NULL)
    {
        sprintf(logFileName, "%s/%s", basedir, subdir);
        mkdir(logFileName, S_IRWXU | S_IRWXG);
        sprintf(logFileName, "%s/%s/%s.log", basedir, subdir, CurrentDate);
    }
    else
    {
        sprintf(logFileName, "%s/%s.log", basedir, CurrentDate);
    }
    LOG_PRINT(verbose_e, "logFileName '%s'\n", logFileName);

    FILE * fp = NULL;
    
    fp = fopen(logFileName, "r");
    /* if ther log file doesn't exist, create it and dump the header */
    if (fp == NULL)
    {
        *newfile = 1;
        LOG_PRINT(verbose_e, "Opening new file '%s'\n", logFileName);
    }
    /* if ther log file exists, open it */
    else
    {
        *newfile = 0;
        LOG_PRINT(verbose_e, "Opening existing new file '%s'\n", logFileName);
        fclose(fp);
    }
    fp = fopen(logFileName, "a");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s' [%s]\n", logFileName, strerror(errno));
    }
    return fp;
}

#ifdef ENABLE_ALARMS
bool Logger::openAlarmsFile()
{
    int newfile = 0;
    if (alarmsfp != NULL)
    {
        LOG_PRINT(verbose_e, "the log file is already open\n");
        return true;
    }
    /*  open the log file */
    alarmsfp = openFile(true, &newfile, AlarmsDir);
    if (alarmsfp == NULL)
    {
        LOG_PRINT(error_e, "cannot dump the log\n");
        return false;
    }
    
    LOG_PRINT(verbose_e, "Opened log file\n");
    return true;
}
#endif

Logger::~Logger()
{
    /* close log file */
    closeAlarmsFile();
}

#ifdef ENABLE_ALARMS
bool Logger::connectToPage(QWidget * p)
{
    if (p != NULL)
    {
        return connect(p, SIGNAL(new_ack(event_msg_t *)), this, SLOT(dumpAck(event_msg_t *)));
    }
    return false;
}
#endif

void Logger::run()
{
#ifdef ENABLE_ALARMS
    QHash<QString, event_t *>::const_iterator i;
#endif
    int recompute_abstime = true;
    struct timespec abstime;
    sem_init(&theLoggingSem, 0, 0);

    logger_start = false;

    while (1)
    {
        if (recompute_abstime) {
            recompute_abstime = false;
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += _period_msec / 1000;
            abstime.tv_nsec += (_period_msec % 1000) * 1000 * 1000; // ms -> ns
            if (abstime.tv_nsec >= (1000*1000*1000)) {
                abstime.tv_sec += abstime.tv_nsec / (1000*1000*1000);
                abstime.tv_nsec = abstime.tv_nsec % (1000*1000*1000);
            }
        }
        /* waiting for maximum _period_msec */
        int rc = sem_timedwait(&theLoggingSem, &abstime);
        if (rc  == -1 && errno == EINTR){
            continue;
        }
        else if (rc == 0) {
            /* shot */
        }
        else if (errno == ETIMEDOUT) {
            /* timeout */
            counterS++;
            counterF++;
            recompute_abstime = true;
        } else {
            recompute_abstime = true;
        }

        /* get the actual time */
        time(&Now);
        timeinfo = localtime (&Now);

        /* check if the actual date is the same of the date described into the logfile name */
        /* if is different, close the actual log file then open a new one with the new date*/
        if (CurrentTimeInfo.tm_year < timeinfo->tm_year || CurrentTimeInfo.tm_yday < timeinfo->tm_yday)
        {
            memcpy(&CurrentTimeInfo, timeinfo, sizeof(struct tm));
#ifdef ENABLE_ALARMS
            closeAlarmsFile();
#endif
#ifdef ENABLE_STORE
            closeStorageFile();
#endif
        }

#ifdef ENABLE_ALARMS
        /* check each event */
        for ( i = EventHash.begin(); i != EventHash.end() && i.value() != NULL ; i++)
        {
            if (i.key().length() == 0)
            {
                continue;
            }
            /* if is active, dump if it is necessary and emit the signal */
            u_int32_t var = 0;
            LOG_PRINT(verbose_e, "Reading '%s' - %d\n", i.key().toAscii().data(), i.value()->CtIndex);
            if (readFromDbLock(i.value()->CtIndex, &var) == 0)
            {
                LOG_PRINT(verbose_e, "Reading '%s' 0x%X\n", i.key().toAscii().data(), var);
                /* an event is active */
                /* if it is the first time, set the begin time  */
                if (i.value()->begin == 0)
                {
                    i.value()->begin = Now;
                    LOG_PRINT(verbose_e, "%s - set begin as %ld\n", i.key().toAscii().data(), Now);
                }
                /* if no time filter is set or if the time filter is expired, dump the event */
                if (i.value()->filtertime == 0 || Now >= i.value()->begin + i.value()->filtertime)
                {
                    i.value()->begin = 0;
                    LOG_PRINT(verbose_e, "%s [%d] = %d - dumpEvent\n", i.key().toAscii().data(), i.value()->CtIndex, var);
                    dumpEvent(i.key(), i.value(), (var != 0) ? alarm_rise_e : alarm_fall_e);
                }
                else
                {
                    /* wait the time filter expiration to dump the event */
                }
            }
            else
            {
                LOG_PRINT(error_e, "cannot read variable '%s'\n", i.key().toAscii().data());
            }
        }
#endif
#ifdef ENABLE_STORE
        /* if the logger is started */
        if (logger_start)
        {
            if (logger_shot == false)
            {
                variation = checkVariation();
            }
            else
            {
                variation = false;
            }
            /* if there is something to dump */
            if (
                    logger_shot || variation
                    ||
                    (LogPeriodSecS > 0 && store_elem_nb_S > 0 && (counterS * _period_msec) >= (LogPeriodSecS * 1000))
                    ||
                    (LogPeriodSecF > 0 && store_elem_nb_F > 0 && (counterF * _period_msec) >= (LogPeriodSecF * 1000))
                    )
            {
                /* if the file is not open, open it */
                if (openStorageFile() == false)
                {
                    LOG_PRINT(error_e, "cannot open the store\n");
                    logger_shot = false;
                    return;
                }
                LOG_PRINT(verbose_e, "store opened\n");
                /* log the store variables */
                dumpStorage();
                logger_shot = false;
            }
        }
        else
        {
            closeStorageFile();
        }
#endif
    }
}

bool Logger::logstart()
{
    if (MaxLogUsageMb == 0)
    {
        LOG_PRINT(warning_e, "No space available for logs [%d]. No logs will be recorded.\n", MaxLogUsageMb);
        logger_start = false;
        return false;
    }
    logger_start = true;
    return true;
}

bool Logger::logstop()
{
    logger_start = false;
    return true;
}

bool Logger::logshot()
{
    logger_shot = true;
    LOG_PRINT(verbose_e, "SHOT!\n");
    if (sem_post(&theLoggingSem)) {
        LOG_PRINT(error_e, "sem_post() failed\n");
        return false;
    }
    while (logger_shot) {
        QCoreApplication::processEvents();
    }
    return true;
}

bool Logger::logreset()
{
    char command[LINE_SIZE];
    closeStorageFile();
    sprintf(command, "rm %s/*", STORE_DIR);
    return system(command) == 0;
}

#ifdef ENABLE_ALARMS
size_t Logger::loadAlarmsTable()
{
    FILE * fp;
    int elem_nb = 0;
    char line[LINE_SIZE];
    char token[LINE_SIZE] = "";
    char tag[LINE_SIZE] = "";
    char * p = NULL;

    /* read the error cross table */
    fp = fopen(CROSS_TABLE, "r");
    if (fp == NULL)
    {
        sprintf(CrossTableErrorMsg, "Cannot open The error cross table '%s': [%s]", CROSS_TABLE, strerror(errno));
        LOG_PRINT(error_e, "%s\n", CrossTableErrorMsg);
        return -1;
    }

    EventHash.clear();

    int index = 0;
    event_t * item = NULL;

    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        LOG_PRINT(verbose_e, "%s\n", line);
        p = strchr(line, ';');
        if (p == NULL)
        {
            LOG_PRINT(verbose_e, "skip empty line [%d]\n", elem_nb);
            continue;
        }

        index++;

        *p = '\0';
        int level = atoi(line);

        p = strrchr(p + 1, ';');
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed line");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            fclose(fp);
            return elem_nb;
        }

        /*
         *[RW] <comments>
         *[RO] <comments>
         *[AL <source> <operator> <value to compare>] <comments>
         *[EV <source> <operator> <value to compare>] <comments>
         *
         *<operator>: '>' '>=' '<' '<='  '==' '!=' 'RISING' 'FALLING'
         *<value to compare> only if <value to compare> is not 'RISING' or 'FALLING'
         */

        p = strrchr(p, '[');
        if (p == NULL)
        {
            continue;
        }

        /* alarm */
        if (strncmp(p, "[AL", 3) == 0)
        {
            item = (event_t*)calloc(1, sizeof(event_t));
            item->type = ALARM;
            item->persistence = 1;
            item->dump = 1;
        }
        /* event */
        else if (strncmp(p, "[EV", 3) == 0)
        {
            item = (event_t*)calloc(1, sizeof(event_t));
            item->type = EVENT;
            item->persistence = 0;
            item->dump = 0;
        }
        /* other */
        else
        {
            continue;
        }

        strcpy(tag, varNameArray[index].tag);
        LOG_PRINT(verbose_e, "FOUND ALARM '%s'\n", token);

        /* skip - source tag */

        /* skip - reference tag */

        /* skip - fix value */

        /* skip - operator */

        /* extract the time filter before sho the alarm/event */
        item->filtertime = 0;

        /* extract the description of the alarm/event */
        p = strrchr(p, ']');
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed line missing ']'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            free(item);
            fclose(fp);
            return elem_nb;
        }
        if (QString(p + 1).simplified().length() == 0)
        {
            strcpy(item->description, varNameArray[index].tag);
        }
        else
        {
            strcpy(item->description, QString(p + 1).simplified().toAscii().data());
        }

        /* extract the level of the alarm/event */
        item->level = level;

        /* extract the CT index of the alarm/event */
        item->CtIndex = index;

        EventHash.insert(tag, item);
        elem_nb++;
    }
    fclose(fp);
    LOG_PRINT(verbose_e, "Loaded %d record\n", elem_nb);

    return elem_nb;
}

/**
 * @brief return false if the alarm is tagged as INVISIBLE into the LevelColorTable or into the StatusColorTable
 */
int Logger::getElemAlarmStyleIndex(event_descr_t * event_msg)
{
    QHash<QString, event_t *>::const_iterator item = EventHash.find(event_msg->tag);
    if (item == EventHash.end())
    {
        LOG_PRINT(error_e, "FATAL: cannot find '%s' into hash table\n", event_msg->tag);
        return false;
    }
    
    event_t * event = item.value();
#if 0
    if (strcasecmp(LevelColorTable[event->level], INVISIBLE) == 0)
    {
        strcpy(event_msg->style, LevelColorTable[event->level]);
        return false;
    }
#endif
    /* active */
    if (event_msg->status == alarm_rise_e)
    {
        /* latched */
        if (event->persistence == 1)
        {
            /* acknowledged */
            if (event_msg->isack)
            {
                event_msg->styleindex = alrm_active_latched_ack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_active_latched_ack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
            /* not acknowledged */
            else
            {
                event_msg->styleindex = alrm_active_latched_nonack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_active_latched_nonack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
        }
        /* not latched */
        else
        {
            /* acknowledged */
            if (event_msg->isack)
            {
                event_msg->styleindex = alrm_active_nonlatched_ack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_active_nonlatched_ack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
            /* not acknowledged */
            else
            {
                event_msg->styleindex = alrm_active_nonlatched_nonack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_active_nonlatched_nonack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
        }
    }
    /* not active */
    else
    {
        /* latched */
        if (event->persistence == 1)
        {
            /* acknowledged */
            if (event_msg->isack)
            {
                event_msg->styleindex = alrm_nonactive_latched_ack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_nonactive_latched_ack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
            /* not acknowledged */
            else
            {
                event_msg->styleindex = alrm_nonactive_latched_nonack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_nonactive_latched_nonack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
        }
        /* not latched */
        else
        {
            /* acknowledged */
            if (event_msg->isack)
            {
                event_msg->styleindex = alrm_nonactive_nonlatched_ack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_nonactive_nonlatched_ack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
            /* not acknowledged */
            else
            {
                event_msg->styleindex = alrm_nonactive_nonlatched_nonack_e;
                LOG_PRINT(verbose_e, "STYLE: alrm_nonactive_nonlatched_nonack_e: '%s' status %d persistance %d ACK %d\n", event_msg->tag, event_msg->status, event->persistence, event_msg->isack);
            }
        }
    }
    LOG_PRINT(verbose_e, "EVENT %s color %d\n", event_msg->tag, event_msg->styleindex);
    return event_msg->styleindex;
}

bool Logger::dumpEvent(QString varname, event_t * item, enum alarm_event_e alarm_event)
{
    bool retval;
    bool todump = false;
    bool toemit = false;
    char msg[LINE_SIZE];
    char buffer [FILENAME_MAX] = "";
    event_descr_e * info_descr = NULL;
    bool to_append = false;
    bool to_free = false;

    strftime (buffer, FILENAME_MAX, "%Y/%m/%d,%H:%M:%S", timeinfo);
    
    pthread_mutex_lock(&alarmevents_list_mutex);
    {
        // called from dumpAck()
        if (varname.isEmpty() && item == NULL && alarm_event == alarm_ack_e)
        {
            retval = true;
            goto exit_function;
        }

        /* check if the alarm associated to the actual event is still into the _active_alarms_events_ */
        for (int i = 0; i < _active_alarms_events_.count(); i++)
        {
            /* if found, update the alarm with the last event */
            if (strcmp(_active_alarms_events_.at(i)->tag, varname.toAscii().data()) == 0)
            {
                info_descr = _active_alarms_events_.at(i);
                LOG_PRINT(verbose_e, "Update existing event for %s\n", info_descr->tag);
                break;
            }
        }

        /* the alarm is not active and is not into the active list */
        if (alarm_event == alarm_fall_e && info_descr == NULL)
        {
            LOG_PRINT(verbose_e, "Nothing interesting to dump...\n");
            retval = true;
            goto exit_function;
        }

        /* the alarm is not into the active list */
        if (info_descr == NULL)
        {
            info_descr = new event_descr_e;
            strcpy(info_descr->tag, varname.toAscii().data());
            info_descr->description[0] = '\0';
            info_descr->styleindex = nb_of_alarm_status_e;
            info_descr->isack = false;
            info_descr->begin = QDateTime();
            info_descr->end = QDateTime();
            info_descr->ack = QDateTime();
            info_descr->status = alarm_none_e;
            info_descr->type = item->type;
            to_append = true;
            to_free = true;
            LOG_PRINT(verbose_e, "New event for %s\n", info_descr->tag);
        }

        if (alarm_event == info_descr->status)  {
            /* avoid useless events */
            retval = true;
            goto exit_function;
        }

        switch (alarm_event)
        {
        case alarm_rise_e:
            toemit = true;
            if (info_descr->status == alarm_fall_e || info_descr->status == alarm_none_e)
            {
                LOG_PRINT(verbose_e, "Rising event for %s\n", info_descr->tag);
                HornACK = false;
                todump = true;
                info_descr->begin = QDateTime().fromString(buffer,"yyyy/MM/dd,HH:mm:ss");
                info_descr->isack = false;
            }
            else if (!info_descr->begin.isValid())
            {
                info_descr->begin = QDateTime().fromString(buffer,"yyyy/MM/dd,HH:mm:ss");
            }
            info_descr->status = alarm_rise_e;
            break;

        case alarm_fall_e:
            if (info_descr->status == alarm_rise_e)
            {
                LOG_PRINT(verbose_e, "Falling event for %s\n", info_descr->tag);
                toemit = true;
                todump = true;
                info_descr->end = QDateTime().fromString(buffer,"yyyy/MM/dd,HH:mm:ss");
                //info_descr->isack = false;
            }
            else if (!info_descr->end.isValid())
            {
                info_descr->end = QDateTime().fromString(buffer,"yyyy/MM/dd,HH:mm:ss");
            }
            info_descr->status = alarm_fall_e;
            break;

        case alarm_ack_e:
        default:
            ;
        }

        if (toemit)
        {
            getElemAlarmStyleIndex(info_descr);
            ForceResetAlarmBanner = true;
            if (to_append)
            {
                _active_alarms_events_.append(info_descr);
                to_free = false;
                LOG_PRINT(verbose_e, "ADD isack %d status %d\n", info_descr->isack, info_descr->status);
            }

            /* emit a signal to the hmi with the new item to display */
            if (item->type == EVENT)
            {
                LOG_PRINT(verbose_e, "Emit New event for %s status %d\n", info_descr->tag, info_descr->status);
                emit new_event(info_descr->tag);
            }
            else
            {
                LOG_PRINT(verbose_e, "Emit New alarm for %s status %d\n", info_descr->tag, info_descr->status);
                emit new_alarm(info_descr->tag);
            }
        }

        if (todump && item->dump == 1)
        {
            /* before dump a new event, check the available space */
            if (checkSpace() == 1)
            {
                /* if necessary delete the oldest logfile */
                switch (removeTheOldestLog(AlarmsDir, alarmsfp))
                {
                case -1:
                    alarmsfp = NULL;
                    openAlarmsFile();
                    break;
                case 0:
                    break;
                case 1:
                default:
                    LOG_PRINT(error_e, "cannot remove the oldest alarm log\n");
                    retval = false;
                    goto exit_function;
                }
            }

            char event[TAG_LEN];
            if (info_descr->status == alarm_rise_e)
            {
                strcpy(event, TAG_RISE);
            }
            else if (info_descr->status == alarm_fall_e)
            {
                strcpy(event, TAG_FALL);
            }
            else if (info_descr->status == alarm_ack_e)
            {
                strcpy(event, TAG_ACK);
                LOG_PRINT(warning_e, "Wrong ACK event for variable '%s'\n", info_descr->tag);
            }
            else
            {
                strcpy(event, TAG_UNK);
                LOG_PRINT(warning_e, "Unknown event for variable '%s'\n", info_descr->tag);
            }

            LOG_PRINT(verbose_e, "DUMP event: '%s' isack %d status %d\n", event, info_descr->isack, info_descr->status);

            /* prepare the event item */
#ifdef LEVEL_TYPE
            /* type;level;tag;event;YYYY/MM/DD,HH:mm:ss;description */
            sprintf(msg, "%d;%d;%s;%s;%s;%s\n",
                    item->type,
                    item->level,
                    varname.toAscii().data(),
                    event,
                    buffer,
                    item->description
                    );
#else
            /* tag;event;YYYY/MM/DD,HH:mm:ss;description */
            sprintf(msg, "%s;%s;%s;%s\n",
                    varname.toAscii().data(),
                    event,
                    buffer,
                    item->description
                    );
#endif

            /* dump the item into log file */
            if (openAlarmsFile() && alarmsfp)
            {
                fputs(msg, alarmsfp);
                fflush(alarmsfp);
                sync();
                doReloadAlarmsLog = true;
                system(RS_ALARM_PUSH_CMD " start > /dev/null 2>&1 &");
            }
        }
        retval = true;

exit_function:

        /* eventually dump the acknowledged and remove the expired */
        for (int i = 0; i < _active_alarms_events_.count(); i++)
        {
            event_descr_t *event_msg = _active_alarms_events_.at(i);

            // dump the just acknowledged alarms (no events)
            if ( event_msg->isack && event_msg->type == ALARM
              && (event_msg->styleindex == alrm_active_latched_nonack_e
                || event_msg->styleindex == alrm_nonactive_latched_nonack_e))
            {
                QHash<QString, event_t *>::const_iterator item;

                item = EventHash.find(event_msg->tag);
                if (item != EventHash.end())
                {
                    event_t * event_data = item.value();
#ifdef LEVEL_TYPE
                    /* type;level;tag;event;YYYY/MM/DD,HH:mm:ss;description */
                    sprintf(msg, "%d;%d;%s;%s;%s;%s\n",
                            event_msg->type,
                            event_msg->level,
                            event_msg->tag,
                            TAG_ACK,
                            event_msg->ack->toString("yyyy/MM/dd,HH:mm:ss").toAscii().data(),
                            event_data->description
                            );
#else
                    /* tag;event;YYYY/MM/DD,HH:mm:ss;description */
                    sprintf(msg, "%s;%s;%s;%s\n",
                            event_msg->tag,
                            TAG_ACK,
                            _active_alarms_events_.at(i)->ack.toString("yyyy/MM/dd,HH:mm:ss").toAscii().data(),
                            event_data->description
                            );
#endif
                    /* dump the event into log file */
                    if (openAlarmsFile() && alarmsfp)
                    {
                        fputs(msg, alarmsfp);
                        fflush(alarmsfp);
                        sync();
                        doReloadAlarmsLog = true;
                        system(RS_ALARM_PUSH_CMD " start > /dev/null 2>&1 &");
                    }
                }
            }

            // then update the styleindex (i.e. the alarm/event status)
            int index = getElemAlarmStyleIndex(_active_alarms_events_.at(i));

            // remove the expired alarms and events
            if (index >= alrm_nonactive_nonlatched_nonack_e)
            {
                LOG_PRINT(verbose_e, "REMOVE '%s'\n", _active_alarms_events_.at(i)->tag);
                event_descr_e * tmp = _active_alarms_events_.at(i);
                if (tmp)
                {
                    free(tmp);
                }
                _active_alarms_events_.removeAt(i);
                i--;
            }

        }

        if (to_free && info_descr != NULL)
        {
            // should not happen
            free(info_descr);
        }

    }
    pthread_mutex_unlock(&alarmevents_list_mutex);
    return retval;
}

bool Logger::closeAlarmsFile()
{
    if (alarmsfp != NULL)
    {
        fclose(alarmsfp);
        alarmsfp = NULL;
    }
    return true;
}

bool Logger::dumpAck(event_msg_e * info_msg)
{
    Q_UNUSED(info_msg);

    /* before dump a new event, check the available space */
    if (checkSpace() == 1)
    {
        /* if necessary delete the oldest logfile */
        switch (removeTheOldestLog(AlarmsDir, alarmsfp))
        {
        case -1:
            alarmsfp = NULL;
            openAlarmsFile();
            break;
        case 0:
            break;
        case 1:
        default:
            LOG_PRINT(error_e, "cannot remove the oldest alarm log\n");
            return false;
        }
    }
    
    dumpEvent(QString(""), NULL, alarm_ack_e);

    return true;
}
#endif

#ifdef ENABLE_STORE
bool Logger::openStorageFile()
{
    int newfile;
    if (storefp != NULL)
    {
        LOG_PRINT(verbose_e, "the log file is already open\n");
        return true;
    }
    /*  open the log file */
    storefp = openFile(false, &newfile, StorageDir);
    if (storefp == NULL)
    {
        LOG_PRINT(error_e, "cannot dump the log\n");
        return false;
    }
    
    if (newfile)
    {
        LOG_PRINT(verbose_e, "NEW log file\n");
        int nb_of_char = fprintf(storefp, "date; time");
        if (nb_of_char != (int)strlen("date; time"))
        {
            LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", "date; time", nb_of_char, (int)strlen("date; time"));
            return false;
        }
        
        for ( int i = 0; StoreArrayS[i].tag[0] != '\0'; i++)
        {
            LOG_PRINT(verbose_e, "dumping title %s\n", StoreArrayS[i].tag);
            nb_of_char = fprintf(storefp, "; %s", StoreArrayS[i].tag);
            if (nb_of_char != (int)strlen(StoreArrayS[i].tag) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", StoreArrayS[i].tag, nb_of_char, (int)strlen(StoreArrayS[i].tag) + 2);
                return false;
            }
        }
        for ( int i = 0; StoreArrayF[i].tag[0] != '\0'; i++)
        {
            LOG_PRINT(verbose_e, "dumping title %s\n", StoreArrayF[i].tag);
            nb_of_char = fprintf(storefp, "; %s", StoreArrayF[i].tag);
            if (nb_of_char != (int)strlen(StoreArrayF[i].tag) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", StoreArrayF[i].tag, nb_of_char, (int)strlen(StoreArrayF[i].tag) + 2);
                return false;
            }
        }
        for ( int i = 0; StoreArrayV[i].tag[0] != '\0'; i++)
        {
            LOG_PRINT(verbose_e, "dumping title %s\n", StoreArrayV[i].tag);
            nb_of_char = fprintf(storefp, "; %s", StoreArrayV[i].tag);
            if (nb_of_char != (int)strlen(StoreArrayV[i].tag) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", StoreArrayV[i].tag, nb_of_char, (int)strlen(StoreArrayV[i].tag) + 2);
                return false;
            }
        }
        for ( int i = 0; StoreArrayX[i].tag[0] != '\0'; i++)
        {
            LOG_PRINT(verbose_e, "dumping title %s\n", StoreArrayX[i].tag);
            nb_of_char = fprintf(storefp, "; %s", StoreArrayX[i].tag);
            if (nb_of_char != (int)strlen(StoreArrayX[i].tag) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", StoreArrayX[i].tag, nb_of_char, (int)strlen(StoreArrayX[i].tag) + 2);
                return false;
            }
        }

        if (fprintf(storefp, "\n") != 1)
        {
            LOG_PRINT(error_e, "cannot dump: '%s'\n", strerror(errno));
            return false;
        }
        fflush(storefp);
    }
    LOG_PRINT(verbose_e, "Opened log file\n");
    return true;
}

bool Logger::closeStorageFile()
{
    if (storefp != NULL)
    {
        fclose(storefp);
        storefp = NULL;
        return true;
    }
    else
    {
        return false;
    }
}

bool Logger::checkVariation()
{
    char svalue [42] = "";
    for (int i = 0; StoreArrayV[i].tag[0] != '\0'; i++)
    {
        register int ctIndex = StoreArrayV[i].CtIndex;
        register char status = pIODataStatusAreaI[ctIndex];

        if (status != DONE && status != BUSY) {
            LOG_PRINT(error_e, "cannot read variable %d '%s'\n", StoreArrayV[i].CtIndex, StoreArrayV[i].tag );
            continue;
        } else {
            register int *p = (int *)IODataAreaI;
            register int ivalue = p[ctIndex]; // atomic, quick and dirty, without locking
            register int decimal = getVarDecimalByCtIndex(ctIndex); // locks only if it's from another variable

            /* dump only if the last value was different */
            sprintf_fromValue(svalue, ctIndex, ivalue, decimal, 10);
            if (strcmp(StoreArrayV[i].value, svalue) != 0)
            {
                return true;
            }
        }
    }
    return false;
}

bool Logger::dumpStorage()
{
    char buffer [FILENAME_MAX] = "";
    char value [42] = "";
    int iF, iS, iV, iX;
#ifdef ENABLE_TREND
    trend_msg_t info_msg;
#endif
    
    /* before dump a new event, check the available space */
    if (checkSpace() == 1)
    {
        /* if necessary delete the oldest logfile, considering that it may be unique */
        switch (removeTheOldestLog(StorageDir, storefp))
        {
        case -1:
            storefp = NULL;
            openStorageFile();
            break;
        case 0:
            break;
        case 1:
        default:
            LOG_PRINT(error_e, "cannot remove the oldest store log\n");
            return false;
        }
    }
    
    if (storefp == NULL)
    {
        LOG_PRINT(error_e, "cannot dump: (storefp is null).\n");
        return false;
    }

    /* prepare the event item */
    strftime (buffer, FILENAME_MAX, "%Y/%m/%d; %H:%M:%S", timeinfo);
    
    LOG_PRINT(verbose_e, "dumping a new line '%s'\n", buffer);
    int nb_of_char = fprintf(storefp, "%s", buffer);
    if (nb_of_char != (int)strlen(buffer))
    {
        LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", buffer, nb_of_char, (int)strlen(buffer));
        return false;
    }

    if (logger_shot || (store_elem_nb_S > 0  && (counterS * _period_msec) >= (LogPeriodSecS * 1000)))
    {
        counterS = 0;
        for ( iS = 0; StoreArrayS[iS].tag[0] != '\0'; iS++)
        {
            float fvalue = sprintf_fromDb(value, StoreArrayS[iS].CtIndex);
            
            nb_of_char = fprintf(storefp, "; %s", value);
            if (nb_of_char != (int)strlen(value) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", value, nb_of_char, (int)strlen(value) + 2);
                return false;
            }
            
#ifdef ENABLE_TREND
            /* emit a signal to the hmi with the new item to display */
            info_msg.CtIndex = StoreArrayS[iS].CtIndex;
            info_msg.timestamp = QDateTime::fromString(buffer,"yyyy/MM/dd; HH:mm:ss");
            info_msg.value = fvalue;
            emit new_trend(info_msg);
            LOG_PRINT(verbose_e, "emitted '%d' at '%s'(%s) value %s(%f)\n", info_msg.CtIndex, info_msg.timestamp.toString("yy/MM/dd HH:mm:ss").toAscii().data(), buffer, value, info_msg.value);
#endif
        }
    }
    else if (variation || (store_elem_nb_F > 0  || (counterF * _period_msec) >= (LogPeriodSecF * 1000)))
    {
        for ( iS = 0; StoreArrayS[iS].tag[0] != '\0'; iS++)
        {
            nb_of_char = fprintf(storefp, "; %s", "-");
            if (nb_of_char != (int)strlen("-") + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", "-", nb_of_char, (int)strlen("-") + 2);
                return false;
            }
        }
        iS = 0;
    }
    
    if (logger_shot || (store_elem_nb_F > 0  && (counterF * _period_msec) >= (LogPeriodSecF * 1000)))
    {
        counterF = 0;
        for ( iF = 0; StoreArrayF[iF].tag[0] != '\0'; iF++)
        {
            float fvalue = sprintf_fromDb(value, StoreArrayF[iF].CtIndex);
            
            nb_of_char = fprintf(storefp, "; %s", value);
            if (nb_of_char != (int)strlen(value) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", value, nb_of_char, (int)strlen(value) + 2);
                return false;
            }
            
#ifdef ENABLE_TREND
            /* emit a signal to the hmi with the new item to display */
            info_msg.CtIndex = StoreArrayF[iF].CtIndex;
            info_msg.timestamp = QDateTime::fromString(buffer,"yyyy/MM/dd; HH:mm:ss");
            info_msg.value = fvalue;
            emit new_trend(info_msg);
            LOG_PRINT(verbose_e, "emitted '%d' at '%s'(%s) value %s(%f)\n", info_msg.CtIndex, info_msg.timestamp.toString("yy/MM/dd HH:mm:ss").toAscii().data(), buffer, value, info_msg.value);
#endif
        }
    }
    else if (variation || (store_elem_nb_S > 0  || (counterS * _period_msec) >= (LogPeriodSecS * 1000)))
    {
        for ( iF = 0; StoreArrayF[iF].tag[0] != '\0'; iF++)
        {
            nb_of_char = fprintf(storefp, "; %s", "-");
            if (nb_of_char != (int)strlen("-") + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", "-", nb_of_char, (int)strlen("-") + 2);
                return false;
            }
        }
        iF = 0;
    }

    if (logger_shot || (store_elem_nb_V > 0 && variation))
    {
        for ( iV = 0; StoreArrayV[iV].tag[0] != '\0'; iV++)
        {
            float fvalue = sprintf_fromDb(value, StoreArrayV[iV].CtIndex);

            /* dump only if the last value was different */
            if (logger_shot || strcmp(StoreArrayV[iV].value, value) != 0)
            {
                strcpy(StoreArrayV[iV].value, value);
                nb_of_char = fprintf(storefp, "; %s", value);
                if (nb_of_char != (int)strlen(value) + 2)
                {
                    LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", value, nb_of_char, (int)strlen(value) + 2);
                    return false;
                }
            }
            else
            {
                nb_of_char = fprintf(storefp, "; %s", "-");
                if (nb_of_char != (int)strlen("-") + 2)
                {
                    LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", "-", nb_of_char, (int)strlen("-") + 2);
                    return false;
                }
            }

#ifdef ENABLE_TREND
            /* emit a signal to the hmi with the new item to display */
            info_msg.CtIndex = StoreArrayV[iV].CtIndex;
            info_msg.timestamp = QDateTime::fromString(buffer,"yyyy/MM/dd; HH:mm:ss");
            info_msg.value = fvalue;
            emit new_trend(info_msg);
            LOG_PRINT(verbose_e, "emitted '%d' at '%s'(%s) value %s(%f)\n", info_msg.CtIndex, info_msg.timestamp.toString("yy/MM/dd HH:mm:ss").toAscii().data(), buffer, value, info_msg.value);
#endif
        }
    }
    else
    {
        for ( iV = 0; StoreArrayV[iV].tag[0] != '\0'; iV++)
        {
            nb_of_char = fprintf(storefp, "; %s", "-");
            if (nb_of_char != (int)strlen("-") + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", "-", nb_of_char, (int)strlen("-") + 2);
                return false;
            }
        }
        iV = 0;
    }

    if (logger_shot && store_elem_nb_X > 0)
    {
        for ( iX = 0; StoreArrayX[iX].tag[0] != '\0'; iX++)
        {
            float fvalue = sprintf_fromDb(value, StoreArrayX[iX].CtIndex);

            nb_of_char = fprintf(storefp, "; %s", value);
            if (nb_of_char != (int)strlen(value) + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", value, nb_of_char, (int)strlen(value) + 2);
                return false;
            }

#ifdef ENABLE_TREND
            /* emit a signal to the hmi with the new item to display */
            info_msg.CtIndex = StoreArrayX[iX].CtIndex;
            info_msg.timestamp = QDateTime::fromString(buffer,"yyyy/MM/dd; HH:mm:ss");
            info_msg.value = fvalue;
            emit new_trend(info_msg);
            LOG_PRINT(verbose_e, "emitted '%d' at '%s'(%s) value %s(%f)\n", info_msg.CtIndex, info_msg.timestamp.toString("yy/MM/dd HH:mm:ss").toAscii().data(), buffer, value, info_msg.value);
#endif
        }
    }
    else
    {
        for ( iX = 0; StoreArrayX[iX].tag[0] != '\0'; iX++)
        {
            nb_of_char = fprintf(storefp, "; %s", "-");
            if (nb_of_char != (int)strlen("-") + 2)
            {
                LOG_PRINT(error_e, "cannot dump '%s' %d vs %d\n", "-", nb_of_char, (int)strlen("-") + 2);
                return false;
            }
        }
        iX = 0;
    }

    if (iF == 0 && iS == 0 && iV == 0 && iX == 0)
    {
        LOG_PRINT(verbose_e, "No signal emitted\n");
#if 0
#ifndef LOG_DISABLED
        for ( int i = 0; i < 10; i++)
        {
            LOG_PRINT(error_e, "StoreArray[%d] = '%s'\n",i, StoreArray[i].tag );
        }
#endif
#endif
    }
    if (fprintf(storefp, "\n") != 1)
    {
        LOG_PRINT(error_e, "cannot dump\n");
        return false;
    }
    fflush(storefp);
    LOG_PRINT(verbose_e, "dumped\n");
    return true;
}
#endif

int sum(__attribute__((unused)) const char *fpath, const struct stat *sb, __attribute__((unused)) int typeflag) {
    totalBytes += sb->st_size;
    return 0;
}

int Logger::checkSpace( void )
{
    struct statvfs fiData;
    
    if((statvfs(LOCAL_DATA_DIR,&fiData)) < 0 )
    {
        LOG_PRINT(error_e,"failed to stat %s:\n", LOCAL_DATA_DIR);
        return -1;
    }
    
    /* it must be available at least 1Mb */
    if (fiData.f_bfree * fiData.f_bsize < 1024)
    {
        LOG_PRINT(warning_e,"the available space is almost finished: free %ld[bytes], minimum free %d[bytes].\n", fiData.f_bfree * fiData.f_bsize, 1024);
        return 1;
    }
    
    /* the space occupied by data bust be less than MaxLogUsageMb */
    if (getSizeDir(STORE_DIR) >= (unsigned long int)MaxLogUsageMb * 1024 * 1024)
    {
        LOG_PRINT(warning_e,"space used is %ld[bytes], max space usable %ld[bytes]\n",
                  totalBytes, (unsigned long int)MaxLogUsageMb * 1024 * 1024);
        return 1;
    }
    LOG_PRINT(verbose_e, "space free %ld, space used %ld, Max space usable %d\n",
              fiData.f_bfree * fiData.f_bsize,
              totalBytes,
              MaxLogUsageMb * 1024 * 1024
              );

    return 0;
}

unsigned long Logger::getSizeDir(const char *dirname) {
    totalBytes = 0;
    if (access(dirname, R_OK)) {
        return 0;
    }
    if (ftw(dirname, &sum, 1)) {
        perror("ftw");
        return 0;
    }
    LOG_PRINT(verbose_e, "%s: %ld\n", dirname, totalBytes);
    return totalBytes;
}

unsigned long Logger::getCapacityDir(const char *dirname) {
    struct statvfs fiData;

    if((statvfs(dirname,&fiData)) < 0 )
    {
        LOG_PRINT(error_e,"failed to stat %s. Disable logs\n", LOCAL_DATA_DIR);
        MaxLogUsageMb = 0;
    }

    unsigned long capacity =
            (unsigned long)
            (
                (unsigned long)((float)fiData.f_bavail / 1024 * fiData.f_bsize)
                +
                (unsigned long)(getSizeDir(LOCAL_DATA_DIR) / 1024)
            )
            ;
    LOG_PRINT(verbose_e, "%s: %ld\n", dirname, capacity);
    return capacity;
}

int Logger::removeOldest(const char * dir)
{
    Q_UNUSED(dir);
    return 1;
}

int removeTheOldestLog(const char * dir, FILE *current)
{
    struct dirent **filelist;
    int fcount;
    int i;
    char filename[FILENAME_MAX];
    int retval = 1;
    
    fcount = scandir(dir, &filelist, 0, alphasort);
    
    if (fcount <= 0)
    {
        LOG_PRINT (error_e, "No file to remove\n");
        perror(dir);
        return 1;
    }

    for (i = 0; i < fcount; i++)
    {
        if (strcmp(filelist[i]->d_name, ".") == 0 || strcmp(filelist[i]->d_name, "..") == 0)
        {
            // skip the current and parent directory
            continue;
        }
        sprintf(filename, "%s/%s", dir, filelist[i]->d_name);

        // check if it is the current logfile
        if (current != NULL)
        {
            struct stat a, b;
            int inode_a, inode_b;
            int fd = fileno(current);

            if (stat(filename, &a))
                continue;
            inode_a = a.st_ino;

            if (fstat(fd, &b))
                continue;
            inode_b = b.st_ino;

            if (inode_a == inode_b)
            {
                LOG_PRINT (error_e, "removed current log file '%s'\n", filelist[i]->d_name);
                fclose(current);
                unlink(filename);
                retval = -1;
                break;
            }
        }
        if (unlink(filename) != 0)
        {
            LOG_PRINT (error_e, "cannot remove oldest log file '%s'\n", filelist[i]->d_name);
            retval = 1;
        }
        else
        {
            LOG_PRINT (warning_e, "removed oldest log file '%s'\n", filelist[i]->d_name);
            retval = 0;
        }
        break;
    }
    
    for (i = 0; i < fcount; i++)
    {
        free(filelist[i]);
    }
    free(filelist);

    return retval;
}
