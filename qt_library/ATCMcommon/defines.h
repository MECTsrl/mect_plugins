/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Defines
 */
#ifndef _DEFINES_VAR_H_
#define _DEFINES_VAR_H_

/* HMI setting values */
#define STR_LEN 128
#define MAX_LINE 256
#define PAGE_NAME_LEN 32
#define MAX_LEN_UPDATE_MSG 5
#define WAIT_COMM_RETRY_NB 20
#define SPLASH_TIMEOUT_MS 2000
#define REFRESH_MS 500
#define LOG_PERIOD_MS 5000

/* Important files */
#define BACKGROUND_FILE ""
#define MOUNT_POINT "/tmp/mnt"
#define SPLASH_FILE LOCAL_ETC_DIR"/img/splash.png"
#define BACKING_FILE "/local/root/backing_file"
#define LANGUAGE_MAP_FILE "/local/etc/sysconfig/lang_table.csv"

/* Configuration file Tags */
//unsigned 16 bit
//unsigned 32 bit (baudrate)
//stringhe 17
#define HOMEPAGE_TAG            "SYSTEM/home_page"
#define HOMEPAGE_DEF            "menu"
#define STARTPAGE_TAG           "SYSTEM/start_page"
#define STARTPAGE_DEF           HOMEPAGE_DEF
#define LOG_PERIOD_SLOW_TAG     "SYSTEM/slow_log_period_s"
#define LOG_PERIOD_SLOW_DEF     LOG_PERIOD_MS/1000
#define LOG_PERIOD_FAST_TAG     "SYSTEM/fast_log_period_s"
#define LOG_PERIOD_FAST_DEF     LOG_PERIOD_MS/1000
#define WINDOW_SEC_TAG          "SYSTEM/trace_window_s"
#define SCREENSAVER_TAG         "SYSTEM/screen_saver_s"
#define BUZZER_ALARM_TAG        "SYSTEM/buzzer_alarm"
#define BUZZER_ALARM_DEF        1
#define BUZZER_TOUCH_TAG        "SYSTEM/buzzer_touch"
#define BUZZER_TOUCH_DEF        1
#define PWD_TIMEOUT_SEC_TAG     "SYSTEM/pwd_timeout_s"
#define PWD_TIMEOUT_SEC_DEF     0
#define PWD_LOGOUT_PAGE_TAG     "SYSTEM/pwd_logout_page"
#define PWD_LOGOUT_PAGE_DEF     ""
#define MAX_SPACE_AVAILABLE_TAG "SYSTEM/max_log_space_MB"
#define MAX_SPACE_AVAILABLE_DEF 5
#define MAX_SPACE_AVAILABLE_MAX 128
#define RETRIES_TAG             "SYSTEM/retries"
#define RETRIES_DEF             3
#define BLACKLIST_TAG           "SYSTEM/blacklist"
#define BLACKLIST_DEF           10
#define HIGH_PRIORITY_TAG       "SYSTEM/high_priority_ms"
#define HIGH_PRIORITY_DEF       10
#define MIDDLE_PRIORITY_TAG     "SYSTEM/middle_priority_ms"
#define MIDDLE_PRIORITY_DEF     100
#define LOW_PRIORITY_TAG        "SYSTEM/low_priority_ms"
#define LOW_PRIORITY_DEF        1000
#define LANGUAGE_TAG            "SYSTEM/language"
#define DEFAULT_LANGUAGE        "it"

#define USB_MODE                "SYSTEM/usb_mode"
#define DEVICE_TAG              "Device"
#define HOST_TAG                "Host"

#endif

