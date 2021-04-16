/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI Main function
 */

#include "global_var.h"
#include "app_usb.h"

#ifdef TARGET
#include <QWSServer>
#include "screensaver.h"
int ScreenSaverSec = 0;
#endif //TARGET

/** @brief io layer to syncronize the data with the PLC */
io_layer_comm * ioComm;

/* Pages */
char HomePage[PAGE_NAME_LEN] = "";
char StartPage[PAGE_NAME_LEN] = "";
char PwdLogoutPage[PAGE_NAME_LEN] = "";
int PwdTimeoutSec = 0;

/* Password */
/**
 * @brief value and description Passwords array and actual password level
 */
int passwords[PASSWORD_NB + 1] = {0,0,0,0};
char PasswordsString[PASSWORD_NB + 1][TAG_LEN] = {"Admin", "SuperUser", "User", "None"};
int active_password = pwd_operator_e;

/* Logger */
int LogPeriodSecS = LOG_PERIOD_MS / 1000;
int LogPeriodSecF = LOG_PERIOD_MS / 1000;
int MaxWindowSec = 60;
char StoreInit[32];
char StoreFinal[32];
int MaxLogUsageMb = MAX_SPACE_AVAILABLE_DEF;

QList<event_descr_e *> _active_alarms_events_;
bool HornACK = false;
bool ForceResetAlarmBanner = false;

/* Buzzer */
int Buzzerfd = -1;
bool BuzzerTouch = true;
bool BuzzerAlarm = true;
QElapsedTimer LastTouch;

/* Trend */
char _layout_ = LANDSCAPE;
char _last_layout_ = LANDSCAPE;
bool _trend_data_reload_ = true;

/* Tag table */
QHash<QString, QString> TagTable;

/** @brief variables used for the USB interface */
#ifdef ENABLE_USB
int USBfeedback[2];
int USBstatus[APP_USB_MAX + 1];
char usb_mnt_point[FILENAME_MAX] = "";
#endif //ENABLE_USB

/* global variables */
unsigned short int FirstCy = 0;

char _language_[3] = DEFAULT_LANGUAGE;
QTranslator* translator;

char _actual_store_[FILENAME_MAX] = "";

QStringList _current_trend_variables_;
char _actual_trend_[FILENAME_MAX] = "";
pen_t pens[PEN_NB + 1];
int actualPen = 0;
bool _online_ = true;
/* first time sample of visible data actually */
QDateTime actualTzero;
/* windows size in seconds of visible data actually */
int actualVisibleWindowSec = 0;

char _actual_recipe_[FILENAME_MAX] = "";

QHash<char *, int *> NameMap;

QStringList userPageList;

QString HMIversion = QString("mect_plugins v%1.%2.%3").arg(MECT_BUILD_MAJOR).arg(MECT_BUILD_MINOR).arg(MECT_BUILD_BUILD);
