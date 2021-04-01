/**
 * @file
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
#include <pthread.h>

#include "common.h"

struct HmiClient;

class io_layer_comm : public QThread
{
    Q_OBJECT
public:
    io_layer_comm(pthread_mutex_t * send_mutex, pthread_mutex_t * recv_mutex); /* extern sem_t theWritingSem; */
    ~io_layer_comm();
public:
    bool finalize(void);
    virtual void run();
private:
    pthread_mutex_t * the_send_mutex;
    pthread_mutex_t * the_recv_mutex;
    HmiClient *hmiClient;
};

#endif // IO_LAYER_COMM_H

