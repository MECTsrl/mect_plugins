/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
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
    char version[LINE_SIZE] = "";

    /* SN:
     * BLD: "Release:"
     * MOD: "Target:"
     * LIB: "MectPlugin:"
     * RT:  "RunTime:"
     * PLC:
     * HMI: "MectApps:"
     */

    /* SN */
    if (app_build_serial_get() != NULL)
    {
        ui->labelSNval->setText(app_build_serial_get());
    }
    else
    {
        ui->labelSNval->setText("-");
    }
    /* BLD */
    if (getVersion("grep Release: /rootfs_version | cut -d: -f2", version, LINE_SIZE))
    {
        ui->labelSOval->setText(version);
    }
    else
    {
        ui->labelSOval->setText("-");
    }
    /* MOD */
    if (getVersion("grep Target: /rootfs_version | cut -d: -f2", version, LINE_SIZE))
    {
        ui->labelTargetval->setText(version);
    }
    else
    {
        ui->labelTargetval->setText("-");
    }

    /* LIB */
    if (getVersion("grep MectPlugin: /rootfs_version | cut -d: -f2", version, LINE_SIZE))
    {
        ui->labelMectLib->setText(version);
    }
    else
    {
        ui->labelMectLib->setText("-");
    }

    /* RT */
    uint16_t PLC_Version;
    readFromDb(5394, &PLC_Version);
    ui->labelFcrtsval->setText(QString("%1").arg((float)PLC_Version/1000.0, 0, 'f', 3));

    /* PLC */
    ui->labelPLCval->setText("-");

    /* HMI */
    ui->labelHMIval->setText(HMIversion);

    char string[32];
    /* MAC */
    if (getMAC("eth0", string) == 0)
    {
        ui->labelMACval->setText(string);
    }
    else
    {
        ui->labelMACval->setText("-");
    }
    /* IP */
    if (getIP("eth0", string) == 0)
    {
        ui->labelIPval->setText(string);
    }
    else
    {
        ui->labelIPval->setText("-");
    }
    /* GATEWAY */
    if (app_build_gateway_get() != NULL)
    {
        ui->labelGatewayval->setText(app_build_gateway_get());
    }
    else
    {
        ui->labelGatewayval->setText("-");
    }
    /* NETMASK */
    if (app_build_netmask_get() != NULL)
    {
        ui->labelNetMaskval->setText(app_build_netmask_get());
    }
    else
    {
        ui->labelNetMaskval->setText("-");
    }
    /* DNS1 */
    if (app_build_dns1_get() != NULL)
    {
        ui->labelDNS1val->setText(app_build_dns1_get());
    }
    else
    {
        ui->labelDNS1val->setText("-");
    }
    /* DNS2 */
    if (app_build_dns2_get() != NULL)
    {
        ui->labelDNS2val->setText(app_build_dns2_get());
    }
    else
    {
        ui->labelDNS2val->setText("-");
    }

}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void info::updateData()
{
	/* call the parent updateData member */
	page::updateData();

    static int divisor = 0;

    if (REFRESH_MS * divisor < REMOVABLE_REFRESH_MS)
    {
        divisor ++;
        return;
    }

    divisor = 0;

    char serial[SN_LEN] = "-";
    if (getSdSN(serial) == 0)
    {
        ui->labelSDval->setText(serial);
    }
    else
    {
        ui->labelSDval->setText("-");
    }

    switch (checkSDusage())
    {
    case 0:
    ui->labelLICval->setText("Unused");
        break;
    case 1:
    ui->labelLICval->setText("Application");
        break;
    case 2:
    ui->labelLICval->setText("Storage");
        break;
    default:
    ui->labelLICval->setText("-");
        break;
    }

    if (USBCheck())
    {
        ui->labelUSBval->setText("INSERTED");
    }
    else
    {
        ui->labelUSBval->setText("-");
    }
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void info::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}
#endif

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

    // Serial #
    szMessage.append(ui->labelSNtxt->text());
    szMessage.append(ui->labelSNval->text());
    szMessage.append("\n");
    // Build
    szMessage.append(ui->labelSOtxt->text());
    szMessage.append(ui->labelSOval->text());
    szMessage.append("\n");
    // Target
    szMessage.append(ui->labelTargettxt->text());
    szMessage.append(ui->labelTargetval->text());
    szMessage.append("\n");
    // Build Version
    szMessage.append(ui->labelMectLibtxt->text());
    szMessage.append(ui->labelMectLib->text());
    szMessage.append("\n");
    // Real Time
    szMessage.append(ui->labelFcrtstxt->text());
    szMessage.append(ui->labelFcrtsval->text());
    szMessage.append("\n");
    // PLC
    szMessage.append(ui->labelPLCtxt->text());
    szMessage.append(ui->labelPLCval->text());
    szMessage.append("\n");
    // HMI
    szMessage.append(ui->labelHMItxt->text());
    szMessage.append(ui->labelHMIval->text());
    szMessage.append("\n");
    // MAC
    szMessage.append(ui->labelMACtxt->text());
    szMessage.append(ui->labelMACval->text());
    szMessage.append("\n");
    // IP
    szMessage.append(ui->labelIPtxt->text());
    szMessage.append(ui->labelIPval->text());
    szMessage.append("\n");
    // NET MASK
    szMessage.append(ui->labelNetMasktxt->text());
    szMessage.append(ui->labelNetMaskval->text());
    szMessage.append("\n");
    // Gateway
    szMessage.append(ui->labelGatewaytxt->text());
    szMessage.append(ui->labelGatewayval->text());
    szMessage.append("\n");
    // DNS1
    szMessage.append(ui->labelDNS1txt->text());
    szMessage.append(ui->labelDNS1val->text());
    szMessage.append("\n");
    // DNS2
    szMessage.append(ui->labelDNS2txt->text());
    szMessage.append(ui->labelDNS2val->text());
    szMessage.append("\n");
    // SD Card
    szMessage.append(ui->labelSDtxt->text());
    szMessage.append(ui->labelSDval->text());
    szMessage.append("\n");
    // USB Port
    szMessage.append(ui->labelUSBtxt->text());
    szMessage.append(ui->labelUSBval->text());
    szMessage.append("\n");
    // License
    szMessage.append(ui->labelLICtxt->text());
    szMessage.append(ui->labelLICval->text());
    szMessage.append("\n");

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
