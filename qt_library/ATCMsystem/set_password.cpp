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
    /* connect the label that show the actual user name */
    labelUserName = ui->labelUserName;

    reload();
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void set_password::reload()
{
    // Label Values
    ui->labelAdmin->setText(QString("%1 Password:") .arg(PasswordsString[pwd_admin_e]));
    ui->labelSuper->setText(QString("%1 Password:") .arg(PasswordsString[pwd_super_user_e]));
    ui->labelUser->setText(QString("%1 Password:") .arg(PasswordsString[pwd_user_e]));
    // Password Actual Values
    ui->pushButtonAdmin->setText(QString("%1") .arg(passwords[pwd_admin_e], 4, 10));
    ui->pushButtonSuper->setText(QString("%1") .arg(passwords[pwd_super_user_e], 4, 10));
    ui->pushButtonUser->setText(QString("%1") .arg(passwords[pwd_user_e], 4, 10));
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void set_password::updateData()
{
    page::updateData();
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

void set_password::on_pushButtonHome_clicked()
{
    go_home();
}

void set_password::on_pushButtonBack_clicked()
{
    go_back();
}

void set_password::on_pushButtonAdmin_clicked()
{
    if (updatePassword(pwd_admin_e))  {
        ui->pushButtonAdmin->setText(QString("%1") .arg(passwords[pwd_admin_e], 4, 10));
    }
}

void set_password::on_pushButtonSuper_clicked()
{
    if (updatePassword(pwd_super_user_e))  {
        ui->pushButtonSuper->setText(QString("%1") .arg(passwords[pwd_super_user_e], 4, 10));
    }
}

void set_password::on_pushButtonUser_clicked()
{
    if (updatePassword(pwd_user_e))  {
        ui->pushButtonUser->setText(QString("%1") .arg(passwords[pwd_user_e], 4, 10));
    }
}

bool    set_password::updatePassword(enum password_level_e passLevel)
{
    bool    pwdChanged = false;
    int     min = 0, max = 999999;
    numpad * dk;
    int     oldPassword = passwords[passLevel];

    int newPassword = oldPassword;
    dk = new numpad(&newPassword, oldPassword, min, max, input_dec, false);
    dk->showFullScreen();

    if (dk->exec() == QDialog::Accepted)  {
        // Range Checking
        if ((newPassword < min || newPassword > max))  {
            QMessageBox::critical(0, trUtf8("Invalid data"), trUtf8("The inserted value is invalid.\nThe value must ranging between %2 and %3").arg(min).arg(max));
        }
        else  {
            // Password has been changed
            if (newPassword != oldPassword)  {
                passwords[passLevel] = newPassword;
                if (dumpPasswords() != 0)   {
                    // Saving error, restoring prev.value
                    passwords[passLevel] = oldPassword;
                    QMessageBox::critical(0, trUtf8("Saving error"), trUtf8("Cannot save the new password."));
                }
                else  {
                    pwdChanged = true;
                }
            }
        }
    }
    dk->deleteLater();
    return pwdChanged;
}
