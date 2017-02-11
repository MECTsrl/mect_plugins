#include <arpa/inet.h>
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
#include <time.h>

#include "app_logprint.h"
#include "app_var_list.h"
#include "common.h"
#include "cross_table_utility.h"
#include "utility.h"

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

size_t SyncroAreaSize = 0;

variable_t varNameArray[DB_SIZE_ELEM + 1];
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
store_t StoreArrayS[DB_SIZE_ELEM];
store_t StoreArrayF[DB_SIZE_ELEM];
store_t StoreArrayV[DB_SIZE_ELEM];
store_t StoreArrayX[DB_SIZE_ELEM];
int store_elem_nb_S = 0;
int store_elem_nb_F = 0;
int store_elem_nb_V = 0;
int store_elem_nb_X = 0;
#endif

char CrossTableErrorMsg[256];

static int ActualAddress = 0;
static int LastAddress = 0;
static int ActualNreg = 0;
static int LastNreg = 0;
static int blockSize = 1;

int cmpSyncroCtIndex(WORD address, int CtIndex);
int setSyncroCtIndex(WORD * address, int CtIndex);
int setStatusVar(const char * varname, char Status);

pthread_mutex_t datasync_recv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t datasync_send_mutex = PTHREAD_MUTEX_INITIALIZER;

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
 * type:     BYTE|UINT|INT|UDINTABCD|UDINTBADC|UDINTCDAB|UDINTDCBA|DINTABCD|DINTBADC|DINTCDAB|DINTDCBA|REAL|REALBADC|REALCDAB|REALDCBA
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
    char * p = NULL, * r = NULL;
    int bytebitperblock = 0;
    int wordbitperblock = 0;
    int dwordbitperblock = 0;
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
    int isstores = 0;
    int isstoref = 0;
    int isstorev = 0;
    int isstorex = 0;
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
        LOG_PRINT(verbose_e, "%s\n", line);
        /* extract the enable/disable field */
        p = strtok_csv(line, SEPARATOR, &r);        
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'enabled/disabled'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        if (p[0] == '\0' && r == NULL)
        {
            LOG_PRINT(verbose_e, "skip empty line [%d]\n", elem_nb);
            continue;
        }

        if (atoi(p) == 0 || p[0] == '\0')
        {
            LOG_PRINT(verbose_e, "element %d disabled [%s]\n", elem_nb, line);
            elem_nb++;
            continue;
        }

        /* extract PLC flag */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'enabled/disabled'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
        isstores = 0;
        isstoref = 0;
        isstorev = 0;
        isstorex = 0;
#endif
        if (p[0] == TAG_PLC || p[0] == TAG_STORED_SLOW || p[0] == TAG_STORED_FAST || p[0] == TAG_STORED_ON_VAR || p[0] == TAG_STORED_ON_SHOT || IS_MIRROR(elem_nb))
        {
            LOG_PRINT(verbose_e, "%s ELEMENT %d [%s]\n", (IS_MIRROR(elem_nb) == 1) ? "MIRROR": "PLC", elem_nb, line);
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
            if (p[0] == TAG_STORED_SLOW)
            {
                isstores = 1;
            }
            if (p[0] == TAG_STORED_FAST)
            {
                isstoref = 1;
            }
            if (p[0] == TAG_STORED_ON_VAR)
            {
                isstorev = 1;
            }
            if (p[0] == TAG_STORED_ON_SHOT)
            {
                isstorex = 1;
            }
#endif
        }

        /* Tag */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'enabled/disabled'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        if (strlen(p) > TAG_LEN)
        {
            sprintf(CrossTableErrorMsg, "Tag '%s' too long, maximum lenght is %d", p, TAG_LEN - 1);
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        int i;
        for (i = 0; i < elem_nb; i++)
        {
            if (strcmp(p, varNameArray[i].tag) == 0)
            {
                sprintf(CrossTableErrorMsg, "Duplicated tag '%s' at lines %d and %d ", p, i, elem_nb);
                LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                return elem_nb;
            }
        }
        strcpy(varNameArray[elem_nb].tag, p);

        /* type */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Type'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        if (strcmp(p, "BYTE") == 0)
        {
            varNameArray[elem_nb].type = byte_e;
        }
        else if (strcmp(p, "UINT") == 0 || strcmp(p, "UINTAB") == 0)
        {
            varNameArray[elem_nb].type = uintab_e;
        }
        else if (strcmp(p, "UINTBA") == 0)
        {
            varNameArray[elem_nb].type = uintba_e;
        }
        else if (strcmp(p, "INT") == 0 || strcmp(p, "INTAB") == 0)
        {
            varNameArray[elem_nb].type = intab_e;
        }
        else if (strcmp(p, "INTBA") == 0)
        {
            varNameArray[elem_nb].type = intba_e;
        }
        else if (strcmp(p, "UDINT") == 0 || strcmp(p, "UDINTABCD") == 0)
        {
            varNameArray[elem_nb].type = udint_abcd_e;
        }
        else if (strcmp(p, "UDINTBADC") == 0)
        {
            varNameArray[elem_nb].type = udint_badc_e;
        }
        else if (strcmp(p, "UDINTCDAB") == 0)
        {
            varNameArray[elem_nb].type = udint_cdab_e;
        }
        else if (strcmp(p, "UDINTDCBA") == 0)
        {
            varNameArray[elem_nb].type = udint_dcba_e;
        }
        else if (strcmp(p, "DINT") == 0 || strcmp(p, "DINTABCD") == 0)
        {
            varNameArray[elem_nb].type = dint_abcd_e;
        }
        else if (strcmp(p, "DINTBADC") == 0)
        {
            varNameArray[elem_nb].type = dint_badc_e;
        }
        else if (strcmp(p, "DINTCDAB") == 0)
        {
            varNameArray[elem_nb].type = dint_cdab_e;
        }
        else if (strcmp(p, "DINTDCBA") == 0)
        {
            varNameArray[elem_nb].type = dint_dcba_e;
        }
        else if (strcmp(p, "FABCD") == 0 || strcmp(p, "FLOAT") == 0 || strcmp(p, "REALABCD") == 0 || strcmp(p, "REAL") == 0)
        {
            varNameArray[elem_nb].type = fabcd_e;
        }
        else if (strcmp(p, "FBADC") == 0 || strcmp(p, "REALBADC") == 0)
        {
            varNameArray[elem_nb].type = fbadc_e;
        }
        else if (strcmp(p, "FCDAB") == 0 || strcmp(p, "REALCDAB") == 0)
        {
            varNameArray[elem_nb].type = fcdab_e;
        }
        else if (strcmp(p, "FDCBA") == 0 || strcmp(p, "REALDCBA") == 0)
        {
            varNameArray[elem_nb].type = fdcba_e;
        }
        else if (strcmp(p, "BIT") == 0)
        {
            varNameArray[elem_nb].type = bit_e;
        }
        else if (strcmp(p, "BYTE_BIT") == 0)
        {
            varNameArray[elem_nb].type = bytebit_e;
        }
        else if (strcmp(p, "WORD_BIT") == 0)
        {
            varNameArray[elem_nb].type = wordbit_e;
        }
        else if (strcmp(p, "DWORD_BIT") == 0)
        {
            varNameArray[elem_nb].type = dwordbit_e;
        }
        else
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'TYPE' '%s'", p);
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        LOG_PRINT(verbose_e, "Variable %s Type BIT %d\n", varNameArray[elem_nb].tag, varNameArray[elem_nb].type);

        /* Decimal */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
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
            varNameArray[elem_nb].decimal = atoi(p);
        }


        /* Protocol */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Protocol'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        if(strcmp(TAG_RTU, p) == 0)
        {
            varNameArray[elem_nb].protocol = prot_rtu_e;
        }
        else if(strcmp(TAG_TCP, p)==0)
        {
            varNameArray[elem_nb].protocol = prot_tcp_e;
        }
        else if(strcmp(TAG_TCPRTU, p)==0)
        {
            varNameArray[elem_nb].protocol = prot_tcprtu_e;
        }
        else
        {
            varNameArray[elem_nb].protocol = prot_none_e;
        }

        /* Port */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Port'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        /* IP */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'IP'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        /* NodeId */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'NodeId'");
            LOG_PRINT(error_e, "%s at line %d. - p %s\n", CrossTableErrorMsg, elem_nb, p);
            return elem_nb;
        }
        if (p[0] == '\0')
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
                varNameArray[elem_nb].node = INTERNAL_VARIABLE_FAKE_NODEID;
            }
            else
            {
                varNameArray[elem_nb].node = atoi(p);
                if (varNameArray[elem_nb].node <= 0 || varNameArray[elem_nb].node > MAX_DEVICE_NB)
                {
                    sprintf(CrossTableErrorMsg, "invalid node %d for variable '%s'\n", varNameArray[elem_nb].node, varNameArray[elem_nb].tag);
                    LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
                    return elem_nb;
                }
            }
        }
        LOG_PRINT(verbose_e, "Node %d [%s].\n", varNameArray[elem_nb].node, p);

        /* Address */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Address'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        if (varNameArray[elem_nb].node != INTERNAL_VARIABLE_FAKE_NODEID)
        {
            LastAddress = ActualAddress;
            ActualAddress = atoi(p);
        }

        /* Block */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Block'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        varNameArray[elem_nb].block = atoi(p);
        LOG_PRINT(verbose_e, "BLOCK %d VAR %s, line %s, p %s\n", varNameArray[elem_nb].block, varNameArray[elem_nb].tag, line, p);

        /* extract the block head */
        if (elem_nb == 1  || varNameArray[elem_nb].block != varNameArray[elem_nb - 1].block)
        {
            varNameArray[elem_nb].blockhead = elem_nb;
        }
        else
        {
            varNameArray[elem_nb].blockhead = varNameArray[elem_nb - 1].blockhead;
        }

        /* NReg */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'Nreg'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        LastNreg = ActualNreg;
        ActualNreg = atoi(p);
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
        if (isstorex == 1)
        {
            strcpy(StoreArrayX[store_elem_nb_X].tag, varNameArray[elem_nb].tag);
            if (Tag2CtIndex(StoreArrayX[store_elem_nb_X].tag, &(StoreArrayX[store_elem_nb_X].CtIndex)) != 0)
            {
                LOG_PRINT(error_e, "cannot find variable '%s'", StoreArrayX[store_elem_nb_X].tag);
            }
            else
            {
                LOG_PRINT(verbose_e, "a new store variable is inserted '%s' at position %d\n", StoreArrayX[store_elem_nb_X].tag, store_elem_nb_X);
                store_elem_nb_X++;
            }
        }
#endif

        LOG_PRINT(verbose_e, "'%s' %d\n", varNameArray[elem_nb].tag, elem_nb);

        elem_nb++;
    }
    fclose(fp);
    elem_nb--;
#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
    LOG_PRINT(verbose_e, "Loaded %d record stored record S: %d F: %d V: %d\n", elem_nb, store_elem_nb_S, store_elem_nb_F, store_elem_nb_V);
#else
    LOG_PRINT(verbose_e, "Loaded %d record\n", elem_nb);
#endif
    CrossTableErrorMsg[0] = '\0';

    return elem_nb;
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
    if (*CtIndex < 0 || *CtIndex > DB_SIZE_ELEM)
    {
        *CtIndex = -1;
        return 1;
    }
    return 0;
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

    if (Tag == NULL || Tag[0] == '\0')
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
    LOG_PRINT(verbose_e, "Cannot find '%s'\n", Tag);
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

int readFromDbLock(int ctIndex, void * value)
{
    register int retval = 0;
    if (pthread_mutex_lock(&datasync_recv_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
    {
        retval = readFromDb(ctIndex, value);
    }
    if (pthread_mutex_unlock(&datasync_recv_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
    return retval;
}

/** @brief Read the value from the internal DB at psition 'ctIndex'
 * @param int ctIndex : Cross table index of the variable to read
 * @param void * value: value read
 * @return 1 Some error occured
 * @return 0 Write done
 */
int readFromDb(int ctIndex, void * value)
{
    int retval = 0;
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

    int byte_nb = (ctIndex - 1) * 4;
    LOG_PRINT(verbose_e, "'%s': %X\n", varNameArray[ctIndex].tag, pIODataAreaI[byte_nb]);

    switch(varNameArray[ctIndex].type)
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
        memcpy(value, &(pIODataAreaI[byte_nb]), sizeof(float));
        LOG_PRINT(verbose_e, "FLOAT: %f\n", *((float*)value));
        break;
    case byte_e:
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        *((BYTE*)value) = pIODataAreaI[byte_nb];
        LOG_PRINT(verbose_e, "BYTE/BIT: %d\n", *((BYTE*)value));
        break;
    default:
        LOG_PRINT(error_e, "Unknown type '%d'\n", varNameArray[ctIndex].type);
        retval = -1;
    }
    return retval;
}

inline void sprintf_fromFloat(char * s, float fvalue, int decimal)
{
    char fmt[8] = "";
    sprintf (fmt, "%%.%df", decimal);
    sprintf(s, fmt, fvalue / powf(10, decimal));
}

inline void sprintf_fromSigned(char * s, int ivalue, int decimal, int base, int bits)
{
    register int n, i;

    // fixed point
    for (n = 0; n < decimal; ++n) {
        ivalue /= 10;
    }
    // formatting Dec, Hex, Bin
    if (base == 10) {
        sprintf(s, "%d", ivalue);
    } else if (base == 16) {
        char fmt[8] = "";
        if (bits > 1) {
            sprintf (fmt, "0x%%0%dx", bits / 4);
        } else {
            sprintf (fmt, "0x%%01x");
        }
        sprintf(s, fmt, ivalue);
    } else if (base == 2) {
        for (n = (bits - 1), i = 0; n >= 0; --n, ++i) {
            if (ivalue & (1 << n)) {
                s[i] = '1';
            } else {
                s[i] = '0';
            }
        }
        s[i] = 'b';
    } else {
        strcat(s, "?");
    }
}

inline void sprintf_fromUnsigned(char * s, unsigned uvalue, int decimal, int base, int bits)
{
    register int n, i;

    // fixed point
    for (n = 0; n < decimal; ++n) {
        uvalue /= 10;
    }
    // formatting Dec, Hex, Bin
    if (base == 10) {
        sprintf(s, "%u", uvalue);
    } else if (base == 16) {
        char fmt[8] = "";
        if (bits > 1) {
            sprintf (fmt, "0x%%0%dx", bits / 4);
        } else {
            sprintf (fmt, "0x%%01x");
        }
        sprintf(s, fmt, uvalue);
    } else if (base == 2) {
        for (n = (bits - 1), i = 0; n >= 0; --n, ++i) {
            if (uvalue & (1 << n)) {
                s[i] = '1';
            } else {
                s[i] = '0';
            }
        }
        s[i] = 'b';
    } else {
        strcat(s, "?");
    }
}

void sprintf_fromValue(char *s, int ctIndex, int value, int decimal, int base)
{
    // 0. variables
    union {
        uint8_t  uint8_var;
        uint16_t uint16_var;
        uint32_t uint32_var;
        int16_t  int16_var;
        int32_t  int32_var;
        float    float_var;
    } var;

    // 1. memcpy
    var.int32_var = value;

    // 2. sprintf
    if (s == NULL) {
        return;
    }
    switch(varNameArray[ctIndex].type)
    {
    case uintab_e:
    case uintba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.uint16_var, decimal);
        } else {
            sprintf_fromUnsigned(s, var.uint16_var, decimal, base, 16);
        }
        break;

    case intab_e:
    case intba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.int16_var, decimal);
        } else {
            sprintf_fromSigned(s, var.int16_var, decimal, base, 16);
        }
        break;

    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.uint32_var, decimal);
        } else {
            sprintf_fromUnsigned(s, var.uint32_var, decimal, base, 32);
        }
        break;

    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.int32_var, decimal);
        } else {
            sprintf_fromSigned(s, var.int32_var, decimal, base, 32);
        }
        break;

    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        // ignore base
        sprintf_fromFloat(s, var.float_var, decimal);
        break;

    case byte_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.uint8_var, decimal);
        } else {
            sprintf_fromUnsigned(s, var.uint8_var, decimal, base, 8);
        }
        break;

    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        // ignore decimal
        if (var.uint8_var == 0) {
            sprintf_fromUnsigned(s, 0, 0, base, 1);
        } else {
            sprintf_fromUnsigned(s, 1, 0, base, 1);
        }
        break;

    default:
        strcat(s, "?");
    }
}

int formattedReadFromDb_string(int ctIndex, char * value)
{
    int decimal = 0;

    /* invalid ctIndex */
    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }

    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);

    decimal = getVarDecimalByCtIndex(ctIndex);

    LOG_PRINT(verbose_e, "CURRENT Decimal is %d to be used for VARIABLE %s\n", decimal, varNameArray[ctIndex].tag);

    switch(varNameArray[ctIndex].type)
    {
    case uintab_e:
    case uintba_e:
    {
        unsigned short int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
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
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
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
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
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
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
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
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
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
    case byte_e:
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
    {
        BYTE _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
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

    switch (pIODataStatusAreaI[ctIndex])
    {
    case DONE:
    case BUSY:
        break;
    case ERROR:
    default:
        LOG_PRINT(verbose_e, "ERROR %d\n", ctIndex);
        return -1;
    }

    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%s'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, value);
    return 0;
}

int formattedReadFromDb_float(int ctIndex, float * fvalue)
{
    int decimal = 0;

    /* invalid ctIndex */
    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }

    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);

    decimal = getVarDecimalByCtIndex(ctIndex);

    LOG_PRINT(verbose_e, "CURRENT Decimal is %d to be used for VARIABLE %s\n", decimal, varNameArray[ctIndex].tag);

    switch(varNameArray[ctIndex].type)
    {
    case uintab_e:
    case uintba_e:
    {
        unsigned short int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *fvalue = (float)_value / pow(10,decimal);
        }
        else
        {
            *fvalue = (float)_value;
        }
    }
        break;
    case intab_e:
    case intba_e:
    {
        short int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *fvalue = (float)_value / pow(10,decimal);
        }
        else
        {
            *fvalue = (float)_value;
        }
    }
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
    {
        unsigned int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *fvalue = (float)_value / pow(10,decimal);
        }
        else
        {
            *fvalue = (float)_value;
        }
    }
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
    {
        int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *fvalue = (float)_value / pow(10,decimal);
        }
        else
        {
            *fvalue = (float)_value;
        }
    }
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
    {
        float _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %f decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        *fvalue = _value; // NO decimal
    }
        break;
    case byte_e:
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
    {
        BYTE _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        *fvalue = (float)_value;
    }
        break;
    default:
        LOG_PRINT(error_e, "unknown type %d\n", ctIndex);
        return -1;
        break;
    }

    switch (pIODataStatusAreaI[ctIndex])
    {
    case DONE:
    case BUSY:
        break;
    case ERROR:
    default:
        LOG_PRINT(verbose_e, "ERROR %d\n", ctIndex);
        return -1;
    }

    LOG_PRINT(verbose_e, "HEX %X - FORMATTED float '%s': %f\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *fvalue);
    return 0;
}

int formattedReadFromDb_int(int ctIndex, int * ivalue)
{
    int decimal = 0;

    /* invalid ctIndex */
    if (ctIndex < 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }

    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);

    decimal = getVarDecimalByCtIndex(ctIndex);

    LOG_PRINT(verbose_e, "CURRENT Decimal is %d to be used for VARIABLE %s\n", decimal, varNameArray[ctIndex].tag);

    switch(varNameArray[ctIndex].type)
    {
    case uintab_e:
    case uintba_e:
    {
        unsigned short int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *ivalue = (int)_value / pow(10,decimal);
        }
        else
        {
            *ivalue = (int)_value;
        }
    }
        break;
    case intab_e:
    case intba_e:
    {
        short int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *ivalue = (int)_value / pow(10,decimal);
        }
        else
        {
            *ivalue = (int)_value;
        }
    }
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
    {
        unsigned int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *ivalue = (int)_value / pow(10,decimal);
        }
        else
        {
            *ivalue = (int)_value;
        }
    }
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
    {
        int _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *ivalue = (int)_value / pow(10,decimal);
        }
        else
        {
            *ivalue = (int)_value;
        }
    }
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
    {
        float _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        LOG_PRINT(verbose_e, "%s - value %f decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        *ivalue = (int)_value; // NO decimal
    }
        break;
    case byte_e:
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
    {
        BYTE _value;
        if (readFromDbLock(ctIndex, &_value) != 0)
        {
            return -1;
        }
        *ivalue = (int)_value;
    }
        break;
    default:
        LOG_PRINT(error_e, "unknown type %d\n", ctIndex);
        return -1;
        break;
    }

    switch (pIODataStatusAreaI[ctIndex])
    {
    case DONE:
    case BUSY:
        break;
    case ERROR:
    default:
        LOG_PRINT(verbose_e, "ERROR %d\n", ctIndex);
        return -1;
    }

    LOG_PRINT(verbose_e, "HEX %X - FORMATTED int '%s': %d\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *ivalue);
    return 0;
}

void writeVarInQueueByCtIndex(int ctIndex, int value)
{
    switch (prepareWriteVarByCtIndex(ctIndex, value, 1))
    {
    case DONE:
        break;
    case BUSY:
        if (pthread_mutex_lock(&write_queue_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
        {
            write_queue_elem_t * queue_elem = (write_queue_elem_t *)calloc(1, sizeof(write_queue_elem_t));
            queue_elem->next = NULL;
            queue_elem->ctIndex = ctIndex;
            queue_elem->value = value;

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
        if (pthread_mutex_unlock(&write_queue_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
        break;
    case ERROR:
    default:
        ;
    }
}

void writeVarQueuedByCtIndex(void)
{
    if (pthread_mutex_lock(&write_queue_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
    {
        // get items from queue head
        write_queue_elem_t * queue_elem = queue_head;
        write_queue_elem_t * queue_prev = NULL;
        while (queue_elem != NULL)
        {
            if (prepareWriteVarByCtIndex(queue_elem->ctIndex, queue_elem->value, 1) == BUSY)
            {
                // BUSY: rimane in coda
                queue_prev = queue_elem;
                queue_elem = queue_elem->next;
            }
            else
            {
                // DONE, ERROR: toglie dalla coda
                if (queue_head == queue_elem)
                {
                    // toglie dalla testa
                    queue_head = queue_elem->next;
                    if (queue_head == NULL)
                    {
                        queue_tail = NULL;
                    }
                    free(queue_elem);
                    queue_elem = queue_head;
                }
                else
                {
                    // toglie da in mezzo
                    queue_prev->next = queue_elem->next;
                    free(queue_elem);
                    queue_elem = queue_prev->next;
                }
            }
        }
    }
    if (pthread_mutex_unlock(&write_queue_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
}

/**
 * @brief perform a block write request
 */
int writePendingInorder()
{
#ifdef AVOID_RECIPES
    return 0;
#else
    unsigned int SynIndex;
    int do_signal = 0;
    if (pthread_mutex_lock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
    {
        for (SynIndex = 0; SynIndex < SyncroAreaSize; SynIndex++)
        {
            if (IS_PREPARE_SYNCRO_FLAG(SynIndex))
            {
                LOG_PRINT(verbose_e, "Clear the PREPARE FLAG %d -> %X\n", SynIndex, pIOSyncroAreaO[SynIndex]);
                SET_SYNCRO_FLAG(SynIndex, WRITE_RCP_MASK);
                LOG_PRINT(verbose_e, "Writing the RCP_WRITE FLAG %d -> %X\n", SynIndex, pIOSyncroAreaO[SynIndex]);
                do_signal = 1;
            }
        }
        if (do_signal)
        {
            pthread_cond_signal(&theWritingCondvar);
        }
    }
    if (pthread_mutex_unlock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
    LOG_PRINT(verbose_e, "Set writing flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
    return 0;
#endif
}

/**
 * @brief prepare all the data in order to send a write request for the variable varname
 * to perform the write request, you need to enable the write flag.
 */

char prepareWriteVarByCtIndex(int ctIndex, int value, int execwrite)
{
    char retval = ERROR;
    unsigned i;

    if (ctIndex <= 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return retval;
    }

    LOG_PRINT(verbose_e, "Writing '%d'\n", ctIndex);
    if (pthread_mutex_lock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
    {
        uint16_t oper;
        uint16_t addr;

        /* search for pending writes on the same variable */
        for (i = 0; i < SyncroAreaSize; i++)
        {
            addr = pIOSyncroAreaO[i] & ADDRESS_MASK;
            oper = pIOSyncroAreaO[i] & OPER_MASK;
               /* marked               all writes             prepare            empty */
            if ((addr == ctIndex) && (oper & 0x8000 || oper == 0x2000 || oper == 0x0000))
            {
                if (execwrite) {
                    LOG_PRINT(warning_e, "busy writing(W) #%d (%u/%u) %s\n", ctIndex, i, SyncroAreaSize, varNameArray[ctIndex].tag);
                } else {
                    LOG_PRINT(warning_e, "busy writing(P) #%d (%u/%u) %s\n", ctIndex, i, SyncroAreaSize, varNameArray[ctIndex].tag);
                }
                retval = BUSY;
                goto exit_function;
            }
        }

        /* create a new item in Syncro area */
        if (SyncroAreaSize >= SYNCRO_DB_SIZE_ELEM)
        {
            retval = BUSY;
            LOG_PRINT(error_e, "full writing %d\n", ctIndex);
            goto exit_function;
        }
        pIOSyncroAreaO[SyncroAreaSize] = ctIndex & ADDRESS_MASK;
        if (execwrite) {
            SET_SYNCRO_FLAG(SyncroAreaSize, WRITE_MASK);
        } else {
            SET_SYNCRO_FLAG(SyncroAreaSize, PREPARE_MASK);
        }
        SyncroAreaSize++;

        /* update the value into the Data area */
        int *p = (int *)IODataAreaO;
        p[ctIndex] = value;
        retval = DONE;
    }
exit_function:
    if (retval == DONE && execwrite)
    {
        pthread_cond_signal(&theWritingCondvar);
    }
    if (pthread_mutex_unlock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
    return retval;
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

/**
 * @brief comunicate to the communicatuion PLC that this variable must be read untill the page is active
 */
int activateVar(const char * varname)
{
    int CtIndex;
    int found = 0;
    unsigned i;

    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return 1;
    }
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return 1;
    }
    if (varNameArray[CtIndex].blockhead != CtIndex)
    {
        LOG_PRINT(error_e, "'%s' is not BlockHead\n", varname);
        return 1;
    }
    if (pthread_mutex_lock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
    {
        for (i = 0; i < SyncroAreaSize; i++)
        {
            if (cmpSyncroCtIndex(pIOSyncroAreaO[i], CtIndex) == 0 && IS_READ_SYNCRO_FLAG(i))
            {
                found = 1;
                break;
            }
        }
        if (! found) {
            setSyncroCtIndex(&(pIOSyncroAreaO[SyncroAreaSize]), CtIndex);
            SET_SYNCRO_FLAG(SyncroAreaSize, READ_MASK);
            SyncroAreaSize++;
            varNameArray[CtIndex].visible = 1;
        }
    }
    if (pthread_mutex_unlock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
    return 0;
}

/**
 * @brief comunicate to the communicatuion PLC that this variable is not necessary to be read
 */
int deactivateVar(const char * varname)
{
    int CtIndex;
    int retval = 1;
    unsigned i;

    if (varname[0] == '\0')
    {
        LOG_PRINT(error_e, "'%s' Empty variable\n", varname);
        return retval;
    }
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "'%s' not found into DataVector\n", varname);
        return retval;
    }
    if (varNameArray[CtIndex].blockhead != CtIndex)
    {
        LOG_PRINT(error_e, "'%s' is not BlockHead\n", varname);
        return retval;
    }
    if (pthread_mutex_lock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex lock\n");};
    {
        for (i = 0; i < SyncroAreaSize; i++)
        {
            if (cmpSyncroCtIndex(pIOSyncroAreaO[i], CtIndex) == 0 && IS_READ_SYNCRO_FLAG(i))
            {
                CLR_SYNCRO_FLAG(i);
                varNameArray[CtIndex].visible = 0;
                retval = 0;
                break;
            }
        }
    }
    if (pthread_mutex_unlock(&datasync_send_mutex)) {LOG_PRINT(error_e, "mutex unlock\n");};
    return retval;
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
    LOG_PRINT(error_e, "called  int setStatusVar()\n");
    return 1;
}

/**
 * @brief check the syncro data input to verify if a pending write finish,
 * if yes, update the syncro data output.
 */
void compactSyncWrites(void)
{
    int  SynIndex;

    // already locked both for send and recv
    // (Wadr,0) --> (0Adr,0)
    // (0Adr,0) --> (0,0)
    for (SynIndex = 0; SynIndex < SyncroAreaSize; ++SynIndex)
    {
        if (IS_WRITE_SYNCRO_FLAG(SynIndex) && (pIOSyncroAreaI[SynIndex] == 1)) // QUEUE_BUSY_WRITE
        {
            CLR_SYNCRO_FLAG(SynIndex);
        }
        else if (IS_EMPTY_SYNCRO_FLAG(SynIndex) && (pIOSyncroAreaI[SynIndex] == 0))
        {
            pIOSyncroAreaO[SynIndex] = 0x0000;
        }
    }
    // (0000,0) --> --SyncroAreaSize
    while (SyncroAreaSize > 0) {
        SynIndex = SyncroAreaSize - 1;
        if (pIOSyncroAreaO[SynIndex] == 0x0000 && pIOSyncroAreaI[SynIndex] == 0) {
            --SyncroAreaSize;
        } else {
            break;
        }
    }
}

int getVarDecimalByCtIndex(const int ctIndex)
{
    int decimal = 0;

    decimal = varNameArray[ctIndex].decimal; // for *bit is set to 0
    if (decimal > 4)
    {
        if (decimal < ctIndex) //We have a dependent decimal
        {
            if (readFromDbLock(varNameArray[ctIndex].decimal, &decimal) != 0)
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

    decimal = getVarDecimalByCtIndex(ctIndex);
    return decimal;
}

inline int theValue(int ctIndex, void * valuep)
{
    int value = 0;

    if (!valuep || ctIndex <= 0 || ctIndex > DB_SIZE_ELEM) {
        LOG_PRINT(error_e, "wrong args %d %p\n", ctIndex, valuep);
        value = -1;
    }
    else
    {
        switch (varNameArray[ctIndex].type)
        {
        case uintab_e:
        case uintba_e:
        case intab_e:
        case intba_e:
            value = *(int16_t *)valuep;
            break;
        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
            value = *(int32_t *)valuep;
            break;
        case byte_e:
        case bit_e:
        case bytebit_e:
        case wordbit_e:
        case dwordbit_e:
            value = *(int8_t *)valuep;
            break;
        default:
            LOG_PRINT(error_e, "wrong type %d '%d'\n", ctIndex, varNameArray[ctIndex].type);
            value = -1;
        }
    }
    return value;
}

inline int theValue_string(int ctIndex, char * valuep)
{
    int value = 0;

    if (!valuep || ctIndex <= 0 || ctIndex > DB_SIZE_ELEM) {
        LOG_PRINT(error_e, "wrong args %d %p\n", ctIndex, valuep);
        value = -1;
    }
    else
    {
        float fvalue = 0;

        switch (varNameArray[ctIndex].type)
        {
        case uintab_e:
        case uintba_e:
        case intab_e:
        case intba_e:
        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
        case byte_e:
        case bit_e:
        case bytebit_e:
        case wordbit_e:
        case dwordbit_e: {
            int decimal = getVarDecimalByCtIndex(ctIndex); // for *bit is set to 0

            if (decimal > 0) {
                fvalue = atof(valuep) * powf(10, decimal);
                value = (float)fvalue;
            } else {
                value = atoi(valuep);
            }
        }   break;
        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
            fvalue = atof(valuep);
            memcpy(&value, &fvalue, sizeof(int));
            break;
        default:
            LOG_PRINT(error_e, "wrong type %d '%d'\n", ctIndex, varNameArray[ctIndex].type);
            value = -1;
        }
    }
    return value;
}

int setFormattedVarByCtIndex(const int ctIndex, char * formattedVar)
{
    int value = theValue_string(ctIndex, formattedVar);
    writeVarInQueueByCtIndex(ctIndex, value);
    return 0;
}

int doWrite(int ctIndex, void * valuep)
{
    writeVarInQueueByCtIndex(ctIndex, theValue(ctIndex, valuep));
    return 0;
}

int getStatus(int CtIndex)
{
    return pIODataStatusAreaI[CtIndex];
}

int addWrite(int ctIndex, void * valuep)
{
#ifdef AVOID_RECIPES
    return doWrite(ctIndex, valuep);
#else
    char retval;

    retval = prepareWriteVarByCtIndex(ctIndex, theValue(ctIndex, valuep), 0);

    switch (retval) {
    case DONE:
        return 0;
    case BUSY:
        LOG_PRINT(error_e, "busy adding #%d\n", ctIndex);
        return 1;
    case ERROR:
        LOG_PRINT(error_e, "error adding #%d\n", ctIndex);
        return 1;
    default:
        return 1;
    }
#endif
}
