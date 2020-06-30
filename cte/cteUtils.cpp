#include "ctecommon.h"
#include "parser.h"
#include "utils.h"

#include <QObject>
#include <QtGui>
#include <QColor>
#include <QBrush>
#include <QString>
#include <QLatin1String>
#include <QList>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFont>
#include <QFontMetrics>
#include <QHeaderView>
#include <QDebug>



QStringList lstPriority;                // Elenco dei valori di Priority
QStringList lstPriorityDesc;            // Descrizioni Priority
QStringList lstBehavior;                // Significato variabili
QStringList lstCondition;               // Operatori Logici per Allarmi
QStringList lstHeadCols;                // Header Colonne Grid principale
QStringList lstHeadNames;               // Nome della colonna nel Grid principale (per XML)
QList<int>  lstHeadSizes;               // Largezza Header Grid Principale
QList<int > lstHeadLeftCols;            // Indici di colonna con allineamento a SX

QStringList lstRegions;                 // Aree della CT
QStringList lstTipi;                    // Descrizione dei Tipi
QStringList lstUpdateNames;             // Descrizione delle Priorità
QStringList lstProtocol;                // Descrizione dei Protocolli
QStringList lstMPNxCols;                // Header Colonne MPNx
QList<int>  lstMPNxHeadSizes;           // Largezza Header MPNx
QList<int>  lstMPNxHeadLeftCols;        // Indici di colonna MPNx con allineamento a SX
QStringList lstSearchCols;              // Header Colonne Search
QList<int>  lstSearchHeadSizes;         // Largezza Header Search
QList<int>  lstSearchHeadLeftCols;      // Indici di colonna Search con allineamento a SX
bool        isSerialPortEnabled;        // Vero se almeno una porta seriale è abilitata
int         nPresentSerialPorts;        // Numero di porte Seriali utilizzabili a bordo
TP_Config   panelConfig;                // Configurazione corrente del Target letta da Form mectSettings


// Cross Table Records
QList<CrossTableRecord> lstCTRecords;   // Lista completa di record per tabella (condivisa tra vari Oggetti di CTE)
QList<CrossTableRecord> lstTemplateRecs; // Lista completa di Record da Template (per confronto con CT)
QList<CrossTableRecord> lstMPNC006_Vars;// Lista delle Variabili MPNC006
QList<CrossTableRecord> lstTPLC050_Vars;// Lista delle Variabili TPLC050
QList<CrossTableRecord> lstMPNE_Vars;   // Lista delle Variabili MPNE

// Colori per sfondi grid
QColor      colorRetentive[2];
QColor      colorNonRetentive[2];
QColor      colorSystem[2];
QColor      colorGray;
QColor      colorMultiEdit;
QColor      colorNormalEdit;

QString     szColorRet[2];
QString     szColorNonRet[2];
QString     szColorSystem[2];
QString     szColorMultiEdit;
QString     szColorNormalEdit;

void initLists()
// Init delle Liste globali
{
    int nCol = 0;

    lstPriority.clear();
    for (nCol = 0; nCol < nNumPriority; nCol++)  {
        lstPriority.append(QString::number(nCol));
    }
    lstPriorityDesc.clear();
    lstPriorityDesc
        << QLatin1String("None")
        << QLatin1String("High")
        << QLatin1String("Medium")
        << QLatin1String("Low")
        ;
    // Lista Significati (da mantenere allineata con enum behaviors in parser.h)
    lstBehavior.clear();
    lstBehavior
            << QLatin1String("Read/Only  (%I)")
            << QLatin1String("Read/Write (%Q)")
            << QLatin1String("Alarm")
            << QLatin1String("Event")
        ;
    // Lista condizioni di Allarme / Eventi
    lstCondition.clear();
    for (nCol = 0; nCol < oper_totals; nCol++)  {
        lstCondition.append(QLatin1String(logic_operators[nCol]));
    }
    // Titoli colonne
    lstHeadCols.clear();
    lstHeadSizes.clear();
    lstHeadNames.clear();
    for (nCol = 0; nCol < colTotals; nCol++)  {
        lstHeadCols.append(szEMPTY);
        lstHeadNames.append(szEMPTY);
        lstHeadSizes.append(10);
    }
    //-----------------------------------------
    // Grid Principale CT
    //-----------------------------------------
    lstHeadCols[colPriority] = QLatin1String("Priority");
    lstHeadNames[colPriority] = QLatin1String("Priority");
    lstHeadSizes[colPriority] = 8;
    lstHeadCols[colUpdate] = QLatin1String("Update");
    lstHeadNames[colUpdate] = QLatin1String("Update");
    lstHeadSizes[colUpdate] = 8;
    lstHeadCols[colGroup] = QLatin1String("Group");
    lstHeadNames[colGroup] = QLatin1String("Group");
    lstHeadSizes[colGroup] = 8;
    lstHeadCols[colModule] = QLatin1String("Module");
    lstHeadNames[colModule] = QLatin1String("Module");
    lstHeadSizes[colModule] = 8;
    lstHeadCols[colName] = QLatin1String("Var.Name");
    lstHeadNames[colName] = QLatin1String("Var_Name");
    lstHeadSizes[colName] = 20;
    lstHeadCols[colType] = QLatin1String("Type");
    lstHeadNames[colType] = QLatin1String("Type");
    lstHeadSizes[colType] = 10;
    lstHeadCols[colDecimal] = QLatin1String("Decimal");
    lstHeadNames[colDecimal] = QLatin1String("Decimal");
    lstHeadSizes[colDecimal] = 8;
    lstHeadCols[colProtocol] = QLatin1String("Protocol");
    lstHeadNames[colProtocol] = QLatin1String("Protocol");
    lstHeadSizes[colProtocol] = 10;
    lstHeadCols[colIP] = QLatin1String("IP Address");
    lstHeadNames[colIP] = QLatin1String("IP_Address");
    lstHeadSizes[colIP] = 18;
    lstHeadCols[colPort] = QLatin1String("Port");
    lstHeadNames[colPort] = QLatin1String("Port");
    lstHeadSizes[colPort] = 8;
    lstHeadCols[colNodeID] = QLatin1String("Node ID");
    lstHeadNames[colNodeID] = QLatin1String("Node_ID");
    lstHeadSizes[colNodeID] = 8;
    lstHeadCols[colInputReg] = QLatin1String("I/R");
    lstHeadNames[colInputReg] = QLatin1String("I_R");
    lstHeadSizes[colInputReg] = 8;
    lstHeadCols[colRegister] = QLatin1String("Register");
    lstHeadNames[colRegister] = QLatin1String("Register");
    lstHeadSizes[colRegister] = 8;
    lstHeadCols[colBlock] = QLatin1String("Block");
    lstHeadNames[colBlock] = QLatin1String("Block");
    lstHeadSizes[colBlock] = 8;
    lstHeadCols[colBlockSize] = QLatin1String("Blk Size");
    lstHeadNames[colBlockSize] = QLatin1String("Blk_Size");
    lstHeadSizes[colBlockSize] = 8;
    lstHeadCols[colComment] = QLatin1String("Comment");
    lstHeadNames[colComment] = QLatin1String("Comment");
    lstHeadSizes[colComment] = 25;
    lstHeadCols[colBehavior] = QLatin1String("Behavior");
    lstHeadNames[colBehavior] = QLatin1String("Behavior");
    lstHeadSizes[colBehavior] = 16;
    lstHeadCols[colSourceVar] = QLatin1String("Source");
    lstHeadNames[colSourceVar] = QLatin1String("Source");
    lstHeadSizes[colSourceVar] = 20;
    lstHeadCols[colCondition] = QLatin1String("Condition");
    lstHeadNames[colCondition] = QLatin1String("Condition");
    lstHeadSizes[colCondition] = 10;
    lstHeadCols[colCompare] = QLatin1String("Compare");
    lstHeadNames[colCompare] = QLatin1String("Compare");
    lstHeadSizes[colCompare] = 20;
    // Colonne allineate a SX nel Grid
    lstHeadLeftCols.clear();
    lstHeadLeftCols.append(colName);
    lstHeadLeftCols.append(colComment);
    lstHeadLeftCols.append(colSourceVar);
    lstHeadLeftCols.append(colCompare);
    //-----------------------------------------
    // Grid MPNx
    //-----------------------------------------
    lstMPNxHeadSizes.clear();
    lstMPNxCols.clear();
    for (nCol = 0; nCol < colMPNxTotals; nCol++)  {
        lstMPNxHeadSizes.append(10);
        lstMPNxCols.append(szEMPTY);
    }
    lstMPNxCols[colMPNxPriority] = lstHeadCols[colPriority];
    lstMPNxHeadSizes[colMPNxPriority] = lstHeadSizes[colPriority];
    lstMPNxCols[colMPNxUpdate] = lstHeadCols[colUpdate];
    lstMPNxHeadSizes[colMPNxUpdate] = lstHeadSizes[colUpdate];
    lstMPNxCols[colMPNxGroup] =  lstHeadCols[colGroup];
    lstMPNxHeadSizes[colMPNxGroup] = lstHeadSizes[colGroup];
    lstMPNxCols[colMPNxModule] =  lstHeadCols[colModule];
    lstMPNxHeadSizes[colMPNxModule] = lstHeadSizes[colModule];
    lstMPNxCols[colMPNxName] = lstHeadCols[colName];
    lstMPNxHeadSizes[colMPNxName] = lstHeadSizes[colName];
    lstMPNxCols[colMPNxType] = lstHeadCols[colType];
    lstMPNxHeadSizes[colMPNxType] = lstHeadSizes[colType];
    lstMPNxCols[colMPNxDecimal] = lstHeadCols[colDecimal];
    lstMPNxHeadSizes[colMPNxDecimal] = lstHeadSizes[colDecimal];
    lstMPNxCols[colMPNxNodeID] = lstHeadCols[colNodeID];
    lstMPNxHeadSizes[colMPNxNodeID] = lstHeadSizes[colNodeID];
    lstMPNxCols[colMPNxRegister] = lstHeadCols[colRegister];
    lstMPNxHeadSizes[colMPNxRegister] = lstHeadSizes[colRegister];
    lstMPNxCols[colMPNxBlock] = lstHeadCols[colBlock];
    lstMPNxHeadSizes[colMPNxBlock] = lstHeadSizes[colBlock];
    lstMPNxCols[colMPNxBlockSize] = lstHeadCols[colBlockSize];
    lstMPNxHeadSizes[colMPNxBlockSize] = lstHeadSizes[colBlockSize];
    lstMPNxCols[colMPNxBehavior] = lstHeadCols[colBehavior];
    lstMPNxHeadSizes[colMPNxBehavior] = lstHeadSizes[colBehavior];
    lstMPNxCols[colMPNxComment] = lstHeadCols[colComment];
    lstMPNxHeadSizes[colMPNxComment] = lstHeadSizes[colComment];
    lstMPNxCols[colMPNxService] = QLatin1String("Service");
    lstMPNxHeadSizes[colMPNxService] = 30;
    // Colonne allineate a SX nel Grid MPNx
    lstMPNxHeadLeftCols.clear();
    lstMPNxHeadLeftCols.append(colMPNxName);
    lstMPNxHeadLeftCols.append(colMPNxComment);
    //-----------------------------------------
    // Grid for Search
    //-----------------------------------------
    lstSearchCols.clear();
    lstSearchHeadSizes.clear();
    for (nCol = 0; nCol < colSearchTotals; nCol++)  {
        lstSearchHeadSizes.append(10);
        lstSearchCols.append(szEMPTY);
    }
    lstSearchCols[colSearchNumRow] = QLatin1String("Row");
    lstSearchHeadSizes[colSearchNumRow] = lstHeadSizes[colPriority];
    lstSearchCols[colSearchPriority] = lstHeadCols[colPriority];
    lstSearchHeadSizes[colSearchPriority] = lstHeadSizes[colPriority];
    lstSearchCols[colSearchUpdate] = lstHeadCols[colUpdate];
    lstSearchHeadSizes[colSearchUpdate] = lstHeadSizes[colUpdate];
    lstSearchCols[colSearchName] = lstHeadCols[colName];
    lstSearchHeadSizes[colSearchName] = lstHeadSizes[colName];
    lstSearchCols[colSearchType] = lstHeadCols[colType];
    lstSearchHeadSizes[colSearchType] = lstHeadSizes[colType];
    lstSearchCols[colSearchDecimal] = lstHeadCols[colDecimal];
    lstSearchHeadSizes[colSearchDecimal] = lstHeadSizes[colDecimal];
    lstSearchCols[colSearchProtocol] = lstHeadCols[colProtocol];
    lstSearchHeadSizes[colSearchProtocol] = lstHeadSizes[colProtocol];
    lstSearchCols[colSearchIP] = lstHeadCols[colIP];
    lstSearchHeadSizes[colSearchIP] = lstHeadSizes[colIP];
    lstSearchCols[colSearchPort] = lstHeadCols[colPort];
    lstSearchHeadSizes[colSearchPort] = lstHeadSizes[colPort];
    lstSearchCols[colSearchNodeID] = lstHeadCols[colNodeID];
    lstSearchHeadSizes[colSearchNodeID] = lstHeadSizes[colNodeID];
    lstSearchCols[colSearchRegister] = lstHeadCols[colRegister];
    lstSearchHeadSizes[colSearchRegister] = lstHeadSizes[colRegister];
    lstSearchCols[colSearchBehavior] = lstHeadCols[colBehavior];
    lstSearchHeadSizes[colSearchBehavior] = lstHeadSizes[colBehavior];
    lstSearchCols[colSearchComment] = lstHeadCols[colComment];
    lstSearchHeadSizes[colSearchComment] = lstHeadSizes[colComment];
    // Colonne allineate a SX nel Grid Search
    lstSearchHeadLeftCols.clear();
    lstSearchHeadLeftCols.append(colSearchName);
    lstSearchHeadLeftCols.append(colSearchComment);
    //-----------------------------------------
    // Regioni CT
    //-----------------------------------------
    lstRegions.clear();
    for (nCol = 0; nCol < regTotals; nCol++)  {
        lstRegions.append(szEMPTY);
    }
    lstRegions[regRetentive]    = QLatin1String("Retentive\t[1 -  192]");
    lstRegions[regNonRetentive] = QLatin1String("Non Retentive\t[193 - 4999]");
    lstRegions[regDiagnostic]   = QLatin1String("Diagnostic\t[5000 - 5299]");
    lstRegions[regLocalIO]      = QLatin1String("Local I/O\t[5300 - 5389]");
    lstRegions[regSystem]       = QLatin1String("System\t[5390 - 5472]");
    // Descrizione Tipi Variabili
    lstTipi.clear();
    for (nCol = 0; nCol < TYPE_TOTALS - 1; nCol++)  {
        lstTipi.append(QLatin1String(varTypeNameExtended[nCol]));
    }
    // Lista Update
    lstUpdateNames.clear();
    for (nCol = Htype; nCol < UPDATE_TOTALS; nCol++)  {
        lstUpdateNames.append(QLatin1String(updateTypeName[nCol]));
    }
    // Lista Protocolli
    lstProtocol.clear();
    for (nCol = PLC; nCol < FIELDBUS_TOTAL; nCol++)  {
        lstProtocol.append(QLatin1String(fieldbusName[nCol]));
    }
    // Costanti per i colori di sfondo
    colorRetentive[0] = QColor(170,255,255,255);           // Azzurro Dark
    colorRetentive[1] = QColor(210,255,255,255);           // Azzurro
    colorNonRetentive[0] = QColor(255,255,190,255);        // Giallino Dark
    colorNonRetentive[1] = QColor(255,255,220,255);        // Giallino
    colorSystem[0] = QColor(255,227,215,255);              // Rosa Dark
    colorSystem[1] = QColor(255,240,233,255);              // Rosa
    colorGray = QColor(200,200,200,255);                   // Gray
    colorMultiEdit = QColor();
    szColorRet[0] = QLatin1String("color: #AAFFFF");
    szColorRet[1] = QLatin1String("color: #D2FFFF");
    szColorNonRet[0] = QLatin1String("color: #FFFFBE");
    szColorNonRet[1] = QLatin1String("color: #FFFFDC");
    szColorSystem[0] = QLatin1String("color: #FFE3D7");
    szColorSystem[1] = QLatin1String("color: #FFF0E9");
    szColorMultiEdit = QLatin1String("color: #87CEEB");

    // Righe della CT
    lstCTRecords.clear();
    // Porte Seriali
    isSerialPortEnabled = false;        // Vero se almeno una porta seriale è abilitata
    nPresentSerialPorts = 0;            // Numero di porte Seriali utilizzabili a bordo
}

void setRowColor(QTableWidget *table, int nRow, int nAlternate, int nUsed, int nPriority, int nBaseOffset)
// Imposta il colore di sfondo di una riga
{
    QColor      cSfondo = colorRetentive[0];
    int         nZoneRow = nRow + nBaseOffset;

    // Impostazione del Backgound color in funzione della zona
    if (nZoneRow >= 0 && nZoneRow < MAX_RETENTIVE)  {
        cSfondo = colorRetentive[nAlternate];
    }
    else if (nZoneRow >= MIN_NONRETENTIVE - 1 && nZoneRow <= MAX_NONRETENTIVE -1) {
        cSfondo = colorNonRetentive[nAlternate];
    }
    else if (nZoneRow >= MIN_DIAG - 1)  {
        if (nZoneRow <= MAX_DIAG - 1)  {
            cSfondo = colorSystem[0];
        }
        else if (nZoneRow >= MIN_NODE - 1 && nZoneRow <= MAX_NODE - 1)  {
            cSfondo = colorSystem[1];
        }
        else if (nZoneRow >= MIN_LOCALIO - 1  && nZoneRow <= MAX_LOCALIO - 1)  {
            cSfondo = colorSystem[0];
        }
        else  {
            cSfondo = colorSystem[1];
        }
    }
    // Righe utilizzate ma a priorità 0
    if (nUsed && nPriority == 0)  {
        // cSfondo = colorGray;
        // cSfondo = cSfondo.lighter(150);
        cSfondo = cSfondo.darker(130);
    }
    // qDebug("setRowColor(): (Row: %d Base: %d) @[%d] --> [R:%d G:%d B:%d]", nRow, nBaseOffset, nZoneRow, cSfondo.red(), cSfondo.green(), cSfondo.blue());
    // Impostazione del colore di sfondo
    QBrush bCell(cSfondo, Qt::SolidPattern);
    setRowBackground(bCell, table->model(), nRow);
}

bool recCT2MPNxFieldsValues(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow, QList<CrossTableRecord> &lstModel, int nModelRow)
// Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
// Da Record C a QStringList di valori per caricamento griglia
// Versione per MPNX Nodes. La lista lstModel è NECESSARIA per recuperare le informazioni di Group e Model che sono presenti SOLO NEL MODELLO
// e non salvate in file CSV della CT
{
    if (nRow < 0 || nRow >= CTRecords.count())  {
        return false;
    }
    listClear(lstRecValues, colMPNxTotals);
    // Recupero informazioni da Record CT
    if (CTRecords[nRow].UsedEntry)  {
        // Priority
        if (CTRecords[nRow].Enable >= 0 && CTRecords[nRow].Enable < nNumPriority)  {
            lstRecValues[colMPNxPriority] = lstPriority.at((int) CTRecords[nRow].Enable);
        }
        // Campo Update
        if (CTRecords[nRow].Update >= 0 && CTRecords[nRow].Update < lstUpdateNames.count())
            lstRecValues[colMPNxUpdate] = lstUpdateNames[CTRecords[nRow].Update];
        // Campo Group
        lstRecValues[colMPNxGroup] = QString::number(lstModel[nModelRow].Group);
        // Campo Module
        lstRecValues[colMPNxModule] = QString::number(lstModel[nModelRow].Module);
        // Campo Name
        lstRecValues[colMPNxName] = QLatin1String(CTRecords[nRow].Tag);
        // Campo Type
        if (CTRecords[nRow].VarType >= BIT && CTRecords[nRow].VarType < TYPE_TOTALS)
            lstRecValues[colMPNxType] = lstTipi[CTRecords[nRow].VarType];
        // Campo Decimal
        lstRecValues[colMPNxDecimal] = QString::number(CTRecords[nRow].Decimal);
        // Node Id
        lstRecValues[colMPNxNodeID] = QString::number(CTRecords[nRow].NodeId);
        // Offeset Register
        lstRecValues[colMPNxRegister] = QString::number(CTRecords[nRow].Offset);
        // Block
        lstRecValues[colMPNxBlock] = QString::number(CTRecords[nRow].Block);
        // N.Registro
        lstRecValues[colMPNxBlockSize] = QString::number(CTRecords[nRow].BlockSize);
        // Behavior
        // R/O o R/W
        if (CTRecords[nRow].Behavior == behavior_readonly)
            lstRecValues[colMPNxBehavior] = lstBehavior[behavior_readonly];
        else if (CTRecords[nRow].Behavior == behavior_readwrite)
            lstRecValues[colMPNxBehavior] = lstBehavior[behavior_readwrite];
        // Commento
        lstRecValues[colMPNxComment] = QString(QLatin1String(CTRecords[nRow].Comment)).trimmed().left(MAX_COMMENT_LEN - 1);
        // Service
        lstRecValues[colMPNxService] = szEMPTY;
    }
    //    qDebug() << QLatin1String("recCT2List() - Parsed Row: %1") .arg(nRow);
    // Return value
    return true;
}

bool recCT2SearchFieldsValues(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow)
// Conversione da CT Record a Lista Stringhe per Grid Search (REC -> Grid)
// Da Record C a QStringList di valori per caricamento griglia
{
    QString     szTemp;
    char        ip[MAX_IPADDR_LEN];

    if (nRow < 0 || nRow >= CTRecords.count())  {
        return false;
    }
    listClear(lstRecValues, colSearchTotals);
    // Recupero informazioni da Record CT
    if (CTRecords[nRow].UsedEntry)  {
        // Numero Riga
        szTemp = QString::fromAscii("%1") .arg(nRow + 1, lstSearchHeadSizes[colSearchNumRow], 10);
        lstRecValues[colSearchNumRow] = szTemp;
        // Priority
        if (CTRecords[nRow].Enable >= 0 && CTRecords[nRow].Enable < nNumPriority)  {
            lstRecValues[colSearchPriority] = lstPriority.at((int) CTRecords[nRow].Enable);
        }
        else   {
            lstRecValues[colSearchPriority] = lstPriority.at(0);
        }
        // Campo Update
        if (CTRecords[nRow].Update >= 0 && CTRecords[nRow].Update < lstUpdateNames.count())  {
            lstRecValues[colSearchUpdate] = lstUpdateNames[CTRecords[nRow].Update];
        }
        else  {
            lstRecValues[colSearchUpdate] = lstUpdateNames[0];
        }
        // Campo Name
        lstRecValues[colSearchName] = QLatin1String(CTRecords[nRow].Tag);
        // Campo Type
        if (CTRecords[nRow].VarType >= BIT && CTRecords[nRow].VarType < TYPE_TOTALS)  {
            lstRecValues[colSearchType] = lstTipi[CTRecords[nRow].VarType];
        }
        else  {
            lstRecValues[colSearchType] = lstTipi[0];
        }
        // Campo Decimal
        lstRecValues[colSearchDecimal] = QString::fromAscii("%1") .arg(CTRecords[nRow].Decimal, lstSearchHeadSizes[colSearchDecimal], 10);
        // Protocol
        if (CTRecords[nRow].Protocol >= 0 && CTRecords[nRow].Protocol < lstProtocol.count())  {
            lstRecValues[colSearchProtocol] = lstProtocol[CTRecords[nRow].Protocol];
        }
        else  {
            lstRecValues[colSearchProtocol] = lstProtocol[PLC];
        }
        // IP Address (Significativo solo per Protocolli a base TCP)
        if (CTRecords[nRow].Protocol == TCP || CTRecords[nRow].Protocol == TCPRTU ||
                CTRecords[nRow].Protocol == TCP_SRV || CTRecords[nRow].Protocol ==TCPRTU_SRV)  {
            ipaddr2str(CTRecords[nRow].IPAddress, ip);
            szTemp = QLatin1String(ip);
        }
        else  {
            szTemp = szEMPTY;
        }
        lstRecValues[colSearchIP] = szTemp;
        // Port
        lstRecValues[colSearchPort] = QString::fromAscii("%1") .arg(CTRecords[nRow].Port, lstSearchHeadSizes[colSearchPort], 10);
        // Node Id
        lstRecValues[colSearchNodeID] = QString::fromAscii("%1") .arg(CTRecords[nRow].NodeId, lstSearchHeadSizes[colSearchNodeID], 10);
        // Offeset (Register)
        lstRecValues[colSearchRegister] = QString::fromAscii("%1") .arg(CTRecords[nRow].Offset, lstSearchHeadSizes[colSearchRegister], 10);
        // PLC forza tutto a Blank
        if (CTRecords[nRow].Protocol == PLC)  {
            lstRecValues[colSearchPort] = szEMPTY;
            lstRecValues[colSearchNodeID] = szEMPTY;
            lstRecValues[colSearchRegister] = szEMPTY;
        }
        // Commento
        lstRecValues[colSearchComment] = QString::fromAscii(CTRecords[nRow].Comment).trimmed().left(MAX_COMMENT_LEN - 1);
        // Behavior
        // Allarme o Evento (non MPNx)
        if (CTRecords[nRow].usedInAlarmsEvents && CTRecords[nRow].Behavior >= behavior_alarm)  {
            // Tipo Allarme-Evento
            if (CTRecords[nRow].ALType == Alarm)
                lstRecValues[colSearchBehavior] = lstBehavior[behavior_alarm];
            else if (CTRecords[nRow].ALType == Event)
                lstRecValues[colSearchBehavior] = lstBehavior[behavior_event];
        }
        else   {
            // R/O o R/W
            if (CTRecords[nRow].Behavior == behavior_readonly)
                lstRecValues[colSearchBehavior] = lstBehavior[behavior_readonly];
            else if (CTRecords[nRow].Behavior == behavior_readwrite)
                lstRecValues[colSearchBehavior] = lstBehavior[behavior_readwrite];
        }
    }
    // Return value
    return true;
}

bool recCT2FieldsValues(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow)
// Conversione da CT Record a record come Lista Stringhe per Interfaccia (Grid)
// Da Record C a QStringList di valori per caricamento griglia
{
    QString     szTemp;
    char        ip[MAX_IPADDR_LEN];

    if (nRow < 0 || nRow >= CTRecords.count())  {
        return false;
    }
    // Pulizia Buffers
    szTemp.clear();
    listClear(lstRecValues, colTotals);
//    qDebug() << QLatin1String("recCT2FieldsValues(): Processing Row=[%1]") .arg(nRow);
    // Recupero informazioni da Record CT
    if (CTRecords[nRow].UsedEntry)  {
        // Priority
        if (CTRecords[nRow].Enable >= 0 && CTRecords[nRow].Enable < nNumPriority)  {
            lstRecValues[colPriority] = lstPriority.at((int) CTRecords[nRow].Enable);
        }
        else   {
            lstRecValues[colPriority] = lstPriority.at(0);
        }
        // Campo Update
        if (CTRecords[nRow].Update >= 0 && CTRecords[nRow].Update < lstUpdateNames.count())  {
            lstRecValues[colUpdate] = lstUpdateNames[CTRecords[nRow].Update];
        }
        else  {
            lstRecValues[colUpdate] = lstUpdateNames[0];
        }
        // Campo Group
        if (CTRecords[nRow].Group >= 0 && CTRecords[nRow].Group < nMax_Int16)  {
            lstRecValues[colGroup] = QString::number(CTRecords[nRow].Group);
        }
        else  {
            lstRecValues[colGroup] = szZERO;
        }
        // Campo Module
        if (CTRecords[nRow].Module >= 0 && CTRecords[nRow].Module < nMax_Int16)  {
            lstRecValues[colModule] = QString::number(CTRecords[nRow].Module);
        }
        else  {
            lstRecValues[colModule] = szZERO;
        }
        // Campo Name
        lstRecValues[colName] = QLatin1String(CTRecords[nRow].Tag);
        // Campo Type
        if (CTRecords[nRow].VarType >= BIT && CTRecords[nRow].VarType < TYPE_TOTALS)  {
            lstRecValues[colType] = lstTipi[CTRecords[nRow].VarType];
        }
        else  {
            lstRecValues[colType] = lstTipi[0];
        }
        // Campo Decimal
        lstRecValues[colDecimal] = QString::number(CTRecords[nRow].Decimal);
        // Protocol
        if (CTRecords[nRow].Protocol >= 0 && CTRecords[nRow].Protocol < lstProtocol.count())  {
            lstRecValues[colProtocol] = lstProtocol[CTRecords[nRow].Protocol];
        }
        else  {
            lstRecValues[colProtocol] = lstProtocol[PLC];
        }
        // IP Address (Significativo solo per Protocolli a base TCP)
        if (CTRecords[nRow].Protocol == TCP || CTRecords[nRow].Protocol == TCPRTU ||
                CTRecords[nRow].Protocol == TCP_SRV || CTRecords[nRow].Protocol ==TCPRTU_SRV)  {
            ipaddr2str(CTRecords[nRow].IPAddress, ip);
            szTemp = QLatin1String(ip);
        }
        else  {
            szTemp = szEMPTY;
        }
        lstRecValues[colIP] = szTemp;
        // Port
        lstRecValues[colPort] = QString::number(CTRecords[nRow].Port);
        // Node Id
        lstRecValues[colNodeID] = QString::number(CTRecords[nRow].NodeId);
        // Input Register
        lstRecValues[colInputReg] = (CTRecords[nRow].InputReg > 0) ? szTRUE : szFALSE;
        // Offeset (Register)
        lstRecValues[colRegister] = QString::number(CTRecords[nRow].Offset);
        // Block
        lstRecValues[colBlock] = QString::number(CTRecords[nRow].Block);
        // Block Size
        lstRecValues[colBlockSize] = QString::number(CTRecords[nRow].BlockSize);
        // PLC forza tutto a Blank
        if (CTRecords[nRow].Protocol == PLC)  {
            lstRecValues[colPort] = szEMPTY;
            lstRecValues[colNodeID] = szEMPTY;
            lstRecValues[colInputReg] = szEMPTY;
            lstRecValues[colRegister] = szEMPTY;
        }
        // Commento
        lstRecValues[colComment] = QString::fromAscii(CTRecords[nRow].Comment).trimmed().left(MAX_COMMENT_LEN - 1);
        // Behavior
        // Allarme o Evento (non MPNx)
        if (CTRecords[nRow].usedInAlarmsEvents && CTRecords[nRow].Behavior >= behavior_alarm)  {
            // Tipo Allarme-Evento
            if (CTRecords[nRow].ALType == Alarm)
                lstRecValues[colBehavior] = lstBehavior[behavior_alarm];
            else if (CTRecords[nRow].ALType == Event)
                lstRecValues[colBehavior] = lstBehavior[behavior_event];
            // Operatore Logico
            if (CTRecords[nRow].ALOperator >= 0 && CTRecords[nRow].ALOperator < oper_totals)
                lstRecValues[colCondition] = lstCondition[CTRecords[nRow].ALOperator];
            else
                lstRecValues[colCondition] = szEMPTY;
            // Source Var
            lstRecValues[colSourceVar] = QLatin1String(CTRecords[nRow].ALSource);
            // Compare Var or Value
            szTemp = QLatin1String(CTRecords[nRow].ALCompareVar);
            if (szTemp.isEmpty())
                lstRecValues[colCompare] = QString::number(CTRecords[nRow].ALCompareVal, 'f', 4);
            else
                lstRecValues[colCompare] = szTemp;
            // Rising o Falling senza seconda parte
            if (CTRecords[nRow].ALOperator == oper_rising || CTRecords[nRow].ALOperator == oper_falling)
                lstRecValues[colCompare] = szEMPTY;
        }
        else   {
            // R/O o R/W
            if (CTRecords[nRow].Behavior == behavior_readonly)
                lstRecValues[colBehavior] = lstBehavior[behavior_readonly];
            else if (CTRecords[nRow].Behavior == behavior_readwrite)
                lstRecValues[colBehavior] = lstBehavior[behavior_readwrite];
            // Source Var - Condition - Compare
            lstRecValues[colSourceVar] = szEMPTY;
            lstRecValues[colCondition] = szEMPTY;
            lstRecValues[colCompare] = szEMPTY;
        }
//        qDebug() << QLatin1String("recCT2FieldsValues(): Row=[%1] - usedInAlarmsEvents=[%2] - Behavior=[%3]") .arg(nRow) .arg(CTRecords[nRow].usedInAlarmsEvents) .arg(CTRecords[nRow].Behavior);
    }
    // qDebug() << QLatin1String("recCT2FieldsValues(): Processed Row=[%1]") .arg(nRow);
    // Return value
    return true;
}

void freeCTrec(QList<CrossTableRecord> &lstCTRecs, int nRow)
// Marca il Record della CT come inutilizzato
{
    if (nRow < 0 || nRow >= lstCTRecs.count())
        return;
    lstCTRecs[nRow].UsedEntry = 0;
    lstCTRecs[nRow].Enable = 0;
    lstCTRecs[nRow].Update = (UpdateType) 0;
    strcpy(lstCTRecs[nRow].Tag, "");
    lstCTRecs[nRow].VarType = (varTypes) 0;
    lstCTRecs[nRow].Decimal = 0;
    lstCTRecs[nRow].Protocol = (FieldbusType) 0;
    lstCTRecs[nRow].IPAddress = 0;
    lstCTRecs[nRow].Port = 0;
    lstCTRecs[nRow].NodeId = 0;
    lstCTRecs[nRow].InputReg = 0;
    lstCTRecs[nRow].Group = 0;
    lstCTRecs[nRow].Module = 0;
    lstCTRecs[nRow].Offset = 0;
    lstCTRecs[nRow].Block = 0;
    lstCTRecs[nRow].BlockBase = 0;
    lstCTRecs[nRow].Behavior = 0;
    lstCTRecs[nRow].Counter = 0;
    lstCTRecs[nRow].OldVal = 0;
    lstCTRecs[nRow].Error = 0;
    lstCTRecs[nRow].nDevice = 0xffff;
    lstCTRecs[nRow].nNode = 0xffff;
    lstCTRecs[nRow].nBlock = 0xffff;
    lstCTRecs[nRow].usedInAlarmsEvents = FALSE;
    lstCTRecs[nRow].ALType = -1;
    strcpy(lstCTRecs[nRow].ALSource, "");
    lstCTRecs[nRow].ALOperator = -1;
    strcpy(lstCTRecs[nRow].ALCompareVar, "");
    lstCTRecs[nRow].ALCompareVal = 0.0;
    lstCTRecs[nRow].ALComparison = -1;
    lstCTRecs[nRow].ALCompatible  = FALSE;
    strcpy(lstCTRecs[nRow].Comment, "");
}

bool fieldValues2CTrecList(QStringList &lstRecValues, QList<CrossTableRecord> &lstCTRecs, int nRow)
// Conversione da Lista Valori di Interfaccia a CT Record (Form -> REC SINGOLO)
// Scrive un Record letto da interfaccia direttamente in lista di Record C
{
    bool        fRes = true;
    bool        fOk = false;
    int         nPos = 0;
    char        ip[MAX_IPADDR_LEN];

    // Abilitazione riga (Nome Vuoto => Riga disabilitata)
    if (lstRecValues[colName].isEmpty())  {
        freeCTrec(lstCTRecs, nRow);
    }
    else  {
        // Priority
        nPos = lstPriority.indexOf(lstRecValues[colPriority]);
        nPos = (nPos >= 0 && nPos < nNumPriority) ? nPos : 0;
        lstCTRecs[nRow].Enable = (int16_t) nPos;
        // Update
        nPos = lstUpdateNames.indexOf(lstRecValues[colUpdate]);
        nPos = (nPos >= 0 && nPos < lstUpdateNames.count()) ? nPos : 0;
        lstCTRecs[nRow].Update = (UpdateType) nPos;
        // Group
        nPos = lstRecValues[colGroup].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecs[nRow].Group = nPos;
        // Module
        nPos = lstRecValues[colModule].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecs[nRow].Module = nPos;
        // Campo Name
        strcpy(lstCTRecs[nRow].Tag, lstRecValues[colName].trimmed().toAscii().data());
        // Campo Abilitazione record
        if (strlen(lstCTRecs[nRow].Tag) > 0)  {
            lstCTRecs[nRow].UsedEntry = 1;
        }
        else  {
            lstCTRecs[nRow].UsedEntry = 0;
        }
        // Campo Type
        nPos = lstTipi.indexOf(lstRecValues[colType]);
        nPos = (nPos >= 0 && nPos < lstTipi.count()) ? nPos : 0;
        lstCTRecs[nRow].VarType = (varTypes) nPos;
        // Campo Decimal
        nPos = lstRecValues[colDecimal].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecs[nRow].Decimal = nPos;
        // Protocol
        nPos = lstProtocol.indexOf(lstRecValues[colProtocol]);
        nPos = (nPos >= 0 && nPos < lstProtocol.count()) ? nPos : 0;
        lstCTRecs[nRow].Protocol = (FieldbusType) nPos;
        // IP Address (Significativo solo per Protocolli a base TCP)
        if (lstCTRecs[nRow].Protocol == TCP || lstCTRecs[nRow].Protocol == TCPRTU ||
                lstCTRecs[nRow].Protocol == TCP_SRV || lstCTRecs[nRow].Protocol ==TCPRTU_SRV)  {
            strcpy(ip, lstRecValues[colIP].trimmed().toAscii().data());
            nPos = str2ipaddr(ip);
            lstCTRecs[nRow].IPAddress = (uint32_t) nPos;
        }
        else
            lstCTRecs[nRow].IPAddress = (uint32_t) 0 ;
        // Port
        nPos = lstRecValues[colPort].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecs[nRow].Port = nPos;
        // Node Id
        nPos = lstRecValues[colNodeID].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecs[nRow].NodeId = nPos;
        // Flag Input Register
        lstCTRecs[nRow].InputReg = (lstRecValues[colInputReg] == szTRUE) ? 1 : 0;
        // OffSet Register
        nPos = lstRecValues[colRegister].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecs[nRow].Offset = nPos;
        // Block
        nPos = lstRecValues[colBlock].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        if (nPos == 0)  {
            lstCTRecs[nRow].Block = nRow + 1;
            lstCTRecs[nRow].BlockSize = 1;
        }
        else {
            lstCTRecs[nRow].Block = nPos;
            // N.Registro
            nPos = lstRecValues[colBlockSize].toInt(&fOk);
            nPos = fOk ? nPos : 1;
            lstCTRecs[nRow].BlockSize = nPos;
        }
        // Commento
        strncpy(lstCTRecs[nRow].Comment, lstRecValues[colComment].trimmed().toAscii().data(), MAX_COMMENT_LEN - 1);
        // Behavior    (No per MultiSelect)
        // Clear all Variable - Event fields
        lstCTRecs[nRow].usedInAlarmsEvents = FALSE;
        lstCTRecs[nRow].ALType = -1;
        strcpy(lstCTRecs[nRow].ALSource, szEMPTY.toAscii().data());
        lstCTRecs[nRow].ALOperator = -1;
        strcpy(lstCTRecs[nRow].ALCompareVar, szEMPTY.toAscii().data());
        lstCTRecs[nRow].ALCompareVal = 0.0;
        lstCTRecs[nRow].ALComparison = -1;
        lstCTRecs[nRow].ALCompatible = 0;
        // Behavior
        nPos = lstBehavior.indexOf(lstRecValues[colBehavior]);
        nPos = (nPos >= 0 && nPos < lstBehavior.count()) ? nPos : 0;
        lstCTRecs[nRow].Behavior = nPos;
        // Salvataggio dei valori di Allarme/Evento
        if (nPos >= behavior_alarm)   {
            // Flag isAlarm
            lstCTRecs[nRow].usedInAlarmsEvents = TRUE;
            // Type of Alarm or Event
            if (nPos == behavior_alarm)
                lstCTRecs[nRow].ALType = Alarm;
            else
                lstCTRecs[nRow].ALType = Event;
            // Left Variable Name
            strcpy(lstCTRecs[nRow].ALSource, lstRecValues[colSourceVar].trimmed().toAscii().data());
            // Operator
            nPos = lstCondition.indexOf(lstRecValues[colCondition]);
            nPos = (nPos >= 0 && nPos < lstCondition.count()) ? nPos : -1;
            lstCTRecs[nRow].ALOperator = nPos;
            // Compare VAR - VAL
            QString szCompare = lstRecValues[colCompare].trimmed();
            if (szCompare.isEmpty())  {
                strcpy(lstCTRecs[nRow].ALCompareVar, szEMPTY.toAscii().data());
                lstCTRecs[nRow].ALCompareVal = 0.0;
            }
            else  {
                // Decisione se il secondo lato dell'espressione sia una costante o un nome variabile
                QChar cc = szCompare.at(0);
                if (cc.isLetter())  {
                    // Variable
                    strcpy(lstCTRecs[nRow].ALCompareVar, szCompare.toAscii().data());
                    lstCTRecs[nRow].ALCompareVal = 0.0;
                }
                else  {
                    float fValue = 0;
                    // Value
                    strcpy(lstCTRecs[nRow].ALCompareVar, szEMPTY.toAscii().data());
                    fValue = szCompare.toFloat(&fOk);
                    fValue = fOk ? fValue : 0.0;
                    lstCTRecs[nRow].ALCompareVal = fValue;
                    // TODO: Fill correct values for Comparison and Compatible
                    lstCTRecs[nRow].ALComparison = COMP_UNSIGNED;
                    lstCTRecs[nRow].ALCompatible = 1;
                }
            }
        }
    }
    // Return Value
    return fRes;
}

int     compareCTwithTemplate(QList<CrossTableRecord> &CTProject, QList<CrossTableRecord> &CTTemplate, TP_Config &configTP, QList<int> &lstDiff, QStringList &lstActions, bool forceDiff)
// Confronta la CT del Progetto con quella di Template di modello aggiornato (Solo in Area di Sistema da 5000 a salire, tranne che per TPAC1008_03)
{
    int     nDifferences = 0;
    int     nRow = 0;
    int     nAnInStart = configTP.analogINrowCT - 1;
    int     nAnInEnd = nAnInStart + configTP.analogIN;
    int     nAnOutStart = configTP.analogOUTrowCT - 1;
    int     nAnOutEnd = nAnOutStart + configTP.analogOUT;

    // Confronta il numero di elementi
    if (CTProject.count() != CTTemplate.count())  {
        return -1;
    }
    // Clear List
    lstDiff.clear();
    lstActions.clear();
    int nStart = MIN_DIAG - 1;
    // Compare CT Area starting from 5000
    if (configTP.modelName == QLatin1String(product_name[TPAC1008_03_AC]) ||
        configTP.modelName == QLatin1String(product_name[TPAC1008_03_AD]))  {
        nStart = 4500;
    }
    for (nRow = nStart; nRow < CTProject.count(); nRow++)  {
        // Repaint
        doEvents();
        // Variabile Template marcata come NON Usata --> ha la priorità il Template Variabile utente disabilitata
        if (CTTemplate[nRow].Enable == 0 && strlen(CTTemplate[nRow].Tag) > 0 && strlen(CTProject[nRow].Tag) > 0 && CTProject[nRow].Enable)  {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QLatin1String("Variable switched OFF"));
            if (forceDiff)  {
                CTProject[nRow].Enable = 0;
            }
        }
        // Variabile Template con priorità differente --> ha la priorità il Template
        if (CTTemplate[nRow].Enable > 0 && strlen(CTTemplate[nRow].Tag) > 0 && strlen(CTProject[nRow].Tag) > 0 && CTProject[nRow].Enable != CTTemplate[nRow].Enable )  {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QString::fromAscii("Forced Priority to %1") .arg(CTTemplate[nRow].Enable));
            if (forceDiff)  {
                CTProject[nRow].Enable = CTTemplate[nRow].Enable;
            }
        }
        // Variabile aggiunta in Template ma non in Project
        if (CTTemplate[nRow].UsedEntry && ! CTProject[nRow].UsedEntry)  {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QLatin1String("New Variable added to Project"));
            if (forceDiff)  {
                CTProject[nRow] = CTTemplate[nRow];
            }
        }
        // Blocco e Block Size
        if (CTTemplate[nRow].UsedEntry && CTProject[nRow].UsedEntry  &&
            (CTTemplate[nRow].Block != CTProject[nRow].Block || CTTemplate[nRow].BlockSize != CTProject[nRow].BlockSize))  {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QLatin1String("Changed Block or Block Size"));
            if (forceDiff)  {
                CTProject[nRow].Block = CTTemplate[nRow].Block;
                CTProject[nRow].BlockSize = CTTemplate[nRow].BlockSize;
            }
        }
        // Tipo
        if (CTTemplate[nRow].UsedEntry && CTProject[nRow].UsedEntry &&
            CTTemplate[nRow].VarType != CTProject[nRow].VarType)   {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QLatin1String("Changed Type"));
            if (forceDiff)  {
                CTProject[nRow].VarType = CTTemplate[nRow].VarType;
            }
        }
        // Decimali Cambiati (Salvo analogiche)
        if (CTTemplate[nRow].UsedEntry && CTProject[nRow].UsedEntry &&
            (CTTemplate[nRow].Decimal != CTProject[nRow].Decimal &&
             ! ((nRow >= nAnInStart && nRow <= nAnInEnd) ||  (nRow >= nAnOutStart && nRow <= nAnOutEnd))))  {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QLatin1String("Changed Decimals"));
            if (forceDiff)  {
                CTProject[nRow].Decimal = CTTemplate[nRow].Decimal;
            }
        }
        // Commento solo in Template ---> Copiato in Progetto
        if (CTTemplate[nRow].UsedEntry && CTProject[nRow].UsedEntry &&
            strlen(CTTemplate[nRow].Comment) > 0 &&
            strlen(CTProject[nRow].Comment) == 0)  {
            nDifferences++;
            lstDiff.append(nRow);
            lstActions.append(QString::fromAscii("Added Comment: %1") .arg(QLatin1String(CTTemplate[nRow].Comment)));
            if (forceDiff)  {
                strcpy(CTProject[nRow].Comment, CTTemplate[nRow].Comment);
            }
        }
    }
    return nDifferences;
}

int     countLoggedVars(QList<CrossTableRecord> &CTRecords, int &nFast, int &nSlow, int &nOnVar, int &nOnShot)
// Conta il Numero delle Variabili CT che sono Loggate
{
    int     nLoggedVars = 0;
    int     nRow = 0;
    int     nUpdate = 0;

    nFast = 0;
    nSlow = 0;
    nOnVar = 0;
    nOnShot = 0;

    for (nRow = 0; nRow < CTRecords.count(); nRow++)  {
        nUpdate = CTRecords[nRow].Update;
        if (nUpdate > Ptype)  {
            nLoggedVars++;
            if (nUpdate == Ftype)
                nFast++;
            else if (nUpdate == Stype)
                nSlow++;
            else if (nUpdate == Vtype)
                nOnVar++;
            else if (nUpdate == Xtype)
                nOnShot++;
        }
    }
    // Return Value
    return nLoggedVars;
}

bool    list2GridRow(QTableWidget *table,  QStringList &lstRecValues, QList<int> &lstLeftCols, int nRow)
// Inserimento o modifica elemento in Grid (valori -> GRID)
{
    int                 nCol = 0;
    QString             szTemp;
    QTableWidgetItem    *tItem;
    bool                fAdd = false;

    // Insert Items at Row, Col
    for (nCol = 0; nCol < lstRecValues.count(); nCol++)  {
        szTemp = lstRecValues[nCol];
        tItem = table->item(nRow, nCol);
        // Allocazione Elemento se non già definito
        if (tItem == NULL)  {
            tItem = new QTableWidgetItem(szTemp);
            fAdd = true;
        }
        else  {
            tItem->setText(szTemp);
        }
        // Allineamento Celle
        if (lstLeftCols.indexOf(nCol) >= 0)  {
            // Item Allineato a Sx
            tItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else  {
            // Item Centrato in Cella
            tItem->setTextAlignment(Qt::AlignCenter);
        }
        // Rende il valore non Editabile
        tItem->setFlags(tItem->flags() ^ Qt::ItemIsEditable);
        // Aggiorna il Grid
        if (fAdd)  {
            table->setItem(nRow, nCol, tItem);
        }
    }
    return true;
}

void getFirstPortFromProtocol(int nProtocol, int &nPort, int &nTotal)
// Cerca la prima porta disponibile in funzione del protocollo e della configurazione corrente
// Ritorna -1 se il protocollo non è disponibile sul modello o tutte le porte sono disabilitate
{

    nPort = -1;
    nTotal = 0;

    switch (nProtocol) {
        // Protocolli Seriali
        case RTU:
        case RTU_SRV:
        case MECT_PTC:
            if (panelConfig.ser0_Enabled)  {
                nPort = 0;
                nTotal++;
            }
            if (panelConfig.ser1_Enabled)  {
                if (nPort < 0)
                    nPort = 1;
                nTotal++;
            }
            if (panelConfig.ser2_Enabled)  {
                if (nPort < 0)
                    nPort = 2;
                nTotal++;
            }
            if (panelConfig.ser3_Enabled)  {
                if (nPort < 0)
                    nPort = 3;
                nTotal++;
            }
            break;
        // Protocolli TCP
        case TCP:
        case TCPRTU:
        case TCP_SRV:
        case TCPRTU_SRV:
            if (panelConfig.ethPorts > 0)  {
                nPort = szDEF_IP_PORT.toInt(0);
                nTotal++;
            }
            break;
        // Protocollo CAN
        case CANOPEN:
            if (panelConfig.can0_Enabled)  {
                nPort = 0;
                nTotal++;
            }
            if (panelConfig.can1_Enabled)  {
                if (nPort < 0)
                    nPort = 1;
                nTotal++;
            }
            break;
        default:
            nPort = 0;
            nTotal = 1;
            break;
    }
}

int     enableSerialPortCombo(QComboBox *cboBox)
{
    int nPorts = 0;

    disableAndBlockSignals(cboBox);
    // Port 0
    if (panelConfig.ser0_Enabled)  {
        enableComboItem(cboBox, 0);
        nPorts++;
    }
    else  {
        disableComboItem(cboBox, 0);
    }
    // Port 1
    if (panelConfig.ser1_Enabled)  {
        enableComboItem(cboBox, 1);
        nPorts++;
    }
    else  {
        disableComboItem(cboBox, 1);
    }
    // Port 2
    if (panelConfig.ser2_Enabled)  {
        enableComboItem(cboBox, 2);
        nPorts++;
    }
    else  {
        disableComboItem(cboBox, 2);
    }
    // Port 3
    if (panelConfig.ser3_Enabled)  {
        enableComboItem(cboBox, 3);
        nPorts++;
    }
    else  {
        disableComboItem(cboBox, 3);
    }
    enableAndUnlockSignals(cboBox);
    // Return Value
    return nPorts;
}

void    setGridParams(QTableWidget *table, QStringList &lstHeadCols, QList<int> &lstHeadSizes, QAbstractItemView::SelectionMode nMode)
// Imposta i parametri generali di visualizzazione Grid
{
    int         nColWidth = 0;
    int         nColHeight = 0;
    int         nCol = 0;

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(nMode);
    table->setHorizontalHeaderLabels(lstHeadCols);
    // Larghezza fissa per alcune colonne
    QFontMetrics fm(table->font());
    QString szTemp;
    for (nCol = 0; nCol < lstHeadCols.count(); nCol++)  {
        szTemp.fill(chX, lstHeadSizes[nCol]);
        nColWidth = fm.width(szTemp) * 1.2;
        nColHeight = fm.height();
        table->setColumnWidth(nCol, nColWidth);
    }
    // Altezza Righe
    nColHeight = fm.height() * 2;
    QHeaderView *verticalHeader = table->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(nColHeight);
    // qDebug() << QLatin1String("setGridParams():    Col Height: %1") .arg(nColHeight);

}

bool    isValidVarName(QString szName)
{
    bool    fRes = true;
    char    anyCh;
    int     i = 0;

    if (!szName.isEmpty())  {
        // Controllo che la variabile non cominci con un carattere numerico
        for (i = 0; i < szName.length(); i++)  {
            anyCh = szName.at(i).toAscii();
            // First Ch is Number
            if (i == 0 && (anyCh <= '9' && anyCh >= '0'))  {
                fRes = false;
                break;
            }
            // Others char must be Digit, Printable Chars or '_'
            if (! ((anyCh >= '0' && anyCh <= '9') ||
                   (anyCh >= 'A' && anyCh <= 'Z') ||
                   (anyCh >= 'a' && anyCh <= 'z') ||
                   (anyCh == '_')))  {
                fRes = false;
                break;
            }
        }
    }
    else  {
        fRes = false;
    }
    // Return value
    return fRes;
}

bool    searchIOModules(const QString szModule, QList<CrossTableRecord> &CTRecords, QList<CrossTableRecord> &CT_IOModule, QList<int> &lstRootRows)
// Ricerca di un Modulo I/O in CT
{
    int     nRow = 0;
    int     nModelRow = 0;
    int     nBaseRow = -1;
    int     nModelSize = CT_IOModule.count();

    lstRootRows.clear();
    if (CTRecords.count() > 0 && CT_IOModule.count() > 0 && CTRecords.count() >= nModelSize)  {
        while (nRow < MIN_DIAG)  {
            // Controllo di identità tra Variabili CT e modello
            if ( CTRecords[nRow].VarType == CT_IOModule[nModelRow].VarType &&
                 // CTRecords[nRow].Decimal == CTModel[nModelRow].Decimal &&
                 CTRecords[nRow].Protocol == CT_IOModule[nModelRow].Protocol &&
                 CTRecords[nRow].Offset == CT_IOModule[nModelRow].Offset &&
                 CTRecords[nRow].Behavior == CT_IOModule[nModelRow].Behavior
                )  {
                // Il controllo del Numero Decimali è significativo solo nel caso dei vari Tipi Bit
                if ((CTRecords[nRow].VarType == BIT ||
                     CTRecords[nRow].VarType == BYTE_BIT ||
                     CTRecords[nRow].VarType == WORD_BIT ||
                     CTRecords[nRow].VarType == DWORD_BIT)  &&
                    (CTRecords[nRow].Decimal != CT_IOModule[nModelRow].Decimal)
                    )  {
                    // Abbandona il confronto perchè la maschera di Bit non è identica
                    nModelRow = 0;
                    nBaseRow = -1;
                    goto nextRow;
                }
                // Segna la base del modello
                if (nModelRow == 0)  {
                    nBaseRow = nRow;
                }
                // Passa alla riga successiva del modello
                nModelRow++;
                // Tutto il modello è Ok
                if (nModelRow == nModelSize && nBaseRow >= 0)  {
                    // Aggiunge base alla Lista delle Basi modello
                    lstRootRows.append(nBaseRow);
                    // Riporta all'inizio il confronto
                    nModelRow = 0;
                    // qDebug() << QString::fromAscii("searchIOModules() - Model Found %1 @ Row: %2") .arg(szModule) .arg(nBaseRow);
                }
            }
            else  {
                // Riporta all'inizio il confronto
                nModelRow = 0;
                nBaseRow = -1;
            }
        nextRow:        // Row Increment
            nRow++;
        }
    }
    // Return Value
    return (lstRootRows.count() > 0);
}

QString     priority2String(int nPriority){
    return (QString::fromAscii("Priority %1 - %2") .arg(nPriority) .arg(lstPriorityDesc[nPriority]));
}

QString getSerialPortSpeed(int nPort)
// Restituisce in forma leggibile i parametri della porta seriale selezionata
{
    QString     szSpeed = QLatin1String("Disabled");

    if (isSerialPortEnabled)  {
        switch (nPort)  {
            case 0:
                if (panelConfig.ser0_Enabled)  {
                    szSpeed = QString::fromAscii("%1,%2,%3,%4")
                            .arg(panelConfig.ser0_BaudRate) .arg(panelConfig.ser0_Parity) .arg(panelConfig.ser0_DataBits) .arg(panelConfig.ser0_StopBits);
                }
                break;
                ;
            case 1:
                if (panelConfig.ser1_Enabled)  {
                    szSpeed = QString::fromAscii("%1,%2,%3,%4")
                            .arg(panelConfig.ser1_BaudRate) .arg(panelConfig.ser1_Parity) .arg(panelConfig.ser1_DataBits) .arg(panelConfig.ser1_StopBits);
                }
                break;
                ;
            case 2:
                if (panelConfig.ser2_Enabled)  {
                    szSpeed = QString::fromAscii("%1,%2,%3,%4")
                            .arg(panelConfig.ser2_BaudRate) .arg(panelConfig.ser2_Parity) .arg(panelConfig.ser2_DataBits) .arg(panelConfig.ser2_StopBits);
                }
                break;
            case 3:
                if (panelConfig.ser3_Enabled)  {
                    szSpeed = QString::fromAscii("%1,%2,%3,%4")
                            .arg(panelConfig.ser3_BaudRate) .arg(panelConfig.ser3_Parity) .arg(panelConfig.ser3_DataBits) .arg(panelConfig.ser3_StopBits);
                }
                break;
            default:
                break;
        }
    }
    // Return value
    return szSpeed;
}
