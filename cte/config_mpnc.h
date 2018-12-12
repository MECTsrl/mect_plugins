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
#include <QGridLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSignalMapper>
#include <QList>
#include <QStringList>



class Config_MPNC : public QWidget
{
    Q_OBJECT
public:
    explicit Config_MPNC(QWidget *parent = 0);


signals:

public slots:
    void    showTestaNodi(int nTesta, QList<int> &lstCapofilaTeste, QList<CrossTableRecord> &lstRows);

private slots:
    void    customizeButtons();                 // Abilitazione delle icone Bottoni in funzione della presenza dei moduli
    void    buttonClicked(int nButton);         // Gestore della pressione dei bottoni (Define Module)
    void    groupItemRemove(int nGroup);        // Rimozione elemento da gruppo
    void    getUsedModules(int nRow);           // Legge a partire dalla riga del Capofila il numero di Moduli utilizzati
    void    changeRootElement(int nItem);       // Cambio di Item della Combo dei MPNC definiti
    void    filterVariables(int nGroup, int nItem); // Filtra le variabili specifiche del modulo identificato da Gruppo e Posizione

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------
    int     getLastModuleUsed(int nGroup);      // Ricerca dell'ultimo modulo usato nel gruppo
    int     rel2AbsModulePos(int nGroup, int nModule);  // Calcola la posizione assoluta del Modulo
    void    abs2RelModulePos(int nAbs, int &nGroup, int &nModule);  // Calcola la posizione assoluta del Modulo
    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------
    // Oggetti di Interfaccia e gestione
    QWidget *               myParent;           // Puntatore a CT Editor
    QVBoxLayout*            externalLayOut;     // LayOut più esterno
    QGridLayout             *mainGrid;          // Main Grid LayOut
    QTableWidget            *tblCT;             // Table Widget per CT
    QComboBox               *cboSelector;       // Combo Box selettore MPNC
    QList<QPushButton *>    lstRemove;          // Lista per i bottoni Remove Modules
    QList<QPushButton *>    lstPulsanti;        // Lista per i bottoni Moduli
    QList<bool>             lstModuleIsPresent; // Lista abilitazione moduli
    QString                 szModuleStyle;      // Style di base dei Bottoni per i Moduli
    QString                 szRemoveStyle;      // Style di base dei Bottoni per Remove Modules
    QStringList             lstSfondi;          // Lista dei nomi degli sfondi associati ai Bottoni
    QStringList             lstModuleName;      // Lista dei nomi dei moduli
    QStringList             lstPosFlags;           // Lista dei nomi delle Posizioni (A..D)
    QSignalMapper           *mapRemoveClicked;  // Signal Mapper per Remove Module Clicked
    QSignalMapper           *mapModuleClicked;  // Signal Mapper per Module Clicked
    QString                 m_szMsg;            // Messaggio di servizio
    // Gestione della testa selezionata e lista delle teste MPNC definite nel sistema
    int                     m_nTesta;           // Testa selezionata, -1 se non presente
    QList<int >             lstCapofila;        // # Riga della elemento capofila della Testa nEsima
    int                     m_nBaseRow;         // Base Row della riga corrente
    QList<CrossTableRecord> lstCTUserRows;      // Lista Record CT (Parte Utente)

};

#endif // CONFIG_MPNC_H
