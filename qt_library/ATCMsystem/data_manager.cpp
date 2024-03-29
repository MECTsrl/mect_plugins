/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Generic page
 */
#include <QDir>
#include "app_logprint.h"
#include "data_manager.h"
#include "item_selector.h"
#include "ui_data_manager.h"

/* this define set the window title */
#define WINDOW_TITLE ""
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ":/libicons/img/Home.png"

/**
 * @brief this macro is used to set the data_manager style.
 * the syntax is html stylesheet-like
 */
#define SET_DATA_MANAGER_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
data_manager::data_manager(QWidget *parent) :
    page(parent),
    ui(new Ui::data_manager)
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

    /* set up the page style */
    //setStyle::set(this);
    /* set the style described into the macro SET_DATA_MANAGER_STYLE */
    SET_DATA_MANAGER_STYLE();

    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the user name */
    //labelUserName = ui->labelUserName;

    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void data_manager::reload()
{
    ui->labelStatus->clear();
    ui->labelStatus->repaint();
    ui->labelStatus->update();
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void data_manager::updateData()
{
    /* call the parent updateData member */
    page::updateData();
}

void data_manager::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
data_manager::~data_manager()
{
    delete ui;
}

void data_manager::on_pushButtonAlarms_clicked()
{
    ui->labelStatus->setText(trUtf8("Loading alarm..."));
    ui->labelStatus->repaint();
    goto_page("alarms");
}

void data_manager::on_pushButtonStore_clicked()
{
    ui->labelStatus->setText(trUtf8("Loading store..."));
    ui->labelStatus->repaint();
    /* select a new item */
    item_selector * sel;
    QString value;
    QStringList list;

    QDir storeDir(CUSTOM_STORE_DIR, "*.csv");
    QStringList storeList = storeDir.entryList(QDir::Files);

    storeList.prepend("All");

    if (storeList.count() != 0)
    {
        sel = new item_selector(storeList, &value,trUtf8("STORE FILTER SELECTOR"));
        sel->showFullScreen();

        if (sel->exec() == QDialog::Accepted)
        {
            if (value.compare("All") != 0)
            {
                strcpy(_actual_store_, value.toAscii().data());
            }
            else
            {
                _actual_store_[0]='\0';
            }
            goto_page("store");
        }
        ui->labelStatus->clear();
        delete sel;
    }
    else
    {
        _actual_store_[0]='\0';
        goto_page("store");
    }
}

void data_manager::on_pushButtonRecipe_clicked()
{
    ui->labelStatus->setText(trUtf8("Loading recipe..."));
    ui->labelStatus->repaint();
    goto_page("recipe_select");
}

void data_manager::on_pushButtonTrend_clicked()
{
    ui->labelStatus->setText(trUtf8("Loading trend..."));
    ui->labelStatus->repaint();
    strcpy(_actual_trend_, ""); // strcpy(_actual_trend_, "trend1");
    _trend_data_reload_ = true  ;
    LOG_PRINT(verbose_e, "_trend_data_reload_ %d\n",  _trend_data_reload_);
    goto_page("trend");
}

void data_manager::on_pushButtonHome_clicked()
{
    go_home();
}

void data_manager::on_pushButtonBack_clicked()
{
    go_back();
}

void data_manager::on_pushButtonLogStart_clicked()
{
    logStart();
}

void data_manager::on_pushButtonLogStop_clicked()
{
    logStop();
}

void data_manager::on_pushButtonReboot_clicked()
{
    system("reboot");
}

void data_manager::on_pushButtonReload_clicked()
{
    system("/etc/rc.d/init.d/autoexec restart");
}
