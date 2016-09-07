#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSettings>
#include <QMessageBox>
#include <QFile>
#include <QInputDialog>
#include <QTextStream>
#include <QDirIterator>

char projectPath [256] = "";

#define SYSTEM_INI (QString(projectPath) + QString("/config/system.ini"))

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    if (!QFile(SYSTEM_INI).exists())
    {
        QMessageBox::critical(this,trUtf8("Error"),trUtf8("System.ini file don't exist, it's impossible tool open."));
        exit(1);
    }

    /*Lettura del file.pro per sapere e costruire la combobox @ le pagine attualmente in uso nel progetto*/
    /* open and load crosstable file */
    QDirIterator it(projectPath, QStringList("*.pro"));
    if (!it.hasNext())
    {
        QMessageBox::information(0, trUtf8("error"), trUtf8(""));
        exit(1);
    }
    QString itemstring = it.next();

    QFile file(itemstring);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, trUtf8("error"), trUtf8(""));
        exit(1);
    }

    QTextStream in(&file);
    QStringList pagesList;

    while(!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().split(' ').at(0).startsWith("page") && line.trimmed().split(' ').at(0).endsWith(".ui"))
        {
            pagesList << QFileInfo(line.trimmed().split(' ').at(0)).baseName();
        }
    }
    file.close();
    /*Aggiungo in coda le pagine di default già note*/
    pagesList << "alarms"
              << "alarms_history"
              << "recipe"
              << "recipe_select"
              << "store"
              << "store_filter"
              << "data_manager"
              << "display_settings"
              << "info"
              << "menu"
              << "options"
              << "system_ini"
              << "time_set"
              << "trend"
              << "trend_option"
              << "trend_range";

    /*Inserimento della lista delle pagine all'interno delle combobox*/
    ui->comboBox_HomePage->addItems(pagesList);
    ui->comboBox_StartPage->addItems(pagesList);
    ui->comboBox_PwdLogoutPage->addItem("");
    ui->comboBox_PwdLogoutPage->addItems(pagesList);

    QSettings settings(SYSTEM_INI, QSettings::IniFormat);

    LanguageMap.clear();

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
    QDirIterator lang_it(projectPath, QDirIterator::Subdirectories);
    QString lang = settings.value("SYSTEM/language", DEFAULT_LANGUAGE).toString();
    int i = 0;
    int indexlang = 0;
    while (lang_it.hasNext()) {
        QString item = lang_it.next();
        if (item.endsWith (".qm") == true)
        {
            QString tmplang = item.mid(item.indexOf("languages_") + strlen("languages_"), 2);
            if (lang == tmplang)
            {
                indexlang = i;
            }
            if (LanguageMap.count() > 0 && LanguageMap.contains(tmplang))
            {
                ui->comboBoxLanguage->addItem(LanguageMap.value(tmplang));
            }
            else
            {
                ui->comboBoxLanguage->addItem(tmplang);
            }
            i++;
        }
    }

    ui->comboBoxLanguage->setCurrentIndex(indexlang);
    ui->lineEdit_Retries->setText(settings.value("SYSTEM/retries").toString());
    ui->lineEdit_Blacklist->setText(settings.value("SYSTEM/blacklist").toString());
    ui->lineEdit_ReadPeriod1->setText(settings.value("SYSTEM/read_period_ms_1").toString());
    ui->lineEdit_ReadPeriod2->setText(settings.value("SYSTEM/read_period_ms_2").toString());
    ui->lineEdit_ReadPeriod3->setText(settings.value("SYSTEM/read_period_ms_3").toString());
    ui->comboBox_HomePage->setCurrentIndex(ui->comboBox_HomePage->findText(settings.value("SYSTEM/home_page").toString()));
    ui->comboBox_StartPage->setCurrentIndex(ui->comboBox_StartPage->findText(settings.value("SYSTEM/start_page").toString()));
    ui->checkBox_BuzzerTouch->setChecked(settings.value("SYSTEM/buzzer_touch").toBool());
    ui->checkBox_BuzzerAlarm->setChecked(settings.value("SYSTEM/buzzer_alarm").toBool());
    ui->lineEdit_PwdTimeout->setText(settings.value("SYSTEM/pwd_timeout_s").toString());
    ui->comboBox_PwdLogoutPage->setCurrentIndex(ui->comboBox_PwdLogoutPage->findText(settings.value("SYSTEM/pwd_logout_page").toString()));
    ui->lineEdit_ScreenSaver->setText(settings.value("SYSTEM/screen_saver_s").toString());
    ui->lineEdit_SlowLogPeriod->setText(settings.value("SYSTEM/slow_log_period_s").toString());
    ui->lineEdit_FastLogPeriod->setText(settings.value("SYSTEM/fast_log_period_s").toString());
    ui->lineEdit_MaxLogSpace->setText(settings.value("SYSTEM/max_log_space_MB").toString());
    ui->lineEdit_TraceWindow->setText(settings.value("SYSTEM/trace_window_s").toString());

    int index;
    QString value;

    value = settings.value("SERIAL_PORT_0/baudrate").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_0->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_0/databits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_0->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_0/parity").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_0->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_0/stopbits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_0->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_0->setText(settings.value("SERIAL_PORT_0/silence_ms").toString());
    ui->lineEdit_Timeout_SERIAL_PORT_0->setText(settings.value("SERIAL_PORT_0/timeout_ms").toString());

    value = settings.value("SERIAL_PORT_1/baudrate").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_1->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_1/databits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_1->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_1/parity").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_1->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_1/stopbits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_1->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_1->setText(settings.value("SERIAL_PORT_1/silence_ms").toString());
    ui->lineEdit_Timeout_SERIAL_PORT_1->setText(settings.value("SERIAL_PORT_1/timeout_ms").toString());

    value = settings.value("SERIAL_PORT_2/baudrate").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_2->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_2/databits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_2->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_2/parity").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_2->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_2/stopbits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_2->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_2->setText(settings.value("SERIAL_PORT_2/silence_ms").toString());
    ui->lineEdit_Timeout_SERIAL_PORT_2->setText(settings.value("SERIAL_PORT_2/timeout_ms").toString());

    value = settings.value("SERIAL_PORT_3/baudrate").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_3->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_3/databits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_3->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_3/parity").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_3->setCurrentIndex(index);
    }

    value = settings.value("SERIAL_PORT_3/stopbits").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_3->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_3->setText(settings.value("SERIAL_PORT_3/silence_ms").toString());
    ui->lineEdit_Timeout_SERIAL_PORT_3->setText(settings.value("SERIAL_PORT_3/timeout_ms").toString());

    ui->lineEdit_Silence_TCP_IP_PORT->setText(settings.value("TCP_IP_PORT/silence_ms").toString());
    ui->lineEdit_Timeout_TCP_IP_PORT->setText(settings.value("TCP_IP_PORT/timeout_ms").toString());

    value = settings.value("CANOPEN_0/baudrate").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_CANOPEN_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for CANOPEN_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_CANOPEN_0->itemText(index)));
        }
        ui->comboBox_Baudrate_CANOPEN_0->setCurrentIndex(index);
    }

    value = settings.value("CANOPEN_1/baudrate").toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_CANOPEN_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for CANOPEN_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_CANOPEN_1->itemText(index)));
        }
        ui->comboBox_Baudrate_CANOPEN_1->setCurrentIndex(index);
    }
}


//funzione salvataggio del file
void MainWindow::save_all()
{
    bool OK;

    QSettings settings(SYSTEM_INI, QSettings::IniFormat);
    QStringList groups = settings.childGroups();

    /* SYSTEM */
    if (groups.indexOf("SYSTEM") >= 0)
    {
        if (ui->lineEdit_Retries->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Retries' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Retries' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Blacklist->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Blacklist' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Blacklist' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_ReadPeriod1->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 1' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 1' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_ReadPeriod2->text().toInt(&OK) < 2 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be greater than 1."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_ReadPeriod3->text().toInt(&OK) < 3 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be greater than 2."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_ReadPeriod3->text().toInt() < ui->lineEdit_ReadPeriod2->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be greater than 'Read Period 2' parameter."));
            return;
        }

        if (ui->lineEdit_ReadPeriod2->text().toInt() < ui->lineEdit_ReadPeriod1->text().toInt())
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
        if (ui->lineEdit_PwdTimeout->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Pwd Timeout' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Pwd Timeout' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_ScreenSaver->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Screen Saver' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Screen Saver' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_SlowLogPeriod->text().toInt(&OK) < 2 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be greater than 1."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_FastLogPeriod->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Fast Log Period' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Fast Log Period' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_SlowLogPeriod->text().toInt() < ui->lineEdit_FastLogPeriod->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be greater than 'Fast Log Period' parameter."));
            return;
        }
        if (ui->lineEdit_MaxLogSpace->text().toInt(&OK) <= 0 && ui->lineEdit_MaxLogSpace->text().toInt(&OK) > MAX_SPACE_AVAILABLE_MAX && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Max Log Space' parameter must be greater than 0 and less than %1.").arg(MAX_SPACE_AVAILABLE_MAX));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Max Log Space' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_TraceWindow->text().toInt(&OK) < 3 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be greater than 2."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_TraceWindow->text().toInt() < (ui->lineEdit_FastLogPeriod->text().toInt()* 3))
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
        settings.setValue("SYSTEM/retries", ui->lineEdit_Retries->text());
        settings.setValue("SYSTEM/blacklist", ui->lineEdit_Blacklist->text());
        settings.setValue("SYSTEM/read_period_ms_1", ui->lineEdit_ReadPeriod1->text());
        settings.setValue("SYSTEM/read_period_ms_2", ui->lineEdit_ReadPeriod2->text());
        settings.setValue("SYSTEM/read_period_ms_3", ui->lineEdit_ReadPeriod3->text());
        settings.setValue("SYSTEM/home_page", ui->comboBox_HomePage->currentText());
        settings.setValue("SYSTEM/start_page", ui->comboBox_StartPage->currentText());
        settings.setValue("SYSTEM/buzzer_touch", (ui->checkBox_BuzzerTouch->isChecked() == true)?1:0);
        settings.setValue("SYSTEM/buzzer_alarm", (ui->checkBox_BuzzerAlarm->isChecked() == true)?1:0);
        settings.setValue("SYSTEM/pwd_timeout_s", ui->lineEdit_PwdTimeout->text());
        settings.setValue("SYSTEM/pwd_logout_page", ui->comboBox_PwdLogoutPage->currentText());
        settings.setValue("SYSTEM/screen_saver_s", ui->lineEdit_ScreenSaver->text());
        settings.setValue("SYSTEM/slow_log_period_s", ui->lineEdit_SlowLogPeriod->text());
        settings.setValue("SYSTEM/fast_log_period_s", ui->lineEdit_FastLogPeriod->text());
        settings.setValue("SYSTEM/max_log_space_MB", ui->lineEdit_MaxLogSpace->text());
        settings.setValue("SYSTEM/trace_window_s", ui->lineEdit_TraceWindow->text());
        settings.sync();
    }
    /* SERIAL 0 */
    if (groups.indexOf("SERIAL_PORT_0") >= 0)
    {
        if (ui->lineEdit_Timeout_SERIAL_PORT_0->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_0->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_0->text().toInt())
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
            settings.setValue("SERIAL_PORT_0/silence_ms", ui->lineEdit_Silence_SERIAL_PORT_0->text());
            settings.setValue("SERIAL_PORT_0/timeout_ms", ui->lineEdit_Timeout_SERIAL_PORT_0->text());
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
        if (ui->lineEdit_Silence_SERIAL_PORT_1->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_1->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_1->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_1->text().toInt())
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
            settings.setValue("SERIAL_PORT_1/silence_ms", ui->lineEdit_Silence_SERIAL_PORT_1->text());
            settings.setValue("SERIAL_PORT_1/timeout_ms", ui->lineEdit_Timeout_SERIAL_PORT_1->text());
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
        if (ui->lineEdit_Silence_SERIAL_PORT_2->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_2->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_2->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_2->text().toInt())
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
            settings.setValue("SERIAL_PORT_2/silence_ms", ui->lineEdit_Silence_SERIAL_PORT_2->text());
            settings.setValue("SERIAL_PORT_2/timeout_ms", ui->lineEdit_Timeout_SERIAL_PORT_2->text());
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
        if (ui->lineEdit_Silence_SERIAL_PORT_3->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_3->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 3' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_3->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_3->text().toInt())
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
            settings.setValue("SERIAL_PORT_3/silence_ms", ui->lineEdit_Silence_SERIAL_PORT_3->text());
            settings.setValue("SERIAL_PORT_3/timeout_ms", ui->lineEdit_Timeout_SERIAL_PORT_3->text());
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
        if (ui->lineEdit_Silence_TCP_IP_PORT->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be greater than or egual 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_TCP_IP_PORT->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 0."));
            return;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be a number."));
            return;
        }

        if (ui->lineEdit_Timeout_TCP_IP_PORT->text().toInt() <= ui->lineEdit_Silence_TCP_IP_PORT->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            return;
        }

        settings.setValue("TCP_IP_PORT/silence_ms", ui->lineEdit_Silence_TCP_IP_PORT->text());
        settings.setValue("TCP_IP_PORT/timeout_ms", ui->lineEdit_Timeout_TCP_IP_PORT->text());
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

    QMessageBox::information(0,trUtf8("Information"),trUtf8("Configuration has been successfully saved."));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_Save_clicked()
{
    save_all();
}

