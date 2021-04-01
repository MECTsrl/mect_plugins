/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef STORE_FILTER_H
#define STORE_FILTER_H

#include <QDateTime>

#include "pagebrowser.h"

namespace Ui {
class store_filter;
}

class store_filter : public page
{
    Q_OBJECT
    
public:
    explicit store_filter(QWidget *parent = 0);
    ~store_filter();
    virtual void reload(void);
    virtual void updateData();

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonCancel_clicked();
    void on_pushButtonOk_clicked();
    void on_pushButtonReset_clicked();
    void on_pushButtonDateStart_clicked();
    void on_pushButtonDateEnd_clicked();
    void on_pushButtonTimeStart_clicked();
    void on_pushButtonTimeEnd_clicked();

private:
    Ui::store_filter *ui;

    QDateTime       filterStart;
    QDateTime       filterEnd;
};

#endif // STORE_FILTER_H

