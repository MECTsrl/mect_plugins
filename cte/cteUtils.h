#ifndef CTEUTILS_H
#define CTEUTILS_H

#include "ctecommon.h"
#include "parser.h"

#include <QColor>
#include <QString>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QComboBox>


extern QStringList lstPriority;         // Elenco dei valori di Priority
extern QStringList lstPriorityDesc;     // Descrizioni Priority
extern QStringList lstBehavior;         // Significato variabili
extern QStringList lstCondition;        // Operatori Logici per Allarmi
extern QStringList lstHeadCols;
extern QStringList lstHeadNames;
extern QList<int>  lstHeadSizes;
extern QList<int > lstHeadLeftCols;     // Indici di colonna con allineamento a SX

extern QStringList lstRegions;          // Aree della CT
extern QStringList lstTipi;             // Descrizione dei Tipi
extern QStringList lstUpdateNames;      // Descrizione delle Priorità
extern QStringList lstProtocol;         // Descrizione dei Protocolli
extern QStringList lstMPNxCols;         // Header Colonne MPNx
extern QList<int>  lstMNPxHeadSizes;    // Largezza Header MPNx
extern QList<int>  lstMPNxHeadLeftCols; // Indici di colonna MPNx con allineamento a SX

extern bool        isSerialPortEnabled; // Vero se almeno una porta seriale è abilitata
extern int         nPresentSerialPorts; // Numero di porte Seriali utilizzabili a bordo
extern TP_Config   panelConfig;         // Configurazione corrente del Target letta da Form mectSettings

// Cross Table Records
extern QList<CrossTableRecord> lstCTRecords;    // Lista completa di record per tabella (condivisa tra vari Oggetti di CTE)
extern QList<CrossTableRecord> lstMPNC006_Vars; // Lista delle Variabili MPNC006
extern QList<CrossTableRecord> lstTPLC050_Vars; // Lista delle Variabili TPLC050
extern QList<CrossTableRecord> lstMPNE_Vars;    // Lista delle Variabili MPNE


// Colori per sfondi grid
extern QColor      colorRetentive[2];
extern QColor      colorNonRetentive[2];
extern QColor      colorSystem[2];
extern QColor      colorGray;
extern QString     szColorRet[2];
extern QString     szColorNonRet[2];
extern QString     szColorSystem[2];


void    initLists();                       // Init delle Liste globali
void    setRowColor(QTableWidget *table, int nRow, int nAlternate, int nUsed, int nPriority, int nBaseOffset = 0);   // Imposta il colore di sfondo di una riga
bool    recCT2MPNxFieldsValues(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow, QList<CrossTableRecord> &lstModel, int nModelRow);// Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
bool    recCT2FieldsValues(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow);        // Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
bool    fieldValues2CTrecList(QStringList &lstRecValues, QList<CrossTableRecord> &lstCTRecs, int nRow);     // Conversione da Lista Stringhe a CT Record (Grid -> REC SINGOLO)
void    freeCTrec(QList<CrossTableRecord> &lstCTRecs, int nRow);                    // Marca il Record della CT come inutilizzato
int     countLoggedVars(QList<CrossTableRecord> &CTRecords, int &nFast, int &nSlow, int &nOnVar, int &nOnShot);                 // Conta il Numero delle Variabili CT che sono Loggate
bool    list2GridRow(QTableWidget *table,  QStringList &lstRecValues, QList<int> &lstLeftCols, int nRow);   // Inserimento o modifica elemento in Grid (valori -> GRID)

int     enableSerialPortCombo(QComboBox *cboBox);
void    setGridParams(QTableWidget *table, QStringList &lstHeadCols, QList<int> &lstHeadSizes, QAbstractItemView::SelectionMode nMode); // Imposta i parametri generali di visualizzazione Grid

bool    isValidVarName(QString szName);                                                                                         // Controllo del Nome Variabile
bool    searchIOModules(QList<CrossTableRecord> &CTRecords, QList<CrossTableRecord> &CT_IOModule, QList<int> &lstRootRows);     // Ricerca di un Modulo I/O in CT
QString priority2String(int nPriority);     // Formattazione stringa per nome priorità
QString getSerialPortSpeed(int nPort);      // Restituisce in forma leggibile i parametri della porta seriale selezionata
#endif // CTEUTILS_H
