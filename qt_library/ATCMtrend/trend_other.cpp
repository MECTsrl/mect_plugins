/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include "app_logprint.h"
#include "trend_other.h"
#include "trend.h"
#include "ui_trend_other.h"

/* this define set the variables list to be to displayed in this page */
#undef VAR_TO_DISPLAY
/* Example:
#define VAR_TO_DISPLAY "RET_REG_1" << "RET_REG_3" << "NRE_REG_1"
*/
/* this define set the window title */
#define WINDOW_TITLE ""
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the trend_other style.
 * the syntax is html stylesheet-like
 */
#define SET_SELECT_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
trend_other::trend_other(QWidget *parent) :
    page(parent),
    ui(new Ui::trend_other)
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
    
    //SET_DIALOG_STYLE();
    SET_SELECT_STYLE();
    tp = (trend *)parent;
    this->setAutoFillBackground(true);
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

void trend_other::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }
}

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void trend_other::reload()
{
    if (((trend*)tp)->getOnline())
    {
        ui->pushButtonOnline->setStyleSheet(
                    "QPushButton"
                    "{"
                    "border: 2px solid  rgb(94, 94, 94);"
                    "border-radius: 8px;"
                    "qproperty-icon: url(:/libicons/img/Pause.png);"
                    "qproperty-iconSize: 24px 24px;"
                    "qproperty-focusPolicy: NoFocus;"
                    "}"
                    "QPushButton:pressed"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "background-color:  rgb(255, 255, 127);"
                    "}"
                    "QPushButton:disabled"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "}"
                    );
    }
    else
    {
        ui->pushButtonOnline->setStyleSheet(
                    "QPushButton"
                    "{"
                    "border: 2px solid  rgb(94, 94, 94);"
                    "border-radius: 8px;"
                    "qproperty-icon: url(:/libicons/img/Sync.png);"
                    "qproperty-iconSize: 24px 24px;"
                    "qproperty-focusPolicy: NoFocus;"
                    "}"
                    "QPushButton:pressed"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "background-color:  rgb(255, 255, 127);"
                    "}"
                    "QPushButton:disabled"
                    "{"
                    "border: 2px solid  rgb(194, 194, 194);"
                    "}"
                    );
    }
    ui->pushButtonOnline->update();
}

void trend_other::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
trend_other::~trend_other()
{
    LOG_PRINT(verbose_e, "############################################################## DELETE\n");
    
    delete ui;
}

#undef VAR_TO_DISPLAY

void trend_other::mouseMoveEvent(QMouseEvent * e)
{
    //this->setEnabled(false);
    LOG_PRINT(verbose_e, "################# x %d y %d\n", e->x(), e->y());
    //this->move(this->x() + e->x(), this->y() + e->y() - this->height()/3);
    //this->move(e->x(), e->y());
}

void trend_other::on_pushButtonOnline_clicked()
{
    LOG_PRINT(verbose_e, "Calling setOnline\n");
    ((trend*)tp)->setOnline(!(((trend*)tp)->getOnline()));
}

void trend_other::on_pushButtonScreenshot_clicked()
{
    ((trend*)tp)->printGraph();
}

void trend_other::on_pushButtonUp_clicked()
{
    if (_layout_ == PORTRAIT)
    {
        ((trend*)tp)->incrementTime(1);
    }
    else
    {
        ((trend*)tp)->incrementValue(1);
    }
}

void trend_other::on_pushButtonDown_clicked()
{
    if (_layout_ == PORTRAIT)
    {
        ((trend*)tp)->incrementTime(-1);
    }
    else
    {
        ((trend*)tp)->incrementValue(-1);
    }
}

void trend_other::on_pushButtonLeft_clicked()
{
    if (_layout_ == PORTRAIT)
    {
        ((trend*)tp)->incrementValue(-1);
    }
    else
    {
        ((trend*)tp)->incrementTime(-1);
    }
}

void trend_other::on_pushButtonRight_clicked()
{
    if (_layout_ == PORTRAIT)
    {
        ((trend*)tp)->incrementValue(1);
    }
    else
    {
        ((trend*)tp)->incrementTime(1);
    }
}

void trend_other::enableButtonUp(bool status)
{
    ui->pushButtonUp->setEnabled(status);
}

void trend_other::enableButtonDown(bool status)
{
    ui->pushButtonDown->setEnabled(status);
}

void trend_other::enableButtonLeft(bool status)
{
    ui->pushButtonLeft->setEnabled(status);
}

void trend_other::enableButtonRight(bool status)
{
    ui->pushButtonRight->setEnabled(status);
}

