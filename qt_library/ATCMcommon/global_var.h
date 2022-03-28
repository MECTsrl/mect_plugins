/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI Main function
 */
#ifndef _GLOBAL_VAR_H_
#define _GLOBAL_VAR_H_
#include <stdio.h>
#include <QDateTime>
#include <QTime>
#include <QEvent>
#include <QHash>
#include <QElapsedTimer>
#include "defines.h"
#include "io_layer_comm.h"

/** @brief io layer to syncronize the data with the PLC */
extern io_layer_comm * ioComm;

// IP
#define NO_IP     "-"
#define NO_IP_LEN 1
#define ZERO_IP "0.0.0.0"

/* Pages*/
extern char StartPage[PAGE_NAME_LEN];
extern char HomePage[PAGE_NAME_LEN];
extern char PwdLogoutPage[PAGE_NAME_LEN];

/* Password */
#define PASSWORD_NB 3
enum password_level_e
{
    pwd_admin_e,
    pwd_super_user_e,
    pwd_user_e,
    pwd_operator_e
};
extern int PwdTimeoutSec;
extern char PasswordsString[PASSWORD_NB + 1][TAG_LEN];
extern int passwords[PASSWORD_NB + 1];
extern int active_password;

/* buzzer */
#define BUZZER      "/dev/buzzer"
#include <linux/mxs-buzzer.h>
#define BUZZER_DURATION_MS 120

#define ID_PLC_time         5390

#define ID_PLC_Version      5394

#define ID_PLC_buzzerOn     5397
#define ID_PLC_PLC_Version  5398
#define ID_PLC_HMI_Version  5399

#define ID_PLC_UPTIME_s      5416
#define ID_PLC_UPTIME_cs     5417
#define ID_PLC_PRODUCT_ID    5420
#define ID_PLC_SERIAL_NUMBER 5421
#define ID_PLC_HMI_PAGE      5422
#define ID_PLC_MS_VERSION    5423

#define ID_PLC_BEEP_VOLUME  5435
#define ID_PLC_TOUCH_VOLUME 5436
#define ID_PLC_ALARM_VOLUME 5437
#define ID_PLC_BUZZER       5438

extern int Buzzerfd;
extern bool BuzzerTouch;
extern bool BuzzerAlarm;
extern QElapsedTimer LastTouch;

extern int MaxWindowSec;

enum parity_e
{
    parity_none_e,
    parity_even_e,
    parity_odd_e
};

extern int BaudRateArray[7];
extern int StopBitArray[2];
extern int BitNbArray[4];
extern char ParityArray[3][8];
extern char ParityArrayShort[3];

/* Tag table */
extern QHash<QString, QString> TagTable;

/* SD card management */
#define STORAGE_PERMISSION "extrastorage"
#define APPLICATION_PERMISSION "application"
#define LICENCE_FILE ".licence"
#define SD_MOUNT_POINT "/local/sd_card"

#define VERSION ( \
    (QDate::fromString(__DATE__, "MMM dd yyyy").toString("yyyyMMdd").length() != 0) \
    ? \
    QDate::fromString(__DATE__, "MMM dd yyyy").toString("yyyyMMdd").toAscii().data() \
    : \
    QDate::fromString(__DATE__, "MMM  d yyyy").toString("yyyyMMdd").toAscii().data() \
    )

extern unsigned short int FirstCy;

typedef struct event_descr_e
{
    char tag[TAG_LEN];
    char description[DESCR_LEN];
    int styleindex;
    QDateTime begin;
    QDateTime end;
    QDateTime ack;
    enum alarm_event_e status;
    bool isack;
    char type;
} event_descr_t;

extern QList<event_descr_e *> _active_alarms_events_;
extern bool HornACK;
extern bool ForceResetAlarmBanner;

/* USB */
#ifdef ENABLE_USB
extern char usb_mnt_point[FILENAME_MAX];
#endif //ENABLE_USB

#include <QTranslator>
extern char _language_[3];
extern QTranslator* translator;

extern char _actual_store_[FILENAME_MAX];

#include "qwt_plot_curve.h"

/* Trend */
extern char _layout_;
extern char _last_layout_;
extern bool _trend_data_reload_;

typedef struct pen_s
{
    int visible;
    int CtIndex;
    char tag[TAG_LEN];
    char color[TAG_LEN];
    char description[DESCR_LEN];
    float yMin;
    float yMax;
    float yMinActual;
    float yMaxActual;
    double *y;
    double *x;
    int sample;
    QwtPlotCurve * curve;
} pen_t;

extern QStringList _current_trend_variables_;
extern char _actual_trend_[FILENAME_MAX];
extern pen_t pens[PEN_NB + 1];
extern int actualPen;
extern char _layout_;
extern char _last_layout_;
extern bool _online_;
/* first time sample of visible data actually */
extern QDateTime actualTzero;
/* windows size in seconds of visible data actually */
extern int actualVisibleWindowSec;
extern bool _trend_data_reload_;

#define MAX_RCP_STEP 64
#define MAX_RCP_VAR  200
extern char _actual_recipe_[FILENAME_MAX];

extern int ScreenSaverSec;

/* Logger */

//extern Logger * logger;
extern int LogPeriodSecS;
extern int LogPeriodSecF;
extern char StoreInit[32];
extern char StoreFinal[32];
extern int MaxLogUsageMb;

extern QStringList userPageList;

extern QString HMIversion;

#endif
