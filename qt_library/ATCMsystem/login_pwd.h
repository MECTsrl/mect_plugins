/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef LOGIN_PWD_H
#define LOGIN_PWD_H

#include "pagebrowser.h"

namespace Ui {
class login_pwd;
}

class login_pwd : public page
{
    Q_OBJECT
    
public:
    explicit login_pwd(QWidget *parent = 0);
    ~login_pwd();
    virtual void reload(void);
    virtual void updateData();

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonLogin_clicked();
    void on_pushButtonPasswords_clicked();
    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::login_pwd *ui;
};

#endif // LOGIN_PWD_H

