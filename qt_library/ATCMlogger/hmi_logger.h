/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Thread to dump the events and the alarms
 */
#ifndef HMI_LOGGER_H
#define HMI_LOGGER_H

#include <QMetaType>
#include <QThread>
#include <QHash>
#include <QDateTime>
#include <semaphore.h>

#include "common.h"
#include "global_var.h"
#include "ntpclient.h"

#define RECORD_SIZE_BYTE 100
#define MAX_RECORD_PER_DAY (24 * 3600 / 5)
#define ALARMS_PERIOD_MS 1000
#define RS_ALARM_PUSH_CMD "/usr/sbin/rs_notify_alert.sh"

typedef struct event_msg_e
{
    char tag[TAG_LEN];
    QDateTime time;
    int event;
} event_msg_t;

typedef struct trend_msg_e
{
    int CtIndex;
    QDateTime timestamp;
    float value;
} trend_msg_t;
Q_DECLARE_METATYPE(trend_msg_t);

enum alarm_level_e
{
    level_all_e,
    level1_e,
    level2_e,
    level3_e,
    level4_e,
    level5_e,
    level6_e,
    level7_e,
    level8_e,
    level9_e,
    level10_e,
    level11_e,
    level12_e,
    level13_e,
    level14_e,
    level15_e,
    level16_e,
    level17_e,
    level18_e,
    level19_e,
    level20_e,
    level21_e,
    level22_e,
    level23_e,
    level24_e,
    level25_e,
    level26_e,
    level27_e,
    level28_e,
    level29_e,
    level30_e,
    level31_e,
    level32_e,
    nb_of_level_e
};

enum alarm_status_e
{
    alrm_active_latched_nonack_e = 0,
    alrm_active_nonlatched_nonack_e,
    alrm_active_latched_ack_e,
    alrm_active_nonlatched_ack_e,
    alrm_nonactive_latched_nonack_e,
    alrm_nonactive_nonlatched_nonack_e,
    alrm_nonactive_latched_ack_e,
    alrm_nonactive_nonlatched_ack_e,
    nb_of_alarm_status_e
};

/**
 * @brief this table will filled with the color to be used for each error staatus:
 * rgb sintax is used (example white is "rgb(255,255,255);")
 * tag "invisible" is used to hide the item
 */
extern char StatusColorTable[nb_of_alarm_status_e][DESCR_LEN];
extern char StatusBannerColorTable[nb_of_alarm_status_e][DESCR_LEN];

#define INVISIBLE "invisible"
#define ISBANNER(index) (strcasecmp(StatusBannerColorTable[index], INVISIBLE) != 0)
#define ISSTATUS(index) (strcasecmp(StatusColorTable[index], INVISIBLE) != 0)

class Logger : public QThread
{
    Q_OBJECT
public:
    explicit Logger(const char * alarms_dir = NULL, const char * store_dir = NULL, int period_msec = ALARMS_PERIOD_MS, QObject *parent = 0);
    ~Logger();
    virtual void run();
    bool connectToPage(QWidget * p);
    bool logstart();
    bool logstop();
    bool logshot();
    bool logreset();
    bool logrunning();
private:
    FILE * openFile(bool daily, int * newfile, const char * basedir, const char * subdir = NULL);
    int getElemAlarmStyleIndex(event_descr_t * event_msg);
    size_t loadAlarmsTable();
    bool dumpEvent(QString varname, event_t * item, alarm_event_e alarm_event);
    bool openAlarmsFile();
    bool closeAlarmsFile();
    bool dumpStorage(bool timeChanged = false, QDateTime timeBefore = QDateTime());
    bool openStorageFile();
    bool closeStorageFile();
    bool checkVariation();
    int checkSpace( void );
    int removeOldest(const char * dir);
    unsigned long getSizeDir(const char *dirname);
    unsigned long getCapacityDir(const char *dirname);
private:
    time_t Now;
    struct tm * timeinfo;
    struct tm CurrentTimeInfo;
    FILE * alarmsfp;
    FILE * storefp;
    int _period_msec;
    char AlarmsDir[FILENAME_MAX];
    char StorageDir[FILENAME_MAX];
    int counterS;
    int counterF;
    bool logger_start;
    bool logger_shot;
    bool variation;
private slots:
    bool dumpAck(event_msg_t * info_msg);
signals:
    void new_event(char * tag);
    void new_alarm(char * tag);
    void new_trend(trend_msg_t msg);
};

extern QHash<QString, event_t *> EventHash;

extern Logger * logger;
extern sem_t theLoggingSem;
extern bool doReloadAlarmsLog;

#define logStart() logger->logstart()
#define logStop()  logger->logstop()
#define logShot()  logger->logshot()
#define logReset() logger->logreset()
#define logRunning() logger->logrunning()

#endif // HMI_LOGGER_H
