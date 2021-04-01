/**
 * @file
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

/* ----  Local Defines:   ----------------------------------------------------- */
#define BYTE  char
#define DWORD uint32_t
#define WORD  uint16_t

#define DB_SIZE_ELEM 5472

/**
 * @brief Status description
 */
#define DONE  0x0
#define ERROR 0x1
#define BUSY  0x2
#define UNK   0xF

#define TAG_UNK "UNK"
#define TAG_NAN "Nan"

#define VAR_NAN NAN

#define VAR_UNKNOWN       "E-UNK"
#define VAR_COMMUNICATION "E-COM"
#define VAR_PROGRESS      "E-PRG"
#define VAR_DISABLED      "E-DIS"
#define VAR_DISCONNECT    "E-NCN"

#define ERROR_VALUE 0xFFFF

#define IOLAYER_PERIOD_ms 100
#define DEFAULT_PLUGIN_REFRESH 200

#define NAME_LEN 3
#define DESCR_LEN (128 + 1)
#define TAG_LEN   (31 + 1)
#define LINE_SIZE 4096

#define MAX_DECIMAL_DIGIT 6

#define SEPARATOR ";"

#define LOCAL_DATA_DIR    "/local/data"
#define LOCAL_ETC_DIR      "/local/etc/sysconfig"
#define LOCAL_ROOT_DIR     "/local/root"

#define ALARMS_DIR         LOCAL_DATA_DIR"/alarms"
#define CUSTOM_STORE_DIR   LOCAL_DATA_DIR"/customstore"
#define STORE_DIR          LOCAL_DATA_DIR"/store"
#define RECIPE_DIR         LOCAL_DATA_DIR"/recipe"
#define CUSTOM_TREND_DIR   LOCAL_DATA_DIR"/customtrend"
#define SCREENSHOT_DIR     LOCAL_DATA_DIR"/screenshot"

#define PASSFILE           LOCAL_ETC_DIR"/.pwd"
#define TAG_TABLE          LOCAL_ETC_DIR"/Tags_Table.csv"

#define CROSS_TABLE        LOCAL_ETC_DIR"/Crosstable.csv"
#define CONFIG_FILE        LOCAL_ETC_DIR"/system.ini"
#define NTP_FILE           LOCAL_ETC_DIR"/ntp.ini"

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
    int active;
    int visible;
    int node;
    enum protocol_e protocol;
} variable_t;

typedef struct store_s
{
    char tag[TAG_LEN];
    int CtIndex;
    char value[TAG_LEN];
} store_t;

enum type_e
{
    bit_e,
    byte_e,
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

#define PEN_NB 4
#define PORTRAIT 'P'
#define LANDSCAPE 'L'

#define MAX_SAMPLE_NB 800 // 1024 10240

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

typedef struct event_s
{
    char type;
    //char tag[TAG_LEN];
    char description[DESCR_LEN];
    int level;
    int persistence;
    int dump;
    int filtertime;
    time_t begin; /* this is used to compare with filter time */
    int CtIndex;
} event_t;

#ifdef __cplusplus
}
#endif
#endif
