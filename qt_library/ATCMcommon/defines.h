/** 
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief HMI Main function
 */
#ifndef _DEFINES_VAR_H_
#define _DEFINES_VAR_H_

#define REFRESH_MS 500
#define DEFAULT_LANGUAGE "it"
#define SPLASH_TIMEOUT_MS 2000
#define BACKGROUND_FILE ""
#define SPLASH_FILE LOCAL_ETC_DIR"/img/splash.png"
#define MOUNT_POINT "/tmp/mnt"
#define BACKING_FILE "/local/root/backing_file"

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

#define MAX_LEN_UPDATE_MSG 5
#define STR_LEN 128
#define WAIT_COMM_RETRY_NB 20

#endif
