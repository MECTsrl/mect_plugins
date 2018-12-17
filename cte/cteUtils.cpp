#include "ctecommon.h"
#include "parser.h"
#include "utils.h"

#include <QObject>
#include <QtGui>
#include <QColor>
#include <QBrush>
#include <QString>
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
QStringList lstHeadCols;
QStringList lstHeadNames;
QList<int>  lstHeadSizes;
QList<int > lstHeadLeftCols;            // Indici di colonna con allineamento a SX

QStringList lstRegions;                 // Aree della CT
QStringList lstTipi;                    // Descrizione dei Tipi
QStringList lstUpdateNames;             // Descrizione delle Priorità
QStringList lstProtocol;                // Descrizione dei Protocolli
QStringList lstMPNxCols;                // Header Colonne MPNx
QList<int>  lstMNPxHeadSizes;           // Largezza Header MPNx
QList<int>  lstMPNxHeadLeftCols;        // Indici di colonna MPNx con allineamento a SX
bool        isSerialPortEnabled;        // Vero se almeno una porta seriale è abilitata
int         nPresentSerialPorts;        // Numero di porte Seriali utilizzabili a bordo
TP_Config   panelConfig;                // Configurazione corrente del Target letta da Form mectSettings


// Cross Table Records
QList<CrossTableRecord> lstCTRecords;   // Lista completa di record per tabella (condivisa tra vari Oggetti di CTE)
QList<QStringList > lstMPNC006_Vars;    // Lista delle Variabili MPNC006
QList<QStringList > lstTPLC050_Vars;    // Lista delle Variabili TPLC050
QList<QStringList > lstMPNE_Vars;       // Lista delle Variabili MPNE

// Colori per sfondi grid
QColor      colorRetentive[2];
QColor      colorNonRetentive[2];
QColor      colorSystem[2];
QColor      colorGray;
QString     szColorRet[2];
QString     szColorNonRet[2];
QString     szColorSystem[2];



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
        << QString::fromAscii("None")
        << QString::fromAscii("High")
        << QString::fromAscii("Medium")
        << QString::fromAscii("Low")
        ;
    // Lista Significati (da mantenere allineata con enum behaviors in parser.h)
    lstBehavior.clear();
    lstBehavior
            << QString::fromAscii("Read/Only  (%I)")
            << QString::fromAscii("Read/Write (%Q)")
            << QString::fromAscii("Alarm")
            << QString::fromAscii("Event")
        ;
    // Lista condizioni di Allarme / Eventi
    lstCondition.clear();
    for (nCol = 0; nCol < oper_totals; nCol++)  {
        lstCondition.append(QString::fromAscii(logic_operators[nCol]));
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

    lstHeadCols[colPriority] = QString::fromAscii("Priority");
    lstHeadNames[colPriority] = QString::fromAscii("Priority");
    lstHeadSizes[colPriority] = 8;
    lstHeadCols[colUpdate] = QString::fromAscii("Update");
    lstHeadNames[colUpdate] = QString::fromAscii("Update");
    lstHeadSizes[colUpdate] = 8;
    lstHeadCols[colGroup] = QString::fromAscii("Group");
    lstHeadNames[colGroup] = QString::fromAscii("Group");
    lstHeadSizes[colGroup] = 8;
    lstHeadCols[colModule] = QString::fromAscii("Module");
    lstHeadNames[colModule] = QString::fromAscii("Module");
    lstHeadSizes[colModule] = 8;
    lstHeadCols[colName] = QString::fromAscii("Var.Name");
    lstHeadNames[colName] = QString::fromAscii("Var_Name");
    lstHeadSizes[colName] = 20;
    lstHeadCols[colType] = QString::fromAscii("Type");
    lstHeadNames[colType] = QString::fromAscii("Type");
    lstHeadSizes[colType] = 10;
    lstHeadCols[colDecimal] = QString::fromAscii("Decimal");
    lstHeadNames[colDecimal] = QString::fromAscii("Decimal");
    lstHeadSizes[colDecimal] = 8;
    lstHeadCols[colProtocol] = QString::fromAscii("Protocol");
    lstHeadNames[colProtocol] = QString::fromAscii("Protocol");
    lstHeadSizes[colProtocol] = 10;
    lstHeadCols[colIP] = QString::fromAscii("IP Address");
    lstHeadNames[colIP] = QString::fromAscii("IP_Address");
    lstHeadSizes[colIP] = 18;
    lstHeadCols[colPort] = QString::fromAscii("Port");
    lstHeadNames[colPort] = QString::fromAscii("Port");
    lstHeadSizes[colPort] = 8;
    lstHeadCols[colNodeID] = QString::fromAscii("Node ID");
    lstHeadNames[colNodeID] = QString::fromAscii("Node_ID");
    lstHeadSizes[colNodeID] = 8;
    lstHeadCols[colInputReg] = QString::fromAscii("I/R");
    lstHeadNames[colInputReg] = QString::fromAscii("I_R");
    lstHeadSizes[colInputReg] = 8;
    lstHeadCols[colRegister] = QString::fromAscii("Register");
    lstHeadNames[colRegister] = QString::fromAscii("Register");
    lstHeadSizes[colRegister] = 8;
    lstHeadCols[colBlock] = QString::fromAscii("Block");
    lstHeadNames[colBlock] = QString::fromAscii("Block");
    lstHeadSizes[colBlock] = 8;
    lstHeadCols[colBlockSize] = QString::fromAscii("Blk Size");
    lstHeadNames[colBlockSize] = QString::fromAscii("Blk_Size");
    lstHeadSizes[colBlockSize] = 8;
    lstHeadCols[colComment] = QString::fromAscii("Comment");
    lstHeadNames[colComment] = QString::fromAscii("Comment");
    lstHeadSizes[colComment] = 20;
    lstHeadCols[colBehavior] = QString::fromAscii("Behavior");
    lstHeadNames[colBehavior] = QString::fromAscii("Behavior");
    lstHeadSizes[colBehavior] = 16;
    lstHeadCols[colSourceVar] = QString::fromAscii("Source");
    lstHeadNames[colSourceVar] = QString::fromAscii("Source");
    lstHeadSizes[colSourceVar] = 20;
    lstHeadCols[colCondition] = QString::fromAscii("Condition");
    lstHeadNames[colCondition] = QString::fromAscii("Condition");
    lstHeadSizes[colCondition] = 10;
    lstHeadCols[colCompare] = QString::fromAscii("Compare");
    lstHeadNames[colCompare] = QString::fromAscii("Compare");
    lstHeadSizes[colCompare] = 20;
    // Colonne allineate a SX nel Grid
    lstHeadLeftCols.clear();
    lstHeadLeftCols.append(colName);
    lstHeadLeftCols.append(colComment);
    lstHeadLeftCols.append(colSourceVar);
    lstHeadLeftCols.append(colCompare);
    // Prompt della Griglia MPNx
    lstMNPxHeadSizes.clear();
    lstMPNxCols.clear();
    for (nCol = 0; nCol < colMPNxTotals; nCol++)  {
        lstMNPxHeadSizes.append(10);
        lstMPNxCols.append(szEMPTY);
    }
    lstMPNxCols[colMPNxRowNum] = QString::fromAscii("Row");
    lstMNPxHeadSizes[colMPNxRowNum] = 8;
    lstMPNxCols[colMPNxPriority] = lstHeadCols[colPriority];
    lstMNPxHeadSizes[colMPNxPriority] = lstHeadSizes[colPriority];
    lstMPNxCols[colMPNxUpdate] = lstHeadCols[colUpdate];
    lstMNPxHeadSizes[colMPNxUpdate] = lstHeadSizes[colUpdate];
    lstMPNxCols[colMPNxGroup] =  lstHeadCols[colGroup];
    lstMNPxHeadSizes[colMPNxGroup] = lstHeadSizes[colGroup];
    lstMPNxCols[colMPNxModule] =  lstHeadCols[colModule];
    lstMNPxHeadSizes[colMPNxModule] = lstHeadSizes[colModule];
    lstMPNxCols[colMPNxName] = lstHeadCols[colName];
    lstMNPxHeadSizes[colMPNxName] = lstHeadSizes[colName];
    lstMPNxCols[colMPNxType] = lstHeadCols[colType];
    lstMNPxHeadSizes[colMPNxType] = lstHeadSizes[colType];
    lstMPNxCols[colMPNxDecimal] = lstHeadCols[colDecimal];
    lstMNPxHeadSizes[colMPNxDecimal] = lstHeadSizes[colDecimal];
    lstMPNxCols[colMPNxNodeID] = lstHeadCols[colNodeID];
    lstMNPxHeadSizes[colMPNxNodeID] = lstHeadSizes[colNodeID];
    lstMPNxCols[colMPNxRegister] = lstHeadCols[colRegister];
    lstMNPxHeadSizes[colMPNxRegister] = lstHeadSizes[colRegister];
    lstMPNxCols[colMPNxBlock] = lstHeadCols[colBlock];
    lstMNPxHeadSizes[colMPNxBlock] = lstHeadSizes[colBlock];
    lstMPNxCols[colMPNxBlockSize] = lstHeadCols[colBlockSize];
    lstMNPxHeadSizes[colMPNxBlockSize] = lstHeadSizes[colBlockSize];
    lstMPNxCols[colMPNxBehavior] = lstHeadCols[colBehavior];
    lstMNPxHeadSizes[colMPNxBehavior] = lstHeadSizes[colBehavior];
    lstMPNxCols[colMPNxComment] = lstHeadCols[colComment];
    lstMNPxHeadSizes[colMPNxComment] = lstHeadSizes[colComment];
    // Colonne allineate a SX nel Grid MPNx
    lstMPNxHeadLeftCols.clear();
    lstMPNxHeadLeftCols.append(colMPNxName);
    lstMPNxHeadLeftCols.append(colMPNxComment);
    // Regioni CT
    lstRegions.clear();
    for (nCol = 0; nCol < regTotals; nCol++)  {
        lstRegions.append(szEMPTY);
    }
    lstRegions[regRetentive]    = QString::fromAscii("Retentive\t[1 -  192]");
    lstRegions[regNonRetentive] = QString::fromAscii("Non Retentive\t[193 - 4999]");
    lstRegions[regDiagnostic]   = QString::fromAscii("Diagnostic\t[5000 - 5299]");
    lstRegions[regLocalIO]      = QString::fromAscii("Local I/O\t[5300 - 5389]");
    lstRegions[regSystem]       = QString::fromAscii("System\t[5390 - 5472]");
    // Descrizione Tipi Variabili
    lstTipi.clear();
    for (nCol = 0; nCol < TYPE_TOTALS - 1; nCol++)  {
        lstTipi.append(QString::fromAscii(varTypeNameExtended[nCol]));
    }
    // Lista Update
    lstUpdateNames.clear();
    for (nCol = Htype; nCol < UPDATE_TOTALS; nCol++)  {
        lstUpdateNames.append(QString::fromAscii(updateTypeName[nCol]));
    }
    // Lista Protocolli
    lstProtocol.clear();
    for (nCol = PLC; nCol < FIELDBUS_TOTAL; nCol++)  {
        lstProtocol.append(QString::fromAscii(fieldbusName[nCol]));
    }
    // Costanti per i colori di sfondo
    colorRetentive[0] = QColor(170,255,255,255);           // Azzurro Dark
    colorRetentive[1] = QColor(210,255,255,255);           // Azzurro
    colorNonRetentive[0] = QColor(255,255,190,255);        // Giallino Dark
    colorNonRetentive[1] = QColor(255,255,220,255);        // Giallino
    colorSystem[0] = QColor(255,227,215,255);              // Rosa Dark
    colorSystem[1] = QColor(255,240,233,255);              // Rosa
    colorGray = QColor(200,200,200,255);                   // Gray
    szColorRet[0] = QString::fromAscii("color: #AAFFFF");
    szColorRet[1] = QString::fromAscii("color: #D2FFFF");
    szColorNonRet[0] = QString::fromAscii("color: #FFFFBE");
    szColorNonRet[1] = QString::fromAscii("color: #FFFFDC");
    szColorSystem[0] = QString::fromAscii("color: #FFE3D7");
    szColorSystem[1] = QString::fromAscii("color: #FFF0E9");

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
    // Impostazione del colore di sfondo
    QBrush bCell(cSfondo, Qt::SolidPattern);
    setRowBackground(bCell, table->model(), nRow);
}
bool recCT2MPNxList(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow, QList<QStringList> &lstModel, int nModelRow)
// Conversione da CT Record a Lista Stringhe per Interfaccia (REC -> Grid)
// Da Record C a QStringList di valori per caricamento griglia
// Versione per MPNX Nodes
{
    QString     szTemp;

    if (nRow < 0 || nRow >= CTRecords.count())  {
        return false;
    }
    listClear(lstRecValues, colMPNxTotals);
    // Row Number
    szTemp = QString::number(nRow + 1);
    lstRecValues[colMPNxRowNum] = szTemp;
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
        lstRecValues[colMPNxGroup] = lstModel[nModelRow][colGroup];
        // Campo Module
        lstRecValues[colMPNxModule] = lstModel[nModelRow][colModule];
        // Campo Name
        lstRecValues[colMPNxName] = QString::fromAscii(CTRecords[nRow].Tag);
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
        lstRecValues[colMPNxComment] = QString::fromAscii(CTRecords[nRow].Comment).trimmed().left(MAX_COMMENT_LEN - 1);
    }
    //    qDebug() << QString::fromAscii("recCT2List() - Parsed Row: %1") .arg(nRow);
    // Return value
    return true;


}

bool recCT2List(QList<CrossTableRecord> &CTRecords, QStringList &lstRecValues, int nRow)
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
        lstRecValues[colName] = QString::fromAscii(CTRecords[nRow].Tag);
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
            szTemp = QString::fromAscii(ip);
        }
        else
            szTemp = szEMPTY;
        lstRecValues[colIP] = szTemp;
        // Port
        lstRecValues[colPort] = QString::number(CTRecords[nRow].Port);
        // Node Id
        lstRecValues[colNodeID] = QString::number(CTRecords[nRow].NodeId);
        // Input Register
        lstRecValues[colInputReg] = (CTRecords[nRow].InputReg > 0) ? szTRUE : szFALSE;
        // Offeset Register
        lstRecValues[colRegister] = QString::number(CTRecords[nRow].Offset);
        // Block
        lstRecValues[colBlock] = QString::number(CTRecords[nRow].Block);
        // N.Registro
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
            lstRecValues[colSourceVar] = QString::fromAscii(CTRecords[nRow].ALSource);
            // Compare Var or Value
            szTemp = QString::fromAscii(CTRecords[nRow].ALCompareVar);
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
    }
    //    qDebug() << QString::fromAscii("recCT2List() - Parsed Row: %1") .arg(nRow);
    // Return value
    return true;
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
    int         nCor = 0;

    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(nMode);
    table->setHorizontalHeaderLabels(lstHeadCols);
    // Larghezza fissa per alcune colonne
    QFontMetrics fm(table->font());
    QString szTemp;
    for (nCor = 0; nCor < lstHeadCols.count(); nCor++)  {
        szTemp.fill(chX, lstHeadSizes[nCor]);
        nColWidth = fm.width(szTemp) * 1.2;
        nColHeight = fm.height();
        table->setColumnWidth(nCor, nColWidth);
    }
    // Altezza Righe
    nColHeight = fm.height() * 2;
    QHeaderView *verticalHeader = table->verticalHeader();
    verticalHeader->setResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(nColHeight);
    qDebug() << QString::fromAscii("setGridParams():    Col Height: %1") .arg(nColHeight);

}
