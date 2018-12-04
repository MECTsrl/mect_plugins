/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
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
#include <QMessageBox>

#define NONE     "-"
#define NONE_LEN 1
#define ZEROIP "0.0.0.0"
/**
 * @brief this macro is used to set the net_conf style.
 * the syntax is html stylesheet-like
 */
#define SET_NET_CONF_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    /* add there the page stilesheet customization */ \
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
    ui->comboBox_wlan0_essid->addItem(NONE);
    is_eth0_enabled = (system("grep -c INTERFACE0 /etc/rc.d/rc.conf >/dev/null 2>&1") == 0);
    is_eth1_enabled = (system("grep -c INTERFACE1 /etc/rc.d/rc.conf >/dev/null 2>&1") == 0);
    ui->tab_eth0->setEnabled(is_eth0_enabled);
    ui->tab_eth1->setEnabled(is_eth1_enabled);
    if (!is_eth1_enabled)
    {
        ui->tabWidget->removeTab(1);
    }
    if (!is_eth0_enabled)
    {
        ui->tabWidget->removeTab(0);
    }
    wlan0_essid = "";
    wlan0_pwd = "";
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
        if (ui->pushButton_eth0_IP->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_IP->text().toAscii().data(), "IPADDR0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDR0"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_eth0_GW->text().compare(NONE) != 0)
        {
            QString     szGW = ui->pushButton_eth0_GW->text();
            if (szGW == ZEROIP)
                szGW = "";
            if (app_netconf_item_set(szGW.toAscii().data(), "GATEWAY0"))
            {
                /* error */
                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAY0"));
                return false;
            }
        }
        /* NETMASK */
        if (ui->pushButton_eth0_NM->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_NM->text().toAscii().data(), "NETMASK0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASK0"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_eth0_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_DNS1->text().toAscii().data(), "NAMESERVER01"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVER01"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_eth0_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_DNS2->text().toAscii().data(), "NAMESERVER02"))
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

/* ETH1 */
bool net_conf::saveETH1cfg()
{
    if (!is_eth1_enabled)
    {
        return true;
    }
    /* DHCP */
    if (ui->checkBox_eth1_DHCP->isChecked())
    {
        if (app_netconf_item_set("[DHCP]", "BOOTPROTO1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTO1"));
            return false;
        }
    }
    else
    {
        if (app_netconf_item_set("[none]", "BOOTPROTO1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nBOOTPROTO1"));
            return false;
        }
        /* IP */
        if (ui->pushButton_eth1_IP->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_IP->text().toAscii().data(), "IPADDR1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDR1"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_eth1_GW->text().compare(NONE) != 0)
        {
            QString     szGW = ui->pushButton_eth1_GW->text();
            if (szGW == ZEROIP)
                szGW = "";

            if (app_netconf_item_set(szGW.toAscii().data(), "GATEWAY1"))
            {
                /* error */
                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAY1"));
                return false;
            }
        }
        /* NETMASK */
        if (ui->pushButton_eth1_NM->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_NM->text().toAscii().data(), "NETMASK1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASK1"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_eth1_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_DNS1->text().toAscii().data(), "NAMESERVER11"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVER11"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_eth1_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_DNS2->text().toAscii().data(), "NAMESERVER12"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVER12"));
            return false;
        }
    }
    if (system("/etc/rc.d/init.d/network restart >/dev/null 2>&1"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the eth0 network configuration"));
        return false;
    }
    return true;
}

/* WLAN0 */
bool net_conf::saveWLAN0cfg()
{
    /* ESSID */
    if (wlan0_essid.compare(NONE) != 0 && app_netconf_item_set(QString(QString("\"") + wlan0_essid + QString("\"")).toAscii().data(), "ESSIDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nESSIDW0"));
        return false;
    }
    /* PASSWORD */
    if (wlan0_pwd.compare(NONE) != 0 && app_netconf_item_set(QString(QString("\"") + wlan0_pwd + QString("\"")).toAscii().data(), "PASSWORDW0"))
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
        if (ui->pushButton_wlan0_IP->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_IP->text().toAscii().data(), "IPADDRW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nIPADDRW0"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_wlan0_GW->text().compare(NONE) != 0)
        {
            QString     szGW = ui->pushButton_wlan0_GW->text();
            if (szGW == ZEROIP)
                szGW = "";
            if(app_netconf_item_set(szGW.toAscii().data(), "GATEWAYW0"))
            {
                /* error */
                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nGATEWAYW0"));
                return false;
            }
        }
        /* NETMASK */
        if (ui->pushButton_wlan0_NM->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_NM->text().toAscii().data(), "NETMASKW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNETMASKW0"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_wlan0_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_DNS1->text().toAscii().data(), "NAMESERVERW01"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERW01"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_wlan0_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_DNS2->text().toAscii().data(), "NAMESERVERW02"))
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
    if (ui->pushButton_wan0_dialnb->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wan0_dialnb->text().toAscii().data(), "DIALNBP0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nDIALNBP0"));
        return false;
    }
    /* APN */
    if (ui->pushButton_wan0_apn->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wan0_apn->text().toAscii().data(), "APNP0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nAPNP0"));
        return false;
    }
    /* DNS1 */
    if (ui->pushButton_wan0_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wan0_DNS1->text().toAscii().data(), "NAMESERVERP01"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot update the network configuration\nNAMESERVERP01"));
        return false;
    }
    /* DNS2 */
    if (ui->pushButton_wan0_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wan0_DNS2->text().toAscii().data(), "NAMESERVERP02"))
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
            saveETH1cfg() &&
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

    setup = true;
    is_wlan_active = isWlanOn();
    if (is_wlan_active)
    {
        ui->pushButton_wlan0_enable->setIcon(QIcon(":/libicons/img/WifiOn.png"));
    }
    else
    {
        ui->pushButton_wlan0_enable->setIcon(QIcon(":/libicons/img/WifiOff.png"));
    }

    is_wan_active = isWanOn();
    if (is_wan_active)
    {
        ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOn.png"));
    }
    else
    {
        ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOff.png"));
    }

    setup = false;

    ui->tabWidget->setCurrentIndex(0);

    if (is_eth0_enabled)
    {
        /* ETH0 */
        /* DHCP */
        if (app_netconf_item_get(&tmp, "BOOTPROTO0") != NULL && strcmp(tmp, "[DHCP]") == 0)
        {
            ui->checkBox_eth0_DHCP->setChecked(true);
            on_checkBox_eth0_DHCP_clicked(true);
        }
        else
        {
            ui->checkBox_eth0_DHCP->setChecked(false);
            on_checkBox_eth0_DHCP_clicked(false);
        }
        /* IP */
        if (app_netconf_item_get(&tmp, "IPADDR0") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth0_IP->setText(tmp);
        }
        else
        {
            ui->pushButton_eth0_IP->setText(NONE);
        }
        /* GATEWAY */
        if (app_netconf_item_get(&tmp, "GATEWAY0") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth0_GW->setText(tmp);
        }
        else
        {
            ui->pushButton_eth0_GW->setText(NONE);
        }
        /* NETMASK */
        if (app_netconf_item_get(&tmp, "NETMASK0") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth0_NM->setText(tmp);
        }
        else
        {
            ui->pushButton_eth0_NM->setText(NONE);
        }
        /* DNS1 */
        if (app_netconf_item_get(&tmp, "NAMESERVER01") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth0_DNS1->setText(tmp);
        }
        else
        {
            ui->pushButton_eth0_DNS1->setText(NONE);
        }
        /* DNS2 */
        if (app_netconf_item_get(&tmp, "NAMESERVER02") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth0_DNS2->setText(tmp);
        }
        else
        {
            ui->pushButton_eth0_DNS2->setText(NONE);
        }
        /* MAC */
#if 0
        if (app_macconf_item_get(&tmp, "MAC0") != NULL && tmp[0] != '\0')
        {
            ui->label_eth0_MAC->setText(tmp);
        }
        else
        {
            ui->label_eth0_MAC->setText(NONE);
        }
#else
        char string[32];
        if (getMAC("eth0", string) == 0)
        {
            ui->label_eth0_MAC->setText(string);
        }
        else
        {
            ui->label_eth0_MAC->setText(NONE);
        }
        if (ui->checkBox_eth0_DHCP->isChecked())
        {
            if (getIP("eth0", string) == 0)
            {
                ui->pushButton_eth0_IP->setText(string);
            }
            else
            {
                ui->pushButton_eth0_IP->setText(NONE);
            }
        }
#endif
    }

    if (is_eth1_enabled)
    {
        /* ETH1 */
        /* DHCP */
        if (app_netconf_item_get(&tmp, "BOOTPROTO1") != NULL && strcmp(tmp, "[DHCP]") == 0)
        {
            ui->checkBox_eth1_DHCP->setChecked(true);
            on_checkBox_eth1_DHCP_clicked(true);
        }
        else
        {
            ui->checkBox_eth1_DHCP->setChecked(false);
            on_checkBox_eth1_DHCP_clicked(false);
        }
        /* IP */
        if (app_netconf_item_get(&tmp, "IPADDR1") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth1_IP->setText(tmp);
        }
        else
        {
            ui->pushButton_eth1_IP->setText(NONE);
        }
        /* GATEWAY */
        if (app_netconf_item_get(&tmp, "GATEWAY1") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth1_GW->setText(tmp);
        }
        else
        {
            ui->pushButton_eth1_GW->setText(NONE);
        }
        /* NETMASK */
        if (app_netconf_item_get(&tmp, "NETMASK1") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth1_NM->setText(tmp);
        }
        else
        {
            ui->pushButton_eth1_NM->setText(NONE);
        }
        /* DNS1 */
        if (app_netconf_item_get(&tmp, "NAMESERVER11") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth1_DNS1->setText(tmp);
        }
        else
        {
            ui->pushButton_eth1_DNS1->setText(NONE);
        }
        /* DNS2 */
        if (app_netconf_item_get(&tmp, "NAMESERVER12") != NULL && tmp[0] != '\0')
        {
            ui->pushButton_eth1_DNS2->setText(tmp);
        }
        else
        {
            ui->pushButton_eth1_DNS2->setText(NONE);
        }
        /* MAC */
#if 0
        if (app_macconf_item_get(&tmp, "MAC1") != NULL && tmp[0] != '\0')
        {
            ui->label_eth1_MAC->setText(tmp);
        }
        else
        {
            ui->label_eth1_MAC->setText(NONE);
        }
#else
        char string[32];
        if (getMAC("eth1", string) == 0)
        {
            ui->label_eth1_MAC->setText(string);
        }
        else
        {
            ui->label_eth1_MAC->setText(NONE);
        }
        if (ui->checkBox_eth1_DHCP->isChecked())
        {
            if (getIP("eth1", string) == 0)
            {
                ui->pushButton_eth1_IP->setText(string);
            }
            else
            {
                ui->pushButton_eth1_IP->setText(NONE);
            }
        }
#endif
    }

    /* WLAN0 */
    /* ESSID */
    if (app_netconf_item_get(&tmp, "ESSIDW0") != NULL && tmp[0] != '\0')
    {
        wlan0_essid = QString(tmp).mid(1, strlen(tmp)-2);
        if (wlan0_essid.length() == 0 && is_wlan_active)
        {
            FILE * fp = popen("iwconfig wlan0 2> /dev/null | grep ESSID: | cut -d: -f2", "r");
            if (fp == NULL)
            {
                QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Problem during wifi network scanning"));
                return;
            }

            if (fscanf(fp, tmp) != EOF)
            {
                wlan0_essid = QString(tmp).mid(1, strlen(tmp)-2);
            }

            pclose(fp);
        }
        int index;
        if (wlan0_essid.length())
        {
            index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
        }
        else
        {
            index = 0;
        }
        if (index < 0)
        {
            ui->comboBox_wlan0_essid->addItem(wlan0_essid);
            index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
        }
        if (index != ui->comboBox_wlan0_essid->currentIndex())
        {
            ui->comboBox_wlan0_essid->setCurrentIndex(index);
        }
    }
    else
    {
        ui->comboBox_wlan0_essid->setCurrentIndex(0);
    }

    /* PASSWORD */
    if (app_netconf_item_get(&tmp, "PASSWORDW0") != NULL && tmp[0] != '\0')
    {
        wlan0_pwd = QString(tmp).mid(1, strlen(tmp)-2);
        ui->pushButton_wlan0_pwd->setText(QString("*").repeated(wlan0_pwd.length()));
    }
    else
    {
        wlan0_pwd = tmp;
        ui->pushButton_wlan0_pwd->setText(NONE);
    }

    if (wlan0_essid.length() > 0 && wlan0_pwd.length() > 0)
    {
        char command[256];
        sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\" >/dev/null 2>&1",
                wlan0_essid.toAscii().data(),
                wlan0_pwd.toAscii().data()
                );
        if (system(command))
        {
            /* error */
            QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Cannot setup the wifi network configuration for '%1'").arg(wlan0_essid));
            return;
        }
    }

    /* DHCP */
    if (app_netconf_item_get(&tmp, "BOOTPROTOW0") != NULL && strcmp(tmp, "[DHCP]") == 0)
    {
        ui->checkBox_wlan0_DHCP->setChecked(true);
        on_checkBox_wlan0_DHCP_clicked(true);
    }
    else
    {
        ui->checkBox_wlan0_DHCP->setChecked(false);
        on_checkBox_wlan0_DHCP_clicked(false);
    }
    /* IP */
    if (app_netconf_item_get(&tmp, "IPADDRW0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_IP->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_IP->setText(NONE);
    }
    /* GATEWAY */
    if (app_netconf_item_get(&tmp, "GATEWAYW0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_GW->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_GW->setText(NONE);
    }
    /* NETMASK */
    if (app_netconf_item_get(&tmp, "NETMASKW0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_NM->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_NM->setText(NONE);
    }
    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVERW01") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_DNS1->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_DNS1->setText(NONE);
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVERW02") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_DNS2->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_DNS2->setText(NONE);
    }
    /* MAC */
#if 0
    if (app_macconf_item_get(&tmp, "MACW0") != NULL && tmp[0] != '\0')
    {
        ui->label_wlan0_MAC->setText(tmp);
    }
    else
    {
        ui->label_wlan0_MAC->setText(NONE);
    }
#else
    char string[32];
    if (getMAC("wlan0", string) == 0)
    {
        ui->label_wlan0_MAC->setText(string);
    }
    else
    {
        ui->label_wlan0_MAC->setText(NONE);
    }
#endif

    /* WAN0 */
    /* DIALNB */
    if (app_netconf_item_get(&tmp, "DIALNBP0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_dialnb->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_dialnb->setText(NONE);
    }

    /* APN */
    if (app_netconf_item_get(&tmp, "APNP0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_apn->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_apn->setText(NONE);
    }
    /* IP */
#if 0
#endif
    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVERP01") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_DNS1->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_DNS1->setText(NONE);
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVERP02") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wan0_DNS2->setText(tmp);
    }
    else
    {
        ui->pushButton_wan0_DNS2->setText(NONE);
    }
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void net_conf::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }
    /* call the parent updateData member */
    page::updateData();
    
    /* To read the cross table variable UINT TEST1:
     *    uint_16 tmp = TEST1;
     */
    /* To write 5 into the the cross table variable UINT TEST1:
     *    doWrite_TEST1(5);
     */
    int static count = 0;
    if (REFRESH_MS * count > 1000)
    {
        count = 0;
        updateIcons();
    }
    else
    {
        count ++;
    }

}

void net_conf::updateIcons()
{
    if (checkUSBwanKey())
    {
        ui->tab_wan0->setEnabled(true);
        is_wan_active = isWanOn();
        char ip[32];
//        ui->label_wan0_IP->setText("-");
        if (is_wan_active)
        {
            ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOn.png"));
            if (getIP("ppp0", ip) == 0)
            {
                ui->label_wan0_IP->setText(ip);
            }
            else
            {
                ui->label_wan0_IP->setText(NONE);
            }
        }
        else
        {
            ui->pushButton_wan0_enable->setIcon(QIcon(":/libicons/img/GprsOff.png"));
            ui->label_wan0_IP->setText(NONE);
        }
    }
    else
    {
        ui->tab_wan0->setEnabled(false);
    }

    if (checkUSBwlanKey())
    {
        ui->tab_wlan0->setEnabled(true);
        is_wlan_active = isWlanOn();
        if (is_wlan_active)
        {
            char string[32];
            ui->pushButton_wlan0_enable->setIcon(QIcon(":/libicons/img/WifiOn.png"));
            if (ui->checkBox_wlan0_DHCP->isChecked())
            {
                if (getIP("wlan0", string) == 0)
                {
                    ui->pushButton_wlan0_IP->setText(string);
                }
            }
            if (getMAC("wlan0", string) == 0)
            {
                ui->label_wlan0_MAC->setText(string);
            }
            else
            {
                ui->label_wlan0_MAC->setText(NONE);
            }
        }
        else
        {
            ui->pushButton_wlan0_enable->setIcon(QIcon(":/libicons/img/WifiOff.png"));
            if (ui->checkBox_wlan0_DHCP->isChecked())
            {
                ui->pushButton_wlan0_IP->setText(NONE);
            }
            ui->label_wlan0_MAC->setText(NONE);
        }
    }
    else
    {
        ui->tab_wlan0->setEnabled(false);
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

bool net_conf::checkNetAddr(char * ipaddr)
{
    QStringList ipaddrStr = QString(ipaddr).split(".");

    if (ipaddrStr.count() != 4)
    {
        return false;
    }

    bool ok;
    for (int i = 0; i < 4; i++)
    {
        int a = ipaddrStr.at(i).toInt(&ok);
        if (ok == false || a < 0 || a > 255)
        {
            return false;
        }
    }
    return true;
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

void net_conf::on_pushButton_eth1_IP_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth1_IP->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_IP->setText(value);
    }
}

void net_conf::on_pushButton_eth1_NM_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth1_NM->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_NM->setText(value);
    }
}

void net_conf::on_pushButton_eth1_GW_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth1_GW->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_GW->setText(value);
    }
}

void net_conf::on_pushButton_eth1_DNS1_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth1_DNS1->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_eth1_DNS2_clicked()
{
    char value [32];
    numpad tastiera_alfanum(value, IPADDR, ui->pushButton_eth1_DNS2->text().toAscii().data());
    tastiera_alfanum.showFullScreen();
    if(tastiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_DNS2->setText(value);
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
    char line[256];

    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NONE);

    FILE * fp = popen("/usr/sbin/wifi.sh scan", "r");
    if (fp == NULL)
    {
        QMessageBox::critical(0,QApplication::trUtf8("Network configuration"), QApplication::trUtf8("Problem during wifi network scanning"));
        return;
    }

    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        char * p = strchr(line, '\t');
        if (p)
        {
            *p = '\0';
            ui->comboBox_wlan0_essid->addItem(line);
        }
    }

    int index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
    if (index < 0)
    {
        ui->comboBox_wlan0_essid->addItem(wlan0_essid);
        index = ui->comboBox_wlan0_essid->findText(wlan0_essid);
    }
    if (index != ui->comboBox_wlan0_essid->currentIndex())
    {
        ui->comboBox_wlan0_essid->setCurrentIndex(index);
    }

    pclose(fp);
}

bool net_conf::isWlanOn(void)
{
    return system("iwconfig wlan0 2> /dev/null | grep -q 'Access Point: Not-Associate'");
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

void net_conf::on_checkBox_eth1_DHCP_clicked(bool checked)
{
    ui->frame_eth1->setEnabled(!checked);
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

bool net_conf::isWanOn(void)
{
    return system("test -e /var/pppd/up.stat && source /var/pppd/up.stat && test -e /proc/$PPPD_PID") == 0;
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

bool net_conf::checkUSBwanKey()
{
    return system("lsmod | grep -q ^usb_wwan && test -e /dev/ttyUSB0") == 0;
}

bool net_conf::checkUSBwlanKey()
{
    return system("ifconfig wlan0 >/dev/null 2>&1") == 0;
}

