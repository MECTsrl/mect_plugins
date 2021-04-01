/** 
 * @file
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

size_t fillSyncroArea(void);
int Tag2CtIndex(const char * Tag, int * CtIndex);
int CtIndex2Tag(int CtIndex, char * Tag);
int CtIndex2Type(int CtIndex);
int readFromDb(int ctIndex, void * value);
int readFromDbLock(int ctIndex, void * value);
char readFromDbQuick(int ctIndex, int * ivaluep);
void sprintf_fromValue(char *s, int ctIndex, int value, int decimal, int base);
float sprintf_fromDb(char *s, int ctIndex);
float float_fromValue(int ctIndex, int value, int decimal);
int int_fromValue(int ctIndex, int value, int decimal);
int getHeadBlock(int CtIndex, char * varblockhead);
int getHeadBlockName(const char * varname, char * varblockhead);

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

extern variable_t varNameArray[DB_SIZE_ELEM + 1];

extern char CrossTableErrorMsg[256];

extern store_t StoreArrayS[DB_SIZE_ELEM];
extern store_t StoreArrayF[DB_SIZE_ELEM];
extern store_t StoreArrayV[DB_SIZE_ELEM];
extern store_t StoreArrayX[DB_SIZE_ELEM];
extern int store_elem_nb_S;
extern int store_elem_nb_F;
extern int store_elem_nb_V;
extern int store_elem_nb_X;

extern pthread_mutex_t datasync_recv_mutex;
extern pthread_mutex_t datasync_send_mutex;
extern pthread_cond_t theWritingCondvar;
extern pthread_mutex_t theWritingMutex;
extern pthread_mutex_t alarmevents_list_mutex;

extern char prepareWriteVarByCtIndex(int ctIndex, int value, int execwrite);
#define beginWrite() {}
#define endWrite()   writePendingInorder()
int writePendingInorder();


#ifdef __cplusplus
}
#endif
#endif

