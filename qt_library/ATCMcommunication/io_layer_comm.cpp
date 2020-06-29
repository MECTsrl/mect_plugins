/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI-FCM communication class
 */
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "app_logprint.h"
#include "common.h"
#include "io_layer_comm.h"
#include "cross_table_utility.h"

#define TIMEOUT_MS 100
#define RETRY_NB 3

pthread_cond_t theWritingCondvar;
pthread_mutex_t theWritingMutex;

static pthread_t theThread;
static pthread_cond_t condvar;
static pthread_mutex_t mutex;
static void *automation_thread(void *arg);

void *automation_thread(void *arg)
{
    pthread_mutex_lock(&mutex);
    {
#ifdef VERBOSE_DEBUG
        struct timespec now;
        long long last_time_ms;
        long long actual_time_ms;
        long long elapsed_ms;
#endif
        pthread_cond_wait(&condvar, &mutex);
#ifdef VERBOSE_DEBUG
        clock_gettime(CLOCK_MONOTONIC, &now);
        last_time_ms = now.tv_sec * 1000LL + now.tv_nsec / 1000000LL;
#endif
        setup();
        do
        {
            pthread_cond_wait(&condvar, &mutex);
#ifdef VERBOSE_DEBUG
            clock_gettime(CLOCK_MONOTONIC, &now);
            actual_time_ms = now.tv_sec * 1000LL + now.tv_nsec / 1000000LL;
            elapsed_ms = actual_time_ms - last_time_ms;
            last_time_ms = actual_time_ms;
            if (elapsed_ms < 100 || elapsed_ms > 200) {
                fprintf(stderr, "------------- calling loop() after %lld ms\n", elapsed_ms);
            }
#endif
            loop();
        }
        while (1);
    }
    pthread_mutex_unlock(&mutex);
    return arg;
}

/**
 * @brief create the communication class
 */
io_layer_comm::io_layer_comm( pthread_mutex_t * send_mutex, pthread_mutex_t * recv_mutex)
{
    the_send_mutex = send_mutex;
    the_recv_mutex = recv_mutex;

    iServerSocketData = -1;
    iClientSocketData = -1;

    iServerSocketSyncro = -1;
    iClientSocketSyncro = -1;

    _setStatusIO = DONE;
    _getStatusIO = DONE;
}

/**
 * @brief destroy the communication class
 */
io_layer_comm::~io_layer_comm()
{
    finalize();
}

void io_layer_comm::run()
{
    if (iServerSocketData <= 0 || iClientSocketData <= 0)
    {
        LOG_PRINT(error_e, "ioLayer is not connected: server %d client %d\n", iServerSocketData, iClientSocketData);
        return;
    }

    if (iServerSocketSyncro <= 0 || iClientSocketSyncro <= 0)
    {
        LOG_PRINT(error_e, "ioLayer is not connected: server %d client %d\n", iServerSocketSyncro, iClientSocketSyncro);
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
                // send
                notifySetData();
                notifySetSyncro();

                // recv
                notifyGetData();
                notifyGetSyncro();

                // update sync queue
                compactSyncWrites();

                // update local variables
                update_all(); // readFromDb
            }
            pthread_mutex_unlock(the_recv_mutex);
        }
        pthread_mutex_unlock(the_send_mutex);

        // another loop for the BUSY cases
        writeVarQueuedByCtIndex();

        // call the setup() and loop()
        if (recompute_abstime) {
            pthread_cond_signal(&condvar);
        }
    }
}

bool io_layer_comm::initializeData(const char * RemoteAddress, const int iUdpRxPort, const int iUdpTxPort, void * ioAreaI, size_t ioAreaSizeI, void * ioAreaO, size_t ioAreaSizeO)
{
    if ((ioAreaI == NULL || ioAreaSizeI == 0) && (ioAreaO == NULL || ioAreaSizeO == 0))
    {
        LOG_PRINT(error_e, "No data to synchonize ioAreaI %p ioAreaSizeI %d, ioAreaO %p ioAreaSizeO %d.\n", ioAreaI, ioAreaSizeI, ioAreaO, ioAreaSizeO);
        return false;
    }

    if (ioAreaI == NULL || ioAreaSizeI == 0)
    {
        ReceiveDataData = NULL;
        ReceiveSizeData = 0;
    }
    else
    {
        ReceiveDataData = ioAreaI;
        ReceiveSizeData = ioAreaSizeI;
    }

    if (ioAreaO == NULL || ioAreaSizeO == 0)
    {
        SendDataData = NULL;
        SendSizeData = 0;
    }
    else
    {
        SendDataData = ioAreaO;
        SendSizeData = ioAreaSizeO;
    }
    LOG_PRINT(verbose_e, "Address %s:%d, AreaI %p SizeI %d, AreaO %p SizeO %d.\n", RemoteAddress, iUdpRxPort, ioAreaI, ioAreaSizeI, ioAreaO, ioAreaSizeO);

    /* Create the server (receive) socket */
    if (iServerSocketData < 0) {
        struct  sockaddr_in ServerAddress; /* structure to hold server's address  */

        iServerSocketData = socket(AF_INET, SOCK_DGRAM | O_NONBLOCK, 0);
        if (iServerSocketData < 0) {
            LOG_PRINT(error_e, "cannot open ServerSocket [%d : %s]\n", iServerSocketData, strerror(errno));
            return false;
        }

        /* Bind a local address to the server socket */
        /* clear sockaddr structure */
        memset((char *)&ServerAddress,0,sizeof(ServerAddress));
        /* set family to Internet */
        ServerAddress.sin_family = AF_INET;
        /* set the local IP address */
        ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        /* set the port number */
        ServerAddress.sin_port = htons((u_short)iUdpRxPort);

        if (bind(iServerSocketData, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress)) < 0) {
            LOG_PRINT(error_e, "cannot bind Server socket: [%s]\n", strerror(errno));
            return false;
        }
        LOG_PRINT(verbose_e, "Waiting for data input on UDP from port %d\n", iUdpRxPort);
    }
    /* Create the client (transmitter) UDP socket */
    if (iClientSocketData < 0) {
        struct hostent *h;
        /* get server IP address (no check if input is IP address or DNS name */
        h = gethostbyname(RemoteAddress);
        if(h == NULL) {
            LOG_PRINT(error_e, "unknown host '%s'\n", RemoteAddress);
            return false;
        }
        /* Construct the server sockaddr_in structure */
        /* clear sockaddr structure   */
        memset(&DestinationAddressData, 0, sizeof(DestinationAddressData));
        /* set family to Internet     */
        DestinationAddressData.sin_family = h->h_addrtype;
        /* set the local IP address   */
        memcpy((char *) &DestinationAddressData.sin_addr.s_addr,
               h->h_addr_list[0], h->h_length);
        /* server port */
        DestinationAddressData.sin_port = htons(iUdpTxPort);

        /* Create the client (transmitter) UDP socket */
        iClientSocketData = socket(AF_INET, SOCK_DGRAM, 0);
        if (iClientSocketData < 0) {
            LOG_PRINT(error_e, "cannot open ClientSocket [%d : %s]\n", iClientSocketData, strerror(errno));
            return false;
        }

        LOG_PRINT(verbose_e, "client socket creation done\n");
    }
    return true;
}

bool io_layer_comm::initializeSyncro(const char * RemoteAddress, const int iUdpRxPort, const int iUdpTxPort, void * ioAreaI, size_t ioAreaSizeI, void * ioAreaO, size_t ioAreaSizeO)
{
    if ((ioAreaI == NULL || ioAreaSizeI == 0) && (ioAreaO == NULL || ioAreaSizeO == 0))
    {
        LOG_PRINT(error_e, "No data to synchonize ioAreaI %p ioAreaSizeI %d, ioAreaO %p ioAreaSizeO %d.\n", ioAreaI, ioAreaSizeI, ioAreaO, ioAreaSizeO);
        return false;
    }

    if (ioAreaI == NULL || ioAreaSizeI == 0)
    {
        ReceiveDataSyncro = NULL;
        ReceiveSizeSyncro = 0;
    }
    else
    {
        ReceiveDataSyncro = ioAreaI;
        ReceiveSizeSyncro = ioAreaSizeI;
    }

    if (ioAreaO == NULL || ioAreaSizeO == 0)
    {
        SendDataSyncro = NULL;
        SendSizeSyncro = 0;
    }
    else
    {
        SendDataSyncro = ioAreaO;
        SendSizeSyncro = ioAreaSizeO;
    }
    LOG_PRINT(verbose_e, "Address %s:%d, AreaI %p SizeI %d, AreaO %p SizeO %d.\n", RemoteAddress, iUdpRxPort, ioAreaI, ioAreaSizeI, ioAreaO, ioAreaSizeO);

    /* Create the server (receive) socket */
    if (iServerSocketSyncro < 0) {
        struct  sockaddr_in ServerAddress; /* structure to hold server's address  */

        iServerSocketSyncro = socket(AF_INET, SOCK_DGRAM | O_NONBLOCK, 0);
        if (iServerSocketSyncro < 0) {
            LOG_PRINT(error_e, "cannot open ServerSocket [%d : %s]\n", iServerSocketSyncro, strerror(errno));
            return false;
        }

        /* Bind a local address to the server socket */
        /* clear sockaddr structure */
        memset((char *)&ServerAddress,0,sizeof(ServerAddress));
        /* set family to Internet */
        ServerAddress.sin_family = AF_INET;
        /* set the local IP address */
        ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
        /* set the port number */
        ServerAddress.sin_port = htons((u_short)iUdpRxPort);

        if (bind(iServerSocketSyncro, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress)) < 0) {
            LOG_PRINT(error_e, "cannot bind Server socket: [%s]\n", strerror(errno));
            return false;
        }
        LOG_PRINT(verbose_e, "Waiting for data input on UDP from port %d\n", iUdpRxPort);
    }
    /* Create the client (transmitter) UDP socket */
    if (iClientSocketSyncro < 0) {
        struct hostent *h;
        /* get server IP address (no check if input is IP address or DNS name */
        h = gethostbyname(RemoteAddress);
        if(h == NULL) {
            LOG_PRINT(error_e, "unknown host '%s'\n", RemoteAddress);
            return false;
        }
        /* Construct the server sockaddr_in structure */
        /* clear sockaddr structure   */
        memset(&DestinationAddressSyncro, 0, sizeof(DestinationAddressSyncro));
        /* set family to Internet     */
        DestinationAddressSyncro.sin_family = h->h_addrtype;
        /* set the local IP address   */
        memcpy((char *) &DestinationAddressSyncro.sin_addr.s_addr,
               h->h_addr_list[0], h->h_length);
        /* server port */
        DestinationAddressSyncro.sin_port = htons(iUdpTxPort);

        /* Create the client (transmitter) UDP socket */
        iClientSocketSyncro = socket(AF_INET, SOCK_DGRAM, 0);
        if (iClientSocketSyncro < 0) {
            LOG_PRINT(error_e, "cannot open ClientSocket [%d : %s]\n", iClientSocketSyncro, strerror(errno));
            return false;
        }

        LOG_PRINT(verbose_e, "client socket creation done\n");
    }
    return true;
}

bool io_layer_comm::finalize(void)
{
    LOG_PRINT(verbose_e, "ioLayer Finalyze\n");
    close(iServerSocketData);
    close(iClientSocketData);

    iServerSocketData = -1;
    iClientSocketData = -1;

    close(iServerSocketSyncro);
    close(iClientSocketSyncro);

    iServerSocketSyncro = -1;
    iClientSocketSyncro = -1;

    return true;
}

bool io_layer_comm::notifyGetData(void)
{
    _getStatusIO = BUSY;

    // wait on server socket, only until timeout
    fd_set recv_set;
    struct timeval tv;
    FD_ZERO(&recv_set);
    FD_SET(iServerSocketData, &recv_set);
    tv.tv_sec = TIMEOUT_MS / 1000;
    tv.tv_usec = (TIMEOUT_MS % 1000) * 1000;
    if (select(iServerSocketData + 1, &recv_set, NULL, NULL, &tv) > 0) {
        size_t received;
        int iByteNum;
#if 1
#define THE_DATA_UDP_SIZE 30720
        iByteNum = recv(iServerSocketData, ReceiveDataData, THE_DATA_UDP_SIZE, 0); //ReceiveSizeData
        if (iByteNum == THE_DATA_UDP_SIZE) {
            _getStatusIO = DONE;
        } else {
            LOG_PRINT(error_e, "recv fail: %d/%u [%s]\n", iByteNum, THE_DATA_UDP_SIZE, strerror(errno))
           _getStatusIO = ERROR;
        }
#else
        do {
            received = 0;
            do {
                iByteNum = recv(iServerSocketData, (char *)ReceiveDataData + received, ReceiveSizeData - received, 0);
                if (iByteNum < 0) {
                    if (errno == EAGAIN) {
                        LOG_PRINT(verbose_e, "No data received %d.\n", iByteNum);
                        _getStatusIO = DONE;
                    } else {
                        LOG_PRINT(error_e, "recv fail: [%s]\n", strerror(errno));
                        _getStatusIO = ERROR;
                    }
                } else {
                    received += iByteNum;
                }
            } while(iByteNum > 0 && received != ReceiveSizeData);
        } while (iByteNum > 0);
        if (_getStatusIO == BUSY) {
            _getStatusIO = DONE;
        }
#endif
    } else {
        LOG_PRINT(warning_e, "select fail\n");
        fprintf(stderr, ".");
    }
    return  (_getStatusIO == DONE);
}

bool io_layer_comm::notifySetData(void)
{
    size_t iByteNum;
    _setStatusIO = BUSY;
    size_t received = 0;
#if 1
    iByteNum = sendto(iClientSocketData, SendDataData, THE_DATA_UDP_SIZE, 0, // SendSizeData
                      (struct sockaddr *) &DestinationAddressData,
                      sizeof(DestinationAddressData));
    if (iByteNum == THE_DATA_UDP_SIZE) {
        _setStatusIO = DONE;
    } else {
        LOG_PRINT(error_e, "sendto fail: %d/%u [%s]\n", iByteNum, THE_DATA_UDP_SIZE, strerror(errno))
       _setStatusIO = ERROR;
    }
#else
    do {
        iByteNum = sendto(iClientSocketData, (char *)SendDataData + received, SendSizeData - received, 0,
                          (struct sockaddr *) &DestinationAddressData,
                          sizeof(DestinationAddressData));
        if (iByteNum > 0) {
            received += iByteNum;
        } else {
            LOG_PRINT(error_e, "send fail: [%s]\n", strerror(errno));
            _setStatusIO = ERROR;
        }
    } while (received != SendSizeData);
    if (_setStatusIO == BUSY) {
        _setStatusIO = DONE;
    }
#endif
    return (_setStatusIO == DONE);
}

bool io_layer_comm::notifyGetSyncro(void)
{
    _getStatusIO = BUSY;

    // wait on server socket, only until timeout
    fd_set recv_set;
    struct timeval tv;
    FD_ZERO(&recv_set);
    FD_SET(iServerSocketSyncro, &recv_set);
    tv.tv_sec = TIMEOUT_MS / 1000;
    tv.tv_usec = (TIMEOUT_MS % 1000) * 1000;
    if (select(iServerSocketSyncro + 1, &recv_set, NULL, NULL, &tv) > 0) {
        size_t received;
        int iByteNum;
#if 1
#define THE_SYNC_UDP_SIZE   11462
        iByteNum = recv(iServerSocketSyncro, ReceiveDataSyncro, THE_SYNC_UDP_SIZE, 0); // ReceiveSizeSyncro
        if (iByteNum == THE_SYNC_UDP_SIZE) {
            _getStatusIO = DONE;
        } else {
            LOG_PRINT(error_e, "recv fail: %d/%u [%s]\n", iByteNum, THE_SYNC_UDP_SIZE, strerror(errno))
           _getStatusIO = ERROR;
        }
#else
        do {
            received = 0;
            do {
                iByteNum = recv(iServerSocketSyncro, (char *)ReceiveDataSyncro + received, ReceiveSizeSyncro - received, 0);
                if (iByteNum < 0) {
                    if (errno == EAGAIN) {
                        LOG_PRINT(verbose_e, "No data received %d.\n", iByteNum);
                        _getStatusIO = DONE;
                    } else {
                        LOG_PRINT(error_e, "recv fail: [%s]\n", strerror(errno));
                        _getStatusIO = ERROR;
                    }
                } else {
                    received += iByteNum;
                }
            } while(iByteNum > 0 && received != ReceiveSizeSyncro);
        } while (iByteNum > 0);
        if (_getStatusIO == BUSY) {
            _getStatusIO = DONE;
        }
#endif
    } else {
        LOG_PRINT(warning_e, "select fail\n");
        fprintf(stderr, ".");
    }
    return  (_getStatusIO == DONE);
}

bool io_layer_comm::notifySetSyncro(void)
{
    size_t iByteNum;
    _setStatusIO = BUSY;
    size_t received = 0;
#if 1
    iByteNum = sendto(iClientSocketSyncro, SendDataSyncro, THE_SYNC_UDP_SIZE, 0, // SendSizeSyncro
                      (struct sockaddr *) &DestinationAddressSyncro,
                      sizeof(DestinationAddressSyncro));
    if (iByteNum == THE_SYNC_UDP_SIZE) {
        _setStatusIO = DONE;
    } else {
        LOG_PRINT(error_e, "sendto fail: %d/%u [%s]\n", iByteNum, THE_SYNC_UDP_SIZE, strerror(errno))
       _setStatusIO = ERROR;
    }
#else
    do {
        iByteNum = sendto(iClientSocketSyncro, (char *)SendDataSyncro + received, SendSizeSyncro - received, 0,
                          (struct sockaddr *) &DestinationAddressSyncro,
                          sizeof(DestinationAddressSyncro));
        if (iByteNum > 0) {
            received += iByteNum;
        } else {
            LOG_PRINT(error_e, "send fail: [%s]\n", strerror(errno));
            _setStatusIO = ERROR;
        }
    } while (received != SendSizeSyncro);
    if (_setStatusIO == BUSY) {
        _setStatusIO = DONE;
    }
#endif
    return (_setStatusIO == DONE);
}

int io_layer_comm::getStatusIO()
{
    return _getStatusIO;
}

int io_layer_comm::setStatusIO()
{
    return _setStatusIO;
}
