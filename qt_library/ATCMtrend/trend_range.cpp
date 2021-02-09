/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Generic page
 */
#include "app_logprint.h"
#include "item_selector.h"
#include "trend_range.h"
#include "ui_trend_range.h"
#include "timepopup.h"
#include "calendar.h"

#define DEFAULT_YMIN -1000
#define DEFAULT_YMAX  1000

/* this define set the window title */
#define WINDOW_TITLE "TREND RANGE"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the TREND_OPTION style.
 * the syntax is html stylesheet-like
 */
#define SET_TREND_OPTION_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
trend_range::trend_range(QWidget *parent) :
    page(parent),
    ui(new Ui::trend_range)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    
    /* if exist and is not empty enable the WINDOW_ICON */
    if (strlen(WINDOW_ICON) > 0)
    {
        /* enable this line, define the WINDOW_ICON and put a Qlabel named labelIcon in your ui file if you want have a window icon */
        //labelIcon = ui->labelIcon;
    }
    /* if exist and is not empty enable the WINDOW_TITLE */
    if (strlen(WINDOW_TITLE) > 0)
    {
        /* enable this line, define the WINDOW_TITLE and put a Qlabel named labelTitle in your ui file if you want have a window title */
        //labelTitle = ui->labelTitle;
    }
    
    //setStyle::set(this);
    SET_TREND_OPTION_STYLE();
    
    /* connect the label that show the date and the time to the timer of the parent updateData */
    //labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;

    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void trend_range::reload()
{
    /* initial time */

#if 0
    ui->dateEdit->setDate(actualTzero.date());
    ui->spinBoxHoursIn->setValue(actualTzero.time().hour());
    ui->spinBoxMinutesIn->setValue(actualTzero.time().minute());
    ui->spinBoxSecondsIn->setValue(actualTzero.time().second());
#else
    ui->pushButtonTime->setText(actualTzero.time().toString("HH:mm:ss"));
    ui->pushButtonCalendar->setText(QDateTime::currentDateTime().date().toString("yyyy-MM-dd"));
#endif

    /* interval */
    QDateTime reference = QDateTime(QDate(0,0,0), QTime(0,0,0)).addSecs(actualVisibleWindowSec);
    ui->spinBoxDays->setMaximum(90);
    ui->spinBoxDays->setValue(   reference.date().day() - 1);
#if 0
    ui->spinBoxHours->setMaximum(23);
    ui->spinBoxMinutes->setMaximum(59);
    ui->spinBoxSeconds->setMaximum(59);
    ui->spinBoxHours->setValue(  reference.time().hour());
    ui->spinBoxMinutes->setValue(reference.time().minute());
    ui->spinBoxSeconds->setValue(reference.time().second());
#else
    ui->pushButtonWidth->setText(reference.time().toString("HH:mm:ss"));
#endif

    /* Y range */
    ui->doubleSpinBoxYmin->setValue(pens[actualPen].yMinActual);
    ui->doubleSpinBoxYmax->setValue(pens[actualPen].yMaxActual);
    
    if (strlen(pens[actualPen].description) > 0)
    {
        ui->labelPen->setText(trUtf8("Trace: %1").arg(pens[actualPen].description));
    }
    else
    {
        ui->labelPen->setText(trUtf8("Trace: %1").arg(pens[actualPen].tag));
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void trend_range::updateData()
{
    /* call the parent updateData member */
    page::updateData();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void trend_range::changeEvent(QEvent * event)
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
trend_range::~trend_range()
{
    delete ui;
}

#undef VAR_TO_DISPLAY


void trend_range::on_pushButtonHome_clicked()
{
    go_home();
}

void trend_range::on_pushButtonBack_clicked()
{
    go_back();
}

void trend_range::on_pushButtonTime_clicked()
{
    if (timepopup) {
        QTime t = QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss"); // and not "HH:mm:ss"
        if (t.isValid()) {
            timepopup->setTime(t);
            timepopup->movePosition(ui->pushButtonTime->geometry().x(),ui->pushButtonTime->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                // ui->timeEdit->setTime(timepop->getTime());
                ui->pushButtonTime->setText(timepopup->getTime().toString("HH:mm:ss"));
            }
        }
    }
}

void trend_range::on_pushButtonWidth_clicked()
{
    if (timepopup) {
        QTime t = QTime::fromString(ui->pushButtonWidth->text(), "hh:mm:ss"); // and not "HH:mm:ss"
        if (t.isValid()) {
            timepopup->setTime(t);
            timepopup->movePosition(ui->pushButtonWidth->geometry().x(),ui->pushButtonWidth->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                // ui->timeEdit->setTime(timepop->getTime());
                ui->pushButtonWidth->setText(timepopup->getTime().toString("HH:mm:ss"));
            }
        }
    }
}

/* save the actual log period */
void trend_range::on_pushButtonOk_clicked()
{
    LOG_PRINT(verbose_e, "actualVisibleWindowSec %d actualTzero '%s'\n",
              actualVisibleWindowSec,
              actualTzero.toString("yy/MM/dd HH:mm:ss").toAscii().data()
              );

    int tmpwindow =
        #if 0
            ui->spinBoxSeconds->value() +
            ui->spinBoxMinutes->value() * 60 +
            ui->spinBoxHours->value() * 3600 +
        #else
            QTime(0,0,0).secsTo(QTime::fromString(ui->pushButtonWidth->text(), "hh:mm:ss")) +
        #endif
            ui->spinBoxDays->value() * 3600 * 24;

    if (actualVisibleWindowSec != tmpwindow)
    {
        actualVisibleWindowSec = tmpwindow;
    }

    QDateTime tmpTzero = QDateTime(                          
#if 0
                ui->dateEdit->date(),
                QTime(ui->spinBoxHoursIn->value(), ui->spinBoxMinutesIn->value(), ui->spinBoxSecondsIn->value())
#else
                QDate::fromString(ui->pushButtonCalendar->text(), "yyyy-MM-dd"),
                QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss") // and not "HH:mm:ss"
#endif
                );
    if (actualTzero != tmpTzero)
    {
        actualTzero = tmpTzero;
        _online_ = false;
    }

    LOG_PRINT(verbose_e, "actualVisibleWindowSec %d actualTzero '%s'\n",
              actualVisibleWindowSec,
              actualTzero.toString("yy/MM/dd HH:mm:ss").toAscii().data()
              );

    /* Y range */
    pens[actualPen].yMinActual = ui->doubleSpinBoxYmin->value();
    pens[actualPen].yMaxActual = ui->doubleSpinBoxYmax->value();

    _trend_data_reload_ = false;
    go_back();
}

void trend_range::on_pushButtonCalendar_clicked()
{ 
    if (calendarpopup) {
        QDate d = QDate::fromString(ui->pushButtonCalendar->text(), "yyyy-MM-dd");
        if (d.isValid()) {
            calendarpopup->setDate(d);
            calendarpopup->movePosition(ui->pushButtonCalendar->geometry().x(),ui->pushButtonCalendar->geometry().y());
            if (calendarpopup->exec() == QDialog::Accepted) {
                ui->pushButtonCalendar->setText(calendarpopup->getDate().toString("yyyy-MM-dd"));
            }
        }
    }
}
