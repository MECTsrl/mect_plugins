#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H

#include "global_var.h"
#include <unistd.h>

enum usb_mode_e
{
    usb_host_e,
    usb_device_e,
    usb_undefined_e
};

/**
 * @brief load the passwords
 *
 * @return 0 success, error otherwise
 */
int loadPasswords();

/**
 * @brief dump the passwords
 *
 * @return 0 success, error otherwise
 */
int dumpPasswords();

/**
 * @brief write/update the config file with the actual value
 *
 * @return 0 success, error otherwise
 */
int writeIniFile(void);

/**
 * @brief read all data from config file
 *
 * @return 0 success, error otherwise
 */
int readIniFile(void);

/**
 * @brief do the initalization step for the communication before start HMI
 *
 * @return 0 success, error otherwise
 */
int initialize();

/**
 * @brief do the finalization step for the communication at HMI end
 *
 * @return 0 success, error otherwise
 */
int finalize();

/**
 * @brief start the socket communication
 *
 * @return true success, false error
 */
bool CommStart();

#ifdef BUZZER

/**
 * @brief beep the buzzer for a while in milliseconds
 *
 * @return true success, false error
 */
bool beep(int duration_ms);

class MyEventFilter: public QObject
{
public:
    MyEventFilter():QObject()
    {
        Buzzerfd = open(BUZZER, O_RDWR);

        if (Buzzerfd < 0) {
            fprintf(stderr,"can't open buzzer device '%s'\n", BUZZER);
        }
    };
    ~MyEventFilter()
    {
        if (Buzzerfd != -1)
        {
            close(Buzzerfd);
        }
    };

    bool eventFilter(QObject* object,QEvent* event)
    {
        Q_UNUSED(object);

        if (event->type() == QEvent::MouseButtonPress)
        {
            struct timespec oldTouch = LastTouch;
            clock_gettime(CLOCK_REALTIME, &LastTouch);
            unsigned delay_ms = (LastTouch.tv_sec - oldTouch.tv_sec) * 1000;
            delay_ms += (LastTouch.tv_nsec - oldTouch.tv_nsec) / 1000000;
            if (delay_ms > BUZZER_DURATION_MS)
            {
                if (BuzzerTouch == true)
                {
                    beep(BUZZER_DURATION_MS);
                }
            }
        }
        return false;
    }
};
#endif

/**
 * @brief mount USB pen drive
 *
 * @return true success, error false
 */
bool USBmount();

/**
 * @brief unmount USB pen drive
 *
 * @return true success, error false
 */
bool USBumount();

/**
 * @brief set the USB port usage (host or device)
 *
 * @return true success, error false
 */
bool setUSBmode(enum usb_mode_e mode);

/**
 * @brief get the USB port usage (host or device)
 *
 * @return usb_mode_e
 */
enum usb_mode_e USBmode();

/**
 * @brief check if a USB pen drive is inserted
 *
 * @return true inserted, false not inserted
 */
bool USBCheck();

/**
 * @brief load a bi-dimensional recipe
 *
 * @return true loaded, false not loaded
 */
int loadRecipe(char *filename, QList<u_int16_t> *indexes, QList<u_int32_t> table[]);

/**
 * @brief write a step of bi-dimensional recipe
 *
 * @return 0 written, otherwise not written
 */
int writeRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[]);

/**
 * @brief read a step of bi-dimensional recipe
 *
 * @return 0 read, otherwise not read
 */
int readRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[]);

/**
 * @brief set the backlight level
 *
 * @return 0 set, otherwise not set
 */
int set_backlight_level(int level);

/**
 * @brief read the backlight level
 *
 * @return 0 read, otherwise not read
 */
int get_backlight_level(void);

#endif // GLOBAL_FUNCTIONS_H
