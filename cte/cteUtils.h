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
extern QStringList lstRegions;          // Aree della CT
extern QStringList lstTipi;             // Descrizione dei Tipi
extern QStringList lstUpdateNames;      // Descrizione delle Priorità
extern QStringList lstProtocol;         // Descrizione dei Protocolli
extern QStringList lstMPNxCols;         // Header Colonne MPNx
extern bool        isSerialPortEnabled; // Vero se almeno una porta seriale è abilitata
extern int         nPresentSerialPorts; // Numero di porte Seriali utilizzabili a bordo
extern TP_Config   panelConfig;         // Configurazione corrente del Target letta da Form mectSettings

// Cross Table Records
extern QList<CrossTableRecord> lstCTRecords;        // Lista completa di record per tabella (condivisa tra vari Oggetti di CTE)


// Colori per sfondi grid
extern QColor      colorRetentive[2];
extern QColor      colorNonRetentive[2];
extern QColor      colorSystem[2];
extern QColor      colorGray;
extern QString     szColorRet[2];
extern QString     szColorNonRet[2];
extern QString     szColorSystem[2];


void    initLists();                       // Init delle Liste globali
void    setRowColor(QTableWidget *table, int nRow, int nAlternate, int nUsed, int nPriority);   // Imposta il colore di sfondo di una riga
bool    recCT2MPNxList(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow);// Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
bool    recCT2List(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow);    // Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
bool    list2GridRow(QTableWidget *table, QStringList &lstRecValues, int nRow, bool fForceAlign = true);  // Inserimento o modifica elemento in Grid (valori -> GRID)
int     enableSerialPortCombo(QComboBox *cboBox);

#endif // CTEUTILS_H
