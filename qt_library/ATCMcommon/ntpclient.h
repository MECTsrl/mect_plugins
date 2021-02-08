#ifndef NTPCLIENT_H
#define NTPCLIENT_H

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
public:
    explicit NtpClient(QObject *parent = 0);

    QString     getNtpServer();
    int         getTimeout_s();
    float       getOffset_h();
    int         getPeriod_h();
    void        setNtpParams(const QString &ntpServer, int ntpTimeout_s, float ntpOffset_h, int ntpPeriod_h);

signals:
    void        ntpSyncFinish(bool timeOut);
    void        ntpDateTimeChangeFinish();

public slots:
    void        requestNTPSync();
    void        requestDateTimeChange(QDateTime newTime);

private:
    int64_t         getTimestamp();
    void            setTimestamp(int64_t TargetTimestamp);
    bool            ntpClientProcedure();


    QElapsedTimer   timer;
    bool            doSync;
    QDateTime       newDateTime;
    QString         ntpServerName;
    int             ntpTimeout;
    float           ntpOffset;
    int             ntpPeriod;

protected:
    bool            ntpSyncOrChangeRequested();
    void            doSyncOrChange();

};

extern NtpClient    *ntpclient;


#endif //  NTPCLIENT_H
