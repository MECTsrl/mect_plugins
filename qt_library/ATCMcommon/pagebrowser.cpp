/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Page browswer base class
 */
#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <math.h>
#include <QProcess>
#include <QFile>
#include <QSettings>

#include <errno.h>

#include "utility.h"
#include "app_logprint.h"
#include "pagebrowser.h"
#include "app_config.h"
#ifdef ENABLE_AUTODUMP
#include "app_usb.h"
#endif

#include "ATCMsystem/system_ini.h"
#include "ATCMsystem/net_conf.h"
#include "ATCMsystem/data_manager.h"
#include "ATCMsystem/display_settings.h"
#include "ATCMsystem/display_test.h"
#include "ATCMsystem/info.h"
#include "ATCMsystem/menu.h"
#include "ATCMsystem/options.h"
#include "ATCMsystem/time_set.h"
#include "page0.h"
#include "defines.h"
#include "global_functions.h"

#ifdef ENABLE_ALARMS
#include "alarms.h"
#include "alarms_history.h"
#endif

#ifdef ENABLE_STORE
#include "store.h"
#include "store_filter.h"
#endif

#ifdef ENABLE_TREND
#include "trend.h"
#include "trend_other.h"
#include "trend_option.h"
#include "trend_range.h"
#endif

#ifdef ENABLE_RECIPE
#include "recipe.h"
#include "recipe_select.h"
#endif

#define RETRY_NB 5


/** @brief variables used for the change page management */
QStack<QString> History;
QHash<QString, page *> ScreenHash;

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
page::page(QWidget *parent) :
    QWidget(parent)
{
    /* set up the time for set the refresh timeout */
    refresh_timer = new QTimer(this);
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    refresh_timer->start(REFRESH_MS);
    labelDataOra = NULL;
    labelUserName = NULL;
    labelIcon = NULL;
    labelTitle = NULL;
    /* set the default protection level */
    protection_level = pwd_operator_e;
    //char vncDisplay[64];
    //printVncDisplayString(vncDisplay);
    if (ID_FORCE_BUZZER == -1)
    {
        Tag2CtIndex("PLC_buzzerOn", &ID_FORCE_BUZZER);
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void page::updateData()
{
#ifdef ENABLE_ALARMS
    setAlarmsBuzzer();
#endif

    /* FIXME: use the static index of the new fix variable FORCE_BUZZER */
    if (ID_FORCE_BUZZER != -1)
    {
        bool FORCE_BUZZER;
        if (readFromDb(ID_FORCE_BUZZER, &FORCE_BUZZER) == 0 && FORCE_BUZZER == true)
        {
            beep(BUZZER_DURATION_MS);
        }
    }

    if (this->isVisible() == false)
    {
        LOG_PRINT(warning_e, "page '%s' not visible but running.\n", this->windowTitle().toAscii().data());
        return;
    }
#ifdef ENABLE_AUTODUMP
    {
        static bool inserted = false;
        /* Check USB key */
        if (USBCheck())
        {
            if ( inserted == false)
            {
                inserted = true;
                if (USBmount() == false)
                {
                    LOG_PRINT(error_e, "cannot mount the usb key\n");
                    QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot mount the usb key"));
                    return;
                }
                
                char srcfilename[FILENAME_MAX];
                char dstfilename[FILENAME_MAX];
                /* compose the source file name ans the destination file name */
                sprintf(srcfilename, "%s", STORE_DIR);
                sprintf(dstfilename, "%s/%s.zip",
                        usb_mnt_point,
                        QDateTime::currentDateTime().toString("yy_MM_dd_hh_mm_ss").toAscii().data());
                
                /* zip the file, the sign file and delete them */
                if (zipAndSave(QStringList() << srcfilename, QString(dstfilename), true) == false)
                {
                    USBumount();
                    QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1.sign'").arg(srcfilename));
                    return;
                }
                
                //QFile::remove(dstfilename);
                //QFile::remove(QString("%1.sign").arg(dstfilename));
                
                /* unmount USB key */
                USBumount();
                LOG_PRINT(verbose_e, "DOWNLOADED\n");
                QMessageBox::information(this,trUtf8("USB info"), trUtf8("File '%1' saved.").arg(dstfilename));
            }
        }
        else
        {
            LOG_PRINT(verbose_e, "USB REMOVED\n");
            inserted = false;
        }
    }
#endif
    /* check the password timeout in order to logout */
    if (PwdTimeoutSec > 0 && active_password != pwd_operator_e)
    {
        struct timespec Now;
        clock_gettime(CLOCK_REALTIME, &Now);
        if (PwdTimeoutSec < (int)(Now.tv_sec - LastTouch.tv_sec))
        {
            active_password = pwd_operator_e;
            if (windowTitle().compare(PwdLogoutPage))
            {
                LOG_PRINT(verbose_e, "password expire after %d seconds. going to '%s' page\n", PwdTimeoutSec, PwdLogoutPage);
                goto_page(PwdLogoutPage);
            }
        }
    }
    
    /* add here te code to be execute everytime */
    if (labelDataOra)
    {
        labelDataOra->setText(QDateTime::currentDateTime().toString("dd/MM/yyyy - HH:mm:ss"));
    }
    if (labelUserName)
    {
        labelUserName->setText(trUtf8("User: %1").arg(PasswordsString[active_password]));
    }
#ifdef ENABLE_DEVICE_DISCONNECT
    /*Re-activate variables local to a page in case of device reconnection*/
    if(DeviceReconnected)
    {
        this->reload();
        DeviceReconnected = 0;
    }
#endif
    
    checkWriting();

    /*
    if(IS_ENGINE_READY == 0)
    {
        QMessageBox::critical(0, "Communication Error", QString("Problem to start communication engine [0x%1].").arg(QString::number((IOSyncroAreaI)[5707]),16));
    }

    LOG_PRINT(verbose_e, "Communication engine started\n");
    */
#ifdef DUMPSCREEN
    QPixmap::grabWidget(this).save(this->windowTitle().append(".png").prepend("/local/root/"));
#endif
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
page::~page()
{
}

bool page::getFormattedVar(const char * varname, bool * formattedVar, QLabel * led)
{
    int ctIndex;
    bool return_value = false;
    
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = VAR_DISCONNECT;
        return return_value;
    }
#endif
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    int type = CtIndex2Type(ctIndex);
    
    if (
            type == byte_e ||
            type == uintab_e ||
            type == uintba_e ||
            type == intab_e ||
            type == intba_e ||
            type == udint_abcd_e ||
            type == udint_badc_e ||
            type == udint_cdab_e ||
            type == udint_dcba_e ||
            type == dint_abcd_e ||
            type == dint_badc_e ||
            type == dint_cdab_e ||
            type == dint_dcba_e ||
            type == fabcd_e ||
            type == fbadc_e ||
            type == fcdab_e ||
            type == fdcba_e
            )
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    /* bit */
    else
    {
        BYTE _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_ERROR);
            return return_value;
        }
        
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            *formattedVar = (_value == 1);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            *formattedVar = (_value == 1);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
    }
    
    LOG_PRINT(verbose_e, "HEX %X - FORMATTED BIT '%s': '%d'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *formattedVar);
    return return_value;
    
}

bool page::getFormattedVar(const char * varname, short int * formattedVar, QLabel * led)
{
    int ctIndex;
    bool return_value = false;
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = (short int)VAR_NAN;
        return return_value;
    }
#endif
    
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);
    
    if (varNameArray[ctIndex].decimal > 0)
    {
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    if (
            CtIndex2Type(ctIndex) == intab_e ||
            CtIndex2Type(ctIndex) == intba_e
            )
    {
        if (readFromDb(ctIndex, formattedVar) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = false;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
    }
    else
    {
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%d'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *formattedVar);
    return return_value;
}

bool page::getFormattedVar(const char * varname, unsigned short int * formattedVar, QLabel * led)
{
    int ctIndex;
    bool return_value = false;
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = (unsigned short int)VAR_NAN;
        return return_value;
    }
#endif
    
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);
    
    if (varNameArray[ctIndex].decimal > 0)
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    if (
            CtIndex2Type(ctIndex) == uintab_e ||
            CtIndex2Type(ctIndex) == uintab_e
            )
    {
        if (readFromDb(ctIndex, formattedVar) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
    }
    else
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%d'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *formattedVar);
    return return_value;
}

bool page::getFormattedVar(const char * varname, int * formattedVar, QLabel * led)
{
    int ctIndex;
    bool return_value = false;
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = (int)VAR_NAN;
        return return_value;
    }
#endif
    
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);
    
    if (varNameArray[ctIndex].decimal > 0)
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    if 	(
         CtIndex2Type(ctIndex) == dint_abcd_e ||
         CtIndex2Type(ctIndex) == dint_badc_e ||
         CtIndex2Type(ctIndex) == dint_cdab_e ||
         CtIndex2Type(ctIndex) == dint_dcba_e
         )
    {
        if (readFromDb(ctIndex, formattedVar) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
    }
    else
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%d'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *formattedVar);
    return return_value;
}

bool page::getFormattedVar(const char * varname, unsigned int * formattedVar, QLabel * led)
{
    int ctIndex;
    bool return_value = false;
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = (unsigned int)VAR_NAN;
        return return_value;
    }
#endif
    
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);
    
    if (varNameArray[ctIndex].decimal > 0)
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    if (
            CtIndex2Type(ctIndex) == udint_abcd_e ||
            CtIndex2Type(ctIndex) == udint_badc_e ||
            CtIndex2Type(ctIndex) == udint_cdab_e ||
            CtIndex2Type(ctIndex) == udint_dcba_e
            )
    {
        if (readFromDb(ctIndex, formattedVar) != 0)
        {
            LOG_PRINT(error_e, "readFromDb\n");
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
    }
    else
    {
        LOG_PRINT(error_e, "Invalid type\n");
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%d'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *formattedVar);
    return return_value;
}

bool page::getFormattedVar(const char * varname, float * formattedVar, QLabel * led)
{
    int ctIndex;
    bool return_value = false;
    char fmt[8] = "";
    int decimal = 0;
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = (float)VAR_NAN;
        return return_value;
    }
#endif
    
    if (Tag2CtIndex(varname, &ctIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "HEXADECIMAL CTI: %d BYTE %d - '%s': 0x%X\n", ctIndex, (ctIndex - 1) * 4, varNameArray[ctIndex].tag, pIODataAreaI[(ctIndex - 1) * 4]);
    
    if (varNameArray[ctIndex].decimal > 4)
    {
        if (readFromDb(varNameArray[ctIndex].decimal, &decimal) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            decimal = 0;
        }
    }
    else if (varNameArray[ctIndex].decimal > 0)
    {
        LOG_PRINT(verbose_e, "Decimal %d\n", varNameArray[ctIndex].decimal);
        decimal = varNameArray[ctIndex].decimal;
    }
    else
    {
        decimal = 0;
    }
    switch (getStatusVarByCtIndex(ctIndex, NULL))
    {
    case BUSY:
        if(led) led->setStyleSheet(STYLE_PROGRESS);
        return_value = true;
        break;
    case ERROR:
        if(led) led->setStyleSheet(STYLE_ERROR);
        return_value = false;
        break;
    case DONE:
        if(led) led->setStyleSheet(STYLE_READY);
        return_value = true;
        break;
    default:
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        return_value = false;
        break;
    }
    if (return_value == false)
    {
        return return_value;
    }
    
    LOG_PRINT(verbose_e, "CURRENT Decimal is %d to be used for VARIABLE %s\n", decimal, varNameArray[ctIndex].tag);
    
    if (decimal > 0)
    {
        sprintf (fmt, "%%.%df", decimal);
    }
    
    switch(	CtIndex2Type(ctIndex))
    {
    case uintab_e:
    case uintba_e:
    {
        unsigned short int _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
        if (return_value == false)
        {
            return return_value;
        }
        
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *formattedVar = (float)_value / pow(10,decimal);
        }
        else
        {
            *formattedVar = (float)_value;
        }
    }
        break;
    case intab_e:
    case intba_e:
    {
        short int _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
        if (return_value == false)
        {
            return return_value;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *formattedVar = (float)_value / pow(10,decimal);
        }
        else
        {
            *formattedVar = (float)_value;
        }
    }
        break;
    case udint_abcd_e:
    case udint_badc_e:
    case udint_cdab_e:
    case udint_dcba_e:
    {
        unsigned int _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
        if (return_value == false)
        {
            return return_value;
        }
        LOG_PRINT(verbose_e, "%s - value %u decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *formattedVar = (float)_value / pow(10,decimal);
        }
        else
        {
            *formattedVar = (float)_value;
        }
    }
        break;
    case dint_abcd_e:
    case dint_badc_e:
    case dint_cdab_e:
    case dint_dcba_e:
    {
        int _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
        if (return_value == false)
        {
            return return_value;
        }
        LOG_PRINT(verbose_e, "%s - value %d decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
        if (decimal > 0)
        {
            *formattedVar = (float)_value / pow(10,decimal);
        }
        else
        {
            *formattedVar = (float)_value;
        }
    }
        break;
    case fabcd_e:
    case fbadc_e:
    case fcdab_e:
    case fdcba_e:
    {
        float _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
        if (return_value == false)
        {
            return return_value;
        }
        LOG_PRINT(verbose_e, "%s - value %f decimal %d divisor %f\n", varNameArray[ctIndex].tag, _value, decimal, pow(10,decimal));
#if 0
        if (decimal > 0)
        {
            *formattedVar = (float)_value / pow(10,decimal);
        }
        else
#endif
        {
            *formattedVar = (float)_value;
        }
    }
        break;
    case bit_e:
    {
        BYTE _value;
        if (readFromDb(ctIndex, &_value) != 0)
        {
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return return_value;
        }
        switch (getStatusVarByCtIndex(ctIndex, NULL))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            return_value = false;
            break;
        }
        if (return_value == false)
        {
            return return_value;
        }
        *formattedVar = (float)(_value == 1);
    }
        break;
    default:
        break;
    }
    
    LOG_PRINT(verbose_e, "HEX %X - FORMATTED '%s': '%f'\n", pIODataAreaI[(ctIndex - 1) * 4], varNameArray[ctIndex].tag, *formattedVar);
    return return_value;
}

/**
 * @brief read the variable with tag varname and put the read value into QString formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QString * formattedVar, QLabel * led)
{
    int CtIndex;
    char value[TAG_LEN];
    char status[TAG_LEN];
#ifdef ENABLE_DEVICE_DISCONNECT
    if (isDeviceConnectedByVarname(varname) != 1)
    {
        LOG_PRINT(warning_e, "device disconnected for variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_DISCONNECT);
        *formattedVar = VAR_DISCONNECT;
        return false;
    }
#endif
    
    formattedVar->clear();
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "cannot extract ctIndex of variable '%s'\n", varname);
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        *formattedVar = VAR_UNKNOWN;
        return false;
    }
    
    if (formattedReadFromDb(CtIndex, value) == 0 && strlen(value) > 0)
    {
        bool return_value = true;
        switch (getStatusVarByCtIndex(CtIndex, status))
        {
        case BUSY:
            if(led) led->setStyleSheet(STYLE_PROGRESS);
            /*
                   if tthe following line is commented, after a write, I will see the new value immediatly
                   else I will see the new value after is fisically set
                 */
            //*formattedVar = value;
            return_value = true;
            break;
        case ERROR:
            if(led) led->setStyleSheet(STYLE_ERROR);
            if (strlen(status) > 0) *formattedVar = status;
            return_value = false;
            break;
        case DONE:
            if(led) led->setStyleSheet(STYLE_READY);
            *formattedVar = value;
            return_value = true;
            break;
        default:
            if(led) led->setStyleSheet(STYLE_UNKNOWN);
            *formattedVar = VAR_UNKNOWN;
            return_value = false;
            break;
        }
        LOG_PRINT(verbose_e, "%s '%s': '%s' status %d\n", (return_value == true) ? "TRUE" : "FALSE", varname, value,getStatusVarByCtIndex(CtIndex, status));
        return return_value;
    }
    else
    {
        if(led) led->setStyleSheet(STYLE_UNKNOWN);
        *formattedVar = VAR_UNKNOWN;
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QLabel formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QLabel * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        formattedVar->setText(value);
        return true;
    }
    else
    {
        formattedVar->setText(value);
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QLineEdit formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QLineEdit * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        formattedVar->setText(value);
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QPushButton formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QPushButton * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        formattedVar->setText(value);
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QSpinBox formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QSpinBox * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        formattedVar->setValue(value.toInt());
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QDoubleSpinBox formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QDoubleSpinBox * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        formattedVar->setValue(value.toFloat());
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QComboBox formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QComboBox * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        int index = formattedVar->findText(value);
        if (index >= 0)
        {
            if(formattedVar->currentIndex() != index)
            {
                formattedVar->setCurrentIndex(index);
            }
            else
            {
                LOG_PRINT(verbose_e,"Value not changed\n");
            }
        }
        else
        {
            /* if is not managed, put an empty string */
            LOG_PRINT(warning_e, "Cannot find data '%s' into selection.\n", value.toAscii().data());
            formattedVar->setEditable(true);
            formattedVar->setEditText("");
        }
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QComboBox formattedVar
 * and set the colour of QLabel led in according with the variable status.
 * Into the map variable yu need to put the mapping between the read value and text string as follow
 *      QList map << val1 << lab1 << val2 << lab2 << ... valn << labn;
 */
bool page::getFormattedVar(const char * varname, QComboBox * formattedVar, QStringList map, QLabel * led)
{
    QString value;
    int i;
    
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        LOG_PRINT(verbose_e, "data read '%s'.\n", value.toAscii().data());
        for (i = 0; i < map.count(); i+=2)
        {
            if (map.at(i).compare(value) == 0)
            {
                LOG_PRINT(verbose_e, "Found mapping '%s' -> '%s'.\n", map.at(i).toAscii().data(), map.at(i + 1).toAscii().data());
                break;
            }
        }
        if (i >= map.count())
        {
            /* if is not managed, put an empty string */
            LOG_PRINT(warning_e, "Cannot find data '%s' into selection.\n", value.toAscii().data());
            formattedVar->setEditable(true);
            formattedVar->setEditText("");
            return false;
        }
        
        int index = formattedVar->findText(map.at(i+1));
        if (index >= 0)
        {
            formattedVar->setCurrentIndex(index);
        }
        else
        {
            /* if is not managed, put an empty string */
            LOG_PRINT(warning_e, "Cannot find data '%s' into selection.\n", value.toAscii().data());
            formattedVar->setEditable(true);
            formattedVar->setEditText("");
            return false;
        }
        return true;
    }
    else
    {
        return false;
    }
}

#if 0
/**
 * @brief read the variable with tag varname and put the read value into QLCDNumber formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QLCDNumber * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        if (value.indexOf(".") >= 0)
        {
            formattedVar->display(value.toFloat());
        }
        else
        {
            formattedVar->display(value.toInt());
        }
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief read the variable with tag varname and put the read value into QwtTextLabel formattedVar
 * and set the colour of QLabel led in according with the variable status
 */
bool page::getFormattedVar(const char * varname, QwtTextLabel * formattedVar, QLabel * led)
{
    QString value;
    if (getFormattedVar(varname, &value, led) && value.length() > 0)
    {
        formattedVar->setText(value);
        return true;
    }
    else
    {
        return false;
    }
}
#endif
/**
 * @brief hide all pages
 */
bool page::hideAll(void)
{
    /* chek if some windows is still open */
    QHash<QString, page *>::const_iterator i;
    for ( i = ScreenHash.begin(); i != ScreenHash.end() && i.value() != NULL ; i++)
    {
        if ( i.value()->isVisible())
        {
            LOG_PRINT(verbose_e,"hiding (%s)...\n", i.value()->windowTitle().toAscii().data() );
            i.value()->hide();
            LOG_PRINT(verbose_e,"done!\n" );
        }
    }
    LOG_PRINT(verbose_e,"Everything hide.\n" );
    return 0;
}

/**
 * @brief go to a page named page_name
 */
bool page::goto_page(const char * page_name, bool remember)
{
    page * p = NULL;
    
    if (strcmp(page_name, "BACK") == 0)
    {
        return go_back();
    }
    if (strcmp(page_name, "HOME") == 0)
    {
        return go_home();
    }
    if (strcmp(page_name, "page0") == 0)
    {
        goto_page(StartPage, false);
    }
    
    LOG_PRINT(verbose_e, "page_name %s\n", page_name);
    
    refresh_timer->stop();
    LOG_PRINT(verbose_e, " %s TIMER STOP\n", this->windowTitle().toAscii().data());
    
    if (remember)
    {
        /* update history */
        History.push(this->windowTitle());
    }
    
    /* clear the variable of the actual page */
    emptySyncroElement();
    
#ifdef ENABLE_TREND
    /* destroy trend page */
    if (strcmp(page_name, "trend") == 0 && _last_layout_ != _layout_)
    {
        _last_layout_ = _layout_;
        if(ScreenHash.contains(page_name))
        {
            p = ScreenHash.take(page_name);
            //p->destroy();
            delete p;
            p = NULL;
        }
    }
#endif

    /*prepare the next page */
    if(!ScreenHash.contains(page_name))
    {
        create_next_page(&p, page_name);
        if (p == NULL)
        {
            LOG_PRINT(error_e,"Fail to create page '%s'\n", page_name);
            //mymutex.unlock();
            QMessageBox::critical(this,trUtf8("Access Denied"), trUtf8("The requested page '%1' doesn't exist.").arg(page_name));
            refresh_timer->start(REFRESH_MS);
            LOG_PRINT(verbose_e, " %s TIMER START\n", this->windowTitle().toAscii().data());
            return false;
        }
        ScreenHash.insert(page_name, p);
        LOG_PRINT(verbose_e,"CREATA NUOVA PAGINA %s\n", page_name);
    }
    else
    {
        p = ScreenHash.value(page_name);
        p->refresh_timer->stop();
        LOG_PRINT(verbose_e, " %s TIMER STOP\n", p->windowTitle().toAscii().data());
        LOG_PRINT(verbose_e,"reload existing page '%s'\n", page_name);
    }

    if (active_password > p->protection_level)
    {
        QMessageBox::critical(this,trUtf8("Access Denied"), trUtf8("Impossible access to '%1': inappropriate privilegies.").arg(p->windowTitle()));
        LOG_PRINT(verbose_e,"active %d, protection %d\n", active_password, p->protection_level);
        go_home();
        return false;
    }
    p->reload();

    if (p != this) {
        /* deactivate the old variables */
        if (this->variableList.count() == 0)
        {
            LOG_PRINT(verbose_e, "No variable to deactivate.\n");
        }
        else if (deactivateVarList(this->variableList) == false)
        {
            LOG_PRINT(error_e, "cannot deactivate the variable list\n");
        }

        /* activate the new variables */
        /* send to the plc the active flag for the active variable */
        /* this code will be active only if the variable VAR_TO_DISPLAY is not empty */
        if (p->variableList.count() == 0)
        {
            LOG_PRINT(verbose_e, "No variable to activate.\n");
        }
        else if (activateVarList(p->variableList) == false)
        {
            LOG_PRINT(error_e, "cannot activate the variable list\n");
        }
    }
    hideAll();
    //hideOthers(p);

    p->refresh_timer->start(REFRESH_MS);
    LOG_PRINT(verbose_e, " %s TIMER START\n", p->windowTitle().toAscii().data());
    p->SHOW();
    LOG_PRINT(verbose_e, "New page '%s' (%s) is shown.\n", p->windowTitle().toAscii().data(), page_name );
    return true;
}

/**
 * @brief go to the home page
 */
bool page::go_home(void)
{
    History.clear();
#ifdef ENABLE_TREND
    _trend_data_reload_ = true;
#endif
    if (goto_page(HomePage) == false)
    {
        return goto_page(HOMEPAGE_DEF);
    }
    return true;
}

/**
 * @brief go to the previous page
 */
bool page::go_back()
{
    if (isVisible() == false)
    {
        return false;
    }
    if (!History.isEmpty())
    {
        QString page_name = History.pop();
        LOG_PRINT(verbose_e, " FROM (%s) BACK TO (%s)\n", this->windowTitle().toAscii().data(), page_name.toAscii().data());
        return goto_page (page_name.toAscii().data(),false);
    }
    else
    {
        LOG_PRINT(warning_e, "History empty\n");
        go_home();
        return false;
    }
}

/* this section is used for hardwares with phisical key */
#ifdef HW_KEY_ENABLED
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* for some reason chis event is assocated when the key is pressed!!! */
void page::keyReleaseEvent(QKeyEvent *event)
{
    fprintf(stderr, "A KEY IS PRESSED: 1 - %d\n", event->key());
    //LOG_PRINT(verbose_e, "A KEY IS PRESSED: 1 - %d\n", event->key());
#if 0
    int i;
    switch (event->key() - 0x1000000 + 11)
    {
    case F01:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 1 - %d\n", event->key());
        i = 0;
        break;
    case F02:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 2 - %d\n", event->key());
        i = 1;
        break;
    case F03:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 3 - %d\n", event->key());
        i = 2;
        break;
    case F04:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 4 - %d\n", event->key());
        i = 3;
        break;
    case F05:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 5 - %d\n", event->key());
        i = 4;
        break;
    case F06:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 6 - %d\n", event->key());
        i = 5;
        break;
    case F07:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 7 - %d\n", event->key());
        i = 6;
        break;
    case F08:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 8 - %d\n", event->key());
        i = 7;
        break;
    case F09:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 9 - %d\n", event->key());
        i = 8;
        break;
    case F10:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 0 - %d\n", event->key());
        i = 9;
        break;
    default:
        //LOG_PRINT(verbose_e,"UNKNOWN KEY IS PRESSED: 0 - %d\n", event->key() - 0x1000000);
        return;
        break;
    }
    KeyPress(keymap[i]);
#endif
}

/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* for some reason chis event is assocated when the key is released!!! */
void page::keyPressEvent(QKeyEvent *event)
{
    fprintf(stderr, "A KEY IS RELEASED: 1 - %d\n", event->key());
    LOG_PRINT(verbose_e, "A KEY IS RELEASED: 1 - %d\n", event->key());
#if 0
    int i;
    switch (event->key() - 0x1000000 + 11)
    {
    case F01:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 1 - %d\n", event->key());
        i = 0;
        break;
    case F02:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 2 - %d\n", event->key());
        i = 1;
        break;
    case F03:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 3 - %d\n", event->key());
        i = 2;
        break;
    case F04:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 4 - %d\n", event->key());
        i = 3;
        break;
    case F05:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 5 - %d\n", event->key());
        i = 4;
        break;
    case F06:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 6 - %d\n", event->key());
        i = 5;
        break;
    case F07:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 7 - %d\n", event->key());
        i = 6;
        break;
    case F08:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 8 - %d\n", event->key());
        i = 7;
        break;
    case F09:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 9 - %d\n", event->key());
        i = 8;
        break;
    case F10:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 0 - %d\n", event->key());
        i = 9;
        break;
    default:
        //LOG_PRINT(verbose_e,"UNKNOWN KEY IS RELEASED: 0 - %d\n", event->key() - 0x1000000);
        return;
        break;
    }
    KeyRelease(keymap[i]);
#endif
}

void page::KeyRelease(const char * Id)
{
    if (Id == NULL)
    {
        return;
    }
    //LOG_PRINT(verbose_e, "RELEASE ##################################### '%s'\n", Id);
}

void page::KeyPress(const char * Id)
{
    if (Id == NULL)
    {
        return;
    }
    //LOG_PRINT(verbose_e, "PRESS ##################################### '%s'\n", Id);
}
#endif


/**
 * @brief call the activateVar function for each variable into listVarname
 */
bool page::activateVarList(const QStringList listVarname)
{
    QString varname;
    bool ret = true;
    
    foreach(varname, listVarname)
    {
        if (activateVar(varname.toAscii().data()) != 0)
        {
            LOG_PRINT(error_e, "cannot activate '%s'\n", varname.toAscii().data());
            ret = false;
        }
        else
        {
            LOG_PRINT(verbose_e, "activated '%s'\n", varname.toAscii().data());
        }
    }
    
    return ret;
}

/**
 * @brief call the deactivateVar function for each variable into listVarname
 */
bool page::deactivateVarList(const QStringList listVarname)
{
    QString varname;
    bool ret = true;
    
    foreach(varname, listVarname)
    {
        if (deactivateVar(varname.toAscii().data()) == false)
        {
            LOG_PRINT(error_e, "cannot deactivate '%s'\n", varname.toAscii().data());
            ret = false;
        }
        else
        {
            LOG_PRINT(verbose_e, "deactivated '%s'\n", varname.toAscii().data());
        }
    }
    
    return ret;
}


/**
 * @brief get the actual status.
 * The status could be:
 * - ERROR:  error flag is set to 1
 * - DONE:   error flag is set to 0
 * - BUSY:   error flag is set to 0
 */
char page::getStatusVar(const char * varname, char * msg)
{
    char Status = DONE;
    char StatusComm = (ioComm == NULL) ? ERROR : ioComm->getStatusIO();
    int CtIndex = - 1;
    
    /* force a status vector update before read the status */
    checkWriting();
    
    if (StatusComm == ERROR)
    {
        Status = ERROR;
        LOG_PRINT(verbose_e, "ioLayer: '%c' [ERROR]\n", StatusComm);
        if (msg != NULL)
        {
            strcpy(msg, VAR_COMMUNICATION);
        }
        return Status;
    }
    else if (StatusComm == BUSY)
    {
        Status = BUSY;
        LOG_PRINT(verbose_e, "ioLayer: '%c' [BUSY]\n", StatusComm);
        if (msg != NULL)
        {
            strcpy(msg, VAR_PROGRESS);
        }
    }
    
    if (Tag2CtIndex(varname, &CtIndex) != 0)
    {
        LOG_PRINT(error_e, "CtIndex '%d' Status %d ERROR\n", CtIndex, Status);
        if (msg != NULL)
        {
            strcpy(msg, VAR_UNKNOWN);
        }
        return ERROR;
    }
    
    Status = getStatusVarByCtIndex(CtIndex, msg);
    return Status;
}

/**
 * @brief set the actual status.
 * The status could be:
 * - ERROR:  error flag is set to 1
 * - DONE:   error flag is set to 0
 * - BUSY:   error flag is set to 0
 */
bool page::setStatusVar(int SynIndex, char Status)
{
    int CtIndex;
    
    /* get the variable address from syncrovector */
    if (SynIndex2CtIndex(SynIndex, &CtIndex) == 0)
    {
        if (Status == BUSY)
        {
            pIODataStatusAreaO[CtIndex] = Status;
        }
        else
        {
            pIODataStatusAreaO[CtIndex] = Status;
        }
        LOG_PRINT(verbose_e, "Status '%d' is '%c'\n", CtIndex, Status);
        return true;
    }
    else
    {
        LOG_PRINT(error_e, "cannot set the status '%c' the variable '%d'\n", Status, CtIndex);
        return false;
    }
}

/**
 * @brief set the actual page as start page
 */
bool page::setAsStartPage(char * window)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    strcpy(StartPage, window);
    settings.setValue(STARTPAGE_TAG, StartPage);
    settings.sync();
    return true;
}
#ifdef ENABLE_ALARMS
void page::setAlarmsBuzzer(int period_ms)
{
    static int index = 0;
    static int counter = 0;
    
    if (BuzzerAlarm == false || HornACK == true)
    {
        LOG_PRINT(verbose_e, "BuzzerAlarm '%d' HornACK '%d'\n", BuzzerAlarm, HornACK);
        return;
    }
    
    if (_active_alarms_events_.count() == 0)
    {
        LOG_PRINT(verbose_e, "NO ALARM '%d'\n", _active_alarms_events_.count());
        return;
    }
    
    /* do the alarm check only any period_ms */
    if (counter * REFRESH_MS < period_ms)
    {
        counter++;
        return;
    }
    counter = 0;
    
    for (; index < _active_alarms_events_.count() && _active_alarms_events_.at(index)->isack == true; index++);
    
#ifdef BUZZER
    if (index < _active_alarms_events_.count() && _active_alarms_events_.at(index)->isack == false)
    {
        LOG_PRINT(verbose_e, "BEEP FOR '%d'\n", period_ms/2);
        if (_active_alarms_events_.at(index)->type == ALARM)
        {
            beep(period_ms/2);
        }
    }
    else
    {
        LOG_PRINT(verbose_e, "NO ACTIVE ALARM FOUND\n");
        index = 0;
    }
#endif
}

void page::sequentialShowError(QLineEdit * line, int period_ms)
{
    static int index = 0;
    static int counter = 0;
    
    if (_active_alarms_events_.count() == 0)
    {
        return;
    }
    
    if (counter * REFRESH_MS < period_ms)
    {
        counter++;
        return;
    }
    counter = 0;
    
    for (; index < _active_alarms_events_.count(); index++)
    {
        /* skip the acknowloged error */
        if (ISBANNER(_active_alarms_events_.at(index)->styleindex) == 0)
        {
            continue;
        }
    }
    
    if (index >= _active_alarms_events_.count())
    {
        LOG_PRINT(verbose_e, "rewind\n");
        index = 0;
    }
    if (_active_alarms_events_.at(index)->isack)
    {
        line->clear();
        LOG_PRINT(verbose_e, "nothing to show\n");
    }
    else
    {
        line->setText((EventHash.find(_active_alarms_events_.at(index)->tag).value())->description);
        LOG_PRINT(verbose_e, "show '%s'\n", line->text().toAscii().data());
    }
}

void page::rotateShowErrorSlot()
{
    if (rotateShowError(_line, _period_ms))
    {
    }
}

bool page::rotateShowError(QLineEdit * line, int period_ms)
{
    static int shift = 0;
    static int counter = 0;
    QString bannerStr;
    static QString bannerStrOld;
    static page * p = NULL;
    static QFontMetrics fm(line->font());
    
    if (p == NULL)
    {
        p = this;
        if (logger && logger->connectToPage(p))
        {
            LOG_PRINT(verbose_e, "'%s' connected to logger\n", this->windowTitle().toAscii().data());
            
        }
        else
        {
            LOG_PRINT(error_e, "cannot connect '%s' to logger\n", this->windowTitle().toAscii().data());
        }
    }
    _line = line;
    _period_ms = period_ms;
    
    counter = 0;
    bannerStr.clear();
    for (int index = 0; index < _active_alarms_events_.count(); index++)
    {
        /* skip the invisible error */
        if (ISBANNER(_active_alarms_events_.at(index)->styleindex) == 0)
        {
            continue;
        }
        if (bannerStr.length() > 0)
        {
            bannerStr.append( ERROR_SEPARATOR );
        }
        bannerStr.append((EventHash.find(_active_alarms_events_.at(index)->tag).value())->description);
    }
    
    if (bannerStrOld.compare(bannerStr) != 0)
    {
        LOG_PRINT(verbose_e, "new error\n");
        bannerStrOld = bannerStr;
        shift = 0;
    }
    else if (bannerStr.length() != 0)
    {
        LOG_PRINT(verbose_e, "rewind %d >= %d\n", line->width(), fm.width(bannerStr.right(bannerStr.length() - shift)));
        if (line->width() < fm.width(bannerStr.right(bannerStr.length() - shift)))
        {
            LOG_PRINT(verbose_e, "rewind %d >= %d\n", line->width(), fm.width(bannerStr.right(bannerStr.length() - shift)));
            shift++;
        }
        else
        {
            shift = 0;
        }
    }
    if (bannerStr.length() == 0)
    {
        line->clear();
        LOG_PRINT(verbose_e, "nothing to show\n");
    }
    else
    {
        line->setText(bannerStr.right(bannerStr.length() - shift));
        line->setCursorPosition(0);
        LOG_PRINT(verbose_e, "show '%s'\n", line->text().toAscii().data());
    }
    QTimer::singleShot(_period_ms, this, SLOT(rotateShowErrorSlot()));
    return true;
}

char * page::getDescription(char* tag)
{
    if (tag != NULL)
    {
        QHash<QString, event_t *>::const_iterator i = EventHash.find(tag);
        
        if (i != EventHash.end())
        {
            return EventHash.find(tag).value()->description;
        }
    }
    return NULL;
}
#endif

int page::countLine(const char * filename)
{
    FILE * fp = NULL;
    int i = 0;
    char line[LINE_SIZE];
    
    fp = fopen (filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open file '%s'\n", filename);
        return false;
    }
    for (i = 0; fgets(line, LINE_SIZE, fp) != NULL; i++);
    fclose (fp);
    return i;
}

bool page::isBlockFullUsed(int block, QStringList variablelist)
{
    int i;
    for (i = 0; i < DB_SIZE_ELEM && varNameArray[i].block != block; i++);
    for (; i < DB_SIZE_ELEM && varNameArray[i].block == block; i++)
    {
        if (variablelist.indexOf(varNameArray[i].tag) == -1)
        {
            return false;
        }
    }
    return true;
}

QStringList page::getBlocksToFill(QStringList variablelist)
{
    char varblockhead[TAG_LEN] = "";
    QStringList list;
    int i;
    for (i = 0; i < variablelist.count(); i++)
    {
        int block = getHeadBlockName(variablelist.at(i).toAscii().data(), varblockhead);
        if (block >= 0)
        {
            if (isBlockFullUsed(block, variablelist) == false)
            {
                list << varblockhead;
            }
        }
    }
    return list;
}


void page::translateFontSize( QWidget *ui )
{
    QObjectList l;
    QObject *w;
    
    if (!ui)
        return;
    
    QFont font = QFont(ui->font());
    font.setPointSize( (font.pointSize() *5) / 3);
    ui->setFont(font);
    
    l = ui->children();
    
    foreach (w, l) {
        QWidget *ww = dynamic_cast<QWidget *>(w);
        if (ww) {
            QFont font = QFont(ww->font());
            font.setPointSize( (font.pointSize() *5) / 3);
            ww->setFont(font);
        }
    }
}

int page::getPageNb()
{
    return atoh(this->windowTitle().right(strlen(PAGE_PREFIX)).toAscii().data());
}

bool page::zipAndSave(QStringList sourcefiles, QString destfile, bool junkdir, QString basedir)
{
    if (destfile.length() == 0)
    {
        LOG_PRINT(error_e, "no zip file found \n");
        return false;
    }
    if (!QFile::exists(ZIP_BIN))
    {
        LOG_PRINT(error_e, "no zip command found\n");
        return false;
    }
    for (int i = 0; i < sourcefiles.count(); i++)
    {
        if (!QFile::exists(sourcefiles.at(i)) && sourcefiles.at(i).compare("*") != 0)
        {
            LOG_PRINT(error_e, "cannot find the file to zip '%s'\n", sourcefiles.at(i).toAscii().data());
            return false;
        }
    }
#if 0 /* this code have some problem to catch the finished() signal*/
    QProcess process;
    if (junkdir)
    {
        process.start(ZIP_BIN, QStringList() << "-r" << "-j" << destfile << sourcefiles);
    }
    else
    {
        process.start(ZIP_BIN, QStringList() << "-r"  << destfile << sourcefiles);
    }
    process.terminate();
    if (!process.waitForStarted())
    {
        LOG_PRINT(error_e, "cannot start command: '%s'\n", process.errorString().toAscii().data());
        process.close();
        return false;
    }

    if (!process.waitForFinished())
    {
        LOG_PRINT(error_e, "cannot execute command: '%s'\n", process.errorString().toAscii().data());
        process.close();
        return false;
    }
    process.close();
#else
    char command[1024];
    if (basedir.length())
    {
        if (junkdir)
        {
            sprintf(command, "cd %s && %s -r -j %s %s >/dev/null 2>&1", basedir.toAscii().data(), ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
        else
        {
            sprintf(command, "cd %s && %s -r %s %s >/dev/null 2>&1", basedir.toAscii().data(), ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
    }
    else
    {
        if (junkdir)
        {
            sprintf(command, "%s -r -j %s %s >/dev/null 2>&1", ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
        else
        {
            sprintf(command, "%s -r %s %s >/dev/null 2>&1", ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
    }

    if (system(command))
    {
        LOG_PRINT(error_e, "cannot execute command: '%s'\n", command);
        return false;
    }
#endif
    return true;
}

bool page::signFile(QString srcfile, QString destfile)
{
    char line[STR_LEN];
    /* create the signature file */
    /* Open the command for reading. */
    sprintf(line, "%s %s", APP_SIGN, srcfile.toAscii().data());
    FILE * fp = popen(line, "r");
    if (fp == NULL) {
        LOG_PRINT(error_e,"Failed to run command '%s'\n", line );
        return false;
    }

    char sign[LINE_SIZE];

    /* Read the output a line at a time - output it. */
    if (fscanf(fp, "%s", sign) > 0) {
        LOG_PRINT(verbose_e,"SIGN: '%s'\n", sign);
    }

    /* close */
    pclose(fp);

    if (sign[0] == '\0')
    {
        LOG_PRINT(error_e,"Failed read sign\n");
        return false;
    }

    FILE * fpout = fopen(destfile.toAscii().data(), "w");
    if (fpout == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", destfile.toAscii().data());
        return false;
    }
    fprintf(fpout, "%s\n", sign);
    fclose(fpout);
    return true;
}

bool page::setTag(QString * label, QString value)
{
    QHash<QString, QString>::const_iterator i;
    LOG_PRINT(verbose_e,"LOOKING FOR %s\n", value.toAscii().data());
    if (value.length() == 0)
    {
        return false;
    }
    else
    {
        i = TagTable.find(value);
    }
    if (i != TagTable.end())
    {
        *label = i.value();
        LOG_PRINT(verbose_e,"FOUND %s\n", i.value().toAscii().data());
        return true;
    }
    return false;
}

int page::checkSDusage()
{
    if (SDCheck() == 0)
    {
        char filename[256];
        sprintf(filename, "%s/%s", SD_MOUNT_POINT, ".application");
        if (QFile::exists(filename))
        {
            return 1;
        }
        sprintf(filename, "%s/%s", SD_MOUNT_POINT, ".extrastorage");
        if (QFile::exists(filename))
        {
            return 2;
        }
        return 0;
    }
    return -1;
}

/*
   -1 error
   0 no sdcard
   1 sdcard without licence
   2 sdcard with storage licence
   3 sdcard with system licence
 */
int page::checkLicence(QString * message)
{
    if (SDCheck() == 0)
    {
        char cid[CID_LEN + 1];
        message->clear();
        if (getSdCID(cid) != 0)
        {
            LOG_PRINT(error_e, "cannot open CID file '%s'\n", CID_FILE );
            *message = "Invalid SD card detected.";
            return -1;
        }
        
        char command[1024];
        
        /* read licence */
        char licence[CID_LEN + 1];
        /* Open the command for reading. */
        sprintf(command, "%s/%s", SD_MOUNT_POINT, LICENCE_FILE);
        
        FILE * fp = fopen(command, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Failed to run open '%s'\n", command );
            *message = "SD card detected, no licence found.";
            return 1;
        }
        
        /* Read the output a line at a time - output it. */
        if (fscanf(fp, "%s", licence) == 1)
        {
            LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        }
        else
        {
            fclose(fp);
            LOG_PRINT(error_e, "Failed to run command '%s': '%s'\n", command, licence );
            *message = "SD card detected, invalid licence found.";
            return 1;
        }
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* close */
        fclose(fp);
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* compose licence */
        char signedcode[LINE_SIZE];
        /* Open the command for reading. */
        sprintf(command, "echo %s%s | %s", cid, STORAGE_PERMISSION, APP_SIGN);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Failed to run command '%s'\n", command );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* Read the output a line at a time - output it. */
        if (fgets(signedcode, LINE_SIZE, fp) != NULL)
        {
            sscanf(signedcode, "%s", signedcode);
            LOG_PRINT(verbose_e, "signedcode: '%s'\n", signedcode);
        }
        else
        {
            pclose(fp);
            LOG_PRINT(error_e, "Failed to run command '%s': '%s'\n", command, signedcode );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* close */
        pclose(fp);
        
        if (strcmp(licence, signedcode) == 0)
        {
            *message = "SD card detected, storage licence found.";
            LOG_PRINT(verbose_e, "SD card detected, storage licence found.\n");
            return 2;
        }
        
        /* Open the command for reading. */
        sprintf(command, "echo %s%s | %s", cid, APPLICATION_PERMISSION, APP_SIGN);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Failed to run command '%s'\n", command );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        /* Read the output a line at a time - output it. */
        if (fgets(signedcode, LINE_SIZE, fp) != NULL)
        {
            sscanf(signedcode, "%s", signedcode);
            LOG_PRINT(verbose_e, "signedcode: '%s'\n", signedcode);
        }
        else
        {
            pclose(fp);
            LOG_PRINT(error_e, "Failed to run command '%s': '%s'\n", command, signedcode );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        /* close */
        pclose(fp);
        
        if (strcmp(licence, signedcode) == 0)
        {
            *message = "SD card detected, application licence found.";
            LOG_PRINT(verbose_e, "SD card detected, application licence found.\n");
            return 3;
        }
        else
        {
            LOG_PRINT(verbose_e, "'%s'\n'%s'\n", licence, signedcode);
            *message = "SD card detected, No valid licence found.";
            return 1;
        }
        
    }
    else
    {
        *message = "No SD card detected.";
        return 0;
    }
}

/**
 * @brief create a new page
 */
bool page::create_next_page(page ** p, const char * t)
{
    *p = NULL;
    /* special pages */
    if (strncmp(t, PAGE_PREFIX, strlen(PAGE_PREFIX)) != 0)
    {
        if (strcmp(t, "system_ini") == 0)
        {
            *p = (page *)(new system_ini);
        }
        else if (strcmp(t, "net_conf") == 0)
        {
            *p = (page *)(new net_conf);
        }
        else if (strcmp(t, "info") == 0)
        {
            *p = (page *)(new info);
        }
        else if (strcmp(t, "data_manager") == 0)
        {
            *p = (page *)(new data_manager);
        }
        else if (strcmp(t, "display_settings") == 0)
        {
            *p = (page *)(new display_settings);
        }
        else if (strcmp(t, "display_test") == 0)
        {
            *p = (page *)(new display_test);
        }
        else if (strcmp(t, "menu") == 0)
        {
            *p = (page *)(new menu);
        }
        else if (strcmp(t, "options") == 0)
        {
            *p = (page *)(new options);
        }
        else if (strcmp(t, "time_set") == 0)
        {
            *p = (page *)(new time_set);
        }
#ifdef ENABLE_ALARMS
        else if (strcmp(t, "alarms") == 0)
        {
            *p = (page *)(new alarms);
        }
        else if (strcmp(t, "alarms_history") == 0)
        {
            *p = (page *)(new alarms_history);
        }
#endif
#ifdef ENABLE_STORE
        else if (strcmp(t, "store") == 0)
        {
            *p = (page *)(new store);
        }
        else if (strcmp(t, "store_filter") == 0)
        {
            *p = (page *)(new store_filter);
        }
#endif
#ifdef ENABLE_TREND
        else if (strcmp(t, "trend") == 0)
        {
            *p = (page *)(new trend);
        }
        else if (strcmp(t, "trend_other") == 0)
        {
            *p = (page *)(new trend_other);
        }
        else if (strcmp(t, "trend_option") == 0)
        {
            *p = (page *)(new trend_option);
        }
        else if (strcmp(t, "trend_range") == 0)
        {
            *p = (page *)(new trend_range);
        }
#endif
#ifdef ENABLE_RECIPE
        else if (strcmp(t, "recipe") == 0)
        {
            *p = (page *)(new recipe);
        }
        else if (strcmp(t, "recipe_select") == 0)
        {
            *p = (page *)(new recipe_select);
        }
#endif
        else
        {
            LOG_PRINT(error_e, "cannot create special page %s\n", t);
            return false;
        }
    }
    /* it is a ordinary pages */
    else
    {
        int pageNb = atoh(&(t[strlen(PAGE_PREFIX)]));

        if (create_page_nb(p, pageNb)!= 0)
        {
            LOG_PRINT(error_e, "cannot create ordinary page %d (%s)\n", pageNb, t);
            return false;
        }
    }
    return true;
}
