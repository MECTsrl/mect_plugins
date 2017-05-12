#ifndef CTECOMMON_H
#define CTECOMMON_H
#include <QString>

// Int Constants
const int nMax_Int16 = 65535;
const int nMaxNodeID = 63;
const int nMaxRegister = 49999;
// Serial Ports
const int nMaxSerialPorts = 3;
// CanOpen Ports
const int nMaxCanPorts = 1;
// TCP Ports
const int nPortFTPControl = 21;
const int nPortSFTP = 22;
const int nPortTELNET = 23;
const int nPortHTTP = 80;
const int nPortVNC = 5900;


// QChar Constants
const QChar   chDOUBLEQUOTE = QChar::fromAscii(34);
const QChar   chSpace = QChar::fromAscii(20);
const QChar   chBACKSLASH = QChar::fromAscii(92);
const QChar   chX = QChar::fromLatin1(88);

// String Costants
const QString szEMPTY = QString::fromAscii("");
const QString szDOUBLEQUOTE = QString(1, chDOUBLEQUOTE);
const QString szZERO = QString::fromAscii("0");
const QString szSEMICOL = QString::fromAscii(";");
const QString szSLASH = QString::fromAscii("/");
const QString szBACKSLASH = QString::fromAscii("\\");
const QString szNEWLINE = QString::fromAscii("\n");
const QString szSHARP = QString::fromAscii("#");
const QString szTRUE = QString::fromAscii("1");
const QString szFALSE = QString::fromAscii("0");
const QString szMectTitle = QString::fromAscii("Mect Editor");



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
