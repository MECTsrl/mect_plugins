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

    void on_pushButton_PwdTimeout_clicked();

    void on_pushButton_ScreenSaver_clicked();

    void on_pushButton_SlowLogPeriod_clicked();

    void on_pushButton_FastLogPeriod_clicked();

    void on_pushButton_MaxLogSpace_clicked();

    void on_pushButton_TraceWindow_clicked();

    void on_pushButton_Retries_clicked();

    void on_pushButton_Blacklist_clicked();

    void on_pushButton_ReadPeriod1_clicked();

    void on_pushButton_ReadPeriod2_clicked();

    void on_pushButton_ReadPeriod3_clicked();

    void on_pushButton_Timeout_SERIAL_PORT_0_clicked();

    void on_pushButton_Silence_SERIAL_PORT_0_clicked();

    void on_pushButton_Timeout_SERIAL_PORT_1_clicked();

    void on_pushButton_Silence_SERIAL_PORT_1_clicked();

    void on_pushButton_Timeout_SERIAL_PORT_2_clicked();

    void on_pushButton_Silence_SERIAL_PORT_2_clicked();

    void on_pushButton_Timeout_SERIAL_PORT_3_clicked();

    void on_pushButton_Silence_SERIAL_PORT_3_clicked();

    void on_pushButton_Timeout_TCP_IP_PORT_clicked();

    void on_pushButton_Silence_TCP_IP_PORT_clicked();

private:
    Ui::system_ini *ui;
    void save_all();
};

#endif // SYSTEM_INI_H


