/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Setup date and time page
 */
#ifndef TIME_SET_H
#define TIME_SET_H

#include "pagebrowser.h"

#include <QString>

namespace Ui {
class time_set;
}

class time_set : public page
{
    Q_OBJECT
    
public:
    explicit time_set(QWidget *parent = 0);
    ~time_set();
    virtual void reload(void);
    virtual void updateData();

private slots:
#ifdef TRANSLATION
    void changeEvent(QEvent * event);
#endif

    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonSetManual_clicked();
    void on_pushButtonTime_clicked();
    void on_pushButtonCalendar_clicked();
    void on_pushButtonNTPSync_clicked();
    void on_pushButtonNTP_clicked();   
    void on_pushButtonOffset_clicked();
    void on_pushButtonTimeOut_clicked();
    void on_pushButtonPeriod_clicked();

    void on_pushButtonNTPSet_clicked();

    void on_pushButtonNTPDefualts_clicked();

private:
    Ui::time_set *ui;
    int         nOffset;
    int         nTimeOut;
    int         nPeriod;
    QString     szTimeServer;

};
#endif // TIME_SET_H

