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
#include <QDebug>



QStringList lstPriority;                // Elenco dei valori di Priority
QStringList lstPriorityDesc;            // Descrizioni Priority
QStringList lstBehavior;                // Significato variabili
QStringList lstCondition;               // Operatori Logici per Allarmi
QStringList lstHeadCols;
QStringList lstHeadNames;
QList<int>  lstHeadSizes;
QStringList lstRegions;                 // Aree della CT
QStringList lstTipi;                    // Descrizione dei Tipi
QStringList lstUpdateNames;             // Descrizione delle Priorità
QStringList lstProtocol;                // Descrizione dei Protocolli

// Cross Table Records
QList<CrossTableRecord> lstCTRecords;   // Lista completa di record per tabella (condivisa tra vari Oggetti di CTE)


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
}
void setRowColor(QTableWidget *table, int nRow, int nAlternate, int nUsed, int nPriority)
// Imposta il colore di sfondo di una riga
{
    QColor      cSfondo = colorRetentive[0];

    // Impostazione del Backgound color in funzione della zona
    if (nRow >= 0 && nRow < MAX_RETENTIVE)  {
        cSfondo = colorRetentive[nAlternate];
        // qDebug() << tr("Row: %1 Alt: %2 - Retentive Row") .arg(nRow) .arg(nAlternate);
    }
    else if (nRow >= MIN_NONRETENTIVE - 1 && nRow <= MAX_NONRETENTIVE -1) {
        cSfondo = colorNonRetentive[nAlternate];
        // qDebug() << tr("Row: %1 Alt: %2 - NON Retentive Row") .arg(nRow) .arg(nAlternate);
    }
    else if (nRow >= MIN_DIAG - 1)  {
        // qDebug() << tr("Row: %1 Alt: %2 - SYSTEM Row") .arg(nRow) .arg(nAlternate);
        if (nRow <= MAX_DIAG - 1)  {
            cSfondo = colorSystem[0];
        }
        else if (nRow >= MIN_NODE - 1 && nRow <= MAX_NODE - 1)  {
            cSfondo = colorSystem[1];
        }
        else if (nRow >= MIN_LOCALIO - 1  && nRow <= MAX_LOCALIO - 1)  {
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
        // Campo Update
        if (CTRecords[nRow].Update >= 0 && CTRecords[nRow].Update < lstUpdateNames.count())
            lstRecValues[colUpdate] = lstUpdateNames[CTRecords[nRow].Update];
        // Campo Name
        lstRecValues[colName] = QString::fromAscii(CTRecords[nRow].Tag);
        // Campo Type
        if (CTRecords[nRow].VarType >= BIT && CTRecords[nRow].VarType < TYPE_TOTALS)
            lstRecValues[colType] = lstTipi[CTRecords[nRow].VarType];
        // Campo Decimal
        lstRecValues[colDecimal] = QString::number(CTRecords[nRow].Decimal);
        // Protocol
        if (CTRecords[nRow].Protocol >= 0 && CTRecords[nRow].Protocol < lstProtocol.count())
            lstRecValues[colProtocol] = lstProtocol[CTRecords[nRow].Protocol];
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
        // Allarme o Evento
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
    qDebug() << QString::fromAscii("recCT2List() - Parsed Row: %1") .arg(nRow);
    // Return value
    return true;
}
bool    list2GridRow(QTableWidget *table,  QStringList &lstRecValues, int nRow)
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
            fAdd = true;
            tItem = new QTableWidgetItem(szTemp);
        }
        else  {
            fAdd = false;
            tItem->setText(szTemp);
        }
        // Allineamento Celle
        if (nCol == colName || nCol == colComment || nCol == colSourceVar || nCol == colCompare)
            // Item Allineato a Sx
            tItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        else
            // Item Centrato in Cella
            tItem->setTextAlignment(Qt::AlignCenter);
        // Rende il valore non Editabile
        tItem->setFlags(tItem->flags() ^ Qt::ItemIsEditable);
        // Flag Marcatore della riga
        // Aggiunta al Grid
        if (fAdd)  {
            qDebug() << QString::fromAscii("list2GridRow(): Added Cell @Row:<%1>-Col <%2> Value:[%3]))") .arg(nRow) .arg(nCol) .arg(szTemp);
        }
        else  {
            qDebug() << QString::fromAscii("list2GridRow(): Modified CEll @Row:<%1>-Col <%2> Value:[%3])") .arg(nRow) .arg(nCol) .arg(szTemp);
        }
        table->setItem(nRow, nCol, tItem);
    }
    return true;
}
