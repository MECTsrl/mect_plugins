/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef SET_PASSWORD_H
#define SET_PASSWORD_H

#include "pagebrowser.h"

namespace Ui {
class set_password;
}

class set_password : public page
{
    Q_OBJECT
    
public:
    explicit set_password(QWidget *parent = 0);
    ~set_password();
    virtual void reload(void);
    virtual void updateData();

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonLogin_clicked();
    void on_pushButtonPasswords_clicked();
    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();

private:
    Ui::set_password *ui;
};

#endif // SET_PASSWORD_H

