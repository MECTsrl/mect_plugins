/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Info page
 */
#ifndef INFO_H
#define INFO_H

#include "pagebrowser.h"

namespace Ui {
class info;
}

class info : public page
{
    Q_OBJECT
    
public:
    explicit info(QWidget *parent = 0);
    ~info();
    virtual void reload(void);
    virtual void updateData();
    bool getVersion(const char * command, char * version, int maxsize);

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonQrc_clicked();
    void on_pushButtonRefresh_clicked();

private:
    Ui::info *ui;
    void refreshSystemTab();
    void refreshApplTab();
    void refreshNetworkingTabs();
    void refreshNTPInfo();
};

#endif // INFO_H

