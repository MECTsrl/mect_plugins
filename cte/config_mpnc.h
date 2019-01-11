#ifndef CONFIG_MPNC_H
#define CONFIG_MPNC_H

#include "parser.h"
#include "ctecommon.h"

#include <QtGui>
#include <QWidget>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QTableWidget>
#include <QSignalMapper>
#include <QList>
#include <QStringList>



class Config_MPNC : public QWidget
{
    Q_OBJECT
public:
    explicit Config_MPNC(QWidget *parent = 0);
    QList<CrossTableRecord> localCTRecords;
    bool    isUpdated();                                                // Ritorna vero se il contenuto dei nodi è stato modificato
    int     getCurrentRow();                                            // Restituisce la riga correntemente selezionata

signals:
    void    varClicked(int  nVarRow);

public slots:
    void    showTestaNodi(int nTesta, QList<int> &lstCapofilaTeste, int nCurRow);

private slots:
    void    customizeButtons();                             // Abilitazione delle icone Bottoni in funzione della presenza dei moduli
    void    changeFilter();                                 // Cambio del filtro sui moduli
    void    buttonClicked(int nButton);                     // Gestore della pressione dei bottoni (Define Module)
    void    groupItemRemove(int nGroup);                    // Rimozione elemento da gruppo
    void    getUsedModules(int nBaseRow);                   // Legge a partire dalla riga del Capofila il numero di Moduli utilizzati
    void    changeRootElement(int nItem);                   // Cambio di Item della Combo dei MPNC definiti
    void    filterVariables(int nGroup, int nItem);         // Filtra le variabili specifiche del modulo identificato da Gruppo e Posizione
    void    on_changePort(int nPort);                       // Evento cambio Porta RTU
    void    on_changeNode();                                // Evento Cambio Nodo
    void    on_RenameVars();                                // Evento Rename Clicked
    void    onRowClicked(const QModelIndex &index);         // Evento Row Clicked
    void    onRowDoubleClicked(const QModelIndex &index);   // Evento Row Double Clicked

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    int     getLastModuleUsed(int nGroup);      // Ricerca dell'ultimo modulo usato nel gruppo
    int     relative2AbsModulePos(int nGroup, int nModule);                 // Calcola la posizione assoluta del Modulo
    void    abs2RelativeModulePos(int nAbs, int &nGroup, int &nModule);     // Calcola la posizione assoluta del Modulo
    void    setGroupVars(int nGroup, int nModule, int16_t nPriority);       // Imposta la Priority per le variabili di Gruppo e Modulo
    bool    canRenameRows(int nBaseRow);                                    // Verifica se tutto il Device può essere rinominato
    void    setFilterButton(int nNewMode);                                  // Imposta il fondo del botton cmd
    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------
    // Oggetti di Interfaccia e gestione
    QWidget *               myParent;           // Puntatore a CT Editor
    QVBoxLayout*            externalLayOut;     // LayOut più esterno
    QGridLayout             *mainGrid;          // Main Grid LayOut
    QTableWidget            *tblCT;             // Table Widget per CT
    QComboBox               *cboSelector;       // Combo Box selettore MPNC
    QPushButton             *cmdRename;         // Push Button per Rename Rows
    QPushButton             *cmdFilter;         // Push Button per Switch visualizzazione
    QLabel                  *lblProtocol;       // Label per Protocollo
    QComboBox               *cboPort;           // Combo per selettore Porta
    QLineEdit               *txtNode;           // Text Box per Node Id
    QList<QPushButton *>    lstRemove;          // Lista per i bottoni Remove Modules
    QList<QPushButton *>    lstPulsanti;        // Lista per i bottoni Moduli
    QList<QLabel *>         lstPosMarker;       // Lista di QLabel marker della posizione modulo
    QList<bool>             lstModuleIsPresent; // Lista abilitazione moduli
    QString                 szModuleStyle;      // Style di base dei Bottoni per i Moduli
    QString                 szRemoveStyle;      // Style di base dei Bottoni per Remove Modules
    QString                 szFilterStyle;      // Style di base dei Bottone Switch View
    QStringList             lstSfondi;          // Lista dei nomi degli sfondi associati ai Bottoni
    QStringList             lstModuleName;      // Lista dei nomi dei moduli
    QStringList             lstPosFlags;           // Lista dei nomi delle Posizioni (A..D)
    QSignalMapper           *mapRemoveClicked;  // Signal Mapper per Remove Module Clicked
    QSignalMapper           *mapModuleClicked;  // Signal Mapper per Module Clicked
    QString                 m_szMsg;            // Messaggio di servizio
    // Visualizzazione
    int                     m_nShowMode;        // Modalità di visualizzazione corrente
    // Gestione della testa selezionata e lista delle teste MPNC definite nel sistema    
    int                     m_nCurrentCTRow;    // Riga di CT correntemente selezionata
    int                     m_nAbsPos;          // Posizione correntemente visualizzata
    int                     m_nTotalRows;       // Elementi MPNC
    bool                    m_fUpdated;         // Vero se le info della finestra sono cambiate
    int                     m_nTesta;           // Testa selezionata, -1 se non presente
    QList<int >             lstCapofila;        // Lista con # Riga delle Teste
    // Parametri dell'Oggetto MPNC corrente
    int                     m_nBaseRow;         // Base Row della riga corrente    
    int                     m_nPort;            // Numero porta assegnato al Device
    int                     m_nNodeId;          // Numero porta assegnato al Device
    int16_t                 m_nRootPriority;    // Priorità complessiva del Nodo
    // Rename Variabili
    int                     m_nMaxVarName;      // Lunghezza massima di un nome di variabile in CT
    int                     m_nMinVarName;      // Lunghezza minima di un nome di variabile in CT
    QString                 m_szVarNamePrefix;  // Prefisso in comune per Nomi Variabili
    bool                    m_fCanRenameVars;   // Flag per abilitare funzione di Rinomina Nodi
};

#endif // CONFIG_MPNC_H
