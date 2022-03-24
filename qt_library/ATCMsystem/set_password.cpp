/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include <QMessageBox>

#include "app_logprint.h"
#include "numpad.h"
#include "set_password.h"
#include "ui_set_password.h"

#include "global_functions.h"

/* this define set the window title */
#define WINDOW_TITLE "LOGIN - PASSWORD"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""


/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
set_password::set_password(QWidget *parent) :
    page(parent),
    ui(new Ui::set_password)
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
    
    //setStyle::set(this);
    labelDataOra = ui->labelDataOra;
    
    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void set_password::reload()
{
    if (active_password == pwd_operator_e)
    {
        ui->pushButtonPasswords->setEnabled(false);
    }
    else
    {
        ui->pushButtonPasswords->setEnabled(true);
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void set_password::updateData()
{
    page::updateData();
    if (active_password == pwd_operator_e)
    {
        ui->pushButtonLogin->setText(trUtf8("LOGIN"));
        ui->pushButtonPasswords->setEnabled(false);
    }
    else
    {
        ui->pushButtonLogin->setText(trUtf8("LOGOUT"));
        ui->pushButtonPasswords->setEnabled(true);
    }
}

void set_password::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
set_password::~set_password()
{
    delete ui;
}


void set_password::on_pushButtonLogin_clicked()
{
    int min = 0, max = 999999;
    numpad * dk;
    int i;
    int password;
    
    if (active_password != pwd_operator_e)
    {
        // Logout Action
        active_password = pwd_operator_e;
        ui->pushButtonLogin->setText(trUtf8("LOGIN"));
    }
    else
    {
        // Login Action
        dk = new numpad(&password, NO_DEFAULT, min, max, input_dec, true);
        dk->showFullScreen();
        
        if (dk->exec() == QDialog::Accepted)
        {
            active_password = pwd_operator_e;
            
            if (min < max && (password < min || password > max))
            {
                QMessageBox::critical(this,trUtf8("Invalid password"), trUtf8("The inserted password is wrong!!!"));
                delete dk;
                return;
            }
            for (i = 0; i <= PASSWORD_NB; i++)
            {
                LOG_PRINT(verbose_e, "#############%d vs %d\n", password, passwords[i]);
                if (password == passwords[i])
                {
                    active_password = i;
                    LOG_PRINT(verbose_e, "%d vs %d\n", password, passwords[i]);
                    break;
                }
            }
            if (active_password == pwd_operator_e)
            {
                QMessageBox::critical(this,trUtf8("Invalid password"), trUtf8("The inserted password is wrong!!!"));
                delete dk;
                return;
            }
            else
            {
                QMessageBox::information(this,trUtf8("Login"), trUtf8("Logged as User: %1.").arg(PasswordsString[active_password]));
                ui->pushButtonLogin->setText(trUtf8("LOGOUT"));
            }
        }
        else
        {
        }
        delete dk;
    }
}

void set_password::on_pushButtonHome_clicked()
{
    go_home();
}

void set_password::on_pushButtonBack_clicked()
{
    go_back();
}

void set_password::on_pushButtonPasswords_clicked()
{
    int min = 0, max = 999999;
    numpad * dk;

    int password;
    dk = new numpad(&password, NO_DEFAULT, min, max, input_dec, true);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)
    {
        if (min < max && (password < min || password > max))
        {
            QMessageBox::critical(this,trUtf8("Invalid data"), trUtf8("The inserted value is invalid.\nThe value must ranging between %2 and %3").arg(min).arg(max));
            delete dk;
            return;
        }

        passwords[active_password] = password;
        if (dumpPasswords() == 0)
        {
            QMessageBox::information(this,trUtf8("Password changed"), trUtf8("The password is succesfully changed."));
        }
        else
        {
            QMessageBox::critical(this,trUtf8("Saving error"), trUtf8("Cannot save the new password."));
        }
    }
    else
    {
    }
    delete dk;

}
