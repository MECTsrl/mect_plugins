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
#include <QMessageBox>

/* this define set the window title */
#define WINDOW_TITLE "DATA E ORA"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

#define TIME_MASK "HH:mm:ss"
#define DATE_MASK "yyyy-MM-dd"

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

    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    // Running Flags
    lockInterface = false;
    ntpSyncRunning = false;
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
  * @brief show the actual date/time
  */
void time_set::reload()
{
    /* Load data and time 1*/
    nOffset = ntpclient->getOffset_h();
    isDst = ntpclient->getDst();
    nTimeOut = ntpclient->getTimeout_s();
    nPeriod = ntpclient->getPeriod_h();
    szTimeServer = ntpclient->getNtpServer();    

    updateIface();
    ui->progressBarElapsed->setVisible(false);
    lockInterface = false;
    lockUI(lockInterface);
    showFullScreen();
}

void time_set::updateData()
{
    lockUI(lockInterface);
    ui->progressBarElapsed->setVisible(ntpSyncRunning);
    // Progress Bar di aggiornamento NTP
    if (ntpSyncRunning && nTimeOut && syncElapsed.isValid())  {
        int nElapsed = syncElapsed.elapsed() / 1000;
        if (nElapsed <= ui->progressBarElapsed->maximum())  {
            ui->progressBarElapsed->setValue(nElapsed);
        }
    }
    page::updateData();
}

void time_set::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

time_set::~time_set()
{
    delete ui;
}


void time_set::on_pushButtonTime_clicked()
{
    if (timepopup) {
        QTime t = QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss"); // and not "HH:mm:ss"
        if (t.isValid()) {
            lockInterface = true;
            lockUI(true);
            timepopup->setTime(t);
            timepopup->setModal(true);
            timepopup->movePosition(ui->pushButtonTime->geometry().x(),ui->pushButtonTime->geometry().y());
            if (timepopup->exec() == QDialog::Accepted) {
                // ui->timeEdit->setTime(timepop->getTime());
                ui->pushButtonTime->setText(timepopup->getTime().toString(TIME_MASK));
            }
            lockInterface = false;
        }
    }
}

/**
  * @brief save the actual date/time
  */

void time_set::on_pushButtonHome_clicked()
{
    QObject::disconnect(ntpclient, 0, 0, 0);
    go_home();
}

void time_set::on_pushButtonBack_clicked()
{
    QObject::disconnect(ntpclient, 0, 0, 0);
    go_back();
}

void time_set::on_pushButtonCalendar_clicked()
{

    if (calendarpopup) {
        QDate d = QDate::fromString(ui->pushButtonCalendar->text(), DATE_MASK);
        if (d.isValid()) {
            lockInterface = true;
            lockUI(true);
            calendarpopup->setDate(d);
            calendarpopup->setModal(true);
            calendarpopup->movePosition(ui->pushButtonCalendar->geometry().x(),ui->pushButtonCalendar->geometry().y());
            if (calendarpopup->exec() == QDialog::Accepted) {
                ui->pushButtonCalendar->setText(calendarpopup->getDate().toString(DATE_MASK));
            }
            lockInterface = false;
        }
    }
}

void time_set::on_pushButtonNTPSync_clicked()
{
    // Avvio della sincronizzazione via NTP
    if (! szTimeServer.isEmpty() && nTimeOut)  {
        lockInterface = true;
        lockUI(lockInterface);
        ui->progressBarElapsed->setMaximum(nTimeOut);
        ntpclient->setNtpParams(szTimeServer, nTimeOut, nOffset, nPeriod, isDst);
        QObject::connect(ntpclient, SIGNAL(ntpSyncFinish(bool )), this, SLOT(ntpSyncDone(bool)));
        ntpclient->requestNTPSync();
        ntpSyncRunning = true;
        syncElapsed.restart();
    }
}

void time_set::on_pushButtonNTPServer_clicked()
{
    char value [64];
    alphanumpad tastiera_alfanum(value,ui->pushButtonNTPServer->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if (tastiera_alfanum.exec()==QDialog::Accepted)
    {
        szTimeServer = QString(value);
        updateIface();
    }
}

void time_set::on_pushButtonSetManual_clicked()
{
    QDate d = QDate::fromString(ui->pushButtonCalendar->text(), DATE_MASK);
    QTime t = QTime::fromString(ui->pushButtonTime->text(), "hh:mm:ss");

    if (d.isValid() && t.isValid())  {
        lockInterface = true;
        lockUI(lockInterface);
        QDateTime   currentDT(d, t);
        QObject::connect(ntpclient, SIGNAL(ntpDateTimeChangeFinish(bool)), this, SLOT(ntpManualSetDone(bool)));
        datetimeTarget = currentDT;
        ntpclient->requestDateTimeChange(currentDT);
    }
}

void time_set::on_pushButtonNTPOffset_clicked()
{
    numpad * dk;
    int value = nOffset;
    int min = -12;
    int max = +12;

    dk = new numpad(&value, nOffset, min, max);
    dk->showFullScreen();
    if (dk->exec() == QDialog::Accepted)
    {
        nOffset = value;
        updateIface();
    }
}

void time_set::on_checkBoxDst_stateChanged(int state)
{
    switch (state) {
    case Qt::Checked:
        isDst = true;
        break;
    case Qt::Unchecked:
    default:
        isDst = false;
    }
}

void time_set::on_pushButtonNTPTimeOut_clicked()
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
        updateIface();
    }
}

void time_set::on_pushButtonNTPPeriod_clicked()
{
    numpad * dk;
    int value = nPeriod;
    int min = 0;
    int max = THE_NTP_MAX_PERIOD_H;

    dk = new numpad(&value, nPeriod, min, max);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        nPeriod = value;
        updateIface();
    }
}

void time_set::on_pushButtonNTPSet_clicked()
{
    if (! szTimeServer.isEmpty())  {
        ntpclient->setNtpParams(szTimeServer, nTimeOut, nOffset, nPeriod, isDst);
    }
}

void time_set::on_pushButtonNTPDefaults_clicked()
{
    // Restore default values
    szTimeServer = QString(THE_NTP_SERVER);
    nTimeOut = 10;
    nPeriod = 0;
    nOffset = 1;
    isDst = false;
    updateIface();
}

void time_set::updateIface()
{
    ui->pushButtonNTPServer->setText(szTimeServer);
    ui->pushButtonNTPOffset->setText(QString("%1 h") .arg(nOffset,2,10));
    ui->checkBoxDst->setCheckState(isDst ? Qt::Checked : Qt::Unchecked);
    ui->pushButtonNTPTimeOut->setText(QString("%1 s") .arg(nTimeOut,2,10));
    ui->pushButtonNTPPeriod->setText(QString("%1 h") .arg(nPeriod,4,10));
    ui->pushButtonTime->setText(QTime::currentTime().toString(TIME_MASK));
    ui->pushButtonCalendar->setText(QDate::currentDate().toString(DATE_MASK));
}

void time_set::lockUI(bool setLocked)
{
    // Aggiornamento dell'interfaccia utente
    ui->pushButtonCalendar->setEnabled(! setLocked);
    ui->pushButtonTime->setEnabled(! setLocked);
    ui->pushButtonNTPServer->setEnabled(! setLocked);
    ui->pushButtonSetManual->setEnabled(! setLocked);
    ui->pushButtonNTPSet->setEnabled(! setLocked);
    ui->pushButtonNTPSync->setEnabled(! setLocked);
    ui->pushButtonNTPDefaults->setEnabled(! setLocked);
    ui->pushButtonNTPOffset->setEnabled(! setLocked);
    ui->checkBoxDst->setEnabled(! setLocked);
    ui->pushButtonNTPTimeOut->setEnabled(! setLocked);
    ui->pushButtonNTPPeriod->setEnabled(! setLocked);
}

void time_set::ntpManualSetDone(bool setOk)
{
    QObject::disconnect(ntpclient, 0, 0, 0);
    if (setOk)  {
        QMessageBox::information(this,trUtf8("Manual Date Time Set"), trUtf8("Current Date and Time set to:\n%1") .arg(datetimeTarget.toString(DATE_MASK" "TIME_MASK)));
    }
    else {
        QMessageBox::critical(this,trUtf8("Manual Date Time Set"), trUtf8("Error setting Date and Time to:\n%1") .arg(datetimeTarget.toString(DATE_MASK" "TIME_MASK)));
    }
    lockInterface = false;
    lockUI(lockInterface);
}

void time_set::ntpSyncDone(bool timeOut)
{
    QObject::disconnect(ntpclient, 0, 0, 0);
    ntpSyncRunning = false;
    if (timeOut)  {
        QMessageBox::warning(this,trUtf8("NTP Time Error"), trUtf8("Time Out syncing Date and Time with NTP Server:\n%1") .arg(szTimeServer));
    }
    else  {
        QMessageBox::information(this,trUtf8("NTP Time Set"), trUtf8("Current Date and Time set from NTP Server:\n%1") .arg(szTimeServer));
    }
    updateIface();
    lockInterface = false;
    lockUI(lockInterface);
}
