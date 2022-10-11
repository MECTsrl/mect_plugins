/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Generic page
 */
#include "app_logprint.h"
#include "atcmplugin.h"
#include "main.h"
#include "net_conf.h"
#include "ui_net_conf.h"
#include "app_config.h"

#include "global_functions.h"

#include <QMessageBox>
#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QProcess>

/**
 * @brief this macro is used to set the net_conf style.
 * the syntax is html stylesheet-like
 */
#define SET_NET_CONF_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    /* add there the page stylesheet customization */ \
    mystyle.append(""); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instantiation of the page.
 */
net_conf::net_conf(QWidget *parent) :
    page(parent),
    ui(new Ui::net_conf)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    
    /* set up the page style */
    //SET_PAGE_STYLE();
    /* set the style described into the macro SET_net_conf_STYLE */
    SET_NET_CONF_STYLE();
    translateFontSize(this);
    is_loading = true;
    is_eth0_enabled = false;
    is_WifiScanning = false;
    is_WanStarting = false;
    saveEth0 = false;
    saveWlan0 = false;
    saveWan = false;
    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NO_IP);
    ui->pushButton_hidden_wlan0->setText(NO_IP);
    ui->checkBox_hiddenESSID->setChecked(false);
    ui->pushButton_hidden_wlan0->setVisible(false);
    wlan0_essid.clear();
    wlan0_pwd.clear();
}

void net_conf::on_pushButtonHome_clicked()
{
    go_home();
}

void net_conf::on_pushButtonBack_clicked()
{
    go_back();
}

/* ETH0 */
bool net_conf::saveETH0cfg()
{
    // check board enabled or update needed ---> Nothing to do
    if (!is_eth0_enabled  || ! saveEth0)  {
        fprintf(stderr, "Lan0: no update needed, return\n");
        return true;
    }

    /* DHCP */
    if (ui->checkBox_eth0_DHCP->isChecked())
    {
        if (app_netconf_item_set("[DHCP]", "BOOTPROTO0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTO0"));
            return false;
        }
    }
    else
    {
        if (app_netconf_item_set("[none]", "BOOTPROTO0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTO0"));
            return false;
        }
        /* IP */
        if (ui->pushButton_eth0_IP->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_eth0_IP->text().toAscii().data(), "IPADDR0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDR0"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_eth0_GW->text().compare(NO_IP) != 0)
        {
            QString     szGW = ui->pushButton_eth0_GW->text();
            if (szGW == ZERO_IP)
                szGW = "";
            if (app_netconf_item_set(szGW.toAscii().data(), "GATEWAY0"))
            {
                /* error */
                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAY0"));
                return false;
            }
        }
        /* NETMASK */
        if (ui->pushButton_eth0_NM->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_eth0_NM->text().toAscii().data(), "NETMASK0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASK0"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_eth0_DNS1->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_eth0_DNS1->text().toAscii().data(), "NAMESERVER01"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVER01"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_eth0_DNS2->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_eth0_DNS2->text().toAscii().data(), "NAMESERVER02"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVER02"));
            return false;
        }
    }
    if (system("/etc/rc.d/init.d/network restart >/dev/null 2>&1"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the eth0 network configuration."));
        return false;
    }
    saveEth0 = false;
    return true;
}

/* WLAN0 */
bool net_conf::saveWLAN0cfg()
{

    // check update needed ---> Nothing to do
    if (! saveWlan0)  {
        fprintf(stderr, "WLan0: no update needed, return\n");
        return true;
    }
    QString szVal;
    /* ESSID  (save only if not "-" )*/
    if (! wlan0_essid.isEmpty() && wlan0_essid.compare(NO_IP) != 0)  {
        szVal = QString("\"") + wlan0_essid.trimmed() + QString("\"");
        if (app_netconf_item_set(szVal.toAscii().data(), "ESSIDW0"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nESSIDW0"));
            return false;
        }
    }
    /* PASSWORD */
    szVal = QString("\"") + wlan0_pwd.trimmed() + QString("\"");
    if (app_netconf_item_set(szVal.toAscii().data(), "PASSWORDW0"))  {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nPASSWORDW0"));
        return false;
    }
    /* DHCP */
    if (ui->checkBox_wlan0_DHCP->isChecked())  {
        if (app_netconf_item_set("[DHCP]", "BOOTPROTOW0"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTOW0"));
            return false;
        }
    }
    else  {
        // Static IP, disable DHCP
        if (app_netconf_item_set("[none]", "BOOTPROTOW0"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTOW0"));
            return false;
        }
        /* IP (save only if not "-" )*/
        szVal = ui->pushButton_wlan0_IP->text().trimmed();
        if (szVal.compare(NO_IP) != 0 && app_netconf_item_set(szVal.toAscii().data(), "IPADDRW0"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDRW0"));
            return false;
        }
        /* GATEWAY */
        szVal = ui->pushButton_wlan0_GW->text().trimmed();
        if (szVal== ZERO_IP)  {
            szVal.clear();
        }
        if (app_netconf_item_set(szVal.toAscii().data(), "GATEWAYW0"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAYW0"));
            return false;
        }
        /* NETMASK (save only if not "-" )*/
        szVal = ui->pushButton_wlan0_NM->text().trimmed();
        if (szVal.compare(NO_IP) != 0 && app_netconf_item_set(szVal.toAscii().data(), "NETMASKW0"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASKW0"));
            return false;
        }
        /* DNS1 (save only if not "-" )*/
        szVal = ui->pushButton_wlan0_DNS1->text().trimmed();
        if (szVal.compare(NO_IP) != 0 && app_netconf_item_set(szVal.toAscii().data(), "NAMESERVERW01"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW01"));
            return false;
        }
        /* DNS2 (save only if not "-" )*/
        szVal = ui->pushButton_wlan0_DNS2->text().trimmed();
        if (szVal.compare(NO_IP) != 0 && app_netconf_item_set(szVal.toAscii().data(), "NAMESERVERW02"))  {
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW02"));
            return false;
        }
    }
    // Stop Service
    char command[256];
    sprintf(command, "/usr/sbin/wifi.sh stop"); // do wait
    fprintf(stderr, "Net Command: [%s]\n", command);
    system(command); // do wait
    // Configure Service (Wait command)
    sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\" >/dev/null 2>&1",
            wlan0_essid.toAscii().data(),
            wlan0_pwd.toAscii().data()
            );
    fprintf(stderr, "Net Command: [%s]\n", command);
    sleep(2);
    system(command);
    saveWlan0 = false;
    return true;
}

/* WAN0 */
bool net_conf::saveWAN0cfg()
{
    if (! saveWan)  {
        fprintf(stderr, "Wan0: no update needed, return\n");
        return true;
    }
    /* DIALNB (save only if not "-" ) */
    if (ui->pushButton_wan0_dialnb->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_dialnb->text().toAscii().data(), "DIALNBP0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nDIALNBP0"));
        return false;
    }
    /* APN (save only if not "-" ) */
    if (ui->pushButton_wan0_apn->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_apn->text().toAscii().data(), "APNP0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nAPNP0"));
        return false;
    }
    /* DNS1 (save only if not "-" ) */
    if (ui->pushButton_wan0_DNS1->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_DNS1->text().toAscii().data(), "NAMESERVERP01"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERP01"));
        return false;
    }
    /* DNS2 (save only if not "-" ) */
    if (ui->pushButton_wan0_DNS2->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_DNS2->text().toAscii().data(), "NAMESERVERP02"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERP02"));
        return false;
    }
    // Stop Service
    char command[256];
    sprintf(command, "/usr/sbin/usb3g.sh stop");
    system(command); // do wait
    fprintf(stderr, "Net Command: [%s]\n", command);
    // Configure Service (Wait command)
    sprintf(command, "/usr/sbin/usb3g.sh setup \"%s\" \"%s\" >/dev/null 2>&1",
            ui->pushButton_wan0_dialnb->text().toAscii().data(),
            ui->pushButton_wan0_apn->text().toAscii().data()
            );
    fprintf(stderr, "Net Command: [%s]\n", command);
    sleep(2);
    system(command);
    saveWan = false;
    return true;
}

void net_conf::on_pushButtonSaveAll_clicked()
{
    /* save all pages if needed */
    if (
            saveETH0cfg() &&
            saveWLAN0cfg() &&
            saveWAN0cfg()
            )
    {
        QMessageBox::information(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("The new configuration is saved and active."));
    }
}


/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void net_conf::reload()
{
    char *tmp = NULL;

    QNetworkAddressEntry    netAddr;
    unsigned                myGW = 0;

    is_loading = true;
    ui->comboBox_wlan0_essid->blockSignals(true);
    ui->pushButton_wlan0_enable->blockSignals(true);
    // eth0
    is_eth0_enabled = (system("grep -c INTERFACE0 /etc/rc.d/rc.conf >/dev/null 2>&1") == 0);
    ui->tab_eth0->setEnabled(is_eth0_enabled);
    ui->tabWidget->setCurrentIndex(0);      // Default on lan0
    // ui->tabWidget->setVisible(is_eth0_enabled);

    // wlan0
    is_wlan_active = isWlanOn();

    // ppp0
    is_wan_active = isWanOn();

    // eth0
    if (is_eth0_enabled)  {
        /* ETH0 */
        /* DHCP */
        if (app_netconf_item_get(&tmp, "BOOTPROTO0") != NULL && strcmp(tmp, "[DHCP]") == 0)  {
            ui->checkBox_eth0_DHCP->setChecked(true);
            on_checkBox_eth0_DHCP_clicked(true);
        }
        else  {
            ui->checkBox_eth0_DHCP->setChecked(false);
            on_checkBox_eth0_DHCP_clicked(false);
        }
        // IP - NETMASK - GATEWAY
        if (! ui->checkBox_eth0_DHCP->isChecked())  {
            // IP
            if (app_netconf_item_get(&tmp, "IPADDR0") != NULL && tmp[0] != '\0')  {
                ui->pushButton_eth0_IP->setText(QString(tmp).trimmed());
            }
            else {
                ui->pushButton_eth0_IP->setText(NO_IP);
            }
            // NETMASK
            if (app_netconf_item_get(&tmp, "NETMASK0") != NULL && tmp[0] != '\0')  {
                ui->pushButton_eth0_NM->setText(QString(tmp).trimmed());
            }
            else {
                ui->pushButton_eth0_NM->setText(NO_IP);
            }
            // GATEWAY
            if (app_netconf_item_get(&tmp, "GATEWAY0") != NULL && tmp[0] != '\0')  {
                ui->pushButton_eth0_GW->setText(QString(tmp).trimmed());
            }
            else {
                ui->pushButton_eth0_GW->setText(NO_IP);
            }
        }
        else  {
            // Read current DHCP settings
            // IP - NETMASK
            if (getBoardIPInfo("eth0", netAddr))  {
                ui->pushButton_eth0_IP->setText(netAddr.ip().toString());
                ui->pushButton_eth0_NM->setText(netAddr.netmask().toString());
            }
            else  {
                ui->pushButton_eth0_IP->setText(NO_IP);
                ui->pushButton_eth0_NM->setText(NO_IP);
            }
            /* GATEWAY */
            if (getBoardGateway("eth0", myGW))  {
                ui->pushButton_eth0_GW->setText(QHostAddress(ntohl(myGW)).toString());
            }
            else  {
                ui->pushButton_eth0_GW->setText(NO_IP);
            }
        }
        /* DNS1 */
        if (app_netconf_item_get(&tmp, "NAMESERVER01") != NULL && tmp[0] != '\0')  {
            ui->pushButton_eth0_DNS1->setText(tmp);
        }
        else  {
            ui->pushButton_eth0_DNS1->setText(NO_IP);
        }
        /* DNS2 */
        if (app_netconf_item_get(&tmp, "NAMESERVER02") != NULL && tmp[0] != '\0')  {
            ui->pushButton_eth0_DNS2->setText(tmp);
        }
        else  {
            ui->pushButton_eth0_DNS2->setText(NO_IP);
        }
        /* MAC */
        ui->label_eth0_MAC->setText(getMacAddr("eth0"));
    }


    /* WLAN0 */
    /* ESSID */
    if (app_netconf_item_get(&tmp, "ESSIDW0") != NULL && tmp[0] != '\0')  {
        wlan0_essid = QString(tmp).mid(1, strlen(tmp)-2).trimmed();
        int index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
        if (index  <  0) {
            ui->comboBox_wlan0_essid->addItem(wlan0_essid);
            index = ui->comboBox_wlan0_essid->count() - 1;
        }
        ui->comboBox_wlan0_essid->setCurrentIndex(index);
        ui->pushButton_hidden_wlan0->setText(wlan0_essid);
    }
    else {
        ui->pushButton_hidden_wlan0->setText(NO_IP);
    }

    /* PASSWORD */
    if (app_netconf_item_get(&tmp, "PASSWORDW0") != NULL && tmp[0] != '\0')  {
        wlan0_pwd = QString(tmp).mid(1, strlen(tmp)-2);
        ui->pushButton_wlan0_pwd->setText(QString("*").repeated(wlan0_pwd.length()));
    }
    else    {
        wlan0_pwd.clear();
        ui->pushButton_wlan0_pwd->setText(NO_IP);
    }
    //        if (! wlan0_essid.isEmpty() && ! wlan0_pwd.isEmpty())
    //        {
    //            char command[256];
    //            sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\" >/dev/null 2>&1",
    //                    wlan0_essid.toAscii().data(),
    //                    wlan0_pwd.toAscii().data()
    //                    );
    //            if (system(command))
    //            {
    //                /* error */
    //                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the wifi network configuration for '%1'").arg(wlan0_essid));
    //                return;
    //            }
    //        }

    /* DHCP */
    if (app_netconf_item_get(&tmp, "BOOTPROTOW0") != NULL && strcmp(tmp, "[DHCP]") == 0)  {
        ui->checkBox_wlan0_DHCP->setChecked(true);
        on_checkBox_wlan0_DHCP_clicked(true);
    }
    else  {
        ui->checkBox_wlan0_DHCP->setChecked(false);
        on_checkBox_wlan0_DHCP_clicked(false);
    }
    // IP - NETMASK - GATEWAY
    if (! ui->checkBox_wlan0_DHCP->isChecked())  {
        /* IP */
        if (app_netconf_item_get(&tmp, "IPADDRW0") != NULL && tmp[0] != '\0')  {
            ui->pushButton_wlan0_IP->setText(tmp);
        }
        else  {
            ui->pushButton_wlan0_IP->setText(NO_IP);
        }
        /* NETMASK */
        if (app_netconf_item_get(&tmp, "NETMASKW0") != NULL && tmp[0] != '\0')  {
            ui->pushButton_wlan0_NM->setText(tmp);
        }
        else  {
            ui->pushButton_wlan0_NM->setText(NO_IP);
        }
        /* GATEWAY */
        if (app_netconf_item_get(&tmp, "GATEWAYW0") != NULL && tmp[0] != '\0')  {
            ui->pushButton_wlan0_GW->setText(tmp);
        }
        else  {
            ui->pushButton_wlan0_GW->setText(NO_IP);
        }
    }
    else  {
        // Read current DHCP settings
        // IP - NETMASK
        if (is_wlan_active && getBoardIPInfo("wlan0", netAddr))  {
            ui->pushButton_wlan0_IP->setText(netAddr.ip().toString());
            ui->pushButton_wlan0_NM->setText(netAddr.netmask().toString());
        }
        else  {
            ui->pushButton_wlan0_IP->setText(NO_IP);
            ui->pushButton_wlan0_NM->setText(NO_IP);
        }
        /* GATEWAY */
        if (is_wlan_active && getBoardGateway("wlan0", myGW))  {
            ui->pushButton_wlan0_GW->setText(QHostAddress(ntohl(myGW)).toString());
        }
        else  {
            ui->pushButton_wlan0_GW->setText(NO_IP);
        }
    }

    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVERW01") != NULL && tmp[0] != '\0')  {
        ui->pushButton_wlan0_DNS1->setText(tmp);
    }
    else  {
        ui->pushButton_wlan0_DNS1->setText(NO_IP);
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVERW02") != NULL && tmp[0] != '\0')  {
        ui->pushButton_wlan0_DNS2->setText(tmp);
    }
    else  {
        ui->pushButton_wlan0_DNS2->setText(NO_IP);
    }
    /* MAC */
    if (check_wifi_board())  {
        ui->label_wlan0_MAC->setText(getMacAddr("wlan0"));
    }
    else
    {
        ui->label_wlan0_MAC->setText(NO_IP);
    }

    /* WAN0 */
    /* DIALNB */
    if (app_netconf_item_get(&tmp, "DIALNBP0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_dialnb->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_dialnb->setText(NO_IP);
    }

    /* APN */
    if (app_netconf_item_get(&tmp, "APNP0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_apn->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_apn->setText(NO_IP);
    }
    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVERP01") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_DNS1->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_DNS1->setText(NO_IP);
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVERP02") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_DNS2->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_DNS2->setText(NO_IP);
    }
    // Reset Update flags
    saveEth0 = false;
    saveWlan0 = false;
    saveWan = false;
    // End of loading
    ui->comboBox_wlan0_essid->blockSignals(false);
    ui->pushButton_wlan0_enable->blockSignals(false);
    is_loading = false;
    updateIcons();
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void net_conf::updateData()
{
    static int nLoop = 0;

    if (this->isVisible() == false)
    {
        return;
    }
    /* call the parent updateData member */
    page::updateData();
    
    // Refresh ogni 2s
    if (! is_loading && (++nLoop % 4) == 0)  {
        nLoop = 0;
        updateIcons();
    }
}

void net_conf::updateIcons()
{
    // lan0 (refresh IP only if DHCP enabled)
    if (is_eth0_enabled && ui->checkBox_eth0_DHCP->isChecked())  {
        ui->pushButton_eth0_IP->setText(getIPAddr("lan0"));
    }

    // wlan0
    bool is_wlan_present = check_wifi_board();

    is_wlan_active = is_wlan_present && isWlanOn();
    ui->tab_wlan0->setEnabled(is_wlan_present && ! is_WifiScanning);
    if (is_wlan_present)  {
        // Refresh MAC Address if needed
        if (ui->label_wlan0_MAC->text() == NO_IP)  {
            ui->label_wlan0_MAC->setText(getMacAddr("wlan0"));
        }
    }
    else  {
        ui->label_wlan0_MAC->setText(NO_IP);
    }
    // wlan0 is connected, get connection info
    if (! is_WifiScanning)  {
        if (is_wlan_active)  {
            ui->label_Wlan_connect->setText("Disconnect");
            ui->pushButton_wlan0_enable->setIcon(QIcon(":/libicons/img/WifiOn.png"));
            // Signal Level
            int nQuality = 0;
            int nSignalLevel = 0;
            if (get_wifi_signal_level(nQuality, nSignalLevel))  {
                ui->label_wlan0_signal->setText(QString("%1%") .arg(nQuality));
            }
            else  {
                ui->label_wlan0_signal->setText(NO_IP);
            }
            // Refresh IP
            if (ui->checkBox_wlan0_DHCP->isChecked())  {
                ui->pushButton_wlan0_IP->setText(getIPAddr("wlan0"));
            }
        }
        else  {
            ui->label_Wlan_connect->setText("Connect");
            ui->pushButton_wlan0_enable->setIcon(QIcon(":/libicons/img/WifiOff.png"));
            ui->label_wlan0_signal->setText(NO_IP);
            if (ui->checkBox_wlan0_DHCP->isChecked())  {
                ui->pushButton_wlan0_IP->setText(NO_IP);
            }
        }
    }

    // ppp0
    bool is_wan_present = check_usb_wan_board();

    ui->tab_wan0->setEnabled(is_wan_present && ! is_WanStarting);
    is_wan_active = is_wan_present && isWanOn();
    // wan0 is connected, get connection info
    if (! is_WanStarting)  {
        if (is_wan_active)  {
            ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOn.png"));
            ui->label_wan0_IP->setText(getIPAddr("ppp0"));
            ui->label_wan_connect->setText("Disconnect");
        }
        else  {
            ui->label_wan_connect->setText("Connect");
            ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOff.png"));
            ui->label_wan0_IP->setText(NO_IP);
        }
    }

    // vpn
    //FIXME: Hidden for 3.4.1
    // ui->tab_vpn->setVisible(false);
    // ui->pushButton_vpn_getcert->setVisible(false);
    //FIXME: Hidden for 3.4.1
    /*
    bool  is_vpn_present = check_vpn_board();
    ui->tab_vpn->setEnabled(! is_VpnStarting);
    ui->pushButton_vpn_getcert->setEnabled(! is_vpn_present && ! is_VpnStarting);
    if (is_vpn_present && ! is_VpnStarting)  {
        ui->label_vpn_IP->setText(getIPAddr("tun_mrs"));
        ui->label_vpn_status->setText("UP");
    }
    else  {
        ui->label_vpn_IP->setText(NO_IP);
        ui->label_vpn_status->setText(NO_IP);
    }
    */

    // Status feedback
    /* fprintf(stderr, "Update Icons: Lan Enabled:%d | WLan Present:%d - Active:%d | Wan Present:%d - Active:%d\n",
                    is_eth0_enabled,  is_wlan_present, is_wlan_active, is_wan_present, is_wan_active); */
}

/**
 * @brief This is the event slot to detect new language translation.
 */
void net_conf::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
net_conf::~net_conf()
{
    delete ui;
}


void net_conf::on_pushButton_eth0_IP_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_eth0_IP->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_eth0_IP->text() != QString(value))  {
            ui->pushButton_eth0_IP->setText(value);
            saveEth0 = true;
        }
    }
}

void net_conf::on_pushButton_eth0_NM_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_eth0_NM->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_eth0_NM->text() != QString(value))  {
            ui->pushButton_eth0_NM->setText(value);
            saveEth0 = true;
        }
    }
}

void net_conf::on_pushButton_eth0_GW_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_eth0_GW->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_eth0_GW->text() != QString(value))  {
            ui->pushButton_eth0_GW->setText(value);
            saveEth0 = true;
        }
    }
}

void net_conf::on_pushButton_eth0_DNS1_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_eth0_DNS1->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_eth0_DNS1->text() != QString(value))  {
            ui->pushButton_eth0_DNS1->setText(value);
            saveEth0 = true;
        }
    }
}

void net_conf::on_pushButton_eth0_DNS2_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_eth0_DNS2->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_eth0_DNS2->text() != QString(value))  {
            ui->pushButton_eth0_DNS2->setText(value);
            saveEth0 = true;
        }
    }
}

void net_conf::on_pushButton_wlan0_IP_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wlan0_IP->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_wlan0_IP->text() != QString(value))  {
            ui->pushButton_wlan0_IP->setText(value);
            saveWlan0 = true;
        }
    }
}

void net_conf::on_pushButton_wlan0_NM_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wlan0_NM->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_wlan0_NM->text() != QString(value))  {
            ui->pushButton_wlan0_NM->setText(value);
            saveWlan0 = true;
        }
    }
}

void net_conf::on_pushButton_wlan0_GW_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wlan0_GW->text().toAscii().data());

    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_wlan0_GW->text() != QString(value))  {
            ui->pushButton_wlan0_GW->setText(value);
            saveWlan0 = true;
        }
    }
}

void net_conf::on_pushButton_wlan0_DNS1_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wlan0_DNS1->text().toAscii().data());
    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_wlan0_DNS1->text() != QString(value))  {
            ui->pushButton_wlan0_DNS1->setText(value);
            saveWlan0 = true;
        }
    }
}

void net_conf::on_pushButton_wlan0_DNS2_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wlan0_DNS2->text().toAscii().data());
    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
            if (ui->pushButton_wlan0_DNS2->text() != QString(value))  {
                ui->pushButton_wlan0_DNS2->setText(value);
                saveWlan0 = true;
            }
        }
    }
}

void net_conf::on_pushButton_wlan0_scan_clicked()
{
    int newIndex = 0;
    QString currentAP = wlan0_essid;

    is_WifiScanning = true;
    setEnableWidgets(false);
    ui->comboBox_wlan0_essid->blockSignals(true);
    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem("...Scanning...");

    QString wifiScanResult;
    QProcess wifiScan;
    QCoreApplication::processEvents();
    wifiScan.start("/usr/sbin/wifi.sh scan");
    wifiScan.waitForFinished();

    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NO_IP);
    if (wifiScan.exitCode() != 0)  {
        QMessageBox::critical(0,this->windowTitle(), "Wifi scanning failure!");
        goto endScanWlan;
    }
    else {
        wifiScanResult = QString(wifiScan.readAll());
        if (! wifiScanResult.isEmpty())  {
            // Get Wifi List
            QStringList wifiList = wifiScanResult.split('\n');
            foreach (QString wifiName, wifiList) {
                // First Column is AP Name
                QString apName = wifiName.split('\t').at(0).trimmed();
                if (! apName.isEmpty()) {
                    ui->comboBox_wlan0_essid->addItem(apName);
                    if (apName == currentAP)  {
                        newIndex = ui->comboBox_wlan0_essid->count() - 1;
                    }
                }
            }
        }
    }
    ui->comboBox_wlan0_essid->setCurrentIndex(newIndex);

endScanWlan:
    ui->comboBox_wlan0_essid->blockSignals(false);
    is_WifiScanning = false;
    setEnableWidgets(true);
}


void net_conf::on_pushButton_wlan0_enable_clicked()
{
    is_WifiScanning = true;
    setEnableWidgets(false);
    ui->tab_wlan0->repaint();
    if (isWlanOn()) {
        // Disable WLan0 on next boot
        if (app_netconf_item_set("0", "ONBOOTW0"))  {
            fprintf(stderr, "Error updating ONBOOTW0 to 0 in [%s]\n", APP_CONFIG_IPADDR_FILE);
            /* error */
            goto endWlan0Set;
        }
        // stop service
        fprintf(stderr, "Stopping wlan0 service\n");
        system("/usr/sbin/wifi.sh stop >/dev/null 2>&1 &");
    }
    else {
        // Update Wlan0 Config, stop and reconfigure service
        // WiFi Current Cfg Update forced
        saveWlan0 = true;
        if (saveWLAN0cfg())  {
            // Enable WLan0 on next boot
            if (app_netconf_item_set("1", "ONBOOTW0"))  {
                fprintf(stderr, "Error updating ONBOOTW0 to 1 in [%s]\n", APP_CONFIG_IPADDR_FILE);
                /* error */
                goto endWlan0Set;
            }
            // start service
            fprintf(stderr, "Starting wlan0 service\n");
            system("/usr/sbin/wifi.sh start >/dev/null 2>&1 &");
        }
        else {
            goto endWlan0Set;
        }
    }
    // Pause 2 secs
    sleep (2);
    // Exit point
endWlan0Set:
    is_WifiScanning = false;
    setEnableWidgets(true);
    ui->tab_wlan0->repaint();
    updateIcons();
}

void net_conf::on_pushButton_wlan0_pwd_clicked()
{
    char value [32];

    // Do not show current password !
    alphanumpad pwdKeyboard(value, true, NULL, true);
    pwdKeyboard.showFullScreen();
    if(pwdKeyboard.exec() == QDialog::Accepted)  {
        if (wlan0_pwd.compare(value) != 0)  {
            wlan0_pwd = value;
            ui->pushButton_wlan0_pwd->setText(QString("*").repeated(wlan0_pwd.length()));
            saveWlan0 = true;
        }
    }
}

void net_conf::on_checkBox_eth0_DHCP_clicked(bool checked)
{
    ui->frame_eth0->setEnabled(!checked);
    saveEth0 = true;
}

void net_conf::on_checkBox_wlan0_DHCP_clicked(bool checked)
{
    ui->frame_wlan0->setEnabled(!checked);
    saveWlan0 = true;
}

void net_conf::on_comboBox_wlan0_essid_currentIndexChanged(const QString &arg1)
{
    if (arg1.compare(wlan0_essid) != 0)  {
        saveWlan0 = true;
        wlan0_essid = arg1;
        // Clear Password and previous settings
        if (! is_loading)  {
            wlan0_pwd.clear();
            ui->pushButton_wlan0_pwd->setText(NO_IP);
            if (isWlanOn())  {
                on_pushButton_wlan0_enable_clicked();
                if (ui->checkBox_wlan0_DHCP->isChecked())  {
                    ui->pushButton_wlan0_IP->setText(NO_IP);
                    ui->pushButton_wlan0_NM->setText(NO_IP);
                    ui->pushButton_wlan0_GW->setText(NO_IP);
                }
            }
        }
    }
}

void net_conf::on_pushButton_wan0_enable_clicked()
{
    is_WanStarting = true;
    setEnableWidgets(false);
    ui->tab_wan0->repaint();
    if (isWanOn())
    {
        // Disable WLan0 on next boot
        if (app_netconf_item_set("0", "ONBOOTP0"))  {
            fprintf(stderr, "Error updating ONBOOTP0 to 0 in [%s]\n", APP_CONFIG_IPADDR_FILE);
            goto endWanSet;
        }
        // stop service
        fprintf(stderr, "Stopping wan0 service\n");
        system("/usr/sbin/usb3g.sh stop >/dev/null 2>&1 &");
    }
    else
    {
        // Update Wan0 Config, stop and reconfigure service
        // Mobile Current Cfg Update forced
        saveWan = true;
        if (saveWAN0cfg())  {
            // Enable Wan0 on next boot
            if (app_netconf_item_set("1", "ONBOOTP0"))  {
                fprintf(stderr, "Error updating ONBOOTP0 to 1 in [%s]\n", APP_CONFIG_IPADDR_FILE);
                /* error */
                goto endWanSet;
            }
            // start service
            fprintf(stderr, "Starting wan0 service\n");
            system("/usr/sbin/usb3g.sh start >/dev/null 2>&1 &");
        }
    }
    // Pause 2 secs
    sleep (2);

    // Exit point
endWanSet:
    is_WanStarting = false;
    setEnableWidgets(true);
    ui->tab_wan0->repaint();
    updateIcons();
}

void net_conf::on_pushButton_wan0_dialnb_clicked()
{
    char value [32];
    numpad dialNumpad(value, DIALNB, ui->pushButton_wan0_dialnb->text().toAscii().data());
    dialNumpad.showFullScreen();
    if(dialNumpad.exec()==QDialog::Accepted)  {
        if (ui->pushButton_wan0_dialnb->text() != QString(value))  {
            ui->pushButton_wan0_dialnb->setText(value);
            saveWan = true;
        }
    }
}

void net_conf::on_pushButton_wan0_apn_clicked()
{
    char value [32];
    alphanumpad apnKeyboard(value, true, ui->pushButton_wan0_apn->text().toAscii().data());

    apnKeyboard.showFullScreen();
    if(apnKeyboard.exec()==QDialog::Accepted)    {
        if (ui->pushButton_wan0_apn->text() != QString(value))  {
            ui->pushButton_wan0_apn->setText(value);
            saveWan = true;
        }
    }
}

void net_conf::on_pushButton_wan0_DNS1_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wan0_DNS1->text().toAscii().data());
    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_wan0_DNS1->text() != QString(value))  {
            ui->pushButton_wan0_DNS1->setText(value);
            saveWan = true;
        }
    }
}

void net_conf::on_pushButton_wan0_DNS2_clicked()
{
    char value [32];
    numpad ipNumpad(value, IPADDR, ui->pushButton_wan0_DNS2->text().toAscii().data());
    ipNumpad.showFullScreen();
    if(ipNumpad.exec()==QDialog::Accepted && checkNetAddr(value))  {
        if (ui->pushButton_wan0_DNS2->text() != QString(value))  {
            ui->pushButton_wan0_DNS2->setText(value);
            saveWan = true;
        }
    }
}


void net_conf::on_pushButton_hidden_wlan0_clicked()
{
    char value [64];
    QString currentText = ui->pushButton_hidden_wlan0->text();
    if(currentText.isEmpty()) {
       currentText = NO_IP;
    }
    alphanumpad hiddenLanKB(value, true, currentText.toAscii().data());
    hiddenLanKB.showFullScreen();
    if(hiddenLanKB.exec()==QDialog::Accepted)  {
        if (currentText != QString(value))  {
            wlan0_essid = QString(value);
            ui->pushButton_hidden_wlan0->setText(wlan0_essid);
            saveWlan0 = true;
        }
    }
}

void net_conf::on_checkBox_hiddenESSID_toggled(bool checked)
{
    QString szVal;
    QString szNewVal;

    if (checked)  {
        ui->comboBox_wlan0_essid->setEnabled(false);
        ui->comboBox_wlan0_essid->blockSignals(true);
        ui->pushButton_hidden_wlan0->setVisible(true);
        ui->pushButton_wlan0_scan->setEnabled(false);
        szVal = ui->pushButton_hidden_wlan0->text().trimmed();
        // Recuper ESSID da combo a Text
        if (szVal == QString(NO_IP))  {
            ui->pushButton_hidden_wlan0->text() = ui->comboBox_wlan0_essid->currentText().trimmed();
        }
        szNewVal = ui->pushButton_hidden_wlan0->text();
    }
    else  {
        ui->pushButton_hidden_wlan0->setVisible(false);
        ui->pushButton_wlan0_scan->setEnabled(true);
        // Recupera ESSID da Hidden a Combo
        szVal = ui->pushButton_hidden_wlan0->text().trimmed();
        if (! szVal.isEmpty() && szVal != QString(NO_IP) &&  ui->comboBox_wlan0_essid->findText(szVal) < 0)  {
            ui->comboBox_wlan0_essid->addItem(szVal);
            ui->comboBox_wlan0_essid->setCurrentIndex(ui->comboBox_wlan0_essid->count() - 1);
        }
        szNewVal = ui->comboBox_wlan0_essid->currentText();
        ui->comboBox_wlan0_essid->blockSignals(false);
        ui->comboBox_wlan0_essid->setEnabled(true);
    }
    // Set new value
    if (szNewVal != wlan0_essid)  {
        wlan0_essid = szNewVal;
        saveWlan0 = true;
    }
}

void net_conf::setEnableWidgets(bool isEnabled)
{
    ui->pushButtonSaveAll->setEnabled(isEnabled);
    ui->pushButtonHome->setEnabled(isEnabled);
    ui->pushButtonBack->setEnabled(isEnabled);
    ui->tab_eth0->setEnabled(isEnabled);
    ui->tab_wlan0->setEnabled(isEnabled);
    ui->tab_wan0->setEnabled(isEnabled);
}
