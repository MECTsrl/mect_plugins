#ifndef TRENDEDITOR_H
#define TRENDEDITOR_H

#include "parser.h"
#include "ctecommon.h"

#include <QWidget>
#include <QLabel>
#include <QString>
#include <QHash>
#include <QColor>
#include <QStringList>

namespace Ui {
class TrendEditor;
}

class TrendEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TrendEditor(QWidget *parent = 0);
    ~TrendEditor();
    void    setTrendsParameters(const QString szModel, const QString &szTrendsPath, const QString szNewFile, const QString szTemplateFile); // Force Model to new received model and enable Save - Save As
    void    fillTrendsCombo(const QString szTrendsPath);                        // Refresh combo of trend files
    bool    isModified();
    QString currentTrendFile();
    void    saveTrend(bool notifUser = false);

signals:

public slots:
    void updateVarLists(const QStringList &lstTrendVars);

private slots:

    void on_cmdColor_1_clicked();
    void on_cmdColor_2_clicked();
    void on_cmdColor_3_clicked();
    void on_cmdColor_4_clicked();
    void on_cmdLoad_clicked();
    void on_cmdSave_clicked();
    void on_cmdSaveAs_clicked();

    void on_cboTrendName_currentIndexChanged(int index);

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    bool    getNewColor(int nTrack);                        // Show Color dialog to select new Track color
    bool    trendFile2Iface(const QString &szFileTrend);    // Load a trends file in Interface Fields
    bool    tokens2Iface(const QStringList &lstTokens, int nRow);   // Da CSV Tokens ad interfaccia
    bool    iface2Tokens(QStringList &lstTokens, int nRow);         // Da Interfaccia a List per scrittura CSV
    bool    iface2TrendFile(const QString &szFileTrend);    // Write Interface Fields to a trends file
    void    iface2MemList(QStringList &lstMemVars);         // Copy Interface Objects to Memory Var List
    void    setObjectColor(QLabel *destObject, QColor newColor);
    bool    checkFields();                                  // Controllo del contenuto dei campi
    bool    updateTemplateFile(const QString &szTrendFile); // Aggiorna la sezione Trend del file template.pri
    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------
    Ui::TrendEditor *ui;
    // Liste varie di supporto all'interfaccia
    QHash<QString, QString> mapOrientation;
    QString         m_szCurrentModel;
    QString         m_szTrendFile;
    QString         m_szTrendPath;
    QString         m_szTemplateFile;
    QString         m_szMsg;                            // Variabile di servizio per Messaggi
    QStringList     lstSourceRows;

};

#endif // TRENDEDITOR_H
