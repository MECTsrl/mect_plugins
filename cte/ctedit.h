#ifndef CTEDIT_H
#define CTEDIT_H


#include "parser.h"
#include "cteerrdef.h"
#include "mectsettings.h"
#include "trendeditor.h"
#include "ctecommon.h"
#include "config_mpnc.h"
#include "config_mpne.h"
#include "searchvariable.h"

#include <QObject>
#include <QTableWidget>
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
#include <QTreeWidgetItem>

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
    void tableCTItemChanged(const QItemSelection & selected, const QItemSelection & deselected);
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
    void on_cmdApply_clicked();
    void on_cmdMultiEdit_clicked(bool checked);
    void on_chkInputRegister_stateChanged(int state);   // Cambio di Stato della Checkbox Input Register
    void on_cboPriority_currentIndexChanged(int index);
    void on_cboUpdate_currentIndexChanged(int index);
    void on_cboType_currentIndexChanged(int index);
    void on_cboProtocol_currentIndexChanged(int index);
    void on_cboBehavior_currentIndexChanged(int index);
    void on_cboVariable1_currentIndexChanged(int index);
    void on_cboCondition_currentIndexChanged(int index);
    void on_cboSections_currentIndexChanged(int index);
    void on_lstEditableFields_itemClicked(QListWidgetItem *itemClicked);
    void on_optFixedVal_toggled(bool checked);
    void on_optVariableVal_toggled(bool checked);
    void on_tblCT_doubleClicked(const QModelIndex &index);
    void on_txtName_editingFinished();              // Modificato nome della variabile
    void treeItemDoubleClicked(const QModelIndex &index);
    void return2GridRow(int nRow);                  // Ritorno da Tab MPNC - MPNE

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    // Lettura e scrittura dati da e per strutture di appoggio
    bool    ctable2Grid();                          // Lettura di tutta la CT in Grid
    bool    grid2CTable();                          // Dump di tutto il Grid in lista di CT Records
    bool    values2Iface(QStringList &lstRecValues, int nRow);// Copia Lista Stringhe convertite da CT Record a Zona di Editing
    bool    iface2values(QStringList &lstRecValues, bool fMultiEdit, int nRow);// Copia da Zona Editing a Lista Stringhe per Grid e Record CT
    bool    loadCTFile(QString szFileCT, QList<CrossTableRecord> &lstCtRecs, bool fLoadGrid);
    void    initTargetList();                       // Init della lista dei Target definiti
    void    exportTargetList(const QString &szFileName);    // Estrazione della lista dei Target definiti
    bool    updateRow(int nRow, bool fMultiEdit);   // Gestisce l'aggiornamento del grid con i valori letti da interfaccia di editing
    int     addRowsToCT(int nRow, QList<QStringList > &lstRecords2Add, QList<int> &lstDestRows, bool checkRTU = true);
    // Gestione interfaccia
    void    enableFields();                         // Abilitazione dei campi form in funzione di Protocollo
    bool    isLineModified(int nRow);               // Check se linea corrente Grid è diversa da Form in Editing
    bool    riassegnaBlocchi();                     // Riassegnazione blocchi variabili
    void    showAllRows(bool fShowAll);             // Visualizza o nascondi tutte le righe
    void    setRowsColor();                         // Imposta il colore di sfondo di tutte le righe senza cambiare riga corrente
    void    jumpToGridRow(int nRow, bool fCenter = false);                // Salto alla riga nRow del Grid
    void    enableInterface();                      // Abilita l'interfaccia in funzione dello stato del sistema

    void    setSectionArea(int nRow);               // Set Current item in combo cboSection from current Row
    int     fillVarList(QStringList &lstVars, QList<int> &lstTypes, QList<int> &lstUpdates, bool fSkipVarDecimal = false); // Fill sorted List of Variables Names for Types in lstTypes and Update Type in lstUpdates
    int     fillComboVarNames(QComboBox *comboBox, QList<int> &lstTypes, QList<int> &lstUpdates, bool fSkipVarDecimal = false);   // Caricamento ComboBox con Nomi Variabili filtrate in funzione del Tipo and Update Type in lstUpdates
    int     fillCompatibleTypesList(varTypes nTypeVar, QList<int> &lstTypes);           // Riempie la lista dei tipi compatibili tra loro
    int     varName2Row(QString &szVarName, QList<CrossTableRecord> &lstCTRecs);        // Search in Crosstable Record List the index of szVarName
    int     findNextVisibleRow(int nRow);           // Cerca la prossima riga visibile cui saltare (Per Enter on Grid)
    void    fillDeviceTree(int nCurRow = 0);        // Riempimento Albero dei device collegati al TP
    void    fillTimingsTree(int nCurRow = 0);       // Riempimento Albero delle variabili raggruppate per Priorità (Timings)
    void    fillLogTree(int nCurRow = 0);           // Riempimento Albero delle variabili raggruppate per Log Period
    // Devices and Timing tree
    QTreeWidgetItem *addItem2Tree(QTreeWidgetItem *tParent, int nRole, const QString &szName, const QString &szInfo, const QString &szTimings, const QString &szToolTip);    // Aggiunta di un Item all'albero
    QTreeWidgetItem *addVariable2Tree(QTreeWidgetItem *tParent, int nRow, int nLevel = treeVariable);      // Aggiunge la variabile della riga nRow agganciandola al nodo tParent. Ritorna oggetto
    QTreeWidgetItem *addDevice2Tree(QTreeWidgetItem *tParent, int nDevice);     // Aggiunge il Device nDevice agganciandolo al nodo tParent. Ritorna oggetto
    QTreeWidgetItem *addNode2Tree(QTreeWidgetItem *tParent, int nNode);         // Aggiunge il Nodo nNode agganciandolo al nodo tParent. Ritorna oggetto
    QTreeWidgetItem *addPriority2Tree(QTreeWidgetItem *tParent, int nPriority); // Aggiunge la Priority nPriority agganciandolo al nodo tParent. Ritorna oggetto
    QTreeWidgetItem *addBlock2Tree(QTreeWidgetItem *tParent, int nBlock, int nBlockSize);   // Aggiunge il blocco nBlock agganciandolo al nodo tParent. Ritorna oggetto
    // Gestione Controlli
    bool    checkCTFile(QString szSourceFile);      // Controllo validità file CT per Import
    int     checkFormFields(int nRow, QStringList &lstValues, bool fSingleLine = true);                     // Controlli formali sulla riga a termine editing
    bool    checkVarsCompatibility(varTypes nTypeV1, int nDecV1, varTypes nTypeV2, int nDecV2);             // Controllo tra i due operandi di un Allarme
    int     checkRegister(int nCurRow, QList<int> &lstUsingRegister);  // Controllo delle variabili che utilizzano un Registro
    int     globalChecks();                         // Controlli complessivi su tutta la CT
    bool    isFormEmpty();                          // Controllo Form Editing vuoto
    bool    isValidPort(int nPort, int nProtocol);  // Controllo di validità di un numero porta in funzione di Modello (da TargetConfig) e del Protocollo
    void    fillErrorMessage(int nRow, int nCol, int nErrCode, QString szVarName, QString szValue, QChar severity, Err_CT *errCt);
    // Gestione Configurazione Progetto
    QString getModelName();                         // Lettura del file template.pri per determinare il modello di TPAC
    int     searchModelInList(QString szModel);     // Ricerca il modello corrente nella Lista modelli attuale
    QString getModelInfo(int nModel);               // Prepara il ToolTipText con le info di modello
    // Calcolo valori in funzione del Modello e del Protocollo
    void    enableProtocolsFromModel();             // Abilita i Protocolli in funzione del Modello corrente (da TargetConfig)
    int     varSizeInBlock(int nVarType);
    int     maxBlockSize(enum FieldbusType nProtocol, int nPort);    // max block size from Protocol && Port
    bool    isModbus(int nProtocol);
    bool    isSameBitField(int nRow);
    bool    isBitField(enum varTypes nVarType);
    bool    isTooBigForBlock(int nRow, int nItemsInBlock, int nCurBlockSize);
    bool    checkServersDevicesAndNodes();          // Caricamento della lista dei Nodi e dei Devices a partire dalle variabili di CT
    double  serialCharTime_ms(int nBaudRate, int nDataBits, int nStopBits);                         // Calcolo del tempo di trasmissione di 1 byte su canale seriale in funzione di Baud Rate, Data Bits, Stop Bits
    double  minSerialSilenceTime(int nBaudRate, double dblCharTime_ms);                             // Calcolo del minimo tempo di Silence per una linea seriale
    int     blockReadTime_ms(int nType, int nRegisters, int nSilence_ms, double dblCharTime_ms);    // Calcolo del Tempo di lettura di un Blocco in funzione del numero di registri e del tempo di lettura di un char
    int     readHoldigRegisters_ms(int nRegisters, int nSilence_ms, double dblCharTime_ms);         // Calcolo del Tempo di lettura di un blocco di Input/Holding Registers in funzione del # Registri, Silence, charTime
    int     readCoils_ms(int nCoils, int nSilence_ms, double dblCharTime_ms);                       // Calcolo del Tempo di lettura di un blocco di Bit Registers (Coils) in funzione del # Bit, Silence, charTime
    bool    isSilenceOk(int nSilence_ms, int nBaudRate, double dblCharTime_ms);                     // Verifica che il tempo di silence specificato sia adeguato al BaudRate corrente
    int     searchBlock(int nBlock);                // Ricerca in theBlocks del blocco nBlock
    // Import dati in XML
    bool    getRowsFromXMLBuffer(QString &szBuffer, QList<QStringList> &lstPastedRecords, QList<int> &lstSourceRows, QList<int> &lstDestRows); // Funzione per leggere da Buffer Clipboard o da una QString (caricata da file XML) delle righe di CT
    bool    readModelVars(const QString szModelName, QList<CrossTableRecord> &lstModelVars);            // Lettura da XML delle variabili CT precaricate per modelli
    bool    checkFreeArea(int nStartRow, int nRows);                                                // Controlla che l'area di destinazione per inserire variabili sia sufficientemente capiente

    bool    addModelVars(const QString szModelName, int nRow, int nPort, int nNode);
    void    showTabMPNC();
    void    showTabMPNE();

    bool    isVarBlock(int nRow, QList<int> &lstBlockStart, int nBlockSize);        // Verifica se la variabile appartiene ad un Blocco identificato da Block Start e Block Size
    bool    isMPNC_Row(int nRow);               // Vero se la riga corrente appartiene ad un blocco MPNC
    bool    isMPNE_Row(int nRow);               // Vero se la riga corrente appartiene ad un blocco MPNE
    bool    isMPNE05_Row(int nRow);             // Vero se la riga corrente appartiene ad un blocco MPNE100105
    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------
    // Gestione Interfaccia
    Ui::ctedit *ui;
    MectSettings    *mectSet;
    TrendEditor     *trendEdit;
    Config_MPNC     *configMPNC;
    Config_MPNE     *configMPNE;
    SearchVariable  *searchForm;

    int         m_nGridRow;                         // Riga corrente sul Grid
    QString     m_szCurrentCTFile;                  // File Crosstable corrente (completo di Path)
    QString     m_szCurrentCTPath;                  // Path del file Crosstable corrente
    QString     m_szCurrentPLCPath;                 // Path della parte PLC del Progetto corrente
    QString     m_szCurrentCTName;                  // Nome del file Crosstable (senza Path)
    QString     m_szCurrentProjectPath;             // Project Path corrente
    QString     m_szCurrentProjectName;             // Project Name senza Path
    QString     m_szCurrentModel;                   // Modello TPAC del progetto
    QString     m_szTemplateCTFile;                 // Nome del file di CT originale da Template
    // Liste varie per prompt colonne e valori Combo Box (per traduzioni)
    QList<bool> lstBusEnabler;
    QStringList lstProductNames;

    // Variabili di servizio
    QString     m_szFormatDate;                     // Format Masks per Date e tempo
    QString     m_szFormatTime;
    QString     m_szMsg;                            // Variabile di servizio per Messaggi
    QTimer      *tmrMessage;                        // Timer per la gestione Messaggi


    // Record CrossTable
    QList<CrossTableRecord> lstCopiedRecords;       // Lista di Record per copia/incolla
    QList<QList<CrossTableRecord> > lstUndo;        // Lista degli Undo di elementi di Crosstable Editor
    CrossTableRecord        CrossTable[1 + DimCrossTable];	 // campi sono riempiti a partire dall'indice 1

    // System Configuration
    QList<TP_Config>        lstTargets;             // Lista di tutti i Target definiti
    // Controllo e Gestione  Errori
    QList<int>              lstAllUpdates;          // Lista contenente tutti i tipi di Update (per filtro su Nomi variabili)
    QList<int>              lstNoHUpdates;          // Lista contenente tutti i tipi di Update tranne H (per Allarmi)
    QList<int>              lstLogUpdates;          // Lista contenente tutti i tipi di Update
    QList<int>              lstAllVarTypes;         // Lista contenente tutti i tipi di Variabili definiti
    QList<int>              lstSelectedRows;        // Lista delle righe di CT in Editing
    QStringList             lstUsedVarNames;        // Lista contenente i nomi delle variabili (per Search)
    QStringList             lstLoggedVars;          // Lista contenente i nomi delle variabili Loggate (per Trends)
    QList<Err_CT>           lstCTErrors;
    QStringList             lstUniqueVarNames;      // Lista per controllo univocità identificatori
    QStringList             lstErrorMessages;       // Lista dei messaggi di errore
    // Gestione Server ModBus
    MODBUS_Srv              serverModBus[srvTotals];// Elenco dei Server ModBus definiti nel Sistema
    //------------------------------------------------------------------
    // Gestione Nodi / Servers per Tab Devices
    //------------------------------------------------------------------
    serverStruct            theServers[nMAX_SERVERS];   // Array con Informazioni Server definiti
    deviceStruct            theDevices[nMAX_DEVICES];   // Array con Informazioni Devices definiti
    nodeStruct              theNodes[nMAX_NODES];       // Array con struttura dei Nodi definiti
    blockStruct             theBlocks[nMAX_BLOCKS];     // Array con struttura dei Blocchi definiti
    int                     theServersNumber;
    int                     theDevicesNumber;
    int                     theTcpDevicesNumber;
    int                     thePlcVarsNumber;
    int                     theNodesNumber;
    int                     theBlocksNumber;

    // Variabili di stato ad uso globale
    int                     m_nCurTab;              // Tab corrente in interfaccia
    bool                    m_isCtModified;         // Flag dati modificato
    bool                    m_isConfModified;       // Flag Configurazione modificata
    bool                    m_isTrendModified;      // Flag Trend Modificato
    bool                    m_rebuildDeviceTree;    // Flag Vero se l'albero dei Device deve essere ricostruito
    bool                    m_rebuildTimingTree;    // Flag Vero se l'albero dei Device deve essere ricostruito
    bool                    m_fShowAllRows;         // Vero se sono visualizzate tutte le righe
    bool                    m_fCutOrPaste;          // Vero se è in corso un Cut or Paste
    bool                    m_fEmptyForm;           // Vero se il Form di Data Entry risulta vuoto
    varTypes                m_vtAlarmVarType;       // Tipo della variabile SX in un espressione Allarme/Evento
    int                     m_nAlarmDecimals;       // Numero di Decimali della Variabile SX di un Allarme
    bool                    m_fSkipLine;            // Se vero non devono essere fatti controlli sulla riga in uscita
    bool                    m_fMultiEdit;           // Flag per abilitare il MultiEdit di righe (da CheckBox)
    QList<int>              lstEditableFields;      // Lista delle colonne editabili in MultiLineEdit
    // Gestione nodi su MPNC
    QList<int>              lstMPNC;                // Liste delle righe capofila delle teste MPNC presenti
    int                     m_nMPNC;                // Indice della Testa MPNC correntemente visualizzata
    // Gestione nodi su MPNE
    QList<int>              lstMPNE;                // Liste delle capofila delle teste MPNE presenti
    int                     m_nMPNE;                // Indice della Testa MPNE correntemente visualizzata
    // Gestione modulo MPNE_05
    bool                    m_fMPNE100105_Present;  // Vero se nelle variabili è stato riconosciuto un modulo MPNE100105 (TPX1070 only)
    QList<int>              lstMPNE100105;
};

#endif // CTEDIT_H
