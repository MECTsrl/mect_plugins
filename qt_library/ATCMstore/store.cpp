/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include <QMessageBox>
#include <QTableWidget>
#include <QStringList>
#include <QDir>
#include <errno.h>

#include "global_functions.h"
#include "app_logprint.h"
#include "app_usb.h"
#include "store.h"
#include "store_extract.h"
#include "app_usb.h"
#include "ui_store.h"
#include "utility.h"

#define LINE_BUFFER_SIZE 12
#define UNDEFINED ""

/* this define set the variables list to be to displayed in this page */
#undef VAR_TO_DISPLAY
/* Example:
#define VAR_TO_DISPLAY "RET_REG_1" << "RET_REG_3" << "NRE_REG_1"
 */
/* this define set the window title */
#define WINDOW_TITLE "STORE"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the store style.
 * the syntax is html stylesheet-like
 */
#define SET_STORE_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

bool actual_filter[DB_SIZE_ELEM];

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
store::store(QWidget *parent) :
    page(parent),
    ui(new Ui::store)
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
    
    //setStyle::set(this);
    SET_STORE_STYLE();
    
    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;
    
    status = 0;
    logfp = NULL;
    
    /* set as default current day */
    StoreInit = QDateTime(QDate::currentDate(), QTime(0,0,0));
    StoreFinal = QDateTime(QDate::currentDate(), QTime(23,59,59));
    //reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void store::reload()
{
    status = 1;
    
}

/** @brief count the column of a csv file and extract the header
 */
int store::getLogColumnNb(const char * filename)
{
    FILE * fp;
    char line[LINE_SIZE] = "";
    
    headerList.clear();
    
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", filename);
        return 0;
    }
    LOG_PRINT(verbose_e, "opened '%s'\n", filename);
    
    if (fgets(line, LINE_SIZE, fp) != NULL)
    {
        LINE2STR(line);
        headerList = QString(line).simplified().replace(QString(" "), QString("")).split(SEPARATOR);
    }
    
    fclose(fp);
    return headerList.count();
}

bool store::LoadStoreFilter(const char * filename)
{
    int rownb = 0;
    int columnnb = 0;
    FILE * fp;
    char line[LINE_SIZE] = "";

    QDir logDir(STORE_DIR);
    QStringList fileList = logDir.entryList(QStringList("*.log"), QDir::Files|QDir::NoDotAndDotDot, QDir::Reversed);
    if (fileList.count() == 0)
    {
        return false;
    }
    sprintf(line, "%s/%s", STORE_DIR, fileList.at(0).toAscii().data());
    sizeof_filter = getLogColumnNb(line);
    if (sizeof_filter == 0)
    {
        return false;
    }
    
    /* clear the actual filter */
    for (int i = 0; i <= sizeof_filter; i++)
    {
        actual_filter[i] = false;
    }

    if (filename[0] != '\0')
    {
        fp = fopen(filename, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "cannot open '%s'\n", filename);
            return false;
        }
        LOG_PRINT(verbose_e, "opened '%s'\n", line);
        /*
     * the file is formatted as
     * <Tag1>
     * ...
     * <TagN>
     */
        QStringList wrongVariables;
        bool something_to_show = false;
        while (fgets(line, LINE_SIZE, fp) != NULL)
        {
            QString linestr = QString(line).simplified();
            if (QString(line).simplified().length() == 0)
            {
                LOG_PRINT(warning_e, "skipping empty line\n");
                continue;
            }

            columnnb = headerList.indexOf(linestr);
            if (columnnb>=0 && columnnb <= sizeof_filter)
            {
                actual_filter[columnnb] = true;
                LOG_PRINT(verbose_e, "tag '%s' actual_filter[%d] = %d\n", linestr.toAscii().data(), columnnb, actual_filter[columnnb]);
                something_to_show = true;
            }
            else
            {
                wrongVariables << linestr;
                LOG_PRINT(error_e, "cannot find tag '%s' into the log header (%s).\n", linestr.toAscii().data(), headerList.join("|").toAscii().data());
            }
            rownb++;
        }
        fclose(fp);
        if (something_to_show == false)
        {
            QMessageBox::critical(this,trUtf8("No variable to show."),
                                     trUtf8("Cannot find any variable to show:\n%1\ninto the log header:\n%2.").arg(wrongVariables.join(",")).arg(headerList.join("|"))
                                     );
        }
        else if (wrongVariables.length() > 0)
        {
            QMessageBox::information(this,trUtf8("Variables not found."),
                                     trUtf8("Cannot find the variables:\n%1\ninto the log header:\n%2.").arg(wrongVariables.join(",")).arg(headerList.join("|"))
                                     );
        }
        return (rownb > 0) && something_to_show;
    }
    /* no filter, load all variables */
    else
    {
        for (int i = 0; i <= sizeof_filter; i++)
        {
            actual_filter[i] = true;
        }
        return true;
    }
}

bool store::LoadStore(QDateTime init, QDateTime final)
{
    char command[256];
    outputfile[0] = '\0';
    
    /* clear the table */
    while(ui->tableWidget->columnCount())
    {
        ui->tableWidget->removeColumn(0);
    }
    while(ui->tableWidget->rowCount())
    {
        ui->tableWidget->removeRow(0);
    }

    mkdir(TMPDIR, S_IRWXU | S_IRWXG);
    sprintf(command, "umount %s >/dev/null 2>&1", TMPDIR);
    /* if TMPDIR is mounted, unmount it */
    system (command);
    sprintf(command, "mount -t tmpfs -o size=32M tmpfs %s >/dev/null 2>&1", TMPDIR);
    if (system (command) != 0)
    {
        LOG_PRINT(error_e, "cannot execute '%s'\n", command);
    }

    char filename[FILENAME_MAX];
    if (_actual_store_[0] == '\0' || strcmp(_actual_store_, "store") == 0)
    {
        filename[0] = '\0';
    }
    else
    {
        if (QFileInfo(_actual_store_).suffix().compare("csv") == 0)
        {
            sprintf(filename, "%s/%s", CUSTOM_STORE_DIR, _actual_store_);
        }
        else
        {
            sprintf(filename, "%s/%s.csv", CUSTOM_STORE_DIR, _actual_store_);
        }
        if (!QFile::exists(filename))
        {
            LOG_PRINT(error_e, "cannot open store '%s' [%s]\n", _actual_store_, filename);
            return false;
        }
    }
    /* remove TMPDIR */
    if (StoreFilter(
                outputfile,
                STORE_DIR,
                TMPDIR,
                filename,
                init.date().toString("yyyy/MM/dd").toAscii().data(),
                init.time().toString("hh:mm:ss").toAscii().data(),
                final.date().toString("yyyy/MM/dd").toAscii().data(),
                final.time().toString("hh:mm:ss").toAscii().data()
                ) != 0)
    {
        LOG_PRINT(error_e, "outputfile '%s'\n", outputfile);
        return false;
    }
    LOG_PRINT(verbose_e, "outputfile '%s'\n", outputfile);
    return LoadStore(QString("%1/%2").arg(TMPDIR).arg(outputfile).toAscii().data());
}
bool store::LoadStore(int fileNb)
{
    if (fileNb < 0 || fileNb > logFileList.count())
    {
        LOG_PRINT(error_e, "invalid number '%d'\n", fileNb);
        return false;
    }
    LOG_PRINT(verbose_e, "fileNb %d < %d\n", fileNb, logFileList.count());
    LOG_PRINT(verbose_e, "logFileList '%s'\n", logFileList.at(fileNb).toAscii().data());
    return LoadStore(QString(STORE_DIR + logFileList.at(fileNb)).toAscii().data());
}

bool store::LoadStore(const char * filename)
{
    char line[LINE_SIZE] = "";
    char * p = NULL, * r = NULL;
    
    /* clear the table */
    while(ui->tableWidget->columnCount())
    {
        ui->tableWidget->removeColumn(0);
    }
    while(ui->tableWidget->rowCount())
    {
        ui->tableWidget->removeRow(0);
    }

    current_row = 0;
    current_column = 0;
    
    int i ,j = 0;
    for (i = 0; i <= sizeof_filter; i++)
    {
        if (actual_filter[i] == true && j < headerList.count())
        {
            ui->tableWidget->insertColumn(j);
            LOG_PRINT(verbose_e, "Inserted column '%d' '%s'\n", j, headerList[i].toAscii().data());
            ui->tableWidget->setHorizontalHeaderItem(j, new QTableWidgetItem(headerList[i]));
            j++;
        }
    }
    
    ui->tableWidget->verticalHeader()->hide();
    
    //sprintf(line, "%s/%s", STORE_DIR, filename);
    strncpy(line,filename,LINE_SIZE);
    logfp = fopen(line, "r");
    if (logfp == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", line);
        return false;
    }
    LOG_PRINT(verbose_e, "opened '%s'\n", line);
    
    int rownb = 0;
    int colfilternb = 0;
    int colnb = 0;
    while (rownb < LINE_BUFFER_SIZE && fgets(line, LINE_SIZE, logfp) != NULL)
    {
        LOG_PRINT(verbose_e, "LINE %s\n", line);

        ui->tableWidget->insertRow(rownb);
        
        colnb = 0;
        colfilternb = 0;

        p = strtok_csv(line, SEPARATOR, &r);
        if (p != NULL)
        {
            do
            {
                /* tag */
                if (actual_filter[colfilternb] == true)
                {
                    if (rownb > 0)
                    {
                        QTableWidgetItem * item = new QTableWidgetItem(p);
                        if (item == NULL)
                        {
                            LOG_PRINT(error_e, "NO MEMORY\n");
                        }
                        ui->tableWidget->setItem(rownb - 1,colnb,item);
                    }
#if 0
                    else
                    {
                        if (headerList[colfilternb].compare(p) != 0)
                        {
                            LOG_PRINT(error_e, "ERROR LOG INCONGRUENTI. %d %s vs %s\n", colfilternb, headerList[colfilternb].toAscii().data(), p);
                        }
//                        ui->tableWidget->setHorizontalHeaderItem(colnb, item);
                    }
#endif
                    LOG_PRINT(verbose_e, "showing tag %d '%s' actual_filter[%d] = %d\n", colnb, p, colfilternb, actual_filter[colfilternb]);
                    colnb++;
                }
                else
                {
                    LOG_PRINT(verbose_e, "filtererd tag %d '%s' actual_filter[%d] = %d\n", colnb, p, colfilternb, actual_filter[colfilternb]);
                }
                colfilternb++;
            }
            while ((p = strtok_csv(NULL, SEPARATOR, &r)) != NULL);
        }
        if (colnb != ui->tableWidget->columnCount())
        {
            LOG_PRINT(verbose_e, "Missing variables %d vs %d\n", colnb, ui->tableWidget->columnCount());
            QTableWidgetItem * item = new QTableWidgetItem(UNDEFINED);
            if (item == NULL)
            {
                LOG_PRINT(error_e, "NO MEMORY\n");
            }
            ui->tableWidget->setItem(rownb - 1,colnb,item);
        }
        if (colnb > 0)
        {
            LOG_PRINT(verbose_e, "ROW %d\n", rownb);
            rownb++;
        }
    }
    if (feof(logfp))
    {
        fclose(logfp);
        logfp = NULL;
    }
    
    for (int i = ui->tableWidget->rowCount() - 1; i >= rownb - 1; i--)
    {
        ui->tableWidget->removeRow(i);
    }

#if 0
    for (int i = ui->tableWidget->columnCount() - 1; i >= colnb; i--)
    {
        ui->tableWidget->removeColumn(i);
    }
#endif
    if (rownb <= 0)
    {
        LOG_PRINT(error_e, "Invalid format [ROW %d COLUMN %d]\n", rownb, colnb);
        ui->labelName->setText(QString("invalid format"));
        ui->labelName->repaint();
    }

    //ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    //ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    //ui->tableWidget->resizeColumnToContents(0);
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    if (rownb > 0 && ui->tableWidget->columnCount() > 2)
    {
        LOG_PRINT(verbose_e, "DONE ROW %d COLUMN %d\n", rownb, colnb);
        return true;
    }
    else
    {
        LOG_PRINT(verbose_e, "EMPTY FILE ROW %d COLUMN %d\n", rownb, colnb);
        return true;
    }
}

bool store::readLine()
{
    char line[LINE_SIZE] = "";
    char * p = NULL, * r = NULL;
    QTableWidgetItem * item;
    
    if (logfp == NULL)
    {
        return false;
    }
    
    if (fgets(line, LINE_SIZE, logfp) != NULL)
    {
        LOG_PRINT(verbose_e, "LINE %s\n", line);
        int colnb = 0;
        int colfilternb = 0;
        
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        p = strtok_csv(line, SEPARATOR, &r);
        if (p != NULL)
        {
            do
            {
                /* tag */
                if (actual_filter[colfilternb] == true)
                {
                    item = new QTableWidgetItem(p);
                    if (item == NULL)
                    {
                        LOG_PRINT(error_e, "NO MEMORY\n");
                    }
                    ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1,colnb,item);

                    LOG_PRINT(verbose_e, "showing tag %d '%s' actual_filter[%d] = %d\n", colnb, p, colfilternb, actual_filter[colfilternb]);
                    colnb++;
                }
                else
                {
                    LOG_PRINT(verbose_e, "filtererd tag %d '%s' actual_filter[%d] = %d\n", colnb, p, colfilternb, actual_filter[colfilternb]);
                }
                colfilternb++;
            }
            while ((p = strtok_csv(NULL, SEPARATOR, &r)) != NULL);
            if (colnb != ui->tableWidget->columnCount())
            {
                LOG_PRINT(verbose_e, "Missing variables %d vs %d\n", colnb, ui->tableWidget->columnCount());
                QTableWidgetItem * item = new QTableWidgetItem(UNDEFINED);
                if (item == NULL)
                {
                    LOG_PRINT(error_e, "NO MEMORY\n");
                }
                ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1,colnb,item);
            }
        }
        LOG_PRINT(verbose_e, "ROW %d\n", ui->tableWidget->rowCount() - 1);
        //ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        return true;
    }
    else
    {
        fclose(logfp);
        logfp = NULL;
        return false;
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void store::updateData()
{

    if (status == 1)
    {
        status = 0;
        LOG_PRINT(verbose_e, "Loading '%s' ...\n", _actual_store_);
        ui->labelFilter->setText(QString("Loading '%1' ...").arg(_actual_store_));
        ui->labelFilter->repaint();
        ui->labelName->setText(_actual_store_);
        ui->labelName->repaint();
        if (logfp != NULL)
        {
            fclose(logfp);
            logfp = NULL;
        }

        QStringList fileList = QDir(STORE_DIR).entryList(QStringList("*.log"), QDir::Files|QDir::NoDotAndDotDot, QDir::Reversed);
        if (fileList.count() == 0)
        {
            ui->labelFilter->setText(trUtf8("No log to show."));
            ui->labelFilter->repaint();
            ui->labelName->setText("");
            ui->labelName->repaint();
            return;
        }

        char filename[FILENAME_MAX];
        if (_actual_store_[0] == '\0')
        {
            filename[0] = '\0';
        }
        else if (QFileInfo(_actual_store_).suffix().compare("csv") == 0)
        {
            sprintf(filename, "%s/%s", CUSTOM_STORE_DIR, _actual_store_);
        }
        else
        {
            sprintf(filename, "%s/%s.csv", CUSTOM_STORE_DIR, _actual_store_);
        }

        if (!QFile::exists(filename))
        {
            filename[0] = '\0';
        }

        /* load the actual store filter  */
        if (LoadStoreFilter(filename) == false)
        {
            QMessageBox::critical(this,trUtf8("Malformed view"), trUtf8("The view '%1' is malformed.").arg(_actual_store_));
            ui->labelName->setText("");
            ui->labelName->repaint();
            ui->labelFilter->setText("");
            ui->labelFilter->repaint();
            go_back();
            return;
        }
        LOG_PRINT(verbose_e, "_actual_store_ %s\n", _actual_store_);

        /* get the file list */
        QDir logDir(STORE_DIR);
        logFileList = logDir.entryList(QDir::Files|QDir::NoDotAndDotDot, QDir::Reversed);

        _current = 0;
        /* no logfile found */
        _file_nb = logFileList.count();
        if (_file_nb == 0)
        {
            LOG_PRINT(verbose_e, "No file to load\n");
            ui->labelFilter->setText(trUtf8("No log to show."));
            ui->labelFilter->repaint();
            ui->labelName->setText("");
            ui->labelName->repaint();
            return;
        }

        /* load the current log file */
        LOG_PRINT(verbose_e, "_current %d\n",_current);

        /* load the actual daily store */
        if (LoadStore(StoreInit, StoreFinal) == false)
        {
            QMessageBox::critical(this,trUtf8("Loading problem"), trUtf8("Cannot load the log %1.").arg(_actual_store_));
            LOG_PRINT(error_e, "cannot load store, force back\n");
            ui->labelName->setText("");
            ui->labelName->repaint();
            ui->labelFilter->setText("");
            ui->labelFilter->repaint();
            go_back();
            return;
        }

        ui->labelFilter->setText(QString("Filter: [%1 - %2]")
                                 .arg(StoreInit.toString("yyyy/MM/dd hh:mm:ss"))
                                 .arg(StoreFinal.toString("yyyy/MM/dd hh:mm:ss")));
        ui->labelFilter->repaint();

        /* set the arrow button status in funtion of the visible items */
        ui->pushButtonLeft->setEnabled(current_column > 0);
        ui->pushButtonRight->setEnabled(current_column < sizeof_filter);
        ui->pushButtonUp->setEnabled(current_row > 0);
        ui->pushButtonDown->setEnabled(logfp != NULL || (current_row < ui->tableWidget->rowCount()));
    }
    if (abs(ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row, 0)).x()) + ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row, ui->tableWidget->columnCount() - 1)).x() + ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row, ui->tableWidget->columnCount() - 1)).width() < ui->tableWidget->width())
    {
        ui->pushButtonLeft->setEnabled(false);
        ui->pushButtonRight->setEnabled(false);
    }
    ui->pushButtonSaveUSB->setEnabled(USBCheck());
    
    page::updateData();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void store::changeEvent(QEvent * event)
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
store::~store()
{
    delete ui;
}

void store::on_pushButtonHome_clicked()
{
    go_home();
}

void store::on_pushButtonBack_clicked()
{
    go_back();
}

void store::on_pushButtonUp_clicked()
{
    current_row--;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (current_row > 0)
    {
        ui->pushButtonDown->setEnabled(true);
    }
    else
    {
        ui->pushButtonUp->setEnabled(false);
    }
}

void store::on_pushButtonDown_clicked()
{
    LOG_PRINT(verbose_e, "current_row %d rowCount %d logfp %p\n", current_row, ui->tableWidget->rowCount(), logfp);
    
    if (current_row >= ui->tableWidget->rowCount() - 2 && logfp != NULL)
    {
        LOG_PRINT(verbose_e, "Reading a new line current_row %d rowCount %d\n", current_row, ui->tableWidget->rowCount());
        readLine();
    }
    
    current_row++;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (current_row < ui->tableWidget->rowCount() - 1)
    {
        ui->pushButtonUp->setEnabled(true);
    }
    else
    {
        ui->pushButtonDown->setEnabled(logfp != NULL);
    }
}

void store::on_pushButtonLeft_clicked()
{
    current_column--;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,0)).x() < 0)
    {
        ui->pushButtonLeft->setEnabled(true);
    }
    else
    {
        ui->pushButtonLeft->setEnabled(false);
    }
    if (
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).x() +
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).width() - ui->tableWidget->width() > 0
            )
    {
        ui->pushButtonRight->setEnabled(true);
    }
    else
    {
        ui->pushButtonRight->setEnabled(false);
    }
}

void store::on_pushButtonRight_clicked()
{
    current_column++;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

    if (ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,0)).x() < 0)
    {
        ui->pushButtonLeft->setEnabled(true);
    }
    else
    {
        ui->pushButtonLeft->setEnabled(false);
    }
    if (
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).x() +
            ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row,ui->tableWidget->columnCount() - 1)).width() - ui->tableWidget->width() > 0
            )
    {
        ui->pushButtonRight->setEnabled(true);
    }
    else
    {
        ui->pushButtonRight->setEnabled(false);
    }
}

#if 0
void store::on_pushButtonPrevious_clicked()
{
    if (_file_nb == 0) return;
    
    LOG_PRINT(verbose_e, "_current %d\n", _current);
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

void store::on_pushButtonNext_clicked()
{
    if (_file_nb == 0) return;
    LOG_PRINT(verbose_e, "_current %d\n", _current);
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
#endif

void store::on_pushButtonSaveUSB_clicked()
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
        
        char srcfilename[FILENAME_MAX];
        char dstfilename[FILENAME_MAX];
        /* compose the source file name and the destination file name */
        sprintf(srcfilename, "%s/%s", TMPDIR, outputfile);
        if (strlen(_actual_store_))
        {
            sprintf(dstfilename, "%s/%s_%s", TMPDIR, QFileInfo(_actual_store_).baseName().toAscii().data(), outputfile);
        }
        else
        {
            sprintf(dstfilename, "%s/%s", TMPDIR, outputfile);
        }

        /* extract only the selected column */
        /* open file */
        FILE * srcfp = fopen(srcfilename, "r");
        if (srcfp == NULL)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot extract the log '%1'").arg(srcfilename));
            USBumount();
            return;
        }
        FILE * dstfp = fopen(dstfilename, "w");
        if (dstfp == NULL)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot extract the log '%1'").arg(dstfilename));
            USBumount();
            fclose(srcfp);
            return;
        }
        char line [STR_LEN];
        while (fgets(line, LINE_SIZE, srcfp) != NULL)
        {
            QStringList fields = QString(line).simplified().replace(QString(" "), QString("")).split(SEPARATOR);

            for (int i = 0; i <= sizeof_filter && i < fields.count(); i++)
            {
                if (actual_filter[i] == true)
                {
                    if (i > 0)
                    {
                        fprintf(dstfp, "; ");
                    }
                    fprintf(dstfp, "%s", fields.at(i).toAscii().data());
                }
            }
            fprintf(dstfp, "\n");
        }
        fclose (srcfp);
        fclose (dstfp);

        strcpy(srcfilename, dstfilename);
        if (strlen(_actual_store_))
        {
            sprintf(dstfilename, "%s/%s_%s.zip",
                    usb_mnt_point,
                    QFileInfo(_actual_store_).baseName().toAscii().data(),
                    outputfile);
        }
        else
        {
            sprintf(dstfilename, "%s/%s.zip",
                    usb_mnt_point,
                    outputfile);
        }

        if (signFile(srcfilename, QString("%1.sign").arg(srcfilename)) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1.sign'").arg(srcfilename));
            USBumount();
            return;
        }

        /* zip the file, the sign file and delete them */
        if (zipAndSave(QStringList() << srcfilename << QString("%1.sign").arg(srcfilename), QString(dstfilename), true) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the zip file '%1'").arg(dstfilename));
            USBumount();
            return;
        }
        
        QFile::remove(srcfilename);
        QFile::remove(QString("%1.sign").arg(srcfilename));
        
        /* unmount USB key */
        USBumount();
        LOG_PRINT(verbose_e, "DOWNLOADED\n");
        QMessageBox::information(this,trUtf8("USB info"), trUtf8("File '%1' saved.").arg(QFileInfo(dstfilename).fileName()));
    }
}

void store::on_pushButtonFilter_clicked()
{
    goto_page("store_filter");
}
