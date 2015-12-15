/** 
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Common function and structure
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#include <string.h>
#include <stdint.h>
#include "app_var_list.h"

/* ----  Configuration Defines:   ----------------------------------------------------- */
#define ENABLE_SCREENSAVER /* enable screensaver management */
#define ENABLE_USB         /* enable USB management */
#define ENABLE_INPUTPAD    /* enable numpad and alphanumpad */
#define TARGET             /* enable compilation for target ARM */
#undef  DUMPSCREEN         /* enable the screen shot of the screen when new page is shown */
#undef  COMPACT_ALARM      /* show the start and the end of an alarm in the same line */
#undef  ENABLE_AUTODUMP    /* enable the automatic dump when you insert the USB key */
#define ENABLE_DEVICE_DISCONNECT /* enable the device disconnected management */
#define ENABLE_TRANSLATION

/* this compilation warnig is shown cause the DUMPSCREEN option is quite heavy, so if it is not necessary is better turn it off */
#ifdef DUMPSCREEN
#warning DUMPSCREEN ON
#endif

/* ----  Local Defines:   ----------------------------------------------------- */

#define HW119_MAX_FIELD_LENGHT 256
#define HW119_MAX_LINE_LEN (HW119_MAX_FIELD_NB * HW119_MAX_FIELD_LENGHT)
#define HW119_SEPARATOR ";"

#define BYTE  char
#define DWORD uint32_t
#define WORD  uint16_t

/**
 * calculate the address
 */
#define BIT_OCCUPATION_BYTE 4
#define REG_OCCUPATION_BYTE 4

/*
 * 4Control reserved area (this area is used by 4C to sign the size of the array)
 */
#define FC_RESERVED_BASE_BYTE   0
#define FC_RESERVED_SIZE_BYTE   4
/**
 * Retentive Register
 */
#define RET_REG_BASE_BYTE   FC_RESERVED_SIZE_BYTE
#define RET_REG_SIZE_BYTE   RET_REG_NB * REG_OCCUPATION_BYTE
/**
 * Retentive Bit
 */
#define RET_BIT_BASE_BYTE   RET_REG_BASE_BYTE + RET_REG_SIZE_BYTE
#define RET_BIT_SIZE_BYTE   RET_BIT_NB * BIT_OCCUPATION_BYTE
/**
 * Mirror Register
 */
#define MIR_REG_BASE_BYTE   RET_BIT_BASE_BYTE + RET_BIT_SIZE_BYTE
#define MIR_REG_SIZE_BYTE   MIR_REG_NB * REG_OCCUPATION_BYTE
/**
 * Retentive Bit
 */
#define MIR_BIT_BASE_BYTE   MIR_REG_BASE_BYTE + MIR_REG_SIZE_BYTE
#define MIR_BIT_SIZE_BYTE   MIR_BIT_NB * BIT_OCCUPATION_BYTE
/**
 * Non Retentive Bit
 */
#define NRE_BIT_BASE_BYTE   MIR_BIT_BASE_BYTE + MIR_BIT_SIZE_BYTE
#define NRE_BIT_SIZE_BYTE   NRE_BIT_NB * BIT_OCCUPATION_BYTE
/**
 * Non Retentive Register
 */
#define NRE_REG_BASE_BYTE   NRE_BIT_BASE_BYTE + NRE_BIT_SIZE_BYTE
#define NRE_REG_SIZE_BYTE   NRE_REG_NB * REG_OCCUPATION_BYTE

#define DB_SIZE_ELEM ( RET_REG_NB + RET_BIT_NB + MIR_REG_NB + MIR_BIT_NB + NRE_BIT_NB + NRE_REG_NB + 1)
#define DB_SIZE_BYTE ( FC_RESERVED_SIZE_BYTE + RET_REG_SIZE_BYTE + RET_BIT_SIZE_BYTE + MIR_REG_SIZE_BYTE + MIR_BIT_SIZE_BYTE + NRE_BIT_SIZE_BYTE + NRE_REG_SIZE_BYTE )

/**
 * Base input array into PLC IO layer 0 (ioData)
 */
#define DB_IN_BASE_BYTE  0

/**
 * Base output array into PLC IO layer 0 (ioData)
 */
#define DB_OUT_BASE_BYTE 0

/**
 * Base status array into PLC IO layer 0 (ioData)
 */
//#define STATUS_BASE_BYTE    (DB_SIZE_BYTE) * 2 + 2 /* + 2 cause 4C reserved area for array size */
#define STATUS_BASE_BYTE 22000

/**
 * Base synchronization array into PLC IO layer 1 (ioSyncro)
 */
#define SYNCRO_BASE_BYTE 0
// #define SYNCRO_SIZE_BYTE (DB_SIZE_ELEM * 2)
#define SYNCRO_EXCHANGE_BASE_BYTE	11000 /*The Queue array fills the syncro Io starting from 0 to 10946*/	
#define SYNCRO_EXCHANGE_DWORD_SIZE	4 /*Exported var is a DWORD*/
#define SYNCRO_EXCHANGE_DWORD_NB	12 /*Number of DWORD exported var*/
#define SYNCRO_EXCHANGE_WORD_SIZE	2 /*Exported var is a WORD*/
#define SYNCRO_EXCHANGE_WORD_NB		207 /*Number of WORD exported var*/
#define SYNCRO_EXCHANGE_SIZE_BYTE	( SYNCRO_EXCHANGE_DWORD_SIZE * SYNCRO_EXCHANGE_DWORD_NB +  SYNCRO_EXCHANGE_WORD_SIZE * SYNCRO_EXCHANGE_WORD_NB)
#define SYNCRO_SIZE_BYTE 		( SYNCRO_EXCHANGE_BASE_BYTE + SYNCRO_EXCHANGE_SIZE_BYTE )
#define SYNCRO_DB_SIZE_ELEM		SYNCRO_SIZE_BYTE/2

#define IS_MIRROR(index)    (index >= (RET_REG_NB + RET_BIT_NB) && index < (RET_REG_NB + RET_BIT_NB + MIR_BIT_NB + MIR_REG_NB))
#define IS_RETENTIVE(index) (index >= 0                         && index < (RET_REG_NB + RET_BIT_NB))

#define GET_FLAG(data, flag) ((((data) >> (flag)) & 0x1) == 0x1)
#define SET_FLAG(data, flag) { (data) =  (  (data)  | (0x1 << (flag)));} 
#define CLR_FLAG(data, flag) { (data) = ~((~(data)) | (0x1 << (flag)));}

#define SET_SIZE_BYTE(area, size) { \
	(area)[1] = ((size) >> 8); \
	(area)[0] = ((size) & 0x00FF); \
}

#define SET_SIZE_WORD(area, size) { \
	(area)[0] = (size); \
}
/*
Used when getting a value on the IOSyncroArea which is a WORD area.
The values are defined as DWORD on the plc side
Index is the index used to access the IOSyncroArea as array of byte hence to access it as WORD it must halved
*/
#define GET_DWORD_FROM_WORD(value, area, index) { \
    value = ((area)[(index)/2] + ((area)[(index)/2 + 1] << 16)); \
}

#define SET_WORD_FROM_DWORD(value, area, index) { \
     (area)[(index)/2] = ((value) & 0x0000FFFF); \
     (area)[(index)/2 + 1] = (((value) & 0xFFFF0000) >> 16); \
}


/*
Used when getting a value on the IOSyncroArea which is a WORD area.
The values are defined as WORD on the plc side
Index is the index used to access the IOSyncroArea as array of byte hence to access it as WORD it must halved
*/
#define GET_WORD_FROM_WORD(value, area, index) { \
	value = ((area)[(index)/2] ); \
}
#define SET_WORD_FROM_WORD(value, area, index) { \
	((area)[(index)/2] ) = value; \
}

#define WRITE_IRQ_VAR SYNCRO_EXCHANGE_BASE_BYTE //11000
#define WRITE_IRQ_ON	0x1
#define WRITE_IRQ_OFF	0x0

#define RESET_ERROR_ON	0x1
#define RESET_ERROR_OFF	0x0

#define PLC_MAIN_REV 			( SYNCRO_EXCHANGE_BASE_BYTE + SYNCRO_EXCHANGE_WORD_SIZE ) //11002
#define PLC_MINOR_REV 			( SYNCRO_EXCHANGE_BASE_BYTE + ( 2 * SYNCRO_EXCHANGE_WORD_SIZE ) ) //11004
#define RESET_RTU_ERROR			( SYNCRO_EXCHANGE_BASE_BYTE + ( 3 * SYNCRO_EXCHANGE_WORD_SIZE ) ) //11006
#define RESET_TCP_ERROR 		( SYNCRO_EXCHANGE_BASE_BYTE + ( 4 * SYNCRO_EXCHANGE_WORD_SIZE ) ) //11008
#define RESET_TCPRTU_ERROR 		( SYNCRO_EXCHANGE_BASE_BYTE + ( 5 * SYNCRO_EXCHANGE_WORD_SIZE ) ) //11010
#define ERROR_COUNTER_NUMBER		64
#define COUNTER_RTU_ERROR  		( SYNCRO_EXCHANGE_BASE_BYTE + ( 6 * SYNCRO_EXCHANGE_WORD_SIZE ) ) //11012
#define COUNTER_TCP_ERROR  		( COUNTER_RTU_ERROR + ( ERROR_COUNTER_NUMBER * SYNCRO_EXCHANGE_WORD_SIZE ) + 2 ) //11142
#define COUNTER_TCPRTU_ERROR  		( COUNTER_TCP_ERROR  + ( ERROR_COUNTER_NUMBER * SYNCRO_EXCHANGE_WORD_SIZE ) + 2 ) //11272
#define BLACKLIST_RTU_ERROR_WORD  	( COUNTER_TCPRTU_ERROR  + ( ERROR_COUNTER_NUMBER * SYNCRO_EXCHANGE_WORD_SIZE ) + 2) //11402
#define OTHER_RTU_ERROR_WORD  		( BLACKLIST_RTU_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE )  //11404
#define BLACKLIST_TCP_ERROR_WORD 	( OTHER_RTU_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE )  //11406
#define OTHER_TCP_ERROR_WORD  		( BLACKLIST_TCP_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE )  //11408
#define BLACKLIST_TCPRTU_ERROR_WORD 	( OTHER_TCP_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE )  //11410
#define OTHER_TCPRTU_ERROR_WORD  	( BLACKLIST_TCPRTU_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE )  //11412
#define ERRORS_SUMMARY				( OTHER_TCPRTU_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE ) //11414
#define ERROR_BIT_DWORD_NUMBER		2
#define BLACKLIST_RTU_BIT_ERROR  	( OTHER_TCPRTU_ERROR_WORD  + SYNCRO_EXCHANGE_WORD_SIZE + 2 ) //11416
#define OTHER_RTU_BIT_ERROR  		( BLACKLIST_RTU_BIT_ERROR  + ERROR_BIT_DWORD_NUMBER * SYNCRO_EXCHANGE_DWORD_SIZE ) //11420
#define BLACKLIST_TCP_BIT_ERROR  	( OTHER_RTU_BIT_ERROR  + ERROR_BIT_DWORD_NUMBER * SYNCRO_EXCHANGE_DWORD_SIZE ) //11424
#define OTHER_TCP_BIT_ERROR  		( BLACKLIST_TCP_BIT_ERROR  + ERROR_BIT_DWORD_NUMBER * SYNCRO_EXCHANGE_DWORD_SIZE ) //11428
#define BLACKLIST_TCPRTU_BIT_ERROR  	( OTHER_TCP_BIT_ERROR  + ERROR_BIT_DWORD_NUMBER * SYNCRO_EXCHANGE_DWORD_SIZE ) //11432
#define OTHER_TCPRTU_BIT_ERROR  	( BLACKLIST_TCPRTU_BIT_ERROR  + ERROR_BIT_DWORD_NUMBER * SYNCRO_EXCHANGE_DWORD_SIZE ) //11436

#define IS_RTU_ERROR ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x01) == 0x01)
#define IS_TCP_ERROR ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x02) == 0x02)
#define IS_TCPRTU_ERROR ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x04) == 0x04)
#define IS_COMMPAR_ERROR ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x08) == 0x08)
#define IS_ALARMSTBL_ERROR ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x10) == 0x10)
#define IS_CROSSTABLE_ERROR ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x20) == 0x20)
#define IS_ENGINE_READY ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x40) == 0x40)
#define IS_RTU_ENABLED ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x80) == 0x80)
#define IS_TCP_ENABLED ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x100) == 0x100)
#define IS_TCPRTU_ENABLED ((((IOSyncroAreaI)[(ERRORS_SUMMARY)/2]) & 0x200) == 0x200)

/**
 * @brief Status description
 */
#define DONE  0x0
#define ERROR 0x1
#define BUSY  0x2
#define UNK  0xF

#define TAG_UNK "UNK"
#define TAG_NAN "Nan"

#define VAR_NAN NAN

#define VAR_UNKNOWN       "E-UNK"
#define VAR_COMMUNICATION "E-COM"
#define VAR_PROGRESS      "E-PRG"
#define VAR_DISABLED      "E-DIS"
#define VAR_DISCONNECT    "E-NCN"

/**
 * @brief Not A Number value is the value if there is some error
 */
#define ERROR_VALUE 0xFFFF
/**
 * @brief Not A Bit value is the bit offset value it the element is not a bit
 */
#define NAB 0xFF

#define LOCAL_SERVER_DATA_RX_PORT 34902
#define LOCAL_SERVER_DATA_TX_PORT 34903
#define LOCAL_SERVER_SYNCRO_RX_PORT 34904
#define LOCAL_SERVER_SYNCRO_TX_PORT 34905
#define LOCAL_SERVER_ADDR "127.0.0.1"

#define IOLAYER_PERIOD_ms 100
#define DEFAULT_REFRESH 500
#define WAITING_PERIOD_US 10000

#define MAX_MSG 1024
#define DATA_SIZE (2 * sizeof(char))
#define NAME_LEN 3
#define DESCR_LEN 64
#define TAG_LEN   (16 + 1)
//#define TAG_LEN   32
#define LINE_SIZE 1024

#define MAX_DECIMAL_DIGIT 6

#define SEPARATOR ";"

#define LOCAL_DATA_DIR    "/local/data"
#define LOCAL_ETC_DIR     "/local/etc/sysconfig"
#define LOCAL_ROOT_DIR     "/local/root"

#define ALARMS_DIR         LOCAL_DATA_DIR"/alarms"
#define CUSTOM_STORE_DIR   LOCAL_DATA_DIR"/customstore"
#define STORE_DIR          LOCAL_DATA_DIR"/store"
#define RECIPE_DIR         LOCAL_DATA_DIR"/recipe"
#define CUSTOM_TREND_DIR   LOCAL_DATA_DIR"/customtrend"
#define SCREENSHOT_DIR     LOCAL_DATA_DIR"/screenshot"

/* WARNING this value is used also into the run time, so if you change the value here you must change it also into vmLib/libHW119.c */
#define CROSS_TABLE        LOCAL_ETC_DIR"/Crosstable.csv"
#define ERROR_TABLE        LOCAL_ETC_DIR"/Alarms.csv"
#define COMMUNICATION_FILE LOCAL_ETC_DIR"/Commpar.csv"
#define CONFIG_FILE        LOCAL_ETC_DIR"/system.ini"
#define PASSFILE           LOCAL_ETC_DIR"/.pwd"
#define TAG_TABLE          LOCAL_ETC_DIR"/Tags_Table.csv"

#define APP_SIGN           "/usr/bin/sign"
#define ZIP_BIN            "/usr/bin/zip"

#define TAG_ONDEMAND    'H'
#define TAG_PLC         'P'
#define TAG_STORED_SLOW 'S'
#define TAG_STORED_FAST 'F'

#define HIGH_PRIORITY	1
#define MEDIUM_PRIORITY	2
#define LOW_PRORITY	3

#define MAX_DEVICE_NB 64

#define TAG_RTU    	"RTU"
#define TAG_TCP    	"TCP"
#define TAG_TCPRTU 	"TCPRTU"
#define TAG_BLACKLIST 	"TIMEOUT"
#define TAG_OTHER	"COMMUNICATION"

#define INTERNAL_VARIABLE_FAKE_NODEID	256
enum protocol_e
{
	prot_rtu_e = 0,
	prot_tcp_e,
	prot_tcprtu_e,
	prot_none_e
};

enum error_kind_e
{
    error_blacklist_e = 0,
    error_other_e
};

typedef struct udp_msg
{
	unsigned short int dataLen;
	char type;
	char elemNb;
	//char Data[MAX_MSG];
	char Data[1];
} udp_msg_t;

typedef struct var_s
{
	char name[NAME_LEN];
	int status;
	char value[2];
} var_t;

typedef struct variable_s
{
	char tag[TAG_LEN];
	int type;
    int block;
    int blockhead;
    int decimal;
    int active;
    int visible;
    int node;
	enum protocol_e protocol;
} variable_t;

#if defined(ENABLE_TREND) || defined(ENABLE_STORE)
typedef struct store_s
{
    char tag[TAG_LEN];
    int CtIndex;
} store_t;
#endif

enum type_e
{
    bit_e,
    uintab_e,
    uintba_e,
    intab_e,
    intba_e,
    udint_abcd_e,
	udint_badc_e,
	udint_cdab_e,
	udint_dcba_e,
	dint_abcd_e,
	dint_badc_e,
	dint_cdab_e,
	dint_dcba_e,
	fabcd_e,
	fbadc_e,
	fcdab_e,
    fdcba_e,
    bytebit_e,
    wordbit_e,
    dwordbit_e
};

#if defined(ENABLE_TREND)
#define PEN_NB 4
#define PORTRAIT 'P'
#define LANDSCAPE 'L'
#endif

/* communication comand */
#define ENABLE_RQST   'E'
#define DISABLE_RQST  'D'
#define WRITE_RQST    'W'
#define READ_RQST     'R'
/* service command */
#define PING_RQST     'P'
#define RELOAD_RQST   'U'

#define ABCD2FLOAT(doubleword) (doubleword)
#define BADC2FLOAT(doubleword) ((((doubleword) & 0x000F) << 4)  | (((doubleword) & 0x00F0) >> 4) | (((doubleword) & 0x0F00) << 4) | (((doubleword) & 0xF000) >> 4))
#define CDAB2FLOAT(doubleword) ((((doubleword) & 0x000F) << 8)  | (((doubleword) & 0x00F0) << 8) | (((doubleword) & 0x0F00) >> 8) | (((doubleword) & 0xF000) >> 8))
#define DCBA2FLOAT(doubleword) ((((doubleword) & 0x000F) << 12) | (((doubleword) & 0x00F0) << 4) | (((doubleword) & 0x0F00) >> 4) | (((doubleword) & 0xF000) >> 12))

#define FLOAT2ABCD(doubleword) ABCD2FLOAT(doubleword)
#define FLOAT2BADC(doubleword) BADC2FLOAT(doubleword)
#define FLOAT2CDAB(doubleword) CDAB2FLOAT(doubleword)
#define FLOAT2DCBA(doubleword) DCBA2FLOAT(doubleword)

/**
 * @brief type bits
 */
struct bits { 
	unsigned char bit0 : 1;
	unsigned char bit1 : 1;
	unsigned char bit2 : 1;
	unsigned char bit3 : 1;
	unsigned char bit4 : 1;
	unsigned char bit5 : 1;
	unsigned char bit6 : 1;
	unsigned char bit7 : 1;
	unsigned char bit8 : 1;
	unsigned char bit9 : 1;
	unsigned char bi10 : 1;
	unsigned char bit11 : 1;
	unsigned char bit12 : 1;
	unsigned char bit13 : 1;
	unsigned char bit14 : 1;
	unsigned char bit15 : 1;
};

/**
 * @brief type elem. this union allow to use this elem_t as word, and bit
 */
typedef union {
	unsigned int word;
	char byte[2];
	struct bits bits;
} elem_t;

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
/**
 * @brief Log setup and define
 */
#define SAMPLE_PERIOD_SEC LOG_PERIOD_MS
#define MAX_SAMPLE_NB 10240
#endif

#ifdef ENABLE_ALARMS
#define EVENT 0
#define ALARM 1

enum alarm_event_e
{
	alarm_fall_e,
	alarm_rise_e,
	alarm_ack_e,
	alarm_none_e
};

#define DUMP   1
#define NODUMP 0

#define TAG_FALL "FALL"
#define TAG_RISE "RISE"
#define TAG_ACK "ACK"

#define TAG_EVENT "E"
#define TAG_ALARM "A"

typedef struct event_s
{
	char type;
	//char tag[TAG_LEN];
	char description[DESCR_LEN];
	int level;
	int persistence;
	int dump;
	int filtertime;
	time_t begin; /* this i s used to compare with filter time */
	int CtIndex;
} event_t;
#endif

#ifdef __cplusplus
}
#endif
#endif

