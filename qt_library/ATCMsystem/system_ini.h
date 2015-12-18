/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef SYSTEM_INI_H
#define SYSTEM_INI_H

#include "pagebrowser.h"

namespace Ui {
class system_ini;
}

class system_ini : public page
{
    Q_OBJECT
    
public:
    explicit system_ini(QWidget *parent = 0);
    ~system_ini();
    virtual void reload(void);
    virtual void updateData();
    
private slots:
    void changeEvent(QEvent * event);
    
    void on_pushButtonSave_clicked();

    void on_pushButtonHome_clicked();

    void on_pushButtonBack_clicked();

    void on_pushButton_PwdLogoutPage_clicked();

    void on_pushButton_StartPage_clicked();

    void on_pushButton_HomePage_clicked();

private:
    Ui::system_ini *ui;
    void save_all();
};

#endif // SYSTEM_INI_H


