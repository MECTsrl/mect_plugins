#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include <errno.h>
#include <sys/time.h>
#include "global_functions.h"
#include "app_logprint.h"
#include "screensaver.h"
#include "hmi_logger.h"
#include "cross_table_utility.h"
#include "app_usb.h"
#include "defines.h"
#include "app_usb.h"
#include "utility.h"
#include <QDirIterator>
#include <QFontDatabase>
//#include "app_config.h"

/**
 * @brief load the passwords
 */
int loadPasswords()
{
    FILE *out;
    int i;
    out = fopen(PASSFILE, "rb");
    if (out == NULL)
    {
        LOG_PRINT(verbose_e,"Cannot open password file\n");
        return 1;
    }
    for (i = 0; i < PASSWORD_NB; i++)
    {
        if (fread(&passwords[i], 4,1, out)==0)
        {
            LOG_PRINT(verbose_e,"Cannot read password %d\n", i);
            fclose(out);
            return 1;
        }
        LOG_PRINT(verbose_e,"password[%d] = %d\n", i, passwords[i]);
    }
    fclose(out);
    return 0;
}

/**
 * @brief save the passwords
 */
int dumpPasswords()
{
    FILE *out;
    int i;
    out = fopen(PASSFILE, "wb");
    if (out != NULL)
    {
        for (i = 0; i < PASSWORD_NB; i++)
        {
            fwrite(&passwords[i], 4,1, out);
            LOG_PRINT(verbose_e,"dump password[%d] = %d\n", i, passwords[i]);
        }
        fclose(out);
        return 0;
    }
    else
    {
        LOG_PRINT(verbose_e,"Cannot create password file\n");
        return 1;
    }
}

/**
 * @brief readCfgFile: read the config file and fill the service area into the ioLayer
 */
int readIniFile(void)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    LOG_PRINT(verbose_e, "Reading configuration...\n");

    strcpy(HomePage, settings.value(HOMEPAGE_TAG, HOMEPAGE_DEF).toString().toAscii().data());
    LOG_PRINT(verbose_e, "'%s' = %s\n", HOMEPAGE_TAG, HomePage);

    strcpy(StartPage, settings.value(STARTPAGE_TAG, STARTPAGE_DEF).toString().toAscii().data());
    LOG_PRINT(verbose_e, "'%s' = %s\n", STARTPAGE_TAG, StartPage);

    BuzzerAlarm = settings.value(BUZZER_ALARM_TAG, BUZZER_ALARM_DEF).toBool();
    LOG_PRINT(verbose_e, "'%s' = %d\n", BUZZER_ALARM_TAG, BuzzerAlarm);

    BuzzerTouch = settings.value(BUZZER_TOUCH_TAG, BUZZER_TOUCH_DEF).toBool();
    LOG_PRINT(verbose_e, "'%s' = %d\n", BUZZER_TOUCH_TAG, BuzzerTouch);

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
    LogPeriodSecS = settings.value(LOG_PERIOD_SLOW_TAG, LOG_PERIOD_SLOW_DEF).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", LOG_PERIOD_SLOW_TAG, LogPeriodSecS);

    LogPeriodSecF = settings.value(LOG_PERIOD_FAST_TAG, LOG_PERIOD_FAST_DEF).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", LOG_PERIOD_FAST_TAG, LogPeriodSecF);

    MaxWindowSec = settings.value(WINDOW_SEC_TAG, MAX_SAMPLE_NB * ((LogPeriodSecF>LogPeriodSecS)?LogPeriodSecF:LogPeriodSecS)).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", WINDOW_SEC_TAG, MaxWindowSec);

    MaxLogUsageMb = settings.value(MAX_SPACE_AVAILABLE_TAG, MAX_SPACE_AVAILABLE_DEF).toInt();
    MaxLogUsageMb = (MaxLogUsageMb < 0) ? 0 : MaxLogUsageMb;
    if (MaxLogUsageMb == 0)
    {
        LOG_PRINT(warning_e, "No space available for logs [%d]. No logs will be recorded.\n", MaxLogUsageMb);
    }
    LOG_PRINT(verbose_e, "'%s' = %d\n", MAX_SPACE_AVAILABLE_TAG, MaxLogUsageMb);
#endif

#ifdef ENABLE_SCREENSAVER
    ScreenSaverSec = settings.value(SCREENSAVER_TAG, SCREENSAVER_DEFAULT_TIME/1000).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", SCREENSAVER_TAG, ScreenSaverSec);
#endif

    PwdTimeoutSec = settings.value(PWD_TIMEOUT_SEC_TAG, PWD_TIMEOUT_SEC_DEF).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", PWD_TIMEOUT_SEC_TAG, PwdTimeoutSec);

    strcpy(PwdLogoutPage, settings.value(PWD_LOGOUT_PAGE_TAG, PWD_LOGOUT_PAGE_DEF).toString().toAscii().data());
    LOG_PRINT(verbose_e, "'%s' = %s\n", PWD_LOGOUT_PAGE_TAG, PwdLogoutPage);

#ifdef TRANSLATION
    strcpy(_language_, settings.value(LANGUAGE_TAG, DEFAULT_LANGUAGE).toString().toAscii().data());
    LOG_PRINT(verbose_e, "'%s' = %s\n", LANGUAGE_TAG, _language_);
#endif

    return 0;
}

/**
 * @brief writeIniFile: write into the config file the data into the ioLayer service area
 */
int writeIniFile(void)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    LOG_PRINT(verbose_e, "Dumping configuration...\n");

    settings.setValue(HOMEPAGE_TAG, HomePage);
    settings.setValue(STARTPAGE_TAG, StartPage);
    settings.setValue(BUZZER_ALARM_TAG, BuzzerAlarm);
    settings.setValue(BUZZER_TOUCH_TAG, BuzzerTouch);

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
    settings.setValue(LOG_PERIOD_SLOW_TAG, LogPeriodSecS);
    settings.setValue(LOG_PERIOD_FAST_TAG, LogPeriodSecF);
    settings.setValue(WINDOW_SEC_TAG, MaxWindowSec);
    settings.setValue(MAX_SPACE_AVAILABLE_TAG, MaxLogUsageMb);
#endif

#ifdef ENABLE_SCREENSAVER
    settings.setValue(SCREENSAVER_TAG, ScreenSaverSec);
#endif

    settings.setValue(PWD_TIMEOUT_SEC_TAG, PwdTimeoutSec);
    settings.setValue(PWD_LOGOUT_PAGE_TAG, PwdLogoutPage);

#ifdef TRANSLATION
    settings.setValue(LANGUAGE_TAG, _language_);
#endif
    settings.sync();

    return 0;
}

int loadRecipe(char *filename, QList<u_int16_t> *indexes, QList<u_int32_t> table[])
{
    FILE * fp;

    // clear lists
    indexes->clear();
    for (int n = 0; n < MAX_RCP_STEP; ++n) {
        table[n].clear();
    }

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(verbose_e, "Cannot open '%s'\n", filename);
        return -1;
    }

    char line[LINE_SIZE] = "";
    char *p, *r;
    int step_max = 0, step;
    for (int line_nb = 0; fgets(line, LINE_SIZE, fp) != NULL; line_nb++)
    {
        /* ignore blank lines */
        if (line[0] == '\n' || line[0] == '\r' || line[0] == 0) {
            LOG_PRINT(verbose_e, "skipping empty line\n");
            continue;
        }
        /* tag */
        p = strtok_csv(line, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s' at line %d\n", line, line_nb);
            continue;
        }
        int ctIndex;
        LOG_PRINT(verbose_e, "Loading variable '%s'\n", p);
        if (Tag2CtIndex(p, &ctIndex))
        {
            LOG_PRINT(error_e, "Invalid variable '%s' at line %d\n", p, line_nb);
            continue;
        }
        indexes->append((u_int16_t)ctIndex);
        int decimal = getVarDecimalByCtIndex(ctIndex);

        /* values */
        u_int32_t value;
        float val_float;
        u_int8_t val_bit;
        int32_t val_int32;
        int16_t val_int16;

        step = 0;
        while ((p = strtok_csv(NULL, SEPARATOR, &r)) != NULL)
        {
            value = 0;
            // compute value
            switch (varNameArray[ctIndex].type)
            {
            case uintab_e:
            case uintba_e:
            case intab_e:
            case intba_e:
            {
                val_float = atof(p);
                for (int n = 0; n < decimal; ++n) {
                    val_float = val_float * 10;
                }
                val_int16 = (int16_t)val_float;
                value = (u_int32_t)val_int16;
                break;
            }
            case udint_abcd_e:
            case udint_badc_e:
            case udint_cdab_e:
            case udint_dcba_e:
            case dint_abcd_e:
            case dint_badc_e:
            case dint_cdab_e:
            case dint_dcba_e:
            {
                val_float = atof(p);
                for (int n = 0; n < decimal; ++n) {
                    val_float = val_float * 10;
                }
                val_int32 = (int32_t)val_float;
                memcpy(&value, &val_int32, sizeof(u_int32_t));
                break;
            }
            case fabcd_e:
            case fbadc_e:
            case fcdab_e:
            case fdcba_e:
            {
                val_float = atof(p);
                memcpy(&value, &val_float, sizeof(u_int32_t));
                break;
            }
            case bytebit_e:
            case wordbit_e:
            case dwordbit_e:
            case bit_e:
            {
                val_bit = atoi(p);
                value = (u_int32_t)val_bit;
                break;
            }
            default:
                /* unknown type */
                value = 0;
            }
            // assign value
            (table[step]) << value;
            step++;
        }
        if (step > step_max) {
            step_max = step;
        }
    }

    LOG_PRINT(verbose_e, "row %d column %d\n", table[step].count(), step_max);
    fclose(fp);
    return step_max;
}

int readRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[])
{
    int errors = 0;

    if (step >= MAX_RCP_STEP)
    {
        return -1;
    }
    for (int i = 0; i < table[step].count(); i++)
    {
        char msg[TAG_LEN] = "";

        uint32_t valueu = 0;
        int ctIndex = (int)(indexes->at(i));
        readFromDb(ctIndex, &valueu);
        LOG_PRINT(verbose_e, "%d -> %d\n", ctIndex, valueu);

        switch (getStatusVarByCtIndex(ctIndex, msg))
        {
        case BUSY:
            //retry_nb = 0;
            LOG_PRINT(verbose_e, "BUSY: %s\n", msg);
            if (msg[0] == '\0')
            {
                strcpy(msg, VAR_PROGRESS);
            }
            errors++;
            break;
        case ERROR:
            LOG_PRINT(verbose_e, "ERROR: %s\n", msg);
            if (msg[0] == '\0')
            {
                strcpy(msg, VAR_COMMUNICATION);
            }
            errors++;
            break;
        case DONE:
            LOG_PRINT(verbose_e, "DONE step %d i %d value %d\n", step, i, valueu);
            table[step][i] = valueu;
            msg[0] = '\0';
            break;
        default:
            LOG_PRINT(verbose_e, "OTHER: %s\n", msg);
            if (msg[0] == '\0')
            {
                strcpy(msg, VAR_UNKNOWN);
            }
            errors++;
            break;
        }
        if (msg[0] != '\0')
        {
            LOG_PRINT(error_e, "Reading (%d) - '%s' - '%s'\n", i, varNameArray[ctIndex].tag, msg);
        }
    }
    return errors;
}

int writeRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[])
{
    int errors = 0;

    if (step >= MAX_RCP_STEP)
    {
        return -1;
    }
    beginWrite();
    for (int i = 0; i < table[step].count(); i++)
    {
        u_int16_t addr = indexes->at(i);
        u_int32_t value = table[step].at(i);
        errors += addWrite(addr, &value);
    }
    endWrite();

    return errors;
}

bool CommStart()
{
    /* Load the cross-table in order to allocate the ioArea to the right size (should be fixed size) and fill the syncro table */
    int elem_read = fillSyncroArea();
    if (elem_read < 0)
    {
        LOG_PRINT(error_e, "cannot find the cross table [%s]\n", CROSS_TABLE);
        QMessageBox::critical(0,QApplication::trUtf8("Cross Table Check"), QApplication::trUtf8("Cannot find the cross table %1\nMSG: '%2'").arg(CROSS_TABLE).arg(CrossTableErrorMsg));
        return false;
    }
    else if (elem_read < DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "cannot load completly the cross table [%dvs%d]\n", elem_read, DB_SIZE_ELEM);
        QMessageBox::critical(0,QApplication::trUtf8("Cross Table Check"), QApplication::trUtf8("Syntax error into the cross table at line %1\nMSG: '%2'").arg(elem_read).arg(CrossTableErrorMsg));
        return false;
    }
    else if (elem_read > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "Too many variable into the cross table [%dvs%d]\n", elem_read, DB_SIZE_ELEM);
        QMessageBox::critical(0,QApplication::trUtf8("Cross Table Check"), QApplication::trUtf8("Syntax error into the cross table at line %1\nMSG: '%2'").arg(elem_read).arg(CrossTableErrorMsg));
        return false;
    }

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
    logger = new Logger(NULL, NULL, LogPeriodSecS*1000);
#endif

    /* start the io layers */
    ioComm = new io_layer_comm(&data_send_mutex, &data_recv_mutex);
    LOG_PRINT(verbose_e, "Starting IOLayer Data\n");

    /* setting output data area size */
    SET_SIZE_BYTE(IODataAreaO + DB_OUT_BASE_BYTE, DB_SIZE_ELEM);
    /* setting status area size */
    SET_SIZE_BYTE(IODataAreaO + STATUS_BASE_BYTE, DB_SIZE_ELEM);

    /* setting input data area size (shouldn't be necessary) -- it has been checked at least for array init*/
    //SET_SIZE_BYTE(IODataAreaI + DB_IN_BASE_BYTE, DB_SIZE_ELEM);
    //SET_SIZE_BYTE(IODataAreaI + STATUS_BASE_BYTE, DB_SIZE_ELEM);

    if (ioComm->initializeData(LOCAL_SERVER_ADDR, LOCAL_SERVER_DATA_RX_PORT, LOCAL_SERVER_DATA_TX_PORT, IODataAreaI, STATUS_BASE_BYTE + DB_SIZE_BYTE, IODataAreaO, STATUS_BASE_BYTE + DB_SIZE_BYTE) == false)
    {
        LOG_PRINT(error_e, "cannot connect to the Data IOLayer\n");
        QMessageBox::critical(0,QApplication::trUtf8("Connection"), QApplication::trUtf8("Cannot connect to the Data IOLayer"));
        return false;
    }

    /* setting output syncro area size, for the corresponding reading area it's not necessary */
    SET_SIZE_WORD(IOSyncroAreaO + SYNCRO_BASE_BYTE, DB_SIZE_ELEM);

    if (ioComm->initializeSyncro(LOCAL_SERVER_ADDR, LOCAL_SERVER_SYNCRO_RX_PORT, LOCAL_SERVER_SYNCRO_TX_PORT, IOSyncroAreaI, SYNCRO_SIZE_BYTE, IOSyncroAreaO, SYNCRO_SIZE_BYTE) == false)
    {
        LOG_PRINT(error_e, "cannot connect to the Syncro IOLayer\n");
        QMessageBox::critical(0,QApplication::trUtf8("Connection"), QApplication::trUtf8("Cannot connect to the Data IOLayer"));
        return false;
    }
    ioComm->start();
    LOG_PRINT(verbose_e, "IOLayer Syncro Started\n");

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
    /* start the logger thread */
    logger->start();
    LOG_PRINT(verbose_e, "Logger Started\n");
#endif

    if (QFile::exists(KINDOFUPDATE_FILE))
    {
        FILE * fp = fopen(KINDOFUPDATE_FILE, "r");
        char msg[STR_LEN];
        QString fullmsg;
        if (fp != NULL)
        {
            int i = 0;
            while ((fgets(msg, STR_LEN, fp))!= NULL)
            {
                if (i < MAX_LEN_UPDATE_MSG)
                {
                    fullmsg.append(msg);
                }
                else
                {
                    fullmsg.append("...");
                    break;
                }
                fullmsg.append("\n");
                i++;
            }
            fclose(fp);
            QMessageBox::information(0, "Update", fullmsg);
        }
        QFile::remove(KINDOFUPDATE_FILE);
    }

    /* load the passwords */
    loadPasswords();
#ifdef LOG_DISABLED
    int i;
    for (i = 0; i <= PASSWORD_NB; i++)
    {
        LOG_PRINT(verbose_e, "Password %d '%s' = '%d'\n", i, PasswordsString[i], passwords[i]);
    }
#endif
    return true;
}

int initialize()
{
    /* load the saved configuration value */
    readIniFile();

#ifdef TARGET
    /* set the cursor as invisible */
    QWSServer::setCursorVisible(false);

    /* load the background to show at QWS */
    QPixmap background;

    background.load(BACKGROUND_FILE);
    QWSServer::setBackground(background);
#endif

    /* prepare the directory tree */
#ifdef ENABLE_ALARMS
    mkdir(ALARMS_DIR, S_IRWXU | S_IRWXG);
#endif
#if defined(ENABLE_TREND) || defined(ENABLE_STORE)
    mkdir(STORE_DIR, S_IRWXU | S_IRWXG);
    mkdir(CUSTOM_STORE_DIR, S_IRWXU | S_IRWXG);
    mkdir(CUSTOM_TREND_DIR, S_IRWXU | S_IRWXG);
#endif
#if defined(ENABLE_TREND)
    mkdir(SCREENSHOT_DIR, S_IRWXU | S_IRWXG);
#endif
#ifdef RECIPE_DIR
    mkdir(RECIPE_DIR, S_IRWXU | S_IRWXG);
#endif
    /* initialize the usb API */
#ifdef ENABLE_USB
    app_usb_init();
#endif

#if defined(TARGET) && defined(ENABLE_SCREENSAVER)
    /* Set up screen saver */
    QWSServer::setScreenSaver( new ScreenSaver );
    QWSServer::setScreenSaverInterval(ScreenSaverSec * 1000); //msec
    QWSServer::setScreenSaverBlockLevel(0);
#endif
#ifdef ENABLE_TRANSLATION
    /* if a language is not set, set italian as default */
    if (strlen(_language_) == 0)
    {
        strcpy(_language_, DEFAULT_LANGUAGE);
    }

    /* load the language selected */
    if (translator == NULL)
    {
        translator = new QTranslator();
    }

    if(translator->load(QString(":/translations/languages_%1.qm").arg(_language_)))
    {
        /* install the selected language */
        qApp->installTranslator(translator);
    }
    else
    {
        LOG_PRINT(error_e, "loading language file\n");
    }
#endif

    /*load font into resource file */
    QDirIterator it(":", QDirIterator::Subdirectories);
    QFontDatabase fontDB;
    while (it.hasNext()) {
        QString item = it.next();
        if (item.endsWith(".ttf", Qt::CaseInsensitive))
        {
            if (fontDB.addApplicationFont(item) == -1)
            {
                LOG_PRINT(error_e, "Cannot load font '%s' from resource file.\n", item.toAscii().data());
            }
        }
    }

    return 0;
}

int finalize()
{
#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
    /* finalyze and free the logger */
    logger->exit();
    delete logger;
#endif

    /* finalyze and free the io Layers */
    ioComm->finalize();
    ioComm->exit();
    delete ioComm;
    return 0;
}

bool setUSBmode(enum usb_mode_e mode)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    if (mode == usb_device_e)
    {
        settings.setValue(USB_MODE, DEVICE_TAG);
        settings.sync();
    }
    else if (mode == usb_host_e)
    {
        settings.setValue(USB_MODE, HOST_TAG);
        settings.sync();
    }
    else
    {
        return false;
    }
    return true;
}

enum usb_mode_e USBmode()
{
    static enum usb_mode_e mode = usb_undefined_e;

    if (mode == usb_undefined_e)
    {
        if (system ("lsmod | grep -qc g_file_storage >/dev/null 2>&1") == 0)
        {
            return usb_device_e;
        }
        else
        {
            return usb_host_e;
        }
    }
    else
    {
        return mode;
    }
}

bool USBCheck()
{
    if (USBmode() == usb_device_e)
    {
        return true;
    }
    else
    {
        /*USBstatus*/
        int i;
        int usb = 1;
        usb = app_usb_status_read();
        if ( usb == 0 )
        {
            for (i = 0; i < APP_USB_MAX+1; i++ )
            {
                USBstatus[i]= (int)app_usb_status[i];
            }
        }
        else if	( usb == -1 )
        {
            USBstatus[APP_USB_MAX]= 30;
        }

        USBfeedback[0]= (int)app_usb_feedback[0];
        USBfeedback[1]= (int)app_usb_feedback[1];

        return  USBstatus[0] != 0;
    }
}

#define RETRY_NB 5
bool USBmount()
{
    if (usb_mnt_point[0] != '\0')
    {
        LOG_PRINT(verbose_e, "Usb already mounted to '%s'\n", usb_mnt_point);
        return true;
    }

    if (USBmode() == usb_device_e)
    {
        char command[256];
        sprintf(command,"LOOP=`losetup -f` && losetup -o 4096 $LOOP %s && mount -t vfat $LOOP %s >/dev/null 2>&1", BACKING_FILE, MOUNT_POINT);
        if (system (command) == 0)
        {
            strcpy(usb_mnt_point, MOUNT_POINT);
            return true;
        }
        else
        {
            usb_mnt_point[0] = '\0';
            return false;
        }
    }
    else
    {
        static int last_usb_status = 0;

        last_usb_status = USBstatus[0];

        if (USBCheck() == false)
        {
            LOG_PRINT(error_e, "Usb Check fail\n");
            return false;
        }

        /* check if a usb key is inserted */
        if (USBstatus[0] != 0 && strlen(usb_mnt_point) == 0)
        {
            int retry_nb = 0;
            /* wait that is not busy */
            do {
                usleep(5000);
                LOG_PRINT (verbose_e, "usb waiting free \n");
                retry_nb++;
            }
            while(USBfeedback[0] != 0 /*|| strlen(Usb_mpoint(1)) != 0*/ && retry_nb < RETRY_NB);
            LOG_PRINT (verbose_e, "found usb USBfeedback[1] %d\n", USBfeedback[1]);
            /* mount the key */
            if (USBfeedback[1] != 0 || Usb_on(1) != 0)
            {
                LOG_PRINT (error_e, "cannot mount the USB! USBfeedback[1] = %d\n", USBfeedback[1]);
                usb_mnt_point[0] = '\0';
                return false;
            }
            strcpy(usb_mnt_point, Usb_mpoint(1));
            LOG_PRINT (verbose_e, "Found USB. Mountpint: '%s'\n", usb_mnt_point);
            return true;
        }
        else if (strlen(usb_mnt_point) != 0)
        {
            LOG_PRINT (verbose_e, "Already mounted to '%s'\n", usb_mnt_point);
            return true;
        }
        else
        {
            LOG_PRINT (verbose_e, "Cannot find USB.\n");
        }

        if (last_usb_status != USBstatus[0])
        {
            LOG_PRINT(verbose_e, "extracted USB\n");
            usb_mnt_point[0] = '\0';
        }

        usb_mnt_point[0] = '\0';
        return false;
    }
}

bool USBumount()
{
    if (usb_mnt_point[0] == '\0')
    {
        LOG_PRINT (verbose_e, "USB already unmounted cause usb_mnt_point = '%s'\n", usb_mnt_point);
        return false;
    }
    if (USBmode() == usb_device_e)
    {
        char command[256];
        sprintf(command,"LOOP=`losetup -f | tr \"\\/dev\\/loop\" \" \" | awk '{printf(\"%%d\\n\", $1 - 1);}'` && umount %s && losetup -d /dev/loop$LOOP", MOUNT_POINT);
        if (system (command) == 0)
        {
            LOG_PRINT(error_e, "cannot unmount: %s\n", strerror(errno));
        }
    }
    else
    {
        Usb_off(1);
    }
    LOG_PRINT(verbose_e, "unmount\n");
    usb_mnt_point[0] = '\0';
    return true;
}

bool beep(int duration_ms)
{
    static time_t buzzer_busy_timeout_ms = 0;
    static time_t now_ms = 0;
    struct timeval now;

    /* return if the buzzer file descriptor is invalid */
    if (Buzzerfd == -1)
    {
        return false;
    }

    /* gives  the number of milliseconds since the Epoch */
    gettimeofday(&now, NULL);
    now_ms = now.tv_sec * 1000 + now.tv_usec / 1000;

    /* initialize the buzzer timeout */
    if (buzzer_busy_timeout_ms == 0)
    {
        buzzer_busy_timeout_ms = now_ms;
    }

    /* if the timeouit is expired, beep the buzzer for duration_ms */
    if (buzzer_busy_timeout_ms <= now_ms)
    {
        LOG_PRINT(verbose_e, "duration %d\n", duration_ms);
        buzzer_busy_timeout_ms = now_ms + duration_ms;
        if (ioctl(Buzzerfd, BUZZER_BEEP, duration_ms) != 0)
        {
            LOG_PRINT(error_e, "problem playng buzzer.\n");
            return false;
        }
    }
    else
    {
        LOG_PRINT(verbose_e, "busy %ld < %ld\n", buzzer_busy_timeout_ms, now_ms);
        return false;
    }
    return true;
}

int set_backlight_level(int level)
{
    int _level;
    char command[256];

    if (level < 0)
    {
        _level = 0;
    }
    else if ( level > 100)
    {
        _level = 100;
    }
    else
    {
        _level = level;
    }

    sprintf (command, "echo %d > %s", level, BACKLIGHT_FILE_SYSTEM);
    system(command);

    //printf("level set to: %d", level);
    return level;
}

int get_backlight_level(void)
{
    int level;
    FILE * fp = fopen(BACKLIGHT_FILE_SYSTEM, "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", &level);
    }
    fclose(fp);
    return level;
}

bool LoadTrend(const char * filename, QString * ErrorMsg)
{
    FILE * fp;
    char line[LINE_SIZE] = "";
    char * p = NULL, *r = NULL;

    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", filename);
        if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Cannot open '%1'").arg(filename);
        return false;
    }
    LOG_PRINT(verbose_e, "opened '%s'\n", filename);
    /*
     * the file is formatted as
     * <Layout>
     * <Visible>;<Tag1>;<color>;<Ymin>;<Ymax>;<description>
     * <Visible>;<Tag2>;<color>;<Ymin>;<Ymax>;<description>
     * <Visible>;<Tag3>;<color>;<Ymin>;<Ymax>;<description>
     * <Visible>;<Tag4>;<color>;<Ymin>;<Ymax>;<description>
     */
    /* extract the layout */
    if (fgets(line, LINE_SIZE, fp) == NULL)
    {
        LOG_PRINT(error_e, "Invalid Layout '%s'\n", line);
        fclose(fp);
        if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Invalid Layout '%1'").arg(line);
        return false;
    }

    //disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));

    switch(line[0])
    {
    case PORTRAIT:
        _layout_ = PORTRAIT;
        break;
    case LANDSCAPE:
        _layout_ = LANDSCAPE;
        break;
    default:
        _layout_ = LANDSCAPE;
        break;
    }
    _last_layout_ = _layout_;

    int rownb = 0;
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        if (rownb >= PEN_NB)
        {
            LOG_PRINT(verbose_e, "Too many pen, ignore them.\n");
            break;
        }
        LOG_PRINT(verbose_e, "line %s\n", line);
        pens[rownb].visible = false;
        if (pens[rownb].curve != NULL)
        {
            //delete pens[rownb].curve;
            pens[rownb].curve = NULL;
        }
        if (pens[rownb].y != NULL)
        {
            delete pens[rownb].y;
            pens[rownb].y = NULL;
        }

        pens[rownb].color[0] = '\0';
        pens[rownb].description[0] = '\0';
        pens[rownb].sample = 0;
        pens[rownb].tag[0] = '\0';
        pens[rownb].visible = 0;
        pens[rownb].y = NULL;
        pens[rownb].yMax = 0;
        pens[rownb].yMin = 0;
        pens[rownb].yMaxActual = 0;
        pens[rownb].yMinActual = 0;

        /* visible */
        p = strtok_csv(line, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid visibility field '%s'\n", line);
            fclose(fp);
            if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Invalid visible tag");
            return false;
        }
        else if (p[0] == '\0')
        {
            pens[rownb].visible = false;
        }
        else
        {
            pens[rownb].visible = (atoi(p) == 1);
            LOG_PRINT(verbose_e, "tag '%s'\n", p);
        }

        /* tag */
        int index;
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Invalid tag field '%s'\n", line);
            fclose(fp);
            if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Invalid variable tag '%1'").arg(line);
            return false;
        }
        else if (p[0] == '\0')
        {
            LOG_PRINT(warning_e, "Empty tag '%s' at line %d\n", p, rownb+1);
            pens[rownb].tag[0] = '\0';
            pens[rownb].visible = false;
        }
        else if (Tag2CtIndex(p, &index) != 0)
        {
            LOG_PRINT(error_e, "The variable '%s' is not present into the Crosstable\n", p);
            pens[rownb].tag[0] = '\0';
            pens[rownb].visible = false;
            pens[rownb].CtIndex = -1;
            if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Cannot find the variable %1 into the Crosstable. The pen will be disabled.").arg(p);
            return false;
        }
        else
        {
            pens[rownb].CtIndex = index;
            strcpy(pens[rownb].tag, p);
            LOG_PRINT(verbose_e, "tag '%s'\n", p);
        }

        /* color */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid color field '%s'\n", line);
            fclose(fp);
            if (ErrorMsg)*ErrorMsg = QObject::trUtf8("Invalid color tag '%1'").arg(line);
            return false;
        }
        else if (p[0] == '\0')
        {
            strcpy(pens[rownb].color, "000000");
            LOG_PRINT(warning_e, "Empty color field. set default as '%s'\n", p);
        }
        else
        {
            strcpy(pens[rownb].color, p);
            LOG_PRINT(verbose_e, "tag '%s'\n", p);
        }

        /* Ymin */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            fclose(fp);
            if (ErrorMsg)* ErrorMsg = QObject::trUtf8("Invalid Ymin field '%1'").arg(line);
            return false;
        }
        else if (p[0] == '\0')
        {
            pens[rownb].yMin = 0;
            pens[rownb].yMinActual = 0;
        }
        else
        {
            pens[rownb].yMin = atof(p);
            pens[rownb].yMinActual = pens[rownb].yMin;
            LOG_PRINT(verbose_e, "tag '%s'\n", p);
        }

        /* Ymax */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            fclose(fp);
            if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Invalid Ymax field '%1'").arg(line);
            return false;
        }
        else if (p[0] == '\0')
        {
            pens[rownb].yMax = 0;
            pens[rownb].yMaxActual = 0;
        }
        else
        {
            pens[rownb].yMax = atof(p);
            pens[rownb].yMaxActual = pens[rownb].yMax;
            LOG_PRINT(verbose_e, "tag '%s'\n", p);
        }

        if (pens[rownb].visible && pens[rownb].yMin >= pens[rownb].yMax)
        {
            LOG_PRINT(warning_e, "Max value must be bigger than min value\n");
            if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Max value must be bigger than min value");
            return false;
        }

        /* description */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0')
        {
            LOG_PRINT(verbose_e, "Empty description '%s'\n", line);
            pens[rownb].description[0] = '\0';
        }
        else
        {
            strcpy(pens[rownb].description, p);
        }

        rownb++;
    }
    fclose(fp);

    if (rownb != PEN_NB)
    {
        LOG_PRINT(error_e, "Incomplete trend file (%d vs %d)\n", rownb, PEN_NB);
        if (ErrorMsg) *ErrorMsg = QObject::trUtf8("Incomplete trend file (%1 vs %2)").arg(rownb).arg(PEN_NB);
        return false;
    }

    /* fill the empty color */
    {
        QStringList default_color;
        int i = 0, j = 0;

        default_color << "FF0000" << "00FF00" << "0000FF" << "000000";

        for (int z = 0; z < PEN_NB; z++)
        {
            /* the actual color is empty*/
            if (pens[z].color[0] == '\0')
            {
                for (i = 0; i < default_color.count(); i++)
                {
                    for (j = 0; j < PEN_NB; j++)
                    {
                        /* the color i is already used by the pen j */
                        if (strcmp(default_color.at(i).toAscii().data(), pens[j].color) == 0)
                        {
                            break;
                        }
                    }
                    /* the color i is not used by the pen j so is a valid color */
                    if (strcmp(default_color.at(i).toAscii().data(), pens[j].color) != 0)
                    {
                        break;
                    }
                }
                /* if all aolor are used, use the default color */
                if (i > PEN_NB)
                {
                    i = default_color.count() - 1;
                }
                strcpy(pens[z].color, default_color.at(i).toAscii().data());
                LOG_PRINT(verbose_e, "set color of pen %d as '%s'\n", z, pens[z].color);
            }
            else
            {
                LOG_PRINT(verbose_e, "the pen %d already have the color '%s'\n", z, pens[z].color);
            }
        }
    }

    actualPen = 0;
    for (int i = 0; i < PEN_NB; i++)
    {
        LOG_PRINT(verbose_e, "pen '%s' position %d color %s min %f max %f\n", pens[i].tag, i, pens[i].color, pens[i].yMin, pens[i].yMax);
        if (pens[i].visible)
        {
            actualPen = i;
            LOG_PRINT(verbose_e, "visible pen '%s' position %d\n", pens[actualPen].tag, actualPen);
            //connect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
            return true;
        }
    }
    LOG_PRINT(warning_e, "No visible pen\n");
    if (ErrorMsg) *ErrorMsg = QObject::trUtf8("No visible pen");
    return false;
}
