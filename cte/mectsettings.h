#ifndef MECTSETTINGS_H
#define MECTSETTINGS_H

#include "parser.h"
#include "ctecommon.h"

#include <QWidget>
#include <QHash>
#include <QString>
#include <QEvent>

namespace Ui {
class MectSettings;
}

class MectSettings : public QWidget
{
    Q_OBJECT

public:
    explicit MectSettings(QWidget *parent = 0);
    ~MectSettings();
    bool loadProjectFiles(const QString &szFileSettings, const QString szFilePro, const QString &szProjectPath, TP_Config &targetConfig);
    bool getTargetConfig(TP_Config &targetConfig);              // Retrieves current target configuration
    bool isModified();

signals:

public slots:
    void    setModel(TP_Config &tpConfig);
    void    saveMectSettings();

private slots:

    void on_cmdSave_clicked();

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    void save_all(QString szFileName, bool notifyUser = false);
    bool checkFields();
    void enablePortsFromModel(TP_Config &targetConfig);
    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------
    QHash<QString, QString> LanguageMap;
    Ui::MectSettings *ui;
    QString     m_szFileSettings;
    QString     m_szProjectPath;
    QString     m_szFilePro;
    QString     m_szModel;
    TP_Config   TargetConfig;
    int         m_nModel;
    bool        m_tabEnabled[tabTotals];
    bool        m_isIniModified;
};

#endif // MECTSETTINGS_H
