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
    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NO_IP);
    ui->pushButton_hidden_wlan0->setText(NO_IP);
    ui->checkBox_hiddenESSID->setChecked(false);
    ui->pushButton_hidden_wlan0->setVisible(false);
    wlan0_essid = "";
    wlan0_pwd = "";
    is_loading = true;
    is_eth0_enabled = false;
    is_WifiScanning = false;
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
    if (!is_eth0_enabled)
    {
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
    return true;
}

/* WLAN0 */
bool net_conf::saveWLAN0cfg()
{
    /* ESSID */
    if (wlan0_essid.compare(NO_IP) != 0 && app_netconf_item_set(QString(QString("\"") + wlan0_essid + QString("\"")).toAscii().data(), "ESSIDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nESSIDW0"));
        return false;
    }
    /* PASSWORD */
    if (wlan0_pwd.compare(NO_IP) != 0 && app_netconf_item_set(QString(QString("\"") + wlan0_pwd + QString("\"")).toAscii().data(), "PASSWORDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nPASSWORDW0"));
        return false;
    }
    /* DHCP */
    if (ui->checkBox_wlan0_DHCP->isChecked())
    {
        if (app_netconf_item_set("[DHCP]", "BOOTPROTOW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTOW0"));
            return false;
        }
    }
    else
    {
        if (app_netconf_item_set("[none]", "BOOTPROTOW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTOW0"));
            return false;
        }
        /* IP */
        if (ui->pushButton_wlan0_IP->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wlan0_IP->text().toAscii().data(), "IPADDRW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDRW0"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_wlan0_GW->text().compare(NO_IP) != 0)
        {
            QString     szGW = ui->pushButton_wlan0_GW->text();
            if (szGW == ZERO_IP)
                szGW = "";
            if(app_netconf_item_set(szGW.toAscii().data(), "GATEWAYW0"))
            {
                /* error */
                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAYW0"));
                return false;
            }
        }
        /* NETMASK */
        if (ui->pushButton_wlan0_NM->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wlan0_NM->text().toAscii().data(), "NETMASKW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASKW0"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_wlan0_DNS1->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wlan0_DNS1->text().toAscii().data(), "NAMESERVERW01"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW01"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_wlan0_DNS2->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wlan0_DNS2->text().toAscii().data(), "NAMESERVERW02"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW02"));
            return false;
        }
    }
    char command[256];
    system("/usr/sbin/wifi.sh stop"); // do wait
    sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\" >/dev/null 2>&1 &",
            wlan0_essid.toAscii().data(),
            wlan0_pwd.toAscii().data()
            );
    system(command);
//    if (system(command))
//    {
//        /* error */
//        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the wifi network configuration for '%1'").arg(wlan0_essid));
//        return false;
//    }
    return true;
}

/* WAN0 */
bool net_conf::saveWAN0cfg()
{
    /* DIALNB */
    if (ui->pushButton_wan0_dialnb->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_dialnb->text().toAscii().data(), "DIALNBP0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nDIALNBP0"));
        return false;
    }
    /* APN */
    if (ui->pushButton_wan0_apn->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_apn->text().toAscii().data(), "APNP0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nAPNP0"));
        return false;
    }
    /* DNS1 */
    if (ui->pushButton_wan0_DNS1->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_DNS1->text().toAscii().data(), "NAMESERVERP01"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERP01"));
        return false;
    }
    /* DNS2 */
    if (ui->pushButton_wan0_DNS2->text().compare(NO_IP) != 0 && app_netconf_item_set(ui->pushButton_wan0_DNS2->text().toAscii().data(), "NAMESERVERP02"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERP02"));
        return false;
    }
    char command[256];
    system("/usr/sbin/usb3g.sh stop"); // do wait
    sprintf(command, "/usr/sbin/usb3g.sh setup \"%s\" \"%s\" >/dev/null 2>&1 ",
            ui->pushButton_wan0_dialnb->text().toAscii().data(),
            ui->pushButton_wan0_apn->text().toAscii().data()
            );
    system(command);
//    if (system(command))
//    {
//        /* error */
//        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the ppp network configuration for '%1'").arg(ui->pushButton_wan0_dialnb->text()));
//        return false;
//    }
    return true;
}

void net_conf::on_pushButtonSaveAll_clicked()
{
    /* save all pages */
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
    // lan0 (refresh IP if DHCP enabled)
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
        is_wlan_active = false;
        ui->label_wlan0_MAC->setText(NO_IP);
    }
    // wlan0 is connected, get connection info
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

    // ppp0
    bool is_wan_present = check_usb_wan_board();

    ui->tab_wan0->setEnabled(is_wan_present);
    is_wan_active = is_wan_present && isWanOn();
    // wan0 is connected, get connection info
    if (is_wlan_active)  {
        ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOn.png"));
        ui->label_wan0_IP->setText(getIPAddr("ppp0"));
    }
    else  {
        ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOff.png"));
        ui->label_wan0_IP->setText(NO_IP);
    }
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
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth0_IP->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_IP->setText(value);
    }
}

void net_conf::on_pushButton_eth0_NM_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth0_NM->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_NM->setText(value);
    }
}

void net_conf::on_pushButton_eth0_GW_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth0_GW->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_GW->setText(value);
    }
}

void net_conf::on_pushButton_eth0_DNS1_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth0_DNS1->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_eth0_DNS2_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth0_DNS2->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_IP_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wlan0_IP->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_IP->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_NM_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wlan0_NM->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_NM->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_GW_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wlan0_GW->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_GW->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_DNS1_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wlan0_DNS1->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_DNS2_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wlan0_DNS2->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_scan_clicked()
{
    int newIndex = 0;
    QString currentAP = wlan0_essid;

    is_WifiScanning = true;
    setEnableWidgets(false);
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
        QMessageBox::critical(this,this->windowTitle(), "Wifi scanning failure!");
        return;
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
                    qDebug("AP Found: [%s] vs [%s]", apName.toLatin1().data(), currentAP.toLatin1().data());
                    ui->comboBox_wlan0_essid->addItem(apName);
                    if (apName == currentAP)  {
                        newIndex = ui->comboBox_wlan0_essid->count() - 1;
                        qDebug("Item Found: [%s] @ pos: [%d]", apName.toLatin1().data(), newIndex);
                    }
                }
            }
        }
    }
    ui->comboBox_wlan0_essid->setCurrentIndex(newIndex);
    is_WifiScanning = false;
    setEnableWidgets(true);
}


void net_conf::on_pushButton_wlan0_enable_clicked()
{
    ui->tab_wlan0->setEnabled(false);
    ui->tab_wlan0->repaint();
    // WiFi Current Cfg Update forced
    saveWLAN0cfg();
    if (!is_wlan_active)
    {
        if (app_netconf_item_set("1", "ONBOOTW0"))
        {
            /* error */
            ui->tab_wlan0->setEnabled(true);
            ui->tab_wlan0->repaint();
            return;
        }
        system("/usr/sbin/wifi.sh start >/dev/null 2>&1 &");
    }
    else
    {
        if (app_netconf_item_set("0", "ONBOOTW0"))
        {
            /* error */
            ui->tab_wlan0->setEnabled(true);
            ui->tab_wlan0->repaint();
            return;
        }
        system("/usr/sbin/wifi.sh stop >/dev/null 2>&1 &");
    }

    ui->tab_wlan0->setEnabled(true);
    ui->tab_wlan0->repaint();
    updateIcons();
}

void net_conf::on_pushButton_wlan0_pwd_clicked()
{
    char value [32];
    alphanumpad tastiera_alfanum(value, wlan0_pwd.toAscii().data(), true);
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted)
    {
        wlan0_pwd = value;
        ui->pushButton_wlan0_pwd->setText(QString("*").repeated(wlan0_pwd.length()));
    }
}

void net_conf::on_checkBox_eth0_DHCP_clicked(bool checked)
{
    ui->frame_eth0->setEnabled(!checked);
}

void net_conf::on_checkBox_wlan0_DHCP_clicked(bool checked)
{
    ui->frame_wlan0->setEnabled(!checked);
}

void net_conf::on_comboBox_wlan0_essid_currentIndexChanged(const QString &arg1)
{
    wlan0_essid = arg1;
}

void net_conf::on_pushButton_wan0_enable_clicked()
{
    ui->tab_wan0->setEnabled(false);
    ui->tab_wan0->repaint();
    // Mobile Current Cfg Update forced
    saveWAN0cfg();
    if (!is_wan_active)
    {
        if (app_netconf_item_set("1", "ONBOOTP0"))
        {
            /* error */
            ui->tab_wan0->setEnabled(true);
            ui->tab_wan0->repaint();
            return;
        }
        system("/usr/sbin/usb3g.sh start >/dev/null 2>&1 &");
    }
    else
    {
        if (app_netconf_item_set("0", "ONBOOTP0"))
        {
            /* error */
            ui->tab_wan0->setEnabled(true);
            ui->tab_wan0->repaint();
            return;
        }
        system("/usr/sbin/usb3g.sh stop >/dev/null 2>&1 &");
    }

    ui->tab_wan0->setEnabled(true);
    ui->tab_wan0->repaint();
    updateIcons();
}

void net_conf::on_pushButton_wan0_dialnb_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, DIALNB, ui->pushButton_wan0_dialnb->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted)
    {
        ui->pushButton_wan0_dialnb->setText(value);
    }
}

void net_conf::on_pushButton_wan0_apn_clicked()
{
    char value [32];
    alphanumpad tastiera_alfanum(value, ui->pushButton_wan0_apn->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted)
    {
        ui->pushButton_wan0_apn->setText(value);
    }
}

void net_conf::on_pushButton_wan0_DNS1_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wan0_DNS1->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wan0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_wan0_DNS2_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_wan0_DNS2->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wan0_DNS2->setText(value);
    }
}


void net_conf::on_pushButton_hidden_wlan0_clicked()
{
    char value [64];
    QString currentText = ui->pushButton_hidden_wlan0->text();
    if(currentText.isEmpty()) {
       currentText = NO_IP;
    }
    alphanumpad tastiera_alfanum(value, currentText.toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted)
    {
        QString szValue = QString(value).trimmed();
        ui->pushButton_hidden_wlan0->setText(szValue);
        wlan0_essid = szValue;
    }
}

void net_conf::on_checkBox_hiddenESSID_toggled(bool checked)
{
    if (checked)  {
        ui->comboBox_wlan0_essid->setEnabled(false);
        ui->pushButton_hidden_wlan0->setVisible(true);
        ui->pushButton_wlan0_scan->setEnabled(false);
        wlan0_essid = ui->pushButton_hidden_wlan0->text();
    }
    else  {
        ui->comboBox_wlan0_essid->setEnabled(true);
        ui->pushButton_hidden_wlan0->setVisible(false);
        ui->pushButton_wlan0_scan->setEnabled(true);
        wlan0_essid = ui->comboBox_wlan0_essid->currentText();
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
