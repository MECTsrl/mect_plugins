#include "ntpclient.h"

#include "common.h"

#include <byteswap.h>
#include <netdb.h> // getaddrinfo()
#include <arpa/inet.h> // inet_ntop()
#include <time.h>
#include <unistd.h>


#define THE_NTP_PORT       123
// #define THE_NTP_TIMEOUT_ms 9000
#define THE_MAX_TRIPTIME_s 1LL

NtpClient * ntpclient = NULL;

NtpClient::NtpClient(QObject *parent) :
    QThread(parent)
{
    QSettings  *objSettings = new QSettings(NTP_FILE, QSettings::IniFormat);
    objSettings->beginGroup("NTP-Server");
    ntpServerName = objSettings->value("serverName", THE_NTP_SERVER).toString();
    ntpTimeout = objSettings->value("serverTimeOut", "10").toInt();
    ntpOffset = objSettings->value("serverOffset", "1").toInt();
    ntpPeriod = objSettings->value("serverPeriod", "0").toInt();
    ntpPeriodms = ntpPeriod * 1000 * 3600;
    objSettings->endGroup();
    // Default values to stop automatic sync
    doSync = false;
    invalidDateTime = QDateTime();
    newDateTime = invalidDateTime;
    // Attiva il timer NTP
    if (ntpPeriod > 0)  {
        requestNTPSync();
        ntpSyncTimer.start();
    }
    timeChanged = false;
}

QString     NtpClient::getNtpServer()
{
    return ntpServerName;
}

int         NtpClient::getTimeout_s()
{
    return ntpTimeout;
}

int       NtpClient::getOffset_h()
{
    return ntpOffset;
}

int         NtpClient::getPeriod_h()
{
    return ntpPeriod;
}

bool        NtpClient::isTimeChanged()
{
    bool fRes;
    if (timeChanged) {
        timeChanged = false;
        fRes = true;
    } else {
        fRes = false;
    }
    return fRes;
}

void        NtpClient::setNtpParams(const QString &ntpServer, int ntpTimeout_s, int ntpOffset_h, int ntpPeriod_h )
{
    QSettings  *objSettings = new QSettings(NTP_FILE, QSettings::IniFormat);
    objSettings->beginGroup("NTP-Server");
    // Server
    ntpServerName = ntpServer.trimmed();
    ntpServerName = ntpServerName.isEmpty() ? QString(THE_NTP_SERVER) : ntpServerName;
    objSettings->setValue("serverName", ntpServerName);
    // TimeOut
    ntpTimeout = ntpTimeout_s;
    ntpTimeout = ntpTimeout > 0 ? ntpTimeout : 1;
    objSettings->setValue("serverTimeOut", ntpTimeout);
    // Offset
    ntpOffset = ntpOffset_h;
    ntpOffset = (ntpOffset > 12 || ntpOffset < -12) ? 1 : ntpOffset;
    objSettings->setValue("serverOffset", ntpOffset);
    // Period
    ntpPeriod = ntpPeriod_h;
    ntpPeriod = (ntpPeriod < 0 || (ntpPeriod > THE_NTP_MAX_PERIOD_H)) ? 0 : ntpPeriod;
    ntpPeriodms = ntpPeriod * 1000 * 3600;
    objSettings->setValue("serverPeriod", ntpPeriod);
    objSettings->endGroup();
    // Attiva il timer NTP
    if (ntpPeriod > 0)  {
        // no requestNTPSync();
        ntpSyncTimer.restart();
    }
}

void        NtpClient::requestNTPSync()
// Reserve next ntp time sync
{
    newDateTime = invalidDateTime;
    timeChanged = true;
    doSync = true;
    ntpSem.release(1);
}

void        NtpClient::requestDateTimeChange(QDateTime newTime)
// Reserve next Manual Date Time Set
{
    newDateTime = newTime;
    timeChanged = true;
    doSync = false;
    ntpSem.release(1);
}

bool        NtpClient::ntpSyncOrChangeRequested()
// check if a Clock change is requested
{
    return (
            newDateTime.isValid()   ||                                  // manual DateTime sync request
            doSync                  ||                                  // manual ntp sync request
            (ntpPeriodms > 0 && ntpSyncTimer.elapsed() > ntpPeriodms)       // automatic ntp sync
            );
}

void        NtpClient::doSyncOrChange()
// do a Clock change
{
    doSync = false;
    // Manual Date Request
    if (newDateTime.isValid())  {
        time_t rt = 0;
        struct tm *pt = NULL;
        struct timezone timez;
        struct timeval temp;
        int rc = 0;
        bool manualOK = false;

        // Lettura del RTC
        rt = time(NULL);
        pt = localtime(&rt);
        if (pt == NULL) {
            fputs(__func__, stderr);
            perror(": while getting local time");
            fflush(stderr);
            goto endManual;
        }
        // Impostazione dei nuovi valori
        pt->tm_year = newDateTime.date().year() - 1900;
        pt->tm_mon = newDateTime.date().month() - 1;
        pt->tm_mday = newDateTime.date().day();

        pt->tm_hour = newDateTime.time().hour();
        pt->tm_min = newDateTime.time().minute();
        pt->tm_sec = newDateTime.time().second();

        rc = gettimeofday(&temp, &timez);
        if (rc < 0) {
            fputs(__func__, stderr);
            perror(": while getting time of day");
            fflush(stderr);
            goto endManual;
        }

        mutexNTP.lock();
        {
            temp.tv_sec = mktime(pt);
            temp.tv_usec = 0;

            rc = settimeofday(&temp, &timez);
            if (rc < 0) {
                fputs(__func__, stderr);
                perror(": while setting time of day");
                fflush(stderr);
            } else {

                // Update RTC from system
                system("/sbin/hwclock -wu");

                manualOK = true;
            }
        }
        mutexNTP.unlock();
    endManual:
        newDateTime = invalidDateTime;
        emit ntpDateTimeChangeFinish(manualOK);
        return;

    }
    else  {
        bool autoSyncOK = false;

        if (ntpPeriod > 0) {
            ntpSyncTimer.restart();
        }
        autoSyncOK = ntpClientProcedure();
        emit ntpSyncFinish(! autoSyncOK);
    }
}

// -------------------------------------------------------------------------

#pragma pack(push, 1)

struct NtpPacketHeader {
    uint8_t Mode : 3;
    uint8_t VN : 3;
    uint8_t LI : 2;
    uint8_t Stratum;
    uint8_t Poll;
    uint8_t Precision;
    uint32_t RootDelay;
    uint32_t RootDispersion;
    uint32_t ReferenceIdentifier;
    int64_t ReferenceTimestamp;
    int64_t OriginateTimestamp;
    int64_t ReceiveTimestamp;
    int64_t TransmitTimestamp;
    uint32_t KeyIdentifier;
    uint32_t MessageDigest[4];
};

#pragma pack(pop)

void swapPacket(const struct NtpPacketHeader &from, struct NtpPacketHeader &to)
{
    to.Mode = from.Mode;
    to.VN = from.VN;
    to.LI = from.LI;
    to.Stratum = from.Stratum;
    to.Poll = from.Poll;
    to.Precision = from.Precision;
    to.RootDelay = bswap_32(from.RootDelay);
    to.RootDispersion = bswap_32(from.RootDispersion);
    to.ReferenceIdentifier = bswap_32(from.ReferenceIdentifier);
    to.ReferenceTimestamp = bswap_64(from.ReferenceTimestamp);
    to.OriginateTimestamp = bswap_64(from.OriginateTimestamp);
    to.ReceiveTimestamp = bswap_64(from.ReceiveTimestamp);
    to.TransmitTimestamp = bswap_64(from.TransmitTimestamp);
    to.KeyIdentifier = bswap_32(from.KeyIdentifier);
    for (int i = 0; i < 4; ++i) {
        to.MessageDigest[i] = from.MessageDigest[i]; // no swap
    }
}

int64_t  NtpClient::getTimestamp()
{
    int64_t retval;
    struct timespec now;
    time_t seconds;
    uint32_t fraction;

    clock_gettime(CLOCK_REALTIME, &now);

    // NTP date from 1900-01-01 <---> UNIX date from 1970-01-01
    seconds = now.tv_sec + ((time_t) 2208988800UL);

    // NTP fixed point [s/2^32] <---> POSIX fixed point [ns=s/10^9]
    fraction = (uint32_t)(now.tv_nsec * 4.294967296); // ns / 1000000000.0 * 4294967296.0
    retval = ((int64_t)seconds << 32) + fraction;

    //qDebug("get time = 0x%08lx s, ntp 0x%016llx\n", now.tv_sec, retval);
    return retval;
}

void NtpClient::setTimestamp(int64_t TargetTimestamp)
{
    struct timespec target;
    time_t seconds;
    uint32_t fraction;

    // NTP fixed point [s/2^32] <---> POSIX fixed point [ns=s/10^9]
    seconds = TargetTimestamp >> 32;
    fraction = TargetTimestamp & 0xFFFFffff;
    target.tv_nsec = (uint32_t)(fraction / 4.294967296);

    // NTP date from 1900-01-01 <---> UNIX date from 1970-01-01
    target.tv_sec = seconds - ((time_t) 2208988800UL);

    //qDebug("set time = 0x%08lx s, ntp 0x%016llx\n", target.tv_sec, TargetTimestamp);

    // time zone correction
    //target.tv_sec += 3600 * TimeZone;
    target.tv_sec += 3600 * ntpOffset;

    // set time
    clock_settime(CLOCK_REALTIME, &target);
}

bool NtpClient::ntpClientProcedure()
{
    struct  NtpPacketHeader request, reply;
    int     packetSize = sizeof(struct NtpPacketHeader);
    int     theUdpSocket = -1;
    bool    fRes = false;

    /*
     * creazione pacchetti dati NTP
     */
    request.VN = 4; // SNTP/NTP version number
    request.Mode = 3; // client
    request.TransmitTimestamp = getTimestamp();

    /*
     * creazione del socket UDP
     */
    theUdpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (theUdpSocket == -1) {
       // qDebug("ERROR: cannot create socket\n");
        goto exit_function;
    }

    /*
     * creazione degli indirizzi IP, con porte
     */
    {
        struct sockaddr_in ntpServerAddr;
        struct sockaddr_in localAddr;
        int sentBytes = -1;

        memset(&localAddr, 0, sizeof(localAddr));
        memset(&ntpServerAddr, 0, sizeof(ntpServerAddr));

        localAddr.sin_family = AF_INET;
        localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        localAddr.sin_port = htons(THE_NTP_PORT);

        char ntpServer[42];
        strcpy(ntpServer, ntpServerName.toLatin1().data());
        if (ntpServer == NULL) {
           // qDebug("ERROR: cannot get server name\n");
            goto exit_function;
        }
        //qDebug("ntp server: %s { ", ntpServerName);
        {
            struct addrinfo hints;
            struct addrinfo *addresses, *address;
            int resolv_error;

            memset(&hints, 0, sizeof(struct addrinfo));
            hints.ai_family = AF_INET; // AF_UNSPEC
            hints.ai_socktype = SOCK_DGRAM;
            hints.ai_flags = 0;
            hints.ai_protocol = 0;
            /// La getaddrinfo è una chiamata bloccante.
            /// Il timeout di 40 secondi è dovuto al resolver e nel resolv.conf è possibile impostare uno.
            /// Il default è "options timeout:5 attempts:2"
            /// Il tempo per effettuare una richiesta è di 4 secondi:
            /// 4 * timeout * attempts = 40 secondi
            resolv_error = getaddrinfo(ntpServer, "ntp", &hints, &addresses);
            if (resolv_error != 0 || addresses == NULL) {
              //  qDebug("cannot resolve ntp server name (%s)\n", gai_strerror(resolv_error));
                goto exit_function;
            }
            for (address = addresses; address != NULL; address = address->ai_next) {
                char buf[INET6_ADDRSTRLEN];

                if (inet_ntop(address->ai_family, &((struct sockaddr_in *)address->ai_addr)->sin_addr, buf, INET6_ADDRSTRLEN)) {
                //   qDebug(" %s", buf);
                }
            }
            address = addresses; // the first
            memcpy(&ntpServerAddr.sin_addr.s_addr, &((struct sockaddr_in *)address->ai_addr)->sin_addr, address->ai_addrlen);
            ntpServerAddr.sin_family = address->ai_family;

            freeaddrinfo(addresses);
        }
       // qDebug("}\n");
        ntpServerAddr.sin_port = htons(THE_NTP_PORT);

        /*
         * aggancio del socket in ricezione
         */
        if (bind(theUdpSocket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
          //  qDebug("ERROR: cannot receive on %d/udp port\n", THE_NTP_PORT);
            goto exit_function;
        }

        /*
         * spedizione della richiesta
         */
        {
            struct NtpPacketHeader buffer;

            swapPacket(request, buffer);
            sentBytes = sendto(theUdpSocket, &buffer, packetSize,
                               0, (struct sockaddr *)&ntpServerAddr, sizeof(struct sockaddr_in));
            if (sentBytes != packetSize) {
                qDebug("ERROR: cannot send request(error = %d)\n", sentBytes);
                goto exit_function;
            }
            //qDebug("...");
        }
    }

    /*
     * attesa risposta, con timeout
     */
    {
        fd_set recv_set;
        struct timeval timeout;
        int selectStatus;

        FD_ZERO(&recv_set);
        FD_SET(theUdpSocket, &recv_set);
        timeout.tv_sec = (long) ntpTimeout;
        timeout.tv_usec = 0;
        selectStatus = select(theUdpSocket + 1, &recv_set, NULL, NULL, &timeout);
       // qDebug("\n");
        if (selectStatus == 0) {
            qDebug("ERROR: cannot receive reply (timeout %d s)\n", ntpTimeout);
            goto exit_function;
        } else if (selectStatus < 0) {
            qDebug("ERROR: cannot receive reply (error = %d)\n", selectStatus);
            goto exit_function;
        }
    }

    /*
     * lettura risposta
     */
    {
        struct NtpPacketHeader buffer;
        int receivedBytes;
        int64_t DestinationTimestamp;
        int64_t TripTimestamp;
        int64_t TargetTimestamp;

        receivedBytes = recv(theUdpSocket, &buffer, packetSize, 0);
        DestinationTimestamp = getTimestamp(); // <---= asap
        swapPacket(buffer, reply);

        if (receivedBytes < (packetSize - 16)) {
          //  qDebug("ERROR: received wrong reply (%d < %d)\n", receivedBytes, (packetSize - 16));
            goto exit_function;
        }
        if (reply.VN != request.VN
         || reply.Mode != 4
         || reply.Stratum == 0 || reply.Stratum > 15) {
           // qDebug("ERROR: received wrong reply (vn = %d, mode = %d, stratum = %d)\n",
           //         reply.VN, reply.Mode, reply.Stratum);
            goto exit_function;
        }
        if (reply.OriginateTimestamp != request.TransmitTimestamp) {
           // qDebug("ERROR: received wrong reply (0x%016llx != 0x%016llx)\n",
           //         reply.OriginateTimestamp, request.TransmitTimestamp);
            goto exit_function;
        }

        /*
         * interpretazione risposta
         */
        TripTimestamp = (DestinationTimestamp - reply.OriginateTimestamp) / 2;
        if (TripTimestamp > (THE_MAX_TRIPTIME_s << 32)) {
          //  qDebug("ERROR: trip time too high (0x%016llx)\n", TripTimestamp);
            goto exit_function;
        }
        //qDebug("server time =                0x%016llx\n", reply.TransmitTimestamp);
       // qDebug("trip time   =                0x%016llx\n", TripTimestamp);

        /*
         * cambio data e ora
         */
        TargetTimestamp = reply.TransmitTimestamp + TripTimestamp;
        //qDebug("target time =                0x%016llx\n", TargetTimestamp);
        mutexNTP.lock();
        {
            setTimestamp(TargetTimestamp);
            fRes = true;
            /*
             * salvataggio in RTC
             */
            system("hwclock --systohc");
        }
        mutexNTP.unlock();
    }

exit_function:
    if (theUdpSocket != -1) {
        shutdown(theUdpSocket, SHUT_RDWR);
        close(theUdpSocket);
        theUdpSocket = -1;
    }
    return fRes;
}

QDateTime NtpClient::getTimeBefore()
{
    return timeBeforeChange;
}

QMutex* NtpClient::getNTPMutex()
{
    return &mutexNTP;
}

void NtpClient::run()
{
    while (1) {
        int ntpTimeWait = ntpPeriod == 0 ? 1 : ntpPeriod;

        if (ntpPeriodms > 0 && ntpSyncTimer.elapsed() > ntpPeriodms) {
            //check for NTP autosync
            timeChanged = true;
            doSync = true;
            ntpSem.release(1);
        }        
        if (ntpSem.tryAcquire(1, ntpTimeWait)) {
            // resource grabbed, do NTP things
            timeBeforeChange = QDateTime::currentDateTime();
            doSyncOrChange();
        } else {
            // cant grab resource;
        }
    }
}
