/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Generic page
 */

#include <QDate>
#include <QTime>
#include <QMessageBox>

#include "calendar.h"
#include "timepopup.h"

#include "app_logprint.h"
#include "store_filter.h"
#include "ui_store_filter.h"

/* this define set the variables list to be to displayed in this page */
#undef VAR_TO_DISPLAY
/* Example:
#define VAR_TO_DISPLAY "RET_REG_1" << "RET_REG_3" << "NRE_REG_1"
 */
/* this define set the window title */
#define WINDOW_TITLE ""
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

#define TIME_MASK "HH:mm:ss"
#define DATE_MASK "yyyy-MM-dd"
#define DATETIME_MASK "yyyy/MM/dd_HH:mm:ss"


/**
 * @brief this macro is used to set the STORE_FILTER style.
 * the syntax is html stylesheet-like
 */
#define SET_STORE_FILTER_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    /* add there the page stylesheet customization */ \
    mystyle.append(""); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
store_filter::store_filter(QWidget *parent) :
    page(parent),
    ui(new Ui::store_filter)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    
    /* set up the page style */
    //setStyle::set(this);
    /* set the style described into the macro SET_STORE_FILTER_STYLE */
    SET_STORE_FILTER_STYLE();    
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void store_filter::reload()
{
    // Init dei tempi di riferimento
    filterStart = QDateTime::fromString(QString(StoreInit), "yyyy/MM/dd_hh:mm:ss");     // Caution: "yyyy/MM/dd_HH:mm:ss" is only for .toSting()...
    filterEnd = QDateTime::fromString(QString(StoreFinal), "yyyy/MM/dd_hh:mm:ss");

    // Check Dates
    if (not filterStart.isValid())  {
        filterStart = QDateTime(QDate::currentDate(), QTime(0,0,0));
    }
    if (not filterEnd.isValid())  {
        filterEnd = QDateTime(QDate::currentDate(), QTime(23,59,0));
    }

    ui->pushButtonTimeStart->setText(filterStart.time().toString(TIME_MASK));
    ui->pushButtonDateStart->setText(filterStart.date().toString(DATE_MASK));

    ui->pushButtonTimeEnd->setText(filterEnd.time().toString(TIME_MASK));
    ui->pushButtonDateEnd->setText(filterEnd.date().toString(DATE_MASK));
}


/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void store_filter::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }
    /* call the parent updateData member */
    page::updateData();
}

void store_filter::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
store_filter::~store_filter()
{
    delete ui;
}

#undef VAR_TO_DISPLAY

void store_filter::on_pushButtonCancel_clicked()
{
    goto_page("store",false);
}

void store_filter::on_pushButtonOk_clicked()
{
    // Check Date Range
    if (not filterStart.isValid() ||
        not filterEnd.isValid() ||
        filterStart > filterEnd)  {
        // Invalid Date Range
        QMessageBox::warning(this, trUtf8("Date Filter Range"), trUtf8("Invalid Date/Time Range!"));

    }
    else  {
        strcpy(StoreInit, filterStart.toString(DATETIME_MASK).toAscii().data());
        strcpy(StoreFinal, filterEnd.toString(DATETIME_MASK).toAscii().data());
        goto_page("store",false);
    }
}

void store_filter::on_pushButtonReset_clicked()
{
    strcpy(StoreInit, QDateTime(QDate::currentDate(), QTime(0,0,0)).toString(DATETIME_MASK).toAscii().data());
    strcpy(StoreFinal, QDateTime(QDate::currentDate(), QTime(23,59,59)).toString(DATETIME_MASK).toAscii().data());
    reload();
}

void store_filter::on_pushButtonDateStart_clicked()
{
    if (calendarpopup) {
        QDate d = filterStart.date();
        QTime t = filterStart.time();
        if (d.isValid()) {
            calendarpopup->setDate(d);
            calendarpopup->setModal(true);
            calendarpopup->movePosition(ui->pushButtonDateStart->geometry().x(),ui->pushButtonDateStart->geometry().y());
            if (calendarpopup->exec() == QDialog::Accepted) {
                d = calendarpopup->getDate();
                ui->pushButtonDateStart->setText(d.toString(DATE_MASK));
                filterStart = QDateTime(d, t);
            }
        }
    }
}

void store_filter::on_pushButtonDateEnd_clicked()
{
    if (calendarpopup) {
        QDate d = filterEnd.date();
        QTime t = filterEnd.time();
        if (d.isValid()) {
            calendarpopup->setDate(d);
            calendarpopup->setModal(true);
            calendarpopup->movePosition(ui->pushButtonDateEnd->geometry().x(),ui->pushButtonDateEnd->geometry().y());
            if (calendarpopup->exec() == QDialog::Accepted) {
                d = calendarpopup->getDate();
                ui->pushButtonDateEnd->setText(d.toString(DATE_MASK));
                filterEnd = QDateTime(d, t);
            }
        }
    }
}

void store_filter::on_pushButtonTimeStart_clicked()
{
    if (timepopup) {
        QDate d = filterStart.date();
        QTime t = filterStart.time();
        if (t.isValid()) {
            timepopup->setTime(t);
            timepopup->setModal(true);
            timepopup->movePosition(ui->pushButtonTimeStart->geometry().x(),ui->pushButtonTimeStart->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                t = timepopup->getTime();
                ui->pushButtonTimeStart->setText(t.toString(TIME_MASK));
                filterStart = QDateTime(d, t);
            }
        }
    }
}

void store_filter::on_pushButtonTimeEnd_clicked()
{
    if (timepopup) {
        QDate d = filterEnd.date();
        QTime t = filterEnd.time();
        if (t.isValid()) {
            timepopup->setTime(t);
            timepopup->setModal(true);
            timepopup->movePosition(ui->pushButtonTimeEnd->geometry().x(),ui->pushButtonTimeEnd->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                t = timepopup->getTime();
                ui->pushButtonTimeEnd->setText(t.toString(TIME_MASK));
                filterEnd = QDateTime(d, t);
            }
        }
    }
}
