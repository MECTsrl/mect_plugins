/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief display the file SPLASH_FILE for SPLASH_TIMEOUT_MS milliseconds
 */
#include <QTimer>
#include <QMessageBox>
#include <QProcess>

#include "page0.h"
#include "app_logprint.h"
#include "ui_page0.h"
#include "global_functions.h"
#ifdef TARGET
#ifdef ENABLE_SCREENSAVER
#include "screensaver.h"
#endif
#endif

page0::page0(QWidget *parent) :
    page(parent),
    ui(new Ui::page0)
{
    ui->setupUi(this);
    
    /* load and show the splash screen*/
    QPixmap splash;
    if (splash.load(SPLASH_FILE))
    {
        LOG_PRINT(info_e, "Splash file: '%s'\n", SPLASH_FILE);
        ui->label->setPixmap(splash);
        ui->label->update();
    }
    else
    {
        LOG_PRINT(info_e, "Cannot load splash file: '%s'\n", SPLASH_FILE);
    }
    
    first_time = true;
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void page0::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}
#endif

page0::~page0()
{
    delete ui;
}

void page0::reload()
{
    /* stop the unused refresh timer */
    refresh_timer->stop();
    
    /* prepare the splash screen time out */
    QTimer::singleShot(10, this, SLOT(changePage()));
}

void page0::changePage()
{
    if (first_time == true)
    {
        /* if fcrts is not running or if it is in Zombie status, start it */
        if (system("PID=`pidof fcrts` && test $PID != '' &&  test  `grep -c zombie /proc/$PID/status` -eq 0"))
        {
            QProcess *myProcess = new QProcess();
            myProcess->start("fcrts.sh");
        }

        if(CommStart() == false)
        {
            exit(0);
        }
        first_time = false;
    
        /* pre-load */
        page * p;
        userPageList.removeDuplicates();
        for (int pageIndex = 0; pageIndex < userPageList.count(); pageIndex++)
        {
            create_next_page(&p, userPageList.at(pageIndex).toAscii().data());
            if (p != NULL)
            {
                ScreenHash.insert(userPageList.at(pageIndex).toAscii().data(), p);
            }
            else
            {
                LOG_PRINT(error_e,"Cannot create page '%s'\n", userPageList.at(pageIndex).toAscii().data());
            }
        }
    }

    /* Check the date. if it is unset 01 Jan 1970 start with time_set page */
    time_t rt = 0;
    struct tm *pt = NULL;
    rt = time(NULL);
    pt = localtime(&rt);
    if (pt != NULL && pt->tm_year == 70 && pt->tm_mon == 0 && pt->tm_mday == 1)
    {
        if (goto_page("time_set", false) == false && goto_page(STARTPAGE_DEF, false) == false && goto_page(HomePage, false) == false && goto_page(HOMEPAGE_DEF, false) == false )
        {
            QMessageBox::critical(0,QApplication::tr("Invalid Page"), QApplication::tr("Cannot show any of Default pages '%1', '%2' and home pages '%3'. '%4'").arg("time_set").arg(STARTPAGE_DEF).arg(HomePage).arg(HOMEPAGE_DEF));
        }
    }

    /* go to the home page */
    else if (goto_page(StartPage, false) == false && goto_page(STARTPAGE_DEF, false) == false && goto_page(HomePage, false) == false && goto_page(HOMEPAGE_DEF, false) == false )
    {
        QMessageBox::critical(0,QApplication::tr("Invalid Page"), QApplication::tr("Cannot show any of Default pages '%1', '%2' and home pages '%3'. '%4'").arg(StartPage).arg(STARTPAGE_DEF).arg(HomePage).arg(HOMEPAGE_DEF));
    }
    this->hide();
}
