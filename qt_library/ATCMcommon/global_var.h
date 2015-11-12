/** 
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI Main function
 */
#ifndef _GLOBAL_VAR_H_
#define _GLOBAL_VAR_H_

#include <QApplication>
#include <QStringList>
#include <QList>
#include <QHash>
#include <QDateTime>
#include <stdio.h>
#include <QEvent>

#include "common.h"
#include "io_layer_comm.h"
#include "defines.h"
#include "app_var_list.h"
//#include "pagebrowser.h"

/** @brief io layer to syncronize the data with the PLC */
extern io_layer_comm * ioComm;

/* Pages*/
#define HOMEPAGE "menu"

#define PAGE_NAME_LEN 32
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

extern int Buzzerfd;
extern bool BuzzerTouch;
extern bool BuzzerAlarm;
extern struct timespec LastTouch;

/* Communication parameters */
#define MAX_PRIORITY_NUMBER 3
#define MBMAXRETRY_DEFAULT  3
#define MBERRORRATE_DEFAULT 10
#define MBTIMEOUT_DEFAULT  1000
#define MBDELAY_DEFAULT     1000

extern int FailNb;
extern int FailDivisor;
extern int PriorityCounter[MAX_PRIORITY_NUMBER];
extern int HighPriorityTimerMsec;
extern int MediumPriorityTimerMsec;
extern int LowPriorityTimerMsec;

#define BAUDRATE_NB 7
#define STOPBIT_NB 2
#define BITNB_NB 4
#define PARITY_NB 3

#define MBBAUD_DEFAULT      9600
#define MBBITNB_DEFAULT     8
#define MBPARITY_DEFAULT    "none"
#define MBSTOP_DEFAULT      0

#define RTU_DEVICE "/dev/ttySP0"

extern char RTUDevice[FILENAME_MAX];
extern int BaudRate;
extern char Parity;
extern int DataBit;
extern int StopBit;
extern int QueryIntervalRTU;
extern int TimeoutRTU;

extern char IpTCP[32];
extern int PortTCP;
extern int QueryIntervalTCP;
extern int TimeoutTCP;

extern char IpTCPRTU[32];
extern int PortTCPRTU;
extern int QueryIntervalTCPRTU;
extern int TimeoutTCPRTU;

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

#define KINDOFUPDATE_FILE "/local/kindofupdate.lck"

#define VERSION ( \
    (QDate::fromString(__DATE__, "MMM dd yyyy").toString("yyyyMMdd").length() != 0) \
    ? \
    QDate::fromString(__DATE__, "MMM dd yyyy").toString("yyyyMMdd").toAscii().data() \
    : \
    QDate::fromString(__DATE__, "MMM  d yyyy").toString("yyyyMMdd").toAscii().data() \
    )

extern unsigned short int FirstCy;

/* Configuration file Tags */
#define HOME "HomePage:"
#define STARTPAGE "StartPage:"
#define LOG_PERIOD_SLOW "LogPeriodSecS:"
#define LOG_PERIOD_FAST "LogPeriodSecF:"
#define WINDOW_SEC "WindowSizeSec:"
#define PWD1 "Password1:"
#define PWD2 "Password2:"
#define PWD3 "Password3:"
#define SCREENSAVER "ScreenSaverSec:"
#define BUZZER_ALARM "BuzzerAlarm:"
#define BUZZER_TOUCH "BuzzerTouch:"
#define USB_MODE "USBmode:"
#define DEVICE_TAG "Device"
#define HOST_TAG "Host"
#define PWD_TIMEOUT_SEC "PwdTimeoutSec:"
#define PWD_LOGOUT_PAGE "PwdLogoutPage:"
#define MAX_SPACE_AVAILABLE_TAG "SpaceAvailableMb:"

#ifdef ENABLE_ALARMS
typedef struct event_descr_e
{
    char tag[TAG_LEN];
    char description[DESCR_LEN];
    int styleindex;
    QDateTime * begin;
    QDateTime * end;
    QDateTime * ack;
    bool status;
    bool isack;
} event_descr_t;

extern QList<event_descr_e *> _active_alarms_events_;
extern bool HornACK;
extern bool ForceResetAlarmBanner;
#endif //ENABLE_ALARMS

/* USB */
#ifdef ENABLE_USB
extern char usb_mnt_point[FILENAME_MAX];
#endif //ENABLE_USB

#ifdef ENABLE_TRANSLATION
#include <QTranslator>
extern char _language_[3];
extern QTranslator* translator;
#endif //ENABLE_TRANSLATION

#ifdef ENABLE_STORE
extern char _actual_store_[FILENAME_MAX];
#endif //ENABLE_STORE

#ifdef ENABLE_TREND
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
#endif //ENABLE_TREND

#ifdef ENABLE_RECIPE
extern char _actual_recipe_[FILENAME_MAX];
#endif //ENABLE_RECIPE

#ifdef ENABLE_SCREENSAVER
extern int ScreenSaverSec;
#endif //ENABLE_SCREENSAVER

/* Logger */
#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
#define DEFAULT_MAX_MB 5

//extern Logger * logger;
extern int LogPeriodSecS;
extern int LogPeriodSecF;
extern QDateTime StoreInit;
extern QDateTime StoreFinal;
extern int MaxLogUsageMb;
#endif //defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)

#endif
