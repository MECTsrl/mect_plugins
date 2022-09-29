#ifndef GLOBAL_FUNCTIONS_H
#define GLOBAL_FUNCTIONS_H

#include "global_var.h"
#include <unistd.h>

#include <QNetworkInterface>
#include <QNetworkAddressEntry>
#include <QHostAddress>

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

        if (Buzzerfd < 0)
            fprintf(stderr,"can't open buzzer device '%s'\n", BUZZER);
        LastTouch.start();
    }

    ~MyEventFilter()
    {
        if (Buzzerfd != -1)
            close(Buzzerfd);
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        Q_UNUSED(object);

        if (event->type() == QEvent::MouseButtonPress) {// no TouchBegin
            bool doBuzzer = false;

            if (LastTouch.elapsed() > (2 * IOLAYER_PERIOD_ms)) { // REFRESH_MS) {
                doBuzzer = true;
            }
            LastTouch.restart();

            if (BuzzerTouch && doBuzzer) {
                // fprintf(stderr, "beep() <-- MouseButtonPress, object=%p %s\n", object, object->objectName().toAscii().data());
                beep(BUZZER_DURATION_MS); // ID_PLC_TOUCH_VOLUME
            }
        }
        // always return false, otherwise we steel the event
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

int checkRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[]);

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

u_int32_t getMectSuiteVersion();

bool check_wifi_board();                // returns true if wifi board wlan0 is present

bool check_vpn_board();                 // returns true if vpn board is present

bool check_usb_wan_board();             // returns true if wan board ppp0 is present

bool get_wifi_signal_level(int &nQuality, int &nSignalLevel);   // Returns true if wlan0 is present and Signal Quality (0..100) and Signal Level (dbm)

bool getBoardGateway(const char * board_name, unsigned &boardGW);   // returns the ip address of default gateway for board <board_name>

bool checkNetAddr(char * ipaddr);       // Check format of an IP Address

QString getMacAddr(QString interface);  // Returns MAC Address for board <interface> as a QString

QString getIPAddr(QString interface);   // Returns IP Address for board <interface> as a QString

bool    getBoardIPInfo(QString interface, QNetworkAddressEntry &netAddr);   // Returns first IP address info for board <interface> as a QNetworkAddressEntry

bool isWlanOn(void);                    // returns true if wifi board wlan0 is up and connected to an AP

bool isWanOn(void);                     // returns true if WLan ppp0 board is up and connected

bool isVpnOn(void);                     // returns true if tun_mrs (VPN) is up

bool waitShellCommand(QString szCommand, int msecs = 30000);   // Shell a command and wait complection waiting almost msecs, msecs = -1 --> wait forever

bool LoadTrend(const char * trend_name, QString * ErrorMsg);

QObject *getPage(QObject *plugin);

#endif // GLOBAL_FUNCTIONS_H
