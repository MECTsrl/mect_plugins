#include <arpa/inet.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h> /* pthread_create() */
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "app_logprint.h"
#include "app_var_list.h"
#include "common.h"
#include "cross_table_utility.h"

#define RETRY_NB 10

BYTE IODataAreaI[STATUS_BASE_BYTE + DB_SIZE_BYTE];
BYTE * pIODataAreaI = &(IODataAreaI[4]);

BYTE IODataAreaO[STATUS_BASE_BYTE + DB_SIZE_BYTE];
BYTE * pIODataAreaO = &(IODataAreaO[4]);

BYTE * pIODataStatusAreaI  = (BYTE *)&(IODataAreaI[STATUS_BASE_BYTE + 1]);
BYTE * pIODataStatusAreaO  = (BYTE *)&(IODataAreaO[STATUS_BASE_BYTE + 1]);

WORD IOSyncroAreaO[SYNCRO_DB_SIZE_ELEM + 1];
WORD * pIOSyncroAreaO = &(IOSyncroAreaO[1]);

WORD IOSyncroAreaI[SYNCRO_DB_SIZE_ELEM + 1];
WORD * pIOSyncroAreaI = &(IOSyncroAreaI[1]);

#ifdef ENABLE_DEVICE_DISCONNECT
/*Array used to dinamically enable or disable a device */
short int device_status[prot_none_e][MAX_DEVICE_NB];
#endif

sem_t theWritingSem;

const char * _protocol_map_[] =
{
    TAG_RTU,
    TAG_TCP,
    TAG_TCPRTU,
    ""
};

char DeviceReconnected;
size_t SyncroAreaSize = 0;

variable_t varNameArray[DB_SIZE_ELEM + 1];
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
store_t StoreArrayS[DB_SIZE_ELEM];
store_t StoreArrayF[DB_SIZE_ELEM];
store_t StoreArrayV[DB_SIZE_ELEM];
int store_elem_nb_S = 0;
int store_elem_nb_F = 0;
int store_elem_nb_V = 0;
#endif

char CrossTableErrorMsg[256];

static int ActualAddress = 0;
static int LastAddress = 0;
static int ActualNreg = 0;
static int LastNreg = 0;
static int blockSize = 1;

int cmpSyncroCtIndex(WORD address, int CtIndex);
int setSyncroCtIndex(WORD * address, int CtIndex);
static void localCheckWriting(void);
int setStatusVar(const char * varname, char Status);
int setStatusVarBySynIndex(int SynIndex, char Status);

pthread_mutex_t data_recv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t data_send_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sync_recv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sync_send_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t write_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static write_queue_elem_t * queue_head = NULL, * queue_tail = NULL;

/**
 * Open the cross table and fill the syncro vector with the Mirror variables and the PLC variables
 *
 * The cross-table file is an acii file with the fllowing syntax:
 * <Enable>; <PLC>; <Tag>; <Type>; <Decimal>; <Protocol>; <Port>; <NodeId>; <Address>; <Block>; <Nreg>; <Handle>
 *
 * Enable:   Flag 0/1
 * PLC:      Flag 0/1
 * Tag:      char[16]
 * type:     UINT|INT|UDINTABCD|UDINTBADC|UDINTCDAB|UDINTDCBA|DINTABCD|DINTBADC|DINTCDAB|DINTDCBA|RABCD|RBADC|RCDAB|RDCBA
 * Decimal:  0-4|address
 * Protocol: RTU|TCP|TCPRTU
 * Port:     char[N]
 * NodeId:   int
 * Address:  word
 * Block:    word
 * nReg:     int max 64
 * Handle
 *
 * syncro vector: array of DWORD
 * 3bit + 27bit + 2bit
 * offset, R, W
 */
size_t fillSyncroArea(void)
{
    FILE * fp;
    int elem_nb = 1;
    char line[LINE_SIZE];
    char token[LINE_SIZE] = "";
    char * p = NULL;
    int isreading = 0;
    int bytebitperblock = 0;
    int wordbitperblock = 0;
    int dwordbitperblock = 0;
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
    int isstores = 0;
    int isstoref = 0;
    int isstorev = 0;
#endif

#ifdef ENABLE_DEVICE_DISCONNECT
    int i, j;
    for (i = 0; i < prot_none_e; i++)
    {
        for (j = 0; j < MAX_DEVICE_NB; j++)
        {
            device_status[i][j] = -1;
        }
    }
#endif

    fp = fopen(CROSS_TABLE, "r");
    if (fp == NULL)
    {
        sprintf(CrossTableErrorMsg, "Cannot open The cross table '%s': [%s]", CROSS_TABLE, strerror(errno));
        LOG_PRINT(error_e, "%s\n", CrossTableErrorMsg);
        return -1;
    }

    memset (pIOSyncroAreaI, 0x0, SYNCRO_DB_SIZE_ELEM);
    memset (pIOSyncroAreaO, 0x0, SYNCRO_DB_SIZE_ELEM);
    memset (pIODataAreaI, 0x0, STATUS_BASE_BYTE + DB_SIZE_BYTE);
    memset (pIODataAreaO, 0x0, STATUS_BASE_BYTE + DB_SIZE_BYTE);

    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        p = line;
        LOG_PRINT(verbose_e, "%s\n", line);
        /* extract the enable/disable field */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            unsigned int i;
            for (i = 0; i < strlen(line) && isspace(line[i]); i++);
            if (i == strlen(line))
            {
                LOG_PRINT(warning_e, "Empty line skipped\n");
                continue;
            }
            else
            {
                sprintf(CrossTableErrorMsg, "Malformed element 'enabled/disabled'");
                LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                return elem_nb;
            }
        }
        if (atoi(token) == 0)
        {
            LOG_PRINT(verbose_e, "element %d disabled [%s]\n", elem_nb, line);
            elem_nb++;
            continue;
        }

        /* extract PLC flag */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Promoted'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
        isstores = 0;
        isstoref = 0;
        isstorev = 0;
#endif
        if (token[0] == TAG_PLC || token[0] == TAG_STORED_SLOW || token[0] == TAG_STORED_FAST || token[0] == TAG_STORED_ON_VAR || IS_MIRROR(elem_nb))
        {
            LOG_PRINT(verbose_e, "%s ELEMENT %d [%s]\n", (IS_MIRROR(elem_nb) == 1) ? "MIRROR": "PLC", elem_nb, line);
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
            if (token[0] == TAG_STORED_SLOW)
            {
                isstores = 1;
            }
            if (token[0] == TAG_STORED_FAST)
            {
                isstoref = 1;
            }
            if (token[0] == TAG_STORED_ON_VAR)
            {
                isstorev = 1;
            }
#endif
            isreading = 1;
        }
        else
        {
            LOG_PRINT(verbose_e, "NORMAL ELEMENT %d [%s]\n", elem_nb, line);
            isreading = 0;
        }

        /* Tag */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Tag'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        if (strlen(token) > TAG_LEN)
        {
            sprintf(CrossTableErrorMsg, "Tag '%s' too long, maximum lenght is %d", token, TAG_LEN - 1);
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        int i;
        for (i = 0; i < elem_nb; i++)
        {
            if (strcmp(token, varNameArray[i].tag) == 0)
            {
                sprintf(CrossTableErrorMsg, "Duplicated tag '%s' at lines %d and %d ", token, i, elem_nb);
                LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                return elem_nb;
            }
        }

        if (strlen(token) >= TAG_LEN)
        {
            sprintf(CrossTableErrorMsg, "Tag len '%s' too long (%d vs %d)", token, strlen(token), TAG_LEN);
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        strcpy(varNameArray[elem_nb].tag, token);

        /* type */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Type'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        if (strcmp(token, "UINT") == 0 || strcmp(token, "UINTAB") == 0)
        {
            varNameArray[elem_nb].type = uintab_e;
        }
        else if (strcmp(token, "UINTBA") == 0)
        {
            varNameArray[elem_nb].type = uintba_e;
        }
        else if (strcmp(token, "INT") == 0 || strcmp(token, "INTAB") == 0)
        {
            varNameArray[elem_nb].type = intab_e;
        }
        else if (strcmp(token, "INTBA") == 0)
        {
            varNameArray[elem_nb].type = intba_e;
        }
        else if (strcmp(token, "UDINT") == 0 || strcmp(token, "UDINTABCD") == 0)
        {
            varNameArray[elem_nb].type = udint_abcd_e;
        }
        else if (strcmp(token, "UDINTBADC") == 0)
        {
            varNameArray[elem_nb].type = udint_badc_e;
        }
        else if (strcmp(token, "UDINTCDAB") == 0)
        {
            varNameArray[elem_nb].type = udint_cdab_e;
        }
        else if (strcmp(token, "UDINTDCBA") == 0)
        {
            varNameArray[elem_nb].type = udint_dcba_e;
        }
        else if (strcmp(token, "DINT") == 0 || strcmp(token, "DINTABCD") == 0)
        {
            varNameArray[elem_nb].type = dint_abcd_e;
        }
        else if (strcmp(token, "DINTBADC") == 0)
        {
            varNameArray[elem_nb].type = dint_badc_e;
        }
        else if (strcmp(token, "DINTCDAB") == 0)
        {
            varNameArray[elem_nb].type = dint_cdab_e;
        }
        else if (strcmp(token, "DINTDCBA") == 0)
        {
            varNameArray[elem_nb].type = dint_dcba_e;
        }
        else if (strcmp(token, "FABCD") == 0 || strcmp(token, "FLOAT") == 0 || strcmp(token, "RABCD") == 0 || strcmp(token, "REAL") == 0)
        {
            varNameArray[elem_nb].type = fabcd_e;
        }
        else if (strcmp(token, "FBADC") == 0 || strcmp(token, "RBADC") == 0)
        {
            varNameArray[elem_nb].type = fbadc_e;
        }
        else if (strcmp(token, "FCDAB") == 0 || strcmp(token, "RCDAB") == 0)
        {
            varNameArray[elem_nb].type = fcdab_e;
        }
        else if (strcmp(token, "FDCBA") == 0 || strcmp(token, "RDCBA") == 0)
        {
            varNameArray[elem_nb].type = fdcba_e;
        }
        else if (strcmp(token, "BIT") == 0)
        {
            varNameArray[elem_nb].type = bit_e;
        }
        else if (strcmp(token, "BYTE_BIT") == 0)
        {
            varNameArray[elem_nb].type = bytebit_e;
        }
        else if (strcmp(token, "WORD_BIT") == 0)
        {
            varNameArray[elem_nb].type = wordbit_e;
        }
        else if (strcmp(token, "DWORD_BIT") == 0)
        {
            varNameArray[elem_nb].type = dwordbit_e;
        }
        else
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'TYPE' '%s'", token);
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        LOG_PRINT(info_e, "Variable %s Type BIT %d\n", varNameArray[elem_nb].tag, varNameArray[elem_nb].type);

        /* Decimal */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Decimal'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        if (
                varNameArray[elem_nb].type == bit_e ||
                varNameArray[elem_nb].type == bytebit_e ||
                varNameArray[elem_nb].type == wordbit_e ||
                varNameArray[elem_nb].type == dwordbit_e
                )
        {
            varNameArray[elem_nb].decimal = 0;
        }
        else
        {
            varNameArray[elem_nb].decimal = atoi(token);
        }


        /* Protocol */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Protocol'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        if(strcmp(TAG_RTU, token) == 0)
        {
            varNameArray[elem_nb].protocol = prot_rtu_e;
        }
        else if(strcmp(TAG_TCP, token)==0)
        {
            varNameArray[elem_nb].protocol = prot_tcp_e;
        }
        else if(strcmp(TAG_TCPRTU, token)==0)
        {
            varNameArray[elem_nb].protocol = prot_tcprtu_e;
        }
        else
        {
            varNameArray[elem_nb].protocol = prot_none_e;
        }

        /* Port */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Port'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        /* IP */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'IP'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        /* NodeId */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'NodeId'");
            LOG_PRINT(error_e, "%s at line %d. - p %p token %s\n", CrossTableErrorMsg, elem_nb, p, token);
            return elem_nb;
        }
        if (p != NULL && token[0] == '\0')
        {
            /* it no node is set and is TCP protocol, force it to 1 */
            if (varNameArray[elem_nb].protocol == prot_tcp_e)
            {
                varNameArray[elem_nb].node = 1;
            }
            /* it no node is set and is not TCP protocol, force it to INTERNAL_VARIABLE_FAKE_NODEID */
            else
            {
                varNameArray[elem_nb].node = INTERNAL_VARIABLE_FAKE_NODEID;
            }
        }
        else
        {
            /* if the protocol is not specified, assume that this is an internal variable */
            if (varNameArray[elem_nb].protocol == prot_none_e && varNameArray[elem_nb].node != INTERNAL_VARIABLE_FAKE_NODEID)
            {
                LOG_PRINT(warning_e, "No protocol is specifyed, force the not from %d to internal node %d. \n", varNameArray[elem_nb].node, INTERNAL_VARIABLE_FAKE_NODEID);
                varNameArray[elem_nb].node = INTERNAL_VARIABLE_FAKE_NODEID;
            }
            else
            {
                varNameArray[elem_nb].node = atoi(token);
                if (varNameArray[elem_nb].node <= 0 || varNameArray[elem_nb].node > MAX_DEVICE_NB)
                {
                    sprintf(CrossTableErrorMsg, "invalid node %d for variable '%s'\n", varNameArray[elem_nb].node, varNameArray[elem_nb].tag);
                    LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                    return elem_nb;
                }
            }
        }
        LOG_PRINT(verbose_e, "Node %d [%s].\n", varNameArray[elem_nb].node, token);

        /* Address */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Address'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        if (varNameArray[elem_nb].node != INTERNAL_VARIABLE_FAKE_NODEID)
        {
            LastAddress = ActualAddress;
            ActualAddress = atoi(token);
        }

        /* Block */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Block'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        varNameArray[elem_nb].block = atoi(token);
        LOG_PRINT(verbose_e, "BLOCK %d VAR %s, line %s, token %s\n", varNameArray[elem_nb].block, varNameArray[elem_nb].tag, line, token);

        /* extract the block head */
        if (elem_nb > 0 && varNameArray[elem_nb].block == varNameArray[elem_nb - 1].block)
        {
            varNameArray[elem_nb].blockhead = varNameArray[elem_nb - 1].blockhead;
        }
        else
        {
            varNameArray[elem_nb].blockhead = elem_nb;
        }

        /* NReg */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Nreg'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        LastNreg = ActualNreg;
        ActualNreg = atoi(token);
        LOG_PRINT(verbose_e, "NREG %d VAR %s\n", ActualNreg, varNameArray[elem_nb].tag);

        /* if the handle tag is not present, a field is missing and the cross table is malformed */
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'handle'. check the number of the field.");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        if (elem_nb > 1)
        {
            /* the actual variable is into the same block of the previous variable */
            if (varNameArray[elem_nb].block != 0 && varNameArray[elem_nb].block == varNameArray[elem_nb - 1].block)
            {
#if 0
                /* check if the block is omogenous */
                if (varNameArray[elem_nb].block != 0 && elem_nb > 0 && varNameArray[elem_nb].type != varNameArray[elem_nb - 1].type)
                {
                    sprintf(CrossTableErrorMsg, "Malformed 'Block', not omogeneous type");
                    LOG_PRINT(error_e, "%s at line %d. %d -> %d\n", CrossTableErrorMsg, elem_nb, varNameArray[elem_nb].type, varNameArray[elem_nb - 1].type);
                    return elem_nb;
                }
#endif
                if (varNameArray[elem_nb].type == bytebit_e)
                {
                    bytebitperblock++;
                }
                else
                {
                    bytebitperblock = 0;
                }

                if (varNameArray[elem_nb].type == wordbit_e)
                {
                    wordbitperblock++;
                }
                else
                {
                    wordbitperblock = 0;
                }

                if (varNameArray[elem_nb].type == dwordbit_e)
                {
                    dwordbitperblock++;
                }
                else
                {
                    dwordbitperblock = 0;
                }

                /* check if two elements in the same block have a consecutive modbus address */
                if (
                        varNameArray[elem_nb].node != INTERNAL_VARIABLE_FAKE_NODEID &&
                        (
                            (
                                ActualAddress != LastAddress + 1 &&
                                varNameArray[elem_nb - 1].type <= intba_e
                                ) ||
                            (
                                ActualAddress != LastAddress + 2 &&
                                varNameArray[elem_nb - 1].type > intba_e
                                )
                            ) &&
                        varNameArray[elem_nb].type == bytebit_e && bytebitperblock > 8
                        &&
                        varNameArray[elem_nb].type == wordbit_e && wordbitperblock > 8
                        &&
                        varNameArray[elem_nb].type == dwordbit_e && dwordbitperblock > 8
                        )
                {
                    sprintf(CrossTableErrorMsg, "Malformed 'Block', not consecutive address");
                    LOG_PRINT(error_e, "%s at line %d. %d -> %d, type %d\n", CrossTableErrorMsg, elem_nb, LastAddress, ActualAddress, varNameArray[elem_nb - 1].type);
                    return elem_nb;
                }
                if (LastNreg != ActualNreg)
                {
                    sprintf(CrossTableErrorMsg, "Malformed 'Nreg'");
                    LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                    return elem_nb;
                }
                blockSize++;
                LOG_PRINT(verbose_e, " %d - '%s'\n", blockSize, line);
            }
            /* the actual variable is into a different block of the previous variable */
            else
            {
                /* check if the previous block size is equal to number of reg to read */
                if (LastNreg != 0 && LastNreg != blockSize)
                {
                    sprintf(CrossTableErrorMsg, "Malformed Block size");
                    LOG_PRINT(error_e, "%s at line %d. %dvs%d\n", CrossTableErrorMsg, elem_nb, LastNreg, blockSize);
                    return elem_nb;
                }
                LOG_PRINT(verbose_e, "New BLOCK\n");
                blockSize = 1;
                bytebitperblock = 0;
                wordbitperblock = 0;
                dwordbitperblock = 0;
            }
            if (varNameArray[elem_nb].decimal < 0
                    ||
                    (varNameArray[elem_nb].decimal > 4 && varNameArray[elem_nb].decimal > elem_nb))
            {
                sprintf(CrossTableErrorMsg, "Malformed Decimal address");
                LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                return elem_nb;
            }
        }
        /* set reading */
        if (isreading == 1)
        {
            int SynIndex = 0;
            int CtIndex = 0;
            char blockhead[TAG_LEN] = "";

            LOG_PRINT(verbose_e, "ELEMENT %d is reading\n", elem_nb);

            /* check if the block or if the variable is already active */
            if (isBlockActive(varNameArray[elem_nb].tag, blockhead) == 0)
            {
                /* looking the variable index from the crosstable structure */
                /* insert it into the syncrovector */
                if (addSyncroElement(blockhead, &CtIndex) == 0)
                {
                    if (CtIndex2SynIndex(CtIndex, &SynIndex) != 0)
                    {
                        LOG_PRINT(error_e, "Cannot add the block to the syncroelement\n");
                        return elem_nb;
                    }
                    /* enable it in reading */
                    SET_SYNCRO_FLAG(SynIndex, READ_MASK);
                    LOG_PRINT(verbose_e, "Set reading flag\n");
                }
                else
                {
                    LOG_PRINT(error_e, "Cannot add the block to the syncroelement\n");
                }
            }
            else
            {
                LOG_PRINT(warning_e, "The variable '%s' come from a block already active\n", varNameArray[elem_nb].tag);
            }
            varNameArray[elem_nb].active = 1;
        }
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
        if (isstores == 1)
        {
            strcpy(StoreArrayS[store_elem_nb_S].tag, varNameArray[elem_nb].tag);
            if (Tag2CtIndex(StoreArrayS[store_elem_nb_S].tag, &(StoreArrayS[store_elem_nb_S].CtIndex)) != 0)
            {
                LOG_PRINT(error_e, "cannot find variable '%s'", StoreArrayS[store_elem_nb_S].tag);
            }
            else
            {
                LOG_PRINT(verbose_e, "a new store variable is inserted '%s' at position %d\n", StoreArrayS[store_elem_nb_S].tag, store_elem_nb_S);
                store_elem_nb_S++;
            }
        }
        if (isstoref == 1)
        {
            strcpy(StoreArrayF[store_elem_nb_F].tag, varNameArray[elem_nb].tag);
            if (Tag2CtIndex(StoreArrayF[store_elem_nb_F].tag, &(StoreArrayF[store_elem_nb_F].CtIndex)) != 0)
            {
                LOG_PRINT(error_e, "cannot find variable '%s'", StoreArrayF[store_elem_nb_F].tag);
            }
            else
            {
                LOG_PRINT(verbose_e, "a new store variable is inserted '%s' at position %d\n", StoreArrayF[store_elem_nb_F].tag, store_elem_nb_F);
                store_elem_nb_F++;
            }
        }
        if (isstorev == 1)
        {
            strcpy(StoreArrayV[store_elem_nb_V].tag, varNameArray[elem_nb].tag);
            if (Tag2CtIndex(StoreArrayV[store_elem_nb_V].tag, &(StoreArrayV[store_elem_nb_V].CtIndex)) != 0)
            {
                LOG_PRINT(error_e, "cannot find variable '%s'", StoreArrayV[store_elem_nb_V].tag);
            }
            else
            {
                LOG_PRINT(verbose_e, "a new store variable is inserted '%s' at position %d\n", StoreArrayV[store_elem_nb_V].tag, store_elem_nb_V);
                store_elem_nb_V++;
            }
        }
#endif

        LOG_PRINT(info_e, "'%s' %d\n", varNameArray[elem_nb].tag, elem_nb);

        elem_nb++;
    }
    fclose(fp);
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
    LOG_PRINT(info_e, "Loaded %d record stored record S: %d F: %d V: %d\n", elem_nb, store_elem_nb_S, store_elem_nb_F, store_elem_nb_V);
#else
    LOG_PRINT(info_e, "Loaded %d record\n", elem_nb);
#endif
    CrossTableErrorMsg[0] = '\0';

#ifdef ENABLE_DEVICE_DISCONNECT
    /* Initialize the arrays containg the status of active devices*/
    for(i = 1; i < elem_nb; i++)
    {

        if (setupConnectedDevice(varNameArray[i].protocol, varNameArray[i].node) == 0)
        {
            LOG_PRINT(verbose_e, "Activated Protocol %d, Node %d VAR '%s' device_status %d\n", varNameArray[i].protocol, varNameArray[i].node, varNameArray[i].tag, device_status[varNameArray[i].protocol][varNameArray[i].node]);
        }

    }
#endif
    return elem_nb;
}

/**
 * @brief get the synIndex of the variable Tag if it is into the syncrotable
 */
int Tag2SynIndex(const char * tag, int * SynIndex)
{
    int CtIndex = -1;

    *SynIndex = -1;

    if (tag[0] == '\0')
    {
        LOG_PRINT(error_e, "Empty tag '%s'\n", tag);
        return -1;
    }

    if (Tag2CtIndex(tag, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", tag);
        return -1;
    }

    LOG_PRINT(verbose_e, "'%s' CtIndex: %d - SyncroAreaSize %d\n", tag, CtIndex, SyncroAreaSize);
    CtIndex2SynIndex(CtIndex, SynIndex);

    if (*SynIndex == -1)
    {
        LOG_PRINT(info_e, "'%s' not found into SyncroVector\n", tag);
        return -1;
    }
    else
    {
        LOG_PRINT(verbose_e, "found '%s' -> %d into SyncroVector\n", tag, *SynIndex);
        return 0;
    }
}

/**
 * @brief add an element into the Syncro vector that have the Tag "tag" and the index CtIndex into
 * the cross-table
 *
 */
int addSyncroElementbyIndex(const char * tag, int CtIndex)
{
    int synIndex = 0;
    if (Tag2SynIndex(tag, &synIndex) != 0)
    {
        LOG_PRINT(info_e, "Adding tag %s, CtIndex %d SyncroAreaSize %d\n", tag, CtIndex, SyncroAreaSize);
        setSyncroCtIndex(&(pIOSyncroAreaO[SyncroAreaSize]), CtIndex);
        SyncroAreaSize++;
        LOG_PRINT(info_e, "Added tag %s, CtIndex %d SyncroAreaSize %d\n", tag, CtIndex, SyncroAreaSize);
        return 0;
    }
    else
    {
        LOG_PRINT(verbose_e, "The tag %s, CtIndex %d synIndex %d already exist!\n", tag, CtIndex, synIndex);
        return 0;
    }
    LOG_PRINT(info_e, "Cannot add tag %s, CtIndex %d\n", tag, CtIndex);
    return -1;
}

/** @brief compare the block of two variables
 * @param int i: first cross table index
 * @param int j: second cross table index
 * @return 0 if the block is the same
 * @return 0 if the block is different
 */
int cmpCrossTableBlock(int i, int j)
{
    return (varNameArray[i].block != varNameArray[j].block);
}

/**
 * @brief add a block of elements into the Syncro vector. The block is the element's block the that have the Tag "tag" into
 * the cross-table and set the variable index with the CtIndex of the variable
 * into the cross-table
 *
 */
int addSyncroElement(const char * tag, int * CtIndex)
{
    *CtIndex = 0;

    if (tag[0] == '\0')
    {
        LOG_PRINT(error_e, "Empty tag '%s'\n", tag);
        return -1;
    }

    /* extract the cross-table index from the cross-table */
    Tag2CtIndex(tag, CtIndex);
    if(*CtIndex < 0)
    {
        LOG_PRINT(error_e, "Cannot find the index for variable '%s'\n", tag);
        return *CtIndex;
    }

    return addSyncroElementbyIndex(tag, *CtIndex);
}

/**
 * @brief delete an element from the Syncro vector that have the Tag "tag" into
 * the cross-table and set the variable CtIndex with the index of the variable
 * into the cross-table
 *
 */
int delSyncroElement(const char * tag)
{
    int synIndex;
    LOG_PRINT(info_e, "Deleting %s\n", tag);
    if (Tag2SynIndex(tag, &synIndex) == 0)
    {
        return delSyncroElementByIndex(synIndex);
    }
    else
    {
        return 0;
    }
}

/**
 * @brief delete the element SynIndex from the Syncro vector
 */
int delSyncroElementByIndex(int synIndex)
{
    unsigned int i;

#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&sync_send_mutex);
#endif

    for (i = synIndex; i < SyncroAreaSize; i++)
    {
        pIOSyncroAreaO[i] = pIOSyncroAreaO[i + 1];
    }

    SyncroAreaSize--;

#ifdef ENABLE_MUTEX
    pthread_mutex_unlock(&sync_send_mutex);
#endif

    return 0;
}

/**
 * @brief delete all the element into the Syncro vector
 */
int emptySyncroElement()
{
    int CtIndex = 0;
    int i;

    for (i = 0; (unsigned int)i < SyncroAreaSize; i++)
    {
        if (SynIndex2CtIndex(i, &CtIndex) == 0)
        {
            if (varNameArray[CtIndex].active == 0)
            {
                LOG_PRINT(info_e, "%d - '%s'\n", CtIndex, varNameArray[CtIndex].tag);
                delSyncroElementByIndex(i);
            }
            else
            {
                LOG_PRINT(info_e, "'%s' is always active\n", varNameArray[CtIndex].tag);
            }
        }
    }

    return 0;
}

/** @brief Compare a cross table index and a syncro element
 * @param WORD address :  syncro table element
 * @param int CtIndex :  cross table index
 * @return 0 if the cross table index and a syncro element are the same
 * @return 1 if the cross table index and a syncro element are different
 */
int cmpSyncroCtIndex(WORD address, int CtIndex)
{
    LOG_PRINT(verbose_e, "CtIndex: %X address %X - %X\n", CtIndex, (int) (address & ADDRESS_MASK), address);
    return !(CtIndex == ((int)(address & ADDRESS_MASK)));
}

/** @brief Set the cross table index into a syncro table element
 * @param WORD * address :  syncro table element
 * @param int CtIndex :  cross table index
 */
int setSyncroCtIndex(WORD * address, int CtIndex)
{
    LOG_PRINT(verbose_e, "CtIndex: %d, address %d\n", CtIndex, *address);
    *address = (WORD)CtIndex;
    LOG_PRINT(verbose_e, "CtIndex: %d, address %d\n", CtIndex, *address);

    return 0;
}

/** @brief Extract the cross table index from a syncro table element
 * @param WORD address :  syncro table element
 * @param int CtIndex :  cross table index
 */
int getSyncroCtIndex(WORD address, int * CtIndex)
{
    *CtIndex = (int)(address & ADDRESS_MASK);
    return 0;
}

/** @brief Extract the cross table index from the syncro table index
 * @param int SynIndex :  syncro table index
 * @return 0 ok
 * @return != error
 */
int SynIndex2CtIndex(int SynIndex, int * CtIndex)
{
    *CtIndex = -1;
    if (getSyncroCtIndex(pIOSyncroAreaO[SynIndex], CtIndex) == 0)
    {
        return 0;
    }
    return 1;
}

/** @brief Extract the syncro table index from the cross table index
 * @param const char * Tag : variable tag
 * @param int * CtIndex :  cross table index
 * @return -1 Cross table element not found into the syncro table
 * @return otherwise syncro table index
 */
int CtIndex2SynIndex(int CtIndex, int *SynIndex)
{
    *SynIndex = -1;
    unsigned int i;

#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&sync_send_mutex);
#endif

    for (i = 0; i < SyncroAreaSize; i++)
    {
        if (cmpSyncroCtIndex(pIOSyncroAreaO[i], CtIndex) == 0)
        {
            *SynIndex = i;
            break;
        }
    }

#ifdef ENABLE_MUTEX
    pthread_mutex_unlock(&sync_send_mutex);
#endif
    return (*SynIndex == -1);
}

/** @brief Extract the cross table index of the variable tagget as 'Tag'
 * @param const char * Tag : variable tag
 * @param int * CtIndex :  cross table index
 * @return -1 Some error occured
 * @return 0 Write done
 */
int Tag2CtIndex(const char * Tag, int * CtIndex)
{
    int i;
    *CtIndex = -1;

    if (Tag[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", Tag);
        return -1;
    }

    for (i = 1; i <= DB_SIZE_ELEM; i++)
    {
        if (strcmp(varNameArray[i].tag, Tag) == 0)
        {
            *CtIndex = i;
            LOG_PRINT(verbose_e, "found '%s' at %d\n", Tag, i);
            return 0;
        }
    }
    LOG_PRINT(verbose_e, "Cannot found '%s'\n", Tag);
    return -1;
}

/**
 * @brief extract the Tag of the variable positioned at the index cross table CtIndex
 */
int CtIndex2Tag(int CtIndex, char * Tag)
{
    strcpy(Tag, varNameArray[CtIndex].tag);
    return 0;
}

/**
 * @brief extract the type of the variable positioned at the index cross table CtIndex
 */
int CtIndex2Type(int CtIndex)
{
    return varNameArray[CtIndex].type;
}

/** @brief Read the value from the internal DB at psition 'ctIndex'
 * @param int ctIndex : Cross table index of the variable to read
 * @param void * value: value read
 * @return 1 Some error occured
 * @return 0 Write done
 */
int readFromDb(int ctIndex, void * value)
{
    if (value == NULL)
    {
        LOG_PRINT(error_e, "NULL value pointer\n");
        return -1;
    }

    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }

#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByCtIndex(ctIndex) != 1)
    {
        return -1;
    }
#endif

    /* fieldbus in error */
    if (getErrorBit(varNameArray[ctIndex].protocol, error_blacklist_e, varNameArray[ctIndex].node) == 1)
    {
        LOG_PRINT(error_e, "Communication error for channel '%d'\n", varNameArray[ctIndex].protocol);
        return -1;
    }

    int byte_nb = (ctIndex - 1) * 4;
    LOG_PRINT(verbose_e, "'%s': %X\n", varNameArray[ctIndex].tag, pIODataAreaI[byte_nb]);
    int type = CtIndex2Type(ctIndex);
#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&data_recv_mutex);
#endif
    switch(type)
    {
    case uintab_e:
    case uintba_e:
        memcpy(value, &(pIODataAreaI[byte_nb]), sizeof(unsigned short));
        LOG_PRINT(verbose_e, "uint_e: %d\n", *((unsigned short*)value));
        break;
    case intab_e:
    case intba_e:
        memcpy(value, &(pIODataAreaI[byte_nb]), sizeof(short));
        LOG_PRINT(verbose_e, "int_e: %d\n", *((short*)value));
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        memcpy(value, &(pIODataAreaI[byte_nb]), sizeof(unsigned int));
        LOG_PRINT(verbose_e, "udint_e: %d\n", *((unsigned int*)value));
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        memcpy(value, &(pIODataAreaI[byte_nb]), sizeof(int));
        LOG_PRINT(verbose_e, "dint_e: %d\n", *((int*)value));
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        memcpy(value, &(pIODataAreaI[byte_nb]), sizeof(DWORD));
        LOG_PRINT(verbose_e, "FLOAT: %f\n", *((float*)value));
        break;
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        *((BYTE*)value) = pIODataAreaI[byte_nb];
        LOG_PRINT(verbose_e, "BIT: %d\n", *((BYTE*)value));
        break;
    default:
        LOG_PRINT(error_e, "Unknown type '%d'\n", varNameArray[ctIndex].type);
        return -1;
        break;
    }
#ifdef ENABLE_MUTEX
    pthread_mutex_unlock(&data_recv_mutex);
#endif
    return 0;
}

/** @brief Write the value 'value' into the internal DB at psition 'ctIndex'
 * @param int ctIndex : Cross table index of the variable to write
 * @param void * value: value to write
 * @return 1 Some error occured
 * @return 0 Write done
 */
int writeToDb(int ctIndex, void * value)
{
    int byte_nb = (ctIndex - 1) * 4;
    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }

#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByCtIndex(ctIndex) != 1)
    {
        return -1;
    }
#endif

    /* fieldbus in error */
    if (getErrorBit(varNameArray[ctIndex].protocol, error_blacklist_e, varNameArray[ctIndex].node) == 1)
    {
        LOG_PRINT(error_e, "Communication error for channel '%d'\n", varNameArray[ctIndex].protocol);
        return -1;
    }

    int type = CtIndex2Type(ctIndex);

#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&data_send_mutex);
#endif
    switch(type)
    {
    case uintab_e:
    case uintba_e:
        memcpy(&(pIODataAreaO[byte_nb]), (unsigned short int*)value, sizeof(unsigned short int));
        LOG_PRINT(verbose_e, "UINT %d\n", pIODataAreaO[byte_nb]);
        break;
    case intab_e:
    case intba_e:
        memcpy(&(pIODataAreaO[byte_nb]), (short int*)value, sizeof(short int));
        LOG_PRINT(verbose_e, "INT %d\n", pIODataAreaO[byte_nb]);
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        memcpy(&(pIODataAreaO[byte_nb]), (unsigned int*)value, sizeof(unsigned int));
        LOG_PRINT(verbose_e, "UDINT %d\n", pIODataAreaO[byte_nb]);
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        memcpy(&(pIODataAreaO[byte_nb]), (int *)value, sizeof(int));
        LOG_PRINT(verbose_e, "DINT %d\n", pIODataAreaO[byte_nb]);
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        memcpy(&(pIODataAreaO[byte_nb]), (float*)value, sizeof(float));
        LOG_PRINT(verbose_e, "FLOAT %f -> .%X.%X.%X.%X.\n", *((float*)value), pIODataAreaO[byte_nb], pIODataAreaO[byte_nb + 1], pIODataAreaO[byte_nb + 2], pIODataAreaO[byte_nb + 3]);
        break;
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        LOG_PRINT(verbose_e, "BIT\n");
        pIODataAreaO[byte_nb] = (WORD)(*((BYTE*)value));
        break;
    default:
        LOG_PRINT(error_e, "Unknown type '%d'\n", varNameArray[ctIndex].type);
        return -1;
        break;
    }
#ifdef ENABLE_MUTEX
    pthread_mutex_unlock(&data_send_mutex);
#endif
    LOG_PRINT(verbose_e, "%d %d '%s': %X\n", ctIndex, byte_nb, varNameArray[ctIndex].tag, pIODataAreaO[byte_nb]);

    return 0;
}

/** @brief Read the value from the internal DB at psition 'ctIndex'
 * and print into the string 'value' in according with the format described into the cross table
 * @param int ctIndex : Cross table index of the variable to read
 * @param void * value: value read and print in the right format
 * @return 1 Some error occured
 * @return 0 Write done
 */
int formattedReadFromDb(int ctIndex, char * value)
{
    int decimal = 0;

    /* invalid ctIndex */
    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }

#ifdef ENABLE_DEVICE_DISCONNECT
    /* device disconnected */
    if (isDeviceConnectedByCtIndex(ctIndex) != 1)
    {
        LOG_PRINT(error_e, "Device disconnected Ctindex %d\n", ctIndex);
        return -1;
    }
#endif

    /* fieldbus in error */
    if (getErrorBit(varNameArray[ctIndex].protocol, error_blacklist_e, varNameArray[ctIndex].node) == 1)
    {
        LOG_PRINT(error_e, "Communication error for channel '%d'\n", varNameArray[ctIndex].protocol);
        return -1;
    }

    /* be sure that the variable is active */
    int SynIndex = -1;
    int myCtIndex = ctIndex;
    /* get the variable address from syncrovector*/
    if (CtIndex2SynIndex(ctIndex, &SynIndex) != 0)
    {
        /* if not exist this variable into the syncro vector, check if exist his headblock, and get the its CtIndex */
        myCtIndex = varNameArray[ctIndex].blockhead;
        if (ctIndex == myCtIndex || CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
        {
            if (activateVar(varNameArray[myCtIndex].tag) != 0)
            {
                LOG_PRINT(error_e, "The variable %d - %s is not active and is not activable\n", myCtIndex, varNameArray[myCtIndex].tag);
                return 1;
            }
            else
            {
                if (CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
                {
                    LOG_PRINT(error_e, "The variable %d - %s is not active and is not activable\n", myCtIndex, varNameArray[myCtIndex].tag);
                    return 1;
                }
            }
        }
    }

    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);

    decimal = getVarDecimal(ctIndex);

    LOG_PRINT(verbose_e, "CURRENT Decimal is %d to be used for VARIABLE %s\n", decimal, varNameArray[ctIndex].tag);

    switch(	CtIndex2Type(ctIndex))
    {
    case uintab_e:
    case uintba_e:
    {
        unsigned short int _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            char fmt[8] = "";
            sprintf (fmt, "%%.%df", decimal);
            sprintf(value, fmt, (float)_value / pow(10,decimal));
        }
        else
        {
            sprintf(value, "%u", (unsigned short)_value);
        }
    }
        break;
    case intab_e:
    case intba_e:
    {
        short int _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            char fmt[8] = "";
            sprintf (fmt, "%%.%df", decimal);
            sprintf(value, fmt, (float)_value / pow(10,decimal));
        }
        else
        {
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
        if (readFromDb(ctIndex, &_value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
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
        if (readFromDb(ctIndex, &_value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            char fmt[8] = "";
            sprintf (fmt, "%%.%df", decimal);
            sprintf(value, fmt, (float)_value / pow(10,decimal));
        }
        else
        {
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
        if (readFromDb(ctIndex, &_value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %f decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
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
        if (readFromDb(ctIndex, &_value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return -1;
        }
        sprintf(value, "%d", (BYTE)_value);
    }
        break;
    default:
        LOG_PRINT(error_e, "unknown type %d\n", ctIndex);
        return -1;
        break;
    }

    switch (getStatusVarByCtIndex(myCtIndex, NULL))
    {
    case DONE:
        break;
    case ERROR:
        LOG_PRINT(error_e, "getStatusVarByCtIndex ERROR %d\n", ctIndex);
        return -1;
        break;
    case BUSY:
        break;
    default:
        LOG_PRINT(error_e, "getStatusVarByCtIndex default %d\n", ctIndex);
        return -1;
        break;
    }

    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%s'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, value);
    return 0;
}

int formattedWriteToDb(int ctIndex, void * value)
{
    int decimal = 0;
    decimal = getVarDecimal(ctIndex);

    if (decimal > 0 || varNameArray[ctIndex].decimal > 4)
    {
        if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
        {
            LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
            return -1;
        }

#ifdef ENABLE_DEVICE_DISCONNECT
        if (isDeviceConnectedByCtIndex(ctIndex) != 1)
        {
            return -1;
        }
#endif
#if 0
        /* fieldbus in error */
        if (getErrorBit(varNameArray[ctIndex].protocol, TAG_BLACKLIST, varNameArray[ctIndex].node) == 1)
        {
            LOG_PRINT(error_e, "Communication error for channel '%d'\n", varNameArray[ctIndex].protocol);
            return -1;
        }

#endif
        LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);

        /* be sure that the variable is active */
        int SynIndex = -1;

        /* get the variable address from syncrovector*/
        if (CtIndex2SynIndex(ctIndex, &SynIndex) != 0)
        {
            /* if not exist this variable into the syncro vector, check if exist his headblock, and get the its CtIndex */
            int myCtIndex = varNameArray[ctIndex].blockhead;
            if (ctIndex == myCtIndex || CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
            {
                if (activateVar(varNameArray[myCtIndex].tag) != 0)
                {
                    LOG_PRINT(error_e, "The variable %d - %s is not active and is not activable\n", myCtIndex, varNameArray[myCtIndex].tag);
                    return 1;
                }
                else
                {
                    if (CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
                    {
                        LOG_PRINT(error_e, "The variable %d - %s is not active and is not activable\n", myCtIndex, varNameArray[myCtIndex].tag);
                        return 1;
                    }
                }
            }
        }

        switch(varNameArray[ctIndex].type)
        {
        case uintab_e:
        case uintba_e:
        {
            unsigned short value2 = (unsigned short)((*(float*)value) * powf(10,decimal));
            LOG_PRINT(info_e, "'%s' decimal %d old %f new %d\n", varNameArray[ctIndex].tag, decimal, *(float*)value, value2);
            return writeToDb(ctIndex, &value2);
        }
            break;
        case intab_e:
        case intba_e:
        {
            short value2 = (short)((*(float*)value) * powf(10,decimal));
            LOG_PRINT(info_e, "'%s' decimal %d old %f new %d\n", varNameArray[ctIndex].tag, decimal, *(float*)value, value2);
            return writeToDb(ctIndex, &value2);
        }
            break;
        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
        {
            unsigned int value2 = (unsigned int)((*(float*)value) * powf(10,decimal));
            LOG_PRINT(info_e, "'%s' decimal %d old %f new %d\n", varNameArray[ctIndex].tag, decimal, *(float*)value, value2);
            return writeToDb(ctIndex, &value2);
        }
            break;
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
        {
            int value2 = (int)((*(float*)value) * powf(10,decimal));
            LOG_PRINT(info_e, "'%s' decimal %d old %f new %d\n", varNameArray[ctIndex].tag, decimal, *(float*)value, value2);
            return writeToDb(ctIndex, &value2);
        }
            break;
        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
        {
            float value2 = (float)((*(float*)value) /** powf(10,decimal)*/);
            LOG_PRINT(info_e, "'%s' decimal %d old %f new %f\n", varNameArray[ctIndex].tag, decimal, *(float*)value, value2);
            return writeToDb(ctIndex, &value2);
        }
            break;
        default:
            LOG_PRINT(warning_e, "Unkown %d '%s' decimal %d value %x\n", varNameArray[ctIndex].type, varNameArray[ctIndex].tag, decimal, *(int*)value);
            return writeToDb(ctIndex, value);
            break;
        }
    }
    else
    {
        LOG_PRINT(warning_e, "DONE ctIndex %d value %X\n", ctIndex, *((int*)value));
        return writeToDb(ctIndex, value);
    }
}

/**
 * @brief perform a read request
 */
int readVar(const char * varname, void * value)
{
    int ctIndex;

    LOG_PRINT(verbose_e, "Reading '%s'\n", varname);
    /* calculate the database offset and the syncro vector offset */
    if (Tag2CtIndex(varname, &ctIndex) == 0)
    {
        if (readFromDb(ctIndex, value) == 0)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else
    {
        return 0;
    }
}

void writeVarInQueueByCtIndex(const int ctIndex, const int value)
{
    write_queue_elem_t * queue_elem = (write_queue_elem_t *)calloc(1, sizeof(write_queue_elem_t));
    queue_elem->next = NULL;
    queue_elem->ctIndex = ctIndex;
    queue_elem->value = value;
    pthread_mutex_lock(&write_queue_mutex);
    {
        // put item to queue tail
        if (queue_tail == NULL || queue_head == NULL)
        {
            queue_head = queue_tail = queue_elem;
        }
        else
        {
            queue_tail->next = queue_elem;
            queue_tail = queue_elem;
        }
    }
    pthread_mutex_unlock(&write_queue_mutex);
    sem_post(&theWritingSem);
}

void writeVarQueuedByCtIndex(void)
{
    int do_post = 0;
    pthread_mutex_lock(&write_queue_mutex);
    {
        // get item from queue head
        if (queue_head != NULL)
        {
            if (writeVarByCtIndex_nowait(queue_head->ctIndex, &queue_head->value) == 0) {
                write_queue_elem_t * queue_elem = queue_head;
                queue_head = queue_head->next;
                if (queue_head == NULL)
                {
                    queue_tail = NULL;
                }
                free(queue_elem);
            } else {
                // rimane in coda
                do_post = 1;
            }
        }
    }
    pthread_mutex_unlock(&write_queue_mutex);
    if (do_post) {
        sem_post(&theWritingSem);
    }
}

/**
 * @brief perform a write request
 */
int writeVarByCtIndex(const int ctIndex, void * value)
{
    int SynIndex = 0;

    if (prepareWriteVarByCtIndex(ctIndex, value, &SynIndex,1,1) == DONE || GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK))
    {
        setStatusVarBySynIndex(SynIndex, BUSY);

        /* update the write flag */
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
#ifdef ENABLE_MUTEX
        pthread_mutex_lock(&sync_send_mutex);
#endif
        CLR_SYNCRO_FLAG(SynIndex, PREPARE_MASK);
        SET_SYNCRO_FLAG(SynIndex, WRITE_MASK);
        /*rise write interrupt*/
        SET_WORD_FROM_WORD(WRITE_IRQ_ON, IOSyncroAreaO, WRITE_IRQ_VAR);
#ifdef ENABLE_MUTEX
        pthread_mutex_unlock(&sync_send_mutex);
#endif
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
        return 0;
    }

    return 1;
}

int writeVarByCtIndex_nowait(const int ctIndex, void * value)
{
    int SynIndex = 0;

    if (prepareWriteVarByCtIndex(ctIndex, value, &SynIndex,0,1) == DONE || GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK))
    {
        setStatusVarBySynIndex(SynIndex, BUSY);

        /* update the write flag */
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
#ifdef ENABLE_MUTEX
        pthread_mutex_lock(&sync_send_mutex);
#endif
        CLR_SYNCRO_FLAG(SynIndex, PREPARE_MASK);
        SET_SYNCRO_FLAG(SynIndex, WRITE_MASK);
        /*rise write interrupt*/
        SET_WORD_FROM_WORD(WRITE_IRQ_ON, IOSyncroAreaO, WRITE_IRQ_VAR);
#ifdef ENABLE_MUTEX
        pthread_mutex_unlock(&sync_send_mutex);
#endif
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
        return 0;
    }

    return 1;
}

int writeVar(const char * varname, void * value)
{
    int CtIndex = 0;
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return -1;
    }
    return writeVarByCtIndex(CtIndex, value);
}

/**
 * @brief perform a block write request
 */
int writePending()
{
    unsigned int SynIndex;
    for (SynIndex = 0; SynIndex < SyncroAreaSize; SynIndex++)
    {
        if (GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK))
        {
            setStatusVarBySynIndex(SynIndex, BUSY);
            LOG_PRINT(info_e, "Writing the PREPARE FLAG %d -> %X\n", SynIndex, pIOSyncroAreaO[SynIndex]);
            CLR_SYNCRO_FLAG(SynIndex, PREPARE_MASK);
            SET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK);
            LOG_PRINT(info_e, "Writing the PREPARE FLAG %d -> %X\n", SynIndex, pIOSyncroAreaO[SynIndex]);
        }
    }
    return 0;
}

/**
 * @brief perform a block write request
 */
int writePendingInorder()
{
    unsigned int SynIndex;
    for (SynIndex = 0; SynIndex < SyncroAreaSize; SynIndex++)
    {
        if (GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK))
        {
            setStatusVarBySynIndex(SynIndex, BUSY);
            LOG_PRINT(info_e, "Clear the PREPARE FLAG %d -> %X\n", SynIndex, pIOSyncroAreaO[SynIndex]);
            CLR_SYNCRO_FLAG(SynIndex, PREPARE_MASK);
            SET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK);
            LOG_PRINT(info_e, "Writing the RCP_WRITE FLAG %d -> %X\n", SynIndex, pIOSyncroAreaO[SynIndex]);
        }
    }
    /*rise write interrupt*/
    SET_WORD_FROM_WORD(WRITE_IRQ_ON, IOSyncroAreaO, WRITE_IRQ_VAR);
    LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
    return 0;
}

char prepareFormattedVarByCtIndex(const int ctIndex, char * formattedVar)
{
    void * value = NULL;

    short int var_int;
    short unsigned int var_uint;
    int var_dint;
    unsigned int var_udint;
    float var_float;
    BYTE var_bit;

    int decimal = 0;

    decimal = getVarDecimal(ctIndex);

    if (decimal > 0 || varNameArray[ctIndex].decimal > 4)
    {
        var_float = atof(formattedVar);
        value = &var_float;
        LOG_PRINT(info_e,"decimal %d [from %d] -> value %f\n", decimal, varNameArray[ctIndex].decimal, var_float);
    }
    else
    {
        switch(CtIndex2Type(ctIndex))
        {
        case intab_e:
        case intba_e:
            var_int = atoi(formattedVar);
            value = &var_int;
            break;
        case uintab_e:
        case uintba_e:
            var_uint = atoi(formattedVar);
            value = &var_uint;
            LOG_PRINT(info_e,"UINT %s = %d\n", varNameArray[ctIndex].tag, var_uint);
            break;
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
            var_dint = atoi(formattedVar);
            value = &var_dint;
            LOG_PRINT(info_e,"DINT %s = %d\n", varNameArray[ctIndex].tag, var_dint);
            break;
        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
            var_udint = atoi(formattedVar);
            value = &var_udint;
            LOG_PRINT(info_e,"UDINT %s = %d\n", varNameArray[ctIndex].tag, var_udint);
            break;
        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
            var_float = atof(formattedVar);
            value = &var_float;
            LOG_PRINT(info_e,"FLOAT %s = %f\n", varNameArray[ctIndex].tag, var_float);
            break;
        default:
            var_bit = (atoi(formattedVar) == 1);
            value = &var_bit;
            LOG_PRINT(info_e,"BIT %s = %d\n", varNameArray[ctIndex].tag, var_bit);
            break;
        }
    }

    switch(prepareWriteVarByCtIndex(ctIndex, value, NULL,1,1))
    {
    case DONE:
        LOG_PRINT(info_e,"################### Prepared VAR: %s = %s\n", varNameArray[ctIndex].tag, formattedVar);
        return DONE;
    case ERROR:
        LOG_PRINT(error_e,"status ERROR\n");
        return ERROR;
    case BUSY:
        LOG_PRINT(info_e,"status BUSY\n");
        return BUSY;
    default:
        return ERROR;
    }
}

char prepareFormattedVar(const char * varname, char * formattedVar)
{
    int ctIndex;
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable '%s'\n", varname);
        return ERROR;
    }
    return prepareFormattedVarByCtIndex(ctIndex, formattedVar);
}

/**
 * @brief prepare all the data in order to send a write request for the variable varname
 * to perform the write request, you need to enable the write flag.
 */

char prepareWriteVarByCtIndex(const int ctIndex, void * value, int * SynIndex, int dowait, int formatted)
{
    int mySynIndex = 0;
    int *pSynIndex = NULL;

    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return ERROR;
    }

#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByCtIndex(ctIndex) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected\n");
        return ERROR;
    }
#endif
    if (SynIndex == NULL)
    {
        pSynIndex = &mySynIndex;
    }
    else
    {
        pSynIndex = SynIndex;
    }

    LOG_PRINT(info_e, "Writing '%d'\n", ctIndex);
    /* if the variable is already active, clear the reading flag */
    if (CtIndex2SynIndex(ctIndex, pSynIndex) == 0)
    {
        int count = 0, still_writing = 1;
        /* waiting for the variable is not in writing mode */
        do
        {
            if ((GET_SYNCRO_FLAG(*pSynIndex, WRITE_MASK) == 1 || pIOSyncroAreaI[*pSynIndex] == 1))
            {
                if (dowait)
                {
                    LOG_PRINT(info_e, "The variable '%d' is still in writing.\n", ctIndex);
                    usleep(1000 * IOLAYER_PERIOD_ms);
                    checkSynIndexWriting(*pSynIndex);
                    count ++;
                }
            }
            else
            {
                still_writing = 0;
            }
        } while (still_writing && dowait && count < RETRY_NB);
        if (still_writing || count > RETRY_NB)
        {
            return BUSY;
        }
        LOG_PRINT(info_e, "Clear reading flag  pIOSyncroAreaO[%d] '%X'\n",*pSynIndex, pIOSyncroAreaO[*pSynIndex]);
        CLR_SYNCRO_FLAG(*pSynIndex, READ_MASK);
        LOG_PRINT(info_e, "Clear reading flag  pIOSyncroAreaO[%d] '%X'\n",*pSynIndex, pIOSyncroAreaO[*pSynIndex]);
    }
    /* if the variable is not active, active it */
    else
    {
        if (addSyncroElementbyIndex(varNameArray[ctIndex].tag, ctIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot addSyncroElement\n");
            return ERROR;
        }
        if (CtIndex2SynIndex(ctIndex, pSynIndex) != 0)
        {
            LOG_PRINT(error_e, "Cannot find the variable '%d'' into the syncro vector\n", ctIndex);
            return ERROR;
        }
    }

    if (*pSynIndex < 0)
    {
        LOG_PRINT(error_e, "cannot extract SynIndex\n");
        return ERROR;
    }

    /* update the value into the Data area */
    if (formatted == 1)
    {
        if (formattedWriteToDb(ctIndex, value) != 0)
        {
            LOG_PRINT(error_e, "formattedWriteToDb\n");
            return ERROR;
        }
    }
    else
    {
        if (writeToDb(ctIndex, value) != 0)
        {
            LOG_PRINT(error_e, "formattedWriteToDb\n");
            return ERROR;
        }
    }

    LOG_PRINT(info_e, "Set prepare flag  pIOSyncroAreaO[%d] '%X' CtIndex %d - 0x%X\n",*pSynIndex, pIOSyncroAreaO[*pSynIndex], ctIndex, ctIndex);
    SET_SYNCRO_FLAG(*pSynIndex, PREPARE_MASK);

    return DONE;
}

char prepareWriteVar(const char * varname, void * value, int * SynIndex)
{
    int CtIndex = 0;
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return -1;
    }
    return prepareWriteVarByCtIndex(CtIndex, value, SynIndex, 1, 1);
}

/**
 * @brief prepare all the data in order to send a write request for the variable varname
 * to perform the write request for its blockhead, you need to enable the write flag.
 */
char prepareWriteBlock(const char * varname, void * value, int * SynIndex)
{
    int CtIndex = -1;
    int mySynIndex = 0;
    int *pSynIndex = NULL;
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected\n");
        return ERROR;
    }
#endif
    if (SynIndex == NULL)
    {
        pSynIndex = &mySynIndex;
    }
    else
    {
        pSynIndex = SynIndex;
    }

    LOG_PRINT(info_e, "Writing '%s'\n", varname);
    /* if the variable is already active, clear the reading flag */
    if (Tag2SynIndex(varname, pSynIndex) == 0)
    {
        if (SynIndex2CtIndex(*pSynIndex, &CtIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot extract ctIndex\n");
            return ERROR;
        }
        /* waiting for the variable is not in writing mode */
        int count = 0;
        while ((GET_SYNCRO_FLAG(*pSynIndex, WRITE_MASK) == 1 ||  pIOSyncroAreaI[*pSynIndex] == 1) && count < RETRY_NB)
        {
            LOG_PRINT(info_e, "The variable '%s' is still in writing.\n", varname);
            usleep(1000 * IOLAYER_PERIOD_ms);
            localCheckWriting();
            count++;
        }
        if (count > RETRY_NB)
        {
            return BUSY;
        }
        LOG_PRINT(info_e, "Clear reading flag  pIOSyncroAreaO[%d] '%X'\n",*pSynIndex, pIOSyncroAreaO[*pSynIndex]);
        CLR_SYNCRO_FLAG(*pSynIndex, READ_MASK);
        LOG_PRINT(info_e, "Clear reading flag  pIOSyncroAreaO[%d] '%X'\n",*pSynIndex, pIOSyncroAreaO[*pSynIndex]);
    }
    /* if the variable is not active, active it */
    else
    {
        char blockhead[TAG_LEN] = "";
        int retval;
        retval = isBlockActive(varname, blockhead);
        if (retval == 1)
        {
            LOG_PRINT(warning_e, "The variable '%s' come from a block already active\n", varname);
            if (SynIndex != NULL && Tag2SynIndex(blockhead, pSynIndex) != 0)
            {
                LOG_PRINT(error_e, "cannot extract SynIndex for variable '%s'\n", varname);
                return ERROR;
            }
        }
        else if (retval == 0)
        {
            if (addSyncroElement(blockhead, &CtIndex) != 0)
            {
                LOG_PRINT(error_e, "cannot addSyncroElement\n");
                return ERROR;
            }
            if (CtIndex2SynIndex(CtIndex, pSynIndex) != 0)
            {
                LOG_PRINT(error_e, "Cannot find the variable '%d'' into the syncro vector\n", CtIndex);
                return ERROR;
            }
        }
    }

    if (SynIndex != NULL && *pSynIndex < 0)
    {
        LOG_PRINT(error_e, "cannot extract SynIndex for variable '%s'\n", varname);
        return ERROR;
    }

    /* update the value into the Data area */
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex\n");
        return ERROR;
    }
    if (formattedWriteToDb(CtIndex, value) != 0)
    {
        LOG_PRINT(error_e, "formattedWriteToDb\n");
        return ERROR;
    }

    LOG_PRINT(info_e, "Set prepare flag  pIOSyncroAreaO[%d] '%X' CtIndex %d - 0x%X\n",*pSynIndex, pIOSyncroAreaO[*pSynIndex], CtIndex, CtIndex);
    SET_SYNCRO_FLAG(*pSynIndex, PREPARE_MASK);

    return DONE;
}

/**
 * @brief re-checks the write flags in queue
 */
static void localCheckWriting(void)
{
    unsigned int i;
    /* scan all the syncro array */
    for (i = 0; i < SyncroAreaSize; i++)
    {
        LOG_PRINT(verbose_e, "%X vs %X (%X vs %X) %d\n", GET_SYNCRO_FLAG(i, WRITE_MASK), pIOSyncroAreaI[i], pIOSyncroAreaO[i], pIOSyncroAreaI[i], i);
        checkSynIndexWriting(i);
    }
}

int setFormattedVarByCtIndex(const int ctIndex, char * formattedVar)
{
    void * value = NULL;

    short int var_int;
    short unsigned int var_uint;
    int var_dint;
    unsigned int var_udint;
    float var_float;
    BYTE var_bit;
    int decimal = 0;

    /* be sure that the variable is active */
    int SynIndex = -1;

    /* get the variable address from syncrovector*/
    if (CtIndex2SynIndex(ctIndex, &SynIndex) != 0)
    {
        /* if not exist this variable into the syncro vector, check if exist his headblock, and get the its CtIndex */
        int myCtIndex = varNameArray[ctIndex].blockhead;
        if (ctIndex == myCtIndex || CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
        {
            if (activateVar(varNameArray[myCtIndex].tag) != 0)
            {
                LOG_PRINT(error_e, "The variable %d - %s is not active and is not activable\n", myCtIndex, varNameArray[myCtIndex].tag);
                return 1;
            }
            else
            {
                if (CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
                {
                    LOG_PRINT(error_e, "The variable %d - %s is not active and is not activable\n", myCtIndex, varNameArray[myCtIndex].tag);
                    return 1;
                }
            }
        }
    }

    decimal = getVarDecimal(ctIndex);

    if (decimal > 0 || varNameArray[ctIndex].decimal > 4)
    {
        var_float = atof(formattedVar);
        value = &var_float;
        LOG_PRINT(info_e,"FLOAT %d = %f\n", ctIndex, var_float);
    }
    else
    {
        switch(CtIndex2Type(ctIndex))
        {
        case intab_e:
        case intba_e:
            var_int = atoi(formattedVar);
            value = &var_int;
            break;
        case uintab_e:
        case uintba_e:
            var_uint = atoi(formattedVar);
            value = &var_uint;
            break;
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
            var_dint = atoi(formattedVar);
            value = &var_dint;
            break;
        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
            var_udint = atoi(formattedVar);
            value = &var_udint;
            break;
        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
            var_float = atof(formattedVar);
            value = &var_float;
            break;
        default:
            var_bit = (atoi(formattedVar) == 1);
            value = &var_bit;
            break;
        }
    }
    if (writeVarByCtIndex(ctIndex, value) == 0)
    {
        LOG_PRINT(info_e,"################### %d = %s\n", ctIndex, formattedVar);
        return 0;
    }
    return 1;
}

int setFormattedVar(const char * varname, char * formattedVar)
{
    int ctIndex;
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable '%s'\n", varname);
        return 1;
    }
    return setFormattedVarByCtIndex(ctIndex, formattedVar);
}

/**
 * @brief perform a block write request
 */
int writeBlock(const char * varname)
{
    int SynIndex = -1;
    char blockhead[TAG_LEN];
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected\n");
        return 1;
    }
#endif

    /* extract the block head */
    isBlockActive(varname, blockhead);

    LOG_PRINT(info_e, "writing Block '%s' of variable '%s'\n", blockhead, varname);
    /* if the variable is into syncrovector */
    if (Tag2SynIndex(blockhead, &SynIndex) != 0)
    {
        int CtIndex = -1;
        if (addSyncroElement(blockhead, &CtIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot addSyncroElement\n");
            return 1;
        }
        if (CtIndex2SynIndex(CtIndex, &SynIndex) != 0)
        {
            LOG_PRINT(error_e, "Cannot find the variable '%d'' into the syncro vector\n", CtIndex);
            return 1;
        }
    }
    else
    {
        unsigned int i;
        setStatusVarBySynIndex(SynIndex, BUSY);

        /* update the write flag */
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
        SET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK);
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
        for (i = 0; i < SyncroAreaSize; i++)
        {
            if (GET_SYNCRO_FLAG(i, PREPARE_MASK))
            {
                LOG_PRINT(info_e, "Cleaning the PREPARE FLAG %d -> %X\n", i, pIOSyncroAreaO[i]);
                CLR_SYNCRO_FLAG(i, PREPARE_MASK);
                LOG_PRINT(info_e, "Cleaned the PREPARE FLAG %d -> %X\n", i, pIOSyncroAreaO[i]);
            }
        }
        /*rise write interrupt*/
        SET_WORD_FROM_WORD(WRITE_IRQ_ON, IOSyncroAreaO, WRITE_IRQ_VAR);
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
    }
    return 0;
}

/** @brief check if the block of variable 'varname' is already active
 * @param const char * varname : name of the variable
 * @param char * varblockhead : string where will be put the variable's block name
 * @return 1 block active
 * @return 0 block inactive
 */
int isBlockActiveByCtIndex(const int CtIndex, char * varblockhead)
{
    int CtIndexHead;

    CtIndexHead = (varNameArray[CtIndex].blockhead == 0) ? CtIndex : varNameArray[CtIndex].blockhead;

    if (CtIndexHead > 0 && strlen(varNameArray[CtIndexHead].tag))
    {
        strcpy(varblockhead, varNameArray[CtIndexHead].tag);
    }
    else
    {
        LOG_PRINT(error_e, "Cannot found the head block of '%s' ('%s', %d)\n", varNameArray[CtIndex].tag, varNameArray[CtIndexHead].tag, CtIndexHead);
    }

    int SynIndex = 0;
    if (CtIndex2SynIndex(CtIndexHead, &SynIndex) == 0)
    {
        LOG_PRINT(verbose_e, "Block is aready active var %s, block %s.\n", varNameArray[CtIndex].tag, varNameArray[CtIndexHead].tag);
        return 1;
    }
    else
    {
        LOG_PRINT(verbose_e, "Block is not active, activate it.\n");
        return 0;
    }
}

int isBlockActive(const char * varname, char * varblockhead)
{
    int CtIndex;
    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return -1;
    }

    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return -1;
    }
    return isBlockActiveByCtIndex(CtIndex, varblockhead);
}

char * mystrtok(char * string, char * token, const char * separator)
{
    char * p;
    int i;

    if (string == NULL)
    {
        if (token != NULL)
        {
            token[0] = '\0';
        }
        return NULL;
    }

    strcpy(token, string);
    p = strstr(token, separator);
    if (p != NULL)
    {
        *p = '\0';
    }
#if 1
    for (i = strlen(token) - 1; i > 0 && isspace(token[i]); i--);
    token[i + 1] = '\0';
    for (i = 0; (unsigned int) i < strlen(token) && isspace(token[i]); i++);
    if (i > 0)
    {
        strcpy(token, &(token[i]));
    }
#else
    sscanf(token, "%s", token);
#endif
    p = strstr(string, separator);
    if (p!= NULL)
    {
        p++;
    }
    return p;
}

/**
 * @brief get the block head and the block number of variable at position CtIndex
 * @param int CtIndex : variable position
 * @param char * varblockhead : string where will be put the variable's block name
 * @return > 0 number of block
 * @return < 0 if an error occured
 */
int getHeadBlock(int CtIndex, char * varblockhead)
{
    int i;
    i = varNameArray[CtIndex].blockhead;
    strcpy(varblockhead, varNameArray[i].tag);
    LOG_PRINT(verbose_e, "Returning block number %d\n", varNameArray[CtIndex].block);
    return varNameArray[CtIndex].block;
}

/**
 * @brief get the block head and the block number of variable at position CtIndex
 * @param const char * varname : name of the variable
 * @param char * varblockhead : string where will be put the variable's block name
 * @return > 0 number of block
 * @return < 0 if an error occured
 */
int getHeadBlockName(const char * varname, char * varblockhead)
{
    int CtIndex;

    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return -1;
    }

    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return -1;
    }
    return getHeadBlock(CtIndex, varblockhead);
}

#ifdef ENABLE_DEVICE_DISCONNECT
int isDeviceConnected(enum protocol_e protocol, int node)
{

    /*Broadcast address 0 must be always connected*/
    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "Internal variable\n");
        return 1;
    }
    if (node <= 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }

    return device_status[protocol][node] && !getErrorBit(protocol, error_blacklist_e, node);
}

int disconnectDevice(enum protocol_e protocol, int node)
{
    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "internal variable\n");
        return 0;
    }
    /*Broadcast address 0 cannot be disconnected*/
    if (node <= 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }
    if (protocol != prot_rtu_e && protocol != prot_tcp_e && protocol != prot_tcprtu_e)
    {
        LOG_PRINT(error_e, "Invalid protocol %d\n",protocol);
        return -1;
    }

    device_status[protocol][node] = 0;

    /*Deactivate already active variable in syncrotable*/
    LOG_PRINT(error_e, "################### Disconnecting %d - %d\n",protocol, node);

    unsigned int i;
    int SynIndex;
    int CtIndex;
    for (i = 0; i < SyncroAreaSize; i++)
    {
        CtIndex = (pIOSyncroAreaO[i] & ADDRESS_MASK);
        LOG_PRINT(verbose_e, "Candidate to removal CtIndex %d - [%x]\n", CtIndex, pIOSyncroAreaO[i]);

        if (! isDeviceConnectedByCtIndex(CtIndex))
        {
            if ( CtIndex2SynIndex(CtIndex, &SynIndex) == 0)
            {
                delSyncroElementByIndex(SynIndex);
                /*When an element gets deleted the search for element to be deleted
                  must restart from the beginning of the queu to be sure to not skip any moved up elem*/
                i=0;
                LOG_PRINT(verbose_e, "Deleted SynIndex %d\n", SynIndex);
            }
        }
    }
    return 0;
}

int setupConnectedDeviceByName(const char * protocol, int node)
{
    enum protocol_e protocol_e;

    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(info_e, "Internal variable\n");
        return 0;
    }
    /*Broadcast address 0 must be always connected*/
    if (node < 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }

    if (strcmp(protocol, TAG_RTU) == 0)
    {
        protocol_e = prot_rtu_e;
    }
    else if (strcmp(protocol, TAG_TCP) == 0)
    {
        protocol_e = prot_tcp_e;
    }
    else if (strcmp(protocol, TAG_TCPRTU) == 0)
    {
        protocol_e = prot_tcprtu_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol '%s', allowed are '%s' - '%s' - '%s'\n", protocol, TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }

    return setupConnectedDevice(protocol_e, node);
}

int setupConnectedDevice(enum protocol_e protocol, int node)
{
    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(info_e, "Internal variable\n");
        return 0;
    }
    /*Broadcast address 0 must be always connected*/
    if (node < 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }

    device_status[protocol][node] = 1;
    return 0;
}

int connectDevice(enum protocol_e protocol, int node)
{
    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "internal variable\n");
        return 0;
    }
    /*Broadcast address 0 is always connected by default*/
    if (node <= 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }
    if (protocol != prot_rtu_e && protocol != prot_tcp_e && protocol != prot_tcprtu_e)
    {
        LOG_PRINT(error_e, "Invalid protocol %d\n",protocol);
        return -1;
    }
    device_status[protocol][node] = 1;

    /*Add activation of eventually disconnected variables beloging to P, S categories*/
    int i;
    for (i = 1; i < DB_SIZE_ELEM +1 ; i++)
    {
        if ((varNameArray[i].node) == node && varNameArray[i].active == 1)
        {
            int SynIndex = 0;
            int CtIndex = 0;
            char blockhead[TAG_LEN] = "";

            LOG_PRINT(verbose_e, "ELEMENT %d is reading\n", i);

            /* check if the block of the variable is already active */
            if (isBlockActive(varNameArray[i].tag, blockhead) == 0)
            {
                /* looking the variable index from the crosstable structure */
                /* insert it into the syncrovector */
                if (addSyncroElement(blockhead, &CtIndex) == 0)
                {
                    if (CtIndex2SynIndex(CtIndex, &SynIndex) != 0)
                    {
                        LOG_PRINT(error_e, "Cannot add the block to the syncroelement\n");
                        /*failed reconnection*/
                        return 1;
                    }
                    /* enable it in reading */
                    SET_SYNCRO_FLAG(SynIndex, READ_MASK);
                    LOG_PRINT(verbose_e, "Set reading flag\n");
                }
                else
                {
                    LOG_PRINT(error_e, "Cannot add the block to the syncroelement\n");
                }
            }
            else
            {
                LOG_PRINT(warning_e, "The variable '%s' come from a block already active\n", varNameArray[i].tag);
            }
        }
    }
    /*Add activation for page activated variables*/
    DeviceReconnected = 1;
    /*ok*/
    return 0;
}

int disconnectDeviceByVarname(const char * varname)
{
    int CtIndex;

    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return -1;
    }

    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return -1;
    }
    return disconnectDevice(varNameArray[CtIndex].protocol, varNameArray[CtIndex].node);
}

int connectDeviceByVarname(const char * varname)
{
    int CtIndex;

    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return -1;
    }

    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return -1;
    }

    return connectDevice(varNameArray[CtIndex].protocol, varNameArray[CtIndex].node);
}

int isDeviceConnectedByVarname(const char * varname)
{
    int CtIndex;

    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return -1;
    }

    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector - Ctindex %d\n", varname, CtIndex);
        return -1;
    }
    if (varNameArray[CtIndex].node <= 0 || varNameArray[CtIndex].node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d for variable '%s' [%d])\n", varNameArray[CtIndex].node, 1, MAX_DEVICE_NB, varNameArray[CtIndex].tag, CtIndex);
        return -1;
    }

    return isDeviceConnected(varNameArray[CtIndex].protocol, varNameArray[CtIndex].node);
}

int isDeviceConnectedByCtIndex(int CtIndex)
{
    if(CtIndex < 0 || CtIndex > DB_SIZE_ELEM )
    {
        LOG_PRINT(error_e, "invalid CtIndex %d.\n", CtIndex);
        return -1;
    }
    if (varNameArray[CtIndex].node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "Internal variable\n");
        return 1;
    }
    if (varNameArray[CtIndex].node <= 0 || varNameArray[CtIndex].node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d for variable '%s' [%d])\n", varNameArray[CtIndex].node, 1, MAX_DEVICE_NB, varNameArray[CtIndex].tag, CtIndex);
        return -1;
    }
    return isDeviceConnected(varNameArray[CtIndex].protocol, varNameArray[CtIndex].node);
}
#endif

/**
 * @brief comunicate to the communicatuion PLC that this variable must be read untill the page is active
 */
int activateVar(const char * varname)
{
    int SynIndex = 0;
    int CtIndex = 0;
    char blockhead[TAG_LEN] = "";
    int retval;

    LOG_PRINT(info_e, "Activating '%s'\n", varname);

#if 0
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected\n");
        return 1;
    }
#endif
#endif
    /* check if the block of the variable is already active */
    retval = isBlockActive(varname, blockhead);
    if (retval == 1)
    {
        LOG_PRINT(warning_e, "The variable '%s' come from a block already active\n", varname);
        return 0;
    }
    else if (retval == 0)
    {
        /* looking the variable index from the crosstable structure */
        /* insert it into the syncrovector */
        if (addSyncroElement(blockhead, &CtIndex) == 0)
        {
            if (CtIndex2SynIndex(CtIndex, &SynIndex) != 0)
            {
                LOG_PRINT(error_e, "Cannot find the variable '%d'' into the syncro vector\n", CtIndex);
                return 1;
            }
            LOG_PRINT(verbose_e, "Set reading flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
            /* enable it in reading */
            SET_SYNCRO_FLAG(SynIndex, READ_MASK);
            //			varNameArray[CtIndex].pageid = pageid;
            LOG_PRINT(verbose_e, "Set reading flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
            varNameArray[CtIndex].visible = 1;
            return 0;
        }
        LOG_PRINT(error_e, "Cannot Activate '%s'\n", varname);
    }
    LOG_PRINT(error_e, "IMPOSSIBLE RET VALUE '%d'\n", retval);
    return 1;
}

/**
 * @brief comunicate to the communicatuion PLC that this variable is not necessary to be read
 */
int deactivateVar(const char * varname)
{
    char blockhead[TAG_LEN] = "";
    int retval;

#if 0
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) == 1)
#endif
#endif
    {
        /* check if the block of the variable is already active */
        retval = isBlockActive(varname, blockhead);
        if (retval == 1)
        {
            LOG_PRINT(warning_e, "The variable '%s' come from a block already active\n", varname);
            return 0;
        }
        else if (retval == 0)
        {

            LOG_PRINT(info_e, "Deactivating block '%s' cause variable '%s' is deactivated\n", blockhead, varname);
            /* looking and delete the variable from the syncrotable */
            if (delSyncroElement(blockhead) == 0)
            {
#if 0
                for (i = 0; i < DB_SIZE_ELEM; i++)
                {
                    if (Tag2CtIndex(varname, &CtIndex) == 0)
                    {
                        varNameArray[CtIndex].pageid = 0;
                    }
                }
#endif
                int CtIndex = -1;
                if (Tag2CtIndex(varname, &CtIndex) != 0)
                {
                    return 1;
                }
                varNameArray[CtIndex].visible = 0;
                return 0;
            }
        }
        return 1;
    }
    return 1;
}

/**
 * @brief get the actual status.
 * The status could be:
 * - ERROR:  error flag is set to 1
 * - DONE:   error flag is set to 0
 * - BUSY:   error flag is set to 0
 */
char getStatusVarByCtIndex(int CtIndex, char * msg)
{
    char Status = DONE;
    int myCtIndex = CtIndex;

    if(myCtIndex < 0 || myCtIndex > DB_SIZE_ELEM )
    {
        LOG_PRINT(error_e, "invalid CtIndex %d.\n", myCtIndex);
        Status = ERROR;
        return Status;
    }
#ifndef CHECK_SYNCRO
#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&sync_recv_mutex);
#endif
#else
    int SynIndex;
    /* get the variable address from syncrovector*/
    if (CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
    {
        /* if not exist this variable into the syncro vector, check if exist his headblock, and get the its CtIndex */
        myCtIndex = varNameArray[CtIndex].blockhead;
        if (CtIndex == myCtIndex || CtIndex2SynIndex(myCtIndex, &SynIndex) != 0)
        {
            Status = ERROR;
            LOG_PRINT(error_e, "The variable %d - %s is not active.\n", myCtIndex, varNameArray[myCtIndex].tag);
            if (msg != NULL)
            {
                strcpy(msg, VAR_UNKNOWN);
            }
            return Status;
        }
    }
    /* force a status vector update before read the status */
    checkSynIndexWriting(SynIndex);

#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&sync_recv_mutex);
#endif

    /* check if is still in read or write */
    if (pIOSyncroAreaI[SynIndex] == 1 /* busy write */|| pIOSyncroAreaI[SynIndex] == 2 /* busy read */)
    {
        Status = BUSY;
        //LOG_PRINT(error_e, "CtIndex '%d' SynIndex '%d' Syncro %c BUSY\n", myCtIndex, SynIndex, pIOSyncroAreaI[SynIndex]);
        if (msg != NULL)
        {
            strcpy(msg, VAR_PROGRESS);
        }
    }
    else if (GET_SYNCRO_FLAG(SynIndex, WRITE_MASK) == 1 || GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK) == 1 || GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK) == 1)
    {
        Status = BUSY;
        //LOG_PRINT(error_e, "CtIndex '%d' SynIndex '%d' Syncro %c BUSY\n", myCtIndex, SynIndex, pIOSyncroAreaI[SynIndex]);
        if (msg != NULL)
        {
            strcpy(msg, VAR_PROGRESS);
        }
    }
    /* the variable is not into the syncro vector */
    else
#endif
    {
        Status = pIODataStatusAreaI[myCtIndex];
        if (Status == DONE)
        {
            LOG_PRINT(verbose_e, "CtIndex '%d' Status %d DONE\n", myCtIndex, Status);
            if (msg != NULL)
            {
                strcpy(msg, "");
            }
        }
        else if (Status == ERROR)
        {
            LOG_PRINT(error_e, "CtIndex '%d' Status %d ERROR\n", myCtIndex, Status);
            if (msg != NULL)
            {
                strcpy(msg, VAR_COMMUNICATION);
            }
        }
        else
        {
            LOG_PRINT(error_e, "CtIndex '%d' Status %d UNKNOWN\n", myCtIndex, Status);
            Status = ERROR;
            if (msg != NULL)
            {
                strcpy(msg, VAR_UNKNOWN);
            }
        }
    }

#ifdef ENABLE_MUTEX
    pthread_mutex_unlock(&sync_recv_mutex);
#endif

    return Status;
}

/**
 * @brief set the actual status.
 * The status could be:
 * - ERROR:  error flag is set to 1
 * - DONE:   error flag is set to 0
 * - BUSY:   error flag is set to 0
 */
int setStatusVar(const char * varname, char Status)
{
    int CtIndex;

    /* get the variable address from syncrovector */
    if (Tag2CtIndex(varname, &CtIndex) == 0)
    {
        if (Status == BUSY)
        {
            pIODataStatusAreaO[CtIndex] = Status;
        }
        else
        {
            pIODataStatusAreaO[CtIndex] = Status;
        }
        LOG_PRINT(info_e, "Status '%s' is '%c'\n", varname, Status);
        return 0;
    }
    else
    {
        LOG_PRINT(error_e, "Cannot set the status '%c' the variable '%s'\n", Status, varname);
        return 1;
    }
}

/**
 * @brief set the actual status.
 * The status could be:
 * - ERROR:  error flag is set to 1
 * - DONE:   error flag is set to 0
 * - BUSY:   error flag is set to 0
 */
int setStatusVarBySynIndex(int SynIndex, char Status)
{
    int CtIndex;
    int retval = 0;

    /* get the variable address from syncrovector */
    if (SynIndex2CtIndex(SynIndex, &CtIndex) == 0)
    {
#ifdef ENABLE_MUTEX
        pthread_mutex_lock(&sync_send_mutex);
#endif
        pIODataStatusAreaO[CtIndex] = Status;
#ifdef ENABLE_MUTEX
        pthread_mutex_unlock(&sync_send_mutex);
#endif
        LOG_PRINT(info_e, "Status '%d' is '%c'\n", CtIndex, Status);
        retval = 0;
    }
    else
    {
        LOG_PRINT(error_e, "Cannot set the status '%c' the variable '%d'\n", Status, CtIndex);
        retval = 1;
    }
    return retval;
}

int getString(const char* varname, int size, char * string)
{
    int ctIndex = -1;
    unsigned short value;
    int i;
#if 0
    if (getFormattedVar(varname, &value) == false)
    {
        LOG_PRINT(error_e, "Cannot read the variable '%s'\n", varname);
        return false;
    }
#else
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected\n");
        return 1;
    }
#endif

    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        return 1;
    }

    LOG_PRINT(info_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);

    if (varNameArray[ctIndex].decimal > 0)
    {
        LOG_PRINT(error_e, "Invalid type\n");
        return 1;
    }
    if(CtIndex2Type(ctIndex) == uintab_e || CtIndex2Type(ctIndex) == uintba_e)
    {
        if (readFromDb(ctIndex, &value) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            return 1;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            break;
        case ERROR:
            return 1;
            break;
        case DONE:
            break;
        default:
            return 1;
            break;
        }
    }
    else
    {
        LOG_PRINT(error_e, "Invalid type\n");
        return 1;
    }

#endif
    LOG_PRINT(info_e, "get var name '%s' size %d\n", varname, size);
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "Cannot extract ctIndex for variable %s\n", varname);
        return 1;
    }
    for (i = 0; i < size; i++)
    {
        if (i%2==0)
        {
            if (readFromDb(ctIndex, &value) != 0)
            {
                LOG_PRINT(error_e, "Cannot read the variable at ctIndex %d\n", ctIndex);
                return 1;
            }
            LOG_PRINT(info_e, "%d value[%d] = %d\n",ctIndex, i, value);
            string[i] = value & 0xFF;
            string[i+1] = (value >> 8) & 0xFF;
            ctIndex++;
        }
    }
    string[size]='\0';
    LOG_PRINT(info_e, "get string '%s'\n", string);
    return 0;
}

int setString(const char* varname, int size, char * string)
{
    char tag[TAG_LEN];
    int ctIndex = -1;
    unsigned short value;
    int i;

    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "Cannot extract ctIndex for variable %s\n", varname);
        return 1;
    }

    for (i = 0; i < size; i++)
    {
        if (i%2==0)
        {
            if (CtIndex2Tag(ctIndex, tag) != 0)
            {
                LOG_PRINT(error_e, "Cannot extract tag name from variable at ctIndex %d\n", ctIndex);
                return 1;
            }
            value = (string[i] | (string[i+1] << 8));
            LOG_PRINT(info_e, "'%c' '%c' -> %x %x -> %x\n", string[i], string[i+1], string[i], string[i+1], value);
            if (prepareWriteBlock(tag, &value, NULL) != DONE)
            {
                LOG_PRINT(error_e, "Cannot prepare the write value into variable %s\n", tag);
                return 1;
            }
            ctIndex++;
        }
    }
    if (writeBlock(varname) != 0)
    {
        LOG_PRINT(error_e, "Cannot perform the write of the block %s\n", varname);
        return 0;
    }
    LOG_PRINT(info_e, "set string '%s'\n", string);
    return 0;
}

int deleteUnusedSynIndex(int SynIndex)
{
    int CtIndex = -1;
    /* if the element is not yet active, delete it */
    if (
            GET_SYNCRO_FLAG(SynIndex, WRITE_MASK)     == 0  && GET_SYNCRO_FLAG(SynIndex, READ_MASK)            == 0 &&
            GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK)   == 0  && GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK)     == 0 &&
            GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK) == 0  && GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_RCP_MASK) == 0 &&
            pIOSyncroAreaI[SynIndex] == 0
            )
    {
        CtIndex = (pIOSyncroAreaO[SynIndex] & ADDRESS_MASK);
        LOG_PRINT(info_e, "The variable %d CtIndex %d state is READ %d WRITE %d PREPARE %d MULTI_WRITE %d WRITE_RCP  %d pIOSyncroAreaI[%d] %d\n",
                  SynIndex,
                  CtIndex,
                  GET_SYNCRO_FLAG(SynIndex, READ_MASK),
                  GET_SYNCRO_FLAG(SynIndex, WRITE_MASK),
                  GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK),
                  GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK),
                  GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK),
                  SynIndex,
                  pIOSyncroAreaI[SynIndex]);
        LOG_PRINT(info_e, "Clear the write flag for the variable '%s' WRITE_MASK %d SyncroArea %X - %X index: %d CT %d\n", varNameArray[CtIndex].tag, GET_SYNCRO_FLAG(SynIndex, WRITE_MASK), pIOSyncroAreaI[SynIndex], *(unsigned char *)(&(pIOSyncroAreaI[SynIndex])), SynIndex, CtIndex);
        if (CtIndex < 0 || CtIndex > DB_SIZE_ELEM)
        {
            LOG_PRINT(error_e, "Invalid CtIndex = %d for variable '%s'.\n", CtIndex, varNameArray[CtIndex].tag);
            LOG_PRINT(info_e, "Clean the inactive variable %d\n", SynIndex);
            delSyncroElementByIndex(SynIndex);
            return 1;
        }
        else
        {
            /* if is 'S' variable, if is 'P' variable, if it is enabled into the actual page, set the reading flag */
            if (
        #if (MIR_REG_NB + MIR_BIT_NB)
                    (CtIndex * 4 > MIR_REG_BASE_BYTE && CtIndex * 4 < MIR_REG_BASE_BYTE + MIR_REG_SIZE_BYTE) ||
        #endif
                    varNameArray[CtIndex].visible == 1 ||
                    varNameArray[CtIndex].active == 1
                    )
            {
                //delSyncroElementByIndex(SynIndex);
                //activateVar(varNameArray[CtIndex].tag);
                char blockhead[TAG_LEN];
                /* if it is a variable in a block already active, remove it from the queue */
                if (isBlockActiveByCtIndex(CtIndex, blockhead) && strcmp(blockhead, varNameArray[CtIndex].tag) != 0)
                {
                    delSyncroElementByIndex(SynIndex);
                    LOG_PRINT(info_e, "DELETE SYNINDEX %d\n", SynIndex);
                }
                /* if it isn't a variable into a block or if it is a head block, reactivate it */
                else
                {
                    LOG_PRINT(verbose_e, "Set the read flag for the variable head '%s' var '%s'\n", blockhead, varNameArray[CtIndex].tag);
                    SET_SYNCRO_FLAG(SynIndex, READ_MASK);
                    LOG_PRINT(verbose_e, "Set the read flag for the variable '%s'\n", varNameArray[CtIndex].tag);
                }
            }
            else
            {
                LOG_PRINT(info_e, "Clean the inactive variable %d\n", SynIndex);
                delSyncroElementByIndex(SynIndex);
                return 0;
            }
        }
    }
    return 0;
}

void checkTagWriting(const char * varname)
{
    int SynIndex;

    if (strlen(varname) == 0)
    {
        LOG_PRINT(error_e, "empty variable '%s'\n", varname);
        return;
    }

    if (Tag2SynIndex(varname, &SynIndex) != 0)
    {
        char blockhead[TAG_LEN] = "";
        isBlockActive(varname, blockhead);
        if (Tag2SynIndex(blockhead, &SynIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot found the Syn index of variable '%s'\n", varname);
            return;
        }
        else
        {
            LOG_PRINT(info_e, "the variable '%s' is a variable of the block '%s'\n", varname, blockhead);
        }
    }
    // The return value does not need to be checked it is needed by checkwriting when looping on the whole syncro area
    checkSynIndexWriting(SynIndex);
}

unsigned short int getCommunicationEngineMainRevision(void)
{
    unsigned short int value;
    GET_WORD_FROM_WORD(value, IOSyncroAreaI, PLC_MAIN_REV);
    LOG_PRINT_NO_INFO(verbose_e, "@@@@@@@@@@@ Main Revision VALUE: .%X.  \n",  value );

    return value;
}

unsigned short int getCommunicationEngineMinorRevision(void)
{
    unsigned short int value;

    GET_WORD_FROM_WORD(value, IOSyncroAreaI, PLC_MINOR_REV);
    LOG_PRINT_NO_INFO(verbose_e, "@@@@@@@@@@@ Minor Revision VALUE: .%X.  \n",  value );

    return value;
}

int resetError(enum protocol_e protocol)
{
    if(protocol == prot_rtu_e)
    {
        SET_WORD_FROM_WORD(RESET_ERROR_ON, IOSyncroAreaO, RESET_RTU_ERROR);
        return 0;
    }
    else if(protocol == prot_tcp_e)
    {
        SET_WORD_FROM_WORD(RESET_ERROR_ON, IOSyncroAreaO, RESET_TCP_ERROR);
        return 0;
    }
    else if(protocol == prot_tcprtu_e)
    {
        SET_WORD_FROM_WORD(RESET_ERROR_ON, IOSyncroAreaO, RESET_TCPRTU_ERROR);
        return 0;
    }
    else
    {
        LOG_PRINT(error_e, "Unrecognized error reset command\n");
        return 1;
    }
}

int resetErrorByName(const char *protocol)
{
    enum protocol_e protocol_e;

    if (strcmp(protocol, TAG_RTU) == 0)
    {
        protocol_e = prot_rtu_e;
    }
    else if (strcmp(protocol, TAG_TCP) == 0)
    {
        protocol_e = prot_tcp_e;
    }
    else if (strcmp(protocol, TAG_TCPRTU) == 0)
    {
        protocol_e = prot_tcprtu_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol '%s', allowed are '%s' - '%s' - '%s'\n", protocol, TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }

    return resetError(protocol_e);
}

short int getErrorCounter(enum protocol_e protocol, int node)
{
    short int value;

    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "internal variable\n");
        return 0;
    }

    if (node <= 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }

    if(protocol == prot_rtu_e)
    {
        GET_WORD_FROM_WORD(value, IOSyncroAreaI, (COUNTER_RTU_ERROR + (node  * 2 ) ) );
        return value;
    }
    else if(protocol == prot_tcp_e)
    {
        GET_WORD_FROM_WORD(value, IOSyncroAreaI, (COUNTER_TCP_ERROR + (node * 2 ) ) );
        return value;
    }
    else if(protocol == prot_tcprtu_e)

    {
        GET_WORD_FROM_WORD(value, IOSyncroAreaI, (COUNTER_TCPRTU_ERROR + (node  * 2 ) ) );
        return value;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol, allowed are '%s - %s - %s'\n", TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }
}

short int getErrorCounterByName(const char *protocol, int node)
{
    enum protocol_e protocol_e;

    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "internal variable\n");
        return 0;
    }

    if (node <= 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid node number %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }

    if (strcmp(protocol, TAG_RTU) == 0)
    {
        protocol_e = prot_rtu_e;
    }
    else if (strcmp(protocol, TAG_TCP) == 0)
    {
        protocol_e = prot_tcp_e;
    }
    else if (strcmp(protocol, TAG_TCPRTU) == 0)
    {
        protocol_e = prot_tcprtu_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol '%s', allowed are '%s' - '%s' - '%s'\n", protocol, TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }

    return getErrorCounter(protocol_e, node);
}

short int getErrorStatus(enum protocol_e protocol, enum error_kind_e kind)
{
    short int value;

    if(protocol == prot_rtu_e)
    {
        if(kind == error_blacklist_e)
        {
            GET_WORD_FROM_WORD(value, IOSyncroAreaI, BLACKLIST_RTU_ERROR_WORD );
        }
        else if(kind == error_other_e)
        {
            GET_WORD_FROM_WORD(value, IOSyncroAreaI, OTHER_RTU_ERROR_WORD );
        }
        else
        {
            LOG_PRINT(error_e, "Invalid kind of error. Allowed are '%s' - '%s'\n", TAG_BLACKLIST, TAG_OTHER);
            return -1;
        }
        return value;
    }
    else if(protocol == prot_tcp_e)
    {
        if(kind == error_blacklist_e)
        {
            GET_WORD_FROM_WORD(value, IOSyncroAreaI, BLACKLIST_TCP_ERROR_WORD );
        }
        else if(kind == error_other_e)
        {
            GET_WORD_FROM_WORD(value, IOSyncroAreaI, OTHER_TCP_ERROR_WORD );
        }
        else
        {
            LOG_PRINT(error_e, "Invalid kind of error. Allowed are '%s' - '%s'\n", TAG_BLACKLIST, TAG_OTHER);
            return -1;
        }
    }
    else if(protocol == prot_tcprtu_e)
    {
        if(kind == error_blacklist_e)
        {
            GET_WORD_FROM_WORD(value, IOSyncroAreaI, BLACKLIST_TCPRTU_ERROR_WORD );
        }
        else if(kind == error_other_e)
        {
            GET_WORD_FROM_WORD(value, IOSyncroAreaI, OTHER_TCPRTU_ERROR_WORD );
        }
        else
        {
            LOG_PRINT(error_e, "Invalid kind of error. Allowed are '%s' - '%s'\n", TAG_BLACKLIST, TAG_OTHER);
            return -1;
        }
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol. Allowed are '%s' - '%s' - '%s'\n", TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }
    return 0;
}

short int getErrorStatusByName(const char *protocol, const char *kind)
{
    enum error_kind_e kind_e;
    enum protocol_e protocol_e;

    if (strcmp(kind, TAG_BLACKLIST) == 0)
    {
        kind_e = error_blacklist_e;
    }
    else if (strcmp(kind, TAG_OTHER) == 0)
    {
        kind_e = error_other_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid kind of error '%s', allowed are '%s' - '%s'\n", protocol, TAG_BLACKLIST, TAG_OTHER);
        return -1;
    }

    if (strcmp(kind, TAG_RTU) == 0)
    {
        protocol_e = prot_rtu_e;
    }
    else if (strcmp(kind, TAG_TCP) == 0)
    {
        protocol_e = prot_tcp_e;
    }
    else if (strcmp(kind, TAG_TCPRTU) == 0)
    {
        protocol_e = prot_tcprtu_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol '%s', allowed are '%s' - '%s' - '%s'\n", protocol, TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }

    return getErrorStatus(protocol_e, kind_e);
}

short int getErrorBit(enum protocol_e protocol, enum error_kind_e kind, int node)
{
    int value;

    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "internal variable\n");
        return 0;
    }
    if (node <= 0 || node > MAX_DEVICE_NB)
    {
        LOG_PRINT(error_e, "Invalid NodeId %d [%d:%d]\n", node, 0, MAX_DEVICE_NB);
        return -1;
    }

    switch(protocol)
    {
    case prot_rtu_e:
        if(kind == error_blacklist_e)
        {
            if (node < MAX_DEVICE_NB/2 )
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, BLACKLIST_RTU_BIT_ERROR);
                LOG_PRINT(info_e,"BLACKLIST LOW DWORD 0x%x BIT %d\n", value, node - 1);
                return (short int)(GET_FLAG(value, (node - 1)));
            }
            else
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, (BLACKLIST_RTU_BIT_ERROR + SYNCRO_EXCHANGE_DWORD_SIZE) );
                LOG_PRINT(info_e,"BLACKLIST HIGH DWORD 0x%x BIT %d\n", value, node - MAX_DEVICE_NB/2 - 1);
                return (short int)(GET_FLAG(value, (node - MAX_DEVICE_NB/2 - 1)));
            }
        }
        else
        {
            if (node < MAX_DEVICE_NB/2 )
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, OTHER_RTU_BIT_ERROR  );
                LOG_PRINT(info_e,"COMM LOW DWORD 0x%x\n", value);
                return (short int)(GET_FLAG(value, (node - 1)));
            }
            else
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, (OTHER_RTU_BIT_ERROR + SYNCRO_EXCHANGE_DWORD_SIZE) );
                LOG_PRINT(info_e,"COMM HIGH DWORD 0x%x\n", value);
                return (short int)(GET_FLAG(value, (node - MAX_DEVICE_NB/2 - 1 )));
            }
        }
        break;
    case prot_tcp_e:
        if(kind == error_blacklist_e)
        {
            if (node < MAX_DEVICE_NB/2 )
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, BLACKLIST_TCP_BIT_ERROR  );
                return (short int)(GET_FLAG(value, (node - 1)));
            }
            else
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, (BLACKLIST_TCP_BIT_ERROR + SYNCRO_EXCHANGE_DWORD_SIZE) );
                return (short int)(GET_FLAG(value, (node - MAX_DEVICE_NB/2 - 1)));
            }
        }
        else
        {
            if (node < MAX_DEVICE_NB/2 )
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, OTHER_TCP_BIT_ERROR  );
                return (short int)(GET_FLAG(value, (node - 1)));
            }
            else
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, (OTHER_TCP_BIT_ERROR + SYNCRO_EXCHANGE_DWORD_SIZE) );
                return (short int)(GET_FLAG(value, (node - MAX_DEVICE_NB/2 - 1)));
            }
        }
        break;
    case prot_tcprtu_e:
        if(kind == error_blacklist_e)
        {
            if (node < MAX_DEVICE_NB/2 )
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, BLACKLIST_TCPRTU_BIT_ERROR  );
                return (short int)(GET_FLAG(value, (node - 1)));
            }
            else
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, (BLACKLIST_TCPRTU_BIT_ERROR + SYNCRO_EXCHANGE_DWORD_SIZE) );
                return (short int)(GET_FLAG(value, (node - MAX_DEVICE_NB/2 - 1)));
            }
        }
        else
        {
            if (node < MAX_DEVICE_NB/2 )
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, OTHER_TCPRTU_BIT_ERROR  );
                return (short int)(GET_FLAG(value, (node - 1)));
            }
            else
            {
                GET_DWORD_FROM_WORD(value, IOSyncroAreaI, (OTHER_TCPRTU_BIT_ERROR + SYNCRO_EXCHANGE_DWORD_SIZE) );
                return (short int)(GET_FLAG(value, (node - MAX_DEVICE_NB/2 - 1)));
            }
        }
        break;
    case prot_none_e:
        LOG_PRINT(verbose_e, "Internal variable\n");
        return 0;
        break;
    default:
        LOG_PRINT(error_e, "Invalid protocol '%d', allowed are '%s' - '%s' - '%s'\n", protocol, TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
        break;
    }
}

short int getErrorBitByName(const char *protocol, const char *kind, int node)
{
    enum error_kind_e kind_e;
    enum protocol_e protocol_e;

    if (node == INTERNAL_VARIABLE_FAKE_NODEID)
    {
        LOG_PRINT(verbose_e, "internal variable\n");
        return 0;
    }

    if ((node <= 0 || node > MAX_DEVICE_NB) )
    {
        LOG_PRINT(error_e, "Requested error bit does not exist. Allowed nodes are %d (%d-%d)\n", node, 1, MAX_DEVICE_NB);
        return -1;
    }

    if (strcmp(kind, TAG_BLACKLIST) == 0)
    {
        kind_e = error_blacklist_e;
    }
    else if (strcmp(kind, TAG_OTHER) == 0)
    {
        kind_e = error_other_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid kind of error '%s', allowed are '%s' - '%s'\n", protocol, TAG_BLACKLIST, TAG_OTHER);
        return -1;
    }

    if (strcmp(kind, TAG_RTU) == 0)
    {
        protocol_e = prot_rtu_e;
    }
    else if (strcmp(kind, TAG_TCP) == 0)
    {
        protocol_e = prot_tcp_e;
    }
    else if (strcmp(kind, TAG_TCPRTU) == 0)
    {
        protocol_e = prot_tcprtu_e;
    }
    else
    {
        LOG_PRINT(error_e, "Invalid protocol '%s', allowed are '%s' - '%s' - '%s'\n", protocol, TAG_RTU, TAG_TCP, TAG_TCPRTU);
        return -1;
    }

    return getErrorBit(protocol_e, kind_e,  node);
}

/**
 * @brief check the syncro data input to verify if all pending recipe write are finished.
 *
 */
int checkRecipeWriting(void)
{
    unsigned int i;
    unsigned int active_writing = 0;
    for (i = 0; i < SyncroAreaSize; i++)
    {
        LOG_PRINT(info_e, "%X vs %X (%X vs %X) %d\n", GET_SYNCRO_FLAG(i, MULTI_WRITE_RCP_MASK), pIOSyncroAreaI[i], pIOSyncroAreaO[i], pIOSyncroAreaI[i], i);

        if ( GET_SYNCRO_FLAG(i, WRITE_RCP_MASK) == 1 )
        {
            active_writing =1;
            break;
        }
    }

    return ((active_writing ) ? 1 : 0 );
}
/**
 * @brief clean the syncro data for all pending write recipe requests.
 *
 */
void cleanRecipeWriting(void)
{
    unsigned int i;

    for (i = 0; i < SyncroAreaSize; i++)
    {
        LOG_PRINT(info_e, "%X vs %X (%X vs %X) %d\n", GET_SYNCRO_FLAG(i, WRITE_RCP_MASK), pIOSyncroAreaI[i], pIOSyncroAreaO[i], pIOSyncroAreaI[i], i);

        if ( GET_SYNCRO_FLAG(i, WRITE_RCP_MASK) == 1 )
        {
            delSyncroElementByIndex(i);
        }
    }

}

int checkSynIndexWriting(int SynIndex)
{
    int write_pending = 0;
    int CtIndex = -1;

#ifdef ENABLE_MUTEX
    pthread_mutex_lock(&sync_recv_mutex);
#endif
    /* if the element is not yet active, delete it */
    if (
            GET_SYNCRO_FLAG(SynIndex, WRITE_MASK)     == 0  && GET_SYNCRO_FLAG(SynIndex, READ_MASK)            == 0 &&
            GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK)   == 0  && GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK)     == 0 &&
            GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK) == 0  && GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_RCP_MASK) == 0 &&
            pIOSyncroAreaI[SynIndex] == 0
            )
    {
    }
    else if (
             GET_SYNCRO_FLAG(SynIndex, WRITE_MASK)       == 1 ||
             GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK)   == 1 ||
             GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK)     == 1 ||
             GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK) == 1
             )
    {
        CtIndex = (pIOSyncroAreaO[SynIndex] & ADDRESS_MASK);
        if(	GET_SYNCRO_FLAG(SynIndex, WRITE_MASK)       == 1 ||
                GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK)   == 1 ||
                GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK) == 1
                )
        {
            write_pending = 1;
        }
        LOG_PRINT(info_e, "The variable %d CtIndex %d is still active READ %d WRITE %d PREPARE %d MULTI_WRITE %d WRITE_RCP  %d pIOSyncroAreaI[%d] %d write_pending '%d'\n",
                  SynIndex,
                  CtIndex,
                  GET_SYNCRO_FLAG(SynIndex, READ_MASK),
                  GET_SYNCRO_FLAG(SynIndex, WRITE_MASK),
                  GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK),
                  GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK),
                  GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK),
                  SynIndex,
                  pIOSyncroAreaI[SynIndex],
                  write_pending);
    }
    /* if the write operation is done, clear the write flag and if it is necessary set the read flag */
    if (
            (
                GET_SYNCRO_FLAG(SynIndex, WRITE_MASK)           == 1 ||
                GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK)     == 1 ||
                GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK)       == 1 ||
                GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_RCP_MASK) == 1
                ) && pIOSyncroAreaI[SynIndex] == 1)
    {

        if (GET_SYNCRO_FLAG(SynIndex, WRITE_MASK))
        {
            CLR_SYNCRO_FLAG(SynIndex, WRITE_MASK);
        }
        if (GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK))
        {
            CLR_SYNCRO_FLAG(SynIndex, MULTI_WRITE_MASK);
        }
        if (GET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK))
        {
            CLR_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK);
        }
        if (GET_SYNCRO_FLAG(SynIndex, MULTI_WRITE_RCP_MASK))
        {
            CLR_SYNCRO_FLAG(SynIndex, MULTI_WRITE_RCP_MASK);
        }
        write_pending = 1;
        LOG_PRINT(verbose_e, "The write is taken in order\n");
        LOG_PRINT(info_e, "Writing has been done\n");
    }
#ifdef ENABLE_MUTEX
    pthread_mutex_unlock(&sync_recv_mutex);
#endif
    /* if the read operation is done, reset the syncro area */
    if (GET_SYNCRO_FLAG(SynIndex, READ_MASK) == 1 && pIOSyncroAreaI[SynIndex] == 2)
    {
        deleteUnusedSynIndex(SynIndex);
        //pIOSyncroAreaI[SynIndex] = 0;
    }
    LOG_PRINT(verbose_e, "Nothing to do\n");
    return write_pending;
}

/**
 * @brief check the syncro data input to verify if a pending write finish,
 * if yes, update the syncro data output.
 */
void checkWriting(void)
{
    unsigned int i;
    int write_pending = 0;

    /* scan all the syncro array */
    for (i = 0; i < SyncroAreaSize; i++)
    {
        LOG_PRINT(verbose_e, "%X vs %X (%X vs %X) %d\n", GET_SYNCRO_FLAG(i, WRITE_MASK), pIOSyncroAreaI[i], pIOSyncroAreaO[i], pIOSyncroAreaI[i], i);
        if (checkSynIndexWriting(i) == 1)
        {
            write_pending = 1;
        }
    }
    if (write_pending == 0)
    {
        size_t SynIndex;
        SET_WORD_FROM_WORD(WRITE_IRQ_OFF, IOSyncroAreaO, WRITE_IRQ_VAR);
        for (SynIndex = 0; SynIndex < SyncroAreaSize; SynIndex++)
        {
            deleteUnusedSynIndex(SynIndex);
            LOG_PRINT(verbose_e, "DELETE SYNINDEX %d\n", SynIndex);
        }
    }
}

int  getVarDivisor(const int ctIndex)
{
    int decimal = 0;

    decimal = varNameArray[ctIndex].decimal;
    if (decimal > 4)
    {
        if (decimal < ctIndex) //We have a dependent decimal
        {

            if (readFromDb(varNameArray[ctIndex].decimal, &decimal) != 0)
            {
                decimal = 0;
            }
        }
        else
        {
            LOG_PRINT(error_e, "Invalid decimal specification for variable '%s'\n", varNameArray[ctIndex].tag);
            decimal = 0;
        }

    }

    return (int)(pow(10,decimal));
}

int getVarDivisorByName(const char * varname)
{
    int decimal = 0;
    int ctIndex = -1;

    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable '%s'\n", varname);
        return (int)(pow(10,decimal));

    }

    return getVarDivisor(ctIndex);
}

int getVarDecimalByCtIndex(const int ctIndex)
{
    return getVarDecimal(ctIndex);
}

int getVarDecimal(const int ctIndex)
{
    int decimal = 0;

    decimal = varNameArray[ctIndex].decimal;
    if (decimal > 4)
    {
        if (decimal < ctIndex) //We have a dependent decimal
        {
            if (readFromDb(varNameArray[ctIndex].decimal, &decimal) != 0)
            {
                decimal = 0;
            }
            else if (decimal > MAX_DECIMAL_DIGIT)
            {
                LOG_PRINT(error_e, "Too many decimal nuber specifyed '%d' by variable '%s'. Max nuber of digit is '%d'\n", decimal, varNameArray[ctIndex].tag, MAX_DECIMAL_DIGIT);
                decimal = MAX_DECIMAL_DIGIT;
            }
            else if (decimal < 0)
            {
                LOG_PRINT(error_e, "The decimal digit number '%d' specyfied by variable '%s' must be greater than '0'\n", decimal, varNameArray[ctIndex].tag);
                decimal = 0;
            }
        }
        else
        {
            LOG_PRINT(error_e, "Invalid decimal specification for variable '%d'\n", ctIndex);
            decimal = 0;
        }
    }
    return decimal;
}

int getVarDecimalByName(const char * varname)
{
    int ctIndex;
    int decimal = 0;

    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable '%s'\n", varname);
        return decimal;

    }

    decimal = getVarDecimal(ctIndex);
    return decimal;
}

int doWrite(int ctIndex, void * value)
{
    int SynIndex = 0;

    if (prepareWriteVarByCtIndex(ctIndex, value, &SynIndex,1, 0) == DONE || GET_SYNCRO_FLAG(SynIndex, PREPARE_MASK))
    {
        setStatusVarBySynIndex(SynIndex, BUSY);

        /* update the write flag */
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
#ifdef ENABLE_MUTEX
        pthread_mutex_lock(&sync_send_mutex);
#endif
        CLR_SYNCRO_FLAG(SynIndex, PREPARE_MASK);
        SET_SYNCRO_FLAG(SynIndex, WRITE_MASK);
        /*rise write interrupt*/
        SET_WORD_FROM_WORD(WRITE_IRQ_ON, IOSyncroAreaO, WRITE_IRQ_VAR);
#ifdef ENABLE_MUTEX
        pthread_mutex_unlock(&sync_send_mutex);
#endif
        LOG_PRINT(info_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
        return 0;
    }

    return 1;
}

int getStatus(int CtIndex)
{
    return getStatusVarByCtIndex(CtIndex, NULL);
}

int addWrite(int ctIndex, void * value)
{
    return (prepareWriteVarByCtIndex(ctIndex, value, NULL, 0, 0) == ERROR);
}
