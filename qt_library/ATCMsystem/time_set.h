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
#include <QElapsedTimer>

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
    void changeEvent(QEvent * event);

    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonSetManual_clicked();
    void on_pushButtonTime_clicked();
    void on_pushButtonCalendar_clicked();
    void on_pushButtonNTPSync_clicked();
    void on_pushButtonNTPServer_clicked();
    void on_pushButtonNTPOffset_clicked();
    void on_checkBoxDst_stateChanged(int state);
    void on_pushButtonNTPTimeOut_clicked();
    void on_pushButtonNTPPeriod_clicked();
    void on_pushButtonNTPSet_clicked();
    void on_pushButtonNTPDefaults_clicked();
    void ntpSyncDone(bool timeOut);
    void ntpManualSetDone(bool setOk);
    void lockUI(bool setLocked);

private:
    void        updateIface();

    Ui::time_set    *ui;
    int             nOffset;
    bool            isDst;
    int             nTimeOut;
    int             nPeriod;
    QString         szTimeServer;
    bool            lockInterface;
    bool            ntpSyncRunning;
    QElapsedTimer   syncElapsed;
    QDateTime       datetimeTarget;
};
#endif // TIME_SET_H

