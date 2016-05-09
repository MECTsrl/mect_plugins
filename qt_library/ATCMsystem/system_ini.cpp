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
#include "item_selector.h"
#include <QSettings>
#include <QMessageBox>
#include <QDirIterator>

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
    ui->tabWidget->setCurrentIndex(0);
}

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void system_ini::reload()
{
    QSettings settings(SYSTEM_INI, QSettings::IniFormat);


    /* load the map of the language and his locale abbreviation */
    FILE * fp = fopen(LANGUAGE_MAP_FILE, "r");
    if (fp)
    {
        char line[LINE_SIZE];
        while (fgets(line, LINE_SIZE, fp) != NULL)
        {
            QStringList strlist = QString(line).split(";");
            if (strlist.count()==2)
            {
                LanguageMap.insert(strlist.at(1).trimmed(),strlist.at(0).trimmed());
            }
        }
        fclose(fp);
    }

    /* load the translations file list */
    QDirIterator it(":/translations", QDirIterator::Subdirectories);
    QString lang = settings.value("SYSTEM/language").toString();
    int i = 0;
    int indexlang = 0;
    while (it.hasNext()) {
        QString item = it.next();
        LOG_PRINT(error_e, " %s\n", item.toAscii().data());
        if (item.endsWith (".qm") == true)
        {
            QString tmplang = item.mid(item.indexOf("languages_") + strlen("languages_"), 2);
            if (lang == tmplang)
            {
                indexlang = i;
            }
            if (LanguageMap.count() > 0 && LanguageMap.contains(item.mid(item.indexOf("languages_") + strlen("languages_"),2)))
            {
                ui->comboBoxLanguage->addItem(LanguageMap.value(item.mid(item.indexOf("languages_") + strlen("languages_"), 2)));
            }
            else
            {
                ui->comboBoxLanguage->addItem(item.mid(item.indexOf("languages_") + strlen("languages_"), 2));
            }
            i++;
        }
    }

    ui->comboBoxLanguage->setCurrentIndex(indexlang);

    ui->pushButton_Retries->setText(settings.value("SYSTEM/retries").toString());
    ui->pushButton_Blacklist->setText(settings.value("SYSTEM/blacklist").toString());
    ui->pushButton_ReadPeriod1->setText(settings.value("SYSTEM/read_period_ms_1").toString());
    ui->pushButton_ReadPeriod2->setText(settings.value("SYSTEM/read_period_ms_2").toString());
    ui->pushButton_ReadPeriod3->setText(settings.value("SYSTEM/read_period_ms_3").toString());
    ui->pushButton_HomePage->setText(settings.value("SYSTEM/home_page").toString());
    ui->pushButton_StartPage->setText(settings.value("SYSTEM/start_page").toString());
    ui->checkBox_BuzzerTouch->setChecked(settings.value("SYSTEM/buzzer_touch").toBool());
    ui->checkBox_BuzzerAlarm->setChecked(settings.value("SYSTEM/buzzer_alarm").toBool());
    ui->pushButton_PwdTimeout->setText(settings.value("SYSTEM/pwd_timeout_s").toString());
    ui->pushButton_PwdLogoutPage->setText(settings.value("SYSTEM/pwd_logout_page").toString());
    ui->pushButton_ScreenSaver->setText(settings.value("SYSTEM/screen_saver_s").toString());
    ui->pushButton_SlowLogPeriod->setText(settings.value("SYSTEM/slow_log_period_s").toString());
    ui->pushButton_FastLogPeriod->setText(settings.value("SYSTEM/fast_log_period_s").toString());
    ui->pushButton_MaxLogSpace->setText(settings.value("SYSTEM/max_log_space_MB").toString());
    ui->pushButton_TraceWindow->setText(settings.value("SYSTEM/trace_window_s").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_0->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_0->findText(settings.value("SERIAL_PORT_0/stopbits").toString()));
    ui->pushButton_Silence_SERIAL_PORT_0->setText(settings.value("SERIAL_PORT_0/silence_ms").toString());
    ui->pushButton_Timeout_SERIAL_PORT_0->setText(settings.value("SERIAL_PORT_0/timeout_ms").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_1->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_1->findText(settings.value("SERIAL_PORT_1/stopbits").toString()));
    ui->pushButton_Silence_SERIAL_PORT_1->setText(settings.value("SERIAL_PORT_1/silence_ms").toString());
    ui->pushButton_Timeout_SERIAL_PORT_1->setText(settings.value("SERIAL_PORT_1/timeout_ms").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_2->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_2->findText(settings.value("SERIAL_PORT_2/stopbits").toString()));
    ui->pushButton_Silence_SERIAL_PORT_2->setText(settings.value("SERIAL_PORT_2/silence_ms").toString());
    ui->pushButton_Timeout_SERIAL_PORT_2->setText(settings.value("SERIAL_PORT_2/timeout_ms").toString());
    ui->comboBox_Baudrate_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Baudrate_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/baudrate").toString()));
    ui->comboBox_Databits_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Databits_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/databits").toString()));
    ui->comboBox_Parity_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Parity_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/parity").toString()));
    ui->comboBox_Stopbits_SERIAL_PORT_3->setCurrentIndex(ui->comboBox_Stopbits_SERIAL_PORT_3->findText(settings.value("SERIAL_PORT_3/stopbits").toString()));
    ui->pushButton_Silence_SERIAL_PORT_3->setText(settings.value("SERIAL_PORT_3/silence_ms").toString());
    ui->pushButton_Timeout_SERIAL_PORT_3->setText(settings.value("SERIAL_PORT_3/timeout_ms").toString());
    ui->pushButton_Silence_TCP_IP_PORT->setText(settings.value("TCP_IP_PORT/silence_ms").toString());
    ui->pushButton_Timeout_TCP_IP_PORT->setText(settings.value("TCP_IP_PORT/timeout_ms").toString());
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

    QSettings settings(SYSTEM_INI, QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    /* SYSTEM */
    if (groups.indexOf("SYSTEM") >= 0)
    {
        if (ui->pushButton_Retries->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Retries' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Retries' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Blacklist->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Blacklist' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Blacklist' parameter must be a number."));
            return;
        }

        if (ui->pushButton_ReadPeriod1->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 1' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 1' parameter must be a number."));
            return;
        }

        if (ui->pushButton_ReadPeriod2->text().toInt(&OK) < 2 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be greater than 1."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be a number."));
            return;
        }

        if (ui->pushButton_ReadPeriod3->text().toInt(&OK) < 3 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be greater than 2."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be a number."));
            return;
        }

        if (ui->pushButton_ReadPeriod3->text().toInt() < ui->pushButton_ReadPeriod2->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be greater than 'Read Period 2' parameter."));
            return;
        }

        if (ui->pushButton_ReadPeriod2->text().toInt() < ui->pushButton_ReadPeriod1->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be greater than 'Read Period 1' parameter."));
            return;
        }

        /* if (ui->pushButton_HomePage->text().length() == 0)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, trUtf8("Error"), trUtf8("'Home Page' parameter can not be empty. Do you want to automatically set the 'Home Page' parameter equal to 'Start Page' parameter?"));
            QMessageBox::Yes|QMessageBox::No;
            if (reply == QMessageBox::Yes) {
                ui->pushButton_HomePage->setText(ui->pushButton_StartPage->text());
            } else {
                return;
            }
        }*/
        if (ui->pushButton_PwdTimeout->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Pwd Timeout' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Pwd Timeout' parameter must be a number."));
            return;
        }

        if (ui->pushButton_ScreenSaver->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Screen Saver' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Screen Saver' parameter must be a number."));
            return;
        }

        if (ui->pushButton_SlowLogPeriod->text().toInt(&OK) < 2 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be greater than 1."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be a number."));
            return;
        }

        if (ui->pushButton_FastLogPeriod->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Fast Log Period' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Fast Log Period' parameter must be a number."));
            return;
        }

        if (ui->pushButton_SlowLogPeriod->text().toInt() < ui->pushButton_FastLogPeriod->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be greater than 'Fast Log Period' parameter."));
            return;
        }
        if (ui->pushButton_MaxLogSpace->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Max Log Space' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Max Log Space' parameter must be a number."));
            return;
        }

        if (ui->pushButton_TraceWindow->text().toInt(&OK) < 3 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be greater than 2."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be a number."));
            return;
        }

        if (ui->pushButton_TraceWindow->text().toInt() < (ui->pushButton_FastLogPeriod->text().toInt()* 3))
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be at least three times 'Fast Log Period' parameter."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Silence' parameter must be a number."));
            return;
        }

        settings.setValue("SYSTEM/language", LanguageMap.key(ui->comboBoxLanguage->currentText(),DEFAULT_LANGUAGE));
        settings.setValue("SYSTEM/retries", ui->pushButton_Retries->text());
        settings.setValue("SYSTEM/blacklist", ui->pushButton_Blacklist->text());
        settings.setValue("SYSTEM/read_period_ms_1", ui->pushButton_ReadPeriod1->text());
        settings.setValue("SYSTEM/read_period_ms_2", ui->pushButton_ReadPeriod2->text());
        settings.setValue("SYSTEM/read_period_ms_3", ui->pushButton_ReadPeriod3->text());
        settings.setValue("SYSTEM/home_page", ui->pushButton_HomePage->text());
        settings.setValue("SYSTEM/start_page", ui->pushButton_StartPage->text());
        settings.setValue("SYSTEM/buzzer_touch", ui->checkBox_BuzzerTouch->isChecked());
        settings.setValue("SYSTEM/buzzer_alarm",  ui->checkBox_BuzzerAlarm->isChecked());
        settings.setValue("SYSTEM/pwd_timeout_s", ui->pushButton_PwdTimeout->text());
        settings.setValue("SYSTEM/pwd_logout_page", ui->pushButton_PwdLogoutPage->text());
        settings.setValue("SYSTEM/screen_saver_s", ui->pushButton_ScreenSaver->text());
        settings.setValue("SYSTEM/slow_log_period_s", ui->pushButton_SlowLogPeriod->text());
        settings.setValue("SYSTEM/fast_log_period_s", ui->pushButton_FastLogPeriod->text());
        settings.setValue("SYSTEM/max_log_space_MB", ui->pushButton_MaxLogSpace->text());
        settings.setValue("SYSTEM/trace_window_s", ui->pushButton_TraceWindow->text());
        settings.sync();
    }
    /* SERIAL 0 */
    if (groups.indexOf("SERIAL_PORT_0") >= 0)
    {
        if (ui->pushButton_Timeout_SERIAL_PORT_0->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_0->text().toInt() <= ui->pushButton_Silence_SERIAL_PORT_0->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            return;
        }

        if (ui->comboBox_Baudrate_SERIAL_PORT_0->currentIndex() > 0)
        {
            settings.setValue("SERIAL_PORT_0/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_0->currentText());
            settings.setValue("SERIAL_PORT_0/databits", ui->comboBox_Databits_SERIAL_PORT_0->currentText());
            settings.setValue("SERIAL_PORT_0/parity", ui->comboBox_Parity_SERIAL_PORT_0->currentText());
            settings.setValue("SERIAL_PORT_0/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_0->currentText());
            settings.setValue("SERIAL_PORT_0/silence_ms", ui->pushButton_Silence_SERIAL_PORT_0->text());
            settings.setValue("SERIAL_PORT_0/timeout_ms", ui->pushButton_Timeout_SERIAL_PORT_0->text());
        }
        else
        {
            settings.remove("SERIAL_PORT_0");
        }
        settings.sync();
    }
    /* SERIAL 1 */
    if (groups.indexOf("SERIAL_PORT_1") >= 0)
    {
        if (ui->pushButton_Silence_SERIAL_PORT_1->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_1->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_1->text().toInt() <= ui->pushButton_Silence_SERIAL_PORT_1->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            return;
        }

        if (ui->comboBox_Baudrate_SERIAL_PORT_1->currentIndex() > 0)
        {
            settings.setValue("SERIAL_PORT_1/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_1->currentText());
            settings.setValue("SERIAL_PORT_1/databits", ui->comboBox_Databits_SERIAL_PORT_1->currentText());
            settings.setValue("SERIAL_PORT_1/parity", ui->comboBox_Parity_SERIAL_PORT_1->currentText());
            settings.setValue("SERIAL_PORT_1/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_1->currentText());
            settings.setValue("SERIAL_PORT_1/silence_ms", ui->pushButton_Silence_SERIAL_PORT_1->text());
            settings.setValue("SERIAL_PORT_1/timeout_ms", ui->pushButton_Timeout_SERIAL_PORT_1->text());
        }
        else
        {
            settings.remove("SERIAL_PORT_1");
        }
        settings.sync();
    }
    /* SERIAL 2 */
    if (groups.indexOf("SERIAL_PORT_2") >= 0)
    {
        if (ui->pushButton_Silence_SERIAL_PORT_2->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }
        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_2->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_2->text().toInt() <= ui->pushButton_Silence_SERIAL_PORT_2->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            return;
        }

        if (ui->comboBox_Baudrate_SERIAL_PORT_2->currentIndex() > 0)
        {
            settings.setValue("SERIAL_PORT_2/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_2->currentText());
            settings.setValue("SERIAL_PORT_2/databits", ui->comboBox_Databits_SERIAL_PORT_2->currentText());
            settings.setValue("SERIAL_PORT_2/parity", ui->comboBox_Parity_SERIAL_PORT_2->currentText());
            settings.setValue("SERIAL_PORT_2/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_2->currentText());
            settings.setValue("SERIAL_PORT_2/silence_ms", ui->pushButton_Silence_SERIAL_PORT_2->text());
            settings.setValue("SERIAL_PORT_2/timeout_ms", ui->pushButton_Timeout_SERIAL_PORT_2->text());
        }
        else
        {
            settings.remove("SERIAL_PORT_2");
        }
        settings.sync();
    }
    /* SERIAL 3 */
    if (groups.indexOf("SERIAL_PORT_3") >= 0)
    {
        if (ui->pushButton_Silence_SERIAL_PORT_3->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_3->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_SERIAL_PORT_3->text().toInt() <= ui->pushButton_Silence_SERIAL_PORT_3->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            return;
        }

        if (ui->comboBox_Baudrate_SERIAL_PORT_3->currentIndex() > 0)
        {
            settings.setValue("SERIAL_PORT_3/baudrate", ui->comboBox_Baudrate_SERIAL_PORT_3->currentText());
            settings.setValue("SERIAL_PORT_3/databits", ui->comboBox_Databits_SERIAL_PORT_3->currentText());
            settings.setValue("SERIAL_PORT_3/parity", ui->comboBox_Parity_SERIAL_PORT_3->currentText());
            settings.setValue("SERIAL_PORT_3/stopbits", ui->comboBox_Stopbits_SERIAL_PORT_3->currentText());
            settings.setValue("SERIAL_PORT_3/silence_ms", ui->pushButton_Silence_SERIAL_PORT_3->text());
            settings.setValue("SERIAL_PORT_3/timeout_ms", ui->pushButton_Timeout_SERIAL_PORT_3->text());
        }
        else
        {
            settings.remove("SERIAL_PORT_3");
        }
        settings.sync();
    }
    /* TCP_IP */
    if (groups.indexOf("TCP_IP_PORT") >= 0)
    {
        if (ui->pushButton_Silence_TCP_IP_PORT->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_TCP_IP_PORT->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->pushButton_Timeout_TCP_IP_PORT->text().toInt() <= ui->pushButton_Silence_TCP_IP_PORT->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            return;
        }

        settings.setValue("TCP_IP_PORT/silence_ms", ui->pushButton_Silence_TCP_IP_PORT->text());
        settings.setValue("TCP_IP_PORT/timeout_ms", ui->pushButton_Timeout_TCP_IP_PORT->text());
        settings.sync();
    }
    /* CANOPEN 0 */
    if (groups.indexOf("CANOPEN_0") >= 0)
    {
        if (ui->comboBox_Baudrate_CANOPEN_0->currentIndex() > 0)
        {
            settings.setValue("CANOPEN_0/baudrate", ui->comboBox_Baudrate_CANOPEN_0->currentText());
        }
        else
        {
            settings.remove("CANOPEN_0");
        }
        settings.sync();
    }
    /* CANOPEN 1 */
    if (groups.indexOf("CANOPEN_1") >= 0)
    {
        if (ui->comboBox_Baudrate_CANOPEN_1->currentIndex() > 0)
        {
            settings.setValue("CANOPEN_1/baudrate", ui->comboBox_Baudrate_CANOPEN_1->currentText());
        }
        else
        {
            settings.remove("CANOPEN_1");
        }
        settings.sync();
    }

    readIniFile();

    QMessageBox::information(0,trUtf8("Information"),trUtf8("Configuration has been successfully saved."));
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

void system_ini::on_pushButton_PwdTimeout_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_PwdTimeout->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_PwdTimeout->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_ScreenSaver_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_ScreenSaver->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_ScreenSaver->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_SlowLogPeriod_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_SlowLogPeriod->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_SlowLogPeriod->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_FastLogPeriod_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_FastLogPeriod->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_FastLogPeriod->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_MaxLogSpace_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_MaxLogSpace->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_MaxLogSpace->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_TraceWindow_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_TraceWindow->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_TraceWindow->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Retries_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Retries->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Retries->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Blacklist_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Blacklist->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Blacklist->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_ReadPeriod1_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_ReadPeriod1->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_ReadPeriod1->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_ReadPeriod2_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_ReadPeriod2->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_ReadPeriod2->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_ReadPeriod3_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_ReadPeriod3->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_ReadPeriod3->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Timeout_SERIAL_PORT_0_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Timeout_SERIAL_PORT_0->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Timeout_SERIAL_PORT_0->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Silence_SERIAL_PORT_0_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Silence_SERIAL_PORT_0->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Silence_SERIAL_PORT_0->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Timeout_SERIAL_PORT_1_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Timeout_SERIAL_PORT_1->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Timeout_SERIAL_PORT_1->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Silence_SERIAL_PORT_1_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Silence_SERIAL_PORT_1->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Silence_SERIAL_PORT_1->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Timeout_SERIAL_PORT_2_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Timeout_SERIAL_PORT_2->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Timeout_SERIAL_PORT_2->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Silence_SERIAL_PORT_2_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Silence_SERIAL_PORT_2->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Silence_SERIAL_PORT_2->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Timeout_SERIAL_PORT_3_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Timeout_SERIAL_PORT_3->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Timeout_SERIAL_PORT_3->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Silence_SERIAL_PORT_3_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Silence_SERIAL_PORT_3->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Silence_SERIAL_PORT_3->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Timeout_TCP_IP_PORT_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Timeout_TCP_IP_PORT->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Timeout_TCP_IP_PORT->setText(QString().setNum(value));
    }
}

void system_ini::on_pushButton_Silence_TCP_IP_PORT_clicked()
{
    int value;
    numpad tatiera(&value, ui->pushButton_Silence_TCP_IP_PORT->text().toInt());
    tatiera.showFullScreen();
    if(tatiera.exec()==QDialog::Accepted)
    {
        ui->pushButton_Silence_TCP_IP_PORT->setText(QString().setNum(value));
    }
}
