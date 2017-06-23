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
#include <semaphore.h>

#define LISTENING_PERIOD_US 1000000

#undef AVOID_RECIPES

#define OPER_MASK            0xE000
#define PREPARE_MASK         0x2000
#define READ_MASK            0x4000
#define WRITE_MASK           0x8000
#define MULTI_WRITE_MASK     0xA000
#define WRITE_RCP_MASK       0xC000
#define MULTI_WRITE_RCP_MASK 0xE000
#define ADDRESS_MASK         0x1FFF

#define IS_EMPTY_SYNCRO_FLAG(index)   ((pIOSyncroAreaO[index] & OPER_MASK) == 0x0000)
#define IS_PREPARE_SYNCRO_FLAG(index) ((pIOSyncroAreaO[index] & OPER_MASK) == 0x2000)
#define IS_READ_SYNCRO_FLAG(index)    ((pIOSyncroAreaO[index] & OPER_MASK) == 0x4000)
#define IS_WRITE_SYNCRO_FLAG(index)   ((pIOSyncroAreaO[index] & 0x8000) == 0x8000) /* all writes */
#define SET_SYNCRO_FLAG(index, flag)  { CLR_SYNCRO_FLAG(index); pIOSyncroAreaO[index] |= flag; }
#define CLR_SYNCRO_FLAG(index)        { pIOSyncroAreaO[index] &= ADDRESS_MASK; }

#define beginWrite() {}
#define endWrite()  writePendingInorder()

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
size_t fillSyncroArea(void);
int indexSyncroElement();
int Tag2CtIndex(const char * Tag, int * CtIndex);
int CtIndex2Tag(int CtIndex, char * Tag);
int CtIndex2Type(int CtIndex);
int writeStringToDb(int ctIndex, char * value);
int readFromDb(int ctIndex, void * value);
int readFromDbLock(int ctIndex, void * value);
char readFromDbQuick(int ctIndex, int * ivaluep);
void sprintf_fromValue(char *s, int ctIndex, int value, int decimal, int base);
float sprintf_fromDb(char *s, int ctIndex);
float float_fromValue(int ctIndex, int value, int decimal);
int int_fromValue(int ctIndex, int value, int decimal);
int getHeadBlock(int CtIndex, char * varblockhead);
int getHeadBlockName(const char * varname, char * varblockhead);
int disconnectDevice(enum protocol_e protocol, int node);
int disconnectDeviceByVarname(const char * varname);

int connectDevice(enum protocol_e protocol, int node);
int connectDeviceByVarname(const char * varname);

int isDeviceConnected(enum protocol_e protocol, int node);
int isDeviceConnectedByVarname(const char * varname);
int isDeviceConnectedByCtIndex(int CtIndex);

int setupConnectedDevice(enum protocol_e protocol, int node);
int setupConnectedDeviceByName(const char * protocol, int node);

extern int setFormattedVar(const char * varname, char * formattedVar);
extern int setFormattedVarByCtIndex(const int ctIndex, char * formattedVar);
extern int intFormattedVarByCtIndex(const int ctIndex, char * formattedVar);
extern int deactivateVar(const char * varname);
extern int activateVar(const char * varname);
extern int getVarDecimalByCtIndex(const int ctIndex);
extern int getVarDecimalByName(const char * varname);
extern int doWrite(int ctIndex, void * valuep);
extern int getStatus(int CtIndex);
extern int addWrite(int ctIndex, void * valuep);

/**
 * @brief enable the update of a variable into internal database
 * 
 * @return  	0 if success, otherwise < 0
 */
int fcm_enable_elem(char * varname);
/**
 * @brief disable the update of a variable into internal database
 * 
 * @return  	0 if success, otherwise < 0
 */
int fcm_disable_elem(char * varname);
/**
 * @brief write a value into a variable into internal database
 * 
 * @return  	0 if success, otherwise < 0
 */
int fcm_write_elem(char * varname, elem_t value);

extern BYTE IODataAreaI[STATUS_BASE_BYTE + DB_SIZE_BYTE];
extern BYTE * pIODataAreaI;

extern BYTE IODataAreaO[STATUS_BASE_BYTE + DB_SIZE_BYTE];
extern BYTE * pIODataAreaO;

extern BYTE * pIODataStatusAreaI;
extern BYTE * pIODataStatusAreaO;

extern WORD IOSyncroAreaI[SYNCRO_DB_SIZE_ELEM + 1];
extern WORD * pIOSyncroAreaI;

extern WORD IOSyncroAreaO[SYNCRO_DB_SIZE_ELEM + 1];
extern WORD * pIOSyncroAreaO;

extern size_t SyncroAreaSize;

extern variable_t varNameArray[DB_SIZE_ELEM + 1];

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

extern short int device_status[prot_none_e][MAX_DEVICE_NB];

extern pthread_mutex_t datasync_recv_mutex;
extern pthread_mutex_t datasync_send_mutex;

extern pthread_mutex_t write_queue_mutex;

extern pthread_cond_t theWritingCondvar;
extern pthread_mutex_t theWritingMutex;

extern pthread_mutex_t alarmevents_list_mutex;

extern char prepareWriteVarByCtIndex(int ctIndex, int value, int execwrite);

int writePending();
int writePendingInorder();

void writeVarInQueueByCtIndex(int ctIndex, int value);
void writeVarQueuedByCtIndex(void);

void compactSyncWrites(void);
int  getVarDecimalByName(const char * varname);

typedef struct write_queue_elem_s {
    int ctIndex;
    int value;
    struct write_queue_elem_s * next;
} write_queue_elem_t;


#ifdef __cplusplus
}
#endif
#endif

