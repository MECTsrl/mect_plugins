#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include <errno.h>
#include "global_functions.h"
#include "app_logprint.h"
#include "screensaver.h"
#include "hmi_logger.h"
#include "cross_table_utility.h"
#include "app_usb.h"
#include "defines.h"
#include "app_usb.h"
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
        LOG_PRINT(info_e,"Cannot open password file\n");
        return 1;
    }
    for (i = 0; i < PASSWORD_NB; i++)
    {
        if (fread(&passwords[i], 4,1, out)==0)
        {
            LOG_PRINT(info_e,"Cannot read password %d\n", i);
            return 1;
        }
        LOG_PRINT(info_e,"password[%d] = %d\n", i, passwords[i]);
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
            LOG_PRINT(info_e,"dump password[%d] = %d\n", i, passwords[i]);
        }
        fclose(out);
        return 0;
    }
    else
    {
        LOG_PRINT(info_e,"Cannot create password file\n");
        return 1;
    }
}

/**
 * @brief readCfgFile: read the config file and fill the service area into the ioLayer
 */
int readIniFile(void)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    LOG_PRINT(info_e, "Reading configuration...\n");

    strcpy(HomePage, settings.value(HOMEPAGE_TAG, HOMEPAGE_DEF).toString().toAscii().data());
    LOG_PRINT(info_e, "'%s' = %s\n", HOMEPAGE_TAG, HomePage);

    strcpy(StartPage, settings.value(STARTPAGE_TAG, STARTPAGE_DEF).toString().toAscii().data());
    LOG_PRINT(info_e, "'%s' = %s\n", STARTPAGE_TAG, StartPage);

    BuzzerAlarm = settings.value(BUZZER_ALARM_TAG, BUZZER_ALARM_DEF).toBool();
    LOG_PRINT(info_e, "'%s' = %d\n", BUZZER_ALARM_TAG, BuzzerAlarm);

    BuzzerTouch = settings.value(BUZZER_TOUCH_TAG, BUZZER_TOUCH_DEF).toBool();
    LOG_PRINT(info_e, "'%s' = %d\n", BUZZER_TOUCH_TAG, BuzzerTouch);

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
    LogPeriodSecS = settings.value(LOG_PERIOD_SLOW_TAG, LOG_PERIOD_SLOW_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", LOG_PERIOD_SLOW_TAG, LogPeriodSecS);

    LogPeriodSecF = settings.value(LOG_PERIOD_FAST_TAG, LOG_PERIOD_FAST_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", LOG_PERIOD_FAST_TAG, LogPeriodSecF);

    MaxWindowSec = settings.value(WINDOW_SEC_TAG, MAX_SAMPLE_NB * ((LogPeriodSecF>LogPeriodSecS)?LogPeriodSecF:LogPeriodSecS)).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", WINDOW_SEC_TAG, MaxWindowSec);

    MaxLogUsageMb = settings.value(MAX_SPACE_AVAILABLE_TAG, MAX_SPACE_AVAILABLE_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", MAX_SPACE_AVAILABLE_TAG, MaxLogUsageMb);
#endif

#ifdef ENABLE_SCREENSAVER
    ScreenSaverSec = settings.value(SCREENSAVER_TAG, SCREENSAVER_DEFAULT_TIME/1000).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", SCREENSAVER_TAG, ScreenSaverSec);
#endif

    PwdTimeoutSec = settings.value(PWD_TIMEOUT_SEC_TAG, PWD_TIMEOUT_SEC_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", PWD_TIMEOUT_SEC_TAG, PwdTimeoutSec);

    strcpy(PwdLogoutPage, settings.value(PWD_LOGOUT_PAGE_TAG, PWD_LOGOUT_PAGE_DEF).toString().toAscii().data());
    LOG_PRINT(info_e, "'%s' = %s\n", PWD_LOGOUT_PAGE_TAG, PwdLogoutPage);

    FailNb = settings.value(RETRIES_TAG, RETRIES_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", RETRIES_TAG, FailNb);

    FailDivisor = settings.value(BLACKLIST_TAG, BLACKLIST_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", BLACKLIST_TAG, FailDivisor);

    FailDivisor = settings.value(HIGH_PRIORITY_TAG, HIGH_PRIORITY_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", HIGH_PRIORITY_TAG, HighPriorityTimerMsec);

    FailDivisor = settings.value(MIDDLE_PRIORITY_TAG, MIDDLE_PRIORITY_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", MIDDLE_PRIORITY_TAG, MediumPriorityTimerMsec);

    FailDivisor = settings.value(LOW_PRIORITY_TAG, LOW_PRIORITY_DEF).toInt();
    LOG_PRINT(info_e, "'%s' = %d\n", LOW_PRIORITY_TAG, LowPriorityTimerMsec);

    return 0;
}

/**
 * @brief writeIniFile: write into the config file the data into the ioLayer service area
 */
int writeIniFile(void)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    LOG_PRINT(info_e, "Dumping configuration...\n");

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

    return 0;
}

int LoadRecipe(const QString filename)
{
    FILE * fp = fopen(filename.toAscii().data(), "r");
    if (fp == NULL)
    {
        LOG_PRINT(info_e, "Cannot open '%s'\n", filename.toAscii().data());
        return -1;
    }
    char varname[TAG_LEN] = "";
    char value[TAG_LEN] = "";
    char line[MAX_LINE] = "";
    char * p;
    int number_of_variables = 0;
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        p = line;
        /* tag */
        p = mystrtok(p, varname, SEPARATOR);
        if (p == NULL || varname[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            continue;
        }

        int ctIndex;
        if (Tag2CtIndex(varname, &ctIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot extract ctIndex for variable '%s'\n", varname);
            return -1;

        }
        int decimal = getVarDecimal(ctIndex);
        /* value */
        p = mystrtok(p, value, SEPARATOR);
        if (value[0] != '\0')
        {
            float val_f = 0;
            val_f = atof(value);
            sprintf(value, "%.*f", decimal, val_f );
            if (prepareFormattedVarByCtIndex (ctIndex, value) == BUSY)
            {
                LOG_PRINT(warning_e, "busy, waiting to write the variable '%s' with the value '%s'\n", varname, value);
            }
            LOG_PRINT(info_e, "value '%s'\n", value);
            if (number_of_variables == 0)
            {
                beginWrite();
            }
            number_of_variables++;
        }
    }
    fclose(fp);
    if (number_of_variables > 0)
    {
        endWrite();
    }
    return number_of_variables;
}

bool loadTagTable()
{
    FILE * fp;
    char line[LINE_SIZE] = "";
    char token[LINE_SIZE] = "";
    char reference[LINE_SIZE] = "";
    char * p;

    fp = fopen(TAG_TABLE, "r");
    if (fp == NULL)
    {
        LOG_PRINT(info_e, "Cannot open '%s'\n", TAG_TABLE);
        return false;
    }
    LOG_PRINT(info_e, "opened '%s'\n", TAG_TABLE);
    /*
     * the file is formatted as
     *   Page's Type;ID Page;Tag Reference;Chars num;Description
     */
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        p = line;
        /* Page's Type */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            continue;
        }

        /* ID Page */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            continue;
        }

        /* Tag Reference */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            continue;
        }
        strcpy(reference, token);

        /* Chars num */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            continue;
        }

        /* Description */
        p = mystrtok(p, token, SEPARATOR);
        if (p == NULL && token[0] == '\0')
        {
            LOG_PRINT(error_e, "Invalid tag '%s'\n", line);
            continue;
        }
        TagTable.insert(reference,token);
        LOG_PRINT(verbose_e, "New tag '%s' - '%s'\n", reference,token);
    }
    LOG_PRINT(error_e, "Loaded '%d' Tag\n", TagTable.count());
    return true;
}

bool CommStart()
{
        /* Load the cross-table in order to allocate the ioArea to the right size (should be fixed size) and fill the syncro table */
        int elem_read = fillSyncroArea();
        if (elem_read < 0)
        {
            LOG_PRINT(error_e, "Cannot found the cross table [%s]\n", CROSS_TABLE);
            QMessageBox::critical(0,QApplication::tr("Cross Table Check"), QApplication::tr("Cannot found the cross table %1\nMSG: '%2'").arg(CROSS_TABLE).arg(CrossTableErrorMsg));
            return false;
        }
        else if (elem_read != DB_SIZE_ELEM)
        {
            LOG_PRINT(error_e, "Cannot load completly the cross table [%dvs%d]\n", elem_read, DB_SIZE_ELEM);
            QMessageBox::critical(0,QApplication::tr("Cross Table Check"), QApplication::tr("Syntax error into the cross table at line %1\nMSG: '%2'").arg(elem_read).arg(CrossTableErrorMsg));
            return false;
        }

        /* start the io layers */

        ioComm = new io_layer_comm(&data_send_mutex, &data_recv_mutex);
        LOG_PRINT(info_e, "Starting IOLayer Data\n");

        /* setting output data area size */
        SET_SIZE_BYTE(IODataAreaO + DB_OUT_BASE_BYTE, DB_SIZE_ELEM);
        /* setting status area size */
        SET_SIZE_BYTE(IODataAreaO + STATUS_BASE_BYTE, DB_SIZE_ELEM);

        /* setting input data area size (shouldn't be necessary) -- it has been checked at least for array init*/
        //SET_SIZE_BYTE(IODataAreaI + DB_IN_BASE_BYTE, DB_SIZE_ELEM);
        //SET_SIZE_BYTE(IODataAreaI + STATUS_BASE_BYTE, DB_SIZE_ELEM);

        if (ioComm->initializeData(LOCAL_SERVER_ADDR, LOCAL_SERVER_DATA_RX_PORT, LOCAL_SERVER_DATA_TX_PORT, IODataAreaI, STATUS_BASE_BYTE + DB_SIZE_BYTE, IODataAreaO, STATUS_BASE_BYTE + DB_SIZE_BYTE) == false)
        {
            LOG_PRINT(error_e, "Cannot connect to the Data IOLayer\n");
            QMessageBox::critical(0,QApplication::tr("Connection"), QApplication::tr("Cannot connect to the Data IOLayer"));
            return false;
        }

        /* setting output syncro area size, for the corresponding reading area it's not necessary */
        SET_SIZE_WORD(IOSyncroAreaO + SYNCRO_BASE_BYTE, DB_SIZE_ELEM);

        if (ioComm->initializeSyncro(LOCAL_SERVER_ADDR, LOCAL_SERVER_SYNCRO_RX_PORT, LOCAL_SERVER_SYNCRO_TX_PORT, IOSyncroAreaI, SYNCRO_SIZE_BYTE, IOSyncroAreaO, SYNCRO_SIZE_BYTE) == false)
        {
            LOG_PRINT(error_e, "Cannot connect to the Syncro IOLayer\n");
            QMessageBox::critical(0,QApplication::tr("Connection"), QApplication::tr("Cannot connect to the Data IOLayer"));
            return false;
        }
        ioComm->start();
        LOG_PRINT(info_e, "IOLayer Syncro Started\n");

#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
        logger = new Logger(NULL, NULL, LogPeriodSecS*1000);
        /* start the logger thread */
        logger->start();
        LOG_PRINT(info_e, "Logger Started\n");
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
            LOG_PRINT(info_e, "Password %d '%s' = '%d'\n", i, PasswordsString[i], passwords[i]);
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
        strcpy(_language_, "it");
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
     }
     else if (mode == usb_host_e)
     {
         settings.setValue(USB_MODE, HOST_TAG);
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
         LOG_PRINT(info_e, "Usb already mounted to '%s'\n", usb_mnt_point);
         return true;
     }

     if (USBmode() == usb_device_e)
     {
         char command[256];
         sprintf(command,"LOOP=`losetup -f` && losetup -o 4096 $LOOP %s && mount -t vfat $LOOP %s", BACKING_FILE, MOUNT_POINT);
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
                 LOG_PRINT (info_e, "usb waiting free \n");
                 retry_nb++;
             }
             while(USBfeedback[0] != 0 /*|| strlen(Usb_mpoint(1)) != 0*/ && retry_nb < RETRY_NB);
             LOG_PRINT (info_e, "found usb USBfeedback[1] %d\n", USBfeedback[1]);
             /* mount the key */
             if (USBfeedback[1] != 0 || Usb_on(1) != 0)
             {
                 LOG_PRINT (error_e, "cannot mount the USB! USBfeedback[1] = %d\n", USBfeedback[1]);
                 usb_mnt_point[0] = '\0';
                 return false;
             }
             strcpy(usb_mnt_point, Usb_mpoint(1));
             LOG_PRINT (info_e, "Found USB. Mountpint: '%s'\n", usb_mnt_point);
             return true;
         }
         else if (strlen(usb_mnt_point) != 0)
         {
             LOG_PRINT (verbose_e, "Already mounted to '%s'\n", usb_mnt_point);
             return true;
         }
         else
         {
             LOG_PRINT (info_e, "Cannot found USB.\n");
         }

         if (last_usb_status != USBstatus[0])
         {
             LOG_PRINT(info_e, "extracted USB\n");
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
         LOG_PRINT (warning_e, "USB already unmounted cause usb_mnt_point = '%s'\n", usb_mnt_point);
         return false;
     }
     if (USBmode() == usb_device_e)
     {
         char command[256];
         sprintf(command,"LOOP=`losetup -f | tr \"\\/dev\\/loop\" \" \" | awk '{printf(\"%%d\\n\", $1 - 1);}'` && umount %s && losetup -d /dev/loop$LOOP", MOUNT_POINT);
         if (system (command) == 0)
         {
             LOG_PRINT(error_e, "Cannot unmount: %s\n", strerror(errno));
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
