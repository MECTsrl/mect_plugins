/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
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
#ifdef ENABLE_SCREENSAVER
#include "screensaver.h"
int ScreenSaverSec = 0;
#endif //ENABLE_SCREENSAVER
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
#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
int LogPeriodSecS = LOG_PERIOD_MS / 1000;
int LogPeriodSecF = LOG_PERIOD_MS / 1000;
int MaxWindowSec = MAX_SAMPLE_NB * ((LogPeriodSecF>LogPeriodSecS)?LogPeriodSecF:LogPeriodSecS);
char StoreInit[32];
char StoreFinal[32];
int MaxLogUsageMb = MAX_SPACE_AVAILABLE_DEF;
#endif //defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)

#ifdef ENABLE_ALARMS
QList<event_descr_e *> _active_alarms_events_;
bool HornACK = false;
bool ForceResetAlarmBanner = false;
#endif //ENABLE_ALARMS

/* Buzzer */
int Buzzerfd = -1;
bool BuzzerTouch = true;
bool BuzzerAlarm = true;
int ID_FORCE_BUZZER =-1;

/* Trend */
#ifdef ENABLE_TREND
char _layout_ = LANDSCAPE;
char _last_layout_ = LANDSCAPE;
bool _trend_data_reload_ = true;
#endif

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

#ifdef ENABLE_TRANSLATION
char _language_[3] = DEFAULT_LANGUAGE;
QTranslator* translator;
#endif //ENABLE_TRANSLATION

#ifdef ENABLE_STORE
char _actual_store_[FILENAME_MAX] = "";
#endif //ENABLE_STORE

#ifdef ENABLE_TREND
QStringList _current_trend_variables_;
char _actual_trend_[FILENAME_MAX] = "";
pen_t pens[PEN_NB + 1];
int actualPen = 0;
bool _online_ = true;
/* first time sample of visible data actually */
QDateTime actualTzero;
/* windows size in seconds of visible data actually */
int actualVisibleWindowSec = 0;
#endif // ENABLE_TREND

#ifdef ENABLE_RECIPE
char _actual_recipe_[FILENAME_MAX] = "";
#endif // ENABLE_RECIPE

QHash<char *, int *> NameMap;

QStringList userPageList;

QString HMIversion;
