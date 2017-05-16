#include "mectsettings.h"
#include "ui_mectsettings.h"
#include "utils.h"

#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QTextStream>
#include <QValidator>
#include <QIntValidator>


#define MAX_SPACE_AVAILABLE_MAX 128
#define LINE_SIZE 1024
#define MAXBLOCKSIZE 64


const QString szPAGE = QString::fromAscii("page");
const QString szDEFLANG  = QString::fromAscii("it");
const QString szLANGMAPFILE = QString::fromAscii(":/ctecsv/others/lang_table.csv");


MectSettings::MectSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MectSettings)
{
    int     i = 0;

    ui->setupUi(this);
    // Impostazione degli opportuni Validator per i campi integers
    ui->lineEdit_Retries->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_Blacklist->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_ReadPeriod1->setValidator(new QIntValidator(1, nMax_Int16, this));
    ui->lineEdit_ReadPeriod2->setValidator(new QIntValidator(1, nMax_Int16, this));
    ui->lineEdit_ReadPeriod3->setValidator(new QIntValidator(1, nMax_Int16, this));
    ui->lineEdit_PwdTimeout->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_ScreenSaver->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_SlowLogPeriod->setValidator(new QIntValidator(2, nMax_Int16, this));
    ui->lineEdit_FastLogPeriod->setValidator(new QIntValidator(1, nMax_Int16, this));
    ui->lineEdit_MaxLogSpace->setValidator(new QIntValidator(1, MAX_SPACE_AVAILABLE_MAX, this));
    ui->lineEdit_TraceWindow->setValidator(new QIntValidator(0, nMax_Int16, this));
    /* SERIAL 0 */
    ui->lineEdit_Silence_SERIAL_PORT_0->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_Timeout_SERIAL_PORT_0->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_0->setValidator(new QIntValidator(1, MAXBLOCKSIZE, this));
    /* SERIAL 1 */
    ui->lineEdit_Silence_SERIAL_PORT_1->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_Timeout_SERIAL_PORT_1->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_1->setValidator(new QIntValidator(1, MAXBLOCKSIZE, this));
    /* SERIAL 2 */
    ui->lineEdit_Silence_SERIAL_PORT_2->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_Timeout_SERIAL_PORT_2->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_2->setValidator(new QIntValidator(1, MAXBLOCKSIZE, this));
    /* TCP_IP */
    ui->lineEdit_Silence_TCP_IP_PORT->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_Timeout_TCP_IP_PORT->setValidator(new QIntValidator(0, nMax_Int16, this));
    ui->lineEdit_MaxBlockSize_TCP_IP_PORT->setValidator(new QIntValidator(1, MAXBLOCKSIZE, this));
    /* CAN 0*/
    ui->lineEdit_MaxBlockSize_CANOPEN_0->setValidator(new QIntValidator(1, MAXBLOCKSIZE, this));
    /* CAN 1*/
    ui->lineEdit_MaxBlockSize_CANOPEN_1->setValidator(new QIntValidator(1, MAXBLOCKSIZE, this));
    // Clear Model Info
    m_nModel = -1;
    m_szModel.clear();
    m_isIniModified = false;
    // Tab Enabler
    for (i = 0; i < tabTotals; i++)
        m_tabEnabled[i] = true;

}

MectSettings::~MectSettings()
{
    delete ui;
}

void    MectSettings::setModel(TP_Config &tpConfig)
{
    int i = 0;

    // Recupera Modello e descrizione
    m_nModel = tpConfig.nModel;
    m_szModel = tpConfig.modelName;
    TargetConfig = tpConfig;
    enablePortsFromModel(TargetConfig);
    // Refresh Port Tabs config
    // qDebug() << tr("Model: %1") .arg(nModel);
    for (i = 0; i < tabTotals; i++)  {
        // ui->tabWidget->setTabEnabled(i, m_tabEnabled[i]);
        ui->tabWidget->widget(i)->setEnabled(m_tabEnabled[i]);
        // qDebug() << tr("Tab %1: %2") .arg(i) .arg(m_tabEnabled[i]);
    }
    // Set Tab System as current Tab
    ui->tabWidget->setCurrentIndex(tabSystem);
}
bool    MectSettings::loadProjectFiles(const QString &szFileSettings, const QString szFilePro, const QString &szProjectPath, TP_Config &targetConfig)
{
    QVariant defBlockSize = QString::number(MAXBLOCKSIZE);

    // Copy of Setting File, Project File, Project Path
    m_szFileSettings = szFileSettings;
    m_szFilePro = szFilePro;
    m_szProjectPath = szProjectPath;
    // qDebug() << "Settings: " << m_szFileSettings;
    // qDebug() << "Project:  " << m_szFilePro;
    // qDebug() << "Path Prj: " << m_szProjectPath;

    // Enable All Tabs as default
    int nTab = 0;
    for (nTab = 0; nTab < tabTotals; nTab++)
        m_tabEnabled[nTab] = true;

    // Ricerca del file di Settings
    if (!QFile(szFileSettings).exists())
    {
        QMessageBox::critical(this,trUtf8("Error"),trUtf8("System.ini file don't exist, it's impossible tool open."));
        return false;
    }

    /*Lettura del file.pro per sapere e costruire la combobox @ le pagine attualmente in uso nel progetto*/
    /* open and load crosstable file */
    QFile file(szFilePro);
    if(!file.exists()) {
        QMessageBox::information(this, trUtf8("Error"),trUtf8("Project File %1 don't exist, it's impossible tool open.") .arg(szFilePro));
        return false;
    }
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    QStringList pagesList;

    // Cerca nel file .pro le righe relative agli oggetti ui
    while(!in.atEnd()) {
        QString line = in.readLine().trimmed();
        line.replace(szBACKSLASH, QString::fromAscii(""));
        line = line.trimmed();
        if (! line.isEmpty())  {
            QStringList lstItems = line.split(chSpace);
            if (lstItems.at(0).startsWith(szPAGE) && lstItems.at(0).endsWith(QString::fromAscii(".ui")))
            {
                // Verifica che il file esista nella directory corrente
                pagesList << QFileInfo(lstItems.at(0)).baseName();
            }
        }
    }
    file.close();
    // qDebug() << "Ui File list" << pagesList;
    /* Aggiungo in coda le pagine di Libreria */
    pagesList << QString::fromAscii("alarms")
              << QString::fromAscii("alarms_history")
              << QString::fromAscii("recipe")
              << QString::fromAscii("recipe_select")
              << QString::fromAscii("store")
              << QString::fromAscii("store_filter")
              << QString::fromAscii("data_manager")
              << QString::fromAscii("display_settings")
              << QString::fromAscii("info")
              << QString::fromAscii("menu")
              << QString::fromAscii("options")
              << QString::fromAscii("system_ini")
              << QString::fromAscii("time_set")
              << QString::fromAscii("trend")
              << QString::fromAscii("trend_option")
              << QString::fromAscii("trend_range");

    /*Inserimento della lista delle pagine all'interno delle combobox*/
    ui->comboBox_HomePage->clear();
    ui->comboBox_HomePage->addItems(pagesList);
    ui->comboBox_StartPage->clear();
    ui->comboBox_StartPage->addItems(pagesList);
    ui->comboBox_PwdLogoutPage->clear();
    ui->comboBox_PwdLogoutPage->addItem(szEMPTY);
    ui->comboBox_PwdLogoutPage->addItems(pagesList);

    QSettings settings(szFileSettings, QSettings::IniFormat);

    LanguageMap.clear();
    ui->comboBoxLanguage->clear();

    /* load Global MECT map of the language and his locale abbreviation */
    QFile langFile(szLANGMAPFILE);
    QString szLine;
    if (langFile.exists() && langFile.open(QFile::ReadOnly | QIODevice::Text)) {
        QTextStream langStream(&langFile);
        while (! langStream.atEnd())  {
            szLine = langStream.readLine(LINE_SIZE);
            QStringList strlist = szLine.split(szSEMICOL);
            if (strlist.count()==2)
            {
                LanguageMap.insert(strlist.at(1).trimmed(), strlist.at(0).trimmed());
            }
        }
        langFile.close();
    }
    // qDebug() << "LanguageMap Items: " << LanguageMap.count();
    /* load the translations file list */
    QDirIterator lang_it(szProjectPath, QDirIterator::Subdirectories);
    QString lang = settings.value(QString::fromAscii("SYSTEM/language"), szDEFLANG).toString();
    QString szLangTempl = QString::fromAscii("languages_");
    int indexlang = -1;
    while (lang_it.hasNext()) {
        QString item = lang_it.next();
        // Cerca tutti i files con estensione .qm
        if (item.endsWith (QString::fromAscii(".qm")) == true)
        {
            int nPos = item.indexOf(szLangTempl);
            QString tmplang;
            if (nPos >= 0)  {
                tmplang = item.mid(nPos + szLangTempl.length(), 2);
                if (tmplang.isEmpty())
                    continue;
            }
            else
                continue;
            if (LanguageMap.count() > 0 && LanguageMap.contains(tmplang))
            {
                // La lingua esiste come chiave, inserisce in combo il valore associato
                ui->comboBoxLanguage->addItem(LanguageMap.value(tmplang));
            }
            else
            {
                // La lingua Non esiste come chiave, inserisce in combo solo la chiave
                ui->comboBoxLanguage->addItem(tmplang);
            }
            if (lang == tmplang)
            {
                indexlang = ui->comboBoxLanguage->count() - 1;
            }

        }
    }
    QString szValue;
    ui->comboBoxLanguage->setCurrentIndex(indexlang);
    ui->lineEdit_Retries->setText(settings.value(QString::fromAscii("SYSTEM/retries")).toString());
    ui->lineEdit_Blacklist->setText(settings.value(QString::fromAscii("SYSTEM/blacklist")).toString());
    ui->lineEdit_ReadPeriod1->setText(settings.value(QString::fromAscii("SYSTEM/read_period_ms_1")).toString());
    ui->lineEdit_ReadPeriod2->setText(settings.value(QString::fromAscii("SYSTEM/read_period_ms_2")).toString());
    ui->lineEdit_ReadPeriod3->setText(settings.value(QString::fromAscii("SYSTEM/read_period_ms_3")).toString());
    szValue = settings.value(QString::fromAscii("SYSTEM/home_page")).toString();
    ui->comboBox_HomePage->setCurrentIndex(ui->comboBox_HomePage->findText(szValue));
    // qDebug() << "Home Page" << szValue;
    szValue = settings.value(QString::fromAscii("SYSTEM/start_page")).toString();
    ui->comboBox_StartPage->setCurrentIndex(ui->comboBox_StartPage->findText(szValue));
    // qDebug() << "Start Page" << szValue;
    ui->checkBox_BuzzerTouch->setChecked(settings.value(QString::fromAscii("SYSTEM/buzzer_touch")).toBool());
    ui->checkBox_BuzzerAlarm->setChecked(settings.value(QString::fromAscii("SYSTEM/buzzer_alarm")).toBool());
    ui->lineEdit_PwdTimeout->setText(settings.value(QString::fromAscii("SYSTEM/pwd_timeout_s")).toString());
    szValue = settings.value(QString::fromAscii("SYSTEM/pwd_logout_page")).toString();
    ui->comboBox_PwdLogoutPage->setCurrentIndex(ui->comboBox_PwdLogoutPage->findText(szValue));
    // qDebug() << "Pwd Logout Page" << szValue;
    ui->lineEdit_ScreenSaver->setText(settings.value(QString::fromAscii("SYSTEM/screen_saver_s")).toString());
    ui->lineEdit_SlowLogPeriod->setText(settings.value(QString::fromAscii("SYSTEM/slow_log_period_s")).toString());
    ui->lineEdit_FastLogPeriod->setText(settings.value(QString::fromAscii("SYSTEM/fast_log_period_s")).toString());
    ui->lineEdit_MaxLogSpace->setText(settings.value(QString::fromAscii("SYSTEM/max_log_space_MB")).toString());
    ui->lineEdit_TraceWindow->setText(settings.value(QString::fromAscii("SYSTEM/trace_window_s")).toString());

    int index;
    QString value;
    /* La presenza del Baud Rate abilita o meno il Tab per la porta in esame */
    value = settings.value(QString::fromAscii("SERIAL_PORT_0/baudrate")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_0->setCurrentIndex(index);
    }
    else  {
        // Disable all Tab
        m_tabEnabled[tabSerial0] = false;
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_0/databits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_0->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_0/parity")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_0->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_0/stopbits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_0->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_0->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_0->setText(settings.value(QString::fromAscii("SERIAL_PORT_0/silence_ms")).toString());
    ui->lineEdit_Timeout_SERIAL_PORT_0->setText(settings.value(QString::fromAscii("SERIAL_PORT_0/timeout_ms")).toString());
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_0->setText(settings.value(QString::fromAscii("SERIAL_PORT_0/max_block_size"), defBlockSize).toString());

    value = settings.value(QString::fromAscii("SERIAL_PORT_1/baudrate")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_1->setCurrentIndex(index);
    }
    else  {
        // Disable all Tab
        m_tabEnabled[tabSerial1] = false;
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_1/databits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_1->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_1/parity")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_1->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_1/stopbits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_1->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_1->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_1->setText(settings.value(QString::fromAscii("SERIAL_PORT_1/silence_ms")).toString());
    ui->lineEdit_Timeout_SERIAL_PORT_1->setText(settings.value(QString::fromAscii("SERIAL_PORT_1/timeout_ms")).toString());
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_1->setText(settings.value(QString::fromAscii("SERIAL_PORT_1/max_block_size"), defBlockSize).toString());

    value = settings.value(QString::fromAscii("SERIAL_PORT_2/baudrate")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_2->setCurrentIndex(index);
    }
    else  {
        // Disable all Tab
        m_tabEnabled[tabSerial2] = false;
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_2/databits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_2->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_2/parity")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_2->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_2/stopbits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_2->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_2 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_2->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_2->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_2->setText(settings.value(QString::fromAscii("SERIAL_PORT_2/silence_ms")).toString());
    ui->lineEdit_Timeout_SERIAL_PORT_2->setText(settings.value(QString::fromAscii("SERIAL_PORT_2/timeout_ms")).toString());
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_2->setText(settings.value(QString::fromAscii("SERIAL_PORT_2/max_block_size"), defBlockSize).toString());

    value = settings.value(QString::fromAscii("SERIAL_PORT_3/baudrate")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'baudrate' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Baudrate_SERIAL_PORT_3->setCurrentIndex(index);
    }
    else  {
        // Disable all Tab
        m_tabEnabled[tabSerial3] = false;
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_3/databits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Databits_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 3;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'databits' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Databits_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Databits_SERIAL_PORT_3->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_3/parity")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Parity_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'parity' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Parity_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Parity_SERIAL_PORT_3->setCurrentIndex(index);
    }

    value = settings.value(QString::fromAscii("SERIAL_PORT_3/stopbits")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Stopbits_SERIAL_PORT_3->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for SERIAL_PORT_3 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Stopbits_SERIAL_PORT_3->itemText(index)));
        }
        ui->comboBox_Stopbits_SERIAL_PORT_3->setCurrentIndex(index);
    }

    ui->lineEdit_Silence_SERIAL_PORT_3->setText(settings.value(QString::fromAscii("SERIAL_PORT_3/silence_ms")).toString());
    ui->lineEdit_Timeout_SERIAL_PORT_3->setText(settings.value(QString::fromAscii("SERIAL_PORT_3/timeout_ms")).toString());
    ui->lineEdit_MaxBlockSize_SERIAL_PORT_3->setText(settings.value(QString::fromAscii("SERIAL_PORT_3/max_block_size"), defBlockSize).toString());

    ui->lineEdit_Silence_TCP_IP_PORT->setText(settings.value(QString::fromAscii("TCP_IP_PORT/silence_ms")).toString());
    ui->lineEdit_Timeout_TCP_IP_PORT->setText(settings.value(QString::fromAscii("TCP_IP_PORT/timeout_ms")).toString());
    ui->lineEdit_MaxBlockSize_TCP_IP_PORT->setText(settings.value(QString::fromAscii("TCP_IP_PORT/max_block_size"), defBlockSize).toString());

    value = settings.value(QString::fromAscii("CANOPEN_0/baudrate")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_CANOPEN_0->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for CANOPEN_0 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_CANOPEN_0->itemText(index)));
        }
        ui->comboBox_Baudrate_CANOPEN_0->setCurrentIndex(index);
        ui->lineEdit_MaxBlockSize_CANOPEN_0->setText(settings.value(QString::fromAscii("CANOPEN_0/max_block_size"), defBlockSize).toString());
    }
    else  {
        // Disable all Tab
        m_tabEnabled[tabCan0] = false;
    }

    value = settings.value(QString::fromAscii("CANOPEN_1/baudrate")).toString();
    if (value.length() > 0)
    {
        index = ui->comboBox_Baudrate_CANOPEN_1->findText(value);
        if ( index < 0)
        {
            index = 0;
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("'stopbits' parameter for CANOPEN_1 have an invalid value: %1. it will be set it as %2.").arg(value).arg(ui->comboBox_Baudrate_CANOPEN_1->itemText(index)));
        }
        ui->comboBox_Baudrate_CANOPEN_1->setCurrentIndex(index);
        ui->lineEdit_MaxBlockSize_CANOPEN_1->setText(settings.value(QString::fromAscii("CANOPEN_1/max_block_size"), defBlockSize).toString());
    }
    else  {
        // Disable all Tab
        m_tabEnabled[tabCan1] = false;
    }
    // Set Model Info for current Model
    setModel(targetConfig);
    m_isIniModified = false;
    ui->cmdSave->setStyleSheet(QString::fromAscii("border: 2px solid green;"));
    // All Ok, return true
    return true;
}

void MectSettings::on_cmdSave_clicked()
{

    // Controllo del contenuto dei campi
    if (! checkFields())
        return;
    if (fileBackUp(m_szFileSettings))
        saveOrCheckAll(m_szFileSettings, false);
    else
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("Error creating back up copy of file: %1") .arg(m_szFileSettings));

}
bool MectSettings::checkOrSet(QSettings &settings, QString szKey, QString szValue, bool checkOnly, bool isChanged)
{
    QString szTemp;
    if (checkOnly)  {
        // Controllo di variazione tra Interfaccia e File di dati
        // recupero valore corrente e confronto
        szTemp = settings.value(szKey).toString();
        if (QString::compare(szTemp, szValue, Qt::CaseSensitive) != 0)  {
            isChanged = true;
        }
    }
    else  {
        // Scrittura del valore su file Settings
        settings.setValue(szKey, szValue);
    }
    return isChanged;
}

bool MectSettings::saveOrCheckAll(QString szFileName, bool checkOnly)
{
    bool        changed = false;
    QSettings   settings(szFileName, QSettings::IniFormat, this);
    qDebug() << tr("File Settings: %1") .arg(szFileName);

    /* SYSTEM */
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/language"), LanguageMap.key(ui->comboBoxLanguage->currentText(),szDEFLANG), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/retries"), ui->lineEdit_Retries->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/blacklist"), ui->lineEdit_Blacklist->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/read_period_ms_1"), ui->lineEdit_ReadPeriod1->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/read_period_ms_2"), ui->lineEdit_ReadPeriod2->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/read_period_ms_3"), ui->lineEdit_ReadPeriod3->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/home_page"), ui->comboBox_HomePage->currentText(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/start_page"), ui->comboBox_StartPage->currentText(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/buzzer_touch"), (ui->checkBox_BuzzerTouch->isChecked() == true) ? QString::fromAscii("1") : QString::fromAscii("0"), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/buzzer_alarm"), (ui->checkBox_BuzzerAlarm->isChecked() == true) ? QString::fromAscii("1") : QString::fromAscii("0"), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/pwd_timeout_s"), ui->lineEdit_PwdTimeout->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/pwd_logout_page"), ui->comboBox_PwdLogoutPage->currentText(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/screen_saver_s"), ui->lineEdit_ScreenSaver->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/slow_log_period_s"), ui->lineEdit_SlowLogPeriod->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/fast_log_period_s"), ui->lineEdit_FastLogPeriod->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/max_log_space_MB"), ui->lineEdit_MaxLogSpace->text(), checkOnly, changed);
    changed = checkOrSet(settings, QString::fromAscii("SYSTEM/trace_window_s"), ui->lineEdit_TraceWindow->text(), checkOnly, changed);

    /* SERIAL 0 */
    if (m_tabEnabled[tabSerial0])  {
        if (ui->comboBox_Baudrate_SERIAL_PORT_0->currentIndex() > 0)
        {
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/baudrate"), ui->comboBox_Baudrate_SERIAL_PORT_0->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/databits"), ui->comboBox_Databits_SERIAL_PORT_0->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/parity"), ui->comboBox_Parity_SERIAL_PORT_0->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/stopbits"), ui->comboBox_Stopbits_SERIAL_PORT_0->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/silence_ms"), ui->lineEdit_Silence_SERIAL_PORT_0->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/timeout_ms"), ui->lineEdit_Timeout_SERIAL_PORT_0->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_0/max_block_size"), ui->lineEdit_MaxBlockSize_SERIAL_PORT_0->text(), checkOnly, changed);
        }
        else
        {
            if (!checkOnly)
                settings.remove(QString::fromAscii("SERIAL_PORT_0"));
        }
    }
    /* SERIAL 1 */
    if (m_tabEnabled[tabSerial1])  {
        if (ui->comboBox_Baudrate_SERIAL_PORT_1->currentIndex() > 0)
        {
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/baudrate"), ui->comboBox_Baudrate_SERIAL_PORT_1->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/databits"), ui->comboBox_Databits_SERIAL_PORT_1->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/parity"), ui->comboBox_Parity_SERIAL_PORT_1->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/stopbits"), ui->comboBox_Stopbits_SERIAL_PORT_1->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/silence_ms"), ui->lineEdit_Silence_SERIAL_PORT_1->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/timeout_ms"), ui->lineEdit_Timeout_SERIAL_PORT_1->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_1/max_block_size"), ui->lineEdit_MaxBlockSize_SERIAL_PORT_1->text(), checkOnly, changed);
        }
        else
        {
            if (!checkOnly)
                settings.remove(QString::fromAscii("SERIAL_PORT_1"));
        }
    }
    /* SERIAL 2 */
    if (m_tabEnabled[tabSerial2])  {
        if (ui->comboBox_Baudrate_SERIAL_PORT_2->currentIndex() > 0)
        {
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/baudrate"), ui->comboBox_Baudrate_SERIAL_PORT_2->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/databits"), ui->comboBox_Databits_SERIAL_PORT_2->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/parity"), ui->comboBox_Parity_SERIAL_PORT_2->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/stopbits"), ui->comboBox_Stopbits_SERIAL_PORT_2->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/silence_ms"), ui->lineEdit_Silence_SERIAL_PORT_2->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/timeout_ms"), ui->lineEdit_Timeout_SERIAL_PORT_2->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_2/max_block_size"), ui->lineEdit_MaxBlockSize_SERIAL_PORT_2->text(), checkOnly, changed);
        }
        else
        {
            if (!checkOnly)
                settings.remove(QString::fromAscii("SERIAL_PORT_2"));
        }
    }
    /* SERIAL 3 */
    if (m_tabEnabled[tabSerial3])  {
        if (ui->comboBox_Baudrate_SERIAL_PORT_3->currentIndex() > 0)
        {
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/baudrate"), ui->comboBox_Baudrate_SERIAL_PORT_3->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/databits"), ui->comboBox_Databits_SERIAL_PORT_3->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/parity"), ui->comboBox_Parity_SERIAL_PORT_3->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/stopbits"), ui->comboBox_Stopbits_SERIAL_PORT_3->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/silence_ms"), ui->lineEdit_Silence_SERIAL_PORT_3->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/timeout_ms"), ui->lineEdit_Timeout_SERIAL_PORT_3->text(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("SERIAL_PORT_3/max_block_size"), ui->lineEdit_MaxBlockSize_SERIAL_PORT_3->text(), checkOnly, changed);
        }
        else
        {
            if (!checkOnly)
                settings.remove(QString::fromAscii("SERIAL_PORT_3"));
        }
    }
    /* TCP_IP */
    if (m_tabEnabled[tabTCP])  {
        changed = checkOrSet(settings, QString::fromAscii("TCP_IP_PORT/silence_ms"), ui->lineEdit_Silence_TCP_IP_PORT->text(), checkOnly, changed);
        changed = checkOrSet(settings, QString::fromAscii("TCP_IP_PORT/timeout_ms"), ui->lineEdit_Timeout_TCP_IP_PORT->text(), checkOnly, changed);
        changed = checkOrSet(settings, QString::fromAscii("TCP_IP_PORT/max_block_size"), ui->lineEdit_MaxBlockSize_TCP_IP_PORT->text(), checkOnly, changed);
    }
    /* CANOPEN 0 */
    if (m_tabEnabled[tabCan0])  {
        if (ui->comboBox_Baudrate_CANOPEN_0->currentIndex() > 0)
        {
            changed = checkOrSet(settings, QString::fromAscii("CANOPEN_0/baudrate"), ui->comboBox_Baudrate_CANOPEN_0->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("CANOPEN_0/max_block_size"), ui->lineEdit_MaxBlockSize_CANOPEN_0->text(), checkOnly, changed);
        }
        else
        {
            if (!checkOnly)
                settings.remove(QString::fromAscii("CANOPEN_0"));
        }
    }
    /* CANOPEN 1 */
    if (m_tabEnabled[tabCan1])  {
        if (ui->comboBox_Baudrate_CANOPEN_1->currentIndex() > 0)
        {
            changed = checkOrSet(settings, QString::fromAscii("CANOPEN_1/baudrate"), ui->comboBox_Baudrate_CANOPEN_1->currentText(), checkOnly, changed);
            changed = checkOrSet(settings, QString::fromAscii("CANOPEN_1/max_block_size"), ui->lineEdit_MaxBlockSize_CANOPEN_1->text(), checkOnly, changed);
        }
        else
        {
            if (!checkOnly)
                settings.remove(QString::fromAscii("CANOPEN_1"));
        }
    }
    // Return value
    if (checkOnly)
        return changed;
    // Global Sync
    settings.sync();
    QMessageBox::information(0,trUtf8("Information"),trUtf8("Configuration has been successfully saved."));
    return true;
}
bool MectSettings::checkFields()
// Controllo del contenuto dei campi
{
    bool    fRes = false;
    bool    OK = false;
    int     nVal = 0;


    //------------------------
    // Tab System
    //------------------------
    if (ui->lineEdit_Retries->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Retries' parameter must be greater than or egual 0."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Retries' parameter must be a number."));
        goto exitCheck;
    }
    if (ui->lineEdit_Blacklist->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Blacklist' parameter must be greater than or egual 0."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Blacklist' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_ReadPeriod1->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 1' parameter must be greater than 0."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 1' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_ReadPeriod2->text().toInt(&OK) < 2 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be greater than 1."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_ReadPeriod3->text().toInt(&OK) < 3 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be greater than 2."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_ReadPeriod3->text().toInt() < ui->lineEdit_ReadPeriod2->text().toInt())
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 3' parameter must be greater than 'Read Period 2' parameter."));
        goto exitCheck;
    }

    if (ui->lineEdit_ReadPeriod2->text().toInt() < ui->lineEdit_ReadPeriod1->text().toInt())
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Read Period 2' parameter must be greater than 'Read Period 1' parameter."));
        goto exitCheck;
    }
    if (ui->lineEdit_PwdTimeout->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Pwd Timeout' parameter must be greater than or egual 0."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Pwd Timeout' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_ScreenSaver->text().toInt(&OK) < 0 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Screen Saver' parameter must be greater than or egual 0."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Screen Saver' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_SlowLogPeriod->text().toInt(&OK) < 2 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be greater than 1."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_FastLogPeriod->text().toInt(&OK) < 1 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Fast Log Period' parameter must be greater than 0."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Fast Log Period' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_SlowLogPeriod->text().toInt() < ui->lineEdit_FastLogPeriod->text().toInt())
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Slow Log Period' parameter must be greater than 'Fast Log Period' parameter."));
        goto exitCheck;
    }
    if (ui->lineEdit_MaxLogSpace->text().toInt(&OK) <= 0 && ui->lineEdit_MaxLogSpace->text().toInt(&OK) > MAX_SPACE_AVAILABLE_MAX && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Max Log Space' parameter must be greater than 0 and less than %1.").arg(MAX_SPACE_AVAILABLE_MAX));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Max Log Space' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_TraceWindow->text().toInt(&OK) < 3 && OK == true)
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be greater than 2."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be a number."));
        goto exitCheck;
    }

    if (ui->lineEdit_TraceWindow->text().toInt() < (ui->lineEdit_FastLogPeriod->text().toInt()* 3))
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("'Trace Window' parameter must be at least three times 'Fast Log Period' parameter."));
        goto exitCheck;
    }

    if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
    {
        QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Silence' parameter must be a number."));
        goto exitCheck;
    }
    //------------------------
    /* SERIAL 0 */
    //------------------------
    if (m_tabEnabled[tabSerial0] && ui->comboBox_Baudrate_SERIAL_PORT_0->currentIndex() > 0)  {
        if (ui->lineEdit_Silence_SERIAL_PORT_0->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Silence' parameter must be greater than or egual 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Silence' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_0->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_0->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_0->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            goto exitCheck;
        }
        // Max Block Size
        nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_0->text().toInt(&OK);
        if (OK == true && nVal < 1)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Max Block Size' parameter must be greater than or egual to 1."));
            goto exitCheck;
        }
        if (OK == true && nVal > MAXBLOCKSIZE)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 0' tab, the 'Max Block Size' parameter must be smaller than or egual to %1.") .arg(MAXBLOCKSIZE));
            goto exitCheck;
        }
    }
    //------------------------
    /* SERIAL 1 */
    //------------------------
    if (m_tabEnabled[tabSerial1] && ui->comboBox_Baudrate_SERIAL_PORT_1->currentIndex() > 0)  {
        if (ui->lineEdit_Silence_SERIAL_PORT_1->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be greater than or egual 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Silence' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_1->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_1->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_1->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            goto exitCheck;
        }
        // Max Block Size
        nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_1->text().toInt(&OK);
        if (OK == true && nVal < 1)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Max Block Size' parameter must be greater than or egual to 1."));
            goto exitCheck;
        }
        if (OK == true && nVal > MAXBLOCKSIZE)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 1' tab, the 'Max Block Size' parameter must be smaller than or egual to %1.") .arg(MAXBLOCKSIZE));
            goto exitCheck;
        }
    }
    //------------------------
    /* SERIAL 2 */
    //------------------------
    if (m_tabEnabled[tabSerial2] && ui->comboBox_Baudrate_SERIAL_PORT_2->currentIndex() > 0)  {
        if (ui->lineEdit_Silence_SERIAL_PORT_2->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be greater than or egual 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Silence' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_2->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_SERIAL_PORT_2->text().toInt() <= ui->lineEdit_Silence_SERIAL_PORT_2->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            goto exitCheck;
        }
        // Max Block Size
        nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_2->text().toInt(&OK);
        if (OK == true && nVal < 1)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Max Block Size' parameter must be greater than or egual to 1."));
            goto exitCheck;
        }
        if (OK == true && nVal > MAXBLOCKSIZE)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'SERIAL PORT 2' tab, the 'Max Block Size' parameter must be smaller than or egual to %1.") .arg(MAXBLOCKSIZE));
            goto exitCheck;
        }
    }
    //------------------------
    /* TCP_IP */
    //------------------------
    if (m_tabEnabled[tabTCP])  {
        if (ui->lineEdit_Silence_TCP_IP_PORT->text().toInt(&OK) < 0 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be greater than or egual 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Silence' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_TCP_IP_PORT->text().toInt(&OK) < 1 && OK == true)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 0."));
            goto exitCheck;
        }

        if(OK == false)/*Controllo che il valore inserito non sia diverso da un numero*/
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be a number."));
            goto exitCheck;
        }

        if (ui->lineEdit_Timeout_TCP_IP_PORT->text().toInt() <= ui->lineEdit_Silence_TCP_IP_PORT->text().toInt())
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Timeout' parameter must be greater than 'Silence' parameter."));
            goto exitCheck;
        }
        // Max Block Size
        nVal = ui->lineEdit_MaxBlockSize_TCP_IP_PORT->text().toInt(&OK);
        if (OK == true && nVal < 1)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Max Block Size' parameter must be greater than or egual to 1."));
            goto exitCheck;
        }
        if (OK == true && nVal > MAXBLOCKSIZE)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'TCP_IP_PORT' tab, the 'Max Block Size' parameter must be smaller than or egual to %1.") .arg(MAXBLOCKSIZE));
            goto exitCheck;
        }
    }
    //------------------------
    /* CAN_0 */
    //------------------------
    if (m_tabEnabled[tabCan0])  {
        // Max Block Size
        nVal = ui->lineEdit_MaxBlockSize_CANOPEN_0->text().toInt(&OK);
        if (OK == true && nVal < 1)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'CAN_0' tab, the 'Max Block Size' parameter must be greater than or egual to 1."));
            goto exitCheck;
        }
        if (OK == true && nVal > MAXBLOCKSIZE)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'CAN_0' tab, the 'Max Block Size' parameter must be smaller than or egual to %1.") .arg(MAXBLOCKSIZE));
            goto exitCheck;
        }
    }
    //------------------------
    /* CAN_1 */
    //------------------------
    if (m_tabEnabled[tabCan1])  {
        // Max Block Size
        nVal = ui->lineEdit_MaxBlockSize_CANOPEN_1->text().toInt(&OK);
        if (OK == true && nVal < 1)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'CAN_0' tab, the 'Max Block Size' parameter must be greater than or egual to 1."));
            goto exitCheck;
        }
        if (OK == true && nVal > MAXBLOCKSIZE)
        {
            QMessageBox::critical(0,trUtf8("Error"),trUtf8("In the 'CAN_0' tab, the 'Max Block Size' parameter must be smaller than or egual to %1.") .arg(MAXBLOCKSIZE));
            goto exitCheck;
        }
    }
    //------------------------
    // All check are Ok, return true
    //------------------------
    fRes = true;

exitCheck:
    return fRes;
}
void MectSettings::enablePortsFromModel(TP_Config &targetConfig)
{
    // tab System and TCP are always ON
    m_tabEnabled[tabSystem] = true;
    m_tabEnabled[tabTCP] = true;
    // Check if model is in range
    if (targetConfig.nModel == AnyTPAC || targetConfig.nModel >= MODEL_TOTALS)  {
        return;
    }
    // Seriale 0
    m_tabEnabled[tabSerial0] = targetConfig.ser0_Enabled;
    // Seriale 1
    m_tabEnabled[tabSerial1] = targetConfig.ser1_Enabled;
    // Seriale 2
    m_tabEnabled[tabSerial2] = targetConfig.ser2_Enabled;
    // Seriale 3
    m_tabEnabled[tabSerial3] = targetConfig.ser3_Enabled;
    // Can 0
    m_tabEnabled[tabCan0] = targetConfig.can0_Enabled;
    // Can1
    m_tabEnabled[tabCan1] = targetConfig.can1_Enabled;;
}
bool MectSettings::getTargetConfig(TP_Config &targetConfig)
// Retrieves current target configuration
{
    bool fRes = true;
    bool fOk = false;
    int  nVal = 0;

    // Update Local && Global TPAC config
    // Retries
    nVal = ui->lineEdit_Retries->text().toInt(&fOk);
    TargetConfig.retries = fOk ? nVal : 0;
    // Black List
    nVal = ui->lineEdit_Blacklist->text().toInt(&fOk);
    TargetConfig.blacklist = fOk ? nVal : 0;
    // ReadPeriod1
    nVal = ui->lineEdit_ReadPeriod1->text().toInt(&fOk);
    TargetConfig.readPeriod1 = fOk ? nVal : 0;
    // ReadPeriod2
    nVal = ui->lineEdit_ReadPeriod2->text().toInt(&fOk);
    TargetConfig.readPeriod2 = fOk ? nVal : 0;
    // ReadPeriod3
    nVal = ui->lineEdit_ReadPeriod3->text().toInt(&fOk);
    TargetConfig.readPeriod3 = fOk ? nVal : 0;
    // fastLogPeriod
    nVal = ui->lineEdit_FastLogPeriod->text().toInt(&fOk);
    TargetConfig.fastLogPeriod = fOk ? nVal : 0;
    // slowLogPeriod
    nVal = ui->lineEdit_SlowLogPeriod->text().toInt(&fOk);
    TargetConfig.slowLogPeriod = fOk ? nVal : 0;
    //  Serial 0
    TargetConfig.ser0_Enabled = (m_tabEnabled[tabSerial0] && ui->comboBox_Baudrate_SERIAL_PORT_0->currentIndex()) > 0;
    nVal = ui->comboBox_Baudrate_SERIAL_PORT_0->currentText().toInt(&fOk);
    TargetConfig.ser0_BaudRate = fOk ? nVal : 0;
    nVal = ui->lineEdit_Timeout_SERIAL_PORT_0->text().toInt(&fOk);
    TargetConfig.ser0_TimeOut = fOk ? nVal : 0;
    nVal = ui->lineEdit_Silence_SERIAL_PORT_0->text().toInt(&fOk);
    TargetConfig.ser0_Silence = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_0->text().toInt(&fOk);
    TargetConfig.ser0_BlockSize = fOk ? nVal : 0;
    //  Serial 1
    TargetConfig.ser1_Enabled = (m_tabEnabled[tabSerial1] && ui->comboBox_Baudrate_SERIAL_PORT_1->currentIndex()) > 0;
    nVal = ui->comboBox_Baudrate_SERIAL_PORT_1->currentText().toInt(&fOk);
    TargetConfig.ser1_BaudRate = fOk ? nVal : 0;
    nVal = ui->lineEdit_Timeout_SERIAL_PORT_1->text().toInt(&fOk);
    TargetConfig.ser1_TimeOut = fOk ? nVal : 0;
    nVal = ui->lineEdit_Silence_SERIAL_PORT_1->text().toInt(&fOk);
    TargetConfig.ser1_Silence = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_1->text().toInt(&fOk);
    TargetConfig.ser1_BlockSize = fOk ? nVal : 0;
    //  Serial 2
    TargetConfig.ser2_Enabled = (m_tabEnabled[tabSerial2] && ui->comboBox_Baudrate_SERIAL_PORT_2->currentIndex()) > 0;
    nVal = ui->comboBox_Baudrate_SERIAL_PORT_2->currentText().toInt(&fOk);
    TargetConfig.ser2_BaudRate = fOk ? nVal : 0;
    nVal = ui->lineEdit_Timeout_SERIAL_PORT_2->text().toInt(&fOk);
    TargetConfig.ser2_TimeOut = fOk ? nVal : 0;
    nVal = ui->lineEdit_Silence_SERIAL_PORT_2->text().toInt(&fOk);
    TargetConfig.ser2_Silence = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_2->text().toInt(&fOk);
    TargetConfig.ser2_BlockSize = fOk ? nVal : 0;
    //  Serial 3
    TargetConfig.ser3_Enabled = (m_tabEnabled[tabSerial3] && ui->comboBox_Baudrate_SERIAL_PORT_3->currentIndex()) > 0;
    nVal = ui->comboBox_Baudrate_SERIAL_PORT_3->currentText().toInt(&fOk);
    TargetConfig.ser3_BaudRate = fOk ? nVal : 0;
    nVal = ui->lineEdit_Timeout_SERIAL_PORT_3->text().toInt(&fOk);
    TargetConfig.ser3_TimeOut = fOk ? nVal : 0;
    nVal = ui->lineEdit_Silence_SERIAL_PORT_3->text().toInt(&fOk);
    TargetConfig.ser3_Silence = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_SERIAL_PORT_3->text().toInt(&fOk);
    TargetConfig.ser3_BlockSize = fOk ? nVal : 0;
    // TCP
    nVal = ui->lineEdit_Timeout_TCP_IP_PORT->text().toInt(&fOk);
    TargetConfig.tcp_TimeOut = fOk ? nVal : 0;
    nVal = ui->lineEdit_Silence_TCP_IP_PORT->text().toInt(&fOk);
    TargetConfig.tcp_Silence = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_TCP_IP_PORT->text().toInt(&fOk);
    TargetConfig.tcp_BlockSize = fOk ? nVal : 0;
    // CAN_0
    TargetConfig.can0_Enabled = (m_tabEnabled[tabCan0] && ui->comboBox_Baudrate_CANOPEN_0->currentIndex()) > 0;
    nVal = ui->comboBox_Baudrate_CANOPEN_0->currentText().toInt(&fOk);
    TargetConfig.can0_BaudRate = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_CANOPEN_0->text().toInt(&fOk);
    TargetConfig.can0_BlockSize = fOk ? nVal : 0;
    // CAN_1
    TargetConfig.can1_Enabled = (m_tabEnabled[tabCan1] && ui->comboBox_Baudrate_CANOPEN_1->currentIndex()) > 0;
    nVal = ui->comboBox_Baudrate_CANOPEN_1->currentText().toInt(&fOk);
    TargetConfig.can1_BaudRate = fOk ? nVal : 0;
    nVal = ui->lineEdit_MaxBlockSize_CANOPEN_1->text().toInt(&fOk);
    TargetConfig.can1_BlockSize = fOk ? nVal : 0;
    qDebug() << tr("Updated Configuration");
    // Global settings
    targetConfig = TargetConfig;
    // Return value
    return fRes;
}
bool MectSettings::isModified()
{
    bool fChanged = false;

    // Check if a Setting file is opened
    m_isIniModified = false;
    if ( ! m_szFileSettings.isEmpty())  {
        fChanged = saveOrCheckAll(m_szFileSettings, true);
        m_isIniModified = fChanged;
    }
    return (fChanged);
}
void MectSettings::saveMectSettings()
{

    if (! m_szFileSettings.isEmpty())  {
        saveOrCheckAll(m_szFileSettings, false);
        m_isIniModified = false;
    }
}
