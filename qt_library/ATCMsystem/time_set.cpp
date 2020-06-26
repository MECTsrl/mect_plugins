/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Setup date and time page
 */
#include "app_logprint.h"
#include "time_set.h"
#include "ui_time_set.h"
#include <sys/time.h>

#include "timepopup.h"
#include "calendar.h"
#include "ntpclient.h"

/* this define set the window title */
#define WINDOW_TITLE "DATA E ORA"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the current page style.
 * the syntax is html stylesheet-like
 */
#define SET_TIME_SET_STYLE() { \
QString mystyle; \
mystyle.append(this->styleSheet()); \
/* add there the page stilesheet customization */ \
mystyle.append(""); \
this->setStyleSheet(mystyle); \
}

time_set::time_set(QWidget *parent) :
    page(parent),
    ui(new Ui::time_set)
{
    ui->setupUi(this);
	/* set here the protection level (administrator_e, service_e, operator_e), default is operator_e
	 * protection_level = operator_e;
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
    SET_TIME_SET_STYLE();

    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;

    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
  * @brief show the actual date/time
  */
void time_set::reload()
{
    /* Load data and time 1*/
#if 0
    ui->dateEdit->setDate(QDateTime::currentDateTime().date());
    ui->spinBoxOre->setValue(QDateTime::currentDateTime().time().hour());
    ui->spinBoxMinuti->setValue(QDateTime::currentDateTime().time().minute());
#else
    ui->pushButtonTime->setText(QTime::currentTime().toString("HH:mm:ss"));
    ui->pushButtonCalendar->setText(QDateTime::currentDateTime().date().toString("yyyy-MM-dd"));
    int tz;
    int tzIni = ntpclient::getTimeZoneDST();
    if(!tzIni) {
        tzIni = 2;
    }
    ui->comboBoxTZDST->clear();
    for (int i=0; i< 25 ; i++) {
        tz = i-12;
        ui->comboBoxTZDST->addItem(QString::number(tz));
        if(tz == tzIni) {
            ui->comboBoxTZDST->setCurrentIndex(i);
        }
    }
    ui->labelSync->setVisible(false);
#endif
    showFullScreen();
}

void time_set::updateData()
{
    page::updateData();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void time_set::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}
#endif

time_set::~time_set()
{
    delete ui;
}


void time_set::on_pushButtonTime_clicked()
{
    TimePopup *timepop = new TimePopup(this->ui->pushButtonTime);

    if (timepop) {
        QTime t = QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss"); // and not "HH:mm:ss"
        timepop->setTime(t);
        if (timepop->exec() == QDialog::Accepted) {
            // ui->timeEdit->setTime(timepop->getTime());
            ui->pushButtonTime->setText(timepop->getTime().toString("HH:mm:ss"));
        }
        delete timepop;
    }
}

/**
  * @brief save the actual date/time
  */
void time_set::on_pushButtonOk_clicked()
{
    /* UPDATE THE SYSTEM CLOCK */

    time_t rt = 0;
    struct tm *pt = NULL;
    struct timezone timez;
    struct timeval temp;
    int rc = 0;

    rt = time(NULL);
    pt = localtime(&rt);
    if (pt == NULL) {
        fputs(__func__, stderr);
        perror(": while getting local time");
        fflush(stderr);

        return;
    }

#if 0
    pt->tm_year = ui->dateEdit->date().year() - 1900;
    pt->tm_mon = ui->dateEdit->date().month() - 1;
    pt->tm_mday = ui->dateEdit->date().day();
    pt->tm_hour = ui->spinBoxOre->value();
    pt->tm_min = ui->spinBoxMinuti->value();
    pt->tm_sec = 0;
#else
    QDate d = QDate::fromString(ui->pushButtonCalendar->text(), "yyyy-MM-dd");
    pt->tm_year = d.year() - 1900;
    pt->tm_mon = d.month() - 1;
    pt->tm_mday = d.day();

    QTime t = QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss");
    pt->tm_hour = t.hour();
    pt->tm_min = t.minute();
    pt->tm_sec = t.second();
#endif

    rc = gettimeofday(&temp, &timez);
    if (rc < 0) {
        fputs(__func__, stderr);
        perror(": while getting time of day");
        fflush(stderr);

        return;
    }

    temp.tv_sec = mktime(pt);
    temp.tv_usec = 0;

    rc = settimeofday(&temp, &timez);
    if (rc < 0) {
        fputs(__func__, stderr);
        perror(": while setting time of day");
        fflush(stderr);

        return;
    }

    system("/sbin/hwclock -wu");        /* Update RTC from system */

}

void time_set::on_pushButtonHome_clicked()
{
    go_home();
}

void time_set::on_pushButtonBack_clicked()
{
    go_back();
}

void time_set::on_pushButtonCalendar_clicked()
{
    Calendar *calendar = new Calendar(this->ui->pushButtonCalendar);

    if (calendar) {
        QDate d = QDate::fromString(ui->pushButtonCalendar->text(), "yyyy-MM-dd");
        calendar->setDate(d);
        if (calendar->exec() == QDialog::Accepted) {
           ui->pushButtonCalendar->setText(calendar->getDate().toString("yyyy-MM-dd"));
        }
        delete calendar;
    }
}

void time_set::on_pushButtonNTPSync_clicked()
{
    ui->labelSync->setVisible(true);
    QCoreApplication::processEvents();

    ntpclient *ntp = new ntpclient();
    ntp->setServerName(ui->pushButtonNTP->text());
    ntp->setTimeZoneDST(ui->comboBoxTZDST->currentText().toInt());
    if(ntp->doNTPSync()){
        ui->pushButtonCalendar->setText(QDate::currentDate().toString("yyyy-MM-dd"));
        ui->pushButtonTime->setText(QTime::currentTime().toString("hh:mm:ss"));
        QMessageBox::information(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Date and Time set."));
    } else {
        QMessageBox::warning(0,QApplication::trUtf8("NTP SYNC"), QApplication::trUtf8("Network Error \nDate and Time can not be set."));
    }
    ui->labelSync->setVisible(false);

}

void time_set::on_pushButtonNTP_clicked()
{
    char value [64];
    alphanumpad tastiera_alfanum(value,ui->pushButtonNTP->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted)
    {
        ui->pushButtonNTP->setText(value);

    }
}
