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
#include "timepopup.h"
#include "calendar.h"
#include <QDirIterator>
#include <QFontDatabase>
#include <QMetaObject>
//#include "app_config.h"
#include "pthread.h"
#include "hmi_plc.h"
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

    LogPeriodSecS = settings.value(LOG_PERIOD_SLOW_TAG, LOG_PERIOD_SLOW_DEF).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", LOG_PERIOD_SLOW_TAG, LogPeriodSecS);

    LogPeriodSecF = settings.value(LOG_PERIOD_FAST_TAG, LOG_PERIOD_FAST_DEF).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", LOG_PERIOD_FAST_TAG, LogPeriodSecF);

    MaxWindowSec = settings.value(WINDOW_SEC_TAG, 60).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", WINDOW_SEC_TAG, MaxWindowSec);

    MaxLogUsageMb = settings.value(MAX_SPACE_AVAILABLE_TAG, MAX_SPACE_AVAILABLE_DEF).toInt();
    MaxLogUsageMb = (MaxLogUsageMb < 0) ? 0 : MaxLogUsageMb;
    if (MaxLogUsageMb == 0)
    {
        LOG_PRINT(warning_e, "No space available for logs [%d]. No logs will be recorded.\n", MaxLogUsageMb);
    }
    LOG_PRINT(verbose_e, "'%s' = %d\n", MAX_SPACE_AVAILABLE_TAG, MaxLogUsageMb);

    ScreenSaverSec = settings.value(SCREENSAVER_TAG, SCREENSAVER_DEFAULT_TIME/1000).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", SCREENSAVER_TAG, ScreenSaverSec);

    PwdTimeoutSec = settings.value(PWD_TIMEOUT_SEC_TAG, PWD_TIMEOUT_SEC_DEF).toInt();
    LOG_PRINT(verbose_e, "'%s' = %d\n", PWD_TIMEOUT_SEC_TAG, PwdTimeoutSec);

    strcpy(PwdLogoutPage, settings.value(PWD_LOGOUT_PAGE_TAG, PWD_LOGOUT_PAGE_DEF).toString().toAscii().data());
    LOG_PRINT(verbose_e, "'%s' = %s\n", PWD_LOGOUT_PAGE_TAG, PwdLogoutPage);

    strcpy(_language_, settings.value(LANGUAGE_TAG, DEFAULT_LANGUAGE).toString().toAscii().data());
    LOG_PRINT(verbose_e, "'%s' = %s\n", LANGUAGE_TAG, _language_);

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

    settings.setValue(LOG_PERIOD_SLOW_TAG, LogPeriodSecS);
    settings.setValue(LOG_PERIOD_FAST_TAG, LogPeriodSecF);
    settings.setValue(WINDOW_SEC_TAG, MaxWindowSec);
    settings.setValue(MAX_SPACE_AVAILABLE_TAG, MaxLogUsageMb);

    settings.setValue(SCREENSAVER_TAG, ScreenSaverSec);

    settings.setValue(PWD_TIMEOUT_SEC_TAG, PwdTimeoutSec);
    settings.setValue(PWD_LOGOUT_PAGE_TAG, PwdLogoutPage);

    settings.setValue(LANGUAGE_TAG, _language_);

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
            LOG_PRINT(verbose_e, "skipping blank line\n");
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

        step = 0;
        for (step = 0; step < MAX_RCP_STEP; ++step)
        {
            p = strtok_csv(NULL, SEPARATOR, &r);
            if (p == NULL)
            {
                // fine riga
                if (step < step_max) {
                    // mancano valori
                    LOG_PRINT(error_e, "Missing columns (only %d vs %d) at line %d\n", step, step_max, line_nb);
                    value = 0;
                    while (step < step_max) {
                        table[step].append(value);
                        ++step;
                    }
                }
                break;
            }
            if (line_nb == 0) {
                // contiamo
                step_max = step + 1;
            } else if (line_nb > 0 && step >= step_max) {
                // colonne superflue
                LOG_PRINT(error_e, "Too many columns %d at line %d\n", step, line_nb);
                break;
            }

            value = 0;
            // compute value
            switch (varNameArray[ctIndex].type) {

            case byte_e: {
                u_int8_t val_uint8;

                if (decimal > 0) {
                    double var_double = strtod(p, NULL);

                    for (int n = 0; n < decimal; ++n) {
                        var_double = var_double * 10.0;
                    }
                    val_uint8 = (u_int8_t)var_double;
                } else {
                    val_uint8 = strtoul(p, NULL, 0);
                }
                value = (u_int32_t)val_uint8;
            }   break;

            case uintab_e:
            case uintba_e: {
                u_int16_t val_uint16;

                if (decimal > 0) {
                    double var_double = strtod(p, NULL);

                    for (int n = 0; n < decimal; ++n) {
                        var_double = var_double * 10.0;
                    }
                    val_uint16 = (u_int16_t)var_double;
                } else {
                    val_uint16 = strtoul(p, NULL, 10);
                }
                memcpy(&value, &val_uint16, sizeof(val_uint16));
            }   break;

            case intab_e:
            case intba_e: {
                int16_t val_int16;

                if (decimal > 0) {
                    double var_double = strtod(p, NULL);

                    for (int n = 0; n < decimal; ++n) {
                        var_double = var_double * 10.0;
                    }
                    val_int16 = (int16_t)var_double;
                } else {
                    val_int16 = strtol(p, NULL, 10);
                }
                memcpy(&value, &val_int16, sizeof(val_int16));
            }   break;

            case udint_abcd_e:
            case udint_badc_e:
            case udint_cdab_e:
            case udint_dcba_e: {
                u_int32_t val_uint32;

                if (decimal > 0) {
                    double var_double = strtod(p, NULL);

                    for (int n = 0; n < decimal; ++n) {
                        var_double = var_double * 10.0;
                    }
                    val_uint32 = (u_int32_t)var_double;
                } else {
                    val_uint32 = strtoul(p, NULL, 10);
                }
                memcpy(&value, &val_uint32, sizeof(val_uint32));
            }   break;

            case dint_abcd_e:
            case dint_badc_e:
            case dint_cdab_e:
            case dint_dcba_e: {
                int32_t val_int32;

                if (decimal > 0) {
                    double var_double = strtod(p, NULL);

                    for (int n = 0; n < decimal; ++n) {
                        var_double = var_double * 10.0;
                    }
                    val_int32 = (int32_t)var_double;
                } else {
                    val_int32 = strtol(p, NULL, 10);
                }
                memcpy(&value, &val_int32, sizeof(val_int32));
            }   break;

            case fabcd_e:
            case fbadc_e:
            case fcdab_e:
            case fdcba_e: {
                float val_float;
                val_float = strtof(p, NULL);
                memcpy(&value, &val_float, sizeof(val_float));
            }   break;

            case bytebit_e:
            case wordbit_e:
            case dwordbit_e:
            case bit_e: {
                u_int8_t val_bit;
                val_bit = strtol(p, NULL, 0);
                value = (u_int32_t)(val_bit ? 1 : 0);
            }   break;

            default:
                /* unknown type */
                value = 0;
            }

            // assign step value
            table[step].append(value);
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
        int ivalue = 0;
        int ctIndex = (int)(indexes->at(i));
        switch (readFromDbQuick(ctIndex, &ivalue))
        {
        case DONE:
        case BUSY:
            table[step].replace(i, ivalue);
            break;
        case ERROR:
        default:
            LOG_PRINT(error_e, "Error reading recipe step #%d for '%s'\n", step, varNameArray[ctIndex].tag);
            errors++;
        }
    }
    return errors;
}

int writeRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[])
{
    if (step >= MAX_RCP_STEP)
    {
        return -1;
    }
    LOG_PRINT(info_e, "writeRecipe() %d (step=%d,indexes=%p) .....\n", table[step].count(), step, indexes);

    beginWrite();
    for (int i = 0; i < table[step].count(); i++)
    {
        char retval;

        u_int16_t ctIndex = indexes->at(i);
        u_int32_t value = table[step].at(i);        

        retval = prepareWriteVarByCtIndex(ctIndex, value, 0);

        switch (retval) {
        case DONE:
            break;
        case BUSY:
        case ERROR:
            LOG_PRINT(error_e, "..... BUSY @%d/%d (step=%d,indexes=%p)\n", i, table[step].count(), step, indexes);
            return 1;
        default:
            LOG_PRINT(error_e, "..... ERROR @%d/%d (step=%d,indexes=%p)\n", i, table[step].count(), step, indexes);
            return 1;
        }
    }
    endWrite();

    LOG_PRINT(info_e, "..... %d (step=%d,indexes=%p)\n", table[step].count(), step, indexes);
    return 0;
}

int checkRecipe(int step, QList<u_int16_t> *indexes, QList<u_int32_t> table[])
{
    int allOk = 1;

    if (step >= MAX_RCP_STEP)
    {
        LOG_PRINT(error_e, "checkRecipe() wrong step %d\n", step);
        return 0;
    }
    pthread_mutex_lock(&datasync_recv_mutex);
    {
        /* search for pending writes on the recipe variables */
        for (int i = 0; i < table[step].count(); ++i)
        {
            u_int16_t ctIndex = indexes->at(i);
            u_int32_t recipeValue = table[step].at(i);
            u_int32_t actualValue = 0;

            readFromDb(ctIndex, &actualValue);
            if (recipeValue != actualValue) {
                if (allOk) {
                    allOk = 0;
                    LOG_PRINT(error_e, "checkRecipe() failed for:\n");
                }
                LOG_PRINT(error_e, "\t%u/%d %s = 0x%08x vs 0x%08x\n", i, table[step].count(),
                          varNameArray[ctIndex].tag, actualValue, recipeValue);
            }
        }
    }
    pthread_mutex_unlock(&datasync_recv_mutex);

    return allOk;
}


bool CommStart()
{
    int elem_read = fillSyncroArea();

    if (elem_read < 0)
    {
        LOG_PRINT(error_e, "cannot find the Crosstable [%s]\n", CROSS_TABLE);
        QMessageBox::critical(0,QApplication::trUtf8("Crosstable Check"), QApplication::trUtf8("Cannot find the Crosstable %1\nMSG: '%2'").arg(CROSS_TABLE).arg(CrossTableErrorMsg));
        return false;
    }
    else if (elem_read < DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "cannot load completly the Crosstable [%dvs%d]\n", elem_read, DB_SIZE_ELEM);
        QMessageBox::critical(0,QApplication::trUtf8("Crosstable Check"), QApplication::trUtf8("Syntax error into the Crosstable at line %1\nMSG: '%2'").arg(elem_read).arg(CrossTableErrorMsg));
        return false;
    }
    else if (elem_read > DB_SIZE_ELEM)
    {
        LOG_PRINT(error_e, "Too many variables into the Crosstable [%dvs%d]\n", elem_read, DB_SIZE_ELEM);
        QMessageBox::critical(0,QApplication::trUtf8("Crosstable Check"), QApplication::trUtf8("Syntax error into the Crosstable at line %1\nMSG: '%2'").arg(elem_read).arg(CrossTableErrorMsg));
        return false;
    }
    timepopup = new TimePopup(NULL);
    calendarpopup  = new Calendar(NULL);
    ntpclient = new NtpClient(NULL);

    logger = new Logger(NULL, NULL, LogPeriodSecS*1000);

    /* start the udp communication */
    ioComm = new io_layer_comm(&datasync_send_mutex, &datasync_recv_mutex);
    ioComm->start();

    /* start the logger thread */
    logger->start();

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
    mkdir(ALARMS_DIR, S_IRWXU | S_IRWXG);
    mkdir(STORE_DIR, S_IRWXU | S_IRWXG);
    mkdir(CUSTOM_STORE_DIR, S_IRWXU | S_IRWXG);
    mkdir(CUSTOM_TREND_DIR, S_IRWXU | S_IRWXG);
    mkdir(SCREENSHOT_DIR, S_IRWXU | S_IRWXG);
    mkdir(RECIPE_DIR, S_IRWXU | S_IRWXG);

    /* initialize the usb API */
#ifdef ENABLE_USB
    app_usb_init();
#endif

#if defined(TARGET)
    /* Set up screen saver */
    QWSServer::setScreenSaver( new ScreenSaver );
    QWSServer::setScreenSaverInterval(ScreenSaverSec * 1000); //msec
    QWSServer::setScreenSaverBlockLevel(0);
#endif
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
    /* finalyze and free the logger */
    logger->exit();
    delete logger;

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
    int volume = 100;
    unsigned on_cs = duration_ms / 10;
    unsigned off_cs = 0;
    unsigned replies = 1;
    unsigned value;

    if (varNameArray[ID_PLC_BUZZER].tag[0]) {
        // new buzzer management
        readFromDbQuick(ID_PLC_TOUCH_VOLUME, &volume);
        value = (volume & 0xFF) + (on_cs << 8) + (off_cs << 16) + (replies << 24);
        doWrite(ID_PLC_BUZZER, &value);
    } else {
        // workaround for old projects
        static int64_t buzzer_busy_timeout_ms = 0;
        static int64_t now_ms = 0;
        struct timespec now;

        /* No buzzer */
        if (Buzzerfd == -1)
            return false;

        /* Milliseconds since some time reference */
        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
            return false;
        now_ms = now.tv_sec * 1000 + now.tv_nsec / 1000000;

        /* Buzz for duration_ms (if done buzzing). */
        if (buzzer_busy_timeout_ms <= now_ms) {
            (void)ioctl(Buzzerfd, BUZZER_BEEP, duration_ms);
            buzzer_busy_timeout_ms = now_ms + duration_ms;
        }
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

    LOG_PRINT(warning_e, "level set to: %d", level);
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

bool LoadTrend(const char * trend_name, QString * ErrorMsg)
{
    FILE * fp;
    char line[LINE_SIZE] = "";
    char * p = NULL, *r = NULL;

    char filename[FILENAME_MAX];
    if (QFileInfo(trend_name).suffix().compare("csv") == 0)
    {
        sprintf(filename, "%s/%s", CUSTOM_TREND_DIR, trend_name);
    }
    else
    {
        sprintf(filename, "%s/%s.csv", CUSTOM_TREND_DIR, trend_name);
    }

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
        pens[rownb].visible = 0;
        pens[rownb].color[0] = '\0';
        pens[rownb].description[0] = '\0';
        pens[rownb].sample = 0;
        pens[rownb].tag[0] = '\0';
        pens[rownb].visible = 0;
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
            pens[rownb].visible = 0;
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
            LOG_PRINT(info_e, "Empty tag '%s' at line %d\n", p, rownb+1);
            pens[rownb].tag[0] = '\0';
            pens[rownb].visible = 0;
        }
        else if (Tag2CtIndex(p, &index) != 0)
        {
            LOG_PRINT(error_e, "The variable '%s' is not present into the Crosstable\n", p);
            pens[rownb].tag[0] = '\0';
            pens[rownb].visible = 0;
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

QObject *getPage(QObject *plugin)
{
    QObject *ancestor = plugin;

    while (ancestor != NULL)
    {
        const char *className = ancestor->metaObject()->className();

        if (strncmp(className, "page", 4) == 0)
        {
            break;
        }
        ancestor = ancestor->parent();
    }
    return ancestor;
}

