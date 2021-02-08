/**
 * @file
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
    ui->pushButtonTime->setText(QTime::currentTime().toString("HH:mm:ss"));
    ui->pushButtonCalendar->setText(QDateTime::currentDateTime().date().toString("yyyy-MM-dd"));

    fOffset = ntpclient->getOffset_h();
    nTimeOut = ntpclient->getTimeout_s();
    nPeriod = ntpclient->getPeriod_h();
    szTimeServer = ntpclient->getNtpServer();

    ui->pushButtonNTP->setText(szTimeServer);
    ui->pushButtonOffset->setText(QString::number(fOffset, 'f', 1));
    ui->pushButtonTimeOut->setText(QString::number(nTimeOut));
    ui->pushButtonPeriod->setText(QString::number(nPeriod));

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

    if (! szTimeServer.isEmpty())  {
        ntpclient->setNtpParams(szTimeServer, nTimeOut, fOffset, nPeriod);
        ntpclient->requestNTPSync();
    }

//    ui->labelSync->setVisible(true);
//    QCoreApplication::processEvents();

//    NtpClient *ntp = new NtpClient();
//    ntp->setServerName(ui->pushButtonNTP->text());
//    ntp->setTimeZoneDST(ui->comboBoxTZDST->currentText().toInt());
//    if(ntp->doNTPSync()){
//        ui->pushButtonCalendar->setText(QDate::currentDate().toString("yyyy-MM-dd"));
//        ui->pushButtonTime->setText(QTime::currentTime().toString("hh:mm:ss"));
//        QMessageBox::information(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Date and Time set."));
//    } else {
//        QMessageBox::warning(0,QApplication::trUtf8("NTP SYNC"), QApplication::trUtf8("Network Error \nDate and Time can not be set."));
//    }
//    ui->labelSync->setVisible(false);

}

void time_set::on_pushButtonNTP_clicked()
{
    char value [64];
    alphanumpad tastiera_alfanum(value,ui->pushButtonNTP->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted)
    {
        szTimeServer = QString(value);
        ui->pushButtonNTP->setText(szTimeServer);
    }
}

void time_set::on_pushButtonSetManual_clicked()
{
    QDate d = QDate::fromString(ui->pushButtonCalendar->text(), "yyyy-MM-dd");
    QTime t = QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss");
    if (d.isValid() && t.isValid())  {
        QDateTime   currentDT(d, t);
        ntpclient->requestDateTimeChange(currentDT);
    }
}

void time_set::on_pushButtonOffset_clicked()
{
    numpad * dk;
    float value = fOffset;
    float min = -12;
    float max = +12;

    dk = new numpad(&value, fOffset, 1, min, max);
    dk->showFullScreen();
    if (dk->exec() == QDialog::Accepted)
    {
        fOffset = value;
        ui->pushButtonOffset->setText(QString::number(fOffset, 'f', 1));
    }
}

void time_set::on_pushButtonTimeOut_clicked()
{
    numpad * dk;
    int value = nTimeOut;
    int min = 1;
    int max = 60;

    dk = new numpad(&value, nTimeOut, min, max);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        nTimeOut = value;
        ui->pushButtonTimeOut->setText(QString::number(nTimeOut));
    }
}

void time_set::on_pushButtonPeriod_clicked()
{
    numpad * dk;
    int value = nPeriod;
    int min = 0;
    int max = THE_NTP_MAX_PERIOD_H;

    dk = new numpad(&value, nTimeOut, min, max);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        nPeriod = value;
        ui->pushButtonPeriod->setText(QString::number(nPeriod));
    }
}

void time_set::on_pushButtonNTPSet_clicked()
{
    if (! szTimeServer.isEmpty())  {
        ntpclient->setNtpParams(szTimeServer, nTimeOut, fOffset, nPeriod);
    }
}

void time_set::on_pushButtonNTPDefualts_clicked()
{
    // Restore default values
    szTimeServer = QString(THE_NTP_SERVER);
    nTimeOut = 10;
    nPeriod = 0;
    fOffset = 1.0;

    ui->pushButtonNTP->setText(szTimeServer);
    ui->pushButtonOffset->setText(QString::number(fOffset, 'f', 1));
    ui->pushButtonTimeOut->setText(QString::number(nTimeOut));
    ui->pushButtonPeriod->setText(QString::number(nPeriod));

}
