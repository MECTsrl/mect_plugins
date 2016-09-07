/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Function prototypes for cross-table management
 */
#ifndef _CROSS_TABLE_UTILITY_H
#define _CROSS_TABLE_UTILITY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "protocol.h"

#define LISTENING_PERIOD_US 1000000

#define ADDRESS_MASK 0x1FFF

#define WRITE_MASK           0x4 /* 100 */
#define MULTI_WRITE_MASK     0x5 /* 101 */
#define PREPARE_MASK         0x1 /* 001 */
#define READ_MASK            0x2 /* 010 */
#define WRITE_RCP_MASK       0x6 /* 110 */
#define MULTI_WRITE_RCP_MASK 0x7 /* 111 */

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
size_t loadCt(void);
int Tag2CtIndex(const char * Tag, int * CtIndex);
int CtIndex2Tag(int CtIndex, char * Tag);
int CtIndex2Type(int CtIndex);

extern int setFormattedVar(const char * varname, char * formattedVar);
extern int getString(const char* varname, int size, char * string);
extern int setString(const char* varname, int size, char * string);
extern int getVarDecimal(const int ctIndex);
extern int getVarDecimal(const int ctIndex);
extern int getVarDecimalByName(const char * varname);

extern variable_t varNameArray[1 + DimCrossTable];

extern char CrossTableErrorMsg[256];

#if defined(ENABLE_STORE) || defined(ENABLE_TREND)
extern store_t StoreArrayS[DB_SIZE_ELEM];
extern store_t StoreArrayF[DB_SIZE_ELEM];
extern store_t StoreArrayV[DB_SIZE_ELEM];
extern store_t StoreArrayX[DB_SIZE_ELEM];
extern int store_elem_nb_S;
extern int store_elem_nb_F;
extern int store_elem_nb_V;
extern int store_elem_nb_X;
#endif

void initCtBusy(void);

typedef struct write_queue_elem_s {
    int ctIndex;
    int value;
    struct write_queue_elem_s * next;
} write_queue_elem_t;

#ifdef __cplusplus
}
#endif

#endif

