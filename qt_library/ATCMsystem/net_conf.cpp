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
        return false;
    }

    /* DHCP */
    if (ui->checkBox_eth0_DHCP->isChecked())
    {
        if (app_netconf_item_set("[dhcp]", "BOOTPROTO0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
    }
    else
    {
        if (app_netconf_item_set("[none]", "BOOTPROTO0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* IP */
        if (ui->pushButton_eth0_IP->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_IP->text().toAscii().data(), "IPADDR0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_eth0_GW->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_GW->text().toAscii().data(), "GATEWAY0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* NETMASK */
        if (ui->pushButton_eth0_NM->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_NM->text().toAscii().data(), "NETMASK0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_eth0_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_DNS1->text().toAscii().data(), "NAMESERVER01"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_eth0_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth0_DNS2->text().toAscii().data(), "NAMESERVER02"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
    }
    return true;
}

/* ETH1 */
bool net_conf::saveETH1cfg()
{
    if (!is_eth1_enabled)
    {
        return false;
    }
    /* DHCP */
    if (ui->checkBox_eth1_DHCP->isChecked())
    {
        if (app_netconf_item_set("[dhcp]", "BOOTPROTO1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
    }
    else
    {
        if (app_netconf_item_set("[none]", "BOOTPROTO1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* IP */
        if (ui->pushButton_eth1_IP->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_IP->text().toAscii().data(), "IPADDR1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_eth1_GW->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_GW->text().toAscii().data(), "GATEWAY1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* NETMASK */
        if (ui->pushButton_eth1_NM->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_NM->text().toAscii().data(), "NETMASK1"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_eth1_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_DNS1->text().toAscii().data(), "NAMESERVER11"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_eth1_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_eth1_DNS2->text().toAscii().data(), "NAMESERVER12"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
    }
    return true;
}

/* WLAN0 */
bool net_conf::saveWLAN0cfg()
{
    /* ONBOOT */
    char onboot[2];
    strcpy(onboot, (ui->checkBox_wlan0_BOOT->isChecked() == true) ? "1" : "0");
    if (app_netconf_item_set(onboot, "ONBOOTW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* ESSID */
    if (ui->comboBox_wlan0_essid->currentIndex() > 0 && app_netconf_item_set(QString(QString("\"") + ui->comboBox_wlan0_essid->currentText() + QString("\"")).toAscii().data(), "ESSIDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* PASSWORD */
    if (wlan0_pwd.compare(NONE) != 0 && app_netconf_item_set(QString(QString("\"") + wlan0_pwd + QString("\"")).toAscii().data(), "PASSWORDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DHCP */
    if (ui->checkBox_wlan0_DHCP->isChecked())
    {
        if (app_netconf_item_set("[dhcp]", "BOOTPROTOW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
    }
    else
    {
        if (app_netconf_item_set("[none]", "BOOTPROTOW0") == 0)
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* IP */
        if (ui->pushButton_wlan0_IP->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_IP->text().toAscii().data(), "IPADDRW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* GATEWAY */
        if (ui->pushButton_wlan0_GW->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_GW->text().toAscii().data(), "GATEWAYW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* NETMASK */
        if (ui->pushButton_wlan0_NM->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_NM->text().toAscii().data(), "NETMASKW0"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* DNS1 */
        if (ui->pushButton_wlan0_DNS1->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_DNS1->text().toAscii().data(), "NAMESERVERW01"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
        /* DNS2 */
        if (ui->pushButton_wlan0_DNS2->text().compare(NONE) != 0 && app_netconf_item_set(ui->pushButton_wlan0_DNS2->text().toAscii().data(), "NAMESERVERW02"))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
            return false;
        }
    }
    char command[256];
    sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\"",
            ui->comboBox_wlan0_essid->currentText().toAscii().data(),
            wlan0_pwd.toAscii().data()
            );
    if (system(command))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot setup the wifi network configuration for '%1'").arg(ui->comboBox_wlan0_essid->currentText()));
        return false;
    }
    return true;
}

void net_conf::on_pushButtonSaveAll_clicked()
{
    /* save all pages */
    saveETH0cfg();
    saveETH1cfg();
    saveWLAN0cfg();
    if (QMessageBox::question(this, tr("Conferma Scrittura"), tr("Configurazione salvata\nPer rendere effettivi i cambiamenti, è necessario riavviare il sistema.\nRiavviare ora?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        system("reboot");
    }
}


/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void net_conf::reload()
{
    char *tmp = NULL;

    char essid[256];
    setup = true;
    is_wlan_active = isWlanOn(essid);
    if (is_wlan_active)
    {
        ui->comboBox_wlan0_essid->clear();
        ui->comboBox_wlan0_essid->addItem(NONE);
        ui->comboBox_wlan0_essid->addItem(essid);
        ui->pushButton_wlan0_enable->setIcon(QIcon(":/systemicons/img/WifiOn.png"));
    }
    else
    {
        ui->pushButton_wlan0_enable->setIcon(QIcon(":/systemicons/img/WifiOff.png"));
    }

    setup = false;

    //ui->tab_wlan0->setEnabled(system("/usr/sbin/wifi.sh scan >/dev/null 2>&1") == 0);

    ui->tabWidget->setCurrentIndex(0);

    if (is_eth0_enabled)
    {
        /* ETH0 */
        /* DHCP */
        if (app_netconf_item_get(&tmp, "BOOTPROTO0") != NULL && strcmp(tmp, "[dhcp]") != 0)
        {
            on_checkBox_eth0_DHCP_clicked(false);
        }
        else
        {
            on_checkBox_eth0_DHCP_clicked(true);
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
#endif
    }

    if (is_eth1_enabled)
    {
        /* ETH1 */
        /* DHCP */
        if (app_netconf_item_get(&tmp, "BOOTPROTO1") != NULL && strcmp(tmp, "[dhcp]") != 0)
        {
            on_checkBox_eth1_DHCP_clicked(false);
        }
        else
        {
            on_checkBox_eth1_DHCP_clicked(true);
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
#endif
    }

    /* WLAN0 */
    /* ESSID */
    if (app_netconf_item_get(&tmp, "ESSIDW0") != NULL && tmp[0] != '\0')
    {
        QString essid = QString(tmp).mid(1, strlen(tmp)-2);
        int index = ui->comboBox_wlan0_essid->findText(essid);
        if (index < 0)
        {
            ui->comboBox_wlan0_essid->addItem(essid);
            index = ui->comboBox_wlan0_essid->findText(essid);
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
        wlan0_pwd = tmp;
        ui->pushButton_wlan0_pwd->setText(QString("*").repeated(wlan0_pwd.length()));
    }
    else
    {
        wlan0_pwd = tmp;
        ui->pushButton_wlan0_pwd->setText(NONE);
    }

    if (ui->comboBox_wlan0_essid->currentText().compare(NONE) != 0 && wlan0_pwd.length() > 0)
    {
        char command[256];
        sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\"",
                ui->comboBox_wlan0_essid->currentText().toAscii().data(),
                wlan0_pwd.toAscii().data()
                );
        if (system(command))
        {
            /* error */
            QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot setup the wifi network configuration for '%1'").arg(ui->comboBox_wlan0_essid->currentText()));
            return;
        }
    }

    /* DHCP */
    if (app_netconf_item_get(&tmp, "BOOTPROTOW0") != NULL && strcmp(tmp, "[dhcp]") != 0)
    {
        on_checkBox_wlan0_DHCP_clicked(false);
    }
    else
    {
        on_checkBox_wlan0_DHCP_clicked(true);
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
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_IP->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_IP->setText(value);
    }
}

void net_conf::on_pushButton_eth0_NM_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_NM->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_NM->setText(value);
    }
}

void net_conf::on_pushButton_eth0_GW_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_GW->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_GW->setText(value);
    }
}

void net_conf::on_pushButton_eth0_DNS1_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_DNS1->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_eth0_DNS2_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_DNS2->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth0_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_eth1_IP_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_IP->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_IP->setText(value);
    }
}

void net_conf::on_pushButton_eth1_NM_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_NM->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_NM->setText(value);
    }
}

void net_conf::on_pushButton_eth1_GW_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_GW->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_GW->setText(value);
    }
}

void net_conf::on_pushButton_eth1_DNS1_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_DNS1->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_eth1_DNS2_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_DNS2->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_eth1_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_IP_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_IP->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_IP->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_NM_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_NM->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_NM->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_GW_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_GW->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_GW->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_DNS1_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_DNS1->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_DNS2_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_DNS2->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr(value))
    {
        ui->pushButton_wlan0_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_scan_clicked()
{
    char line[256];
    QString actual_essid = ui->comboBox_wlan0_essid->currentText();

    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem(NONE);

    FILE * fp = popen("/usr/sbin/wifi.sh scan", "r");
    if (fp == NULL)
    {
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Problem during wifi network scanning"));
        LOG_PRINT(error_e, "Failed to run command '%s'\n", "/usr/sbin/wifi.sh scan" );
        return;
    }

    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        char * p = strrchr(line, '\t');
        if (p)
        {
            *p = '\0';
        }
        p = strrchr(line, '\t');
        if (p)
        {
            *p = '\0';
        }
        sscanf(line, "%s %*s %*s", line);
        LOG_PRINT(info_e, "essid: '%s'\n", line);
        ui->comboBox_wlan0_essid->addItem(line);
    }

    int index = ui->comboBox_wlan0_essid->findText(actual_essid);
    if (index < 0)
    {
        ui->comboBox_wlan0_essid->addItem(actual_essid);
        index = ui->comboBox_wlan0_essid->findText(actual_essid);
    }
    if (index != ui->comboBox_wlan0_essid->currentIndex())
    {
        ui->comboBox_wlan0_essid->setCurrentIndex(index);
    }

    pclose(fp);
}

bool net_conf::isWlanOn(char * essid)
{
    FILE * fp = popen("/usr/sbin/wifi.sh scan", "r");
    int quality = 0;

    if (fp == NULL)
    {
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Problem during wifi network scanning"));
        LOG_PRINT(error_e, "Failed to run command '%s'\n", "/usr/sbin/wifi.sh scan" );
        return false;
    }

    bool ison = false;
    char line[256];
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        char * p = strrchr(line, '\t');
        if (p)
        {
            *p = '\0';
        }
        p = strrchr(line, '\t');
        quality = atoi(p + 1);
        if (essid)
        {
            p = strrchr(line, '\t');
            if (p)
            {
                *p = '\0';
            }
            strcpy(essid, line);
        }
        LOG_PRINT(none_e, "quality: '%d'\n", quality);
        if (quality < 0)
        {
            ison = true;
            break;
        }
    }

    pclose(fp);
    return ison;
}

void net_conf::on_pushButton_wlan0_enable_clicked()
{
    if (!is_wlan_active)
    {
        system("/usr/sbin/wifi.sh start");
    }
    else
    {
        system("/usr/sbin/wifi.sh stop");
    }

    is_wlan_active = isWlanOn(NULL);
    if (is_wlan_active)
    {
        ui->pushButton_wlan0_enable->setIcon(QIcon(":/systemicons/img/WifiOn.png"));
    }
    else
    {
        ui->pushButton_wlan0_enable->setIcon(QIcon(":/systemicons/img/WifiOff.png"));
    }
}

void net_conf::on_pushButton_wlan0_pwd_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, wlan0_pwd.toAscii().data(), true);
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted)
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