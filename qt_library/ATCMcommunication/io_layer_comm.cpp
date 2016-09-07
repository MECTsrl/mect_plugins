/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI-FCM communication class
 */
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#include "app_logprint.h"
#include "common.h"
#include "io_layer_comm.h"
#include "cross_table_utility.h"
#include "global_var.h"

#define TIMEOUT_MS 100
#define RETRY_NB 3

static void *automation_thread(void *arg);

void *automation_thread(void *arg)
{
    pthread_mutex_t cv_mutex;
    pthread_cond_t *pcondvar = (pthread_cond_t *)arg;

    pthread_mutex_init(&cv_mutex, NULL);
    pthread_mutex_lock(&cv_mutex);
    {
        pthread_cond_wait(pcondvar, &cv_mutex);
        setup();
        while (1) {
            pthread_cond_wait(pcondvar, &cv_mutex);
            loop();
        }
    }
    pthread_mutex_unlock(&cv_mutex);
    return arg;
}

/**
 * @brief create the communication class
 */
io_layer_comm::io_layer_comm()
{
    iSocketDataTx = -1;
    iSocketDataRx = -1;
    pthread_mutex_init(&data_send_mutex, NULL);
    pthread_mutex_init(&data_recv_mutex, NULL);
    pthread_cond_init(&loop_condvar, NULL);
    pthread_cond_init(&wrote_condvar, NULL);
    udp_cmd_seq = 0;
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
    if (iSocketDataTx <= 0)
    {
        LOG_PRINT(error_e, "TX socket is not connected (%d)\n", iSocketDataTx);

        return;
    }

    if (iSocketDataRx <= 0)
    {
        LOG_PRINT(error_e, "RX socket is not connected (%d)\n", iSocketDataRx);

        return;
    }

    /* set the absolute time */
    int recompute_abstime = true;
    struct timespec abstime;
    sem_init(&data_send_sem, 0, 0);

    pthread_create(&theThread, NULL, automation_thread, &loop_condvar);

    while (1)
    {
        if (recompute_abstime) {
            recompute_abstime = false;
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += IOLAYER_PERIOD_ms / 1000;
            abstime.tv_nsec += (IOLAYER_PERIOD_ms % 1000) * 1000 * 1000; // ms -> ns
            if (abstime.tv_nsec >= (1000*1000*1000)) {
                abstime.tv_sec += abstime.tv_nsec / (1000*1000*1000);
                abstime.tv_nsec = abstime.tv_nsec % (1000*1000*1000);
            }
        }
        int rc = sem_timedwait(&data_send_sem, &abstime);
        if (rc  == -1 && errno == EINTR){
            recompute_abstime = true;
            continue;
        }
        else if (rc == 0) {
            recompute_abstime = true;
        }
        else if (errno == ETIMEDOUT) {
            recompute_abstime = true;
        }

        /* chiamata sia ogni periodo sia ad ogni scrittura in coda */
        hValRequest();
        sendData(); // data_send_mutex
        receiveData(); // data_recv_mutex

        update_all();

        pthread_cond_signal(&loop_condvar);
        pthread_cond_signal(&wrote_condvar);
    }
    LOG_PRINT(verbose_e, "Finish ioLayer syncronization\n");
}

bool io_layer_comm::initializeData(const char * RemoteAddress, uint16_t portRx, uint16_t portTx)
{
    /* Create the server (receive) socket */
    if (iSocketDataRx < 0) {
        struct  sockaddr_in ServerAddress; /* structure to hold server's address  */

        iSocketDataRx = socket(AF_INET, SOCK_DGRAM | O_NONBLOCK, 0);
        if (iSocketDataRx < 0) {
            LOG_PRINT(error_e, "cannot open RX socket [%d : %s]\n", iSocketDataRx, strerror(errno));
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
        ServerAddress.sin_port = htons(portRx);

        int retval = bind(iSocketDataRx, (struct sockaddr *)&ServerAddress, sizeof(ServerAddress));
        if (retval < 0) {
            LOG_PRINT(error_e, "cannot bind RX socket: [%d : %s]\n", retval, strerror(errno));
            return false;
        }
        LOG_PRINT(verbose_e, "Waiting for data input on UDP from port %d\n", portRx);
    }
    /* Create the client (transmitter) UDP socket */
    if (iSocketDataTx < 0) {
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
        DestinationAddressData.sin_port = htons(portTx);

        /* Create the client (transmitter) UDP socket */
        iSocketDataTx = socket(AF_INET, SOCK_DGRAM, 0);
        if (iSocketDataTx < 0) {
            LOG_PRINT(error_e, "cannot open TX socket [%d : %s]\n", iSocketDataTx, strerror(errno));
            return false;
        }

        LOG_PRINT(verbose_e, "client socket creation done\n");
    }
    return true;
}

bool io_layer_comm::finalize(void)
{
    LOG_PRINT(verbose_e, "ioLayer Finalyze\n");

    if (iSocketDataTx >= 0) {
        close(iSocketDataTx);
        iSocketDataTx = -1;
    }
    if (iSocketDataRx >= 0) {
        close(iSocketDataRx);
        iSocketDataRx = -1;
    }

    return true;
}

void io_layer_comm::receiveData(void)
{
    // wait on server socket, only until timeout
    fd_set recv_set;
    struct timeval tv;
    bool retval = true;

    FD_ZERO(&recv_set);
    FD_SET(iSocketDataRx, &recv_set);
    tv.tv_sec = TIMEOUT_MS / 1000;
    tv.tv_usec = (TIMEOUT_MS % 1000) * 1000;

    if (select(iSocketDataRx + 1, &recv_set, NULL, NULL, &tv) > 0) {
        int iByteNum = 0;
        do {
            size_t received = 0;
            do {
                iByteNum = recv(iSocketDataRx, (char *)&udp_reply + received, sizeof(udp_reply) - received, 0);
                if (iByteNum < 0) {
                    if (errno == EAGAIN) {
                        iByteNum = 0;
                    }
                    else {
                        LOG_PRINT(error_e, "RX error: [ %d : %s ]\n", iByteNum, strerror(errno));
                        retval = false;
                        break;
                    }
                }
                received += iByteNum;
            } while ((iByteNum > 0) && (received != sizeof(udp_reply)));
        } while (iByteNum > 0);

        if (retval) {
            pthread_mutex_lock(&data_recv_mutex);
            {
                memcpy(&value[1], udp_reply.value, DimCrossTable*sizeof(u_int32_t));
                memcpy(&status[1], udp_reply.status, DimCrossTable*sizeof(u_int8_t));
            }
            pthread_mutex_unlock(&data_recv_mutex);
        }
    }
}

void io_layer_comm::sendData(void)
{
    pthread_mutex_lock(&data_send_mutex);
    {
        udp_cmd.seq = ++udp_cmd_seq;
        udp_cmd.magic = MAGIC_CMD;
        size_t sent = 0;
        do {
            int iByteNum = sendto(iSocketDataTx, (char *)&udp_cmd + sent, sizeof(udp_cmd) - sent, 0,
                              (struct sockaddr *)&DestinationAddressData,
                              sizeof(DestinationAddressData));
            if (iByteNum > 0) {
                sent += iByteNum;
            }
            else if (iByteNum == 0) {
                break;
            }
            else {
                LOG_PRINT(error_e, "RX error: [ %d : %s ]\n", iByteNum, strerror(errno));
                break;
            }
        } while (sent <= sizeof(udp_cmd));
        // reset write queue
        udp_cmd.write_num = 0;
    }
    pthread_mutex_unlock(&data_send_mutex);
}

/** @brief Write the value 'value' into the internal DB at psition 'ctIndex'
 * @param int ctIndex : Cross table index of the variable to write
 * @param void * value: value to write
 * @return 1 Some error occured
 * @return 0 Write done
 */
int io_layer_comm::sendUdpWriteCmd(int ctIndex, void *value)
{
    int loops = 0;

    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid croosstable index %d\n", ctIndex);

        return -1;
    }

    pthread_mutex_lock(&data_send_mutex);
    {
        while (udp_cmd.write_num < MAX_WRITES && (loops++ < 42)) {
            pthread_cond_wait(&wrote_condvar, &data_send_mutex);
        }
        if (loops < 42) {
            udp_cmd.write_addr[udp_cmd.write_num] = (uint16_t)ctIndex;
            udp_cmd.write_value[udp_cmd.write_num] = (uint32_t)value;
            ++udp_cmd.write_num;
            sem_post(&data_send_sem);
        }
    }
    pthread_mutex_unlock(&data_send_mutex);

    LOG_PRINT(verbose_e, "0x%p (%d)\n", value, ctIndex);

    return !(loops < 42);
}

int io_layer_comm::sendUdpWriteCmd(const char *varname, void *value)
{
    int ctIndex = 0;

    if (Tag2CtIndex(varname, &ctIndex) != 0) {
        return -1;
    }

    return sendUdpWriteCmd(ctIndex, value);
}

/**
 * @brief get the actual status.
 */
uint8_t io_layer_comm::getStatusVar(int ctIndex, char *msg)
{
    uint8_t st = STATUS_OK;

    if (ctIndex < 1 || ctIndex > DimCrossTable ) {
        LOG_PRINT(error_e, "invalid croosstable index %d\n", ctIndex);
        st = STATUS_ERR;
        return st;
    }

    //pthread_mutex_lock(&data_recv_mutex);
    st = status[ctIndex];
    //pthread_mutex_unlock(&data_recv_mutex);
    if (msg != NULL) {
        if (st & STATUS_OK) {
            strcpy(msg, "");
            LOG_PRINT(verbose_e, "Variable '%s' status DONE\n: [0x%x]\n",  varNameArray[ctIndex].tag, st);
        }
        else if (st & (STATUS_ERR | STATUS_FAIL_W)) {
            strcpy(msg, VAR_COMMUNICATION);
            LOG_PRINT(error_e, "Variable '%s' status ERROR: [0x%x]\n",  varNameArray[ctIndex].tag, st);
        }
        else if (st & (STATUS_BUSY_R | STATUS_BUSY_W)) {
            strcpy(msg, VAR_PROGRESS);
            LOG_PRINT(error_e, "Variable '%s' status INPROGRESS: [0x%x]\n",  varNameArray[ctIndex].tag, st);
        }
        else {
            strcpy(msg, VAR_UNKNOWN);
            LOG_PRINT(verbose_e, "Variable '%s' status UNKNOWN: [0x%x]\n",  varNameArray[ctIndex].tag, st);
        }
    }

    return st;
}

/** @brief Read the value from the internal DB at psition 'ctIndex'
 * and print into the string 'value' in according with the format described into the cross table
 * @param int ctIndex : Cross table index of the variable to read
 * @param void * value: value read and print in the right format
 * @return 1 Some error occured
 * @return 0 Write done
 */
int io_layer_comm::valFromIndex(int ctIndex, char *value)
{
    int decimal = 0;

    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM) {
        LOG_PRINT(error_e, "invalid croosstable index %d\n", ctIndex);

        return -1;
    }

    uint8_t status = getStatusVar(ctIndex, NULL);
    if (status & STATUS_ERR) {
        return -1;
    }
    else if (!((status & STATUS_OK) || (status & STATUS_BUSY_R))) {
        return -1;
    }

    decimal = getVarDecimal(ctIndex);

    switch(CtIndex2Type(ctIndex)) {
        case uintab_e:
        case uintba_e:
        {
            unsigned short int _value;

            if (readUdpReply(ctIndex, &_value) != 0) {
                return -1;
            }

            if (decimal > 0) {
                char fmt[8] = "";

                sprintf (fmt, "%%.%df", decimal);
                sprintf(value, fmt, (float)_value / pow(10,decimal));
            }
            else {
                sprintf(value, "%u", (unsigned short)_value);
            }
        }
        break;

        case intab_e:
        case intba_e:
        {
            short int _value;

            if (readUdpReply(ctIndex, &_value) != 0) {
                return -1;
            }

            if (decimal > 0) {
                char fmt[8] = "";

                sprintf (fmt, "%%.%df", decimal);
                sprintf(value, fmt, (float)_value / pow(10,decimal));
            }
            else {
                sprintf(value, "%d", (short int)_value);
            }
        }
        break;

        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
        {
            unsigned int _value;

            if (readUdpReply(ctIndex, &_value) != 0) {
                return -1;
            }

            if (decimal > 0)
            {
                char fmt[8] = "";
                sprintf (fmt, "%%.%df", decimal);
                sprintf(value, fmt, (float)_value / pow(10,decimal));
            }
            else
            {
                sprintf(value, "%u", (unsigned int)_value);
            }
        }
            break;
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
        {
            int _value;

            if (readUdpReply(ctIndex, &_value) != 0) {
                return -1;
            }

            if (decimal > 0) {
                char fmt[8] = "";

                sprintf (fmt, "%%.%df", decimal);
                sprintf(value, fmt, (float)_value / pow(10,decimal));
            }
            else {
                sprintf(value, "%d", (int)_value);
            }
        }
        break;

        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
        {
            float _value;
            if (readUdpReply(ctIndex, &_value) != 0)
            {
                return -1;
            }
            if (decimal > 0)
            {
                char fmt[8] = "";
                sprintf (fmt, "%%.%df", decimal);
                sprintf(value, fmt, (float)_value);
            }
            else
            {
                sprintf(value, "%.0f", (float)_value);
            }
        }
        break;

        case bit_e:
        case bytebit_e:
        case wordbit_e:
        case dwordbit_e:
        {
            BYTE _value;
            if (readUdpReply(ctIndex, &_value) != 0)
            {
                return -1;
            }
            sprintf(value, "%d", (BYTE)_value);
        }
        break;

        default:
        LOG_PRINT(error_e, "unknown type %d for variable '%s'\n", CtIndex2Type(ctIndex), varNameArray[ctIndex].tag);

            return -1;

        break;
    }

    return 0;
}

/** @brief Read the value from the internal DB at psition 'ctIndex'
 * @param int ctIndex : Cross table index of the variable to read
 * @param void * value: value read
 * @return 1 Some error occured
 * @return 0 Write done
 */
int io_layer_comm::readUdpReply(int ctIndex, void * variable)
{
    int retval = 0;

    if (variable == NULL)
    {
        LOG_PRINT(error_e, "NULL value pointer, please report bug.\n");
        return -1;
    }

    if (ctIndex < 1 || ctIndex > DimCrossTable ) {
        LOG_PRINT(error_e, "invalid croosstable index %d\n", ctIndex);
        return -1;
    }

    int type = CtIndex2Type(ctIndex);
    pthread_mutex_lock(&data_recv_mutex);
    switch(type)
    {
    case uintab_e:
    case uintba_e:
        memcpy(variable, &(value[ctIndex]), sizeof(unsigned short));
        LOG_PRINT(verbose_e, "uint_e: %d\n", *((unsigned short*)variable));
        break;
    case intab_e:
    case intba_e:
        memcpy(variable, &(value[ctIndex]), sizeof(short));
        LOG_PRINT(verbose_e, "int_e: %d\n", *((short*)variable));
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        memcpy(variable, &(value[ctIndex]), sizeof(unsigned int));
        LOG_PRINT(verbose_e, "udint_e: %d\n", *((unsigned int*)variable));
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        memcpy(variable, &(value[ctIndex]), sizeof(int));
        LOG_PRINT(verbose_e, "dint_e: %d\n", *((int*)variable));
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        memcpy(variable, &(value[ctIndex]), sizeof(DWORD));
        LOG_PRINT(verbose_e, "FLOAT: %f\n", *((float*)variable));
        break;
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        *((BYTE*)variable) = value[ctIndex];
        LOG_PRINT(verbose_e, "BIT: %d\n", *((BYTE*)variable));
        break;
    default:
        LOG_PRINT(error_e, "unknown type %d for variable '%s'\n", CtIndex2Type(ctIndex), varNameArray[ctIndex].tag);
        retval = -1;
        break;
    }
    pthread_mutex_unlock(&data_recv_mutex);
    return retval;
}

void io_layer_comm::hValRequest(void)
{
    pthread_mutex_lock(&data_send_mutex);
    {
        int addr;

        for (addr = 1; addr <= DimCrossTable && (udp_cmd.read_num < MAX_READS); addr++)
        {
            /* skip not H-variables */
            if (varNameArray[addr].autoupdate == 1)
                continue;

            if (addr != varNameArray[addr].blockhead)
                continue;

            if (varNameArray[addr].blockbusy == 0)
                continue;

            udp_cmd.read_addr[udp_cmd.read_num] = (uint16_t)addr;
            ++udp_cmd.read_num;
        }
        // no sem_post(&data_send_sem);
    }
    pthread_mutex_unlock(&data_send_mutex);
}

int io_layer_comm::writeRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[])
{
    int sent = 0, loops = 0;

    if (step >= MAX_RCP_STEP)
    {
        return -1;
    }
    pthread_mutex_lock(&data_send_mutex);
    {
        while (sent < table[step].count() && (++loops < 42))
        {
            for (int i = sent; i < table[step].count() && (udp_cmd.write_num < MAX_WRITES); ++i)
            {
                udp_cmd.write_addr[udp_cmd.write_num] = indexes->at(i);
                udp_cmd.write_value[udp_cmd.write_num] = table[step].at(i);
                ++udp_cmd.write_num;
                ++sent;
            }
            sem_post(&data_send_sem);
            pthread_cond_wait(&wrote_condvar, &data_send_mutex);
        }
    }
    pthread_mutex_unlock(&data_send_mutex);

    return !(loops < 42);
}

void io_layer_comm::resetHvarUsage()
{
    for (int addr = 1; addr <= DimCrossTable; addr++)
    {
        if (varNameArray[addr].autoupdate == 0)
        {
            varNameArray[varNameArray[addr].blockhead].blockbusy = 0;
        }
    }
}

bool io_layer_comm::incHvarUsage(int ctIndex)
{
    /* invalid cross table index */
    if (ctIndex < 1 || ctIndex > DimCrossTable ) {
        LOG_PRINT(error_e, "invalid croosstable index %d\n", ctIndex);
        return false;
    }
    /* is not an H variable */
    if (varNameArray[ctIndex].autoupdate == 1)
    {
        return true;
    }
    /* increment the uage fot H variable */
    varNameArray[varNameArray[ctIndex].blockhead].blockbusy++;
    return true;
}

bool io_layer_comm::incHvarUsage(char * varname)
{
    int ctIndex = 0;

    if (Tag2CtIndex(varname, &ctIndex) != 0) {
        return -1;
    }

    return incHvarUsage(ctIndex);
}

bool io_layer_comm::decHvarUsage(int ctIndex)
{
    /* invalid cross table index */
    if (ctIndex < 1 || ctIndex > DimCrossTable ) {
        LOG_PRINT(error_e, "invalid croosstable index %d\n", ctIndex);
        return false;
    }
    /* is not an H variable */
    if (varNameArray[ctIndex].autoupdate == 1)
    {
        return true;
    }
    /* the variable is alredy inactive */
    if (varNameArray[varNameArray[ctIndex].blockhead].blockbusy <= 0)
    {
        varNameArray[varNameArray[ctIndex].blockhead].blockbusy = 0;
        return true;
    }
    /* decrement the uage fot H variable */
    varNameArray[varNameArray[ctIndex].blockhead].blockbusy--;
    return true;
}

bool io_layer_comm::decHvarUsage(char * varname)
{
    int ctIndex = 0;

    if (Tag2CtIndex(varname, &ctIndex) != 0) {
        return -1;
    }

    return decHvarUsage(ctIndex);
}
