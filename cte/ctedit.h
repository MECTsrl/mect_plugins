#ifndef CTEDIT_H
#define CTEDIT_H


#include "parser.h"
#include "cteerrdef.h"
#include "mectsettings.h"
#include "trendeditor.h"

#include <QObject>
#include <QTableView>
#include <QModelIndex>
#include <QModelIndexList>
#include <QItemSelection>
#include <QDialog>
#include <QList>
#include <QComboBox>
#include <QPoint>
#include <QColor>
#include <QTimer>

namespace Ui {
class ctedit;
}

class ctedit : public QDialog
{
    Q_OBJECT

public:
    explicit ctedit(QWidget *parent = 0);
    ~ctedit();
    bool    selectCTFile(QString szFileCT);
    void    saveCTFile();
    void    displayStatusMessage(QString szMessage, int nSeconds = 0);// Show message in ui->lblMessage
    bool    needSave();
    bool    querySave();

protected:
     bool   eventFilter(QObject *obj, QEvent *event);        // Gestore Event Handler

signals:

public slots:
    void    setProjectPath(QString szProjectPath);

private slots:
    void insertRows();                              // Aggiunta righe in posizione cursore
    void emptySelected();                           // Cancellazione delle righe correntemente selezionate
    void removeSelected();                          // Rimozione delle righe correntemente selezionate
    int  copySelected(bool fClearSelection);        // Copia delle righe selezionate in Buffer di Copiatura
    void pasteSelected();                           // Incolla righe da Buffer di copiatura a Riga corrente
    void cutSelected();                             // Taglia righe in Buffer di copiatura
    void displayUserMenu(const QPoint &pos);        // Menu contestuale Grid
    void tableItemChanged(const QItemSelection & selected, const QItemSelection & deselected);
    void clearEntryForm();                          // Svutamento elementi Form Data Entry
    void clearStatusMessage();                      // Clear message in ui->lblMessage
    void tabSelected(int nTab);                     // Change current Tab
    void gotoRow();                                 // Show Dialog Goto Row n
    void on_cmdBlocchi_clicked();                   // Riordino Blocchi
    void on_cmdSave_clicked();                      // Salvataggio file
    void on_cmdImport_clicked();                    // Import Rows from Another CT File
    void on_cmdSearch_clicked();                    // Search Variable by Name
    void on_cmdCompile_clicked();                   // Generate Compiled Files
    void on_cmdUndo_clicked();                      // Retrieve a CT Block from Undo List
    void on_cmdHideShow_toggled(bool checked);
    void on_cmdPLC_clicked();
    void on_cboPriority_currentIndexChanged(int index);
    void on_cboUpdate_currentIndexChanged(int index);
    void on_cboType_currentIndexChanged(int index);
    void on_cboProtocol_currentIndexChanged(int index);
    void on_cboBehavior_currentIndexChanged(int index);
    void on_cboVariable1_currentIndexChanged(int index);
    void on_cboCondition_currentIndexChanged(int index);
    void on_optFixedVal_toggled(bool checked);
    void on_optVariableVal_toggled(bool checked);
    void on_cboSections_currentIndexChanged(int index);
    void on_deviceTree_doubleClicked(const QModelIndex &index);


private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    // Lettura e scrittura dati da e per strutture di appoggio
    bool    ctable2Grid();                          // Lettura di tutta la CT in Grid
    bool    grid2CTable();                          // Dump di tutto il Grid in lista di CT Records
    bool    list2GridRow(QStringList &lstRecValues, int nRow);  // Inserimento o modifica elemento in Grid (valori -> GRID)
    bool    list2CTrec(QStringList &lstRecValues, int nRow);// Conversione da Lista Stringhe a CT Record (Grid -> REC SINGOLO)
    bool    recCT2List(QStringList &lstRecValues, int nRow);// Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
    void    listClear(QStringList &lstRecValues);   // Svuotamento e pulizia Lista Stringhe per passaggio dati Interfaccia <---> Record CT
    bool    values2Iface(QStringList &lstRecValues);// Copia Lista Stringhe convertite da CT Record a Zona di Editing
    bool    iface2values(QStringList &lstRecValues);// Copia da Zona Editing a Lista Stringhe per Grid e Record CT
    void    freeCTrec(int nRow);                    // Marca il Record della CT come inutilizzato
    bool    loadCTFile(QString szFileCT, QList<CrossTableRecord> &lstCtRecs, bool fLoadGrid);
    void    initTargetList();                       // Init della lista dei Target definiti
    bool    updateRow(int nRow);                    // Gestisce l'aggiornamento del grid con i valori letti da interfaccia di editing
    int     addRowsToCT(int nRow, QList<QStringList > &lstRecords2Add, QList<int> &lstDestRows);
    // Gestione interfaccia
    void    enableFields();                         // Abilitazione dei campi form in funzione di Protocollo
    bool    isLineModified(int nRow);               // Check se linea corrente Grid è diversa da Form in Editing
    bool    riassegnaBlocchi();                     // Riassegnazione blocchi variabili
    void    showAllRows(bool fShowAll);             // Visualizza o nascondi tutte le righe
    void    setRowColor(int nRow, int nAlternate);  // Imposta il colore di sfondo di una riga
    void    jumpToGridRow(int nRow, bool fCenter = false);                // Salto alla riga nRow del Grid
    void    enableInterface();                      // Abilita l'interfaccia in funzione dello stato del sistema
    void    setSectionArea(int nRow);               // Set Current item in combo cboSection from current Row
    int     fillVarList(QStringList &lstVars, QList<int> &lstTypes, QList<int> &lstUpdates, bool fSkipVarDecimal = false); // Fill sorted List of Variables Names for Types in lstTypes and Update Type in lstUpdates
    int     fillComboVarNames(QComboBox *comboBox, QList<int> &lstTypes, QList<int> &lstUpdates, bool fSkipVarDecimal = false);   // Caricamento ComboBox con Nomi Variabili filtrate in funzione del Tipo and Update Type in lstUpdates
    int     fillCompatibleTypesList(varTypes nTypeVar, QList<int> &lstTypes);           // Riempie la lista dei tipi compatibili tra loro
    int     varName2Row(QString &szVarName, QList<CrossTableRecord> &lstCTRecs);        // Search in Cross Table Record List the index of szVarName
    int     findNextVisibleRow(int nRow);           // Cerca la prossima riga visibile cui saltare (Per Enter on Grid)
    void    fillDeviceTree(int nCurRow = 0);           // Riempimento Albero dei device collegati al TP
    // Gestione Controlli
    bool    checkCTFile(QString szSourceFile);      // Controllo validità file CT per Import
    int     checkFormFields(int nRow, QStringList &lstValues, bool fSingleLine = true);     // Controlli formali sulla riga a termine editing
    bool    checkVarsCompatibility(varTypes nTypeV1, int nDecV1, varTypes nTypeV2, int nDecV2);      // Controllo tra i due operandi di un Allarme
    int     globalChecks();                         // Controlli complessivi su tutta la CT
    bool    isFormEmpty();                          // Controllo Form Editing vuoto
    bool    isValidVarName(QString szName);         // Controllo del Nome Variabile
    bool    isValidPort(int nPort, int nProtocol);  // Controllo di validità di un numero porta in funzione di Modello (da TargetConfig) e del Protocollo
    void    getFirstPortFromProtocol(int nProtocol, int &nPort, int &nTotal);// Cerca la prima porta disponibile in funzione del protocollo e della configurazione corrente
    void    fillErrorMessage(int nRow, int nCol, int nErrCode, QString szVarName, QString szValue, QChar severity, Err_CT *errCt);
    // Gestione Configurazione Progetto
    QString getModelName();                         // Lettura del file template.pri per determinare il modello di TPAC
    int     searchModelInList(QString szModel);     // Ricerca il modello corrente nella Lista modelli attuale
    QString getModelInfo(int nModel);               // Prepara il ToolTipText con le info di modello
    // Calcolo valori in funzione del Modello e del Protocollo
    void    enableProtocolsFromModel();             // Abilita i Protocolli in funzione del Modello corrente (da TargetConfig)
    int     varSizeInBlock(int nVarType);
    int     maxBlockSize(enum FieldbusType nProtocol, int nPort);    // max block size from Protocol && Port
    bool    isModbus(enum FieldbusType nProtocol);
    bool    isSameBitField(int nRow);
    bool    isBitField(enum varTypes nVarType);
    bool    isTooBigForBlock(int nRow, int nItemsInBlock, int nCurBlockSize);
    // Import dati in XML
    bool    getRowsFromXMLBuffer(QString &szBuffer, QList<QStringList > &lstPastedRecords, QList<int> &lstSourceRows, QList<int> &lstDestRows);
    bool    addModelVars(const QString szModelName, int nRow);
    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------
    // Gestione Interfaccia
    Ui::ctedit *ui;
    MectSettings    *mectSet;
    TrendEditor     *trendEdit;

    int         m_nGridRow;                         // Riga corrente sul Grid
    QString     m_szCurrentCTFile;                  // File Cross Table corrente (completo di Path)
    QString     m_szCurrentCTPath;                  // Path del file Cross Table corrente
    QString     m_szCurrentPLCPath;                 // Path della parte PLC del Progetto corrente
    QString     m_szCurrentCTName;                  // Nome del file Cross Table (senza Path)
    QString     m_szCurrentProjectPath;             // Project Path corrente
    QString     m_szCurrentProjectName;             // Project Name senza Path
    QString     m_szCurrentModel;                   // Modello TPAC del progetto
    // Liste varie per prompt colonne e valori Combo Box (per traduzioni)
    QStringList lstHeadCols;
    QStringList lstHeadNames;
    QList<int>  lstHeadSizes;
    QStringList lstPriority;
    QStringList lstUpdateNames;
    QStringList lstTipi;
    QStringList lstProtocol;
    QList<bool> lstBusEnabler;
    QStringList lstBehavior;
    QStringList lstCondition;
    QStringList lstProductNames;
    QStringList lstRegions;

    // Variabili di servizio
    QString     m_szFormatDate;                     // Format Masks per Date e tempo
    QString     m_szFormatTime;
    QString     m_szMsg;                            // Variabile di servizio per Messaggi
    QTimer      *tmrMessage;                        // Timer per la gestione Messaggi
    // Colori per sfondi grid
    QColor      colorRetentive[2];
    QColor      colorNonRetentive[2];
    QColor      colorSystem[2];
    QString     szColorRet[2];
    QString     szColorNonRet[2];
    QString     szColorSystem[2];

    // Record CrossTable
    QList<CrossTableRecord> lstCopiedRecords;       // Lista di Record per copia/incolla
    QList<CrossTableRecord> lstCTRecords;           // Lista completa di record per tabella
    QList<QList<CrossTableRecord> > lstUndo;        // Lista degli Undo di elementi di Cross Table Editor
    CrossTableRecord        CrossTable[1 + DimCrossTable];	 // campi sono riempiti a partire dall'indice 1
    // System Configuration
    TP_Config               TargetConfig;           // Configurazione corrente del Target letta da Form mectSettings
    QList<TP_Config>        lstTargets;             // Lista di tutti i Target definiti
    // Controllo e Gestione  Errori
    QList<int>              lstAllUpdates;          // Lista contenente tutti i tipi di Update (per filtro su Nomi variabili)
    QList<int>              lstNoHUpdates;          // Lista contenente tutti i tipi di Update tranne H (per Allarmi)
    QList<int>              lstLogUpdates;          // Lista contenente tutti i tipi di Update
    QList<int>              lstAllVarTypes;         // Lista contenente tutti i tipi di Variabili definiti
    QStringList             lstUsedVarNames;        // Lista contenente i nomi delle variabili (per Search)
    QStringList             lstLoggedVars;          // Lista contenente i nomi delle variabili Loggate (per Trends)
    QList<Err_CT>           lstCTErrors;
    QStringList             lstUniqueVarNames;      // Lista per controllo univocità identificatori
    QStringList             lstErrorMessages;       // Lista dei messaggi di errore
    // Gestione Server ModBus
    MODBUS_Srv              serverModBus[srvTotals];// Elenco dei Server ModBus definiti nel Sistema

    // Variabili di stato ad uso globale
    int                     m_nCurTab;              // Tab corrente in interfaccia
    bool                    m_isCtModified;         // Flag dati modificato
    bool                    m_isConfModified;       // Flag Configurazione modificata
    bool                    m_isTrendModified;      // Flag Trend Modificato
    bool                    m_fShowAllRows;         // Vero se sono visualizzate tutte le righe
    bool                    m_fCutOrPaste;          // Vero se è in corso un Cut or Paste
    bool                    m_fEmptyForm;           // Vero se il Form di Data Entry risulta vuoto
    varTypes                m_vtAlarmVarType;       // Tipo della variabile SX in un espressione Allarme/Evento
    int                     m_nAlarmDecimals;       // Numero di Decimali della Variabile SX di un Allarme
    bool                    m_fSkipLine;            // Se vero non devono essere fatti controlli sulla riga in uscita
};

#endif // CTEDIT_H
