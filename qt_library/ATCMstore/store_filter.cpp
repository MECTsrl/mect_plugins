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


/**
 * @brief this macro is used to set the STORE_FILTER style.
 * the syntax is html stylesheet-like
 */
#define SET_STORE_FILTER_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    /* add there the page stilesheet customization */ \
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
    filterStart = QDateTime::fromString(QString(StoreInit), "yyyy/MM/dd_HH:mm:ss");
    filterEnd = QDateTime::fromString(QString(StoreFinal), "yyyy/MM/dd_HH:mm:ss");

    if (not filterStart.isValid())      filterStart = QDateTime(QDate::currentDate(), QTime(0,0,0));
    if (not filterEnd.isValid())        filterEnd   = QDateTime(QDate::currentDate(), QTime(23,59,0));

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

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void store_filter::changeEvent(QEvent * event)
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
//    sprintf(StoreInit, "%d/%02d/%02d_%02d:%02d:%02d",
//            ui->spinBoxYearIn->value(),
//            ui->spinBoxMonthIn->value(),
//            ui->spinBoxDayIn->value(),
//            ui->spinBoxHoursIn->value(),
//            ui->spinBoxMinutesIn->value(),
//            ui->spinBoxSecondsIn->value()
//            );
//    sprintf(StoreFinal, "%d/%02d/%02d_%02d:%02d:%02d",
//            ui->spinBoxYearFin->value(),
//            ui->spinBoxMonthFin->value(),
//            ui->spinBoxDayFin->value(),
//            ui->spinBoxHoursFin->value(),
//            ui->spinBoxMinutesFin->value(),
//            ui->spinBoxSecondsFin->value()
//            );
    goto_page("store",false);
}

void store_filter::on_pushButtonReset_clicked()
{
    strcpy(StoreInit, QDateTime(QDate::currentDate(), QTime(0,0,0)).toString("yyyy/MM/dd_HH:mm:ss").toAscii().data());
    strcpy(StoreFinal, QDateTime(QDate::currentDate(), QTime(23,59,59)).toString("yyyy/MM/dd_HH:mm:ss").toAscii().data());
    reload();
}

void store_filter::on_pushButtonDateStart_clicked()
{
    if (calendarpopup) {
        QDate d = filterStart.date();
        if (d.isValid()) {
            calendarpopup->setDate(d);
            calendarpopup->setModal(true);
            calendarpopup->movePosition(ui->pushButtonDateStart->geometry().x(),ui->pushButtonDateStart->geometry().y());
            if (calendarpopup->exec() == QDialog::Accepted) {
                ui->pushButtonDateStart->setText(calendarpopup->getDate().toString(DATE_MASK));
            }
        }
    }
}

void store_filter::on_pushButtonDateEnd_clicked()
{
    if (calendarpopup) {
        QDate d = filterEnd.date();
        if (d.isValid()) {
            calendarpopup->setDate(d);
            calendarpopup->setModal(true);
            calendarpopup->movePosition(ui->pushButtonDateEnd->geometry().x(),ui->pushButtonDateEnd->geometry().y());
            if (calendarpopup->exec() == QDialog::Accepted) {
                ui->pushButtonDateEnd->setText(calendarpopup->getDate().toString(DATE_MASK));
            }
        }
    }
}

void store_filter::on_pushButtonTimeStart_clicked()
{
    if (timepopup) {
        QTime t = filterStart.time();
        if (t.isValid()) {
            timepopup->setTime(t);
            timepopup->setModal(true);
            timepopup->movePosition(ui->pushButtonTimeStart->geometry().x(),ui->pushButtonTimeStart->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                // ui->timeEdit->setTime(timepop->getTime());
                ui->pushButtonTimeStart->setText(timepopup->getTime().toString(TIME_MASK));
            }
        }
    }
}

void store_filter::on_pushButtonTimeEnd_clicked()
{
    if (timepopup) {
        QTime t = filterEnd.time();
        if (t.isValid()) {
            timepopup->setTime(t);
            timepopup->setModal(true);
            timepopup->movePosition(ui->pushButtonTimeEnd->geometry().x(),ui->pushButtonTimeEnd->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                // ui->timeEdit->setTime(timepop->getTime());
                ui->pushButtonTimeEnd->setText(timepopup->getTime().toString(TIME_MASK));
            }
        }
    }
}
