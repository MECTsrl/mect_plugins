/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Info page
 */
#include <QString>
#include "app_logprint.h"
#include "info.h"
#include "global_functions.h"
#include "ui_info.h"
#include "app_config.h"
#include "qrcode.h"
#include "ntpclient.h"
#include "linux/route.h"

#include <QList>
#include <QString>
#include <QPlainTextEdit>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>


#if 0
#include "fw_build_version.h"
#else
#define FW_RELEASE ( \
        (QDate::fromString(__DATE__, "MMM dd yyyy").toString("yyyyMMdd").length() != 0) \
        ? \
        QDate::fromString(__DATE__, "MMM dd yyyy").toString("yyyyMMdd").toAscii().data() \
        : \
        QDate::fromString(__DATE__, "MMM  d yyyy").toString("yyyyMMdd").toAscii().data() \
        )

#endif

#define REMOVABLE_REFRESH_MS 1000

/* this define set the window title */
#define WINDOW_TITLE "INFO"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

#define SET_INFO_STYLE() { \
	QString mystyle; \
	mystyle.append(this->styleSheet()); \
	mystyle.append("QLabel"); \
	mystyle.append("{"); \
    mystyle.append("    font: 11pt \""FONT_TYPE"\";"); \
	mystyle.append("}"); \
	mystyle.append("QWidget"); \
	mystyle.append("{"); \
	mystyle.append("    background-image: url(\"\");"); \
	mystyle.append("}"); \
	this->setStyleSheet(mystyle); \
}

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
info::info(QWidget *parent) :
	page(parent),
	ui(new Ui::info)
{
	ui->setupUi(this);
	/* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
	 * protection_level = pwd_operator_e;
	 */

	/* if exist and is not empty enable the WINDOW_ICON */
	if (strlen(WINDOW_ICON) > 0)
	{
		/* enable this line, define the WINDOW_ICON and put a Qlabel named labelIcon in your ui file if you want have a window icon */
		labelIcon = ui->labelIcon;
	}
	/* if exist and is not empty enable the WINDOW_TITLE */
	if (strlen(WINDOW_TITLE) > 0)
	{
		/* enable this line, define the WINDOW_TITLE and put a Qlabel named labelTitle in your ui file if you want have a window title */
		labelTitle = ui->labelTitle;
	}

	/* set up the page style */
	////setStyle::set(this);
	/* set the style described into the macro SET_INFO_STYLE */
	//SET_INFO_STYLE();

	/* connect the label that show the date and the time to the timer of the parent updateData */
	labelDataOra = ui->labelDataOra;
	/* connect the label that show the actual user name */
	//labelUserName = ui->labelUserName;
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void info::reload()
{
    refreshSystemTab();
    refreshApplTab();
    refreshNetworkingTabs();
    refreshNTPInfo();
}

void info::refreshSystemTab()
{
    FILE *fp = NULL;
    char buff[1024];
    unsigned uvalue = 0;

    ui->sys_text->setPlainText("");

    readFromDbQuick(ID_PLC_PRODUCT_ID, (int *)&uvalue);
    ui->sys_text->appendPlainText(QString("PLC_PRODUCT_ID: 0x%1").arg(uvalue, 8, 16));
    readFromDbQuick(ID_PLC_SERIAL_NUMBER, (int *)&uvalue);
    ui->sys_text->appendPlainText(QString("PLC_SERIAL_NUMBER: %1").arg(uvalue));

    ui->sys_text->appendPlainText(QString(""));
    readFromDbQuick(ID_PLC_UPTIME_s, (int *)&uvalue);
    ui->sys_text->appendPlainText(QString("PLC_PLC_UPTIME_s:  %1").arg(uvalue));
    readFromDbQuick(ID_PLC_UPTIME_cs, (int *)&uvalue);
    ui->sys_text->appendPlainText(QString("PLC_PLC_UPTIME_cs: %1").arg(uvalue));

    ui->sys_text->appendPlainText(QString(""));
    fp = fopen(APP_CONFIG_SERIAL_FILE, "r");
    if (fp) {
    /*
        $ cat /etc/serial.conf
        2016031007
    */
        while (fgets(buff, 1023, fp)) {
            ui->sys_text->appendPlainText(QString("S/N: %1").arg(buff).trimmed());
        }
        fclose(fp);
        fp = NULL;
    } else {
        ui->sys_text->appendPlainText("missing serial number");
    }

    ui->sys_text->appendPlainText(QString(""));
    fp = fopen("/rootfs_version", "r");
    if (fp) {
    /*
        $ cat /rootfs_version
        Release: 3.0.6
        Target:  TPAC1007_03
        Qt:      4.8.5
        Qwt:     6.1-multiaxes
        RunTime: mect_suite_3.0/0.0
        MectPlugin: mect_suite_3.0/0.0
        MectApps: mect_suite_3.0/0.0
    */
        while (fgets(buff, 1023, fp)) {
            ui->sys_text->appendPlainText(QString(buff).trimmed());
        }
        fclose(fp);
        fp = NULL;
    } else {
        ui->sys_text->appendPlainText("missing /rootfs_version");
    }
    // newline per QRcode
    ui->sys_text->appendPlainText("");
    ui->sys_text->moveCursor(QTextCursor::Start);

}

void info::refreshApplTab()
{
    char SDcardSN[SN_LEN];
    const char *usage[] = {"-", "Unused", "Application", "Storage"};
    int i = 0;

    ui->appl_text->setPlainText("");

    readFromDbQuick(ID_PLC_Version, &i);
    ui->appl_text->appendPlainText(QString("PLC RunTime: %1").arg((float)i/1000.0, 0, 'f', 3));

    readFromDbQuick(ID_PLC_PLC_Version, &i);
    ui->appl_text->appendPlainText(QString("PLC Application: %1").arg((float)i/1000.0, 0, 'f', 3));

    readFromDbQuick(ID_PLC_HMI_Version, &i); // ex HMIversion
    ui->appl_text->appendPlainText(QString("HMI Application: %1").arg((float)i/1000.0, 0, 'f', 3));

    ui->appl_text->appendPlainText("");
    if (getSdSN(SDcardSN)) {
        strcpy(SDcardSN, "-");
    }
    i = checkSDusage() + 1;
    if (i < 0 || i > 4)
        i = 0;
    ui->appl_text->appendPlainText(QString("SDcard: %1 (%2)").arg(SDcardSN).arg(usage[i]));

    ui->appl_text->appendPlainText("");
    if (USBCheck())
    {
        ui->appl_text->appendPlainText("USB: INSERTED");
    }
    else
    {
        ui->appl_text->appendPlainText("USB: -");
    }
    // newline per QRcode
    ui->appl_text->appendPlainText("");
    ui->appl_text->moveCursor(QTextCursor::Start);
}

void info::refreshNetworkingTabs()
{
    FILE *fp = NULL;
    char buff[1024];

    // [dns]
    ui->dns_text->setPlainText("");
    fp = fopen("/etc/resolv.conf", "r");
    if (fp) {
        char buff[1024];
        unsigned b3, b2, b1, b0;
        const char *fmt = "nameserver %u.%u.%u.%u\n";
    /*
        $ cat /etc/resolv.conf
        nameserver 8.8.8.8
        nameserver 8.8.4.4
    */
        int dns = 0;
        while (fgets(buff, 1023, fp)) {
            int num = sscanf(buff, fmt,
            &b3, &b2, &b1, &b0);

            if (num < 4) {
                continue;
            }
            ++dns;
            quint32 nameserver = b0 + (b1 << 8) + (b2 << 16) + (b3 << 24);
            ui->dns_text->appendPlainText(
                QString("DNS[%1] %2")
                    .arg(dns)
                    .arg(QHostAddress(nameserver).toString()));
        }
        fclose(fp);
        fp = NULL;
    }
    // newline per QRcode
    ui->dns_text->appendPlainText("");

    // [eth0],[wlan0],[ppp0],[tun_mrs]: MACaddress and IPaddress(es)
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    QNetworkInterface iface;

    ui->eth0_text->setPlainText("");
    ui->wlan0_text->setPlainText("");
    ui->ppp0_text->setPlainText("");
    ui->tun0_text->setPlainText("");
    foreach(iface, allInterfaces) {
        QList<QNetworkAddressEntry> allEntries = iface.addressEntries();
        QPlainTextEdit *plainText = NULL;

        if (iface.name() == QString("eth0"))  {
            plainText = ui->eth0_text;
        } else if (iface.name() == QString("wlan0"))  {
            plainText = ui->wlan0_text;
        } else if (iface.name() == QString("ppp0"))  {
            plainText = ui->ppp0_text;
        } else if (iface.name() == QString("tun_mrs"))  {
            plainText = ui->tun0_text;
        }
        if (plainText) {
            // MAC 70:B3:D5:62:52:11
            // IP[1] 192.168.5.211/24
            // IP[2] 192.168.0.211/24
            // No MAC for tun0
            if (plainText != ui->tun0_text)  {
                plainText->appendPlainText("MAC " + iface.hardwareAddress());
                plainText->appendPlainText("");
            }
            for (int n = 0; n < allEntries.count(); ++n) {
                plainText->appendPlainText(
                    QString("IP[%1] %2/%3")
                    .arg(n + 1)
                    .arg(QString(allEntries[n].ip().toString()))
                    .arg(QString("%1").arg(allEntries[n].prefixLength()))
                );
            }
        }
    }

    // [eth0],[wlan0],[ppp0],[tun_mrs]: routing
    ui->eth0_text->appendPlainText("");
    ui->wlan0_text->appendPlainText("");
    ui->ppp0_text->appendPlainText("");
    ui->tun0_text->appendPlainText("");
    fp = fopen("/proc/net/route", "r");
    if (fp) {
        char Iface[16];
        unsigned Destination, Gateway;
        int Flags, RefCnt, Use, Metric;
        unsigned Mask;
        int MTU, Window, IRTT;
        const char *fmt = "%16s %X %X %X %d %d %d %X %d %d %d\n";
    /*
        $ cat /proc/net/route
        Iface   Destination     Gateway         Flags   RefCnt  Use     Metric  Mask            MTU     Window  IRTT
        tun_mrs 1500010A        00000000        0005    0       0       0       FFFFFFFF        0       0       0
        eth0    0005A8C0        00000000        0001    0       0       0       00FFFFFF        0       0       0
        tun_mrs 0000010A        1500010A        0003    0       0       0       0000FFFF        0       0       0
        lo      0000007F        00000000        0001    0       0       0       000000FF        0       0       0
        eth0    00000000        0A05A8C0        0003    0       0       0       00000000        0       0       0
        $ route -n
        Kernel IP routing table
        Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
        10.1.0.21       0.0.0.0         255.255.255.255 UH    0      0        0 tun_mrs
        192.168.5.0     0.0.0.0         255.255.255.0   U     0      0        0 eth0
        10.1.0.0        10.1.0.21       255.255.0.0     UG    0      0        0 tun_mrs
        127.0.0.0       0.0.0.0         255.0.0.0       U     0      0        0 lo
        0.0.0.0         192.168.5.10    0.0.0.0         UG    0      0        0 eth0
    */
        while (fgets(buff, 1023, fp)) {
            int num = sscanf(buff, fmt,
            Iface, &Destination, &Gateway, &Flags, &RefCnt, &Use, &Metric, &Mask, &MTU, &Window, &IRTT);

            if (num < 11) {
                continue;
            }
            QPlainTextEdit *plainText = NULL;

            if (strcmp(Iface, "eth0") == 0)  {
                plainText = ui->eth0_text;
            } else if (strcmp(Iface, "wlan0") == 0)  {
                plainText = ui->wlan0_text;
            } else if (strcmp(Iface, "ppp0") == 0)  {
                plainText = ui->ppp0_text;
            } else if (strcmp(Iface, "tun_mrs") == 0)  {
                plainText = ui->tun0_text;
            }
            if (plainText ) {
                if (Flags & RTF_UP) { // no RTF_GATEWAY
                    if (Destination == 0 && Mask == 0) {
                        plainText->appendPlainText(
                            QString("default route via %1")
                            .arg(QHostAddress(ntohl(Gateway)).toString())
                        );
                    } else {
                        QNetworkAddressEntry x = QNetworkAddressEntry();
                        x.setIp(QHostAddress(ntohl(Destination)));
                        x.setNetmask(QHostAddress(ntohl(Mask)));
                        plainText->appendPlainText(
                            QString("route %1/%2 via %3")
                            .arg(QHostAddress(ntohl(Destination)).toString())
                            .arg(x.prefixLength())
                            .arg(QHostAddress(ntohl(Gateway)).toString())
                        );
                    }
                }
            }

        }
        fclose(fp);
        fp = NULL;
    }
    // Signal Level in wlan0
    if (check_wifi_board())  {
        int nQuality = 0;
        int nSignalLevel = 0;
        ui->wlan0_text->appendPlainText("");
        if (get_wifi_signal_level(nQuality, nSignalLevel))  {
            ui->wlan0_text->appendPlainText(
                QString("Signal Quality: %1% - Signal Level: %2 dBm")
                                            .arg(nQuality) .arg(nSignalLevel));
        }
        else  {
            ui->wlan0_text->appendPlainText("Signal Quality: --- - Signal Level: --- dBm");
        }
    }
    // newline per QRcode
    ui->eth0_text->appendPlainText("");
    ui->wlan0_text->appendPlainText("");
    ui->ppp0_text->appendPlainText("");
    ui->tun0_text->appendPlainText("");
    // Rewind the Text Cursor
    ui->eth0_text->moveCursor(QTextCursor::Start);
    ui->wlan0_text->moveCursor(QTextCursor::Start);
    ui->ppp0_text->moveCursor(QTextCursor::Start);
    ui->tun0_text->moveCursor(QTextCursor::Start);
}

void info::refreshNTPInfo()
{
    // Clear NPT Text
    ui->ntp_text->setPlainText("");
    // Refresh NTP Perams
    ui->ntp_text->appendPlainText(QString("NTP Server: \t%1") .arg(ntpclient->getNtpServer()));
    ui->ntp_text->appendPlainText(QString("NTP Offset: \t%1 [Hours]") .arg(ntpclient->getOffset_h(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP TimeOut:\t%1 [Seconds]") .arg(ntpclient->getTimeout_s(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP Period: \t%1 [Hours, 0=Sync disabled]") .arg(ntpclient->getPeriod_h(), 4, 10));
    ui->ntp_text->appendPlainText(QString("NTP DST:    \t%1") .arg(ntpclient->getDst() ? QString("ON") : QString("OFF")));
    ui->ntp_text->moveCursor(QTextCursor::Start);
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void info::updateData()
{
	/* call the parent updateData member */
	page::updateData();

//    static int divisor = 0;

//    if (REFRESH_MS * divisor < REMOVABLE_REFRESH_MS)
//    {
//        divisor ++;
//        return;
//    }

//    divisor = 0;

}

void info::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
info::~info()
{
	delete ui;
}


void info::on_pushButtonHome_clicked()
{
	go_home();
}

bool info::getVersion(const char * command, char * version, int maxsize)
{
    FILE *fp;

    /* Open the command for reading. */
    fp = popen(command, "r");
    if (fp == NULL)
    {
        LOG_PRINT(verbose_e, "Failed to run command '%s'\n", command );
        return false;
    }

    /* Read the output a line at a time - output it. */
    if (fgets(version, maxsize, fp) != NULL)
    {
        sscanf(version, "%s", version);
        LOG_PRINT(verbose_e, "version: '%s'\n", version);
    }
    else
    {
        pclose(fp);
        LOG_PRINT(verbose_e, "Failed to run command '%s': '%s'\n", command, version );
        return false;
    }

    /* close */
    pclose(fp);
    return true;
}

void info::on_pushButtonBack_clicked()
{
    go_back();
}

void info::on_pushButtonQrc_clicked()
{
    QString szMessage;
    QString szFile("/tmp/Info.png");
    qrcode  * myCode;
    int     nRes = 0;
    char command[1024];

    szMessage.append(ui->tabWidget->tabText(0) + "\n");
    szMessage.append(ui->sys_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(1) + "\n");
    szMessage.append(ui->appl_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(2) + "\n");
    szMessage.append(ui->dns_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(3) + "\n");
    szMessage.append(ui->eth0_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(4) + "\n");
    szMessage.append(ui->wlan0_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(5) + "\n");
    szMessage.append(ui->ppp0_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(6) + "\n");
    szMessage.append(ui->tun0_text->toPlainText());
    szMessage.append(ui->tabWidget->tabText(7) + "\n");
    szMessage.append(ui->ntp_text->toPlainText());
    // Compose Command
    sprintf(command,"qrencode -t PNG -o %s \"%s\" > /dev/null 2>&1", szFile.toAscii().data(), szMessage.toAscii().data());
    // Create PNG
    if (system (command) == 0)  {
        myCode = new qrcode(szFile, this);
        myCode->setModal(true);
        nRes = myCode->exec();
        delete myCode;
    }
}

void info::on_pushButtonRefresh_clicked()
{
    refreshSystemTab();
    refreshApplTab();
    refreshNetworkingTabs();
    refreshNTPInfo();
}
