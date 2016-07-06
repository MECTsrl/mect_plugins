/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
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
#define WINDOW_ICON ":/systemicons/img/Home.png"

/**
 * @brief this macro is used to set the ALARM_HISTORY style.
 * the syntax is html stylesheet-like
 */
#define SET_ALARM_HISTORY_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

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
    _file_nb = 0;
    
#ifdef LEVEL_TYPE
    ui->comboBoxLevel->clear();
    for (int i = level_all_e; i < nb_of_level_e; i++)
    {
        ui->comboBoxLevel->addItem(QString("%1").arg(i));
    }
#endif
    
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
        }
    }
    
    _current = 0;
    _alarm = true;
    _event = true;
    _level = level_all_e;
    
    /* no logfile found */
    _file_nb = logFileList.count();
    if (_file_nb == 0)
    {
        LOG_PRINT(warning_e, "No alarms log file to load.\n");
        return;
    }
    
    /* load the current log file */
    LOG_PRINT(info_e, "_current %d\n",_current);
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
    if (fileNb < 0 || fileNb > logFileList.count())
    {
        return false;
    }
    LOG_PRINT(info_e, "fileNb %d < %d\n", fileNb, logFileList.count());
    LOG_PRINT(info_e, "logFileList '%s'\n", logFileList.at(fileNb).toAscii().data());
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
    LOG_PRINT(info_e, "opened '%s'\n", line);
    
    ui->listWidget->clear();
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        /* type;level;tag;event;YYYY/MM/DD,HH:mm:ss;description */
#ifdef LEVEL_TYPE
        /* type */
        p = strtok(line, ";");
        if (p == NULL)
        {
            LOG_PRINT(info_e, "Skip empty line'%s'\n", line);
            continue;
        }
        /* skip the alarms */
        if (atoi(p) == ALARM && alarm == false)
        {
            LOG_PRINT(info_e, "Skip alarm '%s'\n", line);
            continue;
        }
        /* skip the events */
        if (atoi(p) == EVENT && event == false)
        {
            LOG_PRINT(info_e, "Skip event '%s'\n", line);
            continue;
        }

        /* level */
        p = strtok(NULL, ";");
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Malformed log file '%s' [%s]\n", fileName, line);
            return false;
        }
        /* skip the level */
        if (atoi(p) < level)
        {
            LOG_PRINT(info_e, "Skip level '%d %d'\n", atoi(p), level);
            continue;
        }
        /* tag */
        p = strtok(NULL, ";");
#else
        /* tag */
        p = strtok(line, ";");
#endif
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
        LOG_PRINT(info_e, "add '%s'\n", line);
    }
    LOG_PRINT(info_e, "COUNT '%d'\n", ui->listWidget->count());

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
    if (_file_nb == 0) return;
    
    LOG_PRINT(info_e, "_current %d\n", _current);
    if (_current == 0)
    {
        _current = _file_nb - 1;
    }
    else
    {
        _current--;
    }
    ui->comboBoxDate->setCurrentIndex(_current);
}

void alarms_history::on_pushButtonNext_clicked()
{
    if (_file_nb == 0) return;
    LOG_PRINT(info_e, "_current %d\n", _current);
    if (_current == _file_nb - 1)
    {
        _current = 0;
    }
    else
    {
        _current++;
    }
    ui->comboBoxDate->setCurrentIndex(_current);
}

#ifdef LEVEL_TYPE
void alarms_history::on_comboBoxLevel_currentIndexChanged(int index)
{
    LOG_PRINT(info_e, "_level %d\n", _level);
    /* 0 is all level */
    if (_level != index && index < nb_of_level_e)
    {
        _level = index;
        LOG_PRINT(info_e, "_level %d\n", _level);
        if (_file_nb == 0) return;
        if (loadLogFile(_current, _alarm, _event, _level) == false)
        {
            LOG_PRINT(error_e, "Cannot open log file %d\n", _current);
        }
    }
}

void alarms_history::on_comboBoxType_currentIndexChanged(int index)
{
    /* 0 is all
     * 1 is alarm
     * 2 is event
     */
    switch(index)
    {
    case 0:
        _alarm = true;
        _event = true;
        break;
    case 1:
        _alarm = true;
        _event = false;
        break;
    case 2:
        _alarm = false;
        _event = true;
        break;
    default:
        _alarm = true;
        _event = true;
        break;
    }
    LOG_PRINT(info_e, "_current %d\n", _current);
    if (_file_nb == 0) return;
    if (loadLogFile(_current, _alarm, _event, _level) == false)
    {
        LOG_PRINT(error_e, "Cannot open log file %d\n", _current);
    }
}
#endif
void alarms_history::on_comboBoxDate_currentIndexChanged(int index)
{
    if (index < 0)
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

#ifdef LEVEL_TYPE
        char srcfilename [MAX_LINE] = "";
        char dstfilename [MAX_LINE] = "";
        
        /* compose the source file name ans the destination file name */
        sprintf(srcfilename, "%s/%s", ALARMS_DIR, logFileList.at(_current).toAscii().data());
        sprintf(dstfilename, "%s/%s",
                usb_mnt_point,
                logFileList.at(_current).toAscii().data());
        
        /* prepare the alarm filtered file */
        
        /* open the source alarm file */
        fp = fopen(srcfilename, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "cannot open '%s'\n", srcfilename);
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot open the alarm file '%1'").arg(srcfilename));
            return;
        }
        /* open the dest alarm file */
        fpout = fopen(dstfilename, "w");
        if (fpout == NULL)
        {
            LOG_PRINT(error_e, "cannot open '%s'\n", dstfilename);
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot write the alarm file '%1'").arg(srcfilename));
            return;
        }
        char lineout[LINE_SIZE];
        char * p;
        /* read from source and filter the level an the type of alarm and dump the dest alarm file */
        while (fgets(line, LINE_SIZE, fp) != NULL)
        {
            strcpy(lineout,line);
            /* type;level;tag;event;YYYY/MM/DD,HH:mm:ss;description */
            /* type */
            p = strtok(line, ";");
            if (p == NULL)
            {
                LOG_PRINT(info_e, "Skip empty line'%s'\n", line);
                continue;
            }
            /* skip the alarms */
            if (atoi(p) == ALARM && _alarm == false)
            {
                LOG_PRINT(info_e, "Skip alarm '%s'\n", line);
                continue;
            }
            /* skip the events */
            if (atoi(p) == EVENT && _event == false)
            {
                LOG_PRINT(info_e, "Skip event '%s'\n", line);
                continue;
            }
            /* level */
            p = strtok(NULL, ";");
            if (p == NULL)
            {
                LOG_PRINT(error_e, "Malformed log file '%s' [%s]\n", srcfilename, line);
                return;
            }
            /* skip the level */
            if (atoi(p) < _level)
            {
                LOG_PRINT(info_e, "Skip level '%d %d'\n", atoi(p), _level);
                continue;
            }
            fprintf(fpout, "%s", lineout);
        }
        fclose(fp);
        fclose(fpout);

#else
        char dstfilename [MAX_LINE] = "";
        sprintf(dstfilename, "%s/%s", ALARMS_DIR, logFileList.at(_current).toAscii().data());
#endif

        /* create the signature file */

        /* Open the command for reading. */
        sprintf(line, "%s %s", APP_SIGN, dstfilename);
        fp = popen(line, "r");
        if (fp == NULL) {
            LOG_PRINT(error_e,"Failed to run command '%s'\n", line );
            return;
        }

        char sign[LINE_SIZE];

        /* Read the output a line at a time - output it. */
        if (fscanf(fp, "%s", sign) > 0) {
            LOG_PRINT(info_e,"SIGN: '%s'\n", sign);
        }

        /* close */
        pclose(fp);

        if (sign[0] == '\0')
        {
            LOG_PRINT(error_e,"Failed read sign\n");
            QFile::remove(dstfilename);
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1'").arg(line));
            USBumount();
            return;
        }

        sprintf(line, "%s.sign", dstfilename);
        fpout = fopen(line, "w");
        if (fpout == NULL)
        {
            LOG_PRINT(error_e, "cannot open '%s'\n", line);
            QFile::remove(dstfilename);
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1'").arg(line));
            USBumount();
            return;
        }
        fprintf(fpout, "%s\n", sign);
        fclose(fpout);

        /* zip the file, the sign file and delete them */
        if (zipAndSave(QStringList() << QString("%1.sign").arg(dstfilename) << QString(dstfilename), QString("%1.zip").arg(dstfilename), true) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot save the zip file '%1.zip'").arg(dstfilename));
            USBumount();
            return;
        }
        
#ifdef LEVEL_TYPE
        QFile::remove(dstfilename);
#endif
        QFile::remove(QString("%1.sign").arg(dstfilename));
        
        /* unmount USB key */
        USBumount();
        LOG_PRINT(info_e, "DOWNLOADED\n");
        QMessageBox::information(this,trUtf8("USB info"), trUtf8("File '%1' saved.").arg(dstfilename));
    }
}

