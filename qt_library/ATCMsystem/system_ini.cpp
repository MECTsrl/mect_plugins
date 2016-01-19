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

#define SYSTEM_INI "/local/etc/sysconfig/system.ini"

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
    ui->pushButton_HomePage->setText(settings.value("SYSTEM/home_page").toString());
    ui->pushButton_StartPage->setText(settings.value("SYSTEM/start_page").toString());
    ui->checkBox_BuzzerTouch->setChecked(settings.value("SYSTEM/buzzer_touch").toBool());
    ui->checkBox_BuzzerAlarm->setChecked(settings.value("SYSTEM/buzzer_alarm").toBool());
    ui->atcmLabel_PwdTimeout->setText(settings.value("SYSTEM/pwd_timeout_s").toString());
    ui->pushButton_PwdLogoutPage->setText(settings.value("SYSTEM/pwd_logout_page").toString());
    ui->atcmLabel_ScreenSaver->setText(settings.value("SYSTEM/screen_saver_s").toString());
    ui->atcmLabel_SlowLogPeriod->setText(settings.value("SYSTEM/slow_log_period_s").toString());
    ui->atcmLabel_FastLogPeriod->setText(settings.value("SYSTEM/fast_log_period_s").toString());
    ui->atcmLabel_MaxLogSpace->setText(settings.value("SYSTEM/max_log_space_MB").toString());
    ui->atcmLabel_TraceWindow->setText(settings.value("SYSTEM/trace_window_s").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/stopbits").toString()));
    ui->atcmLabel_Silence_SERIAL_PORT_0->setText(settings.value("SERIAL_PORT_0/silence_ms").toString());
    ui->atcmLabel_Timeout_SERIAL_PORT_0->setText(settings.value("SERIAL_PORT_0/timeout_ms").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/stopbits").toString()));
    ui->atcmLabel_Silence_SERIAL_PORT_1->setText(settings.value("SERIAL_PORT_1/silence_ms").toString());
    ui->atcmLabel_Timeout_SERIAL_PORT_1->setText(settings.value("SERIAL_PORT_1/timeout_ms").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/stopbits").toString()));
    ui->atcmLabel_Silence_SERIAL_PORT_2->setText(settings.value("SERIAL_PORT_2/silence_ms").toString());
    ui->atcmLabel_Timeout_SERIAL_PORT_2->setText(settings.value("SERIAL_PORT_2/timeout_ms").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/stopbits").toString()));
    ui->atcmLabel_Silence_SERIAL_PORT_3->setText(settings.value("SERIAL_PORT_3/silence_ms").toString());
    ui->atcmLabel_Timeout_SERIAL_PORT_3->setText(settings.value("SERIAL_PORT_3/timeout_ms").toString());
    ui->atcmLabel_Silence_TCP_IP_PORT->setText(settings.value("TCP_IP_PORT/silence_ms").toString());
    ui->atcmLabel_Timeout_TCP_IP_PORT->setText(settings.value("TCP_IP_PORT/timeout_ms").toString());
    ui->comboBox_Baudrate_CANOPEN_0->setCurrentIndex(ui->comboBox_Baudrate_CANOPEN_0->findText(settings.value("CANOPEN_0/baudrate").toString()));
    ui->comboBox_Baudrate_CANOPEN_1->setCurrentIndex(ui->comboBox_Baudrate_CANOPEN_1->findText(settings.value("CANOPEN_1/baudrate").toString()));
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

//funzione salvataggio del file
void system_ini::save_all()
{
    bool OK;

    if (ui->atcmLabel_Retries->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Retries' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Retries' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Blacklist->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Blacklist' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Blacklist' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_ReadPeriod1->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 1' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 1' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_ReadPeriod2->text().toInt(&OK) < 2 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 2' parameter must be greater than 1."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 2' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_ReadPeriod3->text().toInt(&OK) < 3 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 3' parameter must be greater than 2."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 3' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_ReadPeriod3->text().toInt() < ui->atcmLabel_ReadPeriod2->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 3' parameter must be greater than 'Read Period 2' parameter."));
        return;
    }

    if (ui->atcmLabel_ReadPeriod2->text().toInt() < ui->atcmLabel_ReadPeriod1->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("'Read Period 2' parameter must be greater than 'Read Period 1' parameter."));
        return;
    }

    /* if (ui->pushButton_HomePage->text().length() == 0)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Error"), tr("'Home Page' parameter can not be empty. Do you want to automatically set the 'Home Page' parameter equal to 'Start Page' parameter?"));
        QMessageBox::Yes|QMessageBox::No;
        if (reply == QMessageBox::Yes) {
            ui->pushButton_HomePage->setText(ui->pushButton_StartPage->text());
        } else {
            return;
        }
    }*/
    if (ui->atcmLabel_PwdTimeout->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Pwd Timeout' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Pwd Timeout' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_ScreenSaver->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Screen Saver' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Screen Saver' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_SlowLogPeriod->text().toInt(&OK) < 2 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Slow Log Period' parameter must be greater than 1."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Slow Log Period' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_FastLogPeriod->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Fast Log Period' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Fast Log Period' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_SlowLogPeriod->text().toInt() < ui->atcmLabel_FastLogPeriod->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("'Slow Log Period' parameter must be greater than 'Fast Log Period' parameter."));
        return;
    }
    if (ui->atcmLabel_MaxLogSpace->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Max Log Space' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Max Log Space' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_TraceWindow->text().toInt(&OK) < 3 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("'Trace Window' parameter must be greater than 2."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("'Trace Window' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_TraceWindow->text().toInt() < (ui->atcmLabel_FastLogPeriod->text().toInt()* 3))
    {
        QMessageBox::critical(0,tr("Error"),tr("'Trace Window' parameter must be at least three times 'Fast Log Period' parameter."));
        return;
    }
    if (ui->atcmLabel_Silence_SERIAL_PORT_0->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 0' tab, the 'Silence' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 0' tab, the 'Silence' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_0->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_0->text().toInt() <= ui->atcmLabel_Silence_SERIAL_PORT_0->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
        return;
    }
    if (ui->atcmLabel_Silence_SERIAL_PORT_1->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_1->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_1->text().toInt() <= ui->atcmLabel_Silence_SERIAL_PORT_1->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
        return;
    }
    if (ui->atcmLabel_Silence_SERIAL_PORT_2->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_2->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_2->text().toInt() <= ui->atcmLabel_Silence_SERIAL_PORT_2->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
        return;
    }
    if (ui->atcmLabel_Silence_SERIAL_PORT_3->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 3' tab, the 'Silence' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 3' tab, the 'Silence' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_3->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_SERIAL_PORT_3->text().toInt() <= ui->atcmLabel_Silence_SERIAL_PORT_3->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
        return;
    }
    if (ui->atcmLabel_Silence_TCP_IP_PORT->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be greater than or egual 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_TCP_IP_PORT->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 0."));
        return;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be a number."));
        return;
    }

    if (ui->atcmLabel_Timeout_TCP_IP_PORT->text().toInt() <= ui->atcmLabel_Silence_TCP_IP_PORT->text().toInt())
    {
        QMessageBox::critical(0,tr("Error"),tr("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
        return;
    }

    QSettings settings(SYSTEM_INI, QSettings::IniFormat);

    /* SYSTEM */
    settings.setValue("SYSTEM/retries", ui->atcmLabel_Retries->text());
    settings.setValue("SYSTEM/blacklist", ui->atcmLabel_Blacklist->text());
    settings.setValue("SYSTEM/read_period_ms_1", ui->atcmLabel_ReadPeriod1->text());
    settings.setValue("SYSTEM/read_period_ms_2", ui->atcmLabel_ReadPeriod2->text());
    settings.setValue("SYSTEM/read_period_ms_3", ui->atcmLabel_ReadPeriod3->text());
    settings.setValue("SYSTEM/home_page", ui->pushButton_HomePage->text());
    settings.setValue("SYSTEM/start_page", ui->pushButton_StartPage->text());
    settings.setValue("SYSTEM/buzzer_touch", ui->checkBox_BuzzerTouch->isChecked());
    settings.setValue("SYSTEM/buzzer_alarm",  ui->checkBox_BuzzerAlarm->isChecked());
    settings.setValue("SYSTEM/pwd_timeout_s", ui->atcmLabel_PwdTimeout->text());
    settings.setValue("SYSTEM/pwd_logout_page", ui->pushButton_PwdLogoutPage->text());
    settings.setValue("SYSTEM/screen_saver_s", ui->atcmLabel_ScreenSaver->text());
    settings.setValue("SYSTEM/slow_log_period_s", ui->atcmLabel_SlowLogPeriod->text());
    settings.setValue("SYSTEM/fast_log_period_s", ui->atcmLabel_FastLogPeriod->text());
    settings.setValue("SYSTEM/max_log_space_MB", ui->atcmLabel_MaxLogSpace->text());
    settings.setValue("SYSTEM/trace_window_s", ui->atcmLabel_TraceWindow->text());
    /* SERIAL */
    if (ui->comboBox_Baudrate_SERIAL_PORT_0->currentIndex() > 0)
    {
        settings.setValue("SERIAL_PORT_0/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_0->currentText());
        settings.setValue("SERIAL_PORT_0/databits", ui->comboBox_Databits_SERIAL_PORT_0->currentText());
        settings.setValue("SERIAL_PORT_0/parity", ui->comboBox_Parity_SERIAL_PORT_0->currentText());
        settings.setValue("SERIAL_PORT_0/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_0->currentText());
        settings.setValue("SERIAL_PORT_0/silence_ms", ui->atcmLabel_Silence_SERIAL_PORT_0->text());
        settings.setValue("SERIAL_PORT_0/timeout_ms", ui->atcmLabel_Timeout_SERIAL_PORT_0->text());
    }
    else
    {
        settings.remove("SERIAL_PORT_0");
    }
    if (ui->comboBox_Baudrate_SERIAL_PORT_1->currentIndex() > 0)
    {
        settings.setValue("SERIAL_PORT_1/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_1->currentText());
        settings.setValue("SERIAL_PORT_1/databits", ui->comboBox_Databits_SERIAL_PORT_1->currentText());
        settings.setValue("SERIAL_PORT_1/parity", ui->comboBox_Parity_SERIAL_PORT_1->currentText());
        settings.setValue("SERIAL_PORT_1/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_1->currentText());
        settings.setValue("SERIAL_PORT_1/silence_ms", ui->atcmLabel_Silence_SERIAL_PORT_1->text());
        settings.setValue("SERIAL_PORT_1/timeout_ms", ui->atcmLabel_Timeout_SERIAL_PORT_1->text());
    }
    else
    {
        settings.remove("SERIAL_PORT_1");
    }
    if (ui->comboBox_Baudrate_SERIAL_PORT_2->currentIndex() > 0)
    {
        settings.setValue("SERIAL_PORT_2/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_2->currentText());
        settings.setValue("SERIAL_PORT_2/databits", ui->comboBox_Databits_SERIAL_PORT_2->currentText());
        settings.setValue("SERIAL_PORT_2/parity", ui->comboBox_Parity_SERIAL_PORT_2->currentText());
        settings.setValue("SERIAL_PORT_2/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_2->currentText());
        settings.setValue("SERIAL_PORT_2/silence_ms", ui->atcmLabel_Silence_SERIAL_PORT_2->text());
        settings.setValue("SERIAL_PORT_2/timeout_ms", ui->atcmLabel_Timeout_SERIAL_PORT_2->text());
    }
    else
    {
        settings.remove("SERIAL_PORT_2");
    }
    if (ui->comboBox_Baudrate_SERIAL_PORT_3->currentIndex() > 0)
    {
        settings.setValue("SERIAL_PORT_3/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_3->currentText());
        settings.setValue("SERIAL_PORT_3/databits", ui->comboBox_Databits_SERIAL_PORT_3->currentText());
        settings.setValue("SERIAL_PORT_3/parity", ui->comboBox_Parity_SERIAL_PORT_3->currentText());
        settings.setValue("SERIAL_PORT_3/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_3->currentText());
        settings.setValue("SERIAL_PORT_3/silence_ms", ui->atcmLabel_Silence_SERIAL_PORT_3->text());
        settings.setValue("SERIAL_PORT_3/timeout_ms", ui->atcmLabel_Timeout_SERIAL_PORT_3->text());
    }
    else
    {
        settings.remove("SERIAL_PORT_3");
    }
    /* TCP_IP */
    settings.setValue("TCP_IP_PORT/silence_ms", ui->atcmLabel_Silence_TCP_IP_PORT->text());
    settings.setValue("TCP_IP_PORT/timeout_ms", ui->atcmLabel_Timeout_TCP_IP_PORT->text());
    /* CANOPEN */
    settings.setValue("CANOPEN_0/baudrate", ui->comboBox_Baudrate_CANOPEN_0->currentText());
    settings.setValue("CANOPEN_1/baudrate", ui->comboBox_Baudrate_CANOPEN_1->currentText());

    readIniFile();

    QMessageBox::information(0,tr("Information"),tr("Configuration has been successfully saved."));
}

void system_ini::on_pushButton_PwdLogoutPage_clicked()
{
    char value_pwd [32];
    alphanumpad tatiera_alfanum(value_pwd, ui->pushButton_PwdLogoutPage->text().toAscii().data());//alphanumpad=>tipo oggetto, value => destinazione e valore di default. toAscii().data()=>trasformazione da stringa a Char* xè la dialog box alphanumpad non vuole stringhe
    tatiera_alfanum.showFullScreen();//schermata intera per la dialog box
    if(tatiera_alfanum.exec()==QDialog::Accepted)//exec=>eseguo alphanumpad, se quando termina è uguale ad Accepted, ossia è stato premuto OK => devo considerare il nuovo valore da inserire nel bottone
    {
        ui->pushButton_PwdLogoutPage->setText(value_pwd);
    }
}

void system_ini::on_pushButton_StartPage_clicked()
{
    char value_start [32];
    alphanumpad tatiera_alfanum(value_start, ui->pushButton_StartPage->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted)
    {
        ui->pushButton_StartPage->setText(value_start);
    }
}

void system_ini::on_pushButton_HomePage_clicked()
{
    char value_home [32];
    alphanumpad tatiera_alfanum(value_home, ui->pushButton_HomePage->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted)
    {
        ui->pushButton_HomePage->setText(value_home);
    }
}

void system_ini::on_pushButtonHome_clicked()
{
    go_home();
}

void system_ini::on_pushButtonBack_clicked()
{
    go_back();
}

void system_ini::on_pushButtonSave_clicked()
{
    save_all();
}
