#ifndef UTILS_H
#define UTILS_H
#include <QString>
#include <QWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QColor>


// char constants
const  char     cDoubleQuote = 34;      // Carattere "
const  char     cSingleQuote = 39;      // '
const  char     cSpace = 32;            // Blank
const  char     cZero = 48;             // Zero
const  char     cFloat = 102;           // Carattere f per formattazione Floating Point


    QString     getPathFromFileName(const QString &fileName);               // Estrae il Path assoluto dal File Name ricevuto come parametro
    void        splitFileName(const QString &fileName, QString &szPath,
                       QString &szFile, QString &szSuffisso);               // Spezza un file name nelle sue componenti Path, Nome File, Suffisso
    bool        fileExists(const QString &szFile);                          // Controlla l'esistenza di un file
    bool        fileBackUp(const QString &szFile);                          // Crea una copia di BackUp di un file
    bool        queryUser(QWidget * parent, const QString &szTitle, const QString &szQueryMessage, bool fDefault = false);     // Domanda all'Utente se procedere, return True se Ok
    void        notifyUser(QWidget * parent, const QString &szTitle, const QString &szMessage);         // Notifica all'Utente il messaggio con solo il tasto Ok
    void        warnUser(QWidget * parent, const QString &szTitle, const QString &szMessage);           // Notifica all'Utente il messaggio con solo il tasto Ok e Icona Warning
    bool        checkIntField(const QString szField, int nMin, int nMax);   // Controlla che il valore contenuto nella stringa sia un numero compreso tra Min e Max
    QString     num2Hex(int nValue, int nLen = 2);                          // Converte un numero in stringa Hex, con padding a 0 a Sx per nLen Chars
    QString     int2PaddedString(int nValue, int nLen, int nBase=10);       // Converte un numero in stringa Dec/Hex, con padding a 0 a Sx per nLen Chars
    QString     long2PaddedString(quint64 nValue, int nLen, int nBase=10);  // Converte un numero long in stringa Dec/Hex, con padding a 0 a Sx per nLen Chars
    QString     parseQuote(const QString &szFieldValue);                    // Controlla gli eventuali Quote in una stringa prima di passarli ad uno Statement SQL
    QString     addQuote(const QString &szFieldValue);                      // Aggiunge Single Quote ad un campo
    QString     addDoubleQuote(const QString &szStringValue);               // Aggiunge Doppio Quote ad una stringa
    QString     string2CamelCase(const QString szValue);                    // Conversione di una stringa a Camel Case [Prima iniziale di ogni parola maiuscola]
    QString     szSpace(int nSpaces);                                       // Ritorna una stringa di nSpaces Spazio o vuota se nSpaces <= 0
    QString     bool2String(bool logicalValue, bool onlyYes = false);       // Ritorna Yes/No in funzione di logicalValue
    double      myRound(double dblVal, unsigned decPlaces);                 // Arrotondamento a n Cifre Decimali di un double
    void        listClear(QStringList &lstRecValues, int nCols);            // Svuotamento e pulizia Lista Stringhe
    void        setCellEnabled(QTableWidgetItem *tItem, bool fEnabled);     // Abilita o disabilita una cella di un QTableWidget
    void        setCellEditable(QTableWidgetItem *tItem, bool fEnabled);    // Permette Editing diretto su cella di un QTableWidget
    void        setCellSelectable(QTableWidgetItem *tItem, bool fEnabled);  // Permette la selezione della Cella
    void        setTableRowEnabled(QTableWidget * tTable, int nRow, bool fEnabled);              // Blocca/Sblocca completamente la Riga della Tabella
    void        setTableColEnabled(QTableWidget * tTable, int nCol, bool fEnabled);              // Blocca/Sblocca completamente la Colonna della Tabella
    bool        showFile(QString szFileName);                               // Utilizza i DeskTop Sevices per mostrare un File (es. PDF)
    void        setCellBackground(const QBrush& backBrush, QAbstractItemModel* model, int nRow, int nCol, const bool fForceForeground = false, const QModelIndex &parent = QModelIndex());
    void        setCellForeground(const QBrush& foreBrush, QAbstractItemModel* model, int nRow, int nCol, const QModelIndex &parent = QModelIndex());

    void        setRowBackground(const QBrush& brush, QAbstractItemModel* model, int row, const QModelIndex& parent = QModelIndex());
    void        doEvents();
    void        enableComboItem(QComboBox *Combo, int nItemIndex);          // Riabilita la voce nItemIndex della Combo
    void        disableComboItem(QComboBox *Combo, int nItemIndex);         // Disabilita la voce nItemIndex della Combo
    bool        disableAndBlockSignals(QWidget *widget);                    // Disabilita e blocca segnali di un oggetto QWidget*
    bool        enableAndUnlockSignals(QWidget *widget);                    // Riabilita e sblocca segnali di un oggetto QWidget*
    QColor      getIdealTextColor(const QColor& rBackgroundColor);          // Return an ideal label color, based on the given background color.
    QTreeWidgetItem *searchTreeChild(QTreeWidgetItem *tFather, int nCol, QString &szName);    // Search Child Item in Tree by Column and Name
#endif // UTILS_H
