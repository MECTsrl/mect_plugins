/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include "app_logprint.h"
#include "numpad.h"
#include "display_settings.h"
#include "ui_display_settings.h"
#include <QMessageBox>
#include <QWSServer>
#include <QSettings>
#include <unistd.h>

/* this define set the window title */
#define WINDOW_TITLE ""
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ":/libicons/img/Home.png"

/**
 * @brief this macro is used to set the display_settings style.
 * the syntax is html stylesheet-like
 */
#define SET_DISPLAY_SETTINGS_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
display_settings::display_settings(QWidget *parent) :
    page(parent),
    ui(new Ui::display_settings)
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
    
    ////setStyle::set(this);
    //SET_DISPLAY_SETTINGS_STYLE();
    
    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void display_settings::reload()
{
    
    int brightness_level_old;
    FILE * fp = fopen(BACKLIGHT_FILE_SYSTEM, "r");
    if (fp != NULL)
    {
        fscanf(fp, "%d", &brightness_level);
        fclose(fp);
    }
    
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(change_brightness()));
    increment = 1;
    
    brightness_level_old = brightness_level;
    
    ui->horizontalSlider->setMinimum(MIN_BACKLIGHT_LEVEL);
    ui->horizontalSlider->setMaximum(MAX_BACKLIGHT_LEVEL);
    brightness_level = brightness_level_old;
    ui->horizontalSlider->setValue(brightness_level);
    
    ui->pushButtonON->setStyleSheet("background-color: DarkTurquoise;");
    
    if (ScreenSaverSec == 0)
    {
        ui->pushButtonOFF->setStyleSheet("background-color: DarkTurquoise;");
        ui->pushButtonON->setStyleSheet("background-color: SlateGray;");
        ui->pushButtonON->setText("Disabled");
    }
    else
    {
        ui->pushButtonON->setText(QString("ON %1 sec").arg(ScreenSaverSec));
        ui->pushButtonON->setStyleSheet("background-color: DarkTurquoise;");
        ui->pushButtonOFF->setStyleSheet("background-color: SlateGray;");
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void display_settings::updateData()
{
    page::updateData();
}

void display_settings::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
display_settings::~display_settings()
{
    delete ui;
}

void display_settings::change_brightness()
{
    int value = brightness_level;
    value+=increment;
    if (value > MIN_BACKLIGHT_LEVEL && value < MAX_BACKLIGHT_LEVEL)
    {
        ui->horizontalSlider->setValue(value);
    }
}

void display_settings::on_pushButtonMinus_pressed()
{
    timer->start(50);
    increment = -1;
}

void display_settings::on_pushButtonMinus_released()
{
    timer->stop();
}


void display_settings::on_pushButtonPlus_pressed()
{
    timer->start(50);
    increment = 1;
}

void display_settings::on_pushButtonPlus_released()
{
    timer->stop();
}

void display_settings::on_horizontalSlider_valueChanged(int value)
{
    if (brightness_level < 0)
    {
        return;
    }
    char command[256];
    LOG_PRINT(verbose_e, "%d to %d\n", brightness_level, value);
    sprintf (command, "echo %d > %s", value, BACKLIGHT_FILE_SYSTEM);
    if (system(command) == 0)
    {
        brightness_level = value;
    }
}

void display_settings::on_pushButton_clicked()
{
    FILE * fp;
    fp = fopen(BACKLIGHT_FILE_LOCAL, "w");
    if (fp == NULL)
    {
        QMessageBox::critical(0, trUtf8("Internal error"), trUtf8("Impossible to save brightness value into the file '%1'.").arg(BACKLIGHT_FILE_LOCAL));
    }
    fprintf(fp, "echo %d > %s\n", brightness_level, BACKLIGHT_FILE_SYSTEM);
    fclose(fp);
    sync();
}

void display_settings::on_pushButtonON_clicked()
{
    numpad * dk;
    int value;
    int min = 1; //min time allowed for the screensaver start expressed in seconds
    int max = 3600; //max time allowed for the screensaver start expressed in seconds
    
    dk = new numpad(&value, ScreenSaverSec, min, max);
    dk->showFullScreen();
    
    if (dk->exec() == QDialog::Accepted)
    {
        if (min < max && (value < min || value > max))
        {
            QMessageBox::critical(0, trUtf8("Invalid value"), trUtf8("The typed value is invalid.\nThe value must be in the range %2 and %3 ss").arg(min).arg(max));
            delete dk;
            return;
        }
        ui->pushButtonON->setText(QString("ON %1 sec").arg(value));
        ui->pushButtonON->setStyleSheet("background-color: DarkTurquoise;");
        ui->pushButtonOFF->setStyleSheet("background-color: SlateGray;");
        ScreenSaverSec = value;
        QWSServer::setScreenSaverInterval(ScreenSaverSec*1000); //msec
        
        QSettings settings(CONFIG_FILE, QSettings::IniFormat);
        settings.setValue(SCREENSAVER_TAG, ScreenSaverSec);
        settings.sync();
    }
    delete dk;
}

void display_settings::on_pushButtonOFF_clicked()
{
    //disabling screensaver
    QWSServer::setScreenSaverInterval(0); //msec
    ui->pushButtonOFF->setStyleSheet("background-color: DarkTurquoise;");
    ui->pushButtonON->setStyleSheet("background-color: SlateGray;");
    ui->pushButtonON->setText("Disabled");
    ScreenSaverSec = 0;
    QWSServer::setScreenSaverInterval(ScreenSaverSec*1000); //msec
    
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    settings.setValue(SCREENSAVER_TAG, ScreenSaverSec);
    settings.sync();
}

void display_settings::on_pushButtonHome_clicked()
{
    go_home();
}

void display_settings::on_pushButtonBack_clicked()
{
    go_back();
}


void display_settings::on_pushButtonTEST_clicked()
{
    goto_page("display_test");
}
