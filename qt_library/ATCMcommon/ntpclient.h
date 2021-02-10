#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#include "hmi_logger.h"
#include <QSettings>
#include <QCoreApplication>
#include <QObject>
#include <QString>
#include <QElapsedTimer>
#include <QDateTime>
#include <sys/types.h>

#define THE_NTP_MAX_PERIOD_H    2982       // Massimo numero di ore convertite in ms per Int32
#define THE_NTP_SERVER     "tempo.ien.it"


class NtpClient : public QObject
{
    Q_OBJECT
    friend class Logger;

public:
    explicit NtpClient(QObject *parent = 0);
    QString     getNtpServer();
    int         getTimeout_s();
    int         getOffset_h();
    int         getPeriod_h();
    void        setNtpParams(const QString &ntpServer, int ntpTimeout_s, int ntpOffset_h, int ntpPeriod_h);

signals:
    void        ntpSyncFinish(bool timeOut);                    // ntp Sync Ended
    void        ntpDateTimeChangeFinish(bool setOk);            // Manual Sync Ended

public slots:
    void        requestNTPSync();                               // Starts ntp time sync
    void        requestDateTimeChange(QDateTime newTime);       // Force RTC to newTime

protected:
    bool            ntpSyncOrChangeRequested();     // check if a Clock change is requested
    void            doSyncOrChange();               // do a Clock change

private:
    int64_t         getTimestamp();
    void            setTimestamp(int64_t TargetTimestamp);
    bool            ntpClientProcedure();


    QElapsedTimer   ntpSyncTimer;           // Timer on ntpPeriod
    bool            doSync;
    QDateTime       newDateTime;            // Manual QDateTime to set
    QDateTime       invalidDateTime;        // Invalid QDateTime

    QString         ntpServerName;          // ntp params
    int             ntpTimeout;
    int             ntpOffset;
    int             ntpPeriod;
    qint64          ntpPeriodms;

};

extern NtpClient    *ntpclient;


#endif //  NTPCLIENT_H
