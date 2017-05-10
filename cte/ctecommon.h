#ifndef CTECOMMON_H
#define CTECOMMON_H
#include <QString>

struct  TP_Config {
    // General Params
    QString modelName;
    int     nModel;
    // Structural Parameters
    int     displayWidth;
    int     displayHeight;
    int     usbPorts;
    int     ethPorts;
    int     sdCards;
    int     nEncoders;
    int     digitalIN;
    int     digitalOUT;
    int     analogIN;
    int     analogINrowCT;
    int     analogOUT;
    int     analogOUTrowCT;
    bool    tAmbient;
    int     rpmPorts;
    // Config Parameters
    int     retries;
    int     blacklist;
    int     readPeriod1;
    int     readPeriod2;
    int     readPeriod3;
    int     fastLogPeriod;
    int     slowLogPeriod;
    // Bus Interfaces
    // Serial 0
    bool    ser0_Enabled;
    int     ser0_BaudRate;
    int     ser0_TimeOut;
    int     ser0_Silence;
    int     ser0_BlockSize;
    // Serial 1
    bool    ser1_Enabled;
    int     ser1_BaudRate;
    int     ser1_TimeOut;
    int     ser1_Silence;
    int     ser1_BlockSize;
    // Serial 2
    bool    ser2_Enabled;
    int     ser2_BaudRate;
    int     ser2_TimeOut;
    int     ser2_Silence;
    int     ser2_BlockSize;
    // Serial 3
    bool    ser3_Enabled;
    int     ser3_BaudRate;
    int     ser3_TimeOut;
    int     ser3_Silence;
    int     ser3_BlockSize;
    // TCP
    int     tcp_TimeOut;
    int     tcp_Silence;
    int     tcp_BlockSize;
    // Can0
    bool    can0_Enabled;
    int     can0_BaudRate;
    int     can0_BlockSize;
    // Can1
    bool    can1_Enabled;
    int     can1_BaudRate;
    int     can1_BlockSize;
};

#endif // CTECOMMON_H
