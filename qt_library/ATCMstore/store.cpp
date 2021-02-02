/**
 * @file
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
#include <QWSServer>
#include <QCoreApplication>

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
    fpin = NULL;
    
    /* set as default current day */
    strcpy(StoreInit, QDateTime(QDate::currentDate(), QTime(0,0,0)).toString("yyyy/MM/dd_HH:mm:ss").toAscii().data());
    strcpy(StoreFinal, QDateTime(QDate::currentDate(), QTime(23,59,59)).toString("yyyy/MM/dd_HH:mm:ss").toAscii().data());
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

void store::showLogHeder()
{
    current_row = 0;
    current_column = 0;

    for (int i = 0; i < filterHeaderSize; i++)
    {
            ui->tableWidget->insertColumn(i);
            LOG_PRINT(verbose_e, "Inserted column '%d' '%s'\n", i, filterHeader[i]);
            ui->tableWidget->setHorizontalHeaderItem(i, new QTableWidgetItem(filterHeader[i]));
    }
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->update();
}

void store::showLogRead(char ** outstruct)
{
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
    QTableWidgetItem * item;
    for (int colnb = 0; colnb < filterHeaderSize; colnb++)
    {
        item = new QTableWidgetItem(outstruct[colnb]);
        if (item == NULL)
        {
            LOG_PRINT(error_e, "NO MEMORY\n");
        }
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, colnb, item);
    }
    ui->tableWidget->update();
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

        struct tm tmp_ti, tmp_tf;
        fpin = NULL;

        /* extract the time_t from the date string */
        strptime (StoreInit, "%Y/%m/%d_%H:%M:%S", &tmp_ti);
        tmp_ti.tm_isdst = 0;
        ti = mktime(&tmp_ti);
        strptime (StoreFinal, "%Y/%m/%d_%H:%M:%S", &tmp_tf);
        tmp_tf.tm_isdst = 0;
        tf = mktime(&tmp_tf);

        LOG_PRINT(verbose_e, "log from %s to %s\n", StoreInit, StoreFinal);

        char filename[FILENAME_MAX];
        if (_actual_store_[0] == '\0' || strcmp(_actual_store_, "store") == 0)
        {
            filename[0] = '\0';
            _actual_store_[0] = '\0';
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
        // Clear Table
        ui->tableWidget->clear();
        ui->tableWidget->setRowCount(0);
        ui->tableWidget->setColumnCount(0);
        //        while(ui->tableWidget->rowCount() > 0)
        //        {
        //            ui->tableWidget->removeRow(0);
        //        }
        //        while(ui->tableWidget->columnCount() > 0)
        //        {
        //            ui->tableWidget->removeColumn(0);
        //        }
        ui->tableWidget->update();
        ui->tableWidget->repaint();

        // datein timein storefile
        if (initLogRead(STORE_DIR, filename, ti, tf, &fpin) != 0)
        {
            LOG_PRINT(verbose_e, "no data to show\n");
            ui->labelFilter->setText(trUtf8("no data to show"));
            ui->labelFilter->repaint();
            return;
        }

        showLogHeder();

        int i = 0;
        int retval = 0;
        while (i < LINE_BUFFER_SIZE)
        {
            this->update();
            retval = getLogRead(STORE_DIR, ti, tf, &fpin, outstruct);
            if(retval < 0)
            {
                break;
            }
            else if (retval == 0)
            {
                showLogRead(outstruct);
                i++;
            }
            QCoreApplication::processEvents();
        }

        ui->labelFilter->setText(QString("Filter: [%1 - %2]")
                                 .arg(StoreInit)
                                 .arg(StoreFinal));
        ui->labelFilter->repaint();

#ifdef AUTO_DISABLE_ARROW
        /* set the arrow button status in funtion of the visible items */
        ui->pushButtonLeft->setEnabled(current_column > 0);
        ui->pushButtonRight->setEnabled(current_column < sizeof_filter);
        ui->pushButtonUp->setEnabled(current_row > 0);
        ui->pushButtonDown->setEnabled(logfp != NULL || (current_row < ui->tableWidget->rowCount()));
#endif
    }
#ifdef AUTO_DISABLE_ARROW
    if (abs(ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row, 0)).x()) + ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row, ui->tableWidget->columnCount() - 1)).x() + ui->tableWidget->visualItemRect(ui->tableWidget->item(current_row, ui->tableWidget->columnCount() - 1)).width() < ui->tableWidget->width())
    {
        ui->pushButtonLeft->setEnabled(false);
        ui->pushButtonRight->setEnabled(false);
    }
#endif
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

#ifdef AUTO_DISABLE_ARROW
    if (current_row > 0)
    {
        ui->pushButtonDown->setEnabled(true);
    }
    else
    {
        ui->pushButtonUp->setEnabled(false);
    }
#endif
}

void store::on_pushButtonDown_clicked()
{
    LOG_PRINT(verbose_e, "current_row %d rowCount %d\n", current_row, ui->tableWidget->rowCount());
    
    if (current_row >= ui->tableWidget->rowCount() - 2)
    {
        LOG_PRINT(verbose_e, "Reading a new line current_row %d rowCount %d\n", current_row, ui->tableWidget->rowCount());

        int retval = 0;
        while ( (retval = getLogRead(STORE_DIR, ti, tf, &fpin, outstruct)) > 0);
        if (retval == 0)
        {
            showLogRead(outstruct);
        }
    }
    
    current_row++;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
    if (current_row < ui->tableWidget->rowCount() - 1)
    {
        ui->pushButtonUp->setEnabled(true);
    }
    else
    {
        ui->pushButtonDown->setEnabled(logfp != NULL);
    }
#endif
}

void store::on_pushButtonLeft_clicked()
{
    current_column--;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
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
#endif
}

void store::on_pushButtonRight_clicked()
{
    current_column++;
    current_row = (current_row < ui->tableWidget->rowCount()) ? current_row : ui->tableWidget->rowCount() - 1;
    current_column = (current_column < ui->tableWidget->columnCount()) ? current_column : ui->tableWidget->columnCount() - 1;
    current_row = (current_row > 0) ? current_row : 0;
    current_column = (current_column > 0) ? current_column : 0;
    ui->tableWidget->scrollToItem(ui->tableWidget->item(current_row,current_column),QAbstractItemView::PositionAtCenter);

#ifdef AUTO_DISABLE_ARROW
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
#endif
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
        int retval = 0;
        struct tm tmp_ti, tmp_tf;
        char srcfilename[FILENAME_MAX];
        char dstfilename[FILENAME_MAX];
        char outFileName[FILENAME_MAX];
        FILE * fpout;
        char * p = NULL;

        QWSServer::setCursorVisible(true);
        QApplication::setOverrideCursor(Qt::WaitCursor);
        ui->pushButtonSaveUSB->setEnabled(false);
        this->update();

        if (USBmount() == false)
        {
            LOG_PRINT(error_e, "cannot mount the usb key\n");
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot mount the usb key"));
            goto umount_and_exit;
        }

        if (strlen(_actual_store_))
        {
            sprintf(outFileName, "%s_%s-%s.log",
                    QFileInfo(_actual_store_).baseName().toAscii().data(),
                    StoreInit,
                    StoreFinal
                    );
        }
        else
        {
            sprintf(outFileName, "%s-%s.log",
                    StoreInit,
                    StoreFinal
                    );
        }

        while((p = strchr(outFileName, '/')))
        {
            *p = '_';
        }
        while((p = strchr(outFileName, ':')))
        {
            *p = '_';
        }

        sprintf(srcfilename, "%s/%s", usb_mnt_point, outFileName);

        LOG_PRINT(verbose_e, "srcfilename: %s\n", srcfilename);
        fpout = fopen(srcfilename, "w");
        fpin = NULL;
        if (fpout == NULL)
        {
            LOG_PRINT(error_e, "Cannot open '%s'. [%s]\n", srcfilename, strerror(errno));
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(QFileInfo(srcfilename).fileName()));
            goto umount_and_exit;
        }

        /* extract the time_t from the date string */
        strptime (StoreInit, "%Y/%m/%d_%H:%M:%S", &tmp_ti);
        tmp_ti.tm_isdst = 0;
        ti = mktime(&tmp_ti);
        strptime (StoreFinal, "%Y/%m/%d_%H:%M:%S", &tmp_tf);
        tmp_tf.tm_isdst = 0;
        tf = mktime(&tmp_tf);

        char filename[FILENAME_MAX];
        if (_actual_store_[0] == '\0' || strcmp(_actual_store_, "store") == 0)
        {
            filename[0] = '\0';
            _actual_store_[0] = '\0';
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

        // datein timein storefile
        if (initLogRead(STORE_DIR, filename, ti, tf, &fpin) != 0)
        {
            LOG_PRINT(error_e, "\n");
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(_actual_store_));
            goto umount_and_exit;
        }

        if (dumpLogHeder(fpout) != 0)
        {
            LOG_PRINT(error_e, "\n");
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(QFileInfo(srcfilename).fileName()));
            goto umount_and_exit;
        }

        while ( (retval = getLogRead(STORE_DIR, ti, tf, &fpin, outstruct)) >= 0)
        {
            QCoreApplication::processEvents();
            if (retval == 0)
            {
                if (dumpLogRead(fpout, outstruct) != 0)
                {
                    LOG_PRINT(error_e, "\n");
                    QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(QFileInfo(srcfilename).fileName()));
                    goto umount_and_exit;
                }
            }
            this->update();
        }
        fclose(fpout);

        sprintf(dstfilename, "%s.zip", srcfilename);

        if (signFile(srcfilename, QString("%1.sign").arg(srcfilename)) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the signature '%1.sign'").arg(srcfilename));
            goto umount_and_exit;
        }

        /* zip the file, the sign file and delete them */
        if (zipAndSave(QStringList() << srcfilename << QString("%1.sign").arg(srcfilename), QString(dstfilename), true) == false)
        {
            QMessageBox::critical(this,trUtf8("USB error"), trUtf8("Cannot create the zip file '%1'").arg(dstfilename));
            goto umount_and_exit;
        }
        
        QFile::remove(srcfilename);
        QFile::remove(QString("%1.sign").arg(srcfilename));
        
        LOG_PRINT(verbose_e, "DOWNLOADED\n");
        QMessageBox::information(this,trUtf8("USB info"), trUtf8("File '%1' saved.").arg(QFileInfo(dstfilename).fileName()));

umount_and_exit:
        /* unmount USB key */
        USBumount();
        ui->pushButtonSaveUSB->setEnabled(true);
        QApplication::restoreOverrideCursor();
        QWSServer::setCursorVisible(false);
        update();
        return;
    }
}

void store::on_pushButtonFilter_clicked()
{
    goto_page("store_filter", false);
}
