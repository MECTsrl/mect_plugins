#ifndef CONFIG_MPNE_H
#define CONFIG_MPNE_H

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


class Config_MPNE : public QWidget
{
    Q_OBJECT
public:
    explicit Config_MPNE(QWidget *parent = 0);
    QList<CrossTableRecord> localCTRecords;                             // Copia Locale dei Record CT
    bool    isUpdated();                                                // Ritorna vero se il contenuto dei nodi è stato modificato
    int     getCurrentRow();                                            // Restituisce la riga correntemente selezionata

signals:
    void    varClicked(int  nVarRow);

protected:
     bool   eventFilter(QObject *obj, QEvent *event);        // Gestore Event Handler

public slots:
    void    showTestaNodi(int nTesta, QList<int> lstMPNE, int nCurRow);

private slots:
    void    changeFilter();                                 // Cambio del filtro sui moduli
    void    changeRootElement(int nItem);                   // Cambio di Item della Combo dei MPNC definiti
    void    onRowClicked(const QModelIndex &index);         // Evento Row Clicked
    void    onRowDoubleClicked(const QModelIndex &index);   // Evento Row Double Clicked
    void    on_changePort(int nPort);                       // Evento cambio Porta RTU
    void    on_changeNode();                                // Evento Cambio Nodo
    void    on_RenameVars();                                // Evento Rename Clicked

    void    onLeftModuleChanged(int nIndex);
    void    onRightModuleChanged(int nIndex);

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    void    getUsedModules(int nBaseRow);                   // Calcola a partire dalla riga del Capofila il numero e le Funzioni dei Moduli utilizzati
    void    setFilterButton(int nNewMode);                  // Imposta il fondo del botton cmd
    void    filterVariables(int nPosition, int nFunction);  // Filtra le variabili specifiche del modulo identificato da Gruppo e Posizione
    void    setGroupVars(int nPosition, int nFunction, int16_t nPriority);  // Imposta la Priority per le variabili di Posizione e Funzione
    void    customizeButtons();                             // Abilitazione delle icone Bottoni in funzione della presenza dei moduli
    bool    canRenameRows(int nBaseRow);                    // Verifica se tutto il Device può essere rinominato

    void    updateModule(int nPosition, int nFunction);
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
    QLabel                  *lblModuleLeft;     // Label per selezione modulo SX
    QLabel                  *lblModuleRight;    // Label per selezione modulo DX
    QLabel                  *lblProtocol;       // Label per Protocollo
    QLabel                  *lblBaudRate;       // Label per Baud Rate
    QLabel                  *lblNRows;          // Label per Numero Righe
    QComboBox               *cboPort;           // Combo per selettore Porta
    QComboBox               *cboLeft;           // Combo per modulo SX
    QComboBox               *cboRight;          // Combo per modulo DX
    QLineEdit               *txtNode;           // Text Box per Node Id
    QLabel                  *lblLeft;           // Label per Codice Left
    QLabel                  *lblRight;          // Label per Codice Right
    QList<int>              lstModuleUsage;     // Lista Utilizzo dei Moduli (0=Unused)
    QStringList             lstSfondi;          // Lista dei nomi degli sfondi associati ai Bottoni
    QStringList             lstModuleCode;      // Lista dei Codici dei Moduli
    QStringList             lstModuleColor;     // Lista dei Colori dei Moduli
    QList<QColor>           lstBackColor;
    QStringList             lstModuleName;      // Lista dei nomi dei moduli
    QList<int>              lstModuleFunction;  // Lista delle Funzionalità dei Moduli
    QFrame                  *fraMPNE_Left;      // Frame con immagine MPNE_L
    QFrame                  *fraMPNE_Right;     // Frame con immagine MPNE_R
    QString                 szFrameStyle;       // Style per Frame MPNE
    QString                 szModuleStyle;      // Style di base dei Bottoni per Item
    QString                 szFilterStyle;      // Style di base dei Bottone Switch View
    QString                 szCodeStyle;        // Style di base del codice modulo
    QString                 m_szMsg;            // Messaggio di servizio
    // Visualizzazione
    int                     m_nShowMode;        // Modalità di visualizzazione corrente
    // Gestione della testa selezionata e lista delle teste MPNC definite nel sistema
    int                     m_nCurrentCTRow;    // Riga di CT correntemente selezionata
    int                     m_nAbsPos;          // Posizione correntemente visualizzata
    int                     m_nTotalRows;       // Elementi MPNE
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

#endif // CONFIG_MPNE_H
