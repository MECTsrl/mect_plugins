/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Generic page
 */
#include "app_logprint.h"
#include "atcmplugin.h"
#include "main.h"
#include "system_ini.h"
#include "ui_system_ini.h"
#include <QSettings>

#ifdef TARGET
#define SYSTEM_INI "/local/etc/sysconfig/system.ini"
#else
#define SYSTEM_INI "config/system.ini"
#endif

/**
 * @brief this macro is used to set the system_ini style.
 * the syntax is html stylesheet-like
 */
#define SET_system_ini_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    /* add there the page stilesheet customization */ \
    mystyle.append(""); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instantiation of the page.
 */
system_ini::system_ini(QWidget *parent) :
    page(parent),
    ui(new Ui::system_ini)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    
    /* set up the page style */
    //SET_PAGE_STYLE();
    /* set the style described into the macro SET_system_ini_STYLE */
    SET_system_ini_STYLE();
    translateFontSize(this);
}

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void system_ini::reload()
{
    QSettings settings(SYSTEM_INI, QSettings::IniFormat);

    ui->atcmLabel_Retries->setText(settings.value("SYSTEM/retries").toString());
    ui->atcmLabel_Blacklist->setText(settings.value("SYSTEM/blacklist").toString());
    ui->atcmLabel_ReadPeriod1->setText(settings.value("SYSTEM/read_period_ms_1").toString());
    ui->atcmLabel_ReadPeriod2->setText(settings.value("SYSTEM/read_period_ms_2").toString());
    ui->atcmLabel_ReadPeriod3->setText(settings.value("SYSTEM/read_period_ms_3").toString());
    //ui->comboBox_HomePage = settings.value("SYSTEM/home_page").toString();
    //ui->comboBox_StartPage = settings.value("SYSTEM/start_page").toString();
    ui->checkBox_BuzzerTouch->setChecked(settings.value("SYSTEM/buzzer_touch").toBool());
    ui->checkBox_BuzzerAlarm->setChecked(settings.value("SYSTEM/buzzer_alarm").toBool());
    ui->atcmLabel_PwdTimeout->setText(settings.value("SYSTEM/pwd_timeout_s").toString());
    //ui->comboBox_PwdLogoutPage = settings.value("SYSTEM/pwd_logout_page").toString();
    ui->atcmLabel_ScreenSaver->setText(settings.value("SYSTEM/screen_saver_s").toString());
    ui->atcmLabel_SlowLogPeriod->setText(settings.value("SYSTEM/slow_log_period_s").toString());
    ui->atcmLabel_FastLogPeriod->setText(settings.value("SYSTEM/fast_log_period_s").toString());
    ui->atcmLabel_MaxLogSpace->setText(settings.value("SYSTEM/max_log_space_MB").toString());
    ui->atcmLabel_TraceWindow->setText(settings.value("SYSTEM/trace_window_s").toString());


    /*
       H variables initalizations:
         variableList.clear();
         variableList << "VARIABLE1" << "VARIABLE2" << "VARIABLE3"
       trend initialization:
         strncpy(_actual_trend_, "trend1.csv",FILENAME_MAX);
         _trend_data_reload_ = true;
       store initialization:
         strncpy(_actual_store_, "store1.csv",FILENAME_MAX);
       alarm banner initialization in QLineEdit:
         rotateShowError(ui->myLineEdit, ERROR_LABEL_PERIOD_MS);
     */
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void system_ini::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }
    /* call the parent updateData member */
    page::updateData();
    /* This code show how to connect
     * the cross table variable named "RET_REG_1" to the HMI label "label1val"
     * and if it is not NULL, set the Status led "led1" without using ATCM plugins
     *    WARNING: only MIRROR variable can be read or the variable listed
     *            into the active variable list variableList defined at the top of this file
     
     if (getFormattedVar("RET_REG_1", ui->label1val, ui->led1) == false)
     {
        LOG_PRINT(error_e, "cannot read variable '%s'","RET_REG_1" );
     }
     */
    /* This code show how to write a cross table variable named "NRE_REG_1"  without using ATCM plugins.
     * Usually this code is putted into a button slot.
     * The variable could be not present into the active variable list "variableList"
     if (writeVar("NRE_REG_1", &value) == true)
     {
         LOG_PRINT(info_e,"################### NRE_REG_1 %d\n", value);
     }
     */
}

/**
 * @brief This is the event slot to detect new language translation.
 */
void system_ini::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
system_ini::~system_ini()
{
    delete ui;
}



void system_ini::on_pushButton_Save_clicked()
{
    save_all();
}


//funzione salvataggio del file
void system_ini::save_all()
{
    QSettings settings(SYSTEM_INI, QSettings::IniFormat);

    settings.setValue("SYSTEM/retries", ui->atcmLabel_Retries->text());
    settings.setValue("SYSTEM/blacklist", ui->atcmLabel_Blacklist->text());
    settings.setValue("SYSTEM/read_period_ms_1", ui->atcmLabel_ReadPeriod1->text());
    settings.setValue("SYSTEM/read_period_ms_2", ui->atcmLabel_ReadPeriod2->text());
    settings.setValue("SYSTEM/read_period_ms_3", ui->atcmLabel_ReadPeriod3->text());
    settings.setValue("SYSTEM/home_page", ui->comboBox_HomePage->currentText());
    settings.setValue("SYSTEM/start_page", ui->comboBox_StartPage->currentText());
    settings.setValue("SYSTEM/buzzer_touch", ui->checkBox_BuzzerTouch->isChecked());
    settings.setValue("SYSTEM/buzzer_alarm",  ui->checkBox_BuzzerAlarm->isChecked());
    settings.setValue("SYSTEM/pwd_timeout_s", ui->atcmLabel_PwdTimeout->text());
    settings.setValue("SYSTEM/pwd_logout_page", ui->comboBox_PwdLogoutPage->currentText());
    settings.setValue("SYSTEM/screen_saver_s", ui->atcmLabel_ScreenSaver->text());
    settings.setValue("SYSTEM/slow_log_period_s", ui->atcmLabel_SlowLogPeriod->text());
    settings.setValue("SYSTEM/fast_log_period_s", ui->atcmLabel_FastLogPeriod->text());
    settings.setValue("SYSTEM/max_log_space_MB", ui->atcmLabel_MaxLogSpace->text());
    settings.setValue("SYSTEM/trace_window_s", ui->atcmLabel_TraceWindow->text());
}
