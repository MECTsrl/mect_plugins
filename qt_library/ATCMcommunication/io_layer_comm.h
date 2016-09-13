/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI-PLC communication thread via ioLayer.
 */
#ifndef IO_LAYER_COMM_H
#define IO_LAYER_COMM_H

#include <QThread>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> /* for exit() */
#include <string.h> /* memset() */
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h> /* select() */ 
#include <sys/types.h>

#include "common.h"

extern int update_all(void);
extern void setup(void);
extern void loop(void);

/**
 * @brief This class implement an interface with a 4C IO Layer
 */
class io_layer_comm : public QThread
{
    Q_OBJECT
public:
    /**
          * @brief constructor of the ioLayer class for the comunication with PLC
          *
          * @param char * label : name of the ioLayer
          */
    io_layer_comm(pthread_mutex_t * send_mutex, pthread_mutex_t * recv_mutex); /* extern sem_t theWritingSem; */
    ~io_layer_comm();
public:
    /** 
         * @brief initialize the connection to the 4C IO Layer
         *
         * @param const char * RemoteAddress : remote address where is the PLC
         * @param const int iUdpRxPort       : Receive port
         * @param const int iUdpTxPort       : Send port
         * @param void * ioAreaI             : Pointer to the input area (NULL if it is not used)
         * @param size_t ioAreaSizeI         : Size of the input area (0 if it is not used)
         * @param void * ioAreaO             : Pointer to the output area (NULL if it is not used)
         * @param size_t ioAreaSizeO         : Size of the output area (0 if it is not used)
         */
    bool initializeData(const char * RemoteAddress, const int iUdpRxPort, const int iUdpTxPort, void * ioAreaI, size_t ioAreaSizeI, void * ioAreaO, size_t ioAreaSizeO);
    bool initializeSyncro(const char * RemoteAddress, const int iUdpRxPort, const int iUdpTxPort, void * ioAreaI, size_t ioAreaSizeI, void * ioAreaO, size_t ioAreaSizeO);
    /**
         * @brief finalize the connection to the 4C IO Layer
         */
    bool finalize(void);
    virtual void run();
    int getStatusIO();
    int setStatusIO();
private:
    bool notifyGetData(void);
    bool notifySetData(void);
    bool notifyGetSyncro(void);
    bool notifySetSyncro(void);
private:
    int iServerSocketData;     /* server (receive) socket */
    int iClientSocketData;     /* client (transmit) socket */
    struct  sockaddr_in DestinationAddressData;
    void * SendDataData;
    size_t SendSizeData;
    void * ReceiveDataData;
    size_t ReceiveSizeData;

    int iServerSocketSyncro;     /* server (receive) socket */
    int iClientSocketSyncro;     /* client (transmit) socket */
    struct  sockaddr_in DestinationAddressSyncro;
    void * SendDataSyncro;
    size_t SendSizeSyncro;
    void * ReceiveDataSyncro;
    size_t ReceiveSizeSyncro;

    int _getStatusIO;
    int _setStatusIO;
    pthread_mutex_t * the_send_mutex;
    pthread_mutex_t * the_recv_mutex;
};

#endif // IO_LAYER_COMM_H

