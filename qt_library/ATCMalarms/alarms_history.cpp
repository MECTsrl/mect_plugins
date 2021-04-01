/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Alarm History
 */
#include <QDir>
#include <QMessageBox>
#include "app_logprint.h"
#include "app_usb.h"
#include "alarms_history.h"
#include "hmi_logger.h"
#include "ui_alarms_history.h"
#include "global_functions.h"

/* this define set the window title */
#define WINDOW_TITLE "ALARM HISTORY"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ":/libicons/img/Home.png"

/**
 * @brief this macro is used to set the ALARM_HISTORY style.
 * the syntax is html stylesheet-like
 */
#define SET_ALARM_HISTORY_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

bool doReloadAlarmsLog = false;

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
alarms_history::alarms_history(QWidget *parent) :
    page(parent),
    ui(new Ui::alarms_history)
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
    //setStyle::set(this);
    /* set the style described into the macro SET_ALARM_HISTORY_STYLE */
    SET_ALARM_HISTORY_STYLE();
    
    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the actual user name */
    //labelUserName = ui->labelUserName;
    
    _current = 0;
    _alarm = true;
    _event = true;
    _level = level_all_e;
    _file_nb = 0; // unused
    
    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void alarms_history::reload()
{
    /* clear the old value */
    ui->listWidget->clear();
    ui->pushButtonSave->setEnabled(false);

    /* get the file list */
    QDir logDir(ALARMS_DIR);
    logFileList = logDir.entryList(QDir::Files|QDir::NoDotAndDotDot, QDir::Reversed);
    
    ui->comboBoxDate->clear();
    _current = -1;

    for (int i = 0; i < logFileList.count(); i++)
    {
        if (logFileList.at(i).endsWith(".log") == false)
        {
            logFileList.removeAt(i);
            i--;
        }
        else
        {
            QString label = QDateTime().fromString(logFileList.at(i),"yyyy_MM_dd_HH_mm_ss.log").toString("yyyy/MM/dd - HH:mm:ss");
            if (label.length() == 0)
            {
                label = logFileList.at(i);
            }
            ui->comboBoxDate->addItem(label);
            _current = 0;
        }
    }
    
    _alarm = true;
    _event = true;
    _level = level_all_e;
    
    /* no logfile found */
    if (_current == -1)
    {
        LOG_PRINT(warning_e, "No alarms log file to load.\n");
        return;
    }
    
    /* load the current log file */
    LOG_PRINT(verbose_e, "_current %d\n",_current);
    ui->comboBoxDate->setCurrentIndex(_current);
    loadLogFile(_current, _alarm, _event, _level);
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void alarms_history::updateData()
{
    /* call the parent updateData member */
    page::updateData();
    ui->pushButtonSave->setEnabled(USBCheck());
    if (doReloadAlarmsLog && _current == 0)
    {
        doReloadAlarmsLog = false;
        loadLogFile(_current, _alarm, _event, _level);
    }
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void alarms_history::changeEvent(QEvent * event)
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
alarms_history::~alarms_history()
{
    delete ui;
}

/**
 * @brief Load the log file number fileNb (0 is the last one) and filter by alarm, event and level
 */
bool alarms_history::loadLogFile(int fileNb, bool alarm, bool event, int level)
{
    if (logFileList.count() == 0 || fileNb < 0 || fileNb > logFileList.count())
    {
        return false;
    }
    LOG_PRINT(verbose_e, "fileNb %d < %d\n", fileNb, logFileList.count());
    LOG_PRINT(verbose_e, "logFileList '%s'\n", logFileList.at(fileNb).toAscii().data());
    return loadLogFile(logFileList.at(fileNb).toAscii().data(), alarm, event, level);
}

bool alarms_history::loadLogFile(const char * filename, bool alarm, bool event, int level)
{
    FILE * fp;
    char line[LINE_SIZE] = "";
    char fileName[LINE_SIZE] = "";
    char * p = NULL;
    
    sprintf(fileName, "%s/%s", ALARMS_DIR, filename);
    
    fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", fileName);
        return false;
    }
    LOG_PRINT(verbose_e, "opened '%s'\n", line);
    
    ui->listWidget->clear();
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        /* tag;event;YYYY/MM/DD,HH:mm:ss;description */

        /* tag */
        p = strtok(line, ";");
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Malformed log file '%s' [%s]\n", fileName, line);
            return false;
        }
        char tag[32];
        strcpy(tag, p);

        /* event */
        p = strtok(NULL, ";");
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Malformed log file '%s' [%s]\n", fileName, line);
            return false;
        }
        char event[32];
        strcpy(event, p);

        /* date */
        p = strtok(NULL, ";");
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Malformed log file '%s' [%s]\n", fileName, line);
            return false;
        }
        char date[32];
        strcpy(date, p);

        /* description */
        p = strtok(NULL, ";");
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Malformed log file '%s' [%s]\n", fileName, line);
            return false;
        }
        char description[256];
        strcpy(description, p);
        if (strchr(description, '\n'))
        {
            *strchr(description, '\n') = '\0';
        }

        /* change this code in order to change the output into the alarm history widget */
        sprintf(line, "%s - %s - [%s]",
                date,
                description,
                event);

        ui->listWidget->addItem(line);
        LOG_PRINT(verbose_e, "add '%s'\n", line);
    }
    LOG_PRINT(verbose_e, "COUNT '%d'\n", ui->listWidget->count());

    return true;
}

void alarms_history::on_pushButtonHome_clicked()
{
    go_home();
}

void alarms_history::on_pushButtonBack_clicked()
{
    go_back();
}

void alarms_history::on_pushButtonPrevious_clicked()
{
    LOG_PRINT(verbose_e, "_current %d\n", _current);
    if (_current < 0)
    {
        return;
    }
    else if (_current == 0)
    {
        _current = ui->comboBoxDate->count() - 1;
    }
    else
    {
        _current--;
    }
    ui->comboBoxDate->setCurrentIndex(_current);
}

void alarms_history::on_pushButtonNext_clicked()
{
    LOG_PRINT(verbose_e, "_current %d\n", _current);
    if (_current < 0)
    {
        return;
    }
    else if (_current >= (ui->comboBoxDate->count() - 1))
    {
        _current = 0;
    }
    else
    {
        _current++;
    }
    ui->comboBoxDate->setCurrentIndex(_current);
}

void alarms_history::on_comboBoxDate_currentIndexChanged(int index)
{
    if (index < 0 || ui->comboBoxDate->count() < 1)
    {
        return;
    }
    _current = index;
    if (loadLogFile(_current, _alarm, _event, _level) == false)
    {
        LOG_PRINT(error_e, "cannot open log file %d\n", _current);
    }
}

void alarms_history::on_pushButtonSave_clicked()
{
    /* Check USB key */
    if (USBCheck())
    {
        if (USBmount() == false)
        {
            LOG_PRINT(error_e, "cannot mount the usb key\n");
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot mount the usb key"));
            return;
        }
        
        char line[LINE_SIZE];
        FILE * fp;
        FILE * fpout;
        char srcfilename [MAX_LINE] = "";
        char dstfilename [MAX_LINE] = "";

        /* compose the source file name ans the destination file name */
        sprintf(srcfilename, "%s/%s", ALARMS_DIR, logFileList.at(_current).toAscii().data());
        sprintf(dstfilename, "%s/%s",
                usb_mnt_point,
                logFileList.at(_current).toAscii().data());

        /* create the signature file */

        /* Open the command for reading. */
        sprintf(line, "%s %s", APP_SIGN, srcfilename);
        fp = popen(line, "r");
        if (fp == NULL) {
            LOG_PRINT(error_e,"Failed to run command '%s'\n", line );
            return;
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
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1'").arg(line));
            USBumount();
            return;
        }

        sprintf(line, "%s.sign", dstfilename);
        fpout = fopen(line, "w");
        if (fpout == NULL)
        {
            LOG_PRINT(error_e, "cannot open '%s'\n", line);
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1'").arg(line));
            USBumount();
            return;
        }
        fprintf(fpout, "%s\n", sign);
        fclose(fpout);

        /* zip the file, the sign file and delete them */
        if (zipAndSave(QStringList() << QString("%1.sign").arg(dstfilename) << QString(srcfilename), QString("%1.alarm.zip").arg(dstfilename), true) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot save the zip file '%1.alarm.zip'").arg(dstfilename));
            USBumount();
            return;
        }

        QFile::remove(QString("%1.sign").arg(dstfilename));
        
        /* unmount USB key */
        USBumount();
        LOG_PRINT(verbose_e, "DOWNLOADED\n");
        QMessageBox::information(this,trUtf8("USB info"), trUtf8("File '%1' saved.").arg(QFileInfo(dstfilename).fileName()));
    }
}

