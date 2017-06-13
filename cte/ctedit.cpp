#include "ctedit.h"
#include "ui_ctedit.h"
#include "utils.h"
#include "cteerrorlist.h"
#include "stdlib.h"
#include "ctecommon.h"

#include <QFile>
#include <QFileDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QModelIndexList>
#include <QString>
#include <QRect>
#include <QMessageBox>
#include <QRegExp>
#include <QRegExpValidator>
#include <QValidator>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QDebug>
#include <QAction>
#include <QLocale>
#include <QDate>
#include <QTime>
#include <QMenu>
#include <QAction>
#include <QVariant>
#include <QInputDialog>
#include <QCoreApplication>
#include <QProcess>
#include <QTextStream>
#include <QProcessEnvironment>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QEvent>
#include <QKeyEvent>
#include <QKeySequence>
#include <QIcon>
#include <QMetaObject>
#include <QFont>
#include <QFontMetrics>
#include <QTextStream>
#include <QHostAddress>
#include <QClipboard>
#include <QMimeData>
#include <QSortFilterProxyModel>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>

/* ----  Local Defines:   ----------------------------------------------------- */
#define _TRUE  1
#define _FALSE 0

#define STD_DISPLAY_TIME 5
#define MAX_DISPLAY_TIME 15

#define MAXBLOCKSIZE 64
#define MIN_RETENTIVE 1
#define MAX_RETENTIVE 192
#define MIN_NONRETENTIVE 193
#define MAX_NONRETENTIVE 4999
#define MIN_DIAG 5000
#define MAX_DIAG 5171
#define MIN_NODE 5172
#define MAX_NODE 5299
#define MIN_SYSTEM  5390
#define COMMANDLINE 2048

// Tabs in TabWidget
#define TAB_CT 0
#define TAB_SYSTEM 1
#define TAB_TREND 2

#undef  WORD_BIT


const QString szDEF_IP_PORT = QString::fromAscii("502");
const QString szCT_FILE_NAME = QString::fromAscii("Crosstable.csv");
const QString szEMPTY_IP = QString::fromAscii("0.0.0.0");
const QString szCrossCompier = QString::fromAscii("ctc");
const QString szTemplateFile = QString::fromAscii("template.pri");
const QString szEnvFile = QString::fromAscii("EnvVars.txt");
const QString szPLCEnvVar = QString::fromAscii("PLCUNIXINSTPATH");
const QString szProExt = QString::fromAscii(".pro");
const QString szPLCFILE = QString::fromAscii("plc");
// const QString szPLCExt = QString::fromAscii(".txt");
const QString szPLCExt = QString::fromAscii(".4cp");
const QString szPLCDir = QString::fromAscii("plc");
const QString szINIFILE = QString::fromAscii("system.ini");
const QString szFileQSS = QString::fromAscii("C:/Qt485/desktop/lib/qtcreator/plugins/QtProject/CTE.qss");
// Constanti per gestione XML di transito tra variabili
const QString szXMLCTENAME = QString::fromAscii("Mect_CTE");
const QString szXMLCTEVERSION = QString::fromAscii("Version");
const QString szXMLMODELTAG = QString::fromAscii("Model");
const QString szXMLCTNUMROWS = QString::fromAscii("CT_Rows");
const QString szXMLCTROW = QString::fromAscii("Crosstable_Row");
const QString szXMLCTDESTROW = QString::fromAscii("DestRow");
const QString szXMLCTSOURCEROW = QString::fromAscii("SourceRow");
const QString szXMLExt = QString::fromAscii(".xml");

// Version Number
#ifndef ATCM_VERSION
#define ATCM_VERSION "DevelopmentVersion"
#endif
#define _STR(x) #x
#define STR(x) _STR(x)
const QString szVERSION = QString::fromAscii(STR(ATCM_VERSION));

enum colonne_e
{
    colPriority = 0,
    colUpdate,
    colName,
    colType,
    colDecimal,
    colProtocol,
    colIP,
    colPort,
    colNodeID,
    colRegister,
    colBlock,
    colBlockSize,
    colComment,
    colBehavior,
    colSourceVar,
    colCondition,
    colCompare,
    colTotals
};

enum regions_e
{
    regRetentive = 0,
    regNonRetentive,
    regDiagnostic,
    regLocalIO,
    regSystem,
    regTotals
};


ctedit::ctedit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ctedit)
{
    int     nCol = 0;
    int     nValMin = 0;
    int     nValMax = 9999;
    QString szToolTip;
    bool    oldState = false;

    ui->setupUi(this);
    // Version Number
    ui->lblModel->setToolTip(szVERSION);
    // Liste di servizio
    lstUsedVarNames.clear();
    lstLoggedVars.clear();
    lstUndo.clear();
    lstCTErrors.clear();
    //------------------------
    // Riempimento liste
    //------------------------
    // Lista Modelli
    lstTargets.clear();
    initTargetList();
    TargetConfig = lstTargets[AnyTPAC];
    // Lista Messaggi di Errore
    lstErrorMessages.clear();
    for (nCol = 0; nCol < errCTTotals; nCol++)  {
        lstErrorMessages.append(szEMPTY);
    }
    lstErrorMessages[errCTNoError] = trUtf8("No Error");
    lstErrorMessages[errCTDuplicateName] = trUtf8("Duplicate Variable Name");
    lstErrorMessages[errCTNoPriority] = trUtf8("No Priority Selected");
    lstErrorMessages[errCTNoUpdate] = trUtf8("No PLC Refresh Selected");
    lstErrorMessages[errCTNoName] = trUtf8("Invalid or Empty Variable Name");
    lstErrorMessages[errCTNoType] = trUtf8("No Type Selected");
    lstErrorMessages[errCTNoDecimals] = trUtf8("Invalid Decimals");
    lstErrorMessages[errCTNoDecimalZero] = trUtf8("Decimals Must be 0 for BIT Type");
    lstErrorMessages[errCTNoVarDecimals] = trUtf8("Empty or Invalid Decimal Variable");
    lstErrorMessages[errCTWrongDecimals] = trUtf8("Invalid Bit Position");
    lstErrorMessages[errCTNoProtocol] = trUtf8("No Protocol Selected");
    lstErrorMessages[errCTNoBITAllowed] = trUtf8("BIT Type not allowed for SRV Protocols");
    lstErrorMessages[errCTNoIP] = trUtf8("No IP Address");
    lstErrorMessages[errCTBadIP] = trUtf8("Invalid IP Address");
    lstErrorMessages[errCTNoPort] = trUtf8("Empty or Invalid Port Value");
    lstErrorMessages[errCTNoDevicePort] = trUtf8("Port Not Present or Not Enabled on Device");
    lstErrorMessages[errCTWrongTCPPort] = trUtf8("TCP Port Not Allowed");
    lstErrorMessages[errCTNoNode] = trUtf8("Empty or Invalid Node Address");
    lstErrorMessages[errCTNoRegister] = trUtf8("Empty or Invalid Register Value");
    lstErrorMessages[errCTNoBehavior] = trUtf8("Empty or Invalid Behavior");
    lstErrorMessages[errCTNoBit] = trUtf8("Alarm/Event Variables must be of BIT type");
    lstErrorMessages[errCTBadPriorityUpdate] = trUtf8("Wrong Priority or Update for Alarm/Event Variable");
    lstErrorMessages[errCTNoVar1] = trUtf8("Empty or Invalid Left Condition Variable");
    lstErrorMessages[errCTNoCondition] = trUtf8("Empty or Invalid Alarm/Event Condition");
    lstErrorMessages[errCTRiseFallNotBit] = trUtf8("Rising/Falling Variable must be of BIT Type");
    lstErrorMessages[errCTInvalidNum] = trUtf8("Invalid Numeric Value in Alarm/Event Condition");
    lstErrorMessages[errCTEmptyCondExpression] = trUtf8("Empty or Invalid Expression in Alarm/Event Condition");
    lstErrorMessages[errCTNoVar2] = trUtf8("Empty or Invalid Right Condition Variable");
    lstErrorMessages[errCTIncompatibleVar] = trUtf8("Incompatible Var in Alarm/Event Condition");
    // Titoli colonne
    lstHeadCols.clear();
    lstHeadSizes.clear();
    lstHeadNames.clear();
    for (nCol = 0; nCol < colTotals; nCol++)  {
        lstHeadCols.append(szEMPTY);
        lstHeadNames.append(szEMPTY);
        lstHeadSizes.append(10);
    }

    lstHeadCols[colPriority] = trUtf8("Priority");
    lstHeadNames[colPriority] = trUtf8("Priority");
    lstHeadSizes[colPriority] = 8;
    lstHeadCols[colUpdate] = trUtf8("Update");
    lstHeadNames[colUpdate] = trUtf8("Update");
    lstHeadSizes[colUpdate] = 8;
    lstHeadCols[colName] = trUtf8("Var.Name");
    lstHeadNames[colName] = trUtf8("Var_Name");
    lstHeadSizes[colName] = 20;
    lstHeadCols[colType] = trUtf8("Type");
    lstHeadNames[colType] = trUtf8("Type");
    lstHeadSizes[colType] = 10;
    lstHeadCols[colDecimal] = trUtf8("Decimal");
    lstHeadNames[colDecimal] = trUtf8("Decimal");
    lstHeadSizes[colDecimal] = 8;
    lstHeadCols[colProtocol] = trUtf8("Protocol");
    lstHeadNames[colProtocol] = trUtf8("Protocol");
    lstHeadSizes[colProtocol] = 10;
    lstHeadCols[colIP] = trUtf8("IP Address");
    lstHeadNames[colIP] = trUtf8("IP_Address");
    lstHeadSizes[colIP] = 18;
    lstHeadCols[colPort] = trUtf8("Port");
    lstHeadNames[colPort] = trUtf8("Port");
    lstHeadSizes[colPort] = 8;
    lstHeadCols[colNodeID] = trUtf8("Node ID");
    lstHeadNames[colNodeID] = trUtf8("Node_ID");
    lstHeadSizes[colNodeID] = 8;
    lstHeadCols[colRegister] = trUtf8("Register");
    lstHeadNames[colRegister] = trUtf8("Register");
    lstHeadSizes[colRegister] = 8;
    lstHeadCols[colBlock] = trUtf8("Block");
    lstHeadNames[colBlock] = trUtf8("Block");
    lstHeadSizes[colBlock] = 8;
    lstHeadCols[colBlockSize] = trUtf8("Blk Size");
    lstHeadNames[colBlockSize] = trUtf8("Blk_Size");
    lstHeadSizes[colBlockSize] = 8;
    lstHeadCols[colComment] = trUtf8("Comment");
    lstHeadNames[colComment] = trUtf8("Comment");
    lstHeadSizes[colComment] = 20;
    lstHeadCols[colBehavior] = trUtf8("Behavior");
    lstHeadNames[colBehavior] = trUtf8("Behavior");
    lstHeadSizes[colBehavior] = 16;
    lstHeadCols[colSourceVar] = trUtf8("Source");
    lstHeadNames[colSourceVar] = trUtf8("Source");
    lstHeadSizes[colSourceVar] = 20;
    lstHeadCols[colCondition] = trUtf8("Condition");
    lstHeadNames[colCondition] = trUtf8("Condition");
    lstHeadSizes[colCondition] = 10;
    lstHeadCols[colCompare] = trUtf8("Compare");
    lstHeadNames[colCompare] = trUtf8("Compare");
    lstHeadSizes[colCompare] = 20;
    // Regioni CT
    lstRegions.clear();
    for (nCol = 0; nCol < regTotals; nCol++)  {
        lstRegions.append(szEMPTY);
    }
    lstRegions[regRetentive]    = trUtf8("Retentive\t[1 -  192]");
    lstRegions[regNonRetentive] = trUtf8("Non Retentive\t[193 - 4999]");
    lstRegions[regDiagnostic]   = trUtf8("Diagnostic\t[5000 - 5299]");
    lstRegions[regLocalIO]      = trUtf8("Local I/O\t[5300 - 5389]");
    lstRegions[regSystem]       = trUtf8("System\t[5390 - 5472]");
    // Lista Priorità
    lstPriority.clear();
    lstPriority
            << QString::fromAscii("0")
            << QString::fromAscii("1")
            << QString::fromAscii("2")
            << QString::fromAscii("3")
        ;
    // Lista PLC (Frequenza Aggiornamento)
    lstUpdateNames.clear();
    lstAllUpdates.clear();
    lstNoHUpdates.clear();
    for (nCol = Htype; nCol <= Xtype; nCol++)  {
        lstUpdateNames.append(QString::fromAscii(updateTypeName[nCol]));
        lstAllUpdates.append(nCol);
        // Esclude dalla lista solo le variabili H
        if (nCol != Htype)  {
            lstNoHUpdates.append(nCol);            
        }
        // Esclude dalla lista le H e P
        if (nCol != Htype && nCol != Ptype)  {
            lstLogUpdates.append(nCol);
        }
    }
    // Lista TIPI Variabili (Esclude tipo UNKNOWN con TYPE_TOTALS - 1)
    lstTipi.clear();
    lstAllVarTypes.clear();
    for (nCol = 0; nCol < TYPE_TOTALS - 1; nCol++)  {
        lstTipi.append(QString::fromAscii(varTypeNameExtended[nCol]));
        lstAllVarTypes.append(nCol);
    }
    // Lista Protocolli (tipi di Bus)
    lstProtocol.clear();
    lstBusEnabler.clear();
    for (nCol = PLC; nCol <= TCPRTU_SRV; nCol++)  {
        lstProtocol.append(QString::fromAscii(fieldbusName[nCol]));
        lstBusEnabler.append(true);         // Di default tutti i tipi di Bus sono abilitati
    }
    // Lista Prodotti
    lstProductNames.clear();
    for (nCol = AnyTPAC; nCol < MODEL_TOTALS; nCol++)  {
        lstProductNames.append(QString::fromAscii(product_name[nCol]));
    }
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
    // Caricamento delle varie Combos
    // Combo delle Porte Seriali
    ui->cboPort->clear();
    for (nCol = 0; nCol <= nMaxSerialPorts; nCol++)  {
        ui->cboPort->addItem(QString::number(nCol));
    }
    // Combo Sections (bloccata per evitare Side Effects al currentIndex)
    oldState = ui->cboSections->blockSignals(true);
    for (nCol = 0; nCol < regTotals; nCol++)  {
        ui->cboSections->addItem(lstRegions[nCol]);
    }
    ui->cboSections->setCurrentIndex(-1);
    ui->cboSections->blockSignals(oldState);
    // Combo Priority
    szToolTip.clear();
    szToolTip.append(tr("Enabling the exchange of data with remote devices:\n"));
    szToolTip.append(tr("0 - Disabled\n"));
    szToolTip.append(tr("1 - Enabled High Priority\n"));
    szToolTip.append(tr("2 - Enabled Medium Priority\n"));
    szToolTip.append(tr("3 - Enabled Low Priority"));
    for  (nCol=0; nCol<4; nCol++)   {
        ui->cboPriority->addItem(lstPriority[nCol], QString::number(nCol));
    }
    ui->cboPriority->setToolTip(szToolTip);
    // Combo Update
    szToolTip.clear();
    szToolTip.append(tr("PLC Refresh Rate:\n"));
    szToolTip.append(tr("H - Only in current page\n"));
    szToolTip.append(tr("P - Permanent\n"));
    szToolTip.append(tr("S - Permanent Slow Logging\n"));
    szToolTip.append(tr("F - Permanent Fast Logging\n"));
    szToolTip.append(tr("V - Permanent Logging if Changed\n"));
    szToolTip.append(tr("X - Permanent Logging on Shot"));
    for  (nCol=0; nCol<lstUpdateNames.count(); nCol++)   {
        ui->cboUpdate->addItem(lstUpdateNames[nCol], lstUpdateNames[nCol]);
    }
    ui->cboUpdate->setToolTip(szToolTip);
    // ToolTip nome variabile
    szToolTip.clear();
    szToolTip.append(tr("Name of the variable to be used in HMI:\n"));
    szToolTip.append(tr("Must follow C++ variable naming conventions"));
    ui->txtName->setToolTip(szToolTip);
    // Combo TipoCrossTable
    szToolTip.clear();
    szToolTip.append(tr("Variable Data Type"));
    for  (nCol=0; nCol<lstTipi.count(); nCol++)   {
        ui->cboType->addItem(lstTipi[nCol], lstTipi[nCol]);
    }
    ui->cboType->setToolTip(szToolTip);
    // Decimals
    szToolTip.clear();
    szToolTip.append(tr("Number of decimal places to display data if from 0 to 4,\n"));
    szToolTip.append(tr("otherwise is the position of the variable that contains the number of decimal places\n"));
    szToolTip.append(tr("In the case of BIT variables, it is the position of the bit as low as 1"));
    ui->txtDecimal->setToolTip(szToolTip);
    // Combo Tipo Bus
    szToolTip.clear();
    szToolTip.append(tr("Protocol with the remote device"));
    for  (nCol=0; nCol<lstProtocol.count(); nCol++)   {
        ui->cboProtocol->addItem(lstProtocol[nCol], lstProtocol[nCol]);
    }
    ui->cboProtocol->setToolTip(szToolTip);
    // Indirizzo IP
    szToolTip.clear();    // Lista Priorità
    szToolTip.append(tr("IP address for TCP, TCP_RTU, TCP_SRV, TCPRTU SRV like 192.168.0.42"));
    ui->txtIP->setToolTip(szToolTip);
    // Porta di comunicazione
    szToolTip.clear();
    szToolTip.append(tr("TCP Communication Port, e.g. 502"));
    ui->txtPort->setToolTip(szToolTip);
    // Combo Porta seriale
    szToolTip.clear();
    szToolTip.append(tr("Serial Communication Port, from 0 to %1") .arg(nMaxSerialPorts));
    ui->cboPort->setToolTip(szToolTip);
    // Indirizzo nodo remoto
    szToolTip.clear();
    szToolTip.append(tr("Remode Node Address, 0 for broadcast"));
    ui->txtNode->setToolTip(szToolTip);
    // Register
    szToolTip.clear();
    szToolTip.append(tr("Modbus address of the Register to be used\n"));
    szToolTip.append(tr("ADR=[0..29999] -> Coil / Holding Register (ADR)"));
    szToolTip.append(tr("ADR=[30000..39999] -> Input  Register (ADR-30001)\n"));
    szToolTip.append(tr("ADR=[40000..49999] -> Holding Register (ADR-40001)"));
    ui->txtRegister->setToolTip(szToolTip);
    // Block
    szToolTip.clear();
    szToolTip.append(tr("Address of the first register of the block"));
    ui->txtBlock->setToolTip(szToolTip);
    // Block size
    szToolTip.clear();
    szToolTip.append(tr("Number of registers in the block"));
    ui->txtBlockSize->setToolTip(szToolTip);
    // Combo Behavior
    szToolTip.clear();
    szToolTip.append(tr("Variable behavior: Reading, Reading and Writing, Alarm, Event"));
    for  (nCol=0; nCol<lstBehavior.count(); nCol++)   {
        ui->cboBehavior->addItem(lstBehavior[nCol], lstBehavior[nCol]);
    }
    ui->cboBehavior->setToolTip(szToolTip);
    // Combo Condition
    szToolTip.clear();
    szToolTip.append(trUtf8("Alarm Event Condition:\n"));
    szToolTip.append(trUtf8("GREATER\n"));
    szToolTip.append(trUtf8("GREATER OR EQL\n"));
    szToolTip.append(trUtf8("LESS\n"));
    szToolTip.append(trUtf8("LESS OR EQL\n"));
    szToolTip.append(trUtf8("EQUAL\n"));
    szToolTip.append(trUtf8("RISING EDGE\n"));
    szToolTip.append(trUtf8("FALLING EDGE"));
    // Trick per rendere la Combo Centrata
    ui->cboCondition->setEditable(true);
    // ui->cboCondition->lineEdit()->setReadOnly(true);
    ui->cboCondition->lineEdit()->setAlignment(Qt::AlignCenter);
    for  (nCol=0; nCol<lstCondition.count(); nCol++)   {
        ui->cboCondition->addItem(lstCondition[nCol], lstCondition[nCol]);
        ui->cboCondition->setItemData(nCol, Qt::AlignCenter, Qt::TextAlignmentRole);
    }
    ui->cboCondition->lineEdit()->setReadOnly(true);
    ui->cboCondition->setToolTip(szToolTip);
    // Init Valori
    m_szCurrentCTFile.clear();
    m_szCurrentCTPath.clear();
    m_szCurrentCTName.clear();
    m_szCurrentModel.clear();
    m_szCurrentProjectPath.clear();
    m_nGridRow = 0;
    lstCTRecords.clear();
    // Validator per Interi
    ui->txtDecimal->setValidator(new QIntValidator(nValMin, DimCrossTable, this));
    ui->txtPort->setValidator(new QIntValidator(nValMin, nMax_Int16, this));
    ui->txtNode->setValidator(new QIntValidator(nValMin, nMaxNodeID, this));
    ui->txtRegister->setValidator(new QIntValidator(nValMin, nMaxRegister, this));
    ui->txtNode->setValidator(new QIntValidator(nValMin, MAXBLOCKSIZE -1, this));
    ui->txtBlock->setValidator(new QIntValidator(nValMin, nValMax, this));
    ui->txtBlockSize->setValidator(new QIntValidator(nValMin, nValMax, this));
    // Validatori per Double
    ui->txtFixedValue->setValidator(new QDoubleValidator(this));
    // Validator per txtIp
    QString szExp = QString::fromAscii("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp regExprIP(szExp);
    ui->txtIP->setValidator(new QRegExpValidator(regExprIP, this));
    // Validator per Nome variabile e Comment
    QString szNameExp = QString::fromAscii("\\w+");
    QRegExp regExprName(szNameExp);
    ui->txtName->setValidator(new QRegExpValidator(regExprName, this));
    ui->txtName->setMaxLength(MAX_IDNAME_LEN - 1);
    // Validator per commenti
    QString szCommentExp = QString::fromAscii("^[^\\\\/:;,?\"'<>|]*$");
    QRegExp regExprComment(szCommentExp);
    ui->txtComment->setValidator(new QRegExpValidator(regExprComment, this));
    ui->txtComment->setMaxLength(MAX_COMMENT_LEN - 1);
    // Campi sempre locked
    ui->txtRow->setEnabled(false);
    ui->txtBlock->setEnabled(false);
    ui->txtBlockSize->setEnabled(false);
    // Model Name
    ui->lblModel->setText(szEMPTY);
    // Stringhe generiche per gestione dei formati di Data e ora
    m_szFormatDate = QString::fromAscii("yyyy.MM.dd");
    m_szFormatTime = QString::fromAscii("hh:mm:ss");
    // Costanti per i colori di sfondo
    colorRetentive[0] = QColor(170,255,255,255);           // Azzurro Dark
    colorRetentive[1] = QColor(210,255,255,255);           // Azzurro
    colorNonRetentive[0] = QColor(255,255,190,255);        // Giallino Dark
    colorNonRetentive[1] = QColor(255,255,220,255);        // Giallino
    colorSystem[0] = QColor(255,227,215,255);              // Rosa Dark
    colorSystem[1] = QColor(255,240,233,255);              // Rosa
    szColorRet[0] = QString::fromAscii("color: #AAFFFF");
    szColorRet[1] = QString::fromAscii("color: #D2FFFF");
    szColorNonRet[0] = QString::fromAscii("color: #FFFFBE");
    szColorNonRet[1] = QString::fromAscii("color: #FFFFDC");
    szColorSystem[0] = QString::fromAscii("color: #FFE3D7");
    szColorSystem[1] = QString::fromAscii("color: #FFF0E9");
    // Variabili di stato globale dell'editor
    m_isCtModified = false;
    m_isConfModified = false;
    m_isTrendModified = false;
    m_fShowAllRows = true;
    m_fCutOrPaste = false;
    m_nCurTab = 0;
    m_vtAlarmVarType = UNKNOWN;
    // Creazione del Tab per il System Editor
    QVBoxLayout *vbSystem = new QVBoxLayout(ui->tabSystem);
    mectSet = new MectSettings(ui->tabSystem);
    vbSystem->addWidget(mectSet);
    // Creazione del Tab per il Trend Editor
    QVBoxLayout *vbTrend = new QVBoxLayout(ui->tabTrend);
    trendEdit = new TrendEditor(ui->tabTrend);
    vbTrend->addWidget(trendEdit);

    // Seleziona il primo Tab
    ui->tabWidget->setTabEnabled(TAB_CT, true);
    ui->tabWidget->setTabEnabled(TAB_SYSTEM, true);
    ui->tabWidget->setTabEnabled(TAB_TREND, true);
    ui->tabWidget->setCurrentIndex(m_nCurTab);

    // Connessione Segnali - Slot
    ui->tblCT->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tblCT, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(displayUserMenu(const QPoint &)));
    // connect(ui->tblCT, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
    connect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
            SLOT(tableItemChanged(const QItemSelection &, const QItemSelection & ) ));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected(int)));
    // Timer per messaggi
    tmrMessage = new QTimer(this);
    tmrMessage->setInterval(0);
    tmrMessage->setSingleShot(true);
    connect(tmrMessage, SIGNAL(timeout()), this, SLOT(clearStatusMessage()));
    // Event Filter    
    ui->tblCT->installEventFilter(this);
    ui->fraEdit->installEventFilter(this);
    // Style Sheet
    QFile fileQSS(szFileQSS);
    if (fileQSS.exists())  {
        fileQSS.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(fileQSS.readAll());
        fileQSS.close();
        this->setStyleSheet(styleSheet);
    }
    // Spegne Block e BlockSize
    ui->lblBlock->setVisible(false);
    ui->txtBlock->setVisible(false);
    ui->lblBlockSize->setVisible(false);
    ui->txtBlockSize->setVisible(false);
    ui->cmdBlocchi->setVisible(false);
    ui->cboPort->setVisible(false);
    ui->txtPort->setVisible(true);
    ui->lblPort_RTU->setVisible(false);
    ui->lblPort->setVisible(true);

}

ctedit::~ctedit()
{
    delete ui;
}
void    ctedit::setProjectPath(QString szProjectPath)
{
    QDir projectDir(szProjectPath);
    QString     szProjectName;

    if (projectDir.exists() && ! szProjectPath.isEmpty())  {
        projectDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        QStringList filesList;
        filesList.append(QString::fromAscii("*.pro"));
        QStringList lstFile = projectDir.entryList(filesList);

        if (lstFile.count() > 0)  {
            szProjectName = lstFile.at(0);
        }
        // Costruzione del Path progetto
        m_szCurrentProjectPath = szProjectPath;
        // Nome Progetto senza Path
        m_szCurrentProjectName = szProjectName;
        // Costruzione del Path PLC
        m_szCurrentPLCPath = szProjectPath;
        m_szCurrentPLCPath.append(szSLASH);
        m_szCurrentPLCPath.append(szPLCDir);
        // m_szCurrentPLCPath.append(szSLASH);
        QDir dirPlc(m_szCurrentPLCPath);
        // Create if not exists PLC Dir
        if (!dirPlc.exists()) {
            dirPlc.mkpath(m_szCurrentPLCPath);
        }
    }
    else  {
        m_szCurrentProjectPath.clear();
        m_szCurrentProjectName.clear();
        m_szCurrentPLCPath.clear();
    }
    //qDebug() << "Project Path:" << m_szCurrentProjectPath;
    qDebug() << "Project Name:" << m_szCurrentProjectName;
    // qDebug() << "PLC Path:" << m_szCurrentPLCPath;
}

bool    ctedit::selectCTFile(QString szFileCT)
// Select a current CT File
{
    QString     szFile;
    bool        fRes = false;
    int         nModel = AnyTPAC;

    if (fileExists(szFileCT)) {
        szFile = szFileCT;
    }
    else {
        szFile = QFileDialog::getOpenFileName(this, tr("Open Crosstable File"), szFileCT, tr("Crosstable File (%1)") .arg(szCT_FILE_NAME));
    }
    // Tries to Open CT File
    if (! szFile.isEmpty())   {
        m_szCurrentCTFile = szFile;
        fRes = loadCTFile(m_szCurrentCTFile, lstCTRecords, true);
        if (fRes)
            qDebug() << tr("Loaded CT File: %1 Result: OK") .arg(szFile);
        else
            qDebug() << tr("Loaded CT File: %1 Result: ERROR") .arg(szFile);
    }
    else  {
        m_szCurrentCTFile.clear();
        fRes = false;
        qDebug() << tr("CT File: %1 Not Found or not Selected") .arg(szFile);
    }
    // Retrieving Path and Name of Cross Table file
    m_szCurrentModel.clear();
    m_szCurrentCTPath.clear();
    m_szCurrentCTName.clear();
    // Load Ok, init default values
    if (fRes)  {
        QFileInfo fInfo(m_szCurrentCTFile);
        m_szCurrentCTPath = fInfo.absolutePath();       
        m_szCurrentCTPath.append(szSLASH);
        m_szCurrentCTName = fInfo.baseName();
        // Reading Model from template.pri
        m_szCurrentModel = getModelName();
        ui->lblModel->setText(m_szCurrentModel);
        // Abilitazione dei protocolli in funzione del Modello
        if (! m_szCurrentModel.isEmpty())  {
            // nModel = lstProductNames.indexOf(m_szCurrentModel);
            qDebug() << tr("Model in List: %1") .arg(nModel);
            nModel = searchModelInList(m_szCurrentModel);
            qDebug() << tr("Model Code: <%1> Model No <%2>") .arg(m_szCurrentModel) .arg(nModel);
            ui->lblModel->setStyleSheet(QString::fromAscii("background-color: LightCyan"));
        }
        else  {
            TargetConfig = lstTargets[AnyTPAC];
            ui->lblModel->setStyleSheet(QString::fromAscii("background-color: Red"));
            m_szMsg = tr("No Model Type Found in file: %1") .arg(szTemplateFile);
            warnUser(this, szMectTitle, m_szMsg);
        }
        // Se il modello non è stato trovato in template.pri vale comunque AnyTPAC ma il salvataggio è disabilitato
        m_isCtModified = false;
        m_fCutOrPaste = false;
        // Se tutto Ok, carica anche le impostazioni del file INI
        mectSet->loadProjectFiles(m_szCurrentCTPath + szINIFILE, m_szCurrentProjectPath + szSLASH + m_szCurrentProjectName, m_szCurrentProjectPath + szSLASH, TargetConfig);
        // Rilegge all'indetro le info di configurazione eventualmente aggiornate da system.ini
        mectSet->getTargetConfig(TargetConfig);
        // Aggiorna le abilitazioni dei protocolli in funzione delle porte abilitate
        enableProtocolsFromModel();
        // Se tutto Ok, carica anche il primo trend utile
        QString szFileTemplate;
        szFileTemplate = m_szCurrentProjectPath;
        szFileTemplate.append(szSLASH);
        szFileTemplate.append(szTemplateFile);
        trendEdit->updateVarLists(lstLoggedVars);
        trendEdit->setTrendsParameters(m_szCurrentModel, m_szCurrentCTPath, szEMPTY, szFileTemplate);
        // Abilita interfaccia
        enableInterface();
    }
    return fRes;
}
bool    ctedit::loadCTFile(QString szFileCT, QList<CrossTableRecord> &lstCtRecs, bool fLoadGrid)
// Load the current CT File. If fShowGrid then load data to user Grid
{
    int nRes = 0;
    int nCur = 0;
    bool fRes = false;

    if (szFileCT.isEmpty())
        return false;
    // Clear Data
    lstCtRecs.clear();
    this->setCursor(Qt::WaitCursor);
    // Reset Show Flag && Current Row Index
    m_fShowAllRows = true;
    m_nGridRow = -1;
    // Opening File
    nRes = LoadXTable(szFileCT.toAscii().data(), &CrossTable[0]);
    // Return value is the result of Parsing C structure to C++ Objects.
    // Data in Array starts from element #1, in list from 0...
    if (nRes == 0)  {
        for (nCur = 1; nCur <= DimCrossTable; nCur++)  {
            lstCtRecs.append(CrossTable[nCur]);
        }
        if (fLoadGrid)  {
            fRes = ctable2Grid();
        }
        else
            fRes = true;
    }
    else  {
        m_szMsg = tr("Error Loading CrossTable file: %1") .arg(szFileCT);
        warnUser(this, szMectTitle, m_szMsg);
    }
    this->setCursor(Qt::ArrowCursor);
    return fRes;
}
bool    ctedit::list2GridRow(QStringList &lstRecValues, int nRow)
// Inserimento o modifica elemento in Grid (valori -> GRID)
{
    int                 nCol = 0;
    QString             szTemp;
    QTableWidgetItem    *tItem;
    bool                fAdd = false;

    // Insert Items at Row, Col
    for (nCol = 0; nCol < colTotals; nCol++)  {
        szTemp = lstRecValues[nCol];
        tItem = ui->tblCT->item(nRow, nCol);
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
        if (lstCTRecords[nRow].UsedEntry)
            ui->tblCT->showRow(nRow);
        else
            ui->tblCT->hideRow(nRow);
        // Aggiunta al Grid
        if (fAdd)  {
            ui->tblCT->setItem(nRow, nCol, tItem);
        }
    }
    return true;
}

bool    ctedit::ctable2Grid()
// Lettura di tutta la CT in Grid
{
    bool        fRes = true;
    int         nCur = 0;
    int         nColWidth = 0;
    QStringList lstFields;

    lstFields.clear();
    lstUsedVarNames.clear();
    lstLoggedVars.clear();
    // Preparazione tabella
    this->setCursor(Qt::WaitCursor);
    ui->tblCT->setEnabled(false);
    ui->tblCT->clearSelection();
    ui->tblCT->setRowCount(0);
    ui->tblCT->clear();
    ui->tblCT->setColumnCount(colTotals);
    // Caricamento elementi
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        // Covert CT Record 2 User Values
        fRes = recCT2List(lstFields, nCur);
        // If Ok add row to Table View
        if (fRes)  {
            ui->tblCT->insertRow(nCur);
            fRes = list2GridRow(lstFields, nCur);
        }
    }
    // qDebug() << tr("Loaded Rows: %1") .arg(nCur);
    // Impostazione parametri TableView
    ui->tblCT->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblCT->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tblCT->setHorizontalHeaderLabels(lstHeadCols);
    // Larghezza fissa per alcune colonne
    QFontMetrics fm(ui->tblCT->font());
    QString szTemp;
    for (nCur = 0; nCur < lstHeadCols.count(); nCur++)  {
        szTemp.fill(chX, lstHeadSizes[nCur]);
        nColWidth = fm.width(szTemp) * 1.2;
        ui->tblCT->setColumnWidth(nCur, nColWidth);
    }
    // ui->tblCT;
    //dWidth = fm.width(g_szLocalDateTimeFormat) * 1.10;
    //txtDate->setMaximumWidth(qRound(dWidth));
    //ui->tblCT->horizontalHeader()->setResizeMode(colPriority, QHeaderView::Stretch);
    //ui->tblCT->horizontalHeader()->setResizeMode(colUpdate, QHeaderView::Stretch);
    //ui->tblCT->horizontalHeader()->setResizeMode(colBehavior, QHeaderView::Stretch);
    ui->tblCT->setEnabled(true);
    // Show All Elements
    if (fRes)  {
        m_isCtModified = false;
        m_fCutOrPaste = false;
        ui->cmdHideShow->setChecked(m_fShowAllRows);
        // ui->cmdHideShow->setChecked(true);
        ui->cmdSave->setEnabled(true);
        // ui->cmdSave->setEnabled(m_isCtModified);
        showAllRows(m_fShowAllRows);
        // Aggiorna le liste delle variabili
        fillVarList(lstUsedVarNames, lstAllVarTypes, lstAllUpdates);
        fillVarList(lstLoggedVars, lstAllVarTypes, lstLogUpdates);

    }
    else  {
        qDebug() << tr("Error Loading Rows");
    }
    // Return value
    this->setCursor(Qt::ArrowCursor);
    return fRes;
}
bool ctedit::recCT2List(QStringList &lstRecValues, int nRow)
// Conversione da CT Record a record come Lista Stringhe per Interfaccia (Grid)
// Da Record C a QStringList di valori per caricamento griglia
{
    QString szTemp;
    char ip[MAX_IPADDR_LEN];

    if (nRow < 0 || nRow >= lstCTRecords.count())
        return false;
    // Pulizia Buffers
    szTemp.clear();
    listClear(lstRecValues);
    // Recupero informazioni da Record CT
    // Abilitazione riga
    if (lstCTRecords[nRow].UsedEntry)  {
        // Priority
        if (lstCTRecords[nRow].Enable >= 0 && lstCTRecords[nRow].Enable < lstPriority.count())
            lstRecValues[colPriority] = lstPriority[lstCTRecords[nRow].Enable];
        // Campo Update
        if (lstCTRecords[nRow].Update >= 0 && lstCTRecords[nRow].Update < lstUpdateNames.count())
            lstRecValues[colUpdate] = lstUpdateNames[lstCTRecords[nRow].Update];
        // Campo Name
        lstRecValues[colName] = QString::fromAscii(lstCTRecords[nRow].Tag);
        // Campo Type
        if (lstCTRecords[nRow].VarType >= BIT && lstCTRecords[nRow].VarType < TYPE_TOTALS)
            lstRecValues[colType] = lstTipi[lstCTRecords[nRow].VarType];
        // Campo Decimal
        lstRecValues[colDecimal] = QString::number(lstCTRecords[nRow].Decimal);
        // Protocol
        if (lstCTRecords[nRow].Protocol >= 0 && lstCTRecords[nRow].Protocol < lstProtocol.count())
            lstRecValues[colProtocol] = lstProtocol[lstCTRecords[nRow].Protocol];
        // IP Address (Significativo solo per Protocolli a base TCP)
        if (lstCTRecords[nRow].Protocol == TCP || lstCTRecords[nRow].Protocol == TCPRTU ||
                lstCTRecords[nRow].Protocol == TCP_SRV || lstCTRecords[nRow].Protocol ==TCPRTU_SRV)  {
            ipaddr2str(lstCTRecords[nRow].IPAddress, ip);
            szTemp = QString::fromAscii(ip);
        }
        else
            szTemp = szEMPTY;
        lstRecValues[colIP] = szTemp;
        // Port
        lstRecValues[colPort] = QString::number(lstCTRecords[nRow].Port);
        // Node Id
        lstRecValues[colNodeID] = QString::number(lstCTRecords[nRow].NodeId);
        // Register
        lstRecValues[colRegister] = QString::number(lstCTRecords[nRow].Offset);
        // Block
        lstRecValues[colBlock] = QString::number(lstCTRecords[nRow].Block);
        // N.Registro
        lstRecValues[colBlockSize] = QString::number(lstCTRecords[nRow].BlockSize);
        // PLC forza tutto a Blank
        if (lstCTRecords[nRow].Protocol == PLC)  {
            lstRecValues[colPort] = szEMPTY;
            lstRecValues[colNodeID] = szEMPTY;
            lstRecValues[colRegister] = szEMPTY;
        }
        // Commento
        lstRecValues[colComment] = QString::fromAscii(lstCTRecords[nRow].Comment).trimmed().left(MAX_COMMENT_LEN - 1);
        // Behavior
        // Allarme o Evento
        if (lstCTRecords[nRow].usedInAlarmsEvents && lstCTRecords[nRow].Behavior >= behavior_alarm)  {
            // Tipo Allarme-Evento
            if (lstCTRecords[nRow].ALType == Alarm)
                lstRecValues[colBehavior] = lstBehavior[behavior_alarm];
            else if (lstCTRecords[nRow].ALType == Event)
                lstRecValues[colBehavior] = lstBehavior[behavior_event];
            // Operatore Logico
            if (lstCTRecords[nRow].ALOperator >= 0 && lstCTRecords[nRow].ALOperator < oper_totals)
                lstRecValues[colCondition] = lstCondition[lstCTRecords[nRow].ALOperator];
            else
                lstRecValues[colCondition] = szEMPTY;
            // Source Var
            lstRecValues[colSourceVar] = QString::fromAscii(lstCTRecords[nRow].ALSource);
            // Compare Var or Value
            szTemp = QString::fromAscii(lstCTRecords[nRow].ALCompareVar);
            if (szTemp.isEmpty())
                lstRecValues[colCompare] = QString::number(lstCTRecords[nRow].ALCompareVal, 'f', 4);
            else
                lstRecValues[colCompare] = szTemp;
            // Rising o Falling senza seconda parte
            if (lstCTRecords[nRow].ALOperator == oper_rising || lstCTRecords[nRow].ALOperator == oper_falling)
                lstRecValues[colCompare] = szEMPTY;
        }
        else   {
            // R/O o R/W
            if (lstCTRecords[nRow].Behavior == behavior_readonly)
                lstRecValues[colBehavior] = lstBehavior[behavior_readonly];
            else if (lstCTRecords[nRow].Behavior == behavior_readwrite)
                lstRecValues[colBehavior] = lstBehavior[behavior_readwrite];
            // Source Var - Condition - Compare
            lstRecValues[colSourceVar] = szEMPTY;
            lstRecValues[colCondition] = szEMPTY;
            lstRecValues[colCompare] = szEMPTY;

        }
        // Caso Alarm / Event
            // behavior_alarm,
            // behavior_event,

    }
    // Return value
    return true;
}

void ctedit::on_cmdHideShow_toggled(bool checked)
{

    // Titolo del Bottone
    if (checked)  {
        m_fShowAllRows = true;
        // ui->cmdHideShow->setText(tr("Hide"));
    }
    else  {
        m_fShowAllRows = false;
        // ui->cmdHideShow->setText(tr("Show"));
    }
    // Show-Hide Rows
    showAllRows(m_fShowAllRows);
}
bool ctedit::values2Iface(QStringList &lstRecValues)
// Copia Lista Stringhe convertite da CT Record a Zona di Editing
// Caricamento Lista valori stringa su area editing
{
    QString szTemp;
    int     nPos = 0;
    int     nProtocol = -1;
    QList<int> lstVarTypes;

    lstVarTypes.clear();
    m_vtAlarmVarType = UNKNOWN;
    // Row #
    szTemp = QString::number(m_nGridRow + 1);
    ui->txtRow->setText(szTemp);
    // Priority
    szTemp = lstRecValues[colPriority].trimmed();
    ui->cboPriority->setCurrentIndex(-1);
    if (! szTemp.isEmpty())  {
        nPos = ui->cboPriority->findText(szTemp, Qt::MatchFixedString);
        if (nPos >= 0 && nPos < ui->cboPriority->count())
            ui->cboPriority->setCurrentIndex(nPos);
    }       
    // Update
    szTemp = lstRecValues[colUpdate].trimmed();
    ui->cboUpdate->setCurrentIndex(-1);
    if (! szTemp.isEmpty())  {
        nPos = ui->cboUpdate->findText(szTemp, Qt::MatchFixedString);
        if (nPos >= 0 && nPos < ui->cboUpdate->count())
            ui->cboUpdate->setCurrentIndex(nPos);
    }
    // Name
    szTemp = lstRecValues[colName].trimmed();
    ui->txtName->setText(szTemp);
    // Type
    szTemp = lstRecValues[colType].trimmed();
    ui->cboType->setCurrentIndex(-1);
    if (! szTemp.isEmpty())  {
        nPos = ui->cboType->findText(szTemp, Qt::MatchFixedString);
        if (nPos >= 0 && nPos < ui->cboType->count())
            ui->cboType->setCurrentIndex(nPos);
    }
    // Decimal
    szTemp = lstRecValues[colDecimal].trimmed();
    ui->txtDecimal->setText(szTemp);
    // Protocol
    szTemp = lstRecValues[colProtocol].trimmed();
    ui->cboProtocol->setCurrentIndex(-1);
    if (! szTemp.isEmpty())  {
        nPos = ui->cboProtocol->findText(szTemp, Qt::MatchFixedString);
        if (nPos >= 0 && nPos < ui->cboProtocol->count())  {
            ui->cboProtocol->setCurrentIndex(nPos);
            nProtocol = nPos;
        }
    }
    // IP
    szTemp = lstRecValues[colIP].trimmed();
    ui->txtIP->setText(szTemp);
    // Port
    szTemp = lstRecValues[colPort].trimmed();
    // Porta in ComboBox
    if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
        ui->cboPort->setCurrentIndex(-1);
        nPos = ui->cboPort->findText(szTemp);
        if (nPos >= 0 && nPos < ui->cboPort->count())  {
            ui->cboPort->setCurrentIndex(nPos);
        }
    }
    // Port Comunque in Text
    ui->txtPort->setText(szTemp);
    // Node ID
    szTemp = lstRecValues[colNodeID].trimmed();
    ui->txtNode->setText(szTemp);
    // Register
    szTemp = lstRecValues[colRegister].trimmed();
    ui->txtRegister->setText(szTemp);
    // Block
    szTemp = lstRecValues[colBlock].trimmed();
    ui->txtBlock->setText(szTemp);
    // N° Registro
    szTemp = lstRecValues[colBlockSize].trimmed();
    ui->txtBlockSize->setText(szTemp);
    // Comment
    szTemp = lstRecValues[colComment].trimmed().left(MAX_COMMENT_LEN - 1);
    ui->txtComment->setText(szTemp);
    // Behavior
    szTemp = lstRecValues[colBehavior].trimmed();
    ui->cboBehavior->setCurrentIndex(-1);
    // Analisi del Behavior per determinare uso degli Allarmi
    if (! szTemp.isEmpty())  {
        nPos = ui->cboBehavior->findText(szTemp, Qt::MatchFixedString);
        if (nPos >= 0 && nPos < ui->cboBehavior->count())
            ui->cboBehavior->setCurrentIndex(nPos);
        // Caricamento ulteriori elementi interfaccia Allarmi / Eventi
        if (nPos >= behavior_alarm)  {
            // Clear Data Entry Form for Alarm/Variables
            // Seleziona tutte le variabili tranne le H lstAllVarTypes a prescindere dallo stato della variabile
            ui->cboVariable1->setCurrentIndex(-1);
            ui->cboVariable2->setCurrentIndex(-1);
            fillComboVarNames(ui->cboVariable1, lstAllVarTypes, lstNoHUpdates);
            fillComboVarNames(ui->cboVariable2, lstAllVarTypes, lstNoHUpdates);
            ui->txtFixedValue->setText(szEMPTY);
            // Ricerca posizione prima variabile
            szTemp = lstRecValues[colSourceVar].trimmed();
            // qDebug() << "Alarm Source Variable:" << szTemp;
            nPos = -1;
            if (! szTemp.isEmpty())  {
                nPos = ui->cboVariable1->findText(szTemp, Qt::MatchExactly);
            }
            ui->cboVariable1->setCurrentIndex(nPos);
            // Operatore confronto
            nPos = -1;
            szTemp = lstRecValues[colCondition].trimmed();
            if (! szTemp.isEmpty())  {
                nPos = ui->cboCondition->findText(szTemp, Qt::MatchExactly);
            }
//          // Rising e Falling concesso solo per i vari tipi di Bit
//            if (nPos >= oper_rising && nPos <= oper_falling &&
//                    !(m_nAlarmVarType == BIT || m_nAlarmVarType == BYTE_BIT || m_nAlarmVarType == WORD_BIT || m_nAlarmVarType == DWORD_BIT))  {
//                nPos = -1;
//            }
            ui->cboCondition->setCurrentIndex(nPos);
            // Seconda parte dell'espressione
            // Prepara le cose come se non ci fosse una seconda parte
            ui->optFixedVal->setChecked(false);
            ui->optVariableVal->setChecked(false);
            ui->fraSelector->setEnabled(false);
            ui->fraRight->setEnabled(false);
            // Condizione binaria
            if (nPos < oper_rising)  {
                ui->fraSelector->setEnabled(true);
                ui->fraRight->setEnabled(true);
                // Determina se la parte DX espressione è un Numero o un Nome Variabile
                szTemp = lstRecValues[colCompare].trimmed();
                if (! szTemp.isEmpty())  {
                    QChar c = szTemp.at(0);
                    if (! c.isLetter())  {
                        // Numero
                        ui->optFixedVal->setChecked(true);
                        ui->txtFixedValue->setText(szTemp);
                    }
                    else  {
                        // Variabile
                        ui->optVariableVal->setChecked(true);
                        // Riempimento della Combo già avvenuto nella selezione della variabile1
                        nPos = ui->cboVariable2->findText(szTemp, Qt::MatchExactly);
                        ui->cboVariable2->setCurrentIndex(nPos);
                    }
                }
            }
        }
        else  {
            // Svuota d'ufficio sempre le combo delle variabili di allarme
            ui->cboVariable1->clear();
            ui->cboVariable2->clear();
        }
    }
    // Abilitazione campi
    enableFields();
    return true;
}
bool ctedit::iface2values(QStringList &lstRecValues)
// Copia da Zona Editing a Lista Stringhe per Controlli, Grid e Record CT
{
    QString szTemp;
    int     nPos = 0;
    int     nUpdate = -1;
    int     nProtocol = -1;

    // Pulizia Buffers
    szTemp.clear();
    listClear(lstRecValues);
    // Priority
    nPos = ui->cboPriority->currentIndex();
    if (nPos>= 0 && nPos < lstPriority.count())
        szTemp = ui->cboPriority->itemData(nPos).toString();
    else
        szTemp = szEMPTY;
    lstRecValues[colPriority] = szTemp.trimmed();
    // Update colUpdate
    nUpdate = ui->cboUpdate->currentIndex();
    if (nUpdate >= 0 && nPos < lstUpdateNames.count())
        szTemp = ui->cboUpdate->itemData(nUpdate).toString();
    else
        szTemp = szEMPTY;
    lstRecValues[colUpdate] = szTemp;
    // Name
    szTemp = ui->txtName->text().trimmed();
    lstRecValues[colName] = szTemp;
    // Update Used and Logged Variable List
    if (! szTemp.isEmpty())  {
        // Aggiorna la lista delle variabili utilizzate
        nPos = lstUsedVarNames.indexOf(szTemp);
        if (nPos < 0)  {
            lstUsedVarNames.append(szTemp);
            lstUsedVarNames.sort();
        }
        // Aggiorna la lista delle Variabili loggate se la variabile è loggata
        if (nUpdate != Htype && nUpdate != Ptype)  {
            fillVarList(lstLoggedVars, lstAllVarTypes, lstLogUpdates);
        }
    }
    // Type colType
    nPos = ui->cboType->currentIndex();
    if (nPos >= 0 && nPos < lstTipi.count())
        szTemp = ui->cboType->itemData(nPos).toString();
    else
        szTemp = szEMPTY;
    lstRecValues[colType] = szTemp.trimmed();
    // Decimal
    szTemp = ui->txtDecimal->text();
    lstRecValues[colDecimal] = szTemp.trimmed();
    // Protocol lstBusType
    nProtocol = ui->cboProtocol->currentIndex();
    if (nProtocol >= 0 && nProtocol < lstProtocol.count())
        szTemp = ui->cboProtocol->itemData(nProtocol).toString();
    else
        szTemp = szEMPTY;
    lstRecValues[colProtocol] = szTemp.trimmed();
    // IP
    szTemp = ui->txtIP->text();
    lstRecValues[colIP] = szTemp.trimmed();
    // Port
    // Protocolli Seriali
    if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)
        szTemp = ui->cboPort->currentText();
    else
        szTemp = ui->txtPort->text();
    qDebug() << tr("Port: ") << szTemp;

    lstRecValues[colPort] = szTemp.trimmed();
    // Node ID
    szTemp = ui->txtNode->text();
    lstRecValues[colNodeID]  = szTemp.trimmed();
    // Register
    szTemp = ui->txtRegister->text();
    lstRecValues[colRegister] = szTemp.trimmed();
    // Block
    szTemp = ui->txtBlock->text();
    lstRecValues[colBlock] = szTemp.trimmed();
    // N° Registro
    szTemp = ui->txtBlockSize->text();
    lstRecValues[colBlockSize] = szTemp.trimmed();
    // Comment
    szTemp = ui->txtComment->text();
    lstRecValues[colComment] = szTemp.trimmed().left(MAX_COMMENT_LEN - 1);
    // Clear all values for Alarms/Events
    lstRecValues[colSourceVar] = szEMPTY;
    lstRecValues[colCondition] = szEMPTY;
    lstRecValues[colCompare] = szEMPTY;
    // Behavior
    nPos = ui->cboBehavior->currentIndex();
    if (nPos >= 0 && nPos < lstBehavior.count())
        szTemp = ui->cboBehavior->itemText(nPos);
    else
        szTemp = szEMPTY;
    lstRecValues[colBehavior] = szTemp.trimmed();
    // Gestione Allarmi/Eventi (Se necessario)
    if (nPos >= behavior_alarm)  {
        // Source Var
        nPos = ui->cboVariable1->currentIndex();
        if (nPos >= 0 && nPos < ui->cboVariable1->count())
            szTemp = ui->cboVariable1->itemText(nPos);
        else
            szTemp = szEMPTY;
        // qDebug() << "Variable Var1 Pos: " << nPos;
        lstRecValues[colSourceVar] = szTemp;
        // Operator
        nPos = ui->cboCondition->currentIndex();
        if (nPos >= 0 && nPos < ui->cboCondition->count())
            szTemp = lstCondition[nPos];
        else
            szTemp = szEMPTY;
        lstRecValues[colCondition] = szTemp;
        // Fixed Value or Variable name
        if (ui->optFixedVal->isChecked())  {
            // Save Fixed Value
            szTemp = ui->txtFixedValue->text().trimmed();
        }
        else  {
            // Save Variable Name
            nPos = ui->cboVariable2->currentIndex();
            if (nPos >= 0 && nPos < ui->cboVariable2->count())
                szTemp = ui->cboVariable2->itemText(nPos);
            else
                szTemp = szEMPTY;
        }
        lstRecValues[colCompare] = szTemp;
    }
    // Finalizzazione controlli su protocolli
    // Protocollo PLC tutto abblencato
    if (nProtocol == PLC)  {
        lstRecValues[colIP] = szEMPTY;
        lstRecValues[colPort] = szEMPTY;
        lstRecValues[colNodeID] = szEMPTY;
        lstRecValues[colRegister] = szEMPTY;
    }
    // Protocolli Seriali e CAN Porta a 0 se vuota
    if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV || nProtocol == CANOPEN)  {
        szTemp = lstRecValues[colPort].trimmed();
        if (szTemp.isEmpty())
            lstRecValues[colPort] = szZERO;
    }
    // qDebug() << "Alarm Source Variable: " << lstRecValues[colSourceVar];
    // Return value
    return true;
}

void ctedit::on_cmdBlocchi_clicked()
{
    if (riassegnaBlocchi())  {
        m_isCtModified = true;
    }
    // Refresh abilitazioni interfaccia
    enableInterface();
}

void ctedit::on_cmdSave_clicked()
{
    saveCTFile();
}
void ctedit::saveCTFile()
{
    int     nRes = 0;
    int     nCur = 0;
    int     nErr = 0;
    int     nProtocol = 0;
    int     nCurRow = m_nGridRow;

    // Controllo errori
    nErr = globalChecks();
    if (nErr)  {
        // Ci sono errori, salvo comunque ?
        m_szMsg = tr("There are errors in Data. Save anyway?");
        if (! queryUser(this, szMectTitle, m_szMsg, false))
            return;
    }
    else  {
        // Non ci sono errori, eseguo d'ufficio la rinumerazione blocchi
        if (! riassegnaBlocchi())  {
            m_szMsg = tr("Found Errors in Reassigning Blocks");
            warnUser(this, szMectTitle, m_szMsg);
        }
    }
    // Back-Up Copy of CT File
    fileBackUp(m_szCurrentCTFile);
    // Copy CT Record List to C Array
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        if (nCur < DimCrossTable)  {
            // Forzature legate al protocollo
            nProtocol = lstCTRecords[nCur].Protocol;
            // Per PLC scaraventa tutto a Blank
            if (nProtocol == PLC)  {
                lstCTRecords[nCur].IPAddress = 0;
                lstCTRecords[nCur].Port = 0;
                lstCTRecords[nCur].NodeId = 0;
                lstCTRecords[nCur].Offset = 0;
            }
            if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV || nProtocol == CANOPEN)  {

            }
            CrossTable[nCur + 1] = lstCTRecords[nCur];
        }
    }
    // Saving Source Array to file
    nRes = SaveXTable(m_szCurrentCTFile.toAscii().data(), CrossTable);
    // Return Value
    if (nRes != 0) {
        m_szMsg = tr("Error Saving Crosstable File: %1\n").arg(m_szCurrentCTFile);
        warnUser(this, szMectTitle, m_szMsg);
    }
    else {
        m_isCtModified = false;
    }
    // Return to Row
    jumpToGridRow(nCurRow, false);
    // Refresh abilitazioni interfaccia
    enableInterface();

}
// Riassegnazione blocchi variabili
bool    ctedit::riassegnaBlocchi()
{
    int             nRow = 0;
    bool            fRes = false;
    int             nPrevRow = -2;
    int             nBlockStart = -1;
    int             j = 0;
    int             nNextRegPos = 0;
    uint16_t        curBlock = 0;
    int16_t         curBSize = (int16_t) 0;
    int             nItemsInBlock = 0;

    ui->cmdBlocchi->setEnabled(false);
    this->setCursor(Qt::WaitCursor);
    // Copia l'attuale CT nella lista Undo
    lstUndo.append(lstCTRecords);
    for (nRow = 0; nRow < MIN_DIAG - 1; nRow++)  {
        // Ignora le righe con Priority == 0
        if (lstCTRecords[nRow].Enable > 0)  {
            // Salto riga o condizione di inizio nuovo blocco
            // Inizio nuovo blocco
            if (nPrevRow != nRow - 1
                    || lstCTRecords[nRow - 1].Enable != lstCTRecords[nRow].Enable
                    || lstCTRecords[nRow - 1].Protocol !=  lstCTRecords[nRow].Protocol
                    || lstCTRecords[nRow - 1].IPAddress != lstCTRecords[nRow].IPAddress
                    || lstCTRecords[nRow - 1].Port != lstCTRecords[nRow].Port
                    || lstCTRecords[nRow - 1].NodeId != lstCTRecords[nRow].NodeId
                    || isTooBigForBlock(nRow, nItemsInBlock, curBSize)
                    // Per Modbus devono essere o tutti BIT o tutti != BIT
                    || (isModbus(lstCTRecords[nRow].Protocol) && ((lstCTRecords[nRow - 1].VarType == BIT && lstCTRecords[nRow].VarType != BIT) || (lstCTRecords[nRow - 1].VarType != BIT && lstCTRecords[nRow].VarType == BIT)))
                    // Per Modbus Contiguità di Registro
                    || (isModbus(lstCTRecords[nRow].Protocol) && (lstCTRecords[nRow].Offset != nNextRegPos && !isSameBitField(nRow)))
                    )  {
                // Rinumera block start del Blocco precedente se esiste
                if ((nBlockStart >= 0) && (nRow - nBlockStart > 1))  {
                    for (j = nBlockStart; j < nRow; j++)  {
                        lstCTRecords[j].BlockSize = nItemsInBlock;
                    }
                }
                // Imposta i valori di confronto correnti
                nBlockStart = nRow;
                curBlock = (int16_t) nRow + 1;
                if (lstCTRecords[nRow].Protocol ==  PLC)
                    curBSize = 1;
                else
                    curBSize = varSizeInBlock(lstCTRecords[nRow].VarType);

                nItemsInBlock = 1;
            }
            // Prosecuzione Blocco corrente
            else  {
                if (lstCTRecords[nRow].Protocol ==  PLC)
                    curBSize++;
                else  {
                    if (! isSameBitField(nRow))
                        curBSize += varSizeInBlock(lstCTRecords[nRow].VarType);
                }
                nItemsInBlock++;
            }
            // Aggiornamento Blocco e Size
            lstCTRecords[nRow].Block = curBlock;
            lstCTRecords[nRow].BlockSize = nItemsInBlock;
            nNextRegPos = lstCTRecords[nRow].Offset + varSizeInBlock(lstCTRecords[nRow].VarType);
            // Incremento ultima riga significativa
            nPrevRow = nRow;
        }
        else
            lstCTRecords[nRow].Block = 0;
    }
    // Rinumera ultimo blocco trattato (se esiste)
    if (nBlockStart >= 0)  {
        for (j = nBlockStart; j < MIN_DIAG - 1; j++)  {
            if (lstCTRecords[j].UsedEntry == 0)
                break;
            lstCTRecords[j].BlockSize = nItemsInBlock;
        }
    }
    // Return value as reload CT
    fRes = ctable2Grid();
    // qDebug() << "Reload finished";
    ui->cmdBlocchi->setEnabled(true);
    this->setCursor(Qt::ArrowCursor);
    return fRes;
}
bool ctedit::grid2CTable()
// Dump di tutto il Grid in lista di CT Records
{
    bool        fRes = true;
    int         nCur = 0;
    int         nCol = 0;
    QString     szTemp;
    QTableWidgetItem    *tItem;
    QStringList lstFields;

    // Ciclo sugli elementi di Grid
    for (nCur = 1; nCur <= ui->tblCT->rowCount(); nCur++)  {
        lstFields.clear();
        // Insert Items at Row, Col
        for (nCol = 1; nCol < colTotals; nCol++)  {
            tItem = ui->tblCT->item(nCur, nCol);
            szTemp = tItem->text();
            // Aggiunta alla Lista
            lstFields.append(szTemp);
        }
        // Covert back User Values 2 CT Record
        fRes = list2CTrec(lstFields, nCur);
    }
    // Return Value
    return fRes;
}
void ctedit::freeCTrec(int nRow)
// Marca il Record della CT come inutilizzato
{
    if (nRow < 0 || nRow >= lstCTRecords.count())
        return;
    lstCTRecords[nRow].UsedEntry = 0;
    lstCTRecords[nRow].Enable = 0;
    lstCTRecords[nRow].Update = (UpdateType) 0;
    strcpy(lstCTRecords[nRow].Tag, "");
    lstCTRecords[nRow].VarType = (varTypes) 0;
    lstCTRecords[nRow].Decimal = 0;
    lstCTRecords[nRow].Protocol = (FieldbusType) 0;
    lstCTRecords[nRow].IPAddress = 0;
    lstCTRecords[nRow].Port = 0;
    lstCTRecords[nRow].NodeId = 0;
    lstCTRecords[nRow].Offset = 0;
    lstCTRecords[nRow].Block = 0;
    lstCTRecords[nRow].BlockBase = 0;
    lstCTRecords[nRow].Behavior = 0;
    lstCTRecords[nRow].Counter = 0;
    lstCTRecords[nRow].OldVal = 0;
    lstCTRecords[nRow].Error = 0;
    lstCTRecords[nRow].device = 0;
    lstCTRecords[nRow].usedInAlarmsEvents = FALSE;
    lstCTRecords[nRow].ALType = -1;
    strcpy(lstCTRecords[nRow].ALSource, "");
    lstCTRecords[nRow].ALOperator = -1;
    strcpy(lstCTRecords[nRow].ALCompareVar, "");
    lstCTRecords[nRow].ALCompareVal = 0.0;
    lstCTRecords[nRow].ALComparison = -1;
    lstCTRecords[nRow].ALCompatible  = FALSE;
    strcpy(lstCTRecords[nRow].Comment, "");
}
void ctedit::listClear(QStringList &lstRecValues)
// Svuotamento e pulizia Lista Stringhe per passaggio dati Interfaccia <---> Record CT
// La lista passata come parametro viene svuotata e riempita con colTotals stringhe vuote
{
    int nCol = 0;
    lstRecValues.clear();
    for (nCol = 0; nCol < colTotals; nCol++)  {
        lstRecValues.append(szEMPTY);
    }
}

bool ctedit::list2CTrec(QStringList &lstRecValues, int nRow)
// Conversione da Lista Valori di Interfaccia a CT Record (Form -> REC SINGOLO)
// Scrive un Record letto da interfaccia direttamente in lista di Record C
{
    bool        fRes = true;
    bool        fOk = false;
    int         nPos = 0;
    char        ip[MAX_IPADDR_LEN];

    // Abilitazione riga (Nome Vuoto => Riga disabilitata)
    if (lstRecValues[colName].isEmpty())  {
        freeCTrec(nRow);
    }
    else  {
        // Priority
        nPos = lstPriority.indexOf(lstRecValues[colPriority]);
        nPos = (nPos >= 0 && nPos < lstPriority.count()) ? nPos : 0;
        lstCTRecords[nRow].Enable = (int16_t) nPos;
        // Update
        nPos = lstUpdateNames.indexOf(lstRecValues[colUpdate]);
        nPos = (nPos >= 0 && nPos < lstUpdateNames.count()) ? nPos : 0;
        lstCTRecords[nRow].Update = (UpdateType) nPos;
        // Campo Name
        strcpy(lstCTRecords[nRow].Tag, lstRecValues[colName].trimmed().toAscii().data());
        // Campo Abilitazione record
        if (strlen(lstCTRecords[nRow].Tag) > 0)
            lstCTRecords[nRow].UsedEntry = 1;
        else
            lstCTRecords[nRow].UsedEntry = 0;
        // Campo Type
        nPos = lstTipi.indexOf(lstRecValues[colType]);
        nPos = (nPos >= 0 && nPos < lstTipi.count()) ? nPos : 0;
        lstCTRecords[nRow].VarType = (varTypes) nPos;
        // Campo Decimal
        nPos = lstRecValues[colDecimal].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecords[nRow].Decimal = nPos;
        // Protocol
        nPos = lstProtocol.indexOf(lstRecValues[colProtocol]);
        nPos = (nPos >= 0 && nPos < lstProtocol.count()) ? nPos : 0;
        lstCTRecords[nRow].Protocol = (FieldbusType) nPos;
        // IP Address (Significativo solo per Protocolli a base TCP)
        if (lstCTRecords[nRow].Protocol == TCP || lstCTRecords[nRow].Protocol == TCPRTU ||
                lstCTRecords[nRow].Protocol == TCP_SRV || lstCTRecords[nRow].Protocol ==TCPRTU_SRV)  {
            strcpy(ip, lstRecValues[colIP].trimmed().toAscii().data());
            nPos = str2ipaddr(ip);
            lstCTRecords[nRow].IPAddress = (uint32_t) nPos;
        }
        else
            lstCTRecords[nRow].IPAddress = (uint32_t) 0 ;
        // Port
        nPos = lstRecValues[colPort].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecords[nRow].Port = nPos;
        // Node Id
        nPos = lstRecValues[colNodeID].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecords[nRow].NodeId = nPos;
        // Register
        nPos = lstRecValues[colRegister].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        lstCTRecords[nRow].Offset = nPos;
        // Block
        nPos = lstRecValues[colBlock].toInt(&fOk);
        nPos = fOk ? nPos : 0;
        if (nPos == 0)  {
            lstCTRecords[nRow].Block = nRow + 1;
            lstCTRecords[nRow].BlockSize = 1;
        }
        else {
            lstCTRecords[nRow].Block = nPos;
            // N.Registro
            nPos = lstRecValues[colBlockSize].toInt(&fOk);
            nPos = fOk ? nPos : 1;
            lstCTRecords[nRow].BlockSize = nPos;
        }
        // Commento
        strncpy(lstCTRecords[nRow].Comment, lstRecValues[colComment].trimmed().toAscii().data(), MAX_COMMENT_LEN - 1);
        // Clear all Variable - Event fields
        lstCTRecords[nRow].usedInAlarmsEvents = FALSE;
        lstCTRecords[nRow].ALType = -1;
        strcpy(lstCTRecords[nRow].ALSource, szEMPTY.toAscii().data());
        lstCTRecords[nRow].ALOperator = -1;
        strcpy(lstCTRecords[nRow].ALCompareVar, szEMPTY.toAscii().data());
        lstCTRecords[nRow].ALCompareVal = 0.0;
        lstCTRecords[nRow].ALComparison = -1;
        lstCTRecords[nRow].ALCompatible = 0;
        // Behavior
        nPos = lstBehavior.indexOf(lstRecValues[colBehavior]);
        nPos = (nPos >= 0 && nPos < lstBehavior.count()) ? nPos : 0;
        lstCTRecords[nRow].Behavior = nPos;
        // Salvataggio dei valori di Allarme/Evento
        if (nPos >= behavior_alarm)   {
            // Flag isAlarm
            lstCTRecords[nRow].usedInAlarmsEvents = TRUE;
            // Type of Alarm or Event
            if (nPos == behavior_alarm)
                lstCTRecords[nRow].ALType = Alarm;
            else
                lstCTRecords[nRow].ALType = Event;
            // Left Variable Name
            strcpy(lstCTRecords[nRow].ALSource, lstRecValues[colSourceVar].trimmed().toAscii().data());
            // Operator
            nPos = lstCondition.indexOf(lstRecValues[colCondition]);
            nPos = (nPos >= 0 && nPos < lstCondition.count()) ? nPos : -1;
            lstCTRecords[nRow].ALOperator = nPos;
            // Compare VAR - VAL
            QString szCompare = lstRecValues[colCompare].trimmed();
            if (szCompare.isEmpty())  {
                strcpy(lstCTRecords[nRow].ALCompareVar, szEMPTY.toAscii().data());
                lstCTRecords[nRow].ALCompareVal = 0.0;
            }
            else  {
                // Decisione se il secondo lato dell'espressione sia una costante o un nome variabile
                QChar cc = szCompare.at(0);
                if (cc.isLetter())  {
                    // Variable
                    strcpy(lstCTRecords[nRow].ALCompareVar, szCompare.toAscii().data());
                    lstCTRecords[nRow].ALCompareVal = 0.0;
                }
                else  {
                    float fValue = 0;
                    // Value
                    strcpy(lstCTRecords[nRow].ALCompareVar, szEMPTY.toAscii().data());
                    fValue = szCompare.toFloat(&fOk);
                    fValue = fOk ? fValue : 0.0;
                    lstCTRecords[nRow].ALCompareVal = fValue;
                    // TODO: Fill correct values for Comparison and Compatible
                    lstCTRecords[nRow].ALComparison = COMP_UNSIGNED;
                    lstCTRecords[nRow].ALCompatible = 1;
                }
            }
        }
    }
    // Return Value
    return fRes;
}
void ctedit::enableFields()
// Abilitazione dei campi form in funzione del Protocollo
{
    int     nDefPort = -1;
    int     nTotalPorts = 0;
    int     nProtocol = ui->cboProtocol->currentIndex();

    // Disabilita tutti i campi
    ui->cboPriority->setEnabled(false);
    ui->cboUpdate->setEnabled(false);
    ui->txtName->setEnabled(false);
    ui->cboType->setEnabled(false);
    ui->txtDecimal->setEnabled(false);
    ui->cboProtocol->setEnabled(false);
    ui->txtIP->setEnabled(false);
    ui->txtPort->setEnabled(false);
    ui->cboPort->setEnabled(false);
    ui->txtNode->setEnabled(false);
    ui->txtRegister->setEnabled(false);
    ui->txtComment->setEnabled(false);
    ui->cboBehavior->setEnabled(false);
    // Variabili di Sistema, abilitate in modifica solo il nome, priorità, update. No Insert in campi vuoti
    if (m_nGridRow >= MIN_DIAG -1)  {
        bool fDecimal = false;
        if (nProtocol != -1)  {
            ui->cboPriority->setEnabled(true);
            ui->cboUpdate->setEnabled(true);
            ui->txtName->setEnabled(true);
            ui->txtComment->setEnabled(true);
        }
        // Abilitazione dei Decimal per Input Analogiche se abilitati
        if (TargetConfig.analogIN > 0 && TargetConfig.analogINrowCT > 0)  {
            if (m_nGridRow >= TargetConfig.analogINrowCT -1 && m_nGridRow < TargetConfig.analogINrowCT + TargetConfig.analogIN -1)
                fDecimal = true;
        }
        // Abilitazione dei Decimal per Output Analogiche se abilitati
        if (TargetConfig.analogOUT > 0 && TargetConfig.analogOUTrowCT > 0)  {
            if (m_nGridRow >= TargetConfig.analogOUTrowCT -1 && m_nGridRow < TargetConfig.analogOUTrowCT + TargetConfig.analogOUT -1)
                fDecimal = true;
        }
        ui->txtDecimal->setEnabled(fDecimal);
    }
    else  {
        // Campi comuni
        ui->cboPriority->setEnabled(true);
        ui->cboUpdate->setEnabled(true);
        ui->txtName->setEnabled(true);
        ui->cboType->setEnabled(true);
        ui->txtDecimal->setEnabled(true);
        ui->cboProtocol->setEnabled(true);
        ui->txtComment->setEnabled(true);
        ui->cboBehavior->setEnabled(true);
        // Abilitazione dei campi in funzione del Tipo
        // Tipo BIT -> Blocca decimali
        if (ui->cboType->currentIndex()  == BIT)  {
            ui->txtDecimal->setEnabled(false);
        }
        // Protocollo non definito, es per Riga vuota
        if (nProtocol == -1)  {
            ui->txtIP->setEnabled(true);
            ui->txtPort->setEnabled(true);
            ui->txtNode->setEnabled(true);
            ui->txtRegister->setEnabled(true);
        }
        // Calcola la porta di default in funzione del protocollo (if any available)
        getFirstPortFromProtocol(nProtocol, nDefPort, nTotalPorts);
        // Abilitazione dei campi se non PLC
        if (nProtocol != PLC)  {
            ui->txtNode->setEnabled(true);
            ui->txtRegister->setEnabled(true);
            // TCP, TCPRTU, TCP_SRV, TCPRTU_SRV
            if (nProtocol == TCP || nProtocol == TCPRTU  ||
                     nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV)  {
                ui->txtIP->setEnabled(true);
                ui->txtPort->setEnabled(true);
            }
        }
        // Visibilità Combo Port per Protocolli Seriali di ogni tipo
        if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
            ui->lblPort_RTU->setVisible(true);
            ui->lblPort->setVisible(false);
            ui->cboPort->setEnabled(nTotalPorts > 1);
            ui->cboPort->setVisible(true);
            ui->txtPort->setVisible(false);
        }
        else  {
            ui->lblPort_RTU->setVisible(false);
            ui->lblPort->setVisible(true);
            ui->cboPort->setEnabled(false);
            ui->cboPort->setVisible(false);
            ui->txtPort->setVisible(true);
        }
    }
}
void ctedit::on_cboType_currentIndexChanged(int index)
// Cambio di tipo della variabile
{

    // Il tipo Bit forza i Decimali a 0 e rende il campo non editabile
    if (index == BIT)  {
        ui->txtDecimal->setText(szZERO);
        ui->txtDecimal->setEnabled(false);
        // Abilita possibilità di rendere la variabile un allarme/evento solo se Priority > H
        if (ui->cboPriority->currentIndex() > 0 && ui->cboUpdate->currentIndex() > Htype)  {
            enableComboItem(ui->cboBehavior, behavior_alarm);
            enableComboItem(ui->cboBehavior, behavior_event);
        }
    }
    else  {
        // Se la variabile era allarme ripulisce la selezione
        if (ui->cboBehavior->currentIndex() >= behavior_alarm)
            ui->cboBehavior->setCurrentIndex(-1);
        ui->txtDecimal->setEnabled(true);
        // Default 0 decimale se non specificato o presente valore
        if (ui->txtDecimal->text().trimmed().isEmpty())
            ui->txtDecimal->setText(szZERO);
        // Disabilita possibilità di rendere la variabile un allarme/evento
        disableComboItem(ui->cboBehavior, behavior_alarm);
        disableComboItem(ui->cboBehavior, behavior_event);
    }
}
void ctedit::on_cboProtocol_currentIndexChanged(int index)
// Cambio di Protocollo della Variabile. Abilita i campi specifici del protocollo e imposta eventuali valori di default se necessari
{
    QString     szTemp;
    int         nDefaultPort = -1;
    int         nCurrentPort = -1;
    QString     szCurPort;
    int         nTotalFree = 0;
    bool        fPortOk = false;

    szTemp.clear();
    // No Index
    if (index == -1)  {
        ui->txtIP->setText(szEMPTY);
        ui->txtPort->setText(szEMPTY);
        ui->txtNode->setText(szEMPTY);
        ui->txtRegister->setText(szEMPTY);
    }
    // Calcola la porta di default in funzione del protocollo (if any available)
    getFirstPortFromProtocol(index, nDefaultPort, nTotalFree);
    // Recupera il numero di porta corrente
    szCurPort.clear();
    if (index == RTU || index == MECT_PTC || index == RTU_SRV)  {
        szCurPort = ui->cboPort->currentText();
        nCurrentPort = ui->cboPort->currentIndex();
    }
    else  {
        szCurPort = ui->txtPort->text().trimmed();
        nCurrentPort = ! szCurPort.isEmpty() ? szCurPort.toInt(&fPortOk) : -1;
        nCurrentPort = fPortOk ? nCurrentPort : -1;
    }
    fPortOk = isValidPort(nCurrentPort, index);
    // PLC
    if (index == PLC)  {
        // All Data Entry Cleared
        ui->txtIP->setText(szEMPTY);
        ui->txtPort->setText(szEMPTY);
        ui->txtNode->setText(szEMPTY);
        ui->cboPort->setCurrentIndex(-1);
        ui->txtRegister->setText(szEMPTY);       
    }
    // RTU vari - MECT - CANOPEN
    else if (index == RTU || index == RTU_SRV || index == MECT_PTC || index == CANOPEN)  {
        // Forza Ip Vuoto
        ui->txtIP->setText(szEMPTY);
    }
    // TCP, TCPRTU,
    else if (index == TCP || index == TCPRTU)  {
    }
    // TCP_SRV, TCPRTU_SRV
    else if (index == TCP_SRV || index == TCPRTU_SRV)  {
        // Default IP to Empty for Servers
        szTemp = ui->txtIP->text().trimmed();
        if (szTemp.isEmpty())
            ui->txtIP->setText(szEMPTY_IP);
    }
    // Disabilitazione  tipo BIT per i vari tipi di SRV (TCP_SRV, RTU_SRV, TCP_RTU_SRV)
    // Non è ammesso BIT per i protocolli SERVER (TCP/RTU)
    if (index == TCP_SRV || index == TCPRTU_SRV || index == RTU_SRV)  {
        disableComboItem(ui->cboType, BIT);
    }
    else {
        enableComboItem(ui->cboType, BIT);
    }
    // Imposta la Porta di default se la porta è vuota oppure quella presente non è valida
    if (szCurPort.isEmpty() || (nDefaultPort >= 0 && ! fPortOk))  {
        szCurPort = QString::number(nDefaultPort);
        // Reimposta default
        if (index == RTU || index == MECT_PTC || index == RTU_SRV)
            ui->cboPort->setCurrentIndex(nDefaultPort);
        else
            ui->txtPort->setText(szCurPort);
    }
    // Abilitazione del campi di data entry in funzione del Protocollo
    enableFields();
}
void ctedit::tableItemChanged(const QItemSelection & selected, const QItemSelection & deselected)
// Slot attivato ad ogni cambio di riga in
{
    int         nRow = -1;
    QStringList lstFields;
    bool        fRes = true;
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();

    // Il cambio riga corrente è dovuto a operazioni di tipo cut-paste.
    // Per evitare duplicazioni accidentali di righe ripulisce il buffer di editing ed esce
    if (m_fCutOrPaste || selection.count() > 1)  {
        if (selection.count() > 1)  {
            m_szMsg = tr("Selected Rows: %1") .arg(selection.count());
            displayStatusMessage(m_szMsg, STD_DISPLAY_TIME);
        }
        clearEntryForm();
        ui->fraEdit->setEnabled(false);
        ui->cboSections->setCurrentIndex(-1);
        return;
    }
    ui->fraEdit->setEnabled(true);
    // Si sta uscendo dalla selezione di una riga sola
    if (! deselected.isEmpty() &&  deselected.count() == 1)  {
        // Considera sempre la prima riga della lista
        nRow = deselected.indexes().at(0).row();
        qDebug() << "Previous Row: " << nRow;
    }
    // Se la riga corrente è stata modificata, salva il contenuto
    if (nRow >= 0 && nRow < lstCTRecords.count())  {
        // Il contenuto viene aggiornato solo se la linea risulta modificata e il form non è vuoto
        fRes = updateRow(nRow);
    }
    // Cambio riga Ko
    if (! fRes)    {
        // Disconnette segnale per evitare ricorsione
        disconnect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
                    SLOT(tableItemChanged(const QItemSelection &, const QItemSelection & ) ));
        // Cambia Selezione (ritorna a riga precedente)
        ui->tblCT->selectRow(nRow);
        // Riconnette slot gestione
        connect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
                    SLOT(tableItemChanged(const QItemSelection &, const QItemSelection & ) ));
        return;
    }
    // Si può cambiare riga, legge contenuto
    if (! selected.isEmpty() && selected.count() == 1)  {
        clearEntryForm();
        // Estrae il numero di riga del modello lavorando sulla prima riga selezionata
        nRow = selected.indexes().at(0).row();
        qDebug() << "New Row: " << nRow;
        if (nRow >= 0)  {
            // Cambia riga corrente
            m_nGridRow = nRow;
            // Imposta Sezione in cboSections
            setSectionArea(nRow);
            // Convert CT Record 2 User Values
            fRes = recCT2List(lstFields, nRow);
            if (fRes)
                fRes = values2Iface(lstFields);
        }
    }
    else  {
        clearEntryForm();
    }
    // Refresh abilitazioni interfaccia
    enableInterface();
}
void ctedit::clearEntryForm()
// Svutamento elementi Form Data Entry
{
    ui->txtRow->setText(szEMPTY);
    ui->cboPriority->setCurrentIndex(-1);
    ui->cboUpdate->setCurrentIndex(-1);
    ui->txtName->setText(szEMPTY);
    ui->cboType->setCurrentIndex(-1);
    ui->txtDecimal->setText(szEMPTY);
    ui->cboProtocol->setCurrentIndex(-1);
    ui->txtIP->setText(szEMPTY);
    ui->txtPort->setText(szEMPTY);
    ui->txtNode->setText(szEMPTY);
    ui->txtRegister->setText(szEMPTY);
    ui->txtBlock->setText(szEMPTY);
    ui->txtBlockSize->setText(szEMPTY);
    ui->txtComment->setText(szEMPTY);
    ui->cboBehavior->setCurrentIndex(-1);
    // Sottoform per Allarmi/Eventi
    ui->cboVariable1->setCurrentIndex(-1);
    ui->cboCondition->setCurrentIndex(-1);
    ui->optFixedVal->setChecked(false);
    ui->optVariableVal->setChecked(false);
    ui->txtFixedValue->setText(szEMPTY);
    ui->cboVariable2->setCurrentIndex(-1);
}

void ctedit::displayUserMenu(const QPoint &pos)
// Menu contestuale Grid
{
    int             nRow = ui->tblCT->rowAt(pos.y());
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
    QIcon cIco;
    QClipboard      *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QString         szClipBuffer;
    bool            fClipSourceOk = false;
    QList<QStringList > lstPastedRecords;
    QList<int>      lstDestRows;
    QList<int>      lstSourceRows;



    // Aggiornamento numero riga
    if (nRow >= 0)  {
        m_nGridRow = nRow;
    }
    // Verifica dei dati in ingresso in Clipboard
    // Retrieve Clipboard object
    szClipBuffer.clear();
    if (mimeData->hasText())  {
        szClipBuffer = clipboard->text();
    }
    // Check Clipboard object
    if (! szClipBuffer.isEmpty()) {
        fClipSourceOk = getRowsFromXMLBuffer(szClipBuffer, lstPastedRecords, lstSourceRows, lstDestRows);
    }

    // Oggetto menu contestuale
    QMenu gridMenu(this);
    // Items del Menu contestuale
    // Renum Blocks
    //cIco = QIcon(QString::fromAscii(":/cteicons/img/Blocks.png"));
    //QAction *menuBlocks = gridMenu.addAction(cIco, trUtf8("Renum Blocks"));
    //menuBlocks->setEnabled(true);
    // Goto Line
    cIco = QIcon(QString::fromAscii(":/cteicons/img/Goto.png"));
    QAction *menuRow = gridMenu.addAction(cIco, trUtf8("Goto Line\t\t(Ctrl+L)"));
    menuRow->setEnabled(true);
    // Sep0
    gridMenu.addSeparator();
    // Inserisci righe
    cIco = QIcon(QString::fromAscii(":/cteicons/img/List-add.png"));
    QAction *insRows = gridMenu.addAction(cIco, trUtf8("Insert Blank Rows\t\t(Ins)"));
    insRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // insRows->setShortcut(Qt::Key_Insert);

    // Cancella righe
    cIco = QIcon(QString::fromAscii(":/cteicons/img/Edit-clear.png"));
    QAction *emptyRows = gridMenu.addAction(cIco, trUtf8("Clear Rows\t\t(Del)"));
    emptyRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // emptyRows->setShortcut(Qt::Key_Delete);
    // Elimina righe
    cIco = QIcon(QString::fromAscii(":/cteicons/img/Edit-trash.png"));
    QAction *remRows = gridMenu.addAction(cIco, trUtf8("Delete Rows"));
    remRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // Sep1
    gridMenu.addSeparator();
    // Copia righe (Sempre permesso)
    cIco = QIcon(QString::fromAscii(":/cteicons/img/Copy.png"));
    QAction *copyRows = gridMenu.addAction(cIco, trUtf8("Copy Rows\t\t(Ctrl+C)"));
    copyRows->setEnabled(selection.count() > 0);
    // copyRows->setShortcut(Qt::Key_Copy);
    // Taglia righe
    cIco = QIcon(QString::fromAscii(":/cteicons/img/Cut.png"));
    QAction *cutRows = gridMenu.addAction(cIco, trUtf8("Cut Rows\t\t(Ctrl+X)"));
    cutRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // cutRows->setShortcut(Qt::Key_Cut);
    // Sep 2
    gridMenu.addSeparator();
    // Paste Rows
    cIco = QIcon(QString::fromAscii(":/cteicons/img/edit-paste.png"));
    QAction *pasteRows = gridMenu.addAction(cIco, trUtf8("Paste Rows\t\t(Ctrl+V)"));
    pasteRows->setEnabled(fClipSourceOk && m_nGridRow < MIN_DIAG - 1);
    // pasteRows->setShortcut(Qt::Key_Paste);
    // Sep 3
    gridMenu.addSeparator();
    // Menu per importazione delle variabili per modelli MECT connessi su Bus Seriale
    QAction *addMPNC005 = gridMenu.addAction(cIco, trUtf8("Paste MPNC005 Variables"));
    addMPNC005->setEnabled((TargetConfig.ser0_Enabled || TargetConfig.ser1_Enabled || TargetConfig.ser2_Enabled || TargetConfig.ser3_Enabled)
                           && m_nGridRow < MIN_DIAG - 1);
    // Esecuzione del Menu
    QAction *actMenu = gridMenu.exec(ui->tblCT->viewport()->mapToGlobal(pos));
    this->setCursor(Qt::WaitCursor);
    // Controllo dell'Azione selezionata
    // Inserimento righe
    if (actMenu == insRows)  {
        insertRows();
    }
    // Cancella Righe
    if (actMenu == emptyRows)  {
        emptySelected();
    }
    // Rimozione righe
    else if (actMenu == remRows)  {
        removeSelected();
    }
    // Copia
    else if (actMenu == copyRows)  {
        copySelected(true);
    }
    // Taglia
    else if (actMenu == cutRows)  {
        cutSelected();
    }
    // Incolla
    else if (actMenu == pasteRows)  {
        pasteSelected();
    }
    // Goto Line
    else if (actMenu == menuRow)  {
        gotoRow();
    }
    else if (actMenu == addMPNC005)  {
        addModelVars(szMPNC005, m_nGridRow);
    }
    this->setCursor(Qt::ArrowCursor);

}
int ctedit::copySelected(bool fClearSelection)
// Copia delle righe selezionate in Buffer di Copiatura
{
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
    int             nRow = 0;
    int             nCur = 0;
    int             nCol = 0;
    int             nFirstRow = -1;
    int             nCopied = 0;
    QString         szClipBuffer;
    QStringList     lstRecordFields;
    QClipboard      *clipboard = QApplication::clipboard();
    QXmlStreamWriter xmlBuffer(&szClipBuffer);
    // QStringList     lstFields;

    // Check Selection
    if (selection.count() <= 0)
        return 0;
    m_fCutOrPaste = true;
    // Compile Selected Row List
    xmlBuffer.setAutoFormatting(true);
    xmlBuffer.writeStartDocument();
    xmlBuffer.writeStartElement(szXMLCTENAME);
    xmlBuffer.writeAttribute(szXMLCTEVERSION, szVERSION);
    xmlBuffer.writeAttribute(szXMLMODELTAG, m_szCurrentModel);
    xmlBuffer.writeAttribute(szXMLCTNUMROWS, QString::number(selection.count()));
    for (nCur = 0; nCur < selection.count(); nCur++)  {
        // Reperisce l'Item Row Number dall'elenco degli elementi selezionati
        QModelIndex index = selection.at(nCur);
        nRow = index.row();
        // Remember first row of selection
        if (nFirstRow < 0)
            nFirstRow = nRow;
        // Convert Record to Strings List
        recCT2List(lstRecordFields, nRow);
        // Add Element to XML
        xmlBuffer.writeStartElement(szXMLCTROW);
        // Source Row
        xmlBuffer.writeAttribute(szXMLCTSOURCEROW, QString::number(nRow + 1));
        // Destination Row marked as free
        xmlBuffer.writeAttribute(szXMLCTDESTROW, QString::number(-1));
        for (nCol = 0; nCol < lstRecordFields.count(); nCol++)  {
            xmlBuffer.writeAttribute(lstHeadNames[nCol], lstRecordFields[nCol]);
        }
        nCopied++;
        xmlBuffer.writeEndElement();
    }
    xmlBuffer.writeEndElement();
    xmlBuffer.writeEndDocument();
    clipboard->setText(szClipBuffer);
    // If only Copy, Set Current index Row to first of Selection
    if (fClearSelection)  {
        ui->tblCT->selectionModel()->clearSelection();
        if (nFirstRow >= 0)  {
            jumpToGridRow(nFirstRow, true);
            // recCT2List(lstFields, nFirstRow);
            // values2Iface(lstFields);
        }
    }
    selection.clear();
    m_szMsg = tr("Rows Copied: %1") .arg(nCopied);
    displayStatusMessage(m_szMsg);
    enableInterface();
    // Return value
    return nCopied;
}
void ctedit::pasteSelected()
// Incolla righe da Buffer di copiatura a Riga corrente
{
    int             nRow = ui->tblCT->currentRow();
    int             nCur = 0;
    int             nPasted = 0;
    bool            fUsed = false;
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
    // Gestione della ClipBoard
    QClipboard      *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QString         szClipBuffer;
    bool            fClipSourceOk = false;
    QList<QStringList > lstPastedRecords;
    QList<int>      lstDestRows;
    QList<int>      lstSourceRows;

    // Retrieve Clipboard object
    szClipBuffer.clear();   
    if (mimeData->hasText())  {
        szClipBuffer = clipboard->text();
    }
    // Check Clipboard object
    if (! szClipBuffer.isEmpty()) {
        fClipSourceOk = getRowsFromXMLBuffer(szClipBuffer, lstPastedRecords, lstSourceRows, lstDestRows);
    }
    // No Records copied
    fUsed = false;
    if (! fClipSourceOk)  {
        m_szMsg = tr("Can't Copy as Copy Buffer is Empty or Invalid");
        fUsed = true;
    }
    // Paste in System Area
    else if (m_nGridRow >= MAX_NONRETENTIVE - 1)  {
        m_szMsg = tr("Can't paste rows in System Area");
        fUsed = true;
    }
    else if(m_nGridRow + lstPastedRecords.count() > MAX_NONRETENTIVE - 1)  {
        m_szMsg = tr("Pasted Rows could overlap to System Area");
        fUsed = true;
    }
    // Error detected
    if (fUsed)  {
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    m_fCutOrPaste = true;
    // Paste Rows
    qDebug() << tr("Pasted Rows Count: %1") .arg(lstPastedRecords.count());
    if (fClipSourceOk)  {
        if (nRow + lstPastedRecords.count() < MAX_NONRETENTIVE)  {
            // Verifica che la destinazione delle righe sia libera
            for (nCur = nRow; nCur < nRow + lstPastedRecords.count(); nCur++)  {
                if (lstCTRecords[nCur].UsedEntry)  {
                    fUsed = true;
                    break;
                }
            }
            // Query confirm of used if any row is used
            if (fUsed)  {
                m_szMsg = tr("Some of destination rows may be used. Paste anyway ?");
                fUsed = ! queryUser(this, szMectTitle, m_szMsg);
            }
            // No row used or overwrite confirmed
            if (! fUsed)  {
                nPasted = addRowsToCT(nRow, lstPastedRecords, lstDestRows);
            }
        }
        else  {
            m_szMsg = tr("The Copy Buffer Excedes System Variables Limit. Rows not copied");
            warnUser(this, szMectTitle, m_szMsg);
        }
        m_szMsg = tr("Rows Pasted: %1") .arg(nPasted);
    }
    else {
        m_szMsg = tr("Error Pasting Rows: %1") .arg(lstPastedRecords.count());
    }
    displayStatusMessage(m_szMsg);
    enableInterface();
}

void ctedit::insertRows()
// Aggiunta righe in posizione cursore
{
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
    CrossTableRecord emptyRecord;
    int             nInserted = 0;
    int             nFirstRow = -1;
    int             nRow = 0;
    int             nCur = 0;
    int             nStart = 0;
    int             nCurPos = 0;

    if (selection.count() <= 0 )  {
        return;
    }
    // Controllo di restare nei Bounding delle variabili utente
    nCurPos = m_nGridRow;
    if (nCurPos + selection.count() < MAX_NONRETENTIVE - 1)  {
        // Append to Undo List
        lstUndo.append(lstCTRecords);
        // Enter in Paste Mode
        m_fCutOrPaste = true;
        // Ricerca del Punto di Inserzione
        for (nCur = 0; nCur < selection.count(); nCur++)  {
            // Reperisce l'Item Row Number dall'elenco degli elementi selezionati
            QModelIndex index = selection.at(nCur);
            nRow = index.row();
            // Remember first row of selection
            if (nFirstRow < 0)  {
                nFirstRow = nRow;
                break;
            }
        }
        // Insert New Recs
        for (nCur = 0; nCur < selection.count(); nCur++)  {
            lstCTRecords.insert(nFirstRow, emptyRecord);
            freeCTrec(nFirstRow);
            nInserted ++;
        }
        // Search the right place 2 remove extra records
        if (nFirstRow >= 0 && nFirstRow < MAX_RETENTIVE - 1)
            nStart = MAX_RETENTIVE - 1;
        else if (nFirstRow >= MIN_NONRETENTIVE -1 && nFirstRow < MAX_NONRETENTIVE - 1)
            nStart = MAX_NONRETENTIVE - 1;
        else    // Should never happen....
            nStart = 0;
        // Remove extra Rec to readjust positions
        for (nCur = 0; nCur < nInserted; nCur++)  {
            lstCTRecords.removeAt(nStart);
        }
        // Refresh Grid
        ctable2Grid();
        m_isCtModified = true;
    }
    else  {
        m_szMsg = tr("Too Many Rows selected, passed limit of User Variables!");
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    m_szMsg = tr("Rows Inserted: %1") .arg(selection.count());
    displayStatusMessage(m_szMsg);
    if (nCurPos >= 0)
        jumpToGridRow(nCurPos, true);
    enableInterface();
}
void ctedit::emptySelected()
// Cancellazione delle righe correntemente selezionate
// (solo svuotamento senza Shift in alto delle righe)
{
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
    int             nRow = 0;
    int             nCur = 0;
    int             nRemoved = 0;
    int             nFirstRow = -1;

    // Check Modif. and append data to Undo List
    if (selection.isEmpty() || m_nGridRow >= MIN_DIAG - 1)  {
        m_szMsg = tr("Can't remove rows in System Area");
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    lstUndo.append(lstCTRecords);
    // Compile Selected Row List
    m_fCutOrPaste = true;
    for (nCur = 0; nCur < selection.count(); nCur++)  {
        // Reperisce l'Item Row Number dall'elenco degli elementi selezionati
        QModelIndex index = selection.at(nCur);
        nRow = index.row();
        // Remember first row of selection
        if (nFirstRow < 0)
            nFirstRow = nRow;
        // Free Row
        if (nRow < MAX_NONRETENTIVE)  {
            freeCTrec(nRow);
            nRemoved++;
        }
    }
    // Set Current index Row to first of Selection
    selection.clear();
    // Refresh Grid
    if (nRemoved)  {
        ctable2Grid();
        m_isCtModified = true;
    }
    // Riposiziona alla riga corrente
    if (nRemoved > 0 && nFirstRow >= 0)  {
        jumpToGridRow(nFirstRow, true);
    }
    m_szMsg = tr("Rows Removed: %1") .arg(nRemoved);
    displayStatusMessage(m_szMsg);
    // Update Iface
    enableInterface();
}

void ctedit::removeSelected()
// Rimozione delle righe correntemente selezionate
// (svuotamento con Shift in alto delle righe)
{
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
    int             nRow = 0;
    int             nCur = 0;
    int             nStart = 0;
    int             nRemoved = 0;
    int             nFirstRow = -1;
    CrossTableRecord emptyRecord;

    // Check Modif. and append data to Undo List
    if (selection.isEmpty() || m_nGridRow >= MIN_DIAG - 1)  {
        m_szMsg = tr("Can't remove rows in System Area");
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    lstUndo.append(lstCTRecords);
    // Compile Selected Row List
    m_fCutOrPaste = true;
    for (nCur = 0; nCur < selection.count(); nCur++)  {
        // Reperisce l'Item Row Number dall'elenco degli elementi selezionati
        QModelIndex index = selection.at(nCur);
        nRow = index.row();
        // Remember first row of selection
        if (nFirstRow < 0)
            nFirstRow = nRow;
        // Removing Row from list
        if (nRow < MAX_NONRETENTIVE)  {
            lstCTRecords.removeAt(nRow - nRemoved);
            nRemoved++;
        }
    }
    // Refresh Grid
    if (nRemoved)  {
        // Search the right place 2 insert empty records
        if (nFirstRow >= 0 && nFirstRow < MAX_RETENTIVE - 1)
            nStart = MAX_RETENTIVE - nRemoved - 1;
        else if (nFirstRow >= MIN_NONRETENTIVE -1 && nFirstRow < MAX_NONRETENTIVE - 1)
            nStart = MAX_NONRETENTIVE - nRemoved - 1;
        else    // Should never happen....
            nStart = 0;
        // Insert empty Rec to readjust positions
        for (nCur = 0; nCur < nRemoved; nCur++)  {
            lstCTRecords.insert(nStart, emptyRecord);
            freeCTrec(nStart);
        }
        // Refresh Grid
        ctable2Grid();
        m_isCtModified = true;
    }
    m_szMsg = tr("Rows Removed: %1") .arg(nRemoved);
    displayStatusMessage(m_szMsg);
    // Update Iface
    enableInterface();
}
void ctedit::cutSelected()
// Taglia righe in Buffer di copiatura
{
    int nCopied = 0;
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();

    if (m_nGridRow >= MIN_DIAG - 1)  {
        m_szMsg = tr("Can't remove rows in System Area");
        selection.clear();
        displayStatusMessage(m_szMsg);
        return;
    }
    m_fCutOrPaste = true;
    // Copia Righe
    nCopied = copySelected(false);
    // Elimina Righe
    if(nCopied > 0)
        emptySelected();
    // Result
    m_szMsg = tr("Rows Cutted: %1") .arg(nCopied);
    displayStatusMessage(m_szMsg);
    // qDebug() << m_szMsg;
}
bool ctedit::isFormEmpty()
// Controllo Form Editing vuoto
{
    int nFilled = 0;

    nFilled += (ui->cboPriority->currentIndex() >= 0);
    nFilled += (ui->cboUpdate->currentIndex() >= 0);
    nFilled += (! ui->txtName->text().trimmed().isEmpty());
    nFilled += (ui->cboType->currentIndex() >= 0);
    nFilled += (! ui->txtDecimal->text().trimmed().isEmpty());
    nFilled += (ui->cboProtocol->currentIndex() >= 0);
    nFilled += (! ui->txtIP->text().trimmed().isEmpty());
    nFilled += (! ui->txtPort->text().trimmed().isEmpty());
    nFilled += (! ui->txtNode->text().trimmed().isEmpty());
    nFilled += (! ui->txtRegister->text().trimmed().isEmpty());
    // nFilled += (ui->txtBlock->text().trimmed() != ui->tblCT->item(m_nGridRow, colBlock)->text().trimmed());
    // nFilled += (ui->txtBlockSize->text().trimmed() != ui->tblCT->item(m_nGridRow, colBlockSize)->text().trimmed());
    nFilled += (! ui->txtComment->text().trimmed().isEmpty());
    nFilled += (ui->cboBehavior->currentIndex() >= 0);
    if (ui->cboBehavior->currentIndex() >= behavior_alarm)  {
        nFilled += (ui->cboVariable1->currentIndex() >= 0);
        nFilled += (ui->cboCondition->currentIndex() >= 0);
        nFilled += (ui->cboVariable2->currentIndex() >= 0);
        nFilled += (! ui->txtFixedValue->text().trimmed().isEmpty());
    }
    // Return Value
    // qDebug() << "isFormEmpty(): N.Row:" << m_nGridRow << "Campi definiti:" << nFilled;
    return (nFilled == 0);
}

bool ctedit::isLineModified(int nRow)
// Check se linea corrente Grid è diversa da Form in Editing
{
    int     nModif = 0;
    int     nProtocol = ui->cboProtocol->currentIndex();

    // Confronto tra Form Editing e riga Grid
    if(nRow >= 0 && nRow < lstCTRecords.count())  {
        nModif += (ui->cboPriority->currentText().trimmed() != ui->tblCT->item(nRow, colPriority)->text().trimmed());
        nModif += (ui->cboUpdate->currentText().trimmed() != ui->tblCT->item(nRow, colUpdate)->text().trimmed());
        nModif += (ui->txtName->text().trimmed() != ui->tblCT->item(nRow, colName)->text().trimmed());
        nModif += (ui->cboType->currentText().trimmed() != ui->tblCT->item(nRow, colType)->text().trimmed());
        nModif += (ui->txtDecimal->text().trimmed() != ui->tblCT->item(nRow, colDecimal)->text().trimmed());
        nModif += (ui->cboProtocol->currentText().trimmed() != ui->tblCT->item(nRow, colProtocol)->text().trimmed());
        nModif += (ui->txtIP->text().trimmed() != ui->tblCT->item(nRow, colIP)->text().trimmed());
        if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)
            nModif += ui->cboPort->currentText().trimmed() != ui->tblCT->item(nRow, colPort)->text().trimmed();
        else
            nModif += (ui->txtPort->text().trimmed() != ui->tblCT->item(nRow, colPort)->text().trimmed());
        nModif += (ui->txtNode->text().trimmed() != ui->tblCT->item(nRow, colNodeID)->text().trimmed());
        nModif += (ui->txtRegister->text().trimmed() != ui->tblCT->item(nRow, colRegister)->text().trimmed());
        // nModif += (ui->txtBlock->text().trimmed() != ui->tblCT->item(nRow, colBlock)->text().trimmed());
        // nModif += (ui->txtBlockSize->text().trimmed() != ui->tblCT->item(nRow, colBlockSize)->text().trimmed());
        nModif += (ui->txtComment->text().trimmed() != ui->tblCT->item(nRow, colComment)->text().trimmed());
        nModif += (ui->cboBehavior->currentText().trimmed() != ui->tblCT->item(nRow, colBehavior)->text().trimmed());
        // Frame Allarmi
        if (ui->cboBehavior->currentIndex() >= behavior_alarm)  {
            nModif += (ui->cboVariable1->currentText().trimmed() != ui->tblCT->item(nRow, colSourceVar)->text().trimmed());
            nModif += (ui->cboCondition->currentText().trimmed() != ui->tblCT->item(nRow, colCondition)->text().trimmed());
            if (ui->optFixedVal->isChecked())
                nModif += (ui->txtFixedValue->text().trimmed() != ui->tblCT->item(nRow, colCompare)->text().trimmed());
            else
                nModif += (ui->cboVariable2->currentText().trimmed() != ui->tblCT->item(nRow, colCompare)->text().trimmed());
        }
    }
    // qDebug() << "Modified(): N.Row:" << nRow << "Numero Modifiche:" << nModif;
    return (nModif > 0);
}

void ctedit::on_cmdImport_clicked()
// Import another Cross Table File
{
    QString szSourceFile;
    QString szMsg;
    QList<CrossTableRecord>  lstNewRecs, lstSourceRecs;
    int     nRow = 0;
    bool    fRes = false;

    szSourceFile = QFileDialog::getOpenFileName(this, tr("Import From Crosstable File"), m_szCurrentCTFile, tr("Crosstable File (%1)") .arg(szCT_FILE_NAME));
    if (! szSourceFile.isEmpty())  {
        if (checkCTFile(szSourceFile))  {
            szMsg = tr("Rows from %1 to %2 will be overwritten !!\nDo you want to continue?") .arg(MIN_RETENTIVE) .arg(MAX_NONRETENTIVE);
            if (queryUser(this, szMectTitle, szMsg))  {
                // Copia di Salvataggio
                lstUndo.append(lstCTRecords);
                lstNewRecs.clear();
                lstSourceRecs.clear();
                // Caricamento della nuova Cross Table
                if (loadCTFile(szSourceFile, lstNewRecs, false))  {
                    // Aggiunta alla Cross Table dei record letti dalla nuova CT
                    for (nRow = 0; nRow < MAX_NONRETENTIVE; nRow++)  {
                        lstSourceRecs.append(lstCTRecords[nRow]);
                        lstCTRecords[nRow] = lstNewRecs[nRow];
                    }
                    // Ricarica la lista dei dati CT in Grid
                    fRes = ctable2Grid();
                    if (fRes)  {
                        szMsg = tr("Loaded Crosstable from file:\n%1") .arg(szSourceFile);
                        notifyUser(this, szMectTitle, szMsg);
                    }
                    else {
                        szMsg = tr("Error Loading Crosstable from file:\n%1") .arg(szSourceFile);
                        szMsg.append(tr("\nOriginal Content Reloaded"));
                        warnUser(this, szMectTitle, szMsg);
                        for (nRow = 0; nRow < lstSourceRecs.count(); nRow++)  {
                            lstCTRecords[nRow] = lstSourceRecs[nRow];
                        }
                        fRes = ctable2Grid();
                    }
                    m_isCtModified = true;
                }
            }
        }
        else  {
            szMsg = tr("The Selected file is not a Crosstable file:\n%1") .arg(szSourceFile);
            warnUser(this, szMectTitle, szMsg);
        }
    }
}
void ctedit::setRowColor(int nRow, int nAlternate)
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
    // Impostazione del colore di sfondo
    QBrush bCell(cSfondo, Qt::SolidPattern);
    setRowBackground(bCell, ui->tblCT->model(), nRow);
    /*for (nCol = 0; nCol < colTotals; nCol++)  {
        tItem = ui->tblCT->item(nRow, nCol);
        tItem->setData(Qt::BackgroundRole, bCell);
        // tItem->setBackground(bCell);
    } */
}

void ctedit::showAllRows(bool fShowAll)
// Visualizza o nascondi tutte le righe
{
    int         nAlternate = 0;
    int         nFirstVisible = -1;
    int         nRow = 0;
    int16_t     nPrevBlock = -1;

    // qDebug() << tr("showAllRows: showAll = %1 Current Row = %2") .arg(fShowAll) .arg(m_nGridRow);
    // Items del Grid    
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Determina se il blocco corrente è cambiato dal precedente
        if (lstCTRecords[nRow].UsedEntry)  {
            if (nPrevBlock != lstCTRecords[nRow].Block)
                nAlternate = ((nAlternate + 1) % 2);
            nPrevBlock = lstCTRecords[nRow].Block;
        }
        else
            nPrevBlock = -1;
        // Impostazione colore riga
        setRowColor(nRow, nAlternate);
        // Mostra o nascondi le righe se sono abilitate
        // Ricerca il primo Item visibile
        if (lstCTRecords[nRow].UsedEntry && nFirstVisible < 0)  {
            nFirstVisible = nRow ;
            // qDebug() << tr("First Visible Row:%1") .arg(nFirstVisible);
        }
        // Mostra tutti
        if (fShowAll)  {
            ui->tblCT->showRow(nRow);
        }
        // Nascondi i non abilitati
        else  {
            // Mostra o nascondi le righe se sono abilitate
            if (lstCTRecords[nRow].UsedEntry)  {
                ui->tblCT->showRow(nRow);
            }
            else  {
                ui->tblCT->hideRow(nRow);
            }
        }
    }
    // Riga corrente non definita
    if (m_nGridRow < 0 || m_nGridRow >= ui->tblCT->rowCount())  {
        // Se non ci sono variabili utente definite, prova a saltare a Prima di Local I/O
        if (nFirstVisible == MIN_DIAG - 1)  {
            nFirstVisible = MIN_LOCALIO -1;
            if (! lstCTRecords[MIN_LOCALIO -1].UsedEntry)  {
                nFirstVisible = MIN_SYSTEM - 1;
                if (! lstCTRecords[MIN_SYSTEM -1].UsedEntry)  {
                    nFirstVisible = MIN_DIAG - 1;
                }
            }
        }
        m_nGridRow = nFirstVisible >= 0 ? nFirstVisible : 0;
    }
    else  {
        // Riga corrente piena ?
        if (lstCTRecords[m_nGridRow].UsedEntry == 0)
            m_nGridRow = nFirstVisible;
    }
    // Trucco per centrare la riga...
    nRow = m_nGridRow;
    jumpToGridRow(0, true);
    if (nRow >= 0 && nRow < DimCrossTable)
        jumpToGridRow(nRow, true);
    // ui->tblCT->setFocus();
}
void ctedit::gotoRow()
// Show Dialog Goto Row n
{
    // QStringList lstFields;
    bool fOk;

    // Valori da interfaccia a Lista Stringhe
    // fOk = iface2values(lstFields);
    // Controllo di coerenza sulla riga corrente
    // if (checkFormFields(m_nGridRow, lstFields, true) > 0)
    //     return;
    // Input Dialog per Numero riga
    int nRow = QInputDialog::getInt(this, tr("Row to Jump To"),
                                 tr("Enter Row Number to Jump to:"), m_nGridRow + 1, 1, DimCrossTable, 1, &fOk);
    if (fOk)  {
        nRow--;
        // Se la riga non è visibile chiede conferma se fare ShowAll
        if (lstCTRecords[nRow].UsedEntry == 0 && ! m_fShowAllRows)  {
            m_szMsg = trUtf8("The requested row is not visible. Show all rows?");
            if (queryUser(this, szMectTitle, m_szMsg, false))  {
                ui->cmdHideShow->setChecked(true);
            }
        }
        jumpToGridRow(nRow, true);
    }
}
void ctedit::on_cmdSearch_clicked()
// Search Variable by Name
{
    bool fOk;
    int  nRow = 0;
    // QStringList lstFields;

    // Valori da interfaccia a Lista Stringhe
    // fOk = iface2values(lstFields);
    // if (checkFormFields(m_nGridRow, lstFields, true) > 0)
    //     return;
    // Input Dialog per Nome Variabile
    QString szText;

    szText.clear();
    szText = QInputDialog::getItem(this, tr("Variable Name"),
                                            tr("Enter Variable Name:"), lstUsedVarNames, 0, true, &fOk, Qt::Dialog);
    if (fOk)  {
        // Ricerca sequenziale della stringa
        for (nRow = 0; nRow < lstCTRecords.count(); nRow++)    {
            QString szVarName = QString::fromAscii(lstCTRecords[nRow].Tag);
            if (QString::compare(szText, szVarName, Qt::CaseSensitive) == 0)  {
                break;
            }
        }
        // Item Found
        if (nRow < lstCTRecords.count()) {
            jumpToGridRow(nRow, true);
        }
    }
}
void ctedit::jumpToGridRow(int nRow, bool fCenter)
// Salto alla riga nRow del Grid
{
    // Seleziona la riga nRow
    ui->tblCT->selectRow(nRow);
    // Se vero il flag fCenter, centra la riga selezionata rispetto alla finestra di scroll
    if (fCenter)
        ui->tblCT->scrollToItem(ui->tblCT->currentItem(), QAbstractItemView::PositionAtCenter);
    ui->tblCT->setFocus();
    m_nGridRow = nRow;
}

void ctedit::on_cmdCompile_clicked()
// Generate Compiled Files
{
    QString     szCommand = QCoreApplication::applicationDirPath();     // QT Creator Bin Directory
    QStringList lstArguments;
    QString     szTemp;
    QString     szFileName;
    QByteArray  baCompErr;
    QString     szCompErr;
    QProcess    procCompile;
    int         nExitCode = 0;

    // Controllo presenza di Errori
    if (globalChecks())
        return;
    // CT Compiler Full Path
    szCommand.append(szSLASH);
    szCommand.append(szCrossCompier);
    // Parametro 1: -c Nome del File sorgente CrossTable
    szTemp = QString::fromAscii("-c%1.csv") .arg(m_szCurrentCTName);
    lstArguments.append(szTemp);
    // Parametro 2: -g Nome del file GVL (Case preserved)
    szFileName = QString::fromAscii("%1.gvl") .arg(m_szCurrentCTName);
    szFileName.prepend(szSLASH);
    szFileName.prepend(m_szCurrentPLCPath);
    szTemp = szFileName;
    szTemp.prepend(QString::fromAscii("-g"));
    lstArguments.append(szTemp);
    fileBackUp(szFileName);
    // Parametro 3: -i Nome del file .h (Forced LowerCase)
    szFileName = QString::fromAscii("%1.h") .arg(m_szCurrentCTName.toLower());
    szTemp = szFileName;
    szTemp.prepend(QString::fromAscii("-i"));
    lstArguments.append(szTemp);
    szFileName.prepend(m_szCurrentCTPath);
    fileBackUp(szFileName);
    // Parametro 4: -s Nome del file .h (Forced LowerCase)
    szFileName = QString::fromAscii("%1.cpp") .arg(m_szCurrentCTName.toLower());
    szTemp = szFileName;
    szTemp.prepend(QString::fromAscii("-s"));
    lstArguments.append(szTemp);
    szFileName.prepend(m_szCurrentCTPath);
    fileBackUp(szFileName);
    // Imposta come Directory corrente di esecuzione la directory del File CT
    procCompile.setWorkingDirectory(m_szCurrentCTPath);
    // Esecuzione Comando
    // qDebug() << szCommand << lstArguments;
    procCompile.start(szCommand, lstArguments);
    if (!procCompile.waitForStarted())  {
        m_szMsg = tr("Error Starting Crosstable Compiler!\n");
        m_szMsg.append(szCommand);
        warnUser(this, szMectTitle, m_szMsg);
        goto exit_compile;
    }
    // Attesa termine comando
    if (!procCompile.waitForFinished())  {
        m_szMsg = tr("Error Running Crosstable Compiler!\n");
        m_szMsg.append(szCommand);
        warnUser(this, szMectTitle, m_szMsg);
        goto exit_compile;
    }
    // Esito comando
    baCompErr = procCompile.readAllStandardError();
    nExitCode = procCompile.exitCode();
    if (nExitCode != 0)  {
        m_szMsg = tr("Exit Code of Crosstable Compiler: %1\n") .arg(nExitCode);
        szCompErr = QString::fromAscii(baCompErr.data());
        m_szMsg.append(szCompErr);
        warnUser(this, szMectTitle, m_szMsg);
        // TODO: Analisi errore del Cross Compiler
    }
    else {
        m_szMsg = tr("Crosstable Correctly Compiled");
        notifyUser(this, szMectTitle, m_szMsg);
    }

exit_compile:
    return;
}
QString ctedit::getModelName()
// Lettura del file template.pri per determinare il modello di TPAC
{
    QString     szModel;
    QString     szFileTemplate;
    QFile       fileTemplate;
    QString     szLine;
    int         nPos = -1;

    szModel.clear();
    // Costruzione del nome del file Template
    szFileTemplate = m_szCurrentProjectPath;
    szFileTemplate.append(szSLASH);
    szFileTemplate.append(szTemplateFile);
    fileTemplate.setFileName(szFileTemplate);
    // Verifica esistenza filedisplayMessage
    if (fileTemplate.exists())   {
        fileTemplate.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream templateFile(&fileTemplate);
        // Text reading line by line
        while (! templateFile.atEnd()) {
            szLine = templateFile.readLine();
            szLine = szLine.trimmed();
            // Search TYPE string in Line
            if (szLine.startsWith(QString::fromAscii("TYPE"), Qt::CaseSensitive))  {
                // qDebug() << tr("Model Line: %1") .arg(szLine);
                nPos = szLine.indexOf(QString::fromAscii("="));
                if (nPos > 0)  {
                    szModel = szLine.mid(nPos + 1).trimmed();
                    szModel.remove(szDOUBLEQUOTE);
                    szModel = szModel.trimmed();
                    break;
                }
            }
        }
        // Close file
        fileTemplate.close();
    }
    else {
        qDebug() << tr("Template File: %1 not found") .arg(szFileTemplate);
    }
    qDebug() << tr("Current Model: <%1>") .arg(szModel);
    // return value
    return szModel;
}
void ctedit::displayStatusMessage(QString szMessage, int nSeconds)
// Show message in ui->lblMessage
{

    if (! szMessage.isEmpty())  {
        // Time Out in display time
        if (nSeconds == 0)
            nSeconds = MAX_DISPLAY_TIME;
        ui->lblMessage->setText(szMessage);
        ui->lblMessage->update();
        // Timer per Clear Text
        if (nSeconds > 0)  {
            tmrMessage->setInterval(nSeconds * 1000);
            tmrMessage->start();
        }
        else
            tmrMessage->stop();
        // Force call to Application Loop
        doEvents();
    }
}
void ctedit::clearStatusMessage()
// Clear message in ui->lblMessage
{
    ui->lblMessage->setText(szEMPTY);
}

void ctedit::on_cmdUndo_clicked()
// Retrieve a CT Block from Undo List
{
    if (! lstUndo.isEmpty())  {
        lstCTRecords.clear();
        lstCTRecords = lstUndo[lstUndo.count()-1];
        lstUndo.removeLast();
        // Refresh List
        ctable2Grid();
        // Force Marker to Updated
        m_isCtModified = true;
        enableInterface();
    }
}
void ctedit::tabSelected(int nTab)
// Change current Tab
{
    // Aggiornamento della lista di variabili e ripopolamento liste per Trends
    if (nTab == TAB_TREND) {
        trendEdit->updateVarLists(lstLoggedVars);
    }
    // Ritorno a CT da altro Tab, prudenzialmente aggiorna le info di configurazione
    if (nTab == TAB_CT)  {
        // Rilegge all'indetro le info di configurazione eventualmente aggiornate da system.ini
        mectSet->getTargetConfig(TargetConfig);
        // Aggiorna le abilitazioni dei protocolli in funzione delle porte abilitate
        enableProtocolsFromModel();
    }
    // Set Current Tab
    m_nCurTab = nTab;
}
void ctedit::enableInterface()
// Abilita l'interfaccia in funzione dello stato del sistema
{
    // Abilitazioni elementi di interfaccia ancora da decidere
    ui->cmdUndo->setEnabled(lstUndo.count() > 0);
    ui->cmdBlocchi->setEnabled(true);
    ui->cmdCompile->setEnabled(! m_isCtModified && ! m_szCurrentModel.isEmpty());
    // Salva sempre abilitato, bordo green se non ci sono salvataggi pendenti
    ui->cmdSave->setEnabled(! m_szCurrentModel.isEmpty());
    if (m_isCtModified)  {
        ui->cmdSave->setStyleSheet(QString::fromAscii("border: 2px solid red;"));
    }
    else  {
        ui->cmdSave->setStyleSheet(QString::fromAscii("border: 2px solid green;"));
    }
    ui->cmdPLC->setEnabled(! m_isCtModified && ! m_szCurrentModel.isEmpty());
    ui->fraCondition->setEnabled(true);
    ui->tblCT->setEnabled(true);
    m_fCutOrPaste = false;
}
void    ctedit::enableProtocolsFromModel()
// Abilita i Protocolli in funzione della configurazione del Modello corrente (da TargetConfig)
{
    int nCur = 0;
    QString szPort;


    // qDebug() << tr("Model Searched: %1 - Found: %2") .arg(szModel) .arg(nModel);

    lstBusEnabler.clear();
    // Abilita di default tutti i Protocolli
    for (nCur = 0; nCur < lstProtocol.count(); nCur++)  {
        lstBusEnabler.append(false);
    }
    // PLC abilitato per tutti i modelli
    lstBusEnabler[PLC] = true;
    // TCP e derivati abilitati per tutti i modelli (Perchè ethPorts è almeno 1)
    if (TargetConfig.ethPorts > 0)  {
        lstBusEnabler[TCP] = true;
        lstBusEnabler[TCPRTU] = true;
        lstBusEnabler[TCP_SRV] = true;
        lstBusEnabler[TCPRTU_SRV] = true;
    }
    // Protocollo CAN abilitato solo per Modelli con Can
    if (TargetConfig.can0_Enabled || TargetConfig.can1_Enabled)  {
        lstBusEnabler[CANOPEN] = true;
    }
    // Protocolli Seriali
    if (TargetConfig.ser0_Enabled || TargetConfig.ser1_Enabled || TargetConfig.ser2_Enabled || TargetConfig.ser3_Enabled)  {
        lstBusEnabler[RTU] = true;
        lstBusEnabler[MECT_PTC] = true;
        lstBusEnabler[RTU_SRV] = true;
        // Abilitazione delle entry nella Combo delle Porte
        // Port 0
        if (TargetConfig.ser0_Enabled)
            enableComboItem(ui->cboPort, 0);
        else
            disableComboItem(ui->cboPort, 0);
        // Port 1
        if (TargetConfig.ser1_Enabled)
            enableComboItem(ui->cboPort, 1);
        else
            disableComboItem(ui->cboPort, 1);
        // Port 2
        if (TargetConfig.ser2_Enabled)
            enableComboItem(ui->cboPort, 2);
        else
            disableComboItem(ui->cboPort, 2);
        // Port 3
        if (TargetConfig.ser3_Enabled)
            enableComboItem(ui->cboPort, 3);
        else
            disableComboItem(ui->cboPort, 3);
    }
    // Spegne sulla Combo dei protocolli le voci non abilitate
    for (nCur = 0; nCur < lstProtocol.count(); nCur++)  {
        if (lstBusEnabler[nCur])
            enableComboItem(ui->cboProtocol, nCur);
        else
            disableComboItem(ui->cboProtocol, nCur);
    }
}
void ctedit::on_cboBehavior_currentIndexChanged(int index)
{
    // Condizione di sicurezza per file CT non aperto
    if (lstCTRecords.count() <= 0)
        return;
    // Abilitazione o meno del frame condizioni allarmi/eventi
    if (index > behavior_readwrite) {
        ui->fraCondition->setVisible(true);
        if (ui->cboVariable1->count() <= 0)  {
            fillComboVarNames(ui->cboVariable1, lstAllVarTypes, lstNoHUpdates);
        }
        if (ui->cboVariable1->count() <= 0)  {
            fillComboVarNames(ui->cboVariable2, lstAllVarTypes, lstNoHUpdates);
        }
        // Imposta almeno uno dei due optionButtons
        if (! ui->optFixedVal->isChecked() && ! ui->optVariableVal->isChecked())
            ui->optFixedVal->setChecked(true);
    }
    else
        ui->fraCondition->setVisible(false);
}
void ctedit::fillErrorMessage(int nRow, int nCol, int nErrCode, QString szVarName, QString szValue, QChar severity, Err_CT *errCt)
{
    errCt->cSeverity = severity;
    errCt->nRow = nRow;
    errCt->nCol = nCol;
    errCt->nCodErr = nErrCode;
    errCt->szErrMessage = lstErrorMessages[nErrCode];
    errCt->szVarName = szVarName;
    errCt->szValue = szValue;
}
int ctedit::globalChecks()
// Controlli complessivi su tutta la CT
{
    int         nRow = 0;
    int         nErrors = 0;
    bool        fRecOk = false;
    QStringList lstFields;
    QString     szTemp;
    Err_CT      errCt;
    // Form per Display Errori
    cteErrorList    *errWindow;

    // Condizione di sicurezza per file CT non aperto
    if (lstCTRecords.count() <= 0)
        return 0;
    // Ripulitura lista errori
    lstCTErrors.clear();
    lstUniqueVarNames.clear();
    // Ciclo Globale su tutti gli Items di CT
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Controlla solo righe utilizzate
        if (lstCTRecords[nRow].Enable)  {
            // Controllo univocità di nome
            szTemp = QString::fromAscii(lstCTRecords[nRow].Tag).trimmed();
            if (lstUniqueVarNames.contains(szTemp, Qt::CaseInsensitive))  {
                fillErrorMessage(nRow, colName, errCTDuplicateName, szTemp, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
            else  {
                lstUniqueVarNames.append(szTemp);
            }
        // Controlli specifici di Riga
        fRecOk = recCT2List(lstFields, nRow);
        if (fRecOk)
            nErrors += checkFormFields(nRow, lstFields, false);
        }
    }
    // Display finestra errore
    if (nErrors)  {
        // qDebug() << "Found Errors:" << nErrors;
        errWindow = new cteErrorList(this, false);
        errWindow->setModal(true);
        errWindow->lstErrors2Grid(lstCTErrors);
        // Accepted == gotoRow
        if (errWindow->exec() == QDialog::Accepted)  {
            nRow = errWindow->currentRow();
            if (nRow >= 0 && nRow < DimCrossTable)
                jumpToGridRow(nRow, true);
        }
        delete errWindow;
    }
    return nErrors;
}
bool ctedit::isValidVarName(QString szName)
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
int ctedit::fillVarList(QStringList &lstVars, QList<int> &lstTypes, QList<int> &lstUpdates)
// Fill sorted List of Variables Names for Types in lstTypes and Update Type in lstUpdates
{
    bool    fTypeFilter = lstTypes.count() > 0;
    bool    fUpdateFilter = lstUpdates.count() > 0;
    int     nRow = 0;
    bool    f2Add = false;
    bool    fUpdateOk = false;

    lstVars.clear();

    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        if (lstCTRecords[nRow].Enable)  {
            // Filter on Var Type
            if (fTypeFilter)  {
                f2Add = lstTypes.indexOf(lstCTRecords[nRow].VarType) >= 0 ? true : false;
            }
            else  {
                f2Add = true;
            }
            // Filter on Update Type
            if (f2Add && fUpdateFilter)  {
                fUpdateOk = lstUpdates.indexOf(lstCTRecords[nRow].Update) >= 0 ? true : false;
            }
            else {
                fUpdateOk = f2Add;
            }

            // If Var is defined and of a correct type, insert in list
            if (f2Add && fUpdateOk)  {
                lstVars.append(QString::fromAscii(lstCTRecords[nRow].Tag));
            }
        }
    }
    // Ordimanento Alfabetico della Lista
    lstVars.sort();
    // Return value
    // qDebug() << "Items Added to List:" << lstVars.count();
    return lstVars.count();
}

int ctedit::fillComboVarNames(QComboBox *comboBox, QList<int> &lstTypes, QList<int> &lstUpdates)
// Caricamento ComboBox con Nomi Variabili filtrate in funzione del Tipo e della Persistenza
{
    QStringList lstVars;
    int         nItem = 0;
    bool        oldState = comboBox->blockSignals(true);

    lstVars.clear();
    comboBox->setCurrentIndex(-1);
    comboBox->clear();
    if (fillVarList(lstVars, lstTypes, lstUpdates) > 0)
    {
        for (nItem = 0; nItem < lstVars.count(); nItem++)  {
            comboBox->addItem(lstVars[nItem]);
        }
        comboBox->setCurrentIndex(-1);
//        // Sorting della Combo Box, non necessario perchè la lista su cui si lavora è già sorted
//        QSortFilterProxyModel* proxy = new QSortFilterProxyModel(comboBox);
//        proxy->setSourceModel(comboBox->model());
//        comboBox->model()->setParent(proxy);
//        comboBox->setModel(proxy);
//        comboBox->model()->sort(0);

    }
    comboBox->blockSignals(oldState);
    return lstVars.count();
}
int ctedit::checkFormFields(int nRow, QStringList &lstValues, bool fSingleLine)
// Controlli formali sulla riga a termine editing o ciclicamente durante controllo globale valori
{
    int         nErrors = 0;
    int         nPos = -1;
    int         nPriority = -1;
    int         nType = -1;
    int         nUpdate = -1;
    int         nVal = 0;
    int         nProtocol = -1;
    int         nPort = -1;
    int         nNodeID = -1;
    int         nRegister = -1;
    varTypes    nTypeVar1 = UNKNOWN;
    bool        fOk = false;
    Err_CT      errCt;
    QString     szTemp;
    QString     szVarName;
    QString     szIP;
    QString     szVar1;

    // Form per Display Errori
    cteErrorList    *errWindow;

    // Clear Error List if single line show
    if (fSingleLine)
        lstCTErrors.clear();
    // Recupero Variable Name (Per finestra errore)
    szVarName = lstValues[colName];
    //---------------------------------------
    // Controllo cboPriority
    //---------------------------------------
    szTemp = lstValues[colPriority];
    nPriority = szTemp.isEmpty() ? -1 : lstPriority.indexOf(szTemp);
    if (nPriority < 0)  {
        fillErrorMessage(nRow, colPriority, errCTNoPriority, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    //---------------------------------------
    // Controllo Update
    //---------------------------------------
    szTemp = lstValues[colUpdate];
    nUpdate = szTemp.isEmpty() ? -1 : lstUpdateNames.indexOf(szTemp);
    if (nUpdate < 0)  {
        fillErrorMessage(nRow, colUpdate, errCTNoUpdate, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    //---------------------------------------
    // Controllo Variable Name
    //---------------------------------------
    if (! isValidVarName(szVarName))  {
        fillErrorMessage(nRow, colName, errCTNoName, szVarName, szVarName, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    //---------------------------------------
    // Controllo Type
    //---------------------------------------
    szTemp = lstValues[colType];
    nType = szTemp.isEmpty() ? -1 : lstTipi.indexOf(szTemp);
    if (nType < 0)  {
        fillErrorMessage(nRow, colType, errCTNoType, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    //---------------------------------------
    // Controllo Decimal
    //---------------------------------------
    szTemp = lstValues[colDecimal];
    if (szTemp.isEmpty())  {
        fillErrorMessage(nRow, colDecimal, errCTNoDecimals, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    else  {
        // Numero Decimali
        nVal = szTemp.toInt(&fOk);
        nVal = fOk ? nVal : 0;
        // Decimali a 0 per tipo Bit
        if (nType == BIT && nVal > 0)  {
            fillErrorMessage(nRow, colDecimal, errCTNoDecimalZero, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        else if (nType == BYTE_BIT) {
            if (nVal < 1 or nVal > 8)  {
                fillErrorMessage(nRow, colDecimal, errCTWrongDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        else if (nType == WORD_BIT) {
            if (nVal < 1 or nVal > 16)  {
                fillErrorMessage(nRow, colDecimal, errCTWrongDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        else if (nType == DWORD_BIT) {
            if (nVal < 1 or nVal > 32)  {
                fillErrorMessage(nRow, colDecimal, errCTWrongDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        // Numero Decimali > 4 ===> Variable (per tipi differenti da Bit in tutte le versioni possibili)
        else if (nVal >= 4 && nType != BYTE_BIT && nType == WORD_BIT && nType == DWORD_BIT)  {
            // Controlla che il numero indicato punti ad una variabile del tipo necessario a contenere il numero di decimali
            if (nVal > DimCrossTable || ! lstCTRecords[nVal - 1].Enable ||
                    (lstCTRecords[nVal - 1].VarType != UINT8 &&  lstCTRecords[nVal - 1].VarType != UINT16 && lstCTRecords[nVal - 1].VarType != UINT16BA &&
                     lstCTRecords[nVal - 1].VarType != UDINT &&  lstCTRecords[nVal - 1].VarType != UDINTDCBA && lstCTRecords[nVal - 1].VarType != UDINTCDAB &&
                     lstCTRecords[nVal - 1].VarType != UDINTBADC ) )   {
                fillErrorMessage(nRow, colDecimal, errCTNoVarDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
    }
    //---------------------------------------
    // Controllo Protocol
    //---------------------------------------
    szTemp = lstValues[colProtocol];
    nProtocol = szTemp.isEmpty() ? -1 : lstProtocol.indexOf(szTemp);
    if (nProtocol < 0)  {
        fillErrorMessage(nRow, colProtocol, errCTNoProtocol, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    // Tipo BIT non permesso per protocolli Server
    if (nType == BIT &&
            (nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV || nProtocol == RTU_SRV))  {
        fillErrorMessage(nRow, colProtocol, errCTNoBITAllowed, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    //---------------------------------------
    // Controllo Ip Address
    //---------------------------------------
    szIP = lstValues[colIP].trimmed();
    if (nProtocol == TCP || nProtocol == TCPRTU || nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV)  {
        if (szIP.isEmpty())  {
            fillErrorMessage(nRow, colIP, errCTNoIP, szVarName, szIP, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // IP Valido e diverso da 0 se non server
        uint32_t ipNum = str2ipaddr(szIP.toAscii().data());
        QHostAddress ipAddr;
        if ((ipNum == 0 && (nProtocol != TCP_SRV && nProtocol != TCPRTU_SRV)) || ! ipAddr.setAddress(szIP))  {
            fillErrorMessage(nRow, colIP, errCTBadIP, szVarName, szIP, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
    }
    //---------------------------------------
    // Controllo per Port Value in funzione del Protocollo
    //---------------------------------------
    szTemp = lstValues[colPort];
    nPort = szTemp.isEmpty() ? -1 : szTemp.toInt(&fOk);
    nPort = fOk && nPort != -1 ? nPort : -1;
    // Protocolli Seriali errCTNoDevicePort
    if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
        // # Porta fuori Range
        if (nPort < 0 || nPort > nMaxSerialPorts) {
            fillErrorMessage(nRow, colPort, errCTNoPort, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Numero Porta non abilitata (o non presente) [check solo per parte utente della CT...]
        if ( (nRow < MAX_NONRETENTIVE) && (! isValidPort(nPort, nProtocol)))  {
            fillErrorMessage(nRow, colPort, errCTNoDevicePort, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
    }
    // Protocolli TCP
    else if (nProtocol == TCP || nProtocol == TCPRTU || nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV)  {
        // # Porta fuori Range
        if (nPort < 0 || nPort > nMax_Int16)  {
            fillErrorMessage(nRow, colPort, errCTNoPort, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Porta non permessa
        if (! isValidPort(nPort, nProtocol)) {
            fillErrorMessage(nRow, colPort, errCTWrongTCPPort, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
    }
    // Protocollo CAN
    else if (nProtocol == CANOPEN)  {
        // # Porta fuori Range
        if (nPort < 0 || nPort > nMaxCanPorts) {
            fillErrorMessage(nRow, colPort, errCTNoPort, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Numero Porta non abilitata (o non presente) [check solo per parte utente della CT...]
        if ( (nRow < MAX_NONRETENTIVE) && (!isValidPort(nPort, nProtocol))) {
            fillErrorMessage(nRow, colPort, errCTNoDevicePort, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
    }
    //---------------------------------------
    // Controllo per Node ID
    //---------------------------------------
    szTemp = lstValues[colNodeID];
    nNodeID = szTemp.isEmpty() ? -1 : szTemp.toInt(&fOk);
    nNodeID = fOk && nNodeID != -1 ? nNodeID : -1;
    if (nProtocol != PLC)  {
        if (nNodeID < 0 || nNodeID > nMaxNodeID)  {
            fillErrorMessage(nRow, colNodeID, errCTNoNode, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
    }
    //---------------------------------------
    // Controllo per Register
    //---------------------------------------
    szTemp = lstValues[colRegister];
    nRegister = szTemp.isEmpty() ? -1 : szTemp.toInt(&fOk);
    nRegister = fOk && nRegister != -1 ? nRegister : -1;
    if (nProtocol != PLC)  {
        if (nRegister < 0 || nRegister > nMaxRegister)  {
            fillErrorMessage(nRow, colRegister, errCTNoRegister, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
    }
    //---------------------------------------
    // Controllo Behavior
    //---------------------------------------
    szTemp = lstValues[colBehavior];
    nPos = szTemp.isEmpty() ? -1 : lstBehavior.indexOf(szTemp);
    if (nPos < 0 || nPos >= lstBehavior.count())  {
        fillErrorMessage(nRow, colBehavior, errCTNoBehavior, szVarName, szTemp, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    //---------------------------------------
    // Controlli specifici per Allarmi/Eventi
    //---------------------------------------
    if (nPos >= behavior_alarm && nPos <= behavior_event)  {
        // Controllo che la variabile Alarm/Event sia di tipo BIT
        if (nType != BIT)  {
            fillErrorMessage(nRow, colType, errCTNoBit, szVarName, szVarName, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Controllo che la variabile impostata come Alarm/Event abbia priority > 0 e non sia Update=H
        if (nPriority <= 0 || nUpdate <= Htype)  {
            fillErrorMessage(nRow, colPriority, errCTBadPriorityUpdate, szVarName, szVarName, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Controllo che la variabile selezionata come Var1 sia NON H
        QStringList lstAlarmVars;
        fillVarList(lstAlarmVars, lstAllVarTypes, lstNoHUpdates);
        // Variable 1
        szVar1 = lstValues[colSourceVar];
        nPos = szVar1.isEmpty() ? -1 : lstAlarmVars.indexOf(szVar1);
        if (nPos < 0)  {
            fillErrorMessage(nRow, colSourceVar, errCTNoVar1, szVarName, szVar1, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Ricerca della Variabile 1 per estrarre il Tipo
        if (! szVar1.isEmpty())  {
            nPos = varName2Row(szVar1, lstCTRecords);
            if (nPos >= 0 && nPos < lstCTRecords.count())
                nTypeVar1 = lstCTRecords[nPos].VarType;
        }
        // Controllo sull'operatore di comparazione
        szTemp = lstValues[colCondition];
        if (szTemp.isEmpty())  {
            fillErrorMessage(nRow, colCondition, errCTNoCondition, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Controllo sulla parte DX espressione
        nPos = lstCondition.indexOf(szTemp);
        // Operatore Rising/Falling
        if (nPos >= oper_rising && nPos <= oper_falling)  {
            lstValues[colCompare].clear();
            if (! (nTypeVar1 == BIT || nTypeVar1 == BYTE_BIT || nTypeVar1 == WORD_BIT || nTypeVar1 == DWORD_BIT))  {
                fillErrorMessage(nRow, colSourceVar, errCTRiseFallNotBit, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        // Altri Operatori
        else if (nPos >= 0 && nPos < oper_rising)  {
            // Espressione vuota
            szTemp = lstValues[colCompare];
            if (szTemp.isEmpty())  {
                fillErrorMessage(nRow, colCompare, errCTEmptyCondExpression, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
            else  {
                QChar c = szTemp.at(0);
                if (! c.isLetter())  {
                    // Numero
                    bool fOk;
                    double dblVal = szTemp.toDouble(&fOk);
                    if (!fOk)  {
                        // Invalid Number
                        fillErrorMessage(nRow, colCompare, errCTInvalidNum, szVarName, szTemp, chSeverityError, &errCt);
                        lstCTErrors.append(errCt);
                        nErrors++;
                    }
                }
                else  {
                    // Variabile
                    // Ricerca nome variabile in Var NO H
                    nPos = lstAlarmVars.indexOf(szTemp);
                    if (nPos < 0 )  {
                        // Variabile non definita
                        fillErrorMessage(nRow, colCompare, errCTNoVar2, szVarName, szTemp, chSeverityError, &errCt);
                        lstCTErrors.append(errCt);
                        nErrors++;
                    }
                    nPos = varName2Row(szTemp, lstCTRecords);
                    // Verifica di Compatibilità di tipo
                    if (nPos >= 0 && nPos < lstCTRecords.count())  {
                        varTypes tipoVar2 = lstCTRecords[nPos].VarType;
                        QList<int> lstCompatTypes;
                        // Ricerca dei tipi compatibili con Var1
                        fillCompatibleTypesList(nTypeVar1, lstCompatTypes);
                        if (lstCompatTypes.indexOf(tipoVar2) < 0)  {
                            // Variabile 2 non compatibile
                            fillErrorMessage(nRow, colCompare, errCTIncompatibleVar, szVarName, szTemp, chSeverityError, &errCt);
                            lstCTErrors.append(errCt);
                            nErrors++;

                        }
                    }
                }
            }
        }
    }
    //
    // TODO: Ulteriori controlli formali....
    //
    // qDebug() << "Found Errors:" << nErrors;
    // Form visualizzazione errori se richiesto
    if (fSingleLine && nErrors)  {
        errWindow = new cteErrorList(this, true);
        errWindow->setModal(true);
        errWindow->lstErrors2Grid(lstCTErrors);
        // Accepted == clear
        if (errWindow->exec() == QDialog::Accepted)  {
            // Pulizia Form Editing
            clearEntryForm();
            // Pulizia Record sottostante
            freeCTrec(nRow);
            // Ricarica Record vuoto a griglia
            fOk = recCT2List(lstValues, nRow);
            fOk = list2GridRow(lstValues, nRow);
        }
        delete errWindow;
    }
    // Return Value
    return nErrors;
}

void ctedit::on_cboVariable1_currentIndexChanged(int index)
{
    QString szVarName;
    int     nRow = -1;
    QList<int> lstVTypes;
    QString szRightVar;

    lstVTypes.clear();
    ui->lblTypeVar1->setText(szEMPTY);
    m_vtAlarmVarType = UNKNOWN;
    if (index >= 0)  {
        szRightVar.clear();
        szVarName = ui->cboVariable1->currentText();
        if (!szVarName.isEmpty())  {
            nRow = varName2Row(szVarName, lstCTRecords);
        }
        if (nRow >= 0 && nRow < lstCTRecords.count())  {
            // Recupera il tipo della Variabile a SX dell'espressione Allarme/Evento
            m_vtAlarmVarType = lstCTRecords[nRow].VarType;
            // qDebug() << "Row First Variable in Alarm (Row - Name - nType - Type):" << nRow << szVarName << m_vtAlarmVarType << QString::fromAscii(varTypeName[m_vtAlarmVarType]);
            QString szVar1Type = QString::fromAscii(varTypeNameExtended[m_vtAlarmVarType]);
            szVar1Type.prepend(QString::fromAscii("["));
            szVar1Type.append(QString::fromAscii("]"));
            ui->lblTypeVar1->setText(szVar1Type);
            // Disabilita le voci per Rising and falling
            if (m_vtAlarmVarType == BIT || m_vtAlarmVarType == BYTE_BIT || m_vtAlarmVarType == WORD_BIT || m_vtAlarmVarType == DWORD_BIT)  {
                // qDebug() << "Condition Enabled";
                enableComboItem(ui->cboCondition, oper_rising);
                enableComboItem(ui->cboCondition, oper_falling);
            }
            else  {
                // qDebug() << "Condition is Disabled";
                disableComboItem(ui->cboCondition, oper_rising);
                disableComboItem(ui->cboCondition, oper_falling);
            }
            // Salva il nome della Var DX se già specificato
            if (ui->cboVariable2->count() > 0 && ui->cboVariable2->currentIndex() >= 0)  {
                szRightVar = ui->cboVariable2->currentText();
            }
            // Fill Var Type List with compatible types
            fillCompatibleTypesList(m_vtAlarmVarType, lstVTypes);
            // Fill Right Var Combo List
            fillComboVarNames(ui->cboVariable2, lstVTypes, lstNoHUpdates);
            // Reset Index Variable
            ui->cboVariable2->setCurrentIndex(-1);
            // Search Left variable in Right List to remove it
            nRow = ui->cboVariable2->findText(szVarName, Qt::MatchExactly);
            if (nRow >= 0 && nRow < ui->cboVariable2->count())
                ui->cboVariable2->removeItem(nRow);
            // Search original var in combo if available
            if (! szRightVar.isEmpty())  {
                nRow = ui->cboVariable2->findText(szRightVar, Qt::MatchExactly);
                if (nRow >= 0 && nRow < ui->cboVariable2->count())  {
                    ui->cboVariable2->setCurrentIndex(nRow);
                }
            }
        }
    }
}

void ctedit::on_cboCondition_currentIndexChanged(int index)
{
    // Condizione di sicurezza per file CT non aperto
    if (lstCTRecords.count() <= 0 || index < 0)
        return;
    // In caso di Rising o Falling non esiste comparazione con elementi DX
    if (index >= oper_rising)  {
        ui->cboVariable2->setCurrentIndex(-1);
        ui->txtFixedValue->setText(szEMPTY);
        ui->fraSelector->setEnabled(false);
        ui->fraRight->setEnabled(false);
    }
    else  {
        ui->fraSelector->setEnabled(true);
        ui->fraRight->setEnabled(true);
    }
}

int ctedit::varName2Row(QString &szVarName, QList<CrossTableRecord> &lstCTRecs)
// Search in Cross Table the index of szVarName
{
    int nRow = -1;
    char searchTag[MAX_IDNAME_LEN];

    if (! szVarName.isEmpty())  {
        strcpy(searchTag, szVarName.toAscii().data());
        for (nRow = 0; nRow < lstCTRecs.count(); nRow++)  {
            if (lstCTRecs[nRow].UsedEntry)  {
                if (strcmp(searchTag, lstCTRecs[nRow].Tag) == 0)
                    break;
            }
        }
        // Check Failed Search
        if (nRow == lstCTRecs.count())
            nRow = -1;
    }
    return nRow;
}

void ctedit::on_optFixedVal_toggled(bool checked)
{
    if (checked)  {
        ui->txtFixedValue->setEnabled(true);
        ui->cboVariable2->setEnabled(false);
    }
}

void ctedit::on_optVariableVal_toggled(bool checked)
{
    if (checked)  {
        ui->txtFixedValue->setEnabled(false);
        ui->cboVariable2->setEnabled(true);
    }
}
int  ctedit::fillCompatibleTypesList(varTypes nTypeVar, QList<int> &lstTypes)
// Riempie la lista dei tipi compatibili tra loro
{
    // Pulizia lista di destinazione
    lstTypes.clear();
    if (nTypeVar >= BIT && nTypeVar < TYPE_TOTALS) {
        switch (nTypeVar) {
            // Vari tipi di BIT
            case BIT:
            case BYTE_BIT:
            case WORD_BIT:
            case DWORD_BIT:
                lstTypes.append(BIT);
                lstTypes.append(BYTE_BIT);
                lstTypes.append(WORD_BIT);
                lstTypes.append(DWORD_BIT);
                break;
            // Vari tipi di SIGNED INT
            case INT16:
            case INT16BA:
            case DINT:
            case DINTDCBA:
            case DINTCDAB:
            case DINTBADC:
                lstTypes.append(INT16);
                lstTypes.append(INT16BA);
                lstTypes.append(DINT);
                lstTypes.append(DINTDCBA);
                lstTypes.append(DINTCDAB);
                lstTypes.append(DINTBADC);
                break;
            // Vari tipi di UNSIGNED INT
            case UINT8:
            case UINT16:
            case UINT16BA:
            case UDINT:
            case UDINTDCBA:
            case UDINTCDAB:
            case UDINTBADC:
                lstTypes.append(UINT8);
                lstTypes.append(UINT16);
                lstTypes.append(UINT16BA);
                lstTypes.append(UDINT);
                lstTypes.append(UDINTDCBA);
                lstTypes.append(UDINTCDAB);
                lstTypes.append(UDINTBADC);
                break;
            // Vari tipi di REAL
            case REAL:
            case REALDCBA:
            case REALCDAB:
            case REALBADC:
                lstTypes.append(REAL);
                lstTypes.append(REALDCBA);
                lstTypes.append(REALCDAB);
                lstTypes.append(REALBADC);
                break;
            default:
                lstTypes.append(UNKNOWN);
        }
    }
    return lstTypes.count();
}
// Trucco per impostare il valore del #Blocco e altri valori nel caso di nuova riga
void ctedit::on_cboPriority_currentIndexChanged(int index)
{

    if (lstCTRecords.count() <= 0 ||  index < 0 || m_nGridRow < 0)
        return;
    // Applicazione dei valori di default nel caso di una riga vuota
    if (! lstCTRecords[m_nGridRow].UsedEntry && index >= 0 && m_nGridRow < MAX_NONRETENTIVE -1)  {
        // qDebug() << tr("Adding Row: %1") .arg(m_nGridRow);
        if (m_nGridRow > 0 && lstCTRecords[m_nGridRow - 1].UsedEntry)  {
            // Copia da precedente se definita
            // Update
            ui->cboUpdate->setCurrentIndex(lstCTRecords[m_nGridRow - 1].Update);
            // Type
            ui->cboType->setCurrentIndex(lstCTRecords[m_nGridRow - 1].VarType);
            // Decimals
            ui->txtDecimal->setText(QString::number(lstCTRecords[m_nGridRow - 1].Decimal));
            // Protocol
            ui->cboProtocol->setCurrentIndex(lstCTRecords[m_nGridRow - 1].Protocol);
            // Port
            ui->txtPort->setText(QString::number(lstCTRecords[m_nGridRow - 1].Port));
            // Node id
            ui->txtNode->setText(QString::number(lstCTRecords[m_nGridRow - 1].NodeId));
            // Register
            ui->txtRegister->setText(QString::number(lstCTRecords[m_nGridRow - 1].Offset));
            // Behavior
            ui->cboBehavior->setCurrentIndex(lstCTRecords[m_nGridRow - 1].Behavior);
        }
        else  {
            // Valori di default se non definita
            // Update
            ui->cboUpdate->setCurrentIndex(Ptype);
            // Type
            ui->cboType->setCurrentIndex(BIT);
            // Decimals
            ui->txtDecimal->setText(szZERO);
            // Protocol
            ui->cboProtocol->setCurrentIndex(PLC);
            // Behavior
            ui->cboBehavior->setCurrentIndex(behavior_readonly);
        }
        // Block && Block Size (invisibili....)
        if (ui->txtBlock->text().trimmed().isEmpty())  {
            ui->txtBlock->setText(QString::number(m_nGridRow + 1));
            ui->txtBlockSize->setText(QString::number(1));
        }
    }
}
void ctedit::on_cboUpdate_currentIndexChanged(int index)
{

    if (lstCTRecords.count() <= 0 ||  index < 0)
        return;
    // Per variabili di tipo H spegne la possibilità di essere un allarme
    if (index == Htype)  {
        // qDebug() << tr("Clear Alarm");
        // Se la variabile era allarme ripulisce la selezione
        if (ui->cboBehavior->currentIndex() >= behavior_alarm)
            ui->cboBehavior->setCurrentIndex(-1);
        disableComboItem(ui->cboBehavior, behavior_alarm);
        disableComboItem(ui->cboBehavior, behavior_event);
    }
    else  {
        // Abilitazione scelta Allarme/Evento per tipi BIT
        if (ui->cboType->currentIndex() == BIT)  {
            enableComboItem(ui->cboBehavior, behavior_alarm);
            enableComboItem(ui->cboBehavior, behavior_event);

        }
    }

}

void ctedit::on_cmdPLC_clicked()
// Lancio della visualizzazione del PLC Editor
{
    // QStringList lstEnv;
    QString     szPlcPro2Show;

    // First parameter: File plc.4cp
    szPlcPro2Show = szSLASH;
    szPlcPro2Show.append(szPLCFILE);
    szPlcPro2Show.append(szPLCExt);
    szPlcPro2Show.prepend(m_szCurrentPLCPath);
    // qDebug() << "PLC File: " << szPlcPro2Show;
    QFile plcPro(szPlcPro2Show);
    if (! plcPro.exists())  {
        m_szMsg = tr("PLC Project File Not Found:\n<%1>") .arg(szPlcPro2Show);
        warnUser(this, szMectTitle, m_szMsg);
        fprintf(stderr, "%s\n", m_szMsg.toAscii().data());
        goto endStartPLC;
    }
    // Convert File Path to Opertatin System Native Style
    szPlcPro2Show = QDir::toNativeSeparators(szPlcPro2Show);


#ifdef __linux__
    //linux code goes here
    QString     szCommand;
    QString     szPLCEngPath;
    QStringList lstArguments;
    QString     szTemp;
    QString     szPathPLCApplication;
    QProcess    procPLC;
    qint64      pidPLC;

    // Ricerca della variabile specifica per il lancio del PLC
    szPathPLCApplication = QProcessEnvironment::systemEnvironment().value(szPLCEnvVar, szEMPTY);
    // Search Path of PLC Application
    lstArguments.clear();
    if (! szPathPLCApplication.isEmpty())  {
        // qDebug() << tr("Env. %1 Variable: <%2>") .arg(szPLCEnvVar) .arg(szPathPLCApplication);
        // To be modified with specifics of PLC Application
        szTemp = QString::fromAscii("%1");
        // Remove %1
        szPathPLCApplication.remove(szTemp, Qt::CaseInsensitive);
        // Remove doublequote
        szPathPLCApplication.remove(szDOUBLEQUOTE, Qt::CaseInsensitive);
        szPathPLCApplication = szPathPLCApplication.trimmed();
        // qDebug() << tr("Editor PLC: <%1>") .arg(szPathPLCApplication);
        // Build PLC Editor Application command
        QFileInfo plcExe(szPathPLCApplication);
        if (plcExe.exists())  {
            szPLCEngPath = plcExe.absolutePath();
            // qDebug() << tr("Path PLC: <%1>") .arg(szPLCEngPath);
        }
        szCommand = szPathPLCApplication;
        // Enclose command with double quote
        // szCommand.append(szDOUBLEQUOTE);
        // szCommand.prepend(szDOUBLEQUOTE);
        // qDebug() << "PLC Command: " << szCommand;
        // Verifica e Lancio Engineering
        if (plcExe.exists())  {
            // Enclose parameter with double quote (MayBe done by QProcess)
            //szTemp.append(szDOUBLEQUOTE);
            //szTemp.prepend(szDOUBLEQUOTE);
            lstArguments.append(szPlcPro2Show);
            // Imposta come Directory corrente di esecuzione la directory del File PLC
            procPLC.setWorkingDirectory(szPLCEngPath);
            // qDebug() << "Plc Path: " << szPLCEngPath;
            // Esecuzione Comando
            szCommand = QDir::toNativeSeparators(szCommand);
            qDebug() << m_szMsg;
            if (! procPLC.startDetached(szCommand, lstArguments, m_szCurrentPLCPath, &pidPLC))  {
                QProcess::ProcessError errPlc = procPLC.error();
                m_szMsg = tr("Error Starting PLC Engineering: %1\n") .arg(errPlc);
                m_szMsg.append(szCommand);
                warnUser(this, szTitle, m_szMsg);
                goto endStartPLC;
            }
        }
        else {
            m_szMsg = tr("Program PLC Engineering Not Found!\n%1") .arg(szPLCEnvVar);
            m_szMsg.append(szCommand);
            warnUser(this, szTitle, m_szMsg);
            goto endStartPLC;
        }
    }
    else  {
        m_szMsg = tr("Environment Variable for Application PLC Engineering %1 Not Found!\n") .arg(szPLCEnvVar);
        m_szMsg.append(szCommand);
        warnUser(this, szTitle, m_szMsg);
    }
#elif _WIN32
    // windows code goes here
    // Open only File URL
    if (! showFile(szPlcPro2Show))  {
        m_szMsg = tr("Error Opening URL: %1\n") .arg(szPlcPro2Show);
        warnUser(this, szMectTitle, m_szMsg);
    }
#endif

endStartPLC:
    return;
}

bool ctedit::eventFilter(QObject *obj, QEvent *event)
// Gestore Event Handler
{
    static      int nPrevKey = 0;


    // Evento Key Press
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

//        // Tasto ESC (Intercettato per le Combo Box)
//        if (keyEvent->key() == Qt::Key_Escape) {
//            // Clear item for Combos
//            if (obj->metaObject()->className() == "QComboBox")  {
//                QComboBox *cb = qobject_cast<QComboBox*>(obj);
//                cb->setCurrentIndex(-1);
//                return true;
//            }
//        }
        // Sequenze valide per tutto il form
        // Save
        if (keyEvent->matches(QKeySequence::Save)) {
            if (m_nCurTab == TAB_CT)  {
                // qDebug() << tr("Save");
                if (m_isCtModified)
                    on_cmdSave_clicked();
                return true;
            }
        }
        // Find
        if (keyEvent->matches(QKeySequence::Find)) {
            if (m_nCurTab == TAB_CT)  {
                // qDebug() << tr("Find");
                on_cmdSearch_clicked();
                return true;
            }
        }
        // Undo
        if (keyEvent->matches(QKeySequence::Undo)) {
            if (m_nCurTab == TAB_CT)  {
                // qDebug() << tr("Undo");
                if (! lstUndo.isEmpty())
                    on_cmdUndo_clicked();
                return true;
            }
        }
        // Goto Line
        if (keyEvent->key() == Qt::Key_L && nPrevKey == Qt::Key_Control)  {
            // qDebug() << tr("CTRL-L");
            if (m_nCurTab == TAB_CT)  {
                gotoRow();
                return true;
            }
        }
        // Return / Enter Button
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)  {
            if (m_nCurTab == TAB_CT)  {
                if (obj == ui->fraEdit)  {
                    // Enter su Editing Form
                    // qDebug() << tr("Enter in Form");
                    if (! isFormEmpty() && isLineModified(m_nGridRow)) {
                        updateRow(m_nGridRow);
                        enableInterface();
                    }
                    // QKeyEvent newEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, szEMPTY);
                    ui->tblCT->setFocus();
                    return true;
                }
                else if (obj == ui->tblCT)  {
                    // Enter su Grid
                    // qDebug() << tr("Enter in Grid");
                    // Salto a riga successiva
                    jumpToGridRow(findNextVisibleRow(m_nGridRow), false);
                    return true;
                }
            }
        }
        // Sequenze significative solo sul Grid
        if (obj == ui->tblCT)  {
            // Tasto Insert
            if (keyEvent->key() == Qt::Key_Insert) {
                // qDebug() << tr("Insert");
                insertRows();
                return true;
            }
            // Sequenza Copy
            if (keyEvent->matches(QKeySequence::Copy)) {
                copySelected(true);
                return true;
            }
            // Tasto Paste
            if (keyEvent->matches(QKeySequence::Paste)) {
                pasteSelected();
                return true;
            }
            // Tasto Cut
            if (keyEvent->matches(QKeySequence::Cut)) {
                cutSelected();
                return true;
            }
            // Tasto Delete
            if (keyEvent->matches(QKeySequence::Delete)) {
                emptySelected();
                return true;
            }
        }
        // qDebug() << tr("Pressed Key Value") << keyEvent->key();
        nPrevKey = keyEvent->key();
    }
    // Pass event to standard Event Handler
    return QObject::eventFilter(obj, event);
}
int ctedit::varSizeInBlock(int nVarType)
{
    int nSize = 1;

    switch (nVarType) {
        // Doppio Byte
        case DWORD_BIT:
        case REAL:
        case REALDCBA:
        case REALCDAB:
        case REALBADC:
        case UDINT:
        case UDINTDCBA:
        case UDINTCDAB:
        case UDINTBADC:
        case DINT:
        case DINTDCBA:
        case DINTCDAB:
        case DINTBADC:
            nSize = 2;
            break;
        default:
            nSize = 1;
    }
    return nSize;
}
int ctedit::maxBlockSize(FieldbusType nProtocol, int nPort)
// max block size from Protocol && Port
{
    int nBlockSize = 0;

    // Protocollo PLC dimesione pari a MAXBLOCKSIZE
    switch (nProtocol) {
        case PLC:
            nBlockSize = MAXBLOCKSIZE;
            break;
        case TCP:
        case TCPRTU:
        case TCP_SRV:
        case TCPRTU_SRV:
            nBlockSize = TargetConfig.tcp_BlockSize;
            break;
        case CANOPEN:
            if (nPort == 0)
                nBlockSize = TargetConfig.can0_BlockSize;
            else
                nBlockSize = TargetConfig.can1_BlockSize;
            break;

        default:
            // All Serial Protocols
            if (nPort == 0)
                nBlockSize = TargetConfig.ser0_BlockSize;
            else if (nPort == 1)
                nBlockSize = TargetConfig.ser1_BlockSize;
            else if (nPort == 2)
                nBlockSize = TargetConfig.ser2_BlockSize;
            else if (nPort == 3)
                nBlockSize = TargetConfig.ser3_BlockSize;
            break;
    }
    // return
    return nBlockSize;
}
bool ctedit::isModbus(enum FieldbusType nProtocol)
{
    bool fModbus = false;

    // Protocollo PLC dimesione pari a MAXBLOCKSIZE
    switch (nProtocol) {
        case RTU:
        case TCP:
        case TCPRTU:
        case RTU_SRV:
        case TCP_SRV:
        case TCPRTU_SRV:
            fModbus = true;
            break;

        default:
            fModbus = false;
            break;
    }
    // return
    return fModbus;

}

bool ctedit::isSameBitField(int nRow)
{
    bool    fRes = false;
    if (nRow > 0)  {
        if (isBitField(lstCTRecords[nRow].VarType) &&
            lstCTRecords[nRow].VarType == lstCTRecords[nRow - 1].VarType &&
            lstCTRecords[nRow].Protocol == lstCTRecords[nRow - 1].Protocol &&
            lstCTRecords[nRow].IPAddress == lstCTRecords[nRow -1].IPAddress &&
            lstCTRecords[nRow].Port == lstCTRecords[nRow - 1].Port &&
            lstCTRecords[nRow].NodeId == lstCTRecords[nRow - 1].NodeId &&
            lstCTRecords[nRow].Offset == lstCTRecords[nRow - 1].Offset)  {
                fRes = true;
            }
    }
    return fRes;
}

bool ctedit::isBitField(enum varTypes nVarType)
{
    bool fRes = false;

    switch (nVarType) {
        //
        case BYTE_BIT:
        case WORD_BIT:
        case DWORD_BIT:
            fRes = true;
            break;
        default:
            fRes = false;
    }
    return fRes;
}

bool ctedit::isTooBigForBlock(int nRow, int nItemsInBlock, int nCurBlockSize)
{
    bool    fRes = false;
    int     nVarSize;
    int     maxBSize;
    int     nCur;

    nVarSize = varSizeInBlock(lstCTRecords[nRow].VarType);
    maxBSize = maxBlockSize(lstCTRecords[nRow].Protocol, lstCTRecords[nRow].Port);
    fRes = ((nCurBlockSize + nVarSize) > maxBSize || (nItemsInBlock + 1) > MAXBLOCKSIZE);

    if (! fRes && isBitField(lstCTRecords[nRow].VarType)) {
        int nBitFieldLen = 1;
        for (nCur = nRow + 1; nCur < DimCrossTable; nCur++)  {
            if (isSameBitField(nCur))
                nBitFieldLen++;
            else
                break;
        }
        fRes = (nItemsInBlock + nBitFieldLen) > MAXBLOCKSIZE;
    }
    return fRes;
}
void ctedit::setSectionArea(int nRow)
// Set Current item in combo cboSection from current Row
{
    int nIndex = -1;

    if (nRow < 0)
        return;
    // Blocca la propagazione dei segnali della cboSection
    disableAndBlockSignals(ui->cboSections);
    // Range Retentive
    if (nRow >= (MIN_RETENTIVE - 1) && nRow <= (MAX_RETENTIVE - 1))
        nIndex = regRetentive;
    else if (nRow >= (MIN_NONRETENTIVE - 1) && nRow <= (MAX_NONRETENTIVE - 1))
        nIndex = regNonRetentive;
    else if (nRow >= (MIN_DIAG - 1) && nRow <= (MAX_NODE - 1))
        nIndex = regDiagnostic;
    else if (nRow >= (MIN_LOCALIO - 1) && nRow <= (MAX_LOCALIO - 1))
        nIndex = regLocalIO;
    else if (nRow >= (MIN_SYSTEM - 1) && nRow <= (DimCrossTable - 1))
        nIndex = regSystem;
    // Set Item in Combo
    if (nIndex >= -1 && nIndex < lstRegions.count())
        ui->cboSections->setCurrentIndex(nIndex);
    // Sblocca la propagazione dei segnali della cboSection
    enableAndUnlockSignals(ui->cboSections);

}

void ctedit::on_cboSections_currentIndexChanged(int index)
{
    int     nRow = 0;

    if (index < 0)
        return;
    if (index == regRetentive)
        nRow = MIN_RETENTIVE - 1;
    else if (index == regNonRetentive)
        nRow = MIN_NONRETENTIVE - 1;
    else if (index == regDiagnostic)
        nRow = MIN_DIAG - 1;
    else if (index == regLocalIO)
        nRow = MIN_LOCALIO - 1;
    else if (index == regSystem)
        nRow = MIN_SYSTEM - 1;
    // Controllo se la riga è abilitata
    if (! m_fShowAllRows && ! lstCTRecords[nRow].Enable)  {
        m_fShowAllRows = true;
        ui->cmdHideShow->setChecked(m_fShowAllRows);
    }
    // Jump to Row
    jumpToGridRow(nRow, true);
}

bool ctedit::checkCTFile(QString szSourceFile)
// Controllo validità file CT per Import
{
    bool fRes = false;

    QFileInfo   fileCT(szSourceFile);
    // Controllo esistenza file
    if (fileCT.exists())  {
        // Controllo nome del file
        if (fileCT.fileName() == szCT_FILE_NAME)  {
            QStringList stringList;
            QFile textFile(szSourceFile);
            textFile.open(QIODevice::ReadOnly);
            QTextStream textStream(&textFile);
            while (! textStream.atEnd())
            {
                QString line = textStream.readLine();
                if (line.isNull())
                    break;
                else
                    stringList.append(line);
            }
            textFile.close();
            fRes = (stringList.count() == DimCrossTable);
        }
    }
    // return value
    return  fRes;
}
void ctedit::initTargetList()
// Init della lista dei Target definiti
{
    TP_Config   tpRec;
    int         nModel;

    // Valori di default comuni a tutti i modelli
    // General Params
    tpRec.modelName.clear();    
    tpRec.displayWidth = 480;
    tpRec.displayHeight = 272;
    tpRec.usbPorts = 1;
    tpRec.ethPorts = 1;
    tpRec.sdCards = 0;
    tpRec.nEncoders = 0;
    tpRec.digitalIN = 0;
    tpRec.digitalOUT = 0;
    tpRec.analogIN = 0;
    tpRec.analogINrowCT = -1;
    tpRec.analogOUT = 0;
    tpRec.analogOUTrowCT = -1;
    tpRec.tAmbient = false;
    tpRec.rpmPorts = 0;
    tpRec.retries = 1;
    tpRec.blacklist = 2;
    tpRec.readPeriod1 = 10;
    tpRec.readPeriod2 = 50;
    tpRec.readPeriod3 = 200;
    tpRec.fastLogPeriod = 1;
    tpRec.slowLogPeriod = 10;
    // Serial 0
    tpRec.ser0_Enabled = false;
    tpRec.ser0_BaudRate = 38400;
    tpRec.ser0_TimeOut = 50;
    tpRec.ser0_Silence = 2;
    tpRec.ser0_BlockSize = 64;
    // Serial 1
    tpRec.ser1_Enabled = false;
    tpRec.ser1_BaudRate = 38400;
    tpRec.ser1_TimeOut = 50;
    tpRec.ser1_Silence = 2;
    tpRec.ser1_BlockSize = 64;
    // Serial 2
    tpRec.ser2_Enabled = false;
    tpRec.ser2_BaudRate = 38400;
    tpRec.ser2_TimeOut = 50;
    tpRec.ser2_Silence = 2;
    tpRec.ser2_BlockSize = 64;
    // Serial 3
    tpRec.ser3_Enabled = true;
    tpRec.ser3_BaudRate = 38400;
    tpRec.ser3_TimeOut = 50;
    tpRec.ser3_Silence = 2;
    tpRec.ser3_BlockSize = 64;
    // TCP
    tpRec.tcp_TimeOut = 200;
    tpRec.tcp_Silence = 10;
    tpRec.tcp_BlockSize = 64;
    // Can0
    tpRec.can0_Enabled = false;
    tpRec.can0_BaudRate = 125000;
    tpRec.can0_BlockSize = 64;
    // Can1
    tpRec.can1_Enabled = false;
    tpRec.can1_BaudRate = 125000;
    tpRec.can1_BlockSize = 64;
    // Creazione Lista modelli
    lstTargets.clear();
    for (nModel = 0; nModel < MODEL_TOTALS; nModel++)  {
        tpRec.nModel = nModel;
        lstTargets.append(tpRec);
    }
    // Customizzazione dei modelli
    //00 AnyTPAC = 0
    lstTargets[AnyTPAC].modelName =  QString::fromAscii(product_name[AnyTPAC]);
    //01 TP1043_01_A
    lstTargets[TP1043_01_A].modelName =  QString::fromAscii(product_name[TP1043_01_A]);
    lstTargets[TP1043_01_A].displayWidth =  480;
    lstTargets[TP1043_01_A].displayHeight = 272;
    lstTargets[TP1043_01_A].sdCards = 1;
    lstTargets[TP1043_01_A].digitalIN = 0;
    lstTargets[TP1043_01_A].digitalOUT = 0;
    lstTargets[TP1043_01_A].nEncoders = 0;
    lstTargets[TP1043_01_A].analogIN = 0;
    lstTargets[TP1043_01_A].analogINrowCT = -1;
    lstTargets[TP1043_01_A].analogOUT = 0;
    lstTargets[TP1043_01_A].analogOUTrowCT = -1;
    lstTargets[TP1043_01_A].tAmbient = false;
    lstTargets[TP1043_01_A].rpmPorts = 0;
    lstTargets[TP1043_01_A].ser0_Enabled = true;
    lstTargets[TP1043_01_A].ser1_Enabled = false;
    lstTargets[TP1043_01_A].ser2_Enabled = false;
    lstTargets[TP1043_01_A].ser3_Enabled = false;
    lstTargets[TP1043_01_A].can1_Enabled = false;
    // 02 TP1043_01_B
    lstTargets[TP1043_01_B].modelName =  QString::fromAscii(product_name[TP1043_01_B]);
    lstTargets[TP1043_01_B].displayWidth =  480;
    lstTargets[TP1043_01_B].displayHeight = 272;
    lstTargets[TP1043_01_B].sdCards = 1;
    lstTargets[TP1043_01_B].digitalIN = 0;
    lstTargets[TP1043_01_B].digitalOUT = 0;
    lstTargets[TP1043_01_B].nEncoders = 0;
    lstTargets[TP1043_01_B].analogIN = 0;
    lstTargets[TP1043_01_B].analogINrowCT = -1;
    lstTargets[TP1043_01_B].analogOUT = 0;
    lstTargets[TP1043_01_B].analogOUTrowCT = -1;
    lstTargets[TP1043_01_B].tAmbient = false;
    lstTargets[TP1043_01_B].rpmPorts = 0;
    lstTargets[TP1043_01_B].ser0_Enabled = false;
    lstTargets[TP1043_01_B].ser1_Enabled = false;
    lstTargets[TP1043_01_B].ser2_Enabled = false;
    lstTargets[TP1043_01_B].ser3_Enabled = false;
    lstTargets[TP1043_01_B].can1_Enabled = true;
    // 03 TP1057_01_A
    lstTargets[TP1057_01_A].modelName =  QString::fromAscii(product_name[TP1057_01_A]);
    lstTargets[TP1057_01_A].displayWidth =  320;
    lstTargets[TP1057_01_A].displayHeight = 240;
    lstTargets[TP1057_01_A].sdCards = 0;
    lstTargets[TP1057_01_A].digitalIN = 0;
    lstTargets[TP1057_01_A].digitalOUT = 0;
    lstTargets[TP1057_01_A].nEncoders = 0;
    lstTargets[TP1057_01_A].analogIN = 0;
    lstTargets[TP1057_01_A].analogINrowCT = -1;
    lstTargets[TP1057_01_A].analogOUT = 0;
    lstTargets[TP1057_01_A].analogOUTrowCT = -1;
    lstTargets[TP1057_01_A].tAmbient = false;
    lstTargets[TP1057_01_A].rpmPorts = 0;
    lstTargets[TP1057_01_A].ser0_Enabled = false;
    lstTargets[TP1057_01_A].ser1_Enabled = false;
    lstTargets[TP1057_01_A].ser2_Enabled = false;
    lstTargets[TP1057_01_A].ser3_Enabled = true;
    lstTargets[TP1057_01_A].can1_Enabled = false;
    // 04 TP1057_01_B
    lstTargets[TP1057_01_B].modelName =  QString::fromAscii(product_name[TP1057_01_B]);
    lstTargets[TP1057_01_B].displayWidth =  320;
    lstTargets[TP1057_01_B].displayHeight = 240;
    lstTargets[TP1057_01_B].sdCards = 0;
    lstTargets[TP1057_01_B].digitalIN = 0;
    lstTargets[TP1057_01_B].digitalOUT = 0;
    lstTargets[TP1057_01_B].nEncoders = 0;
    lstTargets[TP1057_01_B].analogIN = 0;
    lstTargets[TP1057_01_B].analogINrowCT = -1;
    lstTargets[TP1057_01_B].analogOUT = 0;
    lstTargets[TP1057_01_B].analogOUTrowCT = -1;
    lstTargets[TP1057_01_B].tAmbient = false;
    lstTargets[TP1057_01_B].rpmPorts = 0;
    lstTargets[TP1057_01_B].ser0_Enabled = false;
    lstTargets[TP1057_01_B].ser1_Enabled = false;
    lstTargets[TP1057_01_B].ser2_Enabled = false;
    lstTargets[TP1057_01_B].ser3_Enabled = true;
    lstTargets[TP1057_01_B].can1_Enabled = true;
    // 05 TP1070_01_A
    lstTargets[TP1070_01_A].modelName =  QString::fromAscii(product_name[TP1070_01_A]);
    lstTargets[TP1070_01_A].displayWidth =  800;
    lstTargets[TP1070_01_A].displayHeight = 480;
    lstTargets[TP1070_01_A].sdCards = 0;
    lstTargets[TP1070_01_A].digitalIN = 0;
    lstTargets[TP1070_01_A].digitalOUT = 0;
    lstTargets[TP1070_01_A].nEncoders = 0;
    lstTargets[TP1070_01_A].analogIN = 0;
    lstTargets[TP1070_01_A].analogINrowCT = -1;
    lstTargets[TP1070_01_A].analogOUT = 0;
    lstTargets[TP1070_01_A].analogOUTrowCT = -1;
    lstTargets[TP1070_01_A].tAmbient = false;
    lstTargets[TP1070_01_A].rpmPorts = 0;
    lstTargets[TP1070_01_A].ser0_Enabled = false;
    lstTargets[TP1070_01_A].ser1_Enabled = false;
    lstTargets[TP1070_01_A].ser2_Enabled = false;
    lstTargets[TP1070_01_A].ser3_Enabled = true;
    lstTargets[TP1070_01_A].can1_Enabled = false;
    // 06 TP1070_01_B
    lstTargets[TP1070_01_B].modelName =  QString::fromAscii(product_name[TP1070_01_B]);
    lstTargets[TP1070_01_B].displayWidth =  800;
    lstTargets[TP1070_01_B].displayHeight = 480;
    lstTargets[TP1070_01_B].sdCards = 0;
    lstTargets[TP1070_01_B].digitalIN = 0;
    lstTargets[TP1070_01_B].digitalOUT = 0;
    lstTargets[TP1070_01_B].nEncoders = 0;
    lstTargets[TP1070_01_B].analogIN = 0;
    lstTargets[TP1070_01_B].analogINrowCT = -1;
    lstTargets[TP1070_01_B].analogOUT = 0;
    lstTargets[TP1070_01_B].analogOUTrowCT = -1;
    lstTargets[TP1070_01_B].tAmbient = false;
    lstTargets[TP1070_01_B].rpmPorts = 0;
    lstTargets[TP1070_01_B].ser0_Enabled = false;
    lstTargets[TP1070_01_B].ser1_Enabled = false;
    lstTargets[TP1070_01_B].ser2_Enabled = false;
    lstTargets[TP1070_01_B].ser3_Enabled = true;
    lstTargets[TP1070_01_B].can1_Enabled = true;
    // 07 TP1070_01_C
    lstTargets[TP1070_01_C].modelName =  QString::fromAscii(product_name[TP1070_01_C]);
    lstTargets[TP1070_01_C].displayWidth =  800;
    lstTargets[TP1070_01_C].displayHeight = 480;
    lstTargets[TP1070_01_C].sdCards = 0;
    lstTargets[TP1070_01_C].digitalIN = 0;
    lstTargets[TP1070_01_C].digitalOUT = 0;
    lstTargets[TP1070_01_C].nEncoders = 0;
    lstTargets[TP1070_01_C].analogIN = 0;
    lstTargets[TP1070_01_C].analogINrowCT = -1;
    lstTargets[TP1070_01_C].analogOUT = 0;
    lstTargets[TP1070_01_C].analogOUTrowCT = -1;
    lstTargets[TP1070_01_C].tAmbient = false;
    lstTargets[TP1070_01_C].rpmPorts = 0;
    lstTargets[TP1070_01_C].ser0_Enabled = true;
    lstTargets[TP1070_01_C].ser1_Enabled = false;
    lstTargets[TP1070_01_C].ser2_Enabled = false;
    lstTargets[TP1070_01_C].ser3_Enabled = true;
    lstTargets[TP1070_01_C].can1_Enabled = false;
    // 08 TPAC1006
    lstTargets[TPAC1006].modelName =  QString::fromAscii(product_name[TPAC1006]);
    lstTargets[TPAC1006].displayWidth =  320;
    lstTargets[TPAC1006].displayHeight = 240;
    lstTargets[TPAC1006].sdCards = 0;
    lstTargets[TPAC1006].digitalIN = 8;
    lstTargets[TPAC1006].digitalOUT = 12;
    lstTargets[TPAC1006].nEncoders = 1;
    lstTargets[TPAC1006].analogIN = 4;
    lstTargets[TPAC1006].analogINrowCT = 5323;
    lstTargets[TPAC1006].analogOUT = 2;
    lstTargets[TPAC1006].analogOUTrowCT = 5332;
    lstTargets[TPAC1006].tAmbient = true;
    lstTargets[TPAC1006].rpmPorts = 1;
    lstTargets[TPAC1006].ser0_Enabled = false;
    lstTargets[TPAC1006].ser1_Enabled = false;
    lstTargets[TPAC1006].ser2_Enabled = false;
    lstTargets[TPAC1006].ser3_Enabled = true;
    lstTargets[TPAC1006].can1_Enabled = true;
    //09 TPAC1007_03
    lstTargets[TPAC1007_03].modelName =  QString::fromAscii(product_name[TPAC1007_03]);
    lstTargets[TPAC1007_03].displayWidth =  480;
    lstTargets[TPAC1007_03].displayHeight = 272;
    lstTargets[TPAC1007_03].sdCards = 1;
    lstTargets[TPAC1007_03].digitalIN = 12;
    lstTargets[TPAC1007_03].digitalOUT = 8;
    lstTargets[TPAC1007_03].nEncoders = 1;
    lstTargets[TPAC1007_03].analogIN = 2;
    lstTargets[TPAC1007_03].analogINrowCT = 5325;
    lstTargets[TPAC1007_03].analogOUT = 1;
    lstTargets[TPAC1007_03].analogOUTrowCT = 5337;
    lstTargets[TPAC1007_03].tAmbient = true;
    lstTargets[TPAC1007_03].rpmPorts = 0;
    lstTargets[TPAC1007_03].ser0_Enabled = true;
    lstTargets[TPAC1007_03].ser1_Enabled = false;
    lstTargets[TPAC1007_03].ser2_Enabled = false;
    lstTargets[TPAC1007_03].ser3_Enabled = false;
    lstTargets[TPAC1007_03].can1_Enabled = false;
    //10 TPAC1007_04_AA
    lstTargets[TPAC1007_04_AA].modelName =  QString::fromAscii(product_name[TPAC1007_04_AA]);
    lstTargets[TPAC1007_04_AA].displayWidth =  480;
    lstTargets[TPAC1007_04_AA].displayHeight = 272;
    lstTargets[TPAC1007_04_AA].sdCards = 1;
    lstTargets[TPAC1007_04_AA].digitalIN = 12;
    lstTargets[TPAC1007_04_AA].digitalOUT = 8;
    lstTargets[TPAC1007_04_AA].nEncoders = 1;
    lstTargets[TPAC1007_04_AA].analogIN = 2;
    lstTargets[TPAC1007_04_AA].analogINrowCT = 5328;
    lstTargets[TPAC1007_04_AA].analogOUT = 4;
    lstTargets[TPAC1007_04_AA].analogOUTrowCT = 5344;
    lstTargets[TPAC1007_04_AA].tAmbient = true;
    lstTargets[TPAC1007_04_AA].rpmPorts = 0;
    lstTargets[TPAC1007_04_AA].ser0_Enabled = true;
    lstTargets[TPAC1007_04_AA].ser1_Enabled = false;
    lstTargets[TPAC1007_04_AA].ser2_Enabled = false;
    lstTargets[TPAC1007_04_AA].ser3_Enabled = false;
    lstTargets[TPAC1007_04_AA].can1_Enabled = false;
    //11 TPAC1007_04_AB
    lstTargets[TPAC1007_04_AB].modelName =  QString::fromAscii(product_name[TPAC1007_04_AB]);
    lstTargets[TPAC1007_04_AB].displayWidth =  480;
    lstTargets[TPAC1007_04_AB].displayHeight = 272;
    lstTargets[TPAC1007_04_AB].sdCards = 1;
    lstTargets[TPAC1007_04_AB].digitalIN = 9;
    lstTargets[TPAC1007_04_AB].digitalOUT = 8;
    lstTargets[TPAC1007_04_AB].nEncoders = 1;
    lstTargets[TPAC1007_04_AB].analogIN = 5;
    lstTargets[TPAC1007_04_AB].analogINrowCT = 5328;
    lstTargets[TPAC1007_04_AB].analogOUT = 4;
    lstTargets[TPAC1007_04_AB].analogOUTrowCT = 5344;
    lstTargets[TPAC1007_04_AB].tAmbient = true;
    lstTargets[TPAC1007_04_AB].rpmPorts = 0;
    lstTargets[TPAC1007_04_AB].ser0_Enabled = true;
    lstTargets[TPAC1007_04_AB].ser1_Enabled = false;
    lstTargets[TPAC1007_04_AB].ser2_Enabled = false;
    lstTargets[TPAC1007_04_AB].ser3_Enabled = false;
    lstTargets[TPAC1007_04_AB].can1_Enabled = false;
    //12 TPAC1007_04_AC
    lstTargets[TPAC1007_04_AC].modelName =  QString::fromAscii(product_name[TPAC1007_04_AC]);
    lstTargets[TPAC1007_04_AC].displayWidth =  480;
    lstTargets[TPAC1007_04_AC].displayHeight = 272;
    lstTargets[TPAC1007_04_AC].sdCards = 1;
    lstTargets[TPAC1007_04_AC].digitalIN = 12;
    lstTargets[TPAC1007_04_AC].digitalOUT = 8;
    lstTargets[TPAC1007_04_AC].nEncoders = 1;
    lstTargets[TPAC1007_04_AC].analogIN = 2;
    lstTargets[TPAC1007_04_AC].analogINrowCT = 5328;
    lstTargets[TPAC1007_04_AC].analogOUT = 1;
    lstTargets[TPAC1007_04_AC].analogOUTrowCT = 5344;
    lstTargets[TPAC1007_04_AC].tAmbient = true;
    lstTargets[TPAC1007_04_AC].rpmPorts = 0;
    lstTargets[TPAC1007_04_AC].ser0_Enabled = true;
    lstTargets[TPAC1007_04_AC].ser1_Enabled = false;
    lstTargets[TPAC1007_04_AC].ser2_Enabled = false;
    lstTargets[TPAC1007_04_AC].ser3_Enabled = false;
    lstTargets[TPAC1007_04_AC].can1_Enabled = false;
    // 13 TPAC1007_LV
    lstTargets[TPAC1007_LV].modelName =  QString::fromAscii(product_name[TPAC1007_LV]);
    lstTargets[TPAC1007_LV].displayWidth =  480;
    lstTargets[TPAC1007_LV].displayHeight = 272;
    lstTargets[TPAC1007_LV].sdCards = 0;
    lstTargets[TPAC1007_LV].digitalIN = 8;
    lstTargets[TPAC1007_LV].digitalOUT = 8;
    lstTargets[TPAC1007_LV].nEncoders = 1;
    lstTargets[TPAC1007_LV].analogIN = 0;
    lstTargets[TPAC1007_LV].analogINrowCT = -1;
    lstTargets[TPAC1007_LV].analogOUT = 0;
    lstTargets[TPAC1007_LV].analogOUTrowCT = -1;
    lstTargets[TPAC1007_LV].tAmbient = true;
    lstTargets[TPAC1007_LV].rpmPorts = 0;
    lstTargets[TPAC1007_LV].ser0_Enabled = true;
    lstTargets[TPAC1007_LV].ser1_Enabled = false;
    lstTargets[TPAC1007_LV].ser2_Enabled = false;
    lstTargets[TPAC1007_LV].ser3_Enabled = true;
    lstTargets[TPAC1007_LV].can1_Enabled = false;
    // 14 TPAC1008_01
    lstTargets[TPAC1008_01].modelName =  QString::fromAscii(product_name[TPAC1008_01]);
    lstTargets[TPAC1008_01].displayWidth =  800;
    lstTargets[TPAC1008_01].displayHeight = 480;
    lstTargets[TPAC1008_01].sdCards = 0;
    lstTargets[TPAC1008_01].digitalIN = 8;
    lstTargets[TPAC1008_01].digitalOUT = 12;
    lstTargets[TPAC1008_01].nEncoders = 1;
    lstTargets[TPAC1008_01].analogIN = 4;
    lstTargets[TPAC1008_01].analogINrowCT = 5323;
    lstTargets[TPAC1008_01].analogOUT = 2;
    lstTargets[TPAC1008_01].analogOUTrowCT = 5332;
    lstTargets[TPAC1008_01].tAmbient = true;
    lstTargets[TPAC1008_01].rpmPorts = 1;
    lstTargets[TPAC1008_01].ser0_Enabled = false;
    lstTargets[TPAC1008_01].ser1_Enabled = false;
    lstTargets[TPAC1008_01].ser2_Enabled = false;
    lstTargets[TPAC1008_01].ser3_Enabled = true;
    lstTargets[TPAC1008_01].can1_Enabled = true;
    // 15 TPAC1008_02_AA
    lstTargets[TPAC1008_02_AA].modelName =  QString::fromAscii(product_name[TPAC1008_02_AA]);
    lstTargets[TPAC1008_02_AA].displayWidth =  800;
    lstTargets[TPAC1008_02_AA].displayHeight = 480;
    lstTargets[TPAC1008_02_AA].sdCards = 0;
    lstTargets[TPAC1008_02_AA].digitalIN = 8;
    lstTargets[TPAC1008_02_AA].digitalOUT = 12;
    lstTargets[TPAC1008_02_AA].nEncoders = 1;
    lstTargets[TPAC1008_02_AA].analogIN = 4;
    lstTargets[TPAC1008_02_AA].analogINrowCT = 5325;
    lstTargets[TPAC1008_02_AA].analogOUT = 2;
    lstTargets[TPAC1008_02_AA].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_02_AA].tAmbient = true;
    lstTargets[TPAC1008_02_AA].rpmPorts = 1;
    lstTargets[TPAC1008_02_AA].ser0_Enabled = false;
    lstTargets[TPAC1008_02_AA].ser1_Enabled = false;
    lstTargets[TPAC1008_02_AA].ser2_Enabled = false;
    lstTargets[TPAC1008_02_AA].ser3_Enabled = true;
    lstTargets[TPAC1008_02_AA].can1_Enabled = true;
    // 16 TPAC1008_02_AB
    lstTargets[TPAC1008_02_AB].modelName =  QString::fromAscii(product_name[TPAC1008_02_AB]);
    lstTargets[TPAC1008_02_AB].displayWidth =  800;
    lstTargets[TPAC1008_02_AB].displayHeight = 480;
    lstTargets[TPAC1008_02_AB].sdCards = 0;
    lstTargets[TPAC1008_02_AB].digitalIN = 8;
    lstTargets[TPAC1008_02_AB].digitalOUT = 12;
    lstTargets[TPAC1008_02_AB].nEncoders = 1;
    lstTargets[TPAC1008_02_AB].analogIN = 4;
    lstTargets[TPAC1008_02_AB].analogINrowCT = 5325;
    lstTargets[TPAC1008_02_AB].analogOUT = 2;
    lstTargets[TPAC1008_02_AB].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_02_AB].tAmbient = true;
    lstTargets[TPAC1008_02_AB].rpmPorts = 1;
    lstTargets[TPAC1008_02_AB].ser0_Enabled = true;
    lstTargets[TPAC1008_02_AB].ser1_Enabled = false;
    lstTargets[TPAC1008_02_AB].ser2_Enabled = false;
    lstTargets[TPAC1008_02_AB].ser3_Enabled = true;
    lstTargets[TPAC1008_02_AB].can1_Enabled = false;
    // 17 TPAC1008_02_AD
    lstTargets[TPAC1008_02_AD].modelName =  QString::fromAscii(product_name[TPAC1008_02_AD]);
    lstTargets[TPAC1008_02_AD].displayWidth =  800;
    lstTargets[TPAC1008_02_AD].displayHeight = 480;
    lstTargets[TPAC1008_02_AD].sdCards = 0;
    lstTargets[TPAC1008_02_AD].digitalIN = 8;
    lstTargets[TPAC1008_02_AD].digitalOUT = 12;
    lstTargets[TPAC1008_02_AD].nEncoders = 1;
    lstTargets[TPAC1008_02_AD].analogIN = 4;
    lstTargets[TPAC1008_02_AD].analogINrowCT = 5325;
    lstTargets[TPAC1008_02_AD].analogOUT = 4;
    lstTargets[TPAC1008_02_AD].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_02_AD].tAmbient = true;
    lstTargets[TPAC1008_02_AD].rpmPorts = 1;
    lstTargets[TPAC1008_02_AD].ser0_Enabled = false;
    lstTargets[TPAC1008_02_AD].ser1_Enabled = false;
    lstTargets[TPAC1008_02_AD].ser2_Enabled = false;
    lstTargets[TPAC1008_02_AD].ser3_Enabled = true;
    lstTargets[TPAC1008_02_AD].can1_Enabled = false;
    // 18 TPAC1008_02_AE
    lstTargets[TPAC1008_02_AE].modelName =  QString::fromAscii(product_name[TPAC1008_02_AE]);
    lstTargets[TPAC1008_02_AE].displayWidth =  800;
    lstTargets[TPAC1008_02_AE].displayHeight = 480;
    lstTargets[TPAC1008_02_AE].sdCards = 0;
    lstTargets[TPAC1008_02_AE].digitalIN = 8;
    lstTargets[TPAC1008_02_AE].digitalOUT = 12;
    lstTargets[TPAC1008_02_AE].nEncoders = 1;
    lstTargets[TPAC1008_02_AE].analogIN = 4;
    lstTargets[TPAC1008_02_AE].analogINrowCT = 5325;
    lstTargets[TPAC1008_02_AE].analogOUT = 4;
    lstTargets[TPAC1008_02_AE].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_02_AE].tAmbient = true;
    lstTargets[TPAC1008_02_AE].rpmPorts = 1;
    lstTargets[TPAC1008_02_AE].ser0_Enabled = false;
    lstTargets[TPAC1008_02_AE].ser1_Enabled = false;
    lstTargets[TPAC1008_02_AE].ser2_Enabled = false;
    lstTargets[TPAC1008_02_AE].ser3_Enabled = true;
    lstTargets[TPAC1008_02_AE].can1_Enabled = false;
    // 19 TPAC1008_02_AF
    lstTargets[TPAC1008_02_AF].modelName =  QString::fromAscii(product_name[TPAC1008_02_AF]);
    lstTargets[TPAC1008_02_AF].displayWidth =  800;
    lstTargets[TPAC1008_02_AF].displayHeight = 480;
    lstTargets[TPAC1008_02_AF].sdCards = 0;
    lstTargets[TPAC1008_02_AF].digitalIN = 8;
    lstTargets[TPAC1008_02_AF].digitalOUT = 12;
    lstTargets[TPAC1008_02_AF].nEncoders = 1;
    lstTargets[TPAC1008_02_AF].analogIN = 4;
    lstTargets[TPAC1008_02_AF].analogINrowCT = 5325;
    lstTargets[TPAC1008_02_AF].analogOUT = 4;
    lstTargets[TPAC1008_02_AF].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_02_AF].tAmbient = true;
    lstTargets[TPAC1008_02_AF].rpmPorts = 1;
    lstTargets[TPAC1008_02_AF].ser0_Enabled = false;
    lstTargets[TPAC1008_02_AF].ser1_Enabled = false;
    lstTargets[TPAC1008_02_AF].ser2_Enabled = false;
    lstTargets[TPAC1008_02_AF].ser3_Enabled = true;
    lstTargets[TPAC1008_02_AF].can1_Enabled = false;
    // 20 TPLC100_01_AA
    lstTargets[TPLC100_01_AA].modelName =  QString::fromAscii(product_name[TPLC100_01_AA]);
    lstTargets[TPLC100_01_AA].displayWidth =  -1;
    lstTargets[TPLC100_01_AA].displayHeight = -1;
    lstTargets[TPLC100_01_AA].usbPorts = 2;
    lstTargets[TPLC100_01_AA].sdCards = 0;
    lstTargets[TPLC100_01_AA].digitalIN = 4;
    lstTargets[TPLC100_01_AA].digitalOUT = 8;
    lstTargets[TPLC100_01_AA].nEncoders = 1;
    lstTargets[TPLC100_01_AA].analogIN = 12;
    lstTargets[TPLC100_01_AA].analogINrowCT = -1;
    lstTargets[TPLC100_01_AA].analogOUT = 2;
    lstTargets[TPLC100_01_AA].analogOUTrowCT = -1;
    lstTargets[TPLC100_01_AA].tAmbient = true;
    lstTargets[TPLC100_01_AA].rpmPorts = 0;
    lstTargets[TPLC100_01_AA].ser0_Enabled = true;
    lstTargets[TPLC100_01_AA].ser1_Enabled = true;
    lstTargets[TPLC100_01_AA].ser2_Enabled = true;
    lstTargets[TPLC100_01_AA].ser3_Enabled = true;
    lstTargets[TPLC100_01_AA].can1_Enabled = true;
    // 21 TPLC100_01_AB
    lstTargets[TPLC100_01_AB].modelName =  QString::fromAscii(product_name[TPLC100_01_AB]);
    lstTargets[TPLC100_01_AB].displayWidth =  -1;
    lstTargets[TPLC100_01_AB].displayHeight = -1;
    lstTargets[TPLC100_01_AB].usbPorts = 2;
    lstTargets[TPLC100_01_AB].sdCards = 0;
    lstTargets[TPLC100_01_AB].digitalIN = 4;
    lstTargets[TPLC100_01_AB].digitalOUT = 8;
    lstTargets[TPLC100_01_AB].nEncoders = 1;
    lstTargets[TPLC100_01_AB].analogIN = 12;
    lstTargets[TPLC100_01_AB].analogINrowCT = -1;
    lstTargets[TPLC100_01_AB].analogOUT = 2;
    lstTargets[TPLC100_01_AB].analogOUTrowCT = -1;
    lstTargets[TPLC100_01_AB].tAmbient = true;
    lstTargets[TPLC100_01_AB].rpmPorts = 0;
    lstTargets[TPLC100_01_AB].ser0_Enabled = true;
    lstTargets[TPLC100_01_AB].ser1_Enabled = true;
    lstTargets[TPLC100_01_AB].ser2_Enabled = true;
    lstTargets[TPLC100_01_AB].ser3_Enabled = true;
    lstTargets[TPLC100_01_AB].can1_Enabled = true;
    // 22 TPLC150
    lstTargets[TPLC150].modelName =  QString::fromAscii(product_name[TPLC150]);
    lstTargets[TPLC150].displayWidth =  -1;
    lstTargets[TPLC150].displayHeight = -1;
    lstTargets[TPLC150].usbPorts = 2;
    lstTargets[TPLC150].ethPorts = 2;
    lstTargets[TPLC150].sdCards = 0;
    lstTargets[TPLC150].digitalIN = 12;
    lstTargets[TPLC150].digitalOUT = 10;
    lstTargets[TPLC150].nEncoders = 1;
    lstTargets[TPLC150].analogIN = 12;
    lstTargets[TPLC150].analogINrowCT = -1;
    lstTargets[TPLC150].analogOUT = 4;
    lstTargets[TPLC150].analogOUTrowCT = -1;
    lstTargets[TPLC150].tAmbient = true;
    lstTargets[TPLC150].rpmPorts = 1;
    lstTargets[TPLC150].ser0_Enabled = true;
    lstTargets[TPLC150].ser1_Enabled = true;
    lstTargets[TPLC150].ser2_Enabled = true;
    lstTargets[TPLC150].ser3_Enabled = true;
    lstTargets[TPLC150].can1_Enabled = true;
}
int ctedit::searchModelInList(QString szModel)
// Ricerca il modello corrente nella Lista modelli attuale.
// Ritorna un # Modello e riempie il record globale TargetConfig
// E' garantito tornare sempre un # Modello valido (se non trovato vale 0 AnyTPAC)
{
    int nCur = 0;
    int nModel = -1;
    for (nCur = 0; nCur < lstTargets.count(); nCur++)  {
        if (lstTargets[nCur].modelName == szModel)  {
            nModel = nCur;
            break;
        }
    }
    // Valori di Default se la ricerca non ha trovato nulla
    if (nCur <= AnyTPAC && nCur >= lstTargets.count())
        nModel = AnyTPAC;
    // Imposta configurazione corrente
    TargetConfig = lstTargets[nModel];
    // Return value
    return nModel;
}
void ctedit::getFirstPortFromProtocol(int nProtocol, int &nPort, int &nTotal)
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
            if (TargetConfig.ser0_Enabled)  {
                nPort = 0;
                nTotal++;
            }
            if (TargetConfig.ser1_Enabled)  {
                if (nPort < 0)
                    nPort = 1;
                nTotal++;
            }
            if (TargetConfig.ser2_Enabled)  {
                if (nPort < 0)
                    nPort = 2;
                nTotal++;
            }
            if (TargetConfig.ser3_Enabled)  {
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
            if (TargetConfig.ethPorts > 0)  {
                nPort = szDEF_IP_PORT.toInt(0);
                nTotal++;
            }
            break;
        // Protocollo CAN
        case CANOPEN:
            if (TargetConfig.can0_Enabled)  {
                nPort = 0;
                nTotal++;
            }
            if (TargetConfig.can1_Enabled)  {
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
bool ctedit::updateRow(int nRow)
// Gestisce l'aggiornamento del grid con i valori letti da interfaccia di editing
{
    bool        fRes = true;
    QStringList lstFields;
    int         nErrors = 0;
    bool        fIsSaved = false;

    if (! isFormEmpty() && isLineModified(nRow))  {
        // Valori da interfaccia a Lista Stringhe
        fRes = iface2values(lstFields);
        // Primo controllo di coerenza sulla riga corrente
        nErrors = checkFormFields(nRow, lstFields, true);
        if (nErrors == 0)  {
            // Copia l'attuale CT nella lista Undo
            lstUndo.append(lstCTRecords);
            if (fRes)  {
                // Salva Record
                fIsSaved = list2CTrec(lstFields, nRow);
                // Aggiorna Grid Utente per riga corrente
                if (fIsSaved)  {
                    fRes = list2GridRow(lstFields, nRow);
                    m_isCtModified = true;
                    ui->tblCT->currentRow();
                }
                // qDebug() << "Row saved:" << nRow;
            }
        }
        else
            fRes = false;
    }
    // Return Value
    return fRes;
}
int ctedit::findNextVisibleRow(int nRow)
// Cerca la prossima riga visibile cui saltare
{
    int nJumpRow = 0;

    // Comincia la ricerca da 0 oppure dalla riga corrente + 1
    if (nRow < 0 || nRow >= DimCrossTable -1)
        nJumpRow = 0;
    else
        nJumpRow = nRow + 1;
    if (! m_fShowAllRows)  {
        while(nJumpRow < DimCrossTable - 1)  {
            if (lstCTRecords[nJumpRow].UsedEntry)  {
                break;
            }
            nJumpRow++;
        }
    }
    // Controllo del boundary sulla griglia
    if (nJumpRow >= DimCrossTable)
        nJumpRow = 0;
    // Return Value
    return nJumpRow;


}
bool ctedit::isValidPort(int nPort, int nProtocol)
// Controllo di validità di un numero porta in funzione di Modello (da TargetConfig) e del Protocollo
{
    bool    fRes = false;

    // Protocollo PLC dimesione pari a MAXBLOCKSIZE
    switch (nProtocol) {
        case PLC:
            fRes = nPort == -1;
            break;

        case TCP:
        case TCPRTU:
        case TCP_SRV:
        case TCPRTU_SRV:
        if ((nPort >= 0 && nPort <= nMax_Int16) && (TargetConfig.ethPorts > 0)) {
                if (nPort != nPortFTPControl && nPort != nPortSFTP && nPort != nPortTELNET && nPort != nPortHTTP && nPort != nPortVNC)  {
                    fRes = true;
                }
            }
            break;

        case CANOPEN:
            if (nPort >= 0 && nPort <= nMaxCanPorts)  {
                if (nPort == 0)
                    fRes = TargetConfig.can0_Enabled;
                else if (nPort == 1)
                    fRes = TargetConfig.can1_Enabled;
            }
            break;

        case RTU:
        case MECT_PTC:
        case RTU_SRV:
            if (nPort >= 0 && nPort <= nMaxSerialPorts)  {
                if ((nPort == 0 && TargetConfig.ser0_Enabled) ||
                    (nPort == 1 && TargetConfig.ser1_Enabled) ||
                    (nPort == 2 && TargetConfig.ser2_Enabled) ||
                    (nPort == 3 && TargetConfig.ser3_Enabled) )  {
                    fRes = true;
                }
            }
            break;

        default:
            fRes = false;
            break;
    }
    // Return Value
    return fRes;
}
bool ctedit::needSave()
// Return true if there are changes to be saved
{
    // Refresh Modif Flags for Config && Trends
    m_isConfModified = mectSet->isModified();
    m_isTrendModified = trendEdit->isModified();
    // Return value valid only if file is specified
    return ((m_isCtModified || m_isConfModified || m_isTrendModified) && ! m_szCurrentCTFile.isEmpty());
}
bool ctedit::querySave()
// Return true if pending changes are saved
{
    bool fRes = false;

    if (! m_szCurrentCTFile.isEmpty() && ! m_szCurrentModel.isEmpty()) {
        // Crosstable file
        if (m_isCtModified)  {
            m_szMsg = tr("Crosstable File has unsaved changes: Save?\n%1") .arg(m_szCurrentCTFile);
            if (queryUser(this, szMectTitle, m_szMsg, true))  {
                saveCTFile();
                fRes = true;
            }
        }
        m_isCtModified = false;
        // Configuration file
        if (m_isConfModified)  {
            m_szMsg = tr("Configuration File has unsaved changes: Save?\n%1") .arg(m_szCurrentCTPath + szINIFILE);
            if (queryUser(this, szMectTitle, m_szMsg, true))  {
                mectSet->saveMectSettings();
                fRes = true;
            }
        }
        m_isConfModified = false;
        // Trend File
        if (m_isTrendModified)  {
            m_szMsg = tr("Trend File has unsaved changes: Save?\n%1") .arg(trendEdit->currentTrendFile());
            if (queryUser(this, szMectTitle, m_szMsg, true))  {
                trendEdit->saveTrend(false);
                fRes = true;
            }
        }
        m_isTrendModified = false;
    }
    else
        fRes = true;
    // Return value
    return fRes;
}
bool ctedit::getRowsFromXMLBuffer(QString &szBuffer, QList<QStringList > &lstPastedRecords, QList<int> &lstSourceRows, QList<int> &lstDestRows)
{
    int             nBufferRows = 0;
    int             nCol = 0;
    int             nRow = 0;
    bool            fClipSourceOk = false;
    bool            fOk = false;
    QStringList     lstRecordFields;
    QXmlStreamReader xmlBuffer;
    QXmlStreamAttributes xmlAttrib;
    QString         xmlElementName;
    QString         xmlName;
    QString         xmlText;
    QString         xmlValue;

    lstPastedRecords.clear();
    lstSourceRows.clear();
    lstDestRows.clear();
    if (! szBuffer.isEmpty())  {
        // Create Row Entity buffer
        for (nCol = 0; nCol < lstHeadNames.count(); nCol++)  {
            lstRecordFields.append(szEMPTY);
        }
        // Link Clipboard buffer to XML Parser
        xmlBuffer.addData(szBuffer.toUtf8());
        // Skip della definizione di XML Document
        xmlBuffer.readNextStartElement();
        // XML Parsing
        while (! xmlBuffer.atEnd())  {
            xmlName = xmlBuffer.name().toString();
            xmlText = xmlBuffer.text().toString();
            if (xmlBuffer.isStartElement())  {
                // Read Elements Attributes
                xmlElementName = xmlBuffer.name().toString();
                xmlAttrib = xmlBuffer.attributes();
                // Parsing number of Rows
                if (xmlName == szXMLCTENAME)  {
                    fClipSourceOk = true;
                    xmlValue = xmlAttrib.value(szXMLCTNUMROWS).toString();
                    if (!xmlValue.isEmpty())  {
                        nBufferRows = xmlValue.toInt(&fOk);
                        nBufferRows = fOk ? nBufferRows : 0;
                        // qDebug() << tr("Number of Rows: %1") .arg(nBufferRows);
                    }
                }
                // Parsing Row Entity
                if (xmlElementName == szXMLCTROW)  {
                    // Fill Row Entity buffer
                    for (nCol = 0; nCol < lstHeadNames.count(); nCol++)  {
                        lstRecordFields[nCol] = xmlAttrib.value(lstHeadNames[nCol]).toString();
                    }
                    // Retrieve Row Source field
                    nRow = -1;
                    xmlValue = xmlAttrib.value(szXMLCTSOURCEROW).toString();
                    if (! xmlValue.isEmpty())  {
                        nRow = xmlValue.toInt(&fOk);
                        nRow = fOk ? nRow : -1;
                    }
                    lstSourceRows.append(nRow);
                    // Retrieve Row Destination field
                    nRow = -1;
                    xmlValue = xmlAttrib.value(szXMLCTDESTROW).toString();
                    if (! xmlValue.isEmpty())  {
                        nRow = xmlValue.toInt(&fOk);
                        nRow = fOk ? nRow : -1;
                    }
                    lstDestRows.append(nRow);
                    // Append Data to Pasting Lists
                    lstPastedRecords.append(lstRecordFields);
                    // qDebug() << tr("Row Variable: %1") .arg(lstRecordFields[colName]);
                }
            }
            xmlBuffer.readNext();
        }
        // Determina se i valori letti sono tra loro coerenti
        fClipSourceOk = fClipSourceOk && (lstPastedRecords.count() > 0) && (lstPastedRecords.count() == nBufferRows);
    }
    // Return value
    return fClipSourceOk;
}
bool ctedit::addModelVars(const QString szModelName, int nRow)
{
    QString     szFileName = szModelName + szXMLExt;
    bool fRes = false;
    QFile       fileXML(szMODELSPATH + szFileName);
    QString     szXMLBuffer;
    QList<QStringList > lstModelRows;
    QList<int>  lstSourceRows;
    QList<int>  lstDestRows;
    int         nCur = 0;
    int         nAdded = 0;
    bool        fUsed = false;

    if (fileXML.exists())  {
        fileXML.open(QIODevice::ReadOnly | QIODevice::Text);
        szXMLBuffer = QString::fromAscii(fileXML.readAll().data());
        // Check Buffer Length
        if (!szXMLBuffer.isEmpty())  {
            // Load Rows from Buffer
            fRes = getRowsFromXMLBuffer(szXMLBuffer, lstModelRows, lstSourceRows, lstDestRows);
            if (fRes)  {
                // Paste Rows in Current position
                if (nRow + lstModelRows.count() < MIN_DIAG - 1)  {
                    // Check free destination area
                    for (nCur = nRow; nCur < nRow + lstModelRows.count(); nCur++)  {
                        if (lstCTRecords[nCur].UsedEntry)  {
                            fUsed = true;
                            break;
                        }
                    }
                    // Query confirm of used if any row is used
                    if (fUsed)  {
                        m_szMsg = tr("Some of destination rows may be used. Paste anyway ?");
                        fUsed = ! queryUser(this, szMectTitle, m_szMsg);
                    }
                    if (! fUsed)  {
                        nAdded = addRowsToCT(nRow, lstModelRows, lstDestRows);
                        m_szMsg = tr("Added %1 Rows for Model: %2") .arg(nAdded) .arg(szModelName);
                    }
                }
                else  {
                    m_szMsg = tr("Pasted Rows could overlap to System Area");
                }
            }
            else {
                m_szMsg = tr("Error reading Variable File for Model: %1") .arg(szModelName);
            }
        }
        else {
            m_szMsg = tr("Empty Variable File for Model: %1") .arg(szModelName);
        }
    }
    else {
        m_szMsg = tr("Variable File Not Found for Model: %1") .arg(szModelName);
    }
    // Notify to user
    displayStatusMessage(m_szMsg);
    // Return Value
    return (nAdded > 0);
}
int ctedit::addRowsToCT(int nRow, QList<QStringList > &lstRecords2Add, QList<int> &lstDestRows)
{
    int     nCur = 0;
    int     nPasted = 0;
    int     nDestRow = 0;

    // Append to Undo List
    lstUndo.append(lstCTRecords);
    // Mark first destination row
    nCur = nRow;
    // Compile Selected Row List
    for (nPasted = 0; nPasted < lstRecords2Add.count(); nPasted ++)  {
        // Check Destination Row
        nDestRow = nRow++;
        if (lstDestRows[nPasted] > -1)
            nDestRow = lstDestRows[nPasted];
        // Force Block address to Row Number
        lstRecords2Add[nPasted][colBlock] = QString::number(nDestRow + 1);
        // Paste element
        list2CTrec(lstRecords2Add[nPasted], nDestRow);
        // Force Block Size to Var Size
        if (lstCTRecords[nDestRow].Protocol ==  PLC)
            lstCTRecords[nDestRow].BlockSize = 1;
        else
            lstCTRecords[nDestRow].BlockSize = varSizeInBlock(lstCTRecords[nDestRow].VarType);
        // Controllo Protocollo / Porta RTU
        int nProtocol = lstCTRecords[nDestRow].Protocol;
        if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
            int nPort = lstCTRecords[nDestRow].Port;
            int nTotal = 0;
            if (! isValidPort(nPort, nProtocol))  {
                getFirstPortFromProtocol(nProtocol, nPort, nTotal);
                if (nTotal > 0)
                    lstCTRecords[nDestRow].Port = nPort;
            }
        }
    }
    // Restore Grid Position to First Destination Row && select first row
    ui->tblCT->selectionModel()->clearSelection();
    m_nGridRow = nCur;
    ctable2Grid();
    m_isCtModified = true;
    jumpToGridRow(nCur + 1, true);
    jumpToGridRow(nCur, true);
    // Return value
    return (nPasted);
}
