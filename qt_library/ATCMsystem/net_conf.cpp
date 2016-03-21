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
    ui->comboBox_wlan0_essid->addItem("-");
}

void net_conf::on_pushButtonHome_clicked()
{
    go_home();
}

void net_conf::on_pushButtonBack_clicked()
{
    go_back();
}

void net_conf::on_pushButtonSave_clicked()
{
    /* save the current page */

    /* ETH0 */
    if (ui->tabWidget->currentIndex() == 0)
    {
        saveETH0cfg();
    }
    else if (ui->tabWidget->currentIndex() == 1)
    {
        saveETH1cfg();
    }
    else if (ui->tabWidget->currentIndex() == 2)
    {
        saveWLAN0cfg();
    }
}

/* ETH0 */
bool net_conf::saveETH0cfg()
{
    /* IP */
    if (ui->pushButton_eth0_IP->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth0_IP->text().toAscii().data(), "IPADDR0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* GATEWAY */
    if (ui->pushButton_eth0_GW->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth0_GW->text().toAscii().data(), "GATEWAY0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* NETMASK */
    if (ui->pushButton_eth0_NM->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth0_NM->text().toAscii().data(), "NETMASK0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DNS1 */
    if (ui->pushButton_eth0_DNS1->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth0_DNS1->text().toAscii().data(), "NAMESERVER01"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DNS2 */
    if (ui->pushButton_eth0_DNS2->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth0_DNS2->text().toAscii().data(), "NAMESERVER02"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    return true;
}

/* ETH1 */
bool net_conf::saveETH1cfg()
{
    /* IP */
    if (ui->pushButton_eth1_IP->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth1_IP->text().toAscii().data(), "IPADDR1"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* GATEWAY */
    if (ui->pushButton_eth1_GW->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth1_GW->text().toAscii().data(), "GATEWAY1"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* NETMASK */
    if (ui->pushButton_eth1_NM->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth1_NM->text().toAscii().data(), "NETMASK1"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DNS1 */
    if (ui->pushButton_eth1_DNS1->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth1_DNS1->text().toAscii().data(), "NAMESERVER11"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DNS2 */
    if (ui->pushButton_eth1_DNS2->text().length() > 1 && app_netconf_item_set(ui->pushButton_eth1_DNS2->text().toAscii().data(), "NAMESERVER12"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    return true;
}

/* WLAN0 */
bool net_conf::saveWLAN0cfg()
{
    /* ESSID */
    if (ui->comboBox_wlan0_essid->currentIndex() > 0 && app_netconf_item_set(ui->comboBox_wlan0_essid->currentText().toAscii().data(), "ESSIDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* PASSWORD */
    if (ui->pushButton_wlan0_pwd->text().length() > 1 && app_netconf_item_set(ui->pushButton_wlan0_pwd->text().toAscii().data(), "PASSWORDW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* IP */
    if (ui->pushButton_wlan0_IP->text().length() > 1 && app_netconf_item_set(ui->pushButton_wlan0_IP->text().toAscii().data(), "IPADDRW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* GATEWAY */
    if (ui->pushButton_wlan0_GW->text().length() > 1 && app_netconf_item_set(ui->pushButton_wlan0_GW->text().toAscii().data(), "GATEWAYW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* NETMASK */
    if (ui->pushButton_wlan0_NM->text().length() > 1 && app_netconf_item_set(ui->pushButton_wlan0_NM->text().toAscii().data(), "NETMASKW0"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DNS1 */
    if (ui->pushButton_wlan0_DNS1->text().length() > 1 && app_netconf_item_set(ui->pushButton_wlan0_DNS1->text().toAscii().data(), "NAMESERVERW01"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    /* DNS2 */
    if (ui->pushButton_wlan0_DNS2->text().length() > 1 && app_netconf_item_set(ui->pushButton_wlan0_DNS2->text().toAscii().data(), "NAMESERVERW02"))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot update the network configuration"));
        return false;
    }
    char command[256];
    sprintf(command, "/usr/sbin/wifi.sh setup \"%s\" \"%s\"",
            ui->comboBox_wlan0_essid->currentText().toAscii().data(),
            ui->pushButton_wlan0_pwd->text().toAscii().data()
            );
    if (system(command))
    {
        /* error */
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Cannot setup the wifi network configuration"));
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
}


/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void net_conf::reload()
{
    char *tmp = NULL;

    ui->tabWidget->setCurrentIndex(0);

    /* ETH0 */
    /* IP */
    if (app_netconf_item_get(&tmp, "IPADDR0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth0_IP->setText(tmp);
    }
    else
    {
        ui->pushButton_eth0_IP->setText("-");
    }
    /* GATEWAY */
    if (app_netconf_item_get(&tmp, "GATEWAY0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth0_GW->setText(tmp);
    }
    else
    {
        ui->pushButton_eth0_GW->setText("-");
    }
    /* NETMASK */
    if (app_netconf_item_get(&tmp, "NETMASK0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth0_NM->setText(tmp);
    }
    else
    {
        ui->pushButton_eth0_NM->setText("-");
    }
    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVER01") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth0_DNS1->setText(tmp);
    }
    else
    {
        ui->pushButton_eth0_DNS1->setText("-");
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVER02") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth0_DNS2->setText(tmp);
    }
    else
    {
        ui->pushButton_eth0_DNS2->setText("-");
    }
    /* MAC */
    if (app_macconf_item_get(&tmp, "MAC0") != NULL && tmp[0] != '\0')
    {
        ui->label_eth0_MAC->setText(tmp);
    }
    else
    {
        ui->label_eth0_MAC->setText("-");
    }


    /* ETH1 */
    /* IP */
    if (app_netconf_item_get(&tmp, "IPADDR1") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth1_IP->setText(tmp);
    }
    else
    {
        ui->pushButton_eth1_IP->setText("-");
    }
    /* GATEWAY */
    if (app_netconf_item_get(&tmp, "GATEWAY1") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth1_GW->setText(tmp);
    }
    else
    {
        ui->pushButton_eth1_GW->setText("-");
    }
    /* NETMASK */
    if (app_netconf_item_get(&tmp, "NETMASK1") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth1_NM->setText(tmp);
    }
    else
    {
        ui->pushButton_eth1_NM->setText("-");
    }
    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVER11") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth1_DNS1->setText(tmp);
    }
    else
    {
        ui->pushButton_eth1_DNS1->setText("-");
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVER12") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_eth1_DNS2->setText(tmp);
    }
    else
    {
        ui->pushButton_eth1_DNS2->setText("-");
    }
    /* MAC */
    if (app_macconf_item_get(&tmp, "MAC1") != NULL && tmp[0] != '\0')
    {
        ui->label_eth1_MAC->setText(tmp);
    }
    else
    {
        ui->label_eth1_MAC->setText("-");
    }

    /* WLAN0 */

    /* ESSID */
    if (app_netconf_item_get(&tmp, "ESSIDW0") != NULL && tmp[0] != '\0')
    {
        int index = ui->comboBox_wlan0_essid->findText(tmp);
        if (index < 0)
        {
            ui->comboBox_wlan0_essid->addItem(tmp);
            index = ui->comboBox_wlan0_essid->findText(tmp);
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
        ui->pushButton_wlan0_pwd->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_pwd->setText("-");
    }

    /* IP */
    if (app_netconf_item_get(&tmp, "IPADDRW0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_IP->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_IP->setText("-");
    }
    /* GATEWAY */
    if (app_netconf_item_get(&tmp, "GATEWAYW0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_GW->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_GW->setText("-");
    }
    /* NETMASK */
    if (app_netconf_item_get(&tmp, "NETMASKW0") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_NM->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_NM->setText("-");
    }
    /* DNS1 */
    if (app_netconf_item_get(&tmp, "NAMESERVERW01") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_DNS1->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_DNS1->setText("-");
    }
    /* DNS2 */
    if (app_netconf_item_get(&tmp, "NAMESERVERW02") != NULL && tmp[0] != '\0')
    {
        ui->pushButton_wlan0_DNS2->setText(tmp);
    }
    else
    {
        ui->pushButton_wlan0_DNS2->setText("-");
    }
    /* MAC */
    if (app_macconf_item_get(&tmp, "MACW0") != NULL && tmp[0] != '\0')
    {
        ui->label_wlan0_MAC->setText(tmp);
    }
    else
    {
        ui->label_wlan0_MAC->setText("-");
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

bool net_conf::checkNetAddr()
{
    return true;
}

void net_conf::on_pushButton_eth0_IP_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_IP->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth0_IP->setText(value);
    }
}

void net_conf::on_pushButton_eth0_NM_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_NM->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth0_NM->setText(value);
    }
}

void net_conf::on_pushButton_eth0_GW_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_GW->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth0_GW->setText(value);
    }
}

void net_conf::on_pushButton_eth0_DNS1_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_DNS1->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_eth0_DNS2_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth0_DNS2->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth0_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_eth1_IP_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_IP->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth1_IP->setText(value);
    }
}

void net_conf::on_pushButton_eth1_NM_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_NM->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth1_NM->setText(value);
    }
}

void net_conf::on_pushButton_eth1_GW_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_GW->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth1_GW->setText(value);
    }
}

void net_conf::on_pushButton_eth1_DNS1_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_DNS1->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth1_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_eth1_DNS2_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_eth1_DNS2->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_eth1_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_IP_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_IP->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_wlan0_IP->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_NM_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_NM->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_wlan0_NM->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_GW_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_GW->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_wlan0_GW->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_DNS1_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_DNS1->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_wlan0_DNS1->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_DNS2_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_DNS2->text().toAscii().data());
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted && checkNetAddr())
    {
        ui->pushButton_wlan0_DNS2->setText(value);
    }
}

void net_conf::on_pushButton_wlan0_scan_clicked()
{
    char line[256];
    char essid[256];

    ui->comboBox_wlan0_essid->clear();
    ui->comboBox_wlan0_essid->addItem("-");

    FILE * fp = popen("/usr/sbin/wifi.sh scan", "r");
    if (fp == NULL)
    {
        QMessageBox::critical(0,QApplication::tr("Network configuration"), QApplication::tr("Problem during wifi network scanning"));
        LOG_PRINT(error_e, "Failed to run command '%s'\n", "/usr/sbin/wifi.sh scan" );
        return;
    }

    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        sscanf(line, "%s %*s %*s", essid);
        LOG_PRINT(info_e, "essid: '%s'\n", essid);
        ui->comboBox_wlan0_essid->addItem(essid);
    }

    pclose(fp);
}

void net_conf::on_pushButton_wlan0_enable_toggled(bool checked)
{
    if (checked)
    {
        if (system("/usr/sbin/wifi.sh start"))
        {
            on_pushButton_wlan0_enable_toggled(false);
        }
    }
    else
    {
        system("/usr/sbin/wifi.sh stop");
    }
}

void net_conf::on_pushButton_wlan0_pwd_clicked()
{
    char value [32];
    alphanumpad tatiera_alfanum(value, ui->pushButton_wlan0_pwd->text().toAscii().data(), true);
    tatiera_alfanum.showFullScreen();
    if(tatiera_alfanum.exec()==QDialog::Accepted)
    {
        ui->pushButton_wlan0_pwd->setText(value);
    }
}
