/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "pagebrowser.h"

namespace Ui {
class data_manager;
}

class data_manager : public page
{
    Q_OBJECT
    
public:
    explicit data_manager(QWidget *parent = 0);
    ~data_manager();
    virtual void reload(void);
    virtual void updateData();

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonAlarms_clicked();
    void on_pushButtonStore_clicked();
    void on_pushButtonRecipe_clicked();
    void on_pushButtonTrend_clicked();
    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonLogStart_clicked();
    void on_pushButtonLogStop_clicked();

    void on_pushButtonReboot_clicked();

    void on_pushButtonReload_clicked();

private:
    Ui::data_manager *ui;
};

#endif // DATA_MANAGER_H

