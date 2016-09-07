/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef NET_CONF_H
#define NET_CONF_H

#include "pagebrowser.h"

namespace Ui {
class net_conf;
}

class net_conf : public page
{
    Q_OBJECT
    
public:
    explicit net_conf(QWidget *parent = 0);
    ~net_conf();
    virtual void reload(void);
    virtual void updateData();
    
private slots:
    void changeEvent(QEvent * event);
    
    void on_pushButton_eth0_IP_clicked();
    void on_pushButton_eth0_NM_clicked();
    void on_pushButton_eth0_GW_clicked();
    void on_pushButton_eth0_DNS1_clicked();
    void on_pushButton_eth0_DNS2_clicked();

    void on_pushButton_eth1_IP_clicked();
    void on_pushButton_eth1_NM_clicked();
    void on_pushButton_eth1_GW_clicked();
    void on_pushButton_eth1_DNS1_clicked();
    void on_pushButton_eth1_DNS2_clicked();

    void on_pushButton_wlan0_scan_clicked();
    void on_pushButton_wlan0_IP_clicked();
    void on_pushButton_wlan0_NM_clicked();
    void on_pushButton_wlan0_GW_clicked();
    void on_pushButton_wlan0_DNS1_clicked();
    void on_pushButton_wlan0_DNS2_clicked();

    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonSaveAll_clicked();

    void on_pushButton_wlan0_pwd_clicked();
    void on_checkBox_eth0_DHCP_clicked(bool checked);
    void on_checkBox_eth1_DHCP_clicked(bool checked);
    void on_checkBox_wlan0_DHCP_clicked(bool checked);
    void on_pushButton_wlan0_enable_clicked();
    void on_comboBox_wlan0_essid_currentIndexChanged(const QString &arg1);

    void on_pushButton_wan0_enable_clicked();
    void on_pushButton_wan0_dialnb_clicked();
    void on_pushButton_wan0_apn_clicked();
    void on_pushButton_wan0_DNS1_clicked();
    void on_pushButton_wan0_DNS2_clicked();

private:
    bool checkNetAddr(char * ipaddr);
    bool saveETH0cfg();
    bool saveETH1cfg();
    bool saveWLAN0cfg();
    bool saveWAN0cfg();

    bool isWlanOn(void);
    bool isWanOn(void);

    bool checkUSBwanKey();
    bool checkUSBwlanKey();

    void updateIcons();

private:
    Ui::net_conf *ui;
    QString wlan0_pwd;
    QString wlan0_essid;
    bool is_eth0_enabled;
    bool is_eth1_enabled;
    bool is_wlan_active;
    bool is_wan_active;
    bool setup;
};

#endif // NET_CONF_H

