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

/* ----  Configuration Defines:   ----------------------------------------------------- */
#define ENABLE_SCREENSAVER /* enable screensaver management */
#define ENABLE_USB         /* enable USB management */
#define TARGET             /* enable compilation for target ARM */
#undef  DUMPSCREEN         /* enable the screen shot of the screen when new page is shown */
#undef  COMPACT_ALARM      /* show the start and the end of an alarm in the same line */
#undef  ENABLE_AUTODUMP    /* enable the automatic dump when you insert the USB key */
#define ENABLE_TRANSLATION

/* this compilation warnig is shown cause the DUMPSCREEN option is quite heavy, so if it is not necessary is better turn it off */
#ifdef DUMPSCREEN
#warning DUMPSCREEN ON
#endif

/* ----  Local Defines:   ----------------------------------------------------- */
#define BYTE  char
#define DWORD uint32_t
#define WORD  uint16_t

#define DB_SIZE_ELEM 5472

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

#define LOCAL_SERVER_ADDR "127.0.0.1"

#define IOLAYER_PERIOD_ms 100

#define NAME_LEN 3
#define DESCR_LEN 64
#define TAG_LEN   (16 + 1)
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

#define PASSFILE           LOCAL_ETC_DIR"/.pwd"
#define TAG_TABLE          LOCAL_ETC_DIR"/Tags_Table.csv"

/* WARNING this value is used also into the run time, so if you change the value here you must change it also into vmLib/libHW119.c */
#define CROSS_TABLE        LOCAL_ETC_DIR"/Crosstable.csv"
#define COMMUNICATION_FILE LOCAL_ETC_DIR"/Commpar.csv"
#define CONFIG_FILE        LOCAL_ETC_DIR"/system.ini"

#define BACKLIGHT_FILE_SYSTEM "/sys/devices/platform/mxs-bl.0/backlight/mxs-bl/brightness"

#define APP_SIGN           "/usr/bin/sign"
#define ZIP_BIN            "/usr/bin/zip"

#define TAG_ONDEMAND       'H'
#define TAG_PLC            'P'
#define TAG_STORED_SLOW    'S'
#define TAG_STORED_FAST    'F'
#define TAG_STORED_ON_VAR  'V'
#define TAG_STORED_ON_SHOT 'X'

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

typedef struct variable_s
{
    char tag[TAG_LEN];
    int type;
    int block;
    int blockhead;
    int decimal;
    int autoupdate;
    int node;
    enum protocol_e protocol;
    int blockbusy;
} variable_t;

#if defined(ENABLE_TREND) || defined(ENABLE_STORE)
typedef struct store_s
{
    char tag[TAG_LEN];
    int CtIndex;
    char value[TAG_LEN];
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

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
/**
 * @brief Log setup and define
 */
#define MAX_SAMPLE_NB 10240
#endif

#ifdef ENABLE_ALARMS

#define EVENT 0
#define TAG_EVENT "E"

#define ALARM 1
#define TAG_ALARM "A"

enum alarm_event_e
{
    alarm_fall_e,
    alarm_rise_e,
    alarm_ack_e,
    alarm_none_e
};

#define TAG_FALL "FALL"
#define TAG_RISE "RISE"
#define TAG_ACK  "ACK"

#define DUMP   1
#define NODUMP 0

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
