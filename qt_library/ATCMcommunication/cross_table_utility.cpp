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

#include "app_logprint.h"
#include "app_var_list.h"
#include "protocol.h"
#include "common.h"
#include "cross_table_utility.h"
#include "utility.h"
#include "global_var.h"

#define RETRY_NB 10

const char *_protocol_map_[] =
{
    TAG_RTU,
    TAG_TCP,
    TAG_TCPRTU,
    ""
};

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
size_t loadCt(void)
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
        if (p[0] == TAG_PLC || p[0] == TAG_STORED_SLOW || p[0] == TAG_STORED_FAST || p[0] == TAG_STORED_ON_VAR || p[0] == TAG_STORED_ON_SHOT)
        {
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
            varNameArray[elem_nb].autoupdate = 1;
        }
        else
        {
            LOG_PRINT(verbose_e, "NORMAL ELEMENT %d [%s]\n", elem_nb, line);
            varNameArray[elem_nb].autoupdate = 0;
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

        if (strcmp(p, "UINT") == 0 || strcmp(p, "UINTAB") == 0)
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
        else if (strcmp(p, "FABCD") == 0 || strcmp(p, "FLOAT") == 0 || strcmp(p, "RABCD") == 0 || strcmp(p, "REAL") == 0)
        {
            varNameArray[elem_nb].type = fabcd_e;
        }
        else if (strcmp(p, "FBADC") == 0 || strcmp(p, "RBADC") == 0)
        {
            varNameArray[elem_nb].type = fbadc_e;
        }
        else if (strcmp(p, "FCDAB") == 0 || strcmp(p, "RCDAB") == 0)
        {
            varNameArray[elem_nb].type = fcdab_e;
        }
        else if (strcmp(p, "FDCBA") == 0 || strcmp(p, "RDCBA") == 0)
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
        if (elem_nb > 0 && varNameArray[elem_nb].block == varNameArray[elem_nb - 1].block)
        {
            varNameArray[elem_nb].blockhead = varNameArray[elem_nb - 1].blockhead;
        }
        else
        {
            varNameArray[elem_nb].blockhead = elem_nb;
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
        /* set reading */
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
        LOG_PRINT(error_e, "no variable name, please report bug.\n");
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
    LOG_PRINT(error_e, "Variable '%s' not found in crosstable\n", Tag);
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

int getString(const char* varname, int size, char *string)
{
    int ctIndex = -1;
    unsigned short value;
    int i;

    if (Tag2CtIndex(varname, &ctIndex) != 0)
        return 1;

    if (varNameArray[ctIndex].decimal > 0) {
        LOG_PRINT(error_e, "Invalid type\n");

        return 1;
    }

    if (CtIndex2Type(ctIndex) == uintab_e || CtIndex2Type(ctIndex) == uintba_e) {
        if (ioComm->readUdpReply(ctIndex, &value) != 0) {
            LOG_PRINT(error_e, "readFromDb\n");

            return 1;
        }

        if (!(ioComm->getStatusVar(ctIndex, NULL) & (STATUS_BUSY_R | STATUS_BUSY_W | STATUS_OK)))
            return 1;
    }
    else {
        LOG_PRINT(error_e, "Invalid type\n");

        return 1;
    }

    LOG_PRINT(verbose_e, "get var name '%s' size %d\n", varname, size);
    if (Tag2CtIndex(varname, &ctIndex) != 0) {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable %s\n", varname);

        return 1;
    }

    for (i = 0; i < size; i++) {
        if (i % 2 == 0) {
            if (ioComm->readUdpReply(ctIndex, &value) != 0) {
                LOG_PRINT(error_e, "cannot read the variable at ctIndex %d\n", ctIndex);

                return 1;
            }

            LOG_PRINT(verbose_e, "%d value[%d] = %d\n",ctIndex, i, value);

            string[i] = value & 0xFF;
            string[i+1] = (value >> 8) & 0xFF;

            ctIndex++;
        }
    }
    string[size]='\0';

    LOG_PRINT(verbose_e, "get string '%s'\n", string);

    return 0;
}

int setString(const char *varname, int size, char *string)
{
#if 0 // FIXME MTL
    char tag[TAG_LEN];
    int ctIndex = -1;
    unsigned short value;
    int i;

    if (Tag2CtIndex(varname, &ctIndex) != 0) {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable %s\n", varname);

        return 1;
    }

    for (i = 0; i < size; i++) {
        if (i % 2 == 0) {
            if (CtIndex2Tag(ctIndex, tag) != 0) {
                LOG_PRINT(error_e, "cannot extract tag name from variable at ctIndex %d\n", ctIndex);

                return 1;
            }

            value = (string[i] | (string[i+1] << 8));
            LOG_PRINT(verbose_e, "'%c' '%c' -> %x %x -> %x\n", string[i], string[i+1], string[i], string[i+1], value);

            if (!(prepareWriteBlock(tag, &value, NULL) & STATUS_OK)) {
                LOG_PRINT(error_e, "cannot prepare the write value into variable %s\n", tag);

                return 1;
            }

            ctIndex++;
        }
    }

    if (writeBlock(varname) != 0) {
        LOG_PRINT(error_e, "cannot perform the write of the block %s\n", varname);

        return 0;
    }

    LOG_PRINT(verbose_e, "set string '%s'\n", string);
#endif
    return 0;
}

int getVarDecimal(const int ctIndex)
{
    int decimal = 0;

    decimal = varNameArray[ctIndex].decimal;
    if (decimal > 4) {
        //We have a dependent decimal
        if (decimal < ctIndex) {
            if (ioComm->readUdpReply(varNameArray[ctIndex].decimal, &decimal) != 0) {
                decimal = 0;
            }
            else if (decimal > MAX_DECIMAL_DIGIT) {
                LOG_PRINT(error_e, "Too many decimal nuber specifyed '%d' by variable '%s'. Max nuber of digit is '%d'\n", decimal, varNameArray[ctIndex].tag, MAX_DECIMAL_DIGIT);

                decimal = MAX_DECIMAL_DIGIT;
            }
            else if (decimal < 0) {
                LOG_PRINT(error_e, "The decimal digit number '%d' specyfied by variable '%s' must be greater than '0'\n", decimal, varNameArray[ctIndex].tag);

                decimal = 0;
            }
        }
        else {
            LOG_PRINT(error_e, "Invalid decimal specification for variable '%d'\n", ctIndex);

            decimal = 0;
        }
    }

    return decimal;
}

int getVarDecimalByName(const char *varname)
{
    int ctIndex;
    int decimal = 0;

    if (Tag2CtIndex(varname, &ctIndex) != 0) {
        LOG_PRINT(error_e, "cannot extract ctIndex for variable '%s'\n", varname);

        return decimal;
    }

    decimal = getVarDecimal(ctIndex);

    return decimal;
}

void initCtBusy(void)
{
    int i;
    for (i = 0; i < DimCrossTable; i++)
    {
        varNameArray[i].blockbusy = 0;
    }
}
