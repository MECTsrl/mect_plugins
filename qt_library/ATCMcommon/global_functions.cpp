#include <QMessageBox>
#include <QFile>
#include <QSettings>
#include "global_functions.h"
#include "app_logprint.h"
#include "screensaver.h"
#include "hmi_logger.h"
#include "cross_table_utility.h"
#include "app_usb.h"
#include "defines.h"

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
        printf("Cannot open password file\n");
        return 1;
    }
    for (i = 0; i < PASSWORD_NB; i++)
    {
        if (fread(&passwords[i], 4,1, out)==0)
        {
            printf("Cannot read password %d\n", i);
            return 1;
        }
        printf("password[%d] = %d\n", i, passwords[i]);
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
            printf("dump password[%d] = %d\n", i, passwords[i]);
        }
        fclose(out);
        return 0;
    }
    else
    {
        printf("Cannot create password file\n");
        return 1;
    }
}

/**
 * @brief readCfgFile: read the config file and fill the service area into the ioLayer
 */
int readCfgFile(void)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);

    LOG_PRINT(info_e, "Reading configuration...\n");

    strcpy(HomePage, settings.value(HOMEPAGE_TAG, HOMEPAGE_DEF).toString().toAscii().data());
    LOG_PRINT(info_e, "'%s' = %s\n", HOMEPAGE_TAG, HomePage);

    strcpy(HomePage, settings.value(STARTPAGE_TAG, STARTPAGE_DEF).toString().toAscii().data());
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
 * @brief writeCfgFile: write into the config file the data into the ioLayer service area
 */
int writeCfgFile(void)
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

        int retry = 0;
        while(IS_ENGINE_READY == 0)
        {
            LOG_PRINT(info_e, "Waiting communication engine\n");
            if(retry > WAIT_COMM_RETRY_NB)
            {
                retry = 0;
                QMessageBox::critical(0, "Communication Error", "Problem to start communication engine.");
            }
            retry++;
            sleep(1);
        }

        LOG_PRINT(info_e, "Communication engine started\n");

        if (IS_COMMPAR_ERROR)
        {
            LOG_PRINT(error_e, "Commpar error\n");
            QMessageBox::critical(0,QApplication::tr("Commpar Table Check"), QApplication::tr("A problem occour when the communication paramenters are loading"));
            return false;
        }
        if (IS_CROSSTABLE_ERROR)
        {
            LOG_PRINT(error_e, "CrossTable error\n");
            QMessageBox::critical(0,QApplication::tr("Cross Table Check"), QApplication::tr("A problem occour when the CrossTable variables are loading"));
            return false;
        }
        if (IS_ALARMSTBL_ERROR)
        {
            if (QFile::exists(ERROR_TABLE))
            {
                LOG_PRINT(error_e, "Alarms error\n");
                QMessageBox::critical(0,QApplication::tr("Alarms Table Check"), QApplication::tr("A problem occour when the alarms are loading"));
                return false;
            }
        }

        /* disable the device connected to a protocol not started */
        if (IS_RTU_ENABLED == 0)
        {
            for (int node = 0; node < MAX_DEVICE_NB; node++)
            {
                device_status[prot_rtu_e][node] = 0;
            }
        }
        if (IS_TCP_ENABLED == 0)
        {
            for (int node = 0; node < MAX_DEVICE_NB; node++)
            {
                device_status[prot_tcp_e][node] = 0;
            }
        }
        if (IS_TCPRTU_ENABLED == 0)
        {
            for (int node = 0; node < MAX_DEVICE_NB; node++)
            {
                device_status[prot_tcprtu_e][node] = 0;
            }
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
    readCfgFile();
    /* dump the value (if is the first time write the default value */
    writeCfgFile();

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
