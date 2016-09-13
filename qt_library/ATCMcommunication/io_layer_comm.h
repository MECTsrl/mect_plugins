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
#include <semaphore.h>
#include "protocol.h"

#include "common.h"

extern int update_all(void);
extern void setup(void);
extern void loop(void);

/**
 * @brief This class implements an interface to a 4C IO Layer
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
    io_layer_comm();
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
    bool initializeData(const char *RemoteAddress, uint16_t portRx, uint16_t portTx);
    /**
     * @brief finalize the connection to the 4C IO Layer
     */
    bool finalize(void);
    virtual void run();
    uint8_t getStatusVar(int CtIndex, char *msg = NULL);
    int valFromIndex(int ctIndex, char *value);
    int readUdpReply(int ctIndex, void * value);
    int sendUdpWriteCmd(int ctIndex, void *value);
    int sendUdpWriteCmd(const char *varname, void *value);
    void hValRequest();
    int writeRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[]);
    void resetHvarUsage();
    bool incHvarUsage(int ctIndex);
    bool incHvarUsage(char * varname);
    bool decHvarUsage(int ctIndex);
    bool decHvarUsage(char * varname);

private:
    void receiveData(void);
    void sendData(void);

private:

    // udp values
    int iSocketDataTx;        /* send socket */
    int iSocketDataRx;        /* receive socket */
    struct  sockaddr_in DestinationAddressData;

    // send command
    pthread_mutex_t data_send_mutex;
    sem_t data_send_sem;
    uint32_t udp_cmd_seq;
    struct udp_cmd udp_cmd;

    // receive reply
    pthread_mutex_t data_recv_mutex;
    struct udp_reply udp_reply;
    u_int32_t value[1 + DimCrossTable];
    u_int8_t status[1 + DimCrossTable];

    // automation thread
    pthread_t theThread;
    pthread_cond_t loop_condvar; // setup+loop
    pthread_cond_t wrote_condvar; // writeRecipe + sendUdpWriteCmd
};

#endif // IO_LAYER_COMM_H
