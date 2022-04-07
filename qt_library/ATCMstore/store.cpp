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
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QItemSelectionModel>
#include <QStringList>
#include <QDir>
#include <errno.h>
#include <QWSServer>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFontMetrics>

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

void store::showLogHeader()
{
    QStringList list;

    ui->tableWidget->setColumnCount(filterHeaderSize);
    list.clear();
    for (int i = 0; i < filterHeaderSize; i++) {
        list.append(filterHeader[i]);
    }
    ui->tableWidget->setHorizontalHeaderLabels(list);
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->update();

    ui->tableWidget->verticalHeader()->hide();
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
    page::updateData(); // moved from the function end

    if (status == 1)
    {
        disableButtons();
        status = 0;
        fpin = NULL;
        {
            struct tm tmp_ti, tmp_tf;

            /* extract the time_t from the date string */
            strptime(StoreInit, "%Y/%m/%d_%H:%M:%S", &tmp_ti);
            strptime(StoreFinal, "%Y/%m/%d_%H:%M:%S", &tmp_tf);
            tmp_ti.tm_isdst = 0;
            tmp_tf.tm_isdst = 0;
            ti = mktime(&tmp_ti);
            tf = mktime(&tmp_tf);
        }
        ui->labelFilter->setText("Loading ...");
        ui->labelName->setText(_actual_store_);
        this->update();
        QCoreApplication::processEvents();

        // Hide and Clear Table
        ui->tableWidget->setVisible(false);
        clearTable();
        {
            char filename[FILENAME_MAX];

            if (_actual_store_[0] == '\0' || strcmp(_actual_store_, "store") == 0) {
                filename[0] = '\0';
                _actual_store_[0] = '\0';
            } else if (QFileInfo(_actual_store_).suffix().compare("csv") == 0) {
                sprintf(filename, "%s/%s", CUSTOM_STORE_DIR, _actual_store_);
            } else {
                sprintf(filename, "%s/%s.csv", CUSTOM_STORE_DIR, _actual_store_);
            }
            if (not QFile::exists(filename)) {
                filename[0] = '\0';
            }

            if (initLogRead(STORE_DIR, filename, ti, tf, &fpin) != 0){
                ui->labelFilter->setText(trUtf8("no data to show"));
                return;
            }
        }

        // intestazione tabella
        showLogHeader();

        QFontMetrics fm(ui->tableWidget->font());
        int lineHeight = fm.height() * 2;
        int maxLines = LINE_BUFFER_SIZE;
        int lines = 0;
        int retval = 0;
        QElapsedTimer timer;

        if (lineHeight > 0) {
            maxLines = ui->tableWidget->height() / lineHeight;
        }
        page::updateData(); // update datetime
        labelDataOra->update();
        ui->labelFilter->setText(ui->labelFilter->text() + ".");
        ui->labelFilter->update();
        QCoreApplication::processEvents();
        timer.restart();
        while (retval >= 0 and lines < maxLines)
        {
            // -1 : "no data to read\n""Cannot get date token\n""Cannot get time token\n" "late %ld %ld %ld\n"
            //  0 : ok riga in outstruct
            //  1 : "early\n" "done\n"
            retval = getLogRead(STORE_DIR, ti, tf, &fpin, outstruct);
            if (retval == 0) {
                showLogRead(outstruct);
                lines++;
            }
            if (timer.elapsed() >= 100) { // 0.5 s
                page::updateData(); // update datetime
                labelDataOra->update();
                ui->labelFilter->setText(ui->labelFilter->text() + ".");
                ui->labelFilter->update();
                QCoreApplication::processEvents();
                timer.restart();
                // fprintf(stderr, "---- %s\n", ui->labelFilter->text().toLatin1().data());
            }
        }
        ui->labelFilter->setText(QString("Filter: [%1 - %2]").arg(StoreInit).arg(StoreFinal));

        if (ui->tableWidget->rowCount() > 0) {
            current_row = 0;
        } else {
            current_row = -1;
        }
        if (ui->tableWidget->columnCount() > 0) {
            current_column = (ui->tableWidget->columnCount() > 2) ? 2 : 0;
        } else {
            current_column = -1;
        }
        if (current_row >= 0 && current_column >= 0) {
            gotoCell(current_row, current_column);
        }
        ui->tableWidget->setVisible(true);
    }
    ui->pushButtonSaveUSB->setEnabled(USBCheck());
    enableDisableButtons();
}

void store::disableButtons()
{
    ui->pushButtonHome->setEnabled(false);
    ui->pushButtonBack->setEnabled(false);
    ui->pushButtonFilter->setEnabled(false);

    ui->pushButtonLeft->setEnabled(false);
    ui->pushButtonRight->setEnabled(false);
    ui->pushButtonUp->setEnabled(false);
    ui->pushButtonDown->setEnabled(false);
}

void store::clearTable()
{
    // widget
    ui->tableWidget->clear();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnCount(0);
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidget->horizontalHeader()->reset();
    ui->tableWidget->setHorizontalHeaderLabels(QStringList());
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // data
    finishLogRead();
}

void store::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
store::~store()
{
    delete ui;
}

void store::on_pushButtonHome_clicked()
{
    clearTable();
    go_home();
}

void store::on_pushButtonBack_clicked()
{
    clearTable();
    go_back();
}

void store::on_pushButtonUp_clicked()
{
    current_row--;
    enableDisableButtons();
    gotoCell(current_row, current_column);
}

void store::on_pushButtonDown_clicked()
{    
    if (current_row == ui->tableWidget->rowCount() - 1) {
        int retval = 0;
        ui->pushButtonDown->setVisible(false);
        ui->pushButtonDown->update();
        while (retval >= 0) {
            retval = getLogRead(STORE_DIR, ti, tf, &fpin, outstruct);
            if (retval == 0) {
                // una nuova riga
                showLogRead(outstruct);
                current_row++;
                break;
            }
        }
        ui->pushButtonDown->setVisible(true);
        ui->pushButtonDown->update();
    } else {
        current_row++;
    }
    enableDisableButtons();
    gotoCell(current_row, current_column);
}

void store::on_pushButtonLeft_clicked()
{
    current_column--;
    enableDisableButtons();
    gotoCell(current_row, current_column);
}

void store::on_pushButtonRight_clicked()
{
    current_column++;
    enableDisableButtons();
    gotoCell(current_row, current_column);
}

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
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot mount the usb key"));
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
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(QFileInfo(srcfilename).fileName()));
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
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(_actual_store_));
            goto umount_and_exit;
        }

        if (dumpLogHeder(fpout) != 0)
        {
            LOG_PRINT(error_e, "\n");
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(QFileInfo(srcfilename).fileName()));
            goto umount_and_exit;
        }

        QElapsedTimer timer;
        retval = 0;
        timer.restart();
        while (retval >= 0)
        {
            retval = getLogRead(STORE_DIR, ti, tf, &fpin, outstruct);
            if (retval == 0)
            {
                if (dumpLogRead(fpout, outstruct) != 0)
                {
                    LOG_PRINT(error_e, "\n");
                    QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot save '%1'.").arg(QFileInfo(srcfilename).fileName()));
                    goto umount_and_exit;
                }
            }
            if (timer.elapsed() >= 500) { // 0.5 s
                QCoreApplication::processEvents();
                timer.restart();
            }
        }
        fclose(fpout);

        sprintf(dstfilename, "%s.zip", srcfilename);

        // 3.2.9 and above: Removed usage of Sign
        // if (signFile(srcfilename, QString("%1.sign").arg(srcfilename)) == false)
        // {
        //     QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot create the signature '%1.sign'").arg(srcfilename));
        //     goto umount_and_exit;
        // }

        /* zip the file, the sign file and delete them */
        // if (zipAndSave(QStringList() << srcfilename << QString("%1.sign").arg(srcfilename), QString(dstfilename), true) == false)
        if (not zipAndSave(QStringList() << srcfilename, QString(dstfilename), true))
        {
            QMessageBox::critical(0, trUtf8("USB error"), trUtf8("Cannot create the zip file '%1'").arg(dstfilename));
            goto umount_and_exit;
        }
        
        QFile::remove(srcfilename);
        // QFile::remove(QString("%1.sign").arg(srcfilename));
        
        LOG_PRINT(verbose_e, "DOWNLOADED\n");
        QMessageBox::information(0, trUtf8("USB info"), trUtf8("File '%1' saved.").arg(QFileInfo(dstfilename).fileName()));

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

void store::enableDisableButtons()
{
    ui->pushButtonHome->setEnabled(true);
    ui->pushButtonBack->setEnabled(true);
    ui->pushButtonFilter->setEnabled(true);
    // Check position
    current_column = current_column < 0 ? 0 : current_column;
    current_column = current_column > ui->tableWidget->columnCount() - 1 ? ui->tableWidget->columnCount() - 1 : current_column;
    current_row = current_row < 0 ? 0 : current_row;
    current_row = current_row > ui->tableWidget->rowCount() - 1 ? ui->tableWidget->rowCount() - 1 : current_row;
    // Movement Buttons
    ui->pushButtonLeft->setEnabled(current_column > 0);
    ui->pushButtonRight->setEnabled(current_column < ui->tableWidget->columnCount() - 1);
    ui->pushButtonUp->setEnabled(current_row > 0);
    ui->pushButtonDown->setEnabled(current_row >= 0 and current_row <= ui->tableWidget->rowCount());
}

void store::on_tableWidget_cellClicked(int row, int column)
{
    current_row = row;
    current_column = column;
    gotoCell(current_row, current_column);
}

void store::gotoCell(int nRow, int nColumn)
{
    // Deselect previous cell
    ui->tableWidget->selectionModel()->clearSelection();
    if (ui->tableWidget->rowCount() &&
        ui->tableWidget->columnCount() &&
        nRow >= 0 && nRow < ui->tableWidget->rowCount() &&
        nColumn >= 0 && nColumn < ui->tableWidget->columnCount())  {
        // Goto Cell
        ui->tableWidget->scrollToItem(ui->tableWidget->item(nRow, nColumn));
        // Select Cell
        ui->tableWidget->selectionModel()->select(ui->tableWidget->model()->index(nRow, nColumn), QItemSelectionModel::ClearAndSelect);
    }
}

