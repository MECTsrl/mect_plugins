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
#include "common.h"
#include "cross_table_utility.h"
#include "utility.h"

#include "hmi_plc.h"

HmiPlcBlock plcBlock; // actual values from plc
HmiPlcBlock hmiBlock; // doWrite(), addWrite(), activateVar(), disactivateVar()

char *pIODataAreaI = (char *)&(plcBlock.values[1]); // only for back compatibility (vedi https://github.com/MECTsrl/mect_apps/tree/mect_suite_3.0/AnyTPAC/DemoDumpVariables)

#define VAR_VALUE(n)  plcBlock.values[n].u32
#define VAR_STATE(n)  plcBlock.states[n]

uint16_t writingCount;
uint16_t writingList[DimCrossTable]; // NB: zero based

variable_t varNameArray[DB_SIZE_ELEM + 1];

store_t StoreArrayS[DB_SIZE_ELEM];
store_t StoreArrayF[DB_SIZE_ELEM];
store_t StoreArrayV[DB_SIZE_ELEM];
store_t StoreArrayX[DB_SIZE_ELEM];
int store_elem_nb_S = 0;
int store_elem_nb_F = 0;
int store_elem_nb_V = 0;
int store_elem_nb_X = 0;

char CrossTableErrorMsg[256];

static int ActualAddress = 0;
static int LastAddress = 0;
static int ActualNreg = 0;
static int LastNreg = 0;
static int blockSize = 1;

pthread_mutex_t datasync_recv_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t datasync_send_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Open the cross table and fill the syncro vector with the Mirror variables and the PLC variables
 *
 * The cross-table file is an acii file with the fllowing syntax:
 * <Enable>; <PLC>; <Tag>; <Type>; <Decimal>; <Protocol>; <Port>; <NodeId>; <Address>; <Block>; <Nreg>; <Handle>
 *
 * Enable:   Flag 0/1
 * PLC:      Flag 0/1
 * Tag:      char[TAG_LEN]
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
    int elem_nb = 1, n;
    char line[LINE_SIZE];
    char * p = NULL, * r = NULL;
    int bytebitperblock = 0;
    int wordbitperblock = 0;
    int dwordbitperblock = 0;

    fp = fopen(CROSS_TABLE, "r");
    if (fp == NULL)
    {
        sprintf(CrossTableErrorMsg, "Cannot open the Crosstable '%s': [%s]", CROSS_TABLE, strerror(errno));
        LOG_PRINT(error_e, "%s\n", CrossTableErrorMsg);
        return -1;
    }

    resetHmiPlcBlocks(&hmiBlock, &plcBlock);
    for (n = 1; n <= DB_SIZE_ELEM; ++n)
    {
        varNameArray[n].type = udint_abcd_e; // workaround for theValue() when using old Crosstables
        writingList[n] = 0;
    }
    writingCount = 0;

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

        /* extract Update flag */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'enabled/disabled'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        varNameArray[elem_nb].active = p[0];
        varNameArray[elem_nb].visible = 1;
        switch (varNameArray[elem_nb].active) {
        case TAG_ONDEMAND:
            varNameArray[elem_nb].visible = 0;
            hmiBlock.states[elem_nb] = varStatus_NOP; // NB: now hmiBlock.first = hmiBlock.last = 0
            break;
        case TAG_PLC:
        case TAG_STORED_SLOW:
        case TAG_STORED_FAST:
        case TAG_STORED_ON_VAR:
        case TAG_STORED_ON_SHOT:
            // ok, fill below after reading the tag
            break;
        default:
            sprintf(CrossTableErrorMsg, "Malformed element 'update'='%c'", p[0]);
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }

        /* Tag */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            sprintf(CrossTableErrorMsg, "Malformed element 'enabled/disabled'");
            LOG_PRINT(error_e, "%s at line %d.\n", CrossTableErrorMsg, elem_nb);
            return elem_nb;
        }
        if (strlen(p) >= TAG_LEN)
        {
            sprintf(CrossTableErrorMsg, "Tag '%s' too long, maximum length is %d", p, TAG_LEN - 1);
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
        switch (varNameArray[elem_nb].active) {
        case TAG_ONDEMAND:
        case TAG_PLC:
            break;
        case TAG_STORED_SLOW:
            strcpy(StoreArrayS[store_elem_nb_S].tag, varNameArray[elem_nb].tag);
            StoreArrayS[store_elem_nb_S].CtIndex = elem_nb;
            store_elem_nb_S++;
            break;
        case TAG_STORED_FAST:
            strcpy(StoreArrayF[store_elem_nb_F].tag, varNameArray[elem_nb].tag);
            StoreArrayF[store_elem_nb_F].CtIndex = elem_nb;
            store_elem_nb_F++;
            break;
        case TAG_STORED_ON_VAR:
            strcpy(StoreArrayV[store_elem_nb_V].tag, varNameArray[elem_nb].tag);
            StoreArrayV[store_elem_nb_V].CtIndex = elem_nb;
            store_elem_nb_V++;
            break;
        case TAG_STORED_ON_SHOT:
            strcpy(StoreArrayX[store_elem_nb_X].tag, varNameArray[elem_nb].tag);
            StoreArrayX[store_elem_nb_X].CtIndex = elem_nb;
            store_elem_nb_X++;
            break;
        default:
            ;
        }

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
        if (varNameArray[elem_nb].type == bit_e ||
            varNameArray[elem_nb].type == bytebit_e ||
            varNameArray[elem_nb].type == wordbit_e ||
            varNameArray[elem_nb].type == dwordbit_e )
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

        if (strcmp(TAG_RTU, p) == 0)
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

        elem_nb++;
    }
    fclose(fp);
    elem_nb--;
    LOG_PRINT(verbose_e, "Loaded %d record stored record S: %d F: %d V: %d\n", elem_nb, store_elem_nb_S, store_elem_nb_F, store_elem_nb_V);
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
    if (Tag && CtIndex >= 1 && CtIndex <= DimCrossTable) {
        strcpy(Tag, varNameArray[CtIndex].tag);
        return 0;
    } else {
        return -1;
    }
}

/**
 * @brief extract the type of the variable positioned at the index cross table CtIndex
 */
int CtIndex2Type(int CtIndex)
{
    if (CtIndex >= 1 && CtIndex <= DimCrossTable) {
        return varNameArray[CtIndex].type;
    } else {
        return -1;
    }
}

char readFromDbQuick(int ctIndex, int * ivaluep)
{
    register int ivalue;
    register char status;

    pthread_mutex_lock(&datasync_recv_mutex);
    {
        ivalue = plcBlock.values[ctIndex].i32;
        status = plcBlock.states[ctIndex];
    }
    pthread_mutex_unlock(&datasync_recv_mutex);

    *ivaluep = ivalue;
    return status;
}

int readFromDbLock(int ctIndex, void * value)
{
    register int retval = 0;
    pthread_mutex_lock(&datasync_recv_mutex);
    {
        retval = readFromDb(ctIndex, value);
    }
    pthread_mutex_unlock(&datasync_recv_mutex);
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
    if (ctIndex <= 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return -1;
    }
    switch(varNameArray[ctIndex].type)
    {
    case uintab_e:
    case uintba_e:
        memcpy(value, &plcBlock.values[ctIndex].u16, sizeof(uint16_t));
        break;
    case intab_e:
    case intba_e:
        memcpy(value, &plcBlock.values[ctIndex].i16, sizeof(int16_t));
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        memcpy(value, &plcBlock.values[ctIndex].u32, sizeof(uint32_t));
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        memcpy(value, &plcBlock.values[ctIndex].i32, sizeof(int32_t));
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        memcpy(value, &plcBlock.values[ctIndex].f, sizeof(float));
        break;
    case byte_e:
        memcpy(value, &plcBlock.values[ctIndex].u8, sizeof(uint8_t));
        break;
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        if (plcBlock.values[ctIndex].u8) {
            *((int *)value) = 1;
        } else {
            *((int *)value) = 0;
        }
        break;
    default:
        LOG_PRINT(error_e, "Unknown type '%d'\n", varNameArray[ctIndex].type);
        retval = -1;
    }
    return retval;
}

inline void sprintf_fromFloat(char * s, float fvalue, int decimal, int fixedpoint)
{
    char fmt[8] = "";
    sprintf (fmt, "%%.%df", decimal);
    sprintf(s, fmt, fvalue / powf(10, fixedpoint));
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
            if (n > 0 && n % 4 == 0) {
                ++i;
                s[i] = '.';
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
            if (n > 0 && n % 4 == 0) {
                ++i;
                s[i] = '.';
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
    varUnion var;

    // 1. memcpy
    var.i32 = value;

    // 2. sprintf
    if (s == NULL) {
        return;
    }
    switch(varNameArray[ctIndex].type)
    {
    case uintab_e:
    case uintba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.u16, decimal, decimal);
        } else {
            sprintf_fromUnsigned(s, var.u16, decimal, base, 16);
        }
        break;

    case intab_e:
    case intba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.i16, decimal, decimal);
        } else {
            sprintf_fromSigned(s, var.i16, decimal, base, 16);
        }
        break;

    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.u32, decimal, decimal);
        } else {
            sprintf_fromUnsigned(s, var.u32, decimal, base, 32);
        }
        break;

    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.i32, decimal, decimal);
        } else {
            sprintf_fromSigned(s, var.i32, decimal, base, 32);
        }
        break;

    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        // ignore base
        sprintf_fromFloat(s, var.f, decimal, 0); // no fixedpoint
        break;

    case byte_e:
        if (decimal > 0 && base == 10) {
            sprintf_fromFloat(s, (float)var.u8, decimal, decimal);
        } else {
            sprintf_fromUnsigned(s, var.u8, decimal, base, 8);
        }
        break;

    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        // ignore decimal
        if (var.u8 == 0) {
            sprintf_fromUnsigned(s, 0, 0, base, 1);
        } else {
            sprintf_fromUnsigned(s, 1, 0, base, 1);
        }
        break;

    default:
        strcat(s, "?");
    }
}

float sprintf_fromDb(char *s, int ctIndex)
{
    float retval;
    int ivalue;

    switch (readFromDbQuick(ctIndex, &ivalue)) {
    case DONE:
    case BUSY: {
        register int decimal = getVarDecimalByCtIndex(ctIndex); // locks only if it's from another variable

        sprintf_fromValue(s, ctIndex, ivalue, decimal, 10);
        retval = float_fromValue(ctIndex, ivalue, decimal);
      } break;
    case ERROR:
    default:
        strcpy(s, TAG_NAN);
        retval = NAN;
    }
    return retval;
}

float float_fromValue(int ctIndex, int value, int decimal)
{
    float retval;

    // 0. variables
    varUnion var;

    // 1. memcpy
    var.i32 = value;
    switch(varNameArray[ctIndex].type) {
    case uintab_e:
    case uintba_e:
        retval = (float)var.u16;
        if (decimal > 0) {
            retval = retval / powf(10, decimal);
        }
        break;
    case intab_e:
    case intba_e:
        retval = (float)var.i16;
        if (decimal > 0) {
            retval = retval / powf(10, decimal);
        }
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        retval = (float)var.u32;
        if (decimal > 0) {
            retval = retval / powf(10, decimal);
        }
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        retval = (float)var.i32;
        if (decimal > 0) {
            retval = retval / powf(10, decimal);
        }
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        retval = var.f;
        // no fixed point adjustment
        break;
    case byte_e:
        retval = (float)var.u8;
        if (decimal > 0) {
            retval = retval / powf(10, decimal);
        }
        break;
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        if (var.u8 == 0) {
            retval = 0.0;
        } else {
            retval = 1.0;
        }
        // no fixed point adjustment
        break;
    default:
        retval = -1;
    }

    return retval;
}

int int_fromValue(int ctIndex, int value, int decimal)
{
    int retval;
    int n;

    // 0. variables
    varUnion var;

    // 1. memcpy
    var.i32 = value;
    switch(varNameArray[ctIndex].type) {
    case uintab_e:
    case uintba_e:
        retval = (int)var.u16;
        for (n = decimal; n > 0; --n) {
            retval = retval / 10;
        }
        break;
    case intab_e:
    case intba_e:
        retval = (int)var.i16;
        for (n = decimal; n > 0; --n) {
            retval = retval / 10;
        }
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
        retval = (int)var.u32;
        for (n = decimal; n > 0; --n) {
            retval = retval / 10;
        }
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
        retval = (int)var.i32;
        for (n = decimal; n > 0; --n) {
            retval = retval / 10;
        }
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
        retval = (int)var.f;
        // no fixed point adjustment
        break;
    case byte_e:
        retval = (int)var.u8;
        for (n = decimal; n > 0; --n) {
            retval = retval / 10;
        }
        break;
    case bit_e:
    case bytebit_e:
    case wordbit_e:
    case dwordbit_e:
        if (var.u8 == 0) {
            retval = 0;
        } else {
            retval = 1;
        }
        // no fixed point adjustment
        break;
    default:
        retval = -1;
    }

    return retval;
}

/**
 * @brief perform a block write request
 */
int writePendingInorder()
{
    int do_signal = 0;

    pthread_mutex_lock(&datasync_send_mutex);
    {
        int addr;

        for (addr = 1; addr <= DimCrossTable; ++addr) {
            if (hmiBlock.states[addr] == varStatus_PREPARING) {
                changeStatusHmiBlock(&hmiBlock, addr, varStatus_DO_WRITE);
                if (writingCount < DimCrossTable) {
                    writingList[writingCount] = addr;
                    ++writingCount;
                } else {
                    LOG_PRINT(error_e, "the writing list is full\n");
                }
                do_signal = 1;
            }
        }
        if (do_signal)
        {
            pthread_cond_signal(&theWritingCondvar);
        }
    }
    pthread_mutex_unlock(&datasync_send_mutex);
    return 0;
}

/**
 * @brief prepare all the data in order to send a write request for the variable varname
 * to perform the write request, you need to enable the write flag.
 */

char prepareWriteVarByCtIndex(int ctIndex, int value, int execwrite)
{
    char retval = ERROR;

    if (ctIndex <= 0 || ctIndex > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "invalid Ctindex %d\n", ctIndex);
        return retval;
    }
    pthread_mutex_lock(&datasync_send_mutex);
    {
        hmiBlock.values[ctIndex].i32 = value;
        if (execwrite) {
            changeStatusHmiBlock(&hmiBlock, ctIndex, varStatus_DO_WRITE); // NB: even if it was varStatus_PREPARING
            if (writingCount < DimCrossTable) {
                writingList[writingCount] = ctIndex;
                ++writingCount;
            } else {
                LOG_PRINT(error_e, "the writing list is full\n");
            }
        } else {
            changeStatusHmiBlock(&hmiBlock, ctIndex, varStatus_PREPARING);
        }
        retval = DONE;
        if (retval == DONE && execwrite)
        {
            pthread_cond_signal(&theWritingCondvar);
        }
    }
    pthread_mutex_unlock(&datasync_send_mutex);
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
    if (! varNameArray[CtIndex].active != TAG_ONDEMAND)
    {
        LOG_PRINT(error_e, "'%s' has not '%c' update type\n", varname, TAG_ONDEMAND);
        return 1;
    }
    pthread_mutex_lock(&datasync_send_mutex);
    {
        varNameArray[CtIndex].visible = 1;
        // NB: may be varStatus_PREPARING or varStatus_DO_WRITE
        // see also io_layer_comm::run()
        if (hmiBlock.states[CtIndex] == varStatus_NOP) {
            changeStatusHmiBlock(&hmiBlock, CtIndex, varStatus_DO_READ);
        }
    }
    pthread_mutex_unlock(&datasync_send_mutex);
    return 0;
}

/**
 * @brief comunicate to the communicatuion PLC that this variable is not necessary to be read
 */
int deactivateVar(const char * varname)
{
    int CtIndex;

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
    if (! varNameArray[CtIndex].active != TAG_ONDEMAND)
    {
        LOG_PRINT(error_e, "'%s' has not '%c' update type\n", varname, TAG_ONDEMAND);
        return 1;
    }
    pthread_mutex_lock(&datasync_send_mutex);
    {
        varNameArray[CtIndex].visible = 0;
        // NB: may be varStatus_PREPARING or varStatus_DO_WRITE
        // see also io_layer_comm::run()
        if (hmiBlock.states[CtIndex] == varStatus_DO_READ) {
            changeStatusHmiBlock(&hmiBlock, CtIndex, varStatus_NOP);
        }
    }
    pthread_mutex_unlock(&datasync_send_mutex);
    return 0;  // it's ok even if not found
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
                LOG_PRINT(error_e, "Too many decimal places specifyed '%d' by variable '%s'. Max nuber of digit is '%d'\n", decimal, varNameArray[ctIndex].tag, MAX_DECIMAL_DIGIT);
                decimal = MAX_DECIMAL_DIGIT;
            }
            else if (decimal < 0)
            {
                LOG_PRINT(error_e, "The decimal places  number '%d' specyfied by variable '%s' must be greater than '0'\n", decimal, varNameArray[ctIndex].tag);
                decimal = 0;
            }
        }
        else
        {
            LOG_PRINT(error_e, "Invalid decimal places for variable '%d'\n", ctIndex);
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

    prepareWriteVarByCtIndex(ctIndex, value, 1);
    return 0;
}

int intFormattedVarByCtIndex(const int ctIndex, char * formattedVar)
{
    return theValue_string(ctIndex, formattedVar);
}

int doWrite(int ctIndex, void * valuep)
{
    prepareWriteVarByCtIndex(ctIndex, theValue(ctIndex, valuep), 1);
    return 0;
}

int getStatus(int CtIndex)
{
    return plcBlock.states[CtIndex];
}

int addWrite(int ctIndex, void * valuep)
{
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
}
