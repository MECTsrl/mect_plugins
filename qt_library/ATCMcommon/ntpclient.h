#ifndef NTPCLIENT_H
#define NTPCLIENT_H

#include <QSettings>
#include <QCoreApplication>

#include <byteswap.h>
#include <netdb.h> // getaddrinfo()
#include <arpa/inet.h> // inet_ntop()
#include <time.h>
#include <unistd.h>

class ntpclient
{

private:
    bool ntpClientProcedure();
    int64_t getTimestamp();
    void setTimestamp(int64_t TargetTimestamp);

public:
    ntpclient();
    bool doNTPSync();

    static int getTimeZoneDST();
    static void setTimeZoneDST(int tzDST);

    static QString getServerName();
    static void setServerName(QString serverName);

};

#endif //  NTPCLIENT_H
