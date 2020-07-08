#include "ctedit.h"
#include "ui_ctedit.h"
#include "utils.h"
#include "cteerrorlist.h"
#include "stdlib.h"
#include "cteUtils.h"
#include "messagelist.h"
#include "queryPortNode.h"

#include <QFile>
#include <QFileDialog>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QModelIndexList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QString>
#include <QLatin1String>
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
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QtAlgorithms>
#include <math.h>


/* ----  Local Defines:   ----------------------------------------------------- */
#define _TRUE  1
#define _FALSE 0

#define STD_DISPLAY_TIME 5
#define MAX_DISPLAY_TIME 15

// Tabs in TabWidget
#define TAB_CT 0
#define TAB_SYSTEM 1
#define TAB_TREND 2
#define TAB_DEVICES 3
#define TAB_TIMINGS 4
#define TAB_MPNC 5
#define TAB_MPNE 6
#define TAB_LOG 7

#undef  WORD_BIT


const QString szCT_FILE_NAME = QLatin1String("Crosstable.csv");
const QString szEMPTY_IP = QLatin1String("0.0.0.0");
const QString szCrossCompier = QLatin1String("ctc");
const QString szTemplateFile = QLatin1String("template.pri");
const QString szEnvFile = QLatin1String("EnvVars.txt");
const QString szPLCEnvVar = QLatin1String("PLCUNIXINSTPATH");
const QString szProExt = QLatin1String(".pro");
const QString szPLCFILE = QLatin1String("plc");
// const QString szPLCExt = QLatin1String(".txt");
const QString szPLCExt = QLatin1String(".4cp");
const QString szPLCDir = QLatin1String("plc");
const QString szINIFILE = QLatin1String("system.ini");
const QString szFileQSS = QLatin1String("C:/Qt485/desktop/lib/qtcreator/plugins/QtProject/CTE.qss");
// CrossTable originale del Modello corrente
const QString szTemplateCTFile = QLatin1String("C:/Qt485/desktop/share/qtcreator/templates/wizards/ATCM-template-project-%1/config/Crosstable.csv");

// Version Number
#ifndef ATCM_VERSION
#define ATCM_VERSION "DevelopmentVersion"
#endif
#define _STR(x) #x
#define STR(x) _STR(x)
const QString szVERSION = QLatin1String(STR(ATCM_VERSION));

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
    ui->lblMessage->setToolTip(szVERSION);
    // Style Sheet
    QFile fileQSS(szFileQSS);
    if (fileQSS.exists())  {
        fileQSS.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(fileQSS.readAll());
        fileQSS.close();
        this->setStyleSheet(styleSheet);
    }
    // Liste di servizio
    lstUsedVarNames.clear();
    lstLoggedVars.clear();
    lstUndo.clear();
    lstCTErrors.clear();
    lstMPNC.clear();                    // Liste delle capofila delle teste MPNC presenti
    m_nMPNC = -1;                       // Indice della Testa MPNC correntemente visualizzata
    lstMPNE.clear();                    // Liste delle capofila delle teste MPNE presenti
    m_nMPNE = -1;                       // Indice della Testa MPNE correntemente visualizzata
    // Colore Frame Editing
    colorNormalEdit = ui->fraEdit->palette().background().color();
    szColorNormalEdit =QString::fromLatin1("color: rgb(%1, %2, %3);")
            .arg(colorNormalEdit.red()) .arg(colorNormalEdit.green()) .arg(colorNormalEdit.blue());
    //------------------------
    // Riempimento liste
    //------------------------
    initLists();
    //------------------------
    // Lista Variabili per Modelli MPNx
    //------------------------
    lstMPNC006_Vars.clear();
    lstTPLC050_Vars.clear();
    lstMPNE_Vars.clear();
    // MPNC006
    if (readModelVars(szMPNC006, lstMPNC006_Vars))  {
        qDebug() << QString::fromAscii("Read Variables for Model: %1 - Variables: %2 - Max Var: %3 - Max Comment: %4")
                    .arg(szMPNC006) .arg(lstMPNC006_Vars.count()) .arg(lstMPNxHeadSizes[colMPNxName]) .arg(lstMPNxHeadSizes[colMPNxComment]);
    }  else  {
        qDebug() << QString::fromAscii("Error Reading Variables for Model: %1") .arg(szMPNC006);
        lstMPNC006_Vars.clear();
    }
    // TPLC050
    if (readModelVars(szTPLC050, lstTPLC050_Vars))  {
        qDebug() << QString::fromAscii("Read Variables for Model: %1 - Variables: %2 - Max Var: %3 - Max Comment: %4")
                    .arg(szMPNC006) .arg(lstMPNC006_Vars.count()) .arg(lstMPNxHeadSizes[colMPNxName]) .arg(lstMPNxHeadSizes[colMPNxComment]);
    }  else  {
        qDebug() << QString::fromAscii("Error Reading Variables for Model: %1") .arg(szTPLC050);
        lstTPLC050_Vars.clear();
    }
    // MPNE10
    if (readModelVars(szMPNE1001, lstMPNE_Vars))  {
        qDebug() << QString::fromAscii("Read Variables for Model: %1 - Variables: %2 - Max Var: %3 - Max Comment: %4")
                    .arg(szMPNC006) .arg(lstMPNC006_Vars.count()) .arg(lstMPNxHeadSizes[colMPNxName]) .arg(lstMPNxHeadSizes[colMPNxComment]);
    }  else  {
        qDebug() << QString::fromAscii("Error Reading Variables for Model: %1") .arg(szMPNE1001);
        lstMPNE_Vars.clear();
    }
    //------------------------
    // Lista Modelli
    //------------------------
    lstTargets.clear();
    initTargetList();
    panelConfig = lstTargets[AnyTPAC];
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
    lstErrorMessages[errCTNameTooLong] = trUtf8("Variable Name Too Long");
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
    lstErrorMessages[errCTRegisterTooBig] = trUtf8("Register Number too big for TCP Server");
    lstErrorMessages[errCTRegisterUsedTwice] = trUtf8("Register Number or Bit Position (Decimal) already used in Other Variables");
    lstErrorMessages[errCTInputOnlyModbusClient]  = trUtf8("Input Register allowed only on Modbus Client");
    lstErrorMessages[errCTModBusServerDuplicate] = trUtf8("Server Already present with different Port/Node");
    lstErrorMessages[errCTNoBehavior] = trUtf8("Empty or Invalid Behavior");
    lstErrorMessages[errCTNoBit] = trUtf8("Alarm/Event Variables must be of BIT type");
    lstErrorMessages[errCTBadPriorityUpdate] = trUtf8("Wrong Priority or Update for Alarm/Event Variable");
    lstErrorMessages[errCTNoVar1] = trUtf8("Empty or Invalid Left Condition Variable");
    lstErrorMessages[errCTNoCondition] = trUtf8("Empty or Invalid Alarm/Event Condition");
    lstErrorMessages[errCTRiseFallNotBit] = trUtf8("Rising/Falling Variable must be of BIT Type");
    lstErrorMessages[errCTInvalidNum] = trUtf8("Invalid Numeric Value in Alarm/Event Condition");;
    lstErrorMessages[errCTInvalidNum] = trUtf8("Invalid Numeric Value in Alarm/Event Condition");
    lstErrorMessages[errCTNegWithUnsigned] = trUtf8("Negative Value compared with Unsigned in Alarm/Event Condition");
    lstErrorMessages[errCTNoVar2] = trUtf8("Empty or Invalid Right Condition Variable");
    lstErrorMessages[errCTIncompatibleVar] = trUtf8("Incompatible Var Type or Decimals in Alarm/Event Condition");

    // Lista PLC (Frequenza Aggiornamento)
    lstAllUpdates.clear();
    lstNoHUpdates.clear();
    for (nCol = Htype; nCol < UPDATE_TOTALS; nCol++)  {
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
    lstAllVarTypes.clear();
    for (nCol = 0; nCol < TYPE_TOTALS - 1; nCol++)  {
        lstAllVarTypes.append(nCol);
    }
    // Lista Protocolli (tipi di Bus)
    lstBusEnabler.clear();
    for (nCol = PLC; nCol < FIELDBUS_TOTAL; nCol++)  {
        lstBusEnabler.append(true);         // Di default tutti i tipi di Bus sono abilitati
    }
    // Lista Prodotti
    lstProductNames.clear();
    for (nCol = AnyTPAC; nCol < MODEL_TOTALS; nCol++)  {
        lstProductNames.append(QLatin1String(product_name[nCol]));
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
    szToolTip.append(QLatin1String("Enabling the exchange of data with remote devices:\n"));
    szToolTip.append(QLatin1String("0 - Disabled\n"));
    szToolTip.append(QLatin1String("1 - Enabled High Priority\n"));
    szToolTip.append(QLatin1String("2 - Enabled Medium Priority\n"));
    szToolTip.append(QLatin1String("3 - Enabled Low Priority"));
    for  (nCol = 0; nCol < nNumPriority; nCol++)   {
        ui->cboPriority->addItem(lstPriority[nCol], lstPriority[nCol]);
    }
    ui->cboPriority->setToolTip(szToolTip);
    // Combo Update
    szToolTip.clear();
    szToolTip.append(QLatin1String("PLC Refresh Rate:\n"));
    szToolTip.append(QLatin1String("H - Only in current page\n"));
    szToolTip.append(QLatin1String("P - Permanent\n"));
    szToolTip.append(QLatin1String("S - Permanent Slow Logging\n"));
    szToolTip.append(QLatin1String("F - Permanent Fast Logging\n"));
    szToolTip.append(QLatin1String("V - Permanent Logging if Changed\n"));
    szToolTip.append(QLatin1String("X - Permanent Logging on Shot"));
    for  (nCol=0; nCol<lstUpdateNames.count(); nCol++)   {
        ui->cboUpdate->addItem(lstUpdateNames[nCol], lstUpdateNames[nCol]);
    }
    ui->cboUpdate->setToolTip(szToolTip);
    // ToolTip nome variabile
    szToolTip.clear();
    szToolTip.append(QLatin1String("Name of the variable to be used in HMI:\n"));
    szToolTip.append(QLatin1String("Must follow C++ variable naming conventions"));
    ui->txtName->setToolTip(szToolTip);
    // Combo TipoCrossTable
    szToolTip.clear();
    szToolTip.append(QLatin1String("Variable Data Type"));
    for  (nCol=0; nCol<lstTipi.count(); nCol++)   {
        ui->cboType->addItem(lstTipi[nCol], lstTipi[nCol]);
    }
    ui->cboType->setToolTip(szToolTip);
    // Decimals
    szToolTip.clear();
    szToolTip.append(QLatin1String("Number of decimal places to display data if from 0 to 4,\n"));
    szToolTip.append(QLatin1String("otherwise is the position of the variable that contains the number of decimal places\n"));
    szToolTip.append(QLatin1String("In the case of BIT variables, it is the position of the bit as low as 1"));
    ui->txtDecimal->setToolTip(szToolTip);
    // Combo Tipo Bus
    szToolTip.clear();
    szToolTip.append(QLatin1String("Protocol with the remote device"));
    for  (nCol=0; nCol<lstProtocol.count(); nCol++)   {
        ui->cboProtocol->addItem(lstProtocol[nCol], lstProtocol[nCol]);
    }
    ui->cboProtocol->setToolTip(szToolTip);
    // Indirizzo IP
    szToolTip.clear();    // Lista Priorità
    szToolTip.append(QLatin1String("IP address for TCP, TCP_RTU, TCP_SRV, TCPRTU SRV like 192.168.0.42"));
    ui->txtIP->setToolTip(szToolTip);
    // Porta di comunicazione
    szToolTip.clear();
    szToolTip.append(QLatin1String("TCP Communication Port, e.g. 502") );
    ui->txtPort->setToolTip(szToolTip);
    // Combo Porta seriale
    szToolTip.clear();
    szToolTip.append(QString::fromAscii("Serial Communication Port, from 0 to %1") .arg(nMaxSerialPorts));
    ui->cboPort->setToolTip(szToolTip);
    // Indirizzo nodo remoto
    szToolTip.clear();
    szToolTip.append(QLatin1String("Remode Node Address 0..255\n"));
    szToolTip.append(QLatin1String("\tFor RTU 1..247\n"));
    szToolTip.append(QLatin1String("\tFor TCP 0 or 255"));
    ui->txtNode->setToolTip(szToolTip);
    // Check Input Register
    szToolTip.clear();
    szToolTip.append(QLatin1String("For Modbus Client:\n"));
    szToolTip.append(QLatin1String("Check to mark as INPUT REGISTER\n"));
    szToolTip.append(QLatin1String("instead of HOLDING REGISTER\n"));
    ui->chkInputRegister->setToolTip(szToolTip);
    // Register
    szToolTip.clear();
    szToolTip.append(QLatin1String("For Modbus:\n"));
    szToolTip.append(QLatin1String("ADR=[0..65535]\n\n"));
    szToolTip.append(QLatin1String("For Modbus Server: (RTU_SRV, TCP_SRV or TCPRTU_SRV):\n"));
    szToolTip.append(QLatin1String("ADR=[0..4095]"));
    ui->txtRegister->setToolTip(szToolTip);
    // Block
    szToolTip.clear();
    szToolTip.append(QLatin1String("Address of the first register of the block"));
    ui->txtBlock->setToolTip(szToolTip);
    // Block size
    szToolTip.clear();
    szToolTip.append(QLatin1String("Number of registers in the block"));
    ui->txtBlockSize->setToolTip(szToolTip);
    // Combo Behavior
    szToolTip.clear();
    szToolTip.append(QLatin1String("Variable behavior: Reading, Reading and Writing, Alarm, Event"));
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
    for (nCol=0; nCol<lstCondition.count(); nCol++)   {
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
    theServersNumber = 0;
    theDevicesNumber = 0;
    theTcpDevicesNumber = 0;
    theNodesNumber = 0;
    thePlcVarsNumber = 0;
    theBlocksNumber = 0;
    m_fMultiSelect = false;
    m_fMultiEdit = false;
    ui->lblEditableFields->setVisible(false);
    ui->lstEditableFields->setVisible(false);
    ui->cmdApply->setVisible(false);
    lstSelectedRows.clear();
    // Validator per Interi
    ui->txtDecimal->setValidator(new QIntValidator(nValMin, DimCrossTable, this));
    ui->txtPort->setValidator(new QIntValidator(nValMin, nMax_Int16, this));
    ui->txtNode->setValidator(new QIntValidator(0, nMaxNodeID, this));
    ui->txtRegister->setValidator(new QIntValidator(nValMin, nMax_Int16, this));
    ui->txtBlock->setValidator(new QIntValidator(nValMin, nValMax, this));
    ui->txtBlockSize->setValidator(new QIntValidator(nValMin, nValMax, this));
    // Validatori per Double
    ui->txtFixedValue->setValidator(new QDoubleValidator(this));
    // Validator per txtIp
    QString szExp = QString::fromAscii("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
    QRegExp regExprIP(szExp);
    ui->txtIP->setValidator(new QRegExpValidator(regExprIP, this));
    // Validator per Nome variabile e Comment
    QString szNameExp = QLatin1String("\\w+");
    QRegExp regExprName(szNameExp);
    ui->txtName->setValidator(new QRegExpValidator(regExprName, this));
    ui->txtName->setMaxLength(MAX_IDNAME_LEN);
    // Validator per commenti
    QString szCommentExp = QString::fromAscii("^[^\\\\/:\\\[\\\]\$;,?\"'<>|]*$");
    QRegExp regExprComment(szCommentExp);
    if (regExprComment.isValid())  {
        ui->txtComment->setValidator(new QRegExpValidator(regExprComment, this));
//        qDebug() << QString::fromLatin1("Valid RegExp: %1") .arg(szCommentExp);
    }
    else  {
        ui->txtComment->setValidator(new QRegExpValidator(regExprName, this));
//        qDebug() << QString::fromLatin1("Invalid RegExp: %1") .arg(szCommentExp);
    }
    ui->txtComment->setMaxLength(MAX_COMMENT_LEN - 1);
    // Campi sempre locked
    ui->txtRow->setEnabled(false);
    ui->txtBlock->setEnabled(false);
    ui->txtBlockSize->setEnabled(false);
    // Model Name
    ui->lblModel->setText(szEMPTY);
    // Lista dei campi editabili in ListView
    // QString szStyleItem = "QListWidget :: Item: hover {background: skyblue;} QListWidget :: item: selected {background: darkblue; color: yellow;}";
    lstEditableFields.clear();
    ui->lstEditableFields->clear();
    connect(ui->lstEditableFields, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(highlightEditableFields(QListWidgetItem*)));
    QStringList lstItems;
    lstItems.append(lstHeadCols[colPriority]);
    lstItems.append(lstHeadCols[colUpdate]);
    lstItems.append(lstHeadCols[colType]);
    lstItems.append(lstHeadCols[colDecimal]);
    lstItems.append(lstHeadCols[colProtocol]);
    lstItems.append(lstHeadCols[colIP]);
    lstItems.append(lstHeadCols[colPort]);
    lstItems.append(lstHeadCols[colNodeID]);
    lstItems.append(lstHeadCols[colRegister]);
    lstItems.append(lstHeadCols[colBehavior]);
    lstItems.append(lstHeadCols[colComment]);
    ui->lstEditableFields->addItems(lstItems);
    QListWidgetItem* listItem = 0;
    for (nCol = 0; nCol < lstItems.count(); nCol++)  {
        listItem = ui->lstEditableFields->item(nCol);
        listItem->setFlags(listItem->flags() |  Qt::ItemIsUserCheckable);
        // Abilitazione campi di default
        if (listItem->text() == lstHeadCols[colPriority] ||
            listItem->text() == lstHeadCols[colUpdate]   ||
            listItem->text() == lstHeadCols[colProtocol] ||
            listItem->text() == lstHeadCols[colNodeID]   ||
            listItem->text() == lstHeadCols[colBehavior] )    {
            listItem->setCheckState(Qt::Checked);
        }
        else  {
            listItem->setCheckState(Qt::Unchecked);
        }
    }
    // Stringhe generiche per gestione dei formati di Data e ora
    m_szFormatDate = QLatin1String("yyyy.MM.dd");
    m_szFormatTime = QLatin1String("hh:mm:ss");
    // Variabili di stato globale dell'editor
    m_isCtModified = false;
    m_isConfModified = false;
    m_isTrendModified = false;
    m_fShowAllRows = true;
    m_fCutOrPaste = false;
    m_rebuildDeviceTree = true;    // Flag Vero se l'albero dei Device deve essere ricostruito
    m_rebuildTimingTree = true;    // Flag Vero se l'albero dei Device deve essere ricostruito
    m_vtAlarmVarType = UNKNOWN;
    m_nAlarmDecimals = -1;
    // Creazione del Tab per il System Editor
    QVBoxLayout *vbSystem = new QVBoxLayout(ui->tabSystem);
    mectSet = new MectSettings(ui->tabSystem);
    vbSystem->addWidget(mectSet);
    // Creazione del Tab per il Trend Editor
    QVBoxLayout *vbTrend = new QVBoxLayout(ui->tabTrend);
    trendEdit = new TrendEditor(ui->tabTrend);
    vbTrend->addWidget(trendEdit);
    // Creazione del Tab per il configuratore MPNC
    QVBoxLayout *vbMPNC= new QVBoxLayout(ui->tabMPNC);
    configMPNC = new Config_MPNC(ui->tabMPNC);
    vbMPNC->addWidget(configMPNC);
    // Creazione del Tab per il configuratore MPNE
    QVBoxLayout *vbMPNE= new QVBoxLayout(ui->tabMPNE);
    configMPNE = new Config_MPNE(ui->tabMPNE);
    vbMPNE->addWidget(configMPNE);
    // Creazione Form ricerca
    searcForm = new SearchVariable(this);

    // Abilitazione dei Tabs
    m_nCurTab = TAB_CT;
    ui->tabWidget->setTabEnabled(TAB_CT, true);
    ui->tabWidget->setTabEnabled(TAB_SYSTEM, true);
    ui->tabWidget->setTabEnabled(TAB_TREND, true);
    ui->tabWidget->setTabEnabled(TAB_DEVICES, true);
    ui->tabWidget->setTabEnabled(TAB_TIMINGS, true);
    ui->tabWidget->setTabEnabled(TAB_MPNC, true);
    ui->tabWidget->setTabEnabled(TAB_MPNE, true);
    ui->tabWidget->setTabEnabled(TAB_LOG, true);
    ui->tabWidget->setCurrentIndex(TAB_CT);

    // Connessione Segnali - Slot
    ui->tblCT->setCornerButtonEnabled(false);
    ui->tblCT->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tblCT, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(displayUserMenu(const QPoint &)));
    // connect(ui->tblCT, SIGNAL(clicked(QModelIndex)), this, SLOT(itemClicked(QModelIndex)));
    connect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
            SLOT(tableCTItemChanged(const QItemSelection &, const QItemSelection & ) ));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected(int)));
    connect(ui->deviceTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(treeItemDoubleClicked(QModelIndex)));
    connect(ui->timingTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(treeItemDoubleClicked(QModelIndex)));
    connect(ui->logTree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(treeItemDoubleClicked(QModelIndex)));
    // Tab MPNC
    connect(configMPNC, SIGNAL(varClicked(int)), this, SLOT(return2GridRow(int)));
    connect(configMPNE, SIGNAL(varClicked(int)), this, SLOT(return2GridRow(int)));
    // Timer per messaggi
    tmrMessage = new QTimer(this);
    tmrMessage->setInterval(0);
    tmrMessage->setSingleShot(true);
    connect(tmrMessage, SIGNAL(timeout()), this, SLOT(clearStatusMessage()));
    // Event Filter    
    ui->tblCT->installEventFilter(this);
    ui->fraEdit->installEventFilter(this);
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
    QDir        projectDir(szProjectPath);
    QString     szProjectName;
    QString     szTemplates;

    if (projectDir.exists() && ! szProjectPath.isEmpty())  {
        projectDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
        QStringList filesList;
        filesList.append(QLatin1String("*.pro"));
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
    //qDebug() << "Project Name:" << m_szCurrentProjectName;
    //qDebug() << "PLC Path:" << m_szCurrentPLCPath;
}

bool    ctedit::selectCTFile(QString szFileCT)
// Select a current CT File
{
    QString     szFile;
    bool        fRes = false;
    int         nModel = AnyTPAC;
    int         nErr = 0;

    if (fileExists(szFileCT)) {
        szFile = szFileCT;
    }
    else {
        szFile = QFileDialog::getOpenFileName(this, QLatin1String("Open Crosstable File"), szFileCT, QString::fromAscii("Crosstable File (%1)") .arg(szCT_FILE_NAME));
    }
    // Tries to Open CT File
    if (! szFile.isEmpty())   {
        m_szCurrentCTFile = szFile;
        fRes = loadCTFile(m_szCurrentCTFile, lstCTRecords, true);
        if (fRes)
            qDebug() << QString::fromAscii("Loaded CT File: %1 Result: OK") .arg(szFile);
        else
            qDebug() << QString::fromAscii("Loaded CT File: %1 Result: ERROR") .arg(szFile);
    }
    else  {
        m_szCurrentCTFile.clear();
        fRes = false;
        qDebug() << QString::fromAscii("CT File: %1 Not Found or not Selected") .arg(szFile);
    }
    // Retrieving Path and Name of Cross Table file
    m_szCurrentModel.clear();
    m_szCurrentCTPath.clear();
    m_szCurrentCTName.clear();
    m_szTemplateCTFile.clear();
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
            // qDebug() << QString::fromAscii("Model in List: %1") .arg(nModel);
            nModel = searchModelInList(m_szCurrentModel);
            ui->lblModel->setStyleSheet(QLatin1String("background-color: LightCyan"));
            ui->lblModel->setToolTip(getModelInfo(nModel));
            qDebug() << QString::fromAscii("selectCTFile: Model Code: <%1> Model No <%2>") .arg(m_szCurrentModel) .arg(nModel);
            m_szTemplateCTFile = QString(szTemplateCTFile) .arg(m_szCurrentModel);
        }
        else  {
            nModel = AnyTPAC;
        }
        // Imposta configurazione corrente
        panelConfig = lstTargets[nModel];
        // Se il modello non è stato trovato in template.pri oppure nella lista modelli definiti vale comunque AnyTPAC ma il salvataggio è disabilitato
        // (Viene ripulita la variabile m_szCurrentModel)
        if (nModel == AnyTPAC)  {
            ui->lblModel->setStyleSheet(QLatin1String("background-color: Red"));
            m_szMsg = QString::fromAscii("No Model Type Found in file: <%1>\nor Model Unknown: <%2>") .arg(szTemplateFile) .arg(m_szCurrentModel);
            warnUser(this, szMectTitle, m_szMsg);
            m_szCurrentModel.clear();
            m_szTemplateCTFile.clear();
        }
        m_isCtModified = false;
        m_fCutOrPaste = false;
        qDebug() << QString::fromAscii("Original Template File: %1") .arg(m_szTemplateCTFile);
        // Carica anche le impostazioni del file INI
        mectSet->loadProjectFiles(m_szCurrentCTPath + szINIFILE, m_szCurrentProjectPath + szSLASH + m_szCurrentProjectName, m_szCurrentProjectPath + szSLASH, panelConfig);
//        qDebug() << QString::fromAscii("LoadProjectFiles: <%1> Done") .arg(m_szCurrentModel) ;
        // Rilegge all'indetro le info di configurazione eventualmente aggiornate da system.ini
        mectSet->getTargetConfig(panelConfig);
//        qDebug() << QString::fromAscii("getTargetConfig: <%1> Done") .arg(TargetConfig.modelName) ;
        // Aggiorna le abilitazioni dei protocolli in funzione delle porte abilitate
        enableProtocolsFromModel();
//        qDebug() << QString::fromAscii("enableProtocolsFromModel: <%1> Done") .arg(TargetConfig.modelName) ;
        // Se tutto Ok, carica anche il primo trend utile
        QString szFileTemplate;
        szFileTemplate = m_szCurrentProjectPath;
        szFileTemplate.append(szSLASH);
        szFileTemplate.append(szTemplateFile);
        trendEdit->updateVarLists(lstLoggedVars);
        trendEdit->setTrendsParameters(m_szCurrentModel, m_szCurrentCTPath, szEMPTY, szFileTemplate);
        // Caricamento del file da Template
        if (! m_szTemplateCTFile.isEmpty())  {
            if (loadCTFile(m_szTemplateCTFile, lstTemplateRecs, false))  {
                // Confronto tra CT del Template e CT corrente
                QList<int> lstDifferences;
                QStringList lstActions;
                // Clessidra
                this->setCursor(Qt::WaitCursor);
                int nDiff = compareCTwithTemplate(lstCTRecords, lstTemplateRecs, panelConfig, lstDifferences, lstActions, false);
                if (nDiff > 0)  {
                    QStringList lstRow;
                    QList<QStringList>  lstMessages;
                    QList<int>  lstColSizes;
                    // Preparing Header Row
                    lstRow.append(QLatin1String("Row"));
                    lstRow.append(QLatin1String("Project Variable"));
                    lstRow.append(QLatin1String("Template Variable"));
                    lstRow.append(QLatin1String("Action"));
                    // Preparing Col Sizes
                    lstColSizes.append(- nRowColWidth);
                    lstColSizes.append(lstHeadSizes[colName]);
                    lstColSizes.append(lstHeadSizes[colName]);
                    lstColSizes.append(lstHeadSizes[colName] * 2);
                    // Adding Header Row
                    lstMessages.append(lstRow);
                    m_szMsg = QString::fromAscii("Found [%1] Differences between Project and [%2] Template Cross Table file\n\nAlign Project to Template ?")
                            .arg(nDiff) .arg(m_szCurrentModel);
                    for (nErr = 0; nErr < lstDifferences.count(); nErr++)  {
                        int nPos = lstDifferences[nErr];
                        QString szCTVar = QString::fromAscii(lstCTRecords[nPos].Tag).trimmed();
                        QString szTemplVar = QString::fromAscii(lstTemplateRecs[nPos].Tag).trimmed();
                        szCTVar = szCTVar.isEmpty() ? QLatin1String(" --- ") : szCTVar;
                        szTemplVar = szTemplVar.isEmpty() ? QLatin1String(" --- ") : szTemplVar;
                        // Formatting Row
                        lstRow.clear();
                        lstRow.append(QString::fromAscii("%1") .arg(nPos + 1, 5, 10));
                        lstRow.append(szCTVar);
                        lstRow.append(szTemplVar);
                        lstRow.append(lstActions[nErr]);
                        lstMessages.append(lstRow);
                    }
                    // Cursore Normale
                    this->setCursor(Qt::ArrowCursor);
                    // Repaint
                    doEvents();
                    // Query User
                    messageList *resList = new messageList(szMectTitle, m_szMsg, lstMessages, lstColSizes);
                    resList->setModal(true);
                    int nResList = resList->exec();
                    if (nResList == QDialog::Accepted)  {
                        // Clessidra
                        this->setCursor(Qt::WaitCursor);
                        doEvents();
                        lstUndo.append(lstCTRecords);
                        nDiff = compareCTwithTemplate(lstCTRecords, lstTemplateRecs, panelConfig, lstDifferences, lstActions, true);
                        qDebug() << QLatin1String("Applied Differences to CT from Template");
                        ctable2Grid();
                        saveCTFile();

                    }
                }
            }
        }
        // Cursore Normale
        this->setCursor(Qt::ArrowCursor);
        // Abilita interfaccia
        m_fMultiSelect = false;
        enableInterface();
        // Controllo errori globali su Cross Table
        nErr = globalChecks();
        qDebug() << QString::fromAscii("Load Errors: <%1>") .arg(nErr);
        if (nErr > 0)  {
            m_szMsg = QString::fromAscii("Found Errors Loading CrossTable file: %1\n Total Erros: %2") .arg(szFileCT) .arg(nErr);
            warnUser(this, szMectTitle, m_szMsg);
        }
        m_isCtModified = false;
        // Porta in primo piano il Tab delle Variabili
        ui->tabWidget->setCurrentIndex(TAB_CT);
    }
    return fRes;
}
bool    ctedit::loadCTFile(QString szFileCT, QList<CrossTableRecord> &lstCtRecs, bool fLoadGrid)
// Load the current CT File. If fLoadGrid then load data to user Grid
{
    int     nRes = 0;
    int     nCur = 0;
    int     nTotalRows = 0;
    bool    fRes = false;

    if (szFileCT.isEmpty())
        return false;
    // Clear Data
    lstCtRecs.clear();
    this->setCursor(Qt::WaitCursor);
    // Reset Show Flag && Current Row Index
    m_fShowAllRows = true;
    m_fMultiSelect = false;
    lstSelectedRows.clear();
    m_nGridRow = -1;
    // Opening File
    nRes = LoadXTable(szFileCT.toAscii().data(), &CrossTable[0], &nTotalRows);
    // Return value is the result of Parsing C structure to C++ Objects.
    // Data in Array starts from element #1, in list from 0...
    // The Loading is Ok if nRes == 0 or there are enough rows of CT format
    // Former checking will follow
    if (nRes == 0 || nTotalRows == DimCrossTable)  {
        // Nessun errore oppure errori di secondo livello del Parser
        for (nCur = 1; nCur <= DimCrossTable; nCur++)  {
            lstCtRecs.append(CrossTable[nCur]);
        }
        // Load new records in Interface Grid
        if (fLoadGrid)  {
            fRes = ctable2Grid();
            m_rebuildDeviceTree = true;
            m_rebuildTimingTree = true;
        }
        else
            fRes = true;
    }
    else  {
        m_szMsg = QString::fromAscii("Error Loading CrossTable file: %1\n At Row: %2\n\nCannot continue") .arg(szFileCT) .arg(nTotalRows);
        warnUser(this, szMectTitle, m_szMsg);
    }
    this->setCursor(Qt::ArrowCursor);
    return fRes;
}
bool    ctedit::ctable2Grid()
// Lettura di tutta la CT in Grid
{
    bool            fRes = true;
    int             nCur = 0;
    QStringList     lstFields;
    QStringList     lstRowNumbers;
    QBrush          brushRed(Qt::SolidPattern);


    lstFields.clear();
    lstRowNumbers.clear();
    lstUsedVarNames.clear();
    lstLoggedVars.clear();
    brushRed.setColor(Qt::red);
    // Preparazione tabella
    this->setCursor(Qt::WaitCursor);
    disableAndBlockSignals(ui->tblCT);
    ui->tblCT->setEnabled(false);
    ui->tblCT->clearSelection();
    ui->tblCT->setRowCount(0);
    ui->tblCT->clear();
    ui->tblCT->setColumnCount(colTotals);
    // Caricamento elementi
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        // Convert CT Record 2 User Values
        fRes = recCT2FieldsValues(lstCTRecords, lstFields, nCur);
        // If Ok add row to Table View
        if (fRes)  {
            ui->tblCT->insertRow(nCur);
            fRes = list2GridRow(ui->tblCT, lstFields, lstHeadLeftCols, nCur);
            // Imposta Numero Riga
            lstRowNumbers.append(QString::fromAscii("%1") .arg(nCur + 1, 5, 10));
            // Evidenzia gli Input Registers in Rosso
            if (lstFields[colInputReg] == szTRUE)  {
                setCellForeground(brushRed, ui->tblCT->model(), nCur, colRegister);
            }
        }
    }
    // qDebug() << QString::fromAscii("Loaded Rows: %1") .arg(nCur);
    // Impostazione parametri TableView
    ui->tblCT->setVerticalHeaderLabels(lstRowNumbers);
    setGridParams(ui->tblCT, lstHeadCols, lstHeadSizes, QAbstractItemView::ExtendedSelection);
    // Nascondi Colonne Input Registers, Group, Module
    ui->tblCT->setColumnHidden(colInputReg, true);
    ui->tblCT->setColumnHidden(colGroup, true);
    ui->tblCT->setColumnHidden(colModule, true);
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
        qDebug() << QLatin1String("Error Loading Rows");
    }
    // Return value
    enableAndUnlockSignals(ui->tblCT);
    this->setCursor(Qt::ArrowCursor);
    qDebug("ctable2Grid(): Result %d", fRes);
    return fRes;
}

void ctedit::on_cmdHideShow_toggled(bool checked)
{

    // Titolo del Bottone
    if (checked)  {
        m_fShowAllRows = true;
        // ui->cmdHideShow->setText(QLatin1String("Hide"));
    }
    else  {
        m_fShowAllRows = false;
        // ui->cmdHideShow->setText(QLatin1String("Show"));
    }
    // Show-Hide Rows
    showAllRows(m_fShowAllRows);
}
bool ctedit::values2Iface(QStringList &lstRecValues, int nRow)
// Copia Lista Stringhe convertite da CT Record a Zona di Editing
// Caricamento Lista valori stringa su area editing
{
    QString szTemp;
    int     nPos = 0;
    int     nProtocol = -1;
    int     nDec = 0;
    bool    fOk = false;
    int     nLeftVar = 0;
    QList<int> lstVarTypes;
    bool    fMultiEdit = m_fMultiEdit && m_fMultiSelect;

    lstVarTypes.clear();
    m_vtAlarmVarType = UNKNOWN;
    m_nAlarmDecimals = -1;
    // Row #
    szTemp = QString::number(nRow + 1);
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
    // Group (Per ora ignorato)
    szTemp = lstRecValues[colGroup].trimmed();
    // Module (Per ora ignorato)
    szTemp = lstRecValues[colModule].trimmed();
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
    nDec = szTemp.toInt(&fOk);
    nDec = fOk ? nDec : 0;
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
    // Input Register
    szTemp = lstRecValues[colInputReg].trimmed();
    ui->chkInputRegister->setChecked(szTemp == szTRUE);
    // Register
    szTemp = lstRecValues[colRegister].trimmed();
    ui->txtRegister->setText(szTemp);
    // Block
    szTemp = lstRecValues[colBlock].trimmed();
    ui->txtBlock->setText(szTemp);
    // Dimensione Blocco
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
        if (nPos >= 0 && nPos < ui->cboBehavior->count())  {
            ui->cboBehavior->setCurrentIndex(nPos);
        }
        // Caricamento ulteriori elementi interfaccia Allarmi / Eventi
        if (! fMultiEdit && nPos >= behavior_alarm)  {
            // Clear Data Entry Form for Alarm/Variables
            // Seleziona tutte le variabili tranne le H lstAllVarTypes a prescindere dallo stato della variabile
            ui->cboVariable1->setCurrentIndex(-1);
            ui->cboVariable2->setCurrentIndex(-1);
            fillComboVarNames(ui->cboVariable1, lstAllVarTypes, lstNoHUpdates, true);
            fillComboVarNames(ui->cboVariable2, lstAllVarTypes, lstNoHUpdates, true);
            ui->txtFixedValue->setText(szEMPTY);
            // Ricerca posizione prima variabile
            szTemp = lstRecValues[colSourceVar].trimmed();
            // qDebug() << "Alarm Source Variable:" << szTemp;
            nPos = -1;
            nDec = 4;
            nLeftVar = -1;
            if (! szTemp.isEmpty())  {
                nPos = ui->cboVariable1->findText(szTemp, Qt::MatchExactly);
                nLeftVar = varName2Row(szTemp, lstCTRecords);
                // Recupera il Numero Decimali per la formattazione del valore fixed
                if (nLeftVar > 0)  {
                    nDec = lstCTRecords[nLeftVar].Decimal;
                }
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
                        // Formattazione del Numero in funzione dei Decimali della variabile di source
                        QLocale cLocale(QLocale::C);
                        double dVal = cLocale.toDouble(szTemp, &fOk);
                        dVal = fOk ? dVal : 0.0;
                        // Valore numerico in Stringa formattata in funzione dei decimali previsti
                        if (lstCTRecords[nLeftVar].VarType == BYTE_BIT ||
                            lstCTRecords[nLeftVar].VarType  == WORD_BIT ||
                            lstCTRecords[nLeftVar].VarType  == DWORD_BIT)  {
                            szTemp = QString::number(dVal, 'f', nCompareDecimals);
                        }
                        else  {
                            szTemp = QString::number(dVal, 'f', nDec);
                        }
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
bool ctedit::iface2values(QStringList &lstRecValues, bool fMultiEdit, int nRow)
// Copia da Zona Editing a Lista Stringhe per Controlli, Grid e Record CT
// Se è in corso un MultiLine Edit, recupera il valore direttamente dal Grid
{
    QString szTemp;
    int     nPos = 0;
    int     nUpdate = -1;
    int     nProtocol = -1;
    int     nType = -1;

    // Pulizia Buffers
    szTemp.clear();
    listClear(lstRecValues, colTotals);
    // Priority
    if (! fMultiEdit || lstEditableFields.indexOf(colPriority) >= 0)  {
        nPos = ui->cboPriority->currentIndex();
        if (nPos>= 0 && nPos < nNumPriority)  {
            szTemp = ui->cboPriority->itemData(nPos).toString();
        }
        else  {
            szTemp = szEMPTY;
        }
        lstRecValues[colPriority] = szTemp.trimmed();
    }
    else  {
        lstRecValues[colPriority] = ui->tblCT->item(nRow, colPriority)->text().trimmed();
    }
    // Update
    if (! fMultiEdit || lstEditableFields.indexOf(colUpdate) >= 0)  {
        nUpdate = ui->cboUpdate->currentIndex();
        if (nUpdate >= 0 && nPos < lstUpdateNames.count())  {
            szTemp = ui->cboUpdate->itemData(nUpdate).toString();
        }
        else  {
            szTemp = szEMPTY;
        }
        lstRecValues[colUpdate] = szTemp;
    }
    else  {
        lstRecValues[colUpdate] = ui->tblCT->item(nRow, colUpdate)->text().trimmed();
    }
    // Gruppo, per ora forzato a 0
    lstRecValues[colGroup] = szZERO;
    // Gruppo, per ora forzato a 0
    lstRecValues[colModule] = szZERO;
    // Name (No per MultiSelect)
    if (! fMultiEdit)  {
        szTemp = ui->txtName->text().trimmed();
        lstRecValues[colName] = szTemp;
    }
    else  {
        lstRecValues[colName] = ui->tblCT->item(nRow, colName)->text().trimmed();
    }
    // Update Used and Logged Variable List
    if (! szTemp.isEmpty() && ! fMultiEdit)  {
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
    if (! fMultiEdit || lstEditableFields.indexOf(colType) >= 0)  {
        nType = ui->cboType->currentIndex();
        if (nType >= 0 && nType < lstTipi.count())  {
            szTemp = ui->cboType->itemData(nType).toString();
        }
        else  {
            szTemp = szEMPTY;
        }
        lstRecValues[colType] = szTemp.trimmed();
    }
    else  {
        lstRecValues[colType] = ui->tblCT->item(nRow, colType)->text().trimmed();
    }
    // Decimal  (No per MultiSelect se di tipo BIT)
    if (! fMultiEdit || (lstEditableFields.indexOf(colDecimal) >= 0 && ! isBitField((varTypes) nType)))  {
        szTemp = ui->txtDecimal->text();
        lstRecValues[colDecimal] = szTemp.trimmed();
    }
    else  {
        lstRecValues[colDecimal]  = ui->tblCT->item(nRow, colDecimal)->text().trimmed();
    }
    // Protocol lstBusType
    if (! fMultiEdit || lstEditableFields.indexOf(colProtocol) >= 0)  {
        nProtocol = ui->cboProtocol->currentIndex();
        if (nProtocol >= 0 && nProtocol < lstProtocol.count())  {
            szTemp = ui->cboProtocol->itemData(nProtocol).toString();
        }
        else  {
            szTemp = szEMPTY;
        }
    }
    else  {
        szTemp = ui->tblCT->item(nRow, colProtocol)->text().trimmed();
        nProtocol = ui->cboProtocol->findText(szTemp);
    }
    lstRecValues[colProtocol] = szTemp.trimmed();
    // IP
    // Protocol lstBusType
    if (! fMultiEdit || lstEditableFields.indexOf(colIP) >= 0)  {
        szTemp = ui->txtIP->text();
        lstRecValues[colIP] = szTemp.trimmed();
    }
    else  {
        lstRecValues[colIP]  = ui->tblCT->item(nRow, colIP)->text().trimmed();
    }
    // Port
    if (! fMultiEdit || lstEditableFields.indexOf(colPort) >= 0)  {
        // Protocolli Seriali
        if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
            szTemp = ui->cboPort->currentText();
        }
        else  {
            szTemp = ui->txtPort->text();
        }
        lstRecValues[colPort] = szTemp.trimmed();
    }
    else  {
        lstRecValues[colPort]  = ui->tblCT->item(nRow, colPort)->text().trimmed();
    }
    // Node ID
    if (! fMultiEdit || lstEditableFields.indexOf(colNodeID) >= 0)  {
        szTemp = ui->txtNode->text();
        lstRecValues[colNodeID]  = szTemp.trimmed();
    }
    else  {
        lstRecValues[colNodeID]  = ui->tblCT->item(nRow, colNodeID)->text().trimmed();
    }
    // Input Register (NO MULTIEDIT)
    if (! fMultiEdit)  {
        szTemp = ui->chkInputRegister->isChecked() ? szTRUE : szFALSE;
        lstRecValues[colInputReg] = szTemp.trimmed();
    }
    else  {
        lstRecValues[colInputReg]  = ui->tblCT->item(nRow, colInputReg)->text().trimmed();
    }
    // Register  (No per MultiSelect)
    if (! fMultiEdit || lstEditableFields.indexOf(colRegister) >= 0)  {
        szTemp = ui->txtRegister->text();
        lstRecValues[colRegister] = szTemp.trimmed();
    }
    else {
        lstRecValues[colRegister] = ui->tblCT->item(nRow, colRegister)->text().trimmed();
    }
    // Block
    szTemp = ui->txtBlock->text();
    lstRecValues[colBlock] = szTemp.trimmed();
    // Block Size
    szTemp = ui->txtBlockSize->text();
    lstRecValues[colBlockSize] = szTemp.trimmed();
    // Comment
    if (! fMultiEdit || lstEditableFields.indexOf(colComment) >= 0)  {
        szTemp = ui->txtComment->text();
        lstRecValues[colComment] = szTemp.trimmed().left(MAX_COMMENT_LEN - 1);
    }
    else  {
        lstRecValues[colComment] = ui->tblCT->item(nRow, colComment)->text().trimmed();
    }
    // Clear all values for Alarms/Events
    lstRecValues[colSourceVar] = szEMPTY;
    lstRecValues[colCondition] = szEMPTY;
    lstRecValues[colCompare] = szEMPTY;
    // Behavior  (per MultiSelect solo se NON allarme)
    nPos = ui->cboBehavior->currentIndex();
    if (nPos >= 0 && nPos < lstBehavior.count())  {
        szTemp = ui->cboBehavior->itemText(nPos);
    }
    else  {
        szTemp = szEMPTY;
    }
    if (nPos < behavior_alarm)  {
        if (! fMultiEdit || lstEditableFields.indexOf(colBehavior) >= 0)  {
            lstRecValues[colBehavior] = szTemp.trimmed();
        }
        else  {
            lstRecValues[colBehavior]  = ui->tblCT->item(nRow, colBehavior)->text().trimmed();
        }
    }
    else  {
        if (nPos >= behavior_alarm && ! fMultiEdit)  {
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
        else  {
            lstRecValues[colBehavior] = ui->tblCT->item(nRow, colBehavior)->text().trimmed();
            lstRecValues[colSourceVar] = ui->tblCT->item(nRow, colSourceVar)->text().trimmed();;
            lstRecValues[colCondition] = ui->tblCT->item(nRow, colCondition)->text().trimmed();;
            lstRecValues[colCompare] = ui->tblCT->item(nRow, colCompare)->text().trimmed();;
        }
    }
    // Finalizzazione controlli su protocolli
    // Protocollo PLC tutto abblencato
    if (nProtocol == PLC)  {
        lstRecValues[colIP] = szEMPTY;
        lstRecValues[colPort] = szEMPTY;
        lstRecValues[colNodeID] = szEMPTY;
        lstRecValues[colInputReg] = szEMPTY;
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
    if (! riassegnaBlocchi())  {
        m_szMsg = QLatin1String("Found Errors in Reassigning Blocks");
        warnUser(this, szMectTitle, m_szMsg);
    }
    // Refresh abilitazioni interfaccia
    m_fMultiSelect = false;
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

    // Rinumerazione Blocchi
    if (! riassegnaBlocchi())  {
        m_szMsg = QLatin1String("Found Errors in Reassigning Blocks");
        warnUser(this, szMectTitle, m_szMsg);
    }
    // Controllo errori
    nErr = globalChecks();
    if (nErr)  {
        // Ci sono errori, salvo comunque ?
        m_szMsg = QLatin1String("There are errors in Data. Save anyway?");
        if (! queryUser(this, szMectTitle, m_szMsg, false))
            return;
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
                lstCTRecords[nCur].InputReg = 0;
                lstCTRecords[nCur].Group = 0;
                lstCTRecords[nCur].Module = 0;
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
        m_szMsg = QString::fromAscii("Error Saving Crosstable File: %1\n").arg(m_szCurrentCTFile);
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
    uint32_t        nNextRegPos = 0;
    uint16_t        curBlock = 0;
    int16_t         curBSize = (int16_t) 0;
    int             nItemsInBlock = 0;

    ui->cmdBlocchi->setEnabled(false);
    this->setCursor(Qt::WaitCursor);
    // Copia l'attuale CT nella lista Undo
    lstUndo.append(lstCTRecords);
    qDebug() << "riassegnaBlocchi() - lstUndo added";
    for (nRow = 0; nRow < MIN_DIAG - 1; nRow++)  {
        // Ignora le righe con Priority == 0
        if (lstCTRecords[nRow].Enable > 0)  {
            // Salto riga o condizione di inizio nuovo blocco
            // Condizioni che scatenano un nuovo blocco Inizio nuovo blocco
            if (nPrevRow != nRow - 1
                    || lstCTRecords[nRow - 1].Enable != lstCTRecords[nRow].Enable                       // Different Level of Priority
                    || lstCTRecords[nRow - 1].Protocol !=  lstCTRecords[nRow].Protocol                  // Different Protocol
                    || lstCTRecords[nRow - 1].IPAddress != lstCTRecords[nRow].IPAddress                 // Different IP Address
                    || lstCTRecords[nRow - 1].Port != lstCTRecords[nRow].Port                           // Different Port
                    || lstCTRecords[nRow - 1].NodeId != lstCTRecords[nRow].NodeId                       // Different Node ID
                    || (lstCTRecords[nRow - 1].Update == Htype && lstCTRecords[nRow].Update != Htype)   // From H to Not H
                    || (lstCTRecords[nRow - 1].Update != Htype && lstCTRecords[nRow].Update == Htype)   // From Not H to H
                    || isTooBigForBlock(nRow, nItemsInBlock, curBSize)                                  // Block too big
                    // Per Modbus devono essere o tutti BIT o tutti != BIT
                    || (isModbus(lstCTRecords[nRow].Protocol) && ((lstCTRecords[nRow - 1].VarType == BIT && lstCTRecords[nRow].VarType != BIT) || (lstCTRecords[nRow - 1].VarType != BIT && lstCTRecords[nRow].VarType == BIT)))
                    // Per Modbus Contiguità di Registro e stesso Tipo (Holding || Input)
                    || (isModbus(lstCTRecords[nRow].Protocol) && (lstCTRecords[nRow].InputReg != lstCTRecords[nRow - 1].InputReg))
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
    m_isCtModified = true;
    m_rebuildDeviceTree = true;
    m_rebuildTimingTree = true;
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
        fRes = fieldValues2CTrecList(lstFields, lstCTRecords, nCur);
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
    int     nType = ui->cboType->currentIndex();
    bool    fMultiEdit = m_fMultiEdit && m_fMultiSelect;

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
    // Variabili di Sistema, abilitate in modifica solo il nome, commento. No Insert in campi vuoti
    // Da versione 3.1.2 disabilitati i campi (priorità)
    if (m_nGridRow >= MIN_DIAG -1)  {
        bool fDecimal = false;
        if (nProtocol != -1)  {
//            ui->cboPriority->setEnabled(true);
            ui->cboUpdate->setEnabled(true);
            ui->txtName->setEnabled(! fMultiEdit);
            ui->txtComment->setEnabled(true);
        }
        // Abilitazione dei Decimal per Input Analogiche se abilitati
        if (panelConfig.analogIN > 0 && panelConfig.analogINrowCT > 0)  {
            if (m_nGridRow >= panelConfig.analogINrowCT -1 && m_nGridRow < panelConfig.analogINrowCT + panelConfig.analogIN -1)
                fDecimal = true;
        }
        // Abilitazione dei Decimal per Output Analogiche se abilitati
        if (panelConfig.analogOUT > 0 && panelConfig.analogOUTrowCT > 0)  {
            if (m_nGridRow >= panelConfig.analogOUTrowCT -1 && m_nGridRow < panelConfig.analogOUTrowCT + panelConfig.analogOUT -1)
                fDecimal = true;
        }
        ui->txtDecimal->setEnabled(fDecimal);
    }
    else  {
        // Campi comuni
        ui->cboPriority->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colPriority) >= 0);
        ui->cboUpdate->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colUpdate) >= 0);
        ui->txtName->setEnabled(! fMultiEdit);  // Nome abilitato solo se no selezione multipla
        ui->cboType->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colType) >= 0);
        ui->txtDecimal->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colDecimal) >= 0);
        ui->cboProtocol->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colProtocol) >= 0);
        ui->txtComment->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colComment) >= 0);
        ui->cboBehavior->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colBehavior) >= 0);
        // Abilitazione dei campi in funzione del Tipo
        // Tipo BIT -> Blocca decimali
        if (nType == BIT)  {
            ui->txtDecimal->setEnabled(false);
        }
        // Decimali bloccati per i tipi Bit se Multiedit
        if (fMultiEdit && (nType >= 0 && nType < TYPE_TOTALS) && isBitField((varTypes) nType) )  {
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
            ui->txtNode->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colNodeID) >= 0);
            // ui->txtRegister->setEnabled(! fMultiEdit);
            ui->txtRegister->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colRegister) >= 0);
            // IP abilitato solo per protocolli Client TCP, TCPRTU
            if (nProtocol == TCP || nProtocol == TCPRTU)  {
                ui->txtIP->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colIP) >= 0);
            }
            // Porta Abilitata per tutti i protocolli di Network
            // TCP, TCPRTU, TCP_SRV, TCPRTU_SRV
            if (nProtocol == TCP || nProtocol == TCPRTU ||
                nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV    )  {
                ui->txtPort->setEnabled(! fMultiEdit ? true : lstEditableFields.indexOf(colPort) >= 0);
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
        // Visibilità Check Box Input Regs per protocolli RTU - TCP - TCPRTU
        if (nProtocol == RTU || nProtocol == RTU_SRV ||
            nProtocol == TCP || nProtocol == TCPRTU  ||
            nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV)  {
            ui->lblInputRegister->setVisible(true);
            ui->chkInputRegister->setVisible(true);
            ui->chkInputRegister->setEnabled(true);
        }
        else  {
            ui->lblInputRegister->setVisible(false);
            ui->chkInputRegister->setVisible(false);
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
//void ctedit::on_chkInputRegister_clicked(bool checked)
//Cambio di Registro da Holding a Input Register e viceversa
//{
//    bool    fOk = false;
//    int     nVal = ui->txtRegister->text().toInt(&fOk);

//    nVal = fOk ? nVal : 0;
//    // Passaggio da valore normale a Input Register
//    if (checked && (nVal >= 0 && nVal <= nMax_Int16))  {
//        nVal += nStartInputRegister;
//    }
//    if (! checked && nVal >= nStartInputRegister)  {
//        nVal -= nStartInputRegister;
//    }
//    // Refresh del valore
//    ui->txtRegister->setText(QString::number(nVal));
//}
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
        ui->chkInputRegister->setChecked(false);
        ui->txtRegister->setText(szEMPTY);
    }
    // Calcola la porta di default in funzione del protocollo (if any available)
    getFirstPortFromProtocol(index, nDefaultPort, nTotalFree);
    // Recupera il numero di porta corrente (Per protocolli seriali è attiva la Combo di Selezione)
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
        ui->chkInputRegister->setChecked(false);
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
        if (szTemp.isEmpty())  {
            ui->txtIP->setText(szEMPTY_IP);
        }        
    }
    // Disabilitazione  tipo BIT per i vari tipi di SRV (TCP_SRV, RTU_SRV, TCP_RTU_SRV)
    // Non è ammesso BIT per i protocolli SERVER (TCP/RTU)
    if (index == TCP_SRV || index == TCPRTU_SRV || index == RTU_SRV)  {
        disableComboItem(ui->cboType, BIT);
    }
    else {
        enableComboItem(ui->cboType, BIT);
    }
    // De-check Flag Input register for all but ModBus
    if ( ! isModbus(index))  {
        ui->chkInputRegister->setChecked(false);
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
void ctedit::tableCTItemChanged(const QItemSelection & selected, const QItemSelection & deselected)
// Slot attivato ad ogni cambio di riga in Table CT
{
    int         nRow = -1;
    int         nItem = -1;
    int         nLastSelectedRow = -1;
    QStringList lstFields;
    bool        fRes = true;

    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();

    if (ui->tblCT->rowCount() == 0)  {
        return;
    }
    if (selected.count())  {
        int nCount = selected.indexes().count();
        nLastSelectedRow = selected.indexes().at(nCount - 1).row();
    }
    qDebug("tableCTItemChanged(): Rows: %d Selected Items: %d Deselected Items: %d - Last Selected Row: %d",
           ui->tblCT->rowCount(), selected.count(), deselected.count(), nLastSelectedRow + 1);
    // Il cambio riga corrente è dovuto a operazioni di tipo cut-paste.
    // Per evitare duplicazioni accidentali di righe ripulisce il buffer di editing ed esce
    if (m_fCutOrPaste)  {
        clearEntryForm();
        ui->fraEdit->setEnabled(false);
        ui->cboSections->setCurrentIndex(-1);
        m_fMultiSelect = false;
        return;
    }
    // Si sta uscendo dalla selezione di una riga sola
    if (deselected.count() == 1 && ! m_fMultiSelect && ! m_fMultiEdit)  {
        // Considera sempre la prima riga della lista
        nRow = deselected.indexes().at(0).row();
        // Se la riga corrente è stata modificata, salva il contenuto
        if (nRow >= 0 && nRow < lstCTRecords.count())  {
            // Il contenuto viene aggiornato solo se la linea risulta modificata e il form non è vuoto
            fRes = updateRow(nRow, false);
            // Cambio riga Ko
            if (! fRes)    {
                // Disconnette segnale per evitare ricorsione
                disconnect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
                            SLOT(tableCTItemChanged(const QItemSelection &, const QItemSelection & ) ));
                // Cambia Selezione (ritorna a riga precedente)
                ui->tblCT->selectRow(nRow);
                m_nGridRow = nRow;
                // Riconnette slot gestione
                connect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
                            SLOT(tableCTItemChanged(const QItemSelection &, const QItemSelection & ) ));
                return;
            }
        }
    }
    // Nessuna Nuova Riga selezionata
    m_fMultiSelect = false;
    clearStatusMessage();
    clearEntryForm();
    if (selection.count() == 0)  {
        lstSelectedRows.clear();
        qDebug("tableCTItemChanged(): Clear Selection");
    }
    // Una sola riga selezionata
    else if (selection.count() == 1)  {
        lstSelectedRows.clear();
        nRow = selection.at(0).row();
        lstSelectedRows.append(nRow);
        // qDebug() << "New Row: " << nRow;
        if (nRow >= 0 && nRow < lstCTRecords.count())  {
            // Cambia riga corrente
            m_nGridRow = nRow;
            // Imposta Sezione in cboSections
            setSectionArea(nRow);
        }
    }
    // Selezionate più righe
    if (selection.count() > 1)  {
        m_szMsg = QString::fromAscii("Selected Rows: %1") .arg(selection.count());
        displayStatusMessage(m_szMsg, STD_DISPLAY_TIME);
        lstSelectedRows.clear();
        for (nItem = 0; nItem < selection.count(); nItem++)  {
            int newRow = selection.at(nItem).row();
            if (lstSelectedRows.indexOf(newRow) < 0)  {
                lstSelectedRows.append(newRow);
            }
        }
        qSort(lstSelectedRows.begin(), lstSelectedRows.end());
        ui->fraCondition->setVisible(false);
        m_fMultiSelect = true;
    }
    // Elenco delle variabili selezionate
    bool fLoaded = false;
    for (nItem = 0; nItem < lstSelectedRows.count(); nItem++)  {
        qDebug("tableCTItemChanged(): Selected Item %d @Row: %d", nItem + 1, lstSelectedRows.at(nItem) + 1);
        nRow = lstSelectedRows[nItem];
        // Prova a caricare da CT i valori presenti nella lista dati se la variabile è definita
        if (! fLoaded && lstCTRecords[nRow].UsedEntry && nRow == nLastSelectedRow)  {
            // Solo se c'è qualcosa da editare (singolo Record o MultiEdit
            if (! m_fMultiSelect || (m_fMultiSelect && m_fMultiEdit))  {
                // Convert CT Record 2 User Values
                fRes = recCT2FieldsValues(lstCTRecords, lstFields, nRow);
                // Se la conversione è Ok, aggiorna interfaccia
                if (fRes)  {
                    fRes = values2Iface(lstFields, nRow);
                    if (fRes)  {
                        fLoaded = true;
                        // Cambia riga corrente
                        m_nGridRow = nRow;
                    }
                }
            }
        }
    }
    // La Cella è abilitata se ne è stata letta almeno una o se la riga corrente è vuota (No MultiEdit in corso)
    ui->fraEdit->setEnabled(fLoaded || (! m_fMultiSelect && ! m_fMultiEdit));
    // Si può cambiare riga, legge contenuto
    if (! selected.isEmpty() && selected.count() == 1)  {
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
    ui->chkInputRegister->setChecked(false);
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
    QIcon           cIco;
    QClipboard      *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    QString         szClipBuffer;
    bool            fClipSourceOk = false;
    QList<QStringList > lstPastedRecords;
    QList<int>      lstDestRows;
    QList<int>      lstSourceRows;
    QLatin1String   szModuleMessage("Enter the RTU port and node ID for the device [%1].\n You can later change these settings in the configuration form");



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
    //cIco = QIcon(szPathIMG + QLatin1String("Blocks.png"));
    //QAction *menuBlocks = gridMenu.addAction(cIco, trUtf8("Renum Blocks"));
    //menuBlocks->setEnabled(true);
    // Goto Line
    cIco = QIcon(szPathIMG + QLatin1String("Goto.png"));
    QAction *menuRow = gridMenu.addAction(cIco, trUtf8("Goto Line\t\t(Ctrl+L)"));
    menuRow->setEnabled(true);
    // Sep0
    gridMenu.addSeparator();
    // Inserisci righe
    cIco = QIcon(szPathIMG + QLatin1String("List-add.png"));
    QAction *insRows = gridMenu.addAction(cIco, trUtf8("Insert Blank Rows\t\t(Ins)"));
    insRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // insRows->setShortcut(Qt::Key_Insert);

    // Cancella righe
    cIco = QIcon(szPathIMG + QLatin1String("Edit-clear.png"));
    QAction *emptyRows = gridMenu.addAction(cIco, trUtf8("Clear Rows\t\t(Del)"));
    emptyRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // emptyRows->setShortcut(Qt::Key_Delete);
    // Elimina righe
    cIco = QIcon(szPathIMG + QLatin1String("Edit-trash.png"));
    QAction *remRows = gridMenu.addAction(cIco, trUtf8("Delete Rows"));
    remRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // Sep1
    gridMenu.addSeparator();
    // Copia righe (Sempre permesso)
    cIco = QIcon(szPathIMG + QLatin1String("Copy.png"));
    QAction *copyRows = gridMenu.addAction(cIco, trUtf8("Copy Rows\t\t(Ctrl+C)"));
    copyRows->setEnabled(selection.count() > 0);
    // copyRows->setShortcut(Qt::Key_Copy);
    // Taglia righe
    cIco = QIcon(szPathIMG + QLatin1String("Cut.png"));
    QAction *cutRows = gridMenu.addAction(cIco, trUtf8("Cut Rows\t\t(Ctrl+X)"));
    cutRows->setEnabled(selection.count() > 0 && m_nGridRow < MIN_DIAG - 1);
    // cutRows->setShortcut(Qt::Key_Cut);
    // Sep 2
    gridMenu.addSeparator();
    // Paste Rows
    cIco = QIcon(szPathIMG + QLatin1String("edit-paste.png"));
    QAction *pasteRows = gridMenu.addAction(cIco, trUtf8("Paste Rows\t\t(Ctrl+V)"));
    pasteRows->setEnabled(fClipSourceOk && m_nGridRow < MIN_DIAG - 1);
    // pasteRows->setShortcut(Qt::Key_Paste);
    // Sep 3
    gridMenu.addSeparator();
    // Menu per importazione delle variabili per modelli MECT connessi su Bus Seriale
    QAction *addMPNC006 = gridMenu.addAction(cIco, trUtf8("Paste MPNC006 Modules"));
    addMPNC006->setEnabled((isSerialPortEnabled)
                           && (m_nGridRow + lstMPNC006_Vars.count()) < MIN_DIAG - 1);
    QAction *addMPNE1001 = gridMenu.addAction(cIco, trUtf8("Paste MPNE1001 Module"));
    addMPNE1001->setEnabled((isSerialPortEnabled)
                          && (m_nGridRow + lstMPNE_Vars.count()) < MIN_DIAG - 1);
    // Menu per importazione delle variabili per modelli MECT connessi su Bus Seriale (solo per TPLC050)
    QAction *addTPLC050 = gridMenu.addAction(cIco, trUtf8("Paste TPLC050 Modules"));
    addTPLC050->setVisible((panelConfig.modelName.contains(szTPLC050))
                           && (m_nGridRow + lstTPLC050_Vars.count()) < MIN_DIAG - 1);
    addTPLC050->setEnabled(addTPLC050->isVisible());
    // Menù Grafico per MPNC // MPNE
    QAction *editMPNC = 0;
    QAction *editMPNE = 0;
    if (isSerialPortEnabled)  {
        // Menu Editing grafico MPNC (Abilitato se esistono Moduli definiti)
        bool isMPNC = false;
        int nModule = 0;
        for (nModule = 0; nModule < lstMPNC.count(); nModule++)  {
            int nBase = lstMPNC.at(nModule);
            if (m_nGridRow >= nBase && m_nGridRow < nBase + lstMPNC006_Vars.count())  {
                isMPNC = true;
                break;
            }
        }
        if (m_nGridRow < MIN_DIAG - 1 && isMPNC)  {
            cIco = QIcon(szPathIMG + QLatin1String("Card_32.png"));
            gridMenu.addSeparator();
            editMPNC = gridMenu.addAction(cIco, trUtf8("Graphic Editor for MPNC006"));
        }
        // Menù Grafico per MPNE (Abilitato se esistono Moduli definiti)
        bool isMPNE = false;
        for (nModule = 0; nModule < lstMPNE.count(); nModule++)  {
            int nBase = lstMPNE.at(nModule);
            if (m_nGridRow >= nBase && m_nGridRow < nBase + lstMPNE_Vars.count())  {
                isMPNE = true;
                break;
            }
        }
        if (m_nGridRow < MIN_DIAG - 1 && isMPNE)  {
            cIco = QIcon(szPathIMG + QLatin1String("Card_32.png"));
            gridMenu.addSeparator();
            editMPNE = gridMenu.addAction(cIco, trUtf8("Graphic Editor for MPNE1001"));
        }
    }
    // Esecuzione del Menu
    QAction *actMenu = gridMenu.exec(ui->tblCT->viewport()->mapToGlobal(pos));
    // Controllo dell'Azione selezionata
    // Inserimento righe
    if (actMenu == insRows)  {
        insertRows();
    }
    // Cancella Righe
    else if (actMenu == emptyRows)  {
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
    // Add MPNC006
    else if (actMenu == addMPNC006)  {
        // Controllo dell'area di destinazione
        if (checkFreeArea(nRow, lstMPNC006_Vars.count()))  {
            // Richiesta Porta e Nodo di destinazione
            int nPort = -1;
            int nNode = -1;
            QString szMsg = QString(szModuleMessage) .arg(szMPNC006);
            queryPortNode *qryPort = new queryPortNode(szMectTitle, szMsg, this);
            qryPort->setModal(true);
            int nResPort = qryPort->exec();
            if (nResPort == QDialog::Accepted)  {
                qryPort->getPortNode(nPort, nNode);
                addModelVars(szMPNC006, m_nGridRow, nPort, nNode);
                ui->tabWidget->setCurrentIndex(TAB_MPNC);
            }
            qryPort->deleteLater();
        }
    }
    // Add MPNE10
    else if (actMenu == addMPNE1001)  {
        // Controllo dell'area di destinazione
        if (checkFreeArea(nRow, lstMPNE_Vars.count()))  {
            // Richiesta Porta e Nodo di destinazione
            int nPort = -1;
            int nNode = -1;
            QString szMsg = QString(szModuleMessage) .arg(szMPNE1001);
            queryPortNode *qryPort = new queryPortNode(szMectTitle, szMsg, this);
            qryPort->setModal(true);
            int nResPort = qryPort->exec();
            if (nResPort == QDialog::Accepted)  {
                qryPort->getPortNode(nPort, nNode);
                addModelVars(szMPNE1001, m_nGridRow, nPort, nNode);
                ui->tabWidget->setCurrentIndex(TAB_MPNE);
            }
            qryPort->deleteLater();
        }
    }
    // Add TPLC050
    else if (actMenu == addTPLC050)  {
        // Controllo dell'area di destinazione
        if (checkFreeArea(nRow, lstTPLC050_Vars.count()))  {
            // Porta fissa a 3 e Nodo fisso a 1
            int nPort = 3;
            int nNode = 1;
            addModelVars(szTPLC050, m_nGridRow, nPort, nNode);
        }
    }
    // Edit MPNC
    else if (editMPNC != 0 && actMenu == editMPNC)  {
        ui->tabWidget->setCurrentIndex(TAB_MPNC);
    }
    // Edit MPNE
    else if (editMPNE != 0 && actMenu == editMPNE)  {
        ui->tabWidget->setCurrentIndex(TAB_MPNE);
    }

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
    if (selection.count() <= 0)  {
        return 0;
    }
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
        recCT2FieldsValues(lstCTRecords, lstRecordFields, nRow);
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
    m_szMsg = QString::fromAscii("Rows Copied: %1") .arg(nCopied);
    displayStatusMessage(m_szMsg);
    m_fMultiSelect = false;
    enableInterface();
    // Return value
    return nCopied;
}

void ctedit::pasteSelected()
// Incolla righe da Buffer di copiatura a Riga corrente
{
    int             nRow = ui->tblCT->currentRow();
    int             nPasted = 0;
    // Recupera righe selezionate
    // QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();
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
    if (! fClipSourceOk)  {
        m_szMsg = QLatin1String("Can't Copy as Copy Buffer is Empty or Invalid");
    }
    // Controllo area di destinazione
    if (! checkFreeArea(nRow, lstPastedRecords.count()))  {
        return;
    }

    // Paste Rows
    qDebug() << QString::fromAscii("Pasted Rows Count: %1") .arg(lstPastedRecords.count());
    if (fClipSourceOk)  {
        if (nRow + lstPastedRecords.count() < MAX_NONRETENTIVE)  {
            nPasted = addRowsToCT(nRow, lstPastedRecords, lstDestRows);
        }
        else  {
            m_szMsg = QLatin1String("The Copy Buffer Excedes System Variables Limit. Rows not copied");
            warnUser(this, szMectTitle, m_szMsg);
        }
        m_szMsg = QString::fromAscii("Rows Pasted: %1") .arg(nPasted);
    }
    else {
        m_szMsg = QString::fromAscii("Error Pasting Rows: %1") .arg(lstPastedRecords.count());
    }
    displayStatusMessage(m_szMsg);
    m_fMultiSelect = false;
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
        qDebug() << "insertRows() - lstUndo added";
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
            freeCTrec(lstCTRecords, nFirstRow);
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
        // Se era in corso un Multiselect ripristina il pannello di Editing

        // Refresh Grid
        ctable2Grid();
        m_isCtModified = true;
        m_rebuildDeviceTree = true;
        m_rebuildTimingTree = true;
    }
    else  {
        m_szMsg = QLatin1String("Too Many Rows selected, passed limit of User Variables!");
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    m_szMsg = QString::fromAscii("Rows Inserted: %1") .arg(selection.count());
    displayStatusMessage(m_szMsg);
    if (nCurPos >= 0)  {
        jumpToGridRow(nCurPos, true);
    }
    m_fMultiSelect = false;
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
        m_szMsg = QLatin1String("Can't remove rows in System Area");
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    lstUndo.append(lstCTRecords);
    qDebug() << "emptySelected() - lstUndo added";
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
            freeCTrec(lstCTRecords, nRow);
            nRemoved++;
        }
    }
    // Set Current index Row to first of Selection
    selection.clear();
    // Refresh Grid
    if (nRemoved)  {
        ctable2Grid();
        m_isCtModified = true;
        m_rebuildDeviceTree = true;
        m_rebuildTimingTree = true;
    }
    // Riposiziona alla riga corrente
    if (nRemoved > 0 && nFirstRow >= 0)  {
        jumpToGridRow(nFirstRow, true);
    }
    m_szMsg = QString::fromAscii("Rows Removed: %1") .arg(nRemoved);
    displayStatusMessage(m_szMsg);
    // Update Iface
    m_fMultiSelect = false;
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
        m_szMsg = QLatin1String("Can't remove rows in System Area");
        displayStatusMessage(m_szMsg);
        selection.clear();
        return;
    }
    lstUndo.append(lstCTRecords);
    qDebug() << "removeSelected() - lstUndo added";
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
            freeCTrec(lstCTRecords, nStart);
        }
        // Refresh Grid
        ctable2Grid();
        m_isCtModified = true;
        m_rebuildDeviceTree = true;
        m_rebuildTimingTree = true;
    }
    m_szMsg = QString::fromAscii("Rows Removed: %1") .arg(nRemoved);
    displayStatusMessage(m_szMsg);
    // Update Iface
    m_fMultiSelect = false;
    enableInterface();
}
void ctedit::cutSelected()
// Taglia righe in Buffer di copiatura
{
    int nCopied = 0;
    // Recupera righe selezionate
    QModelIndexList selection = ui->tblCT->selectionModel()->selectedRows();

    if (m_nGridRow >= MIN_DIAG - 1)  {
        m_szMsg = QLatin1String("Can't remove rows in System Area");
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
    m_szMsg = QString::fromAscii("Rows Cutted: %1") .arg(nCopied);
    displayStatusMessage(m_szMsg);
    // qDebug() << m_szMsg;
}
bool ctedit::isFormEmpty()
// Controllo Form Editing vuoto
{
    int     nFilled = 0;
    bool    fMultiEdit = m_fMultiEdit && m_fMultiSelect;

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
    if (ui->cboBehavior->currentIndex() >= behavior_alarm && ! fMultiEdit)  {
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
    QString szTemp(szEMPTY);
    bool    fMultiEdit = m_fMultiEdit && m_fMultiSelect;

    // Confronto tra Form Editing e riga Grid
    if(nRow >= 0 && nRow < lstCTRecords.count())  {
        // Se è attivo il MultiSelect NON si salva all'interno di righe vuote
        if (fMultiEdit && ui->tblCT->item(nRow, colName)->text().trimmed().isEmpty())  {
            nModif = 0;
        }
        else  {
            nModif += (ui->cboPriority->currentText().trimmed() != ui->tblCT->item(nRow, colPriority)->text().trimmed());
            nModif += (ui->cboUpdate->currentText().trimmed() != ui->tblCT->item(nRow, colUpdate)->text().trimmed());
            // La Colonna Group non ha inpatto in calcolo modifiche
            // nModif += (ui->cboUpdate->currentText().trimmed() != ui->tblCT->item(nRow, colGroup)->text().trimmed());
            nModif += (! fMultiEdit && (ui->txtName->text().trimmed() != ui->tblCT->item(nRow, colName)->text().trimmed()));
            nModif += (ui->cboType->currentText().trimmed() != ui->tblCT->item(nRow, colType)->text().trimmed());
            nModif += (ui->txtDecimal->text().trimmed() != ui->tblCT->item(nRow, colDecimal)->text().trimmed());
            nModif += (ui->cboProtocol->currentText().trimmed() != ui->tblCT->item(nRow, colProtocol)->text().trimmed());
            nModif += (ui->txtIP->text().trimmed() != ui->tblCT->item(nRow, colIP)->text().trimmed());
            if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)
                nModif += ui->cboPort->currentText().trimmed() != ui->tblCT->item(nRow, colPort)->text().trimmed();
            else
                nModif += (ui->txtPort->text().trimmed() != ui->tblCT->item(nRow, colPort)->text().trimmed());
            nModif += (ui->txtNode->text().trimmed() != ui->tblCT->item(nRow, colNodeID)->text().trimmed());
            // Input Register
            if (ui->chkInputRegister->isVisible()) {
                szTemp = ui->chkInputRegister->isChecked() ? szTRUE : szFALSE;
                nModif += (szTemp != ui->tblCT->item(nRow, colInputReg)->text().trimmed());
            }
            // Offset Register
            nModif += (! fMultiEdit && (ui->txtRegister->text().trimmed() != ui->tblCT->item(nRow, colRegister)->text().trimmed()));
            // nModif += (ui->txtBlock->text().trimmed() != ui->tblCT->item(nRow, colBlock)->text().trimmed());
            // nModif += (ui->txtBlockSize->text().trimmed() != ui->tblCT->item(nRow, colBlockSize)->text().trimmed());
            nModif += (ui->txtComment->text().trimmed() != ui->tblCT->item(nRow, colComment)->text().trimmed());
            nModif += (ui->cboBehavior->currentText().trimmed() != ui->tblCT->item(nRow, colBehavior)->text().trimmed());
            // Frame Allarmi
            if ((! fMultiEdit) && ui->cboBehavior->currentIndex() >= behavior_alarm)  {
                nModif += (ui->cboVariable1->currentText().trimmed() != ui->tblCT->item(nRow, colSourceVar)->text().trimmed());
                nModif += (ui->cboCondition->currentText().trimmed() != ui->tblCT->item(nRow, colCondition)->text().trimmed());
                if (ui->optFixedVal->isChecked())  {
                    // Confronto tra Real arrotondati a 4 decimali
                    bool    fIfVal = false;
                    bool    fTbVal = false;
                    double dblIfaceVal = ui->txtFixedValue->text().toDouble(&fIfVal);
                    dblIfaceVal = fIfVal ? myRound(dblIfaceVal, nCompareDecimals) : 0.0;
                    double dblTableVal = ui->tblCT->item(nRow, colCompare)->text().toDouble(&fTbVal);
                    dblTableVal = fIfVal ? myRound(dblTableVal, nCompareDecimals) : 0.0;
                    // Entrambi convertiti, valori diversi, oppure Uno dei due non convertibile
                    if ((fIfVal && fTbVal && dblIfaceVal != dblTableVal) || (fIfVal != fTbVal))
                        nModif++;
    //                nModif += (ui->txtFixedValue->text().trimmed() != ui->tblCT->item(nRow, colCompare)->text().trimmed());
                }
                else
                    nModif += (ui->cboVariable2->currentText().trimmed() != ui->tblCT->item(nRow, colCompare)->text().trimmed());
            }
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
    QList<CrossTableRecord>  lstNewRecs;
    int     nRow = 0;
    int     nProtocol = 0;
    int     nPort = 0;
    int     nTotal = 0;
    bool    fRes = false;

    szSourceFile = QFileDialog::getOpenFileName(this, QLatin1String("Import From Crosstable File"), m_szCurrentCTFile, QString::fromAscii("Crosstable File (%1)") .arg(szCT_FILE_NAME));
    if (! szSourceFile.isEmpty())  {
        if (checkCTFile(szSourceFile))  {
            szMsg = QString::fromAscii("Rows from %1 to %2 will be overwritten !!\nDo you want to continue?") .arg(MIN_RETENTIVE) .arg(MAX_NONRETENTIVE);
            if (queryUser(this, szMectTitle, szMsg))  {
                // Copia di Salvataggio
                lstUndo.append(lstCTRecords);
                qDebug() << "on_cmdImport_clicked() - lstUndo added";
                lstNewRecs.clear();
                // Caricamento della nuova Cross Table (a questo livello non vengono fatti checks sulle righe caricate)
                if (loadCTFile(szSourceFile, lstNewRecs, false))  {
                    // Aggiunta alla Cross Table dei record letti dalla nuova CT
                    for (nRow = 0; nRow < MAX_NONRETENTIVE; nRow++)  {
                        // Controllo Protocollo / Porta RTU su righe importate
                        nProtocol = lstNewRecs[nRow].Protocol;
                        if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
                            nPort = lstNewRecs[nRow].Port;
                            nTotal = 0;
                            if (! isValidPort(nPort, nProtocol))  {
                                getFirstPortFromProtocol(nProtocol, nPort, nTotal);
                                if (nTotal > 0)
                                    lstNewRecs[nRow].Port = nPort;
                            }
                        }
                        // Rimpiazza record in Cross Table
                        lstCTRecords[nRow] = lstNewRecs[nRow];
                    }
                    // Ricarica la lista dei dati CT in Grid
                    fRes = ctable2Grid();
                    if (fRes)  {
                        szMsg = QString::fromAscii("Loaded Crosstable from file:\n%1") .arg(szSourceFile);
                        notifyUser(this, szMectTitle, szMsg);
                    }
                    else {
                        szMsg = QString::fromAscii("Error Loading Crosstable from file:\n%1") .arg(szSourceFile);
                        szMsg.append(QLatin1String("\nOriginal Content Reloaded"));
                        warnUser(this, szMectTitle, szMsg);
                        on_cmdUndo_clicked();
                    }
                    m_isCtModified = true;
                    m_rebuildDeviceTree = true;
                    m_rebuildTimingTree = true;
                }
            }
        }
        else  {
            szMsg = QString::fromAscii("The Selected file is not a Crosstable file:\n%1") .arg(szSourceFile);
            warnUser(this, szMectTitle, szMsg);
        }
    }
}
void    ctedit::setRowsColor()
// Imposta il colore di sfondo di tutte le righe senza cambiare riga corrente
{
    int         nRow = 0;
    int         nAlternate = 0;
    int16_t     nPrevBlock = -1;

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
        setRowColor(ui->tblCT, nRow, nAlternate, lstCTRecords[nRow].UsedEntry, lstCTRecords[nRow].Enable, 0);
    }
}


void ctedit::showAllRows(bool fShowAll)
// Visualizza o nascondi tutte le righe
{
    int         nFirstVisible = -1;
    int         nRow = 0;

    // qDebug() << QString::fromAscii("showAllRows: showAll = %1 Current Row = %2") .arg(fShowAll) .arg(m_nGridRow);
    setRowsColor();
    // Items del Grid    
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Ricerca il primo Item visibile
        if (lstCTRecords[nRow].UsedEntry && nFirstVisible < 0)  {
            nFirstVisible = nRow ;
            // qDebug() << QString::fromAscii("First Visible Row:%1") .arg(nFirstVisible);
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
    int nRow = QInputDialog::getInt(this, QLatin1String("Row to Jump To"),
                                 QString::fromAscii("Enter Row Number to Jump to:"), m_nGridRow + 1, 1, DimCrossTable, 1, &fOk);
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
    int  nRow = 0;

    searcForm->setModal(true);
    if (searcForm->exec()  == QDialog::Accepted)   {
        nRow = searcForm->getSelectedVariable();
        qDebug("on_cmdSearch_clicked(): Selected Row: %d", nRow);
        if (nRow > 0 && nRow <= lstCTRecords.count())  {
            jumpToGridRow(nRow - 1, true);
        }
    }
    else {
        qCritical("Search Cancelled");
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

    // Rinumerazione Blocchi
    if (! riassegnaBlocchi())  {
        m_szMsg = QLatin1String("Found Errors in Reassigning Blocks");
        warnUser(this, szMectTitle, m_szMsg);
    }
    // Controllo presenza di Errori
    int         nErr = globalChecks();
    m_isCtModified = false;
    if (nErr)  {
        return;
    }
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
    szTemp.prepend(QLatin1String("-g"));
    lstArguments.append(szTemp);
    fileBackUp(szFileName);
    // Parametro 3: -i Nome del file .h (Forced LowerCase)
    szFileName = QString::fromAscii("%1.h") .arg(m_szCurrentCTName.toLower());
    szTemp = szFileName;
    szTemp.prepend(QLatin1String("-i"));
    lstArguments.append(szTemp);
    szFileName.prepend(m_szCurrentCTPath);
    fileBackUp(szFileName);
    // Parametro 4: -s Nome del file .h (Forced LowerCase)
    szFileName = QString::fromAscii("%1.cpp") .arg(m_szCurrentCTName.toLower());
    szTemp = szFileName;
    szTemp.prepend(QLatin1String("-s"));
    lstArguments.append(szTemp);
    szFileName.prepend(m_szCurrentCTPath);
    fileBackUp(szFileName);
    // Imposta come Directory corrente di esecuzione la directory del File CT
    procCompile.setWorkingDirectory(m_szCurrentCTPath);
    // Esecuzione Comando
    // qDebug() << szCommand << lstArguments;
    procCompile.start(szCommand, lstArguments);
    if (!procCompile.waitForStarted())  {
        m_szMsg = QLatin1String("Error Starting Crosstable Compiler!\n");
        m_szMsg.append(szCommand);
        warnUser(this, szMectTitle, m_szMsg);
        goto exit_compile;
    }
    // Attesa termine comando
    if (!procCompile.waitForFinished())  {
        m_szMsg = QLatin1String("Error Running Crosstable Compiler!\n");
        m_szMsg.append(szCommand);
        warnUser(this, szMectTitle, m_szMsg);
        goto exit_compile;
    }
    // Esito comando
    baCompErr = procCompile.readAllStandardError();
    nExitCode = procCompile.exitCode();
    if (nExitCode != 0)  {
        m_szMsg = QString::fromAscii("Exit Code of Crosstable Compiler: %1\n") .arg(nExitCode);
        szCompErr = QLatin1String(baCompErr.data());
        m_szMsg.append(szCompErr);
        warnUser(this, szMectTitle, m_szMsg);
        // TODO: Analisi errore del Cross Compiler
    }
    else {
        m_szMsg = QLatin1String("Crosstable Correctly Compiled");
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
            if (szLine.startsWith(QLatin1String("TYPE"), Qt::CaseSensitive))  {
                // qDebug() << QString::fromAscii("Model Line: %1") .arg(szLine);
                nPos = szLine.indexOf(QLatin1String("="));
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
        qDebug() << QString::fromAscii("Template File: %1 not found") .arg(szFileTemplate);
    }
    // qDebug() << QString::fromAscii("getModelName: Current Model: <%1>") .arg(szModel);
    // return value
    return szModel;
}
QString ctedit::getModelInfo(int nModel)
// Prepara il ToolTipText con le info di modello
{
    QString     szText(szEMPTY);

    if (nModel >= 0 && nModel < lstTargets.count())  {
        szText.append(QString::fromAscii("Model: \t%1\n\n") .arg(lstTargets[nModel].modelName ));
        // Structural Parameters
        szText.append(QString::fromAscii("Display Width: \t\t%1\n") .arg(lstTargets[nModel].displayWidth));
        szText.append(QString::fromAscii("Display Height: \t%1\n") .arg(lstTargets[nModel].displayHeight));
        szText.append(QString::fromAscii("USB Ports: \t\t%1\n") .arg(lstTargets[nModel].usbPorts));
        szText.append(QString::fromAscii("Ethernet Ports: \t\t%1\n") .arg(lstTargets[nModel].ethPorts));
        szText.append(QString::fromAscii("SD Cards: \t\t%1\n") .arg(lstTargets[nModel].sdCards));
        szText.append(QString::fromAscii("Encoders: \t\t%1\n") .arg(lstTargets[nModel].nEncoders));
        szText.append(QString::fromAscii("Digital IN: \t\t%1\n") .arg(lstTargets[nModel].digitalIN));
        szText.append(QString::fromAscii("Digital OUT: \t\t%1\n") .arg(lstTargets[nModel].digitalOUT));
        szText.append(QString::fromAscii("Analog IN: \t\t%1\n") .arg(lstTargets[nModel].analogIN));
        szText.append(QString::fromAscii("Analog OUT: \t\t%1\n") .arg(lstTargets[nModel].analogOUT));
        if (lstTargets[nModel].fastIn)  {
            szText.append(QString::fromAscii("Fast IN: \t\t%1\n") .arg(lstTargets[nModel].fastIn));
        }
        if (lstTargets[nModel].fastOut)  {
            szText.append(QString::fromAscii("Fast OUT: \t\t%1\n") .arg(lstTargets[nModel].fastOut));
        }
        if (lstTargets[nModel].pwm)  {
            szText.append(QString::fromAscii("PWM-PTO: \t\t%1\n") .arg(lstTargets[nModel].pwm));
        }
        if (lstTargets[nModel].rpmPorts)  {
            szText.append(QString::fromAscii("RPM In: \t\t%1\n") .arg(lstTargets[nModel].rpmPorts));
        }
        if (lstTargets[nModel].loadCells)  {
            szText.append(QString::fromAscii("Load Cells: \t\t%1\n") .arg(lstTargets[nModel].loadCells));
        }
        // Bus Interfaces
        // Serial 0
        szText.append(QString::fromAscii("Serial 0 Enabled: \t%1\n") .arg(bool2String(lstTargets[nModel].ser0_Enabled)));
        // Serial 1
        szText.append(QString::fromAscii("Serial 1 Enabled: \t%1\n") .arg(bool2String(lstTargets[nModel].ser1_Enabled)));
        // Serial 2
        szText.append(QString::fromAscii("Serial 2 Enabled: \t%1\n") .arg(bool2String(lstTargets[nModel].ser2_Enabled)));
        // Serial 3
        szText.append(QString::fromAscii("Serial 3 Enabled: \t%1\n") .arg(bool2String(lstTargets[nModel].ser3_Enabled)));
        // Can0
        szText.append(QString::fromAscii("Can 0 Enabled: \t%1\n") .arg(bool2String(lstTargets[nModel].can0_Enabled)));
        // Can1
        szText.append(QString::fromAscii("Can 1 Enabled: \t%1\n") .arg(bool2String(lstTargets[nModel].can1_Enabled)));
    }
    return szText;
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
        m_rebuildDeviceTree = true;
        m_rebuildTimingTree = true;
        m_fMultiSelect = false;
        enableInterface();
    }
}
void ctedit::tabSelected(int nTab)
// Change current Tab
{
    int nPrevTab = m_nCurTab;
    qDebug() << QString::fromAscii("tabSelected(): Previous Tab: %1 - New Tab: %2") .arg(nPrevTab) .arg(nTab);
    //---------------------------------
    // Gestione del Tab di Provenienza
    //---------------------------------
    if (nPrevTab == TAB_SYSTEM)  {
        // Rilegge all'indetro le info di configurazione eventualmente aggiornate da system.ini
        mectSet->getTargetConfig(panelConfig);
        // Aggiorna le abilitazioni dei protocolli in funzione delle porte abilitate
        enableProtocolsFromModel();
        m_rebuildDeviceTree = true;
        m_rebuildTimingTree = true;
    }
    // Ritorno a CT da Tab MPNC / MPNE
    else if (nPrevTab == TAB_MPNC) {
        // Se qualcosa della configurazione è cambiato rilegge la CT dalla Lista
        int nOldRow = m_nGridRow;
        if (configMPNC->isUpdated())  {
            ui->tblCT->selectionModel()->clearSelection();
            lstUndo.append(lstCTRecords);
            lstCTRecords = configMPNC->localCTRecords;
            // Rinumera Blocchi con i cambiamenti selezionati (e ricarica griglia)
            if (! riassegnaBlocchi())  {
                m_szMsg = QLatin1String("Found Errors in Reassigning Blocks");
                warnUser(this, szMectTitle, m_szMsg);
            }
            m_rebuildDeviceTree = true;
            m_rebuildTimingTree = true;
        }
        // Jump n+1
        jumpToGridRow(nOldRow + 1, true);
        jumpToGridRow(nOldRow, true);
        m_fMultiSelect = false;
        enableInterface();
    }
    else if (nPrevTab == TAB_MPNE)  {
        // Se qualcosa della configurazione è cambiato rilegge la CT dalla Lista
        int nOldRow = m_nGridRow;
        if (configMPNE->isUpdated())  {
            ui->tblCT->selectionModel()->clearSelection();
            lstUndo.append(lstCTRecords);
            lstCTRecords = configMPNE->localCTRecords;
            // Rinumera Blocchi con i cambiamenti selezionati (e ricarica griglia)
            if (! riassegnaBlocchi())  {
                m_szMsg = QLatin1String("Found Errors in Reassigning Blocks");
                warnUser(this, szMectTitle, m_szMsg);
            }
            m_rebuildDeviceTree = true;
            m_rebuildTimingTree = true;
        }
        // Jump n+1
        jumpToGridRow(nOldRow + 1, true);
        jumpToGridRow(nOldRow, true);
        m_fMultiSelect = false;
        enableInterface();
    }
    //---------------------------------
    // Gestione del Tab di Destinazione
    //---------------------------------
    // Ritorno a CT Editor Main Tab
    if (nTab == TAB_CT)  {
        // Nulla di particolare
        this->setCursor(Qt::ArrowCursor);
    }
    // Entering SYSTEM Editor
    else if (nTab == TAB_SYSTEM)  {
        mectSet->enableSerialPanel(panelConfig.ser0_Enabled && panelConfig.ser0_Editable,
                                   panelConfig.ser1_Enabled && panelConfig.ser1_Editable,
                                   panelConfig.ser2_Enabled && panelConfig.ser2_Editable,
                                   panelConfig.ser3_Enabled && panelConfig.ser3_Editable);
    }
    // Entering Trends: Aggiornamento della lista di variabili e ripopolamento liste per Trends
    else if (nTab == TAB_TREND) {
        trendEdit->updateVarLists(lstLoggedVars);
        trendEdit->fillTrendsCombo(m_szCurrentCTPath);
    }
    // Passaggio a tab DEVICES
    else if (nTab == TAB_DEVICES)  {
        // Ricostruisce l'albero dei Devices
        if (m_isCtModified || nPrevTab == TAB_SYSTEM || m_rebuildDeviceTree)  {
            qDebug() << QString::fromAscii("Device Tree Columns: %1 - Rebuilded") .arg(ui->deviceTree->columnCount());
            fillDeviceTree(m_nGridRow);
        }
    }
    // Passaggio a tab TIMINGS
    else if (nTab == TAB_TIMINGS)  {
        // Ricostruisce l'albero dei Timings
        if (m_isCtModified || nPrevTab == TAB_SYSTEM || m_rebuildTimingTree)  {
            qDebug() << QString::fromAscii("Timings Tree Columns: %1 - Rebuilded") .arg(ui->timingTree->columnCount());
            fillTimingsTree(m_nGridRow);
        }
    }
    else if (nTab == TAB_MPNC)  {
        showTabMPNC();
    }
    else if (nTab == TAB_MPNE)  {
        showTabMPNE();
    }
    else if (nTab == TAB_LOG)  {
        qDebug() << QString::fromAscii("Log Tab Selected %1") .arg(ui->logTree->columnCount());
        fillLogTree(m_nGridRow);
    }
    // Set Current Tab
    m_nCurTab = nTab;
}

void ctedit::enableInterface()
// Abilita l'interfaccia in funzione dello stato del sistema
{
    bool        fMultiEdit = m_fMultiEdit && m_fMultiSelect;
    QString     szFameEditBackGround;


    szFameEditBackGround.clear();
    // Imposta lo sfondo del frame di Editing
    ui->fraEdit->setStyleSheet(QLatin1String(""));
    szFameEditBackGround = QLatin1String("background-");
    if (fMultiEdit)  {
        szFameEditBackGround.append(szColorMultiEdit);
    }
    else  {
        szFameEditBackGround.append(szColorNormalEdit);
    }
    ui->cboSections->setEnabled(! fMultiEdit);
    ui->fraEdit->setStyleSheet(szFameEditBackGround);
    // Abilitazioni elementi di interfaccia da impostare ad ogni cambio riga
    ui->cmdHideShow->setVisible(! fMultiEdit);
    ui->cmdMultiEdit->setEnabled(true);
    ui->cmdSearch->setVisible(! fMultiEdit);
    ui->cmdImport->setVisible(! fMultiEdit);
    ui->cmdUndo->setVisible(true);
    ui->cmdUndo->setEnabled(lstUndo.count() > 0);
    ui->cmdBlocchi->setVisible(false);
    ui->cmdBlocchi->setEnabled(false);
    ui->cmdSave->setVisible(! fMultiEdit);
    ui->cmdCompile->setVisible(! fMultiEdit);
    ui->cmdCompile->setEnabled(! m_isCtModified && ! m_szCurrentModel.isEmpty() && ! fMultiEdit);
    // Salva sempre abilitato, bordo green se non ci sono salvataggi pendenti
    ui->cmdSave->setEnabled(! m_szCurrentModel.isEmpty() && ! fMultiEdit);
    if (m_isCtModified)  {
        ui->cmdSave->setStyleSheet(QLatin1String("border: 2px solid red;"));
    }
    else  {
        ui->cmdSave->setStyleSheet(QLatin1String("border: 2px solid green;"));
    }
    ui->cmdPLC->setVisible(! fMultiEdit);
    ui->cmdPLC->setEnabled(! m_isCtModified && ! m_szCurrentModel.isEmpty() && ! fMultiEdit);
    // Frame MultiEdit
    ui->lblEditableFields->setVisible(fMultiEdit);
    ui->lstEditableFields->setVisible(fMultiEdit);
    ui->cmdApply->setVisible(fMultiEdit);
    ui->cmdApply->setEnabled(fMultiEdit);
    ui->fraCondition->setEnabled(! fMultiEdit);
    ui->tblCT->setEnabled(true);
    m_fCutOrPaste = false;
    // Abilitazione del Tab MPNC e MPNE solo se esiste una Seriale disponibile nel sistema
    m_nMPNC = -1;
    if (isSerialPortEnabled && ! fMultiEdit)  {
        bool enableMPNC = searchIOModules(szMPNC006, lstCTRecords, lstMPNC006_Vars, lstMPNC);
        bool enableMPNE = searchIOModules(szMPNE1001, lstCTRecords, lstMPNE_Vars, lstMPNE);
        m_nMPNC = 0;
        m_nMPNE = 0;
        ui->tabWidget->setTabEnabled(TAB_MPNC, enableMPNC);
        ui->tabWidget->setTabEnabled(TAB_MPNE, enableMPNE);
    }
    else {
        ui->tabWidget->setTabEnabled(TAB_MPNC, false);
        ui->tabWidget->setTabEnabled(TAB_MPNE, false);
    }
    // Tab Logged Vars
    int nFast = 0;
    int nSlow = 0;
    int nVar = 0;
    int nShot = 0;
    if (countLoggedVars(lstCTRecords, nFast, nSlow, nVar, nShot) > 0)  {
        ui->tabWidget->setTabEnabled(TAB_LOG, ! fMultiEdit);
    }
    else  {
        ui->tabWidget->setTabEnabled(TAB_LOG, false);
    }
}
void    ctedit::enableProtocolsFromModel()
// Abilita i Protocolli in funzione della configurazione del Modello corrente (da TargetConfig)
{
    int nCur = 0;
    QString szPort;


    // qDebug() << QString::fromAscii("Model Searched: %1 - Found: %2") .arg(szModel) .arg(nModel);

    lstBusEnabler.clear();
    // Abilita di default tutti i Protocolli
    for (nCur = 0; nCur < lstProtocol.count(); nCur++)  {
        lstBusEnabler.append(false);
    }
    // PLC abilitato per tutti i modelli
    lstBusEnabler[PLC] = true;
    // TCP e derivati abilitati per tutti i modelli (Perchè ethPorts è almeno 1)
    if (panelConfig.ethPorts > 0)  {
        lstBusEnabler[TCP] = true;
        lstBusEnabler[TCPRTU] = true;
        lstBusEnabler[TCP_SRV] = true;
        lstBusEnabler[TCPRTU_SRV] = true;
    }
    // Protocollo CAN abilitato solo per Modelli con Can
    if (panelConfig.can0_Enabled || panelConfig.can1_Enabled)  {
        lstBusEnabler[CANOPEN] = true;
    }
    // Protocolli Seriali
    isSerialPortEnabled = panelConfig.ser0_Enabled || panelConfig.ser1_Enabled || panelConfig.ser2_Enabled || panelConfig.ser3_Enabled;        // Vero se almeno una porta seriale è abilitata
    nPresentSerialPorts = 0;            // Numero di porte Seriali utilizzabili a bordo
    // Calcolo delle porte seriali presenti a bordo
    if (isSerialPortEnabled)  {
        lstBusEnabler[RTU] = true;
        lstBusEnabler[MECT_PTC] = true;
        lstBusEnabler[RTU_SRV] = true;
        // Abilitazione delle entry nella Combo delle Porte
        nPresentSerialPorts = enableSerialPortCombo(ui->cboPort);
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
            fillComboVarNames(ui->cboVariable1, lstAllVarTypes, lstNoHUpdates, true);
        }
        if (ui->cboVariable2->count() <= 0)  {
            fillComboVarNames(ui->cboVariable2, lstAllVarTypes, lstNoHUpdates, true);
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
    // Ricaricamento della liste Device-Nodi per i controlli di unicità registro
    // Rebuild Server-Device-Nodes structures
    qDebug() << QLatin1String("globalChecks(): Starting");
    if (! checkServersDevicesAndNodes())  {
        m_szMsg = QLatin1String("Error checking Device and Nodes structure, cannot continue checks !");
        warnUser(this, szMectTitle, m_szMsg);
        return 1;
    }
    qDebug() << QLatin1String("globalChecks(): checkServersDevicesAndNodes Done");
    // Ripulitura Array Server ModBus (1 per ogni Seriale + Protocolli TCP_SRV e TCPRTU_SRV
    for (nRow = 0; nRow < srvTotals; nRow++)  {
        serverModBus[nRow].nProtocol = -1;
        serverModBus[nRow].nPort = -1;
        serverModBus[nRow].nodeId = -1;
    }
    qDebug() << QLatin1String("globalChecks(): Clean Servers List Done");
    // Ciclo Globale su tutti gli Items di CT
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Controlla solo righe utilizzate
        if (lstCTRecords[nRow].Enable)  {
            // qDebug() << QString::fromAscii("globalChecks(): Checking Row: %1") .arg(nRow);
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
            fRecOk = recCT2FieldsValues(lstCTRecords, lstFields, nRow);
            if (fRecOk)  {
                nErrors += checkFormFields(nRow, lstFields, false);
            }
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
    qDebug() << QLatin1String("globalChecks(): Ended");
    return nErrors;
}
int ctedit::fillVarList(QStringList &lstVars, QList<int> &lstTypes, QList<int> &lstUpdates, bool fSkipVarDecimal)
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
            // Skip Vars with decimal number defined with another variable [Only for NON BIT Variables]
            if (fSkipVarDecimal)  {
                if (! isBitField(lstCTRecords[nRow].VarType) && lstCTRecords[nRow].Decimal > 3)  {
                    f2Add = false;
                }
            }
            // If Var is defined and of a correct type, insert in list
            if (f2Add && fUpdateOk)  {
                lstVars.append(QLatin1String(lstCTRecords[nRow].Tag));
            }
        }
    }
    // Ordimanento Alfabetico della Lista
    lstVars.sort();
    // Return value
    // qDebug() << "Items Added to List:" << lstVars.count();
    return lstVars.count();
}

int ctedit::fillComboVarNames(QComboBox *comboBox, QList<int> &lstTypes, QList<int> &lstUpdates, bool fSkipVarDecimal)
// Caricamento ComboBox con Nomi Variabili filtrate in funzione del Tipo e della Persistenza
{
    QStringList lstVars;
    int         nItem = 0;
    bool        oldState = comboBox->blockSignals(true);

    lstVars.clear();
    comboBox->setCurrentIndex(-1);
    comboBox->clear();
    if (fillVarList(lstVars, lstTypes, lstUpdates, fSkipVarDecimal) > 0)
    {
        for (nItem = 0; nItem < lstVars.count(); nItem++)  {
            comboBox->addItem(lstVars[nItem]);
        }
        comboBox->setCurrentIndex(-1);
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
    int         nDecimals = 0;
    int         nUpdate = -1;
    int         nProtocol = -1;
    int         nPort = -1;
    int         nNodeID = -1;
    int         nRegister = -1;
    uint32_t    ipNum = 0;
    bool        fOk = false;
    Err_CT      errCt;
    QString     szTemp;
    QString     szVarName;
    QString     szIP;
    QString     szVar1;
    QString     szNodeID;
    bool        fMultiEdit = m_fMultiEdit && m_fMultiSelect;

    // Form per Display Errori
    cteErrorList    *errWindow;

    // Clear Error List if single line show
    if (fSingleLine)
        lstCTErrors.clear();
    // Recupero Variable Name (Per finestra errore)
    if (fMultiEdit)  {
        szVarName = ui->tblCT->item(nRow, colName)->text().trimmed();
    }
    else  {
        szVarName = lstValues[colName];
    }
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
    // Controllo Group (per ora nessun controllo)
    //---------------------------------------
    szTemp = lstValues[colGroup];
    //---------------------------------------
    // Controllo Module (per ora nessun controllo)
    //---------------------------------------
    szTemp = lstValues[colModule];
    //---------------------------------------
    // Controllo Variable Name
    //---------------------------------------
    if (! isValidVarName(szVarName))  {
        fillErrorMessage(nRow, colName, errCTNoName, szVarName, szVarName, chSeverityError, &errCt);
        lstCTErrors.append(errCt);
        nErrors++;
    }
    // Controllo Lunghezza (si può sfiorare solo in lettura da file)
    if (szVarName.length() > MAX_IDNAME_LEN)  {
        szTemp = QLatin1String("Name Lenght Exceedes: ") + QString::number(MAX_IDNAME_LEN) + QLatin1String(" chars. Will be truncated");
        fillErrorMessage(nRow, colName, errCTNameTooLong, szVarName, szTemp, chSeverityWarning, &errCt);
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
        nDecimals = szTemp.toInt(&fOk);
        nDecimals = fOk ? nDecimals : 0;
        // Decimali a 0 per tipo Bit
        if (nType == BIT && nDecimals > 0)  {
            fillErrorMessage(nRow, colDecimal, errCTNoDecimalZero, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        else if (nType == BYTE_BIT) {
            if (nDecimals < 1 or nDecimals > 8)  {
                fillErrorMessage(nRow, colDecimal, errCTWrongDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        else if (nType == WORD_BIT) {
            if (nDecimals < 1 or nDecimals > 16)  {
                fillErrorMessage(nRow, colDecimal, errCTWrongDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        else if (nType == DWORD_BIT) {
            if (nDecimals < 1 or nDecimals > 32)  {
                fillErrorMessage(nRow, colDecimal, errCTWrongDecimals, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        // Numero Decimali > 4 ===> Variable (per tipi differenti da Bit in tutte le versioni possibili)
        else if (nDecimals >= 4 && nType != BYTE_BIT && nType == WORD_BIT && nType == DWORD_BIT)  {
            // Controlla che il numero indicato punti ad una variabile del tipo necessario a contenere il numero di decimali
            if (nDecimals > DimCrossTable || ! lstCTRecords[nDecimals - 1].Enable ||
                    (lstCTRecords[nDecimals - 1].VarType != UINT8 &&  lstCTRecords[nDecimals - 1].VarType != UINT16 && lstCTRecords[nDecimals - 1].VarType != UINT16BA &&
                     lstCTRecords[nDecimals - 1].VarType != UDINT &&  lstCTRecords[nDecimals - 1].VarType != UDINTDCBA && lstCTRecords[nDecimals - 1].VarType != UDINTCDAB &&
                     lstCTRecords[nDecimals - 1].VarType != UDINTBADC ) )   {
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
    ipNum = 0;
    if (nProtocol == TCP || nProtocol == TCPRTU || nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV)  {
        if (szIP.isEmpty())  {
            fillErrorMessage(nRow, colIP, errCTNoIP, szVarName, szIP, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // IP Valido e diverso da 0 se non server
        ipNum = str2ipaddr(szIP.toAscii().data());
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
    // Calcolo Numero Porta
    szTemp = lstValues[colPort];
    nPort = szTemp.isEmpty() ? -1 : szTemp.toInt(&fOk);
    nPort = fOk && nPort != -1 ? nPort : -1;
    // Calcolo numero Nodo
    szNodeID = lstValues[colNodeID];
    nNodeID = szNodeID.isEmpty() ? -1 : szNodeID.toInt(&fOk);
    nNodeID = fOk && nNodeID != -1 ? nNodeID : -1;
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
            // Controllo disabilitato per TPCL050 su Protocollo RTU - Porta 3 - Nodo 1
            if (! (panelConfig.modelName.contains(szTPLC050) && nProtocol == RTU && nPort == 3 && nNodeID == 1))  {
                fillErrorMessage(nRow, colPort, errCTNoDevicePort, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
        // Numero di Nodo non utilizzabile su TPAC_1007_04_AE per Variabili Utente (Nodo 20) su Porta 3
        if ( (nRow < MAX_NONRETENTIVE) && (panelConfig.modelName.contains(QLatin1String(product_name[TPAC1007_04_AE]))) && (nPort == 3) && (nNodeID == 20))  {
            fillErrorMessage(nRow, colPort, errCTNoNode, szVarName, szTemp, chSeverityError, &errCt);
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
    if (nProtocol != PLC)  {
        // Il Node ID deve essere compreso tra 0 e 255
        if (nNodeID < 0 || nNodeID > nMaxNodeID)  {
            fillErrorMessage(nRow, colNodeID, errCTNoNode, szVarName, szNodeID, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        /*
        // Per RTU valori 0..247
        else if (nProtocol == RTU && nNodeID > nMaxRTUNodeID)  {
            szTemp = QLatin1String("Port: ") + lstValues[colPort] + QLatin1String("Must be from 0 to ") + QString::number(nMaxRTUNodeID);
            fillErrorMessage(nRow, colNodeID, errCTNoNode, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        // Per TCP sono ammessi solo i valori 0 o 255
        else if (nProtocol == TCP && (nNodeID > 0 || nNodeID < nMaxNodeID))  {
            szTemp = QLatin1String("Port: ") + lstValues[colPort] + QLatin1String("Must be from 0 or ") + QString::number(nMaxNodeID);
            fillErrorMessage(nRow, colNodeID, errCTNoNode, szVarName, szTemp, chSeverityError, &errCt);
            lstCTErrors.append(errCt);
            nErrors++;
        }
        */
    }
    //---------------------------------------
    // Controlli Univocità Server (MODBUS)
    //---------------------------------------
    if (nProtocol == TCP_SRV)  {
        // Non esiste ancora un Server TCP definito
        if (serverModBus[srvTCP].nPort < 0)  {
            serverModBus[srvTCP].nPort = nPort;
            serverModBus[srvTCP].nProtocol = nProtocol;
            serverModBus[srvTCP].szIpAddress = szIP;
            serverModBus[srvTCP].nodeId = nNodeID;
        }
        else  {
            // Controllo su Porta
            if (serverModBus[srvTCP].nPort != nPort)  {
                szTemp = QLatin1String("Port: ") + lstValues[colPort] + QLatin1String(" Vs TCP Server Port: ") + QString::number(serverModBus[srvTCP].nPort);
                fillErrorMessage(nRow, colPort, errCTModBusServerDuplicate, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
            // Controllo su Node ID
            if (serverModBus[srvTCP].nodeId != nNodeID)  {
                szTemp = QLatin1String("Node Id: ") + lstValues[colNodeID] + QLatin1String(" Vs TCP Server Node Id: ") + QString::number(serverModBus[srvTCP].nodeId);
                fillErrorMessage(nRow, colNodeID, errCTModBusServerDuplicate, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
    }
    else if (nProtocol == TCPRTU_SRV)  {
        // Non esiste ancora un Server TCP_RTU definito
        if (serverModBus[srvTCPRTU].nPort < 0)  {
            serverModBus[srvTCPRTU].nPort = nPort;
            serverModBus[srvTCPRTU].nProtocol = nProtocol;
            serverModBus[srvTCPRTU].szIpAddress = szIP;
            serverModBus[srvTCPRTU].nodeId = nNodeID;
        }
        else  {
            // Controllo su Porta
            if (serverModBus[srvTCPRTU].nPort != nPort)  {
                szTemp = QLatin1String("Port: ") + lstValues[colPort] + QLatin1String(" Vs TCP_RTU Server Port: ") + QString::number(serverModBus[srvTCPRTU].nPort);
                fillErrorMessage(nRow, colPort, errCTModBusServerDuplicate, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
            // Controllo su Node ID
            if (serverModBus[srvTCPRTU].nodeId != nNodeID)  {
                szTemp = QLatin1String("Node Id: ") + lstValues[colNodeID] + QLatin1String(" Vs TCP_RTU Server Node Id: ") + QString::number(serverModBus[srvTCPRTU].nodeId);
                fillErrorMessage(nRow, colNodeID, errCTModBusServerDuplicate, szVarName, szTemp, chSeverityError, &errCt);
                lstCTErrors.append(errCt);
                nErrors++;
            }
        }
    }
    else if (nProtocol == RTU_SRV)  {
        // Non esiste ancora un Server RTU_SRV definito per la porta richiesta
        // # Porta entro i Range
        if (nPort >= 0 && nPort <= nMaxSerialPorts) {
            if (serverModBus[nPort].nPort < 0)  {
                serverModBus[nPort].nPort = nPort;
                serverModBus[nPort].nProtocol = nProtocol;
                serverModBus[nPort].szIpAddress = szIP;
                serverModBus[nPort].nodeId = nNodeID;
            }
            else {
                // Controllo su Node ID
                if (serverModBus[nPort].nodeId != nNodeID)  {
                    szTemp = QLatin1String("Node Id: ") + lstValues[colNodeID] + QLatin1String(" Vs RTU Server Node Id: ") + QString::number(serverModBus[nPort].nodeId);
                    fillErrorMessage(nRow, colNodeID, errCTModBusServerDuplicate, szVarName, szTemp, chSeverityError, &errCt);
                    lstCTErrors.append(errCt);
                    nErrors++;
                }
            }
        }
    }
    //---------------------------------------
    // Controllo per Register (MODBUS)
    //---------------------------------------
    if (! fMultiEdit)  {
        szTemp = lstValues[colRegister];
        nRegister = szTemp.isEmpty() ? -1 : szTemp.toInt(&fOk);
        nRegister = fOk && nRegister != -1 ? nRegister : -1;
        if (nProtocol != PLC)  {
            if (nProtocol== TCP_SRV || nProtocol == TCPRTU_SRV)  {
                // Range allowed 0..MAX_TCPSRV_REGS (4095)
                if (nRegister > MAX_TCPSRV_REGS)  {
                    fillErrorMessage(nRow, colRegister, errCTRegisterTooBig, szVarName, szTemp, chSeverityError, &errCt);
                    lstCTErrors.append(errCt);
                    nErrors++;
                }
            }
            else {
                // Range allowed: 0..65535
                if (nRegister < 0 || nRegister > nMax_Int16)  {
                    fillErrorMessage(nRow, colRegister, errCTNoRegister, szVarName, szTemp, chSeverityError, &errCt);
                    lstCTErrors.append(errCt);
                    nErrors++;
                }
                // Input Register allowed only on MODBUS Client (no SERVER)
                if ((lstValues[colInputReg] == szTRUE) &&
                    ! ((nProtocol == RTU) ||
                       (nProtocol == TCPRTU)) )  {
                    fillErrorMessage(nRow, colRegister, errCTInputOnlyModbusClient, szVarName, szTemp, chSeverityError, &errCt);
                    lstCTErrors.append(errCt);
                    nErrors++;
                }
            }
            // Controllo Registro già utilizzato per ModBus
            // Solo come controllo globale (prima di salvataggio)
            if (! fSingleLine && isModbus(nProtocol))  {
                QList<int> lstUsingRegister;
                int nUsingRegister = checkRegister(nRow, lstUsingRegister);
                if (nUsingRegister >= 0 && lstUsingRegister.count() > 0)  {
                    szTemp.clear();
                    int nDuplicate = 0;
                    for (nDuplicate = 0; nDuplicate < lstUsingRegister.count(); nDuplicate++)  {
                        szTemp.append(QString::fromAscii("Row %1 ") .arg(lstUsingRegister[nDuplicate] + 1));
                    }
                    fillErrorMessage(nRow, colRegister, errCTRegisterUsedTwice, szVarName, szTemp, chSeverityError, &errCt);
                    lstCTErrors.append(errCt);
                    nErrors++;
                }
            }
        }
    }
    //---------------------------------------
    // Controllo Behavior
    //---------------------------------------
    szTemp = lstValues[colBehavior].trimmed();
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
        varTypes    nTypeVar1 = UNKNOWN;
        int         nDecVar1 = -1;
        int         nDecVar2 = -1;
        QStringList lstAlarmVars;

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
        fillVarList(lstAlarmVars, lstAllVarTypes, lstNoHUpdates, true);
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
            if (nPos >= 0 && nPos < lstCTRecords.count())  {
                nTypeVar1 = lstCTRecords[nPos].VarType;
                nDecVar1 = lstCTRecords[nPos].Decimal;
            }
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
                    // Controllo di coerenza sul Valore da confrontare con il tipo della variabile
                    if (dblVal < 0.0 &&
                        (nTypeVar1 ==  UINT8 || nTypeVar1 ==  UINT16 || nTypeVar1 ==  UINT16BA || nTypeVar1 ==  UDINT || nTypeVar1 ==  UDINTDCBA || nTypeVar1 ==  UDINTCDAB || nTypeVar1 ==  UDINTBADC))  {
                        // Valore Negativo comparato con un valore UNSIGNED
                        fillErrorMessage(nRow, colCompare, errCTNegWithUnsigned, szVarName, szTemp, chSeverityError, &errCt);
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
                    // Verifica di Compatibilità di tipo e decimali tra i due operandi
                    if (nPos >= 0 && nPos < lstCTRecords.count())  {
                        varTypes tipoVar2 = lstCTRecords[nPos].VarType;
                        nDecVar2 = lstCTRecords[nPos].Decimal;
                        if (! checkVarsCompatibility(nTypeVar1, nDecVar1, tipoVar2, nDecVar2))  {
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
            freeCTrec(lstCTRecords, nRow);
            // Ricarica Record vuoto a griglia
            fOk = recCT2FieldsValues(lstCTRecords, lstValues, nRow);
            fOk = list2GridRow(ui->tblCT, lstValues, lstHeadLeftCols, nRow);
            // Repaint Colori Grid
            if (fOk)  {
                setRowsColor();
            }
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
    int     nDecimals = 0;
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
            // Recupera il # decimali della variabile
            nDecimals = lstCTRecords[nRow].Decimal;
            // qDebug() << "Row First Variable in Alarm (Row - Name - nType - Type):" << nRow << szVarName << m_vtAlarmVarType << QLatin1String(varTypeName[m_vtAlarmVarType]);
            QString szVar1Type = QLatin1String(varTypeNameExtended[m_vtAlarmVarType]);
            szVar1Type.prepend(QLatin1String("["));
            szVar1Type.append(QLatin1String("]"));
            szVar1Type.append(QLatin1String(" - [Dec: "));
            szVar1Type.append(QString::number(nDecimals));
            szVar1Type.append(QLatin1String("]"));
            ui->lblTypeVar1->setText(szVar1Type);
            // Disabilita le voci per Rising and falling
            if (m_vtAlarmVarType == BIT || m_vtAlarmVarType == BYTE_BIT || m_vtAlarmVarType == WORD_BIT || m_vtAlarmVarType == DWORD_BIT)  {
                // qDebug() << "Condition Enabled";
                enableComboItem(ui->cboCondition, oper_rising);
                enableComboItem(ui->cboCondition, oper_falling);
                // Decimals for BYTE_BIT WORD_BIT DWORD_BIT is the Bit Index -> Forced to 0 for Validate
                nDecimals = 0;
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
            fillComboVarNames(ui->cboVariable2, lstVTypes, lstNoHUpdates, true);
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
            // Decimal Validator
            QDoubleValidator * validFixed = ((QDoubleValidator *) ui->txtFixedValue->validator());
            if (validFixed != 0)  {
                validFixed->setDecimals(nDecimals);
                validFixed->setNotation(QDoubleValidator::StandardNotation);
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
    char searchTag[MAX_IDNAME_LEN + 1];

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
bool ctedit::checkVarsCompatibility(varTypes nTypeV1, int nDecV1, varTypes nTypeV2, int nDecV2)
// Controllo di compatibilità completo tra i due operandi di un Allarme
{
    bool   isCompatible = false;

    switch (nTypeV1) {

    case BIT:
    case BYTE_BIT:
    case WORD_BIT:
    case DWORD_BIT:
        switch (nTypeV2) {
        case BIT:
        case BYTE_BIT:
        case WORD_BIT:
        case DWORD_BIT:
            isCompatible = true;
            break;
        case INT16:
        case INT16BA:
        case DINT:
        case DINTDCBA:
        case DINTCDAB:
        case DINTBADC:
            isCompatible = false; // only == 0 and != 0
            break;
        case UINT8:
        case UINT16:
        case UINT16BA:
        case UDINT:
        case UDINTDCBA:
        case UDINTCDAB:
        case UDINTBADC:
            isCompatible = false; // only == 0 and != 0
            break;
        case REAL:
        case REALDCBA:
        case REALCDAB:
        case REALBADC:
            isCompatible = false; // only == 0 and != 0
            break;
        default:
            ; // FIXME: assert
        }
        break;

    case INT16:
    case INT16BA:
    case DINT:
    case DINTDCBA:
    case DINTCDAB:
    case DINTBADC:
        switch (nTypeV2) {
        case BIT:
        case BYTE_BIT:
        case WORD_BIT:
        case DWORD_BIT:
            isCompatible = false;
            break;
        case INT16:
        case INT16BA:
        case DINT:
        case DINTDCBA:
        case DINTCDAB:
        case DINTBADC:
            isCompatible = (nDecV1 == nDecV2);
            break;
        case UINT8:
        case UINT16:
        case UINT16BA:
        case UDINT:
        case UDINTDCBA:
        case UDINTCDAB:
        case UDINTBADC:
            isCompatible = false;
            break;
        case REAL:
        case REALDCBA:
        case REALCDAB:
        case REALBADC:
            isCompatible = false;
            break;
        default:
            ; // FIXME: assert
        }
        break;

    case UINT8:
    case UINT16:
    case UINT16BA:
    case UDINT:
    case UDINTDCBA:
    case UDINTCDAB:
    case UDINTBADC:
        switch (nTypeV2) {
        case BIT:
        case BYTE_BIT:
        case WORD_BIT:
        case DWORD_BIT:
            isCompatible = false;
            break;
        case INT16:
        case INT16BA:
        case DINT:
        case DINTDCBA:
        case DINTCDAB:
        case DINTBADC:
            isCompatible = false;
            break;
        case UINT8:
        case UINT16:
        case UINT16BA:
        case UDINT:
        case UDINTDCBA:
        case UDINTCDAB:
        case UDINTBADC:
            isCompatible = (nDecV1 == nDecV2);
            break;
        case REAL:
        case REALDCBA:
        case REALCDAB:
        case REALBADC:
            isCompatible = false;
            break;
        default:
            ; // FIXME: assert
        }
        break;

    case REAL:
    case REALDCBA:
    case REALCDAB:
    case REALBADC:
        switch (nTypeV2) {
        case BIT:
        case BYTE_BIT:
        case WORD_BIT:
        case DWORD_BIT:
            isCompatible = false;
            break;
        case INT16:
        case INT16BA:
        case DINT:
        case DINTDCBA:
        case DINTCDAB:
        case DINTBADC:
            isCompatible = false;
            break;
        case UINT8:
        case UINT16:
        case UINT16BA:
        case UDINT:
        case UDINTDCBA:
        case UDINTCDAB:
        case UDINTBADC:
            isCompatible = false;
            break;
        case REAL:
        case REALDCBA:
        case REALCDAB:
        case REALBADC:
            isCompatible = true; // no decimal test
            break;
        default:
            ;
        }
        break;

    default:
        ;
    }
    // Return Value
    return isCompatible;
}

void ctedit::on_cboPriority_currentIndexChanged(int index)
// Trucco per impostare il valore del #Blocco e altri valori nel caso di nuova riga
{

    if (lstCTRecords.count() <= 0 ||  index < 0 || m_nGridRow < 0)
        return;
    // Applicazione dei valori di default nel caso di una riga vuota
    if (! lstCTRecords[m_nGridRow].UsedEntry && index >= 0 && m_nGridRow < MAX_NONRETENTIVE -1)  {
        // qDebug() << QString::fromAscii("Adding Row: %1") .arg(m_nGridRow);
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
            // Input Register
            // ui->chkInputRegister->setChecked(lstCTRecords[m_nGridRow - 1].InputReg == 1);
            // Register
            ui->txtRegister->setText(QString::number(lstCTRecords[m_nGridRow - 1].Offset + varSizeInBlock(lstCTRecords[m_nGridRow - 1].VarType)));
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
            ui->cboBehavior->setCurrentIndex(behavior_readwrite);
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
        // qDebug() << QLatin1String("Clear Alarm");
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
        m_szMsg = QString::fromAscii("PLC Project File Not Found:\n<%1>") .arg(szPlcPro2Show);
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
        // qDebug() << QString::fromAscii("Env. %1 Variable: <%2>") .arg(szPLCEnvVar) .arg(szPathPLCApplication);
        // To be modified with specifics of PLC Application
        szTemp = QLatin1String("%1");
        // Remove %1
        szPathPLCApplication.remove(szTemp, Qt::CaseInsensitive);
        // Remove doublequote
        szPathPLCApplication.remove(szDOUBLEQUOTE, Qt::CaseInsensitive);
        szPathPLCApplication = szPathPLCApplication.trimmed();
        // qDebug() << QString::fromAscii("Editor PLC: <%1>") .arg(szPathPLCApplication);
        // Build PLC Editor Application command
        QFileInfo plcExe(szPathPLCApplication);
        if (plcExe.exists())  {
            szPLCEngPath = plcExe.absolutePath();
            // qDebug() << QString::fromAscii("Path PLC: <%1>") .arg(szPLCEngPath);
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
                m_szMsg = QString::fromAscii("Error Starting PLC Engineering: %1\n") .arg(errPlc);
                m_szMsg.append(szCommand);
                warnUser(this, szTitle, m_szMsg);
                goto endStartPLC;
            }
        }
        else {
            m_szMsg = QString::fromAscii("Program PLC Engineering Not Found!\n%1") .arg(szPLCEnvVar);
            m_szMsg.append(szCommand);
            warnUser(this, szTitle, m_szMsg);
            goto endStartPLC;
        }
    }
    else  {
        m_szMsg = QString::fromAscii("Environment Variable for Application PLC Engineering %1 Not Found!\n") .arg(szPLCEnvVar);
        m_szMsg.append(szCommand);
        warnUser(this, szTitle, m_szMsg);
    }
#elif _WIN32
    // windows code goes here
    // Open only File URL
    if (! showFile(szPlcPro2Show))  {
        m_szMsg = QString::fromAscii("Error Opening URL: %1\n") .arg(szPlcPro2Show);
        warnUser(this, szMectTitle, m_szMsg);
    }
#endif

endStartPLC:
    return;
}
void ctedit::on_chkInputRegister_stateChanged(int state)
// Cambio di Stato della Checkbox Input Register
{
    if (ui->chkInputRegister->isVisible() && ui->lblInputRegister->isVisible())  {
        QString szStyle;

        if (state == Qt::Checked)  {
            szStyle = QLatin1String("color: Red");
        }
        else  {
            szStyle = QLatin1String("color: Black");
        }
        ui->chkInputRegister->setStyleSheet(szStyle);
        ui->lblInputRegister->setStyleSheet(szStyle);
    }
}

bool ctedit::eventFilter(QObject *obj, QEvent *event)
// Gestore Event Handler
{
    static      int nPrevKey = 0;
    bool        fMultiEdit = m_fMultiEdit && m_fMultiSelect && m_nCurTab == TAB_CT;

    // Evento Key Press
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // Tasto ESC (Intercettato per le Combo Box)
        if (keyEvent->key() == Qt::Key_Escape) {
            qDebug() << QLatin1String("ESC: Abort Editing");
            if (m_nCurTab == TAB_CT && ! fMultiEdit)  {
                // Abort Editing on current Row
                if (lstCTRecords[m_nGridRow].UsedEntry)  {
                    // Convert CT Record 2 User Values
                    QStringList lstFields;
                    bool fRes = recCT2FieldsValues(lstCTRecords, lstFields, m_nGridRow);
                    if (fRes)  {
                        fRes = values2Iface(lstFields, m_nGridRow);
                    }
                }
                return true;
            }
        }
        //-------------------------------------------------
        // Sequenze valide per tutto il Tab CrossTable Editor
        //-------------------------------------------------
        // Save su Tab CrossTable Editor
        if (keyEvent->matches(QKeySequence::Save)) {
            if (m_nCurTab == TAB_CT && ! fMultiEdit)  {
                // qDebug() << QLatin1String("Save");
                if (m_isCtModified)
                    on_cmdSave_clicked();
                return true;
            }
        }
        // F2 Rename Variable
        if (keyEvent->key() == Qt::Key_F2)  {
            if (m_nCurTab == TAB_CT && ! fMultiEdit)  {
                // Edit Var Name on Used Row
                if (lstCTRecords[m_nGridRow].UsedEntry)  {
                    qDebug() << QLatin1String("F2: Rename Var");
                    ui->txtName->selectAll();
                    ui->txtName->setFocus();
                    return true;
                }
            }
        }
        // Find su Tab CrossTable Editor
        if (keyEvent->matches(QKeySequence::Find)) {
            if (m_nCurTab == TAB_CT && ! fMultiEdit)  {
                // qDebug() << QLatin1String("Find");
                on_cmdSearch_clicked();
                return true;
            }
        }
        // Undo su Tab CrossTable Editor
        if (keyEvent->matches(QKeySequence::Undo)) {
            if (m_nCurTab == TAB_CT && ! fMultiEdit)  {
                // qDebug() << QLatin1String("Undo");
                if (! lstUndo.isEmpty())
                    on_cmdUndo_clicked();
                return true;
            }
        }
        // Goto Line su Tab CrossTable Editor
        if (keyEvent->key() == Qt::Key_L && nPrevKey == Qt::Key_Control && ! fMultiEdit)  {
            // qDebug() << QLatin1String("CTRL-L");
            if (m_nCurTab == TAB_CT)  {
                gotoRow();
                return true;
            }
        }
        // Return / Enter Button [differenziato tra Grid e Form di Data Entry]
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)  {
            if (m_nCurTab == TAB_CT && ! fMultiEdit)  {
                if (obj == ui->fraEdit)  {
                    int nextRow = findNextVisibleRow(m_nGridRow);
                    // Salto a riga successiva
                    jumpToGridRow(nextRow, false);
//                    bool fRowOk = false;
//                    // Enter su Editing Form
//                    // qDebug() << QLatin1String("Enter in Form");
//                    if (! isFormEmpty() && isLineModified(m_nGridRow)) {
//                        fRowOk = updateRow(m_nGridRow);
//                        if (fRowOk)  {
//                            int nextRow = findNextVisibleRow(m_nGridRow);
//                            // Salto a riga successiva
//                            jumpToGridRow(nextRow, false);
//                            // Eventuale Cambio Area utente
//                            setSectionArea(nextRow);
//                            m_isCtModified = true;
//                            // Aggiornamento interfaccia
//                            enableInterface();
//                        }
//                    }
                    // QKeyEvent newEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier, szEMPTY);
                    ui->tblCT->setFocus();
                    return true;
                }
                else if (obj == ui->tblCT)  {
                    // Enter su Grid
                    // qDebug() << QLatin1String("Enter in Grid");
                    // Salto a riga successiva
                    jumpToGridRow(findNextVisibleRow(m_nGridRow), false);
                    return true;
                }
            }
        }
        //-------------------------------------------------
        // Sequenze significative solo sul Grid del Tab CTE
        //-------------------------------------------------
        if (obj == ui->tblCT)  {
            // Tasto Insert
            if (keyEvent->key() == Qt::Key_Insert) {
                // qDebug() << QLatin1String("Insert");
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
        // qDebug() << QLatin1String("Pressed Key Value") << keyEvent->key();
        nPrevKey = keyEvent->key();
    }
    // Pass event to standard Event Handler
    return QObject::eventFilter(obj, event);
}
int ctedit::varSizeInBlock(int nVarType)
{
    int nSize = 1;

    switch (nVarType) {
        // Doppio Word
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
            nBlockSize = panelConfig.tcp_BlockSize;
            break;
        case CANOPEN:
            if (nPort == 0)
                nBlockSize = panelConfig.can0_BlockSize;
            else
                nBlockSize = panelConfig.can1_BlockSize;
            break;

        default:
            // All Serial Protocols
            if (nPort == 0)
                nBlockSize = panelConfig.ser0_BlockSize;
            else if (nPort == 1)
                nBlockSize = panelConfig.ser1_BlockSize;
            else if (nPort == 2)
                nBlockSize = panelConfig.ser2_BlockSize;
            else if (nPort == 3)
                nBlockSize = panelConfig.ser3_BlockSize;
            break;
    }
    // return
    return nBlockSize;
}
bool ctedit::isModbus(int nProtocol)
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
            lstCTRecords[nRow].InputReg == lstCTRecords[nRow - 1].InputReg &&
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
void ctedit::on_txtName_editingFinished()
// Modificato nome della variabile
{
    QString szVarName;
    QString szOldVarName;
    int nRow = 0;
    QTableWidgetItem    *tItem;

    disableAndBlockSignals(ui->txtName);
    if (ui->txtName->isModified())  {
        ui->txtName->setModified(false);
        szVarName = ui->txtName->text().trimmed();
        szOldVarName = QString::fromAscii(lstCTRecords[m_nGridRow].Tag).trimmed();
        // qDebug() << QString::fromAscii("Variable Name Changed from [%1] to [%2] @Row: <%3>") .arg(szOldVarName) .arg(szVarName) .arg(m_nGridRow);
        // Applicazione dei valori di default Priority nel caso di una riga vuota preceduta da una riga vuota o con protocollo differente
        if (! szVarName.isEmpty() && ! lstCTRecords[m_nGridRow].UsedEntry)  {
            // Solo se la riga è la prima di un blocco oppure ha un protocollo differente dalla precedente
            if ((m_nGridRow == 0) || (m_nGridRow > 0 && (lstCTRecords[m_nGridRow - 1].UsedEntry == 0 || lstCTRecords[m_nGridRow - 1].Protocol != ui->cboProtocol->currentIndex())))  {
                // Forza il livello di priorià a Alto
                ui->cboPriority->setCurrentIndex(nPriorityHigh);
            }
        }
        // Variabile già precedentemente utilizzata
        else if (! szVarName.isEmpty() && lstCTRecords[m_nGridRow].UsedEntry)  {
            // Se il nome della variabile è cambiato rispetto al valore precedente
            if (szVarName != szOldVarName && (! szVarName.isEmpty()))  {
                // Forza inserimento in lista Undo (al limite ci saranno 2 entry in lista Undo se esistono anche altre modifiche sulla riga corrente)
                lstUndo.append(lstCTRecords);
                // Aggiorna il nome della variabile in CT (Inibisce l'aggiornamento della lista Undo)
                strcpy(lstCTRecords[m_nGridRow].Tag, szVarName.toAscii().data());
                // Controlla che la variabile non sia presente nei valori SX e DX di un allarme
                for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
                    // Test solo per righe diverse dalla riga corrente e utilizzate
                    if (nRow != m_nGridRow && lstCTRecords[nRow].UsedEntry)  {
                        // Alarm Source Variable
                        if (szOldVarName == QLatin1String(lstCTRecords[nRow].ALSource))  {
                            strcpy(lstCTRecords[nRow].ALSource, szVarName.toAscii().data());
                            // Aggiornamento diretto delle variabile in Grid
                            tItem = ui->tblCT->item(nRow, colSourceVar);
                            tItem->setText(szVarName);
                            // qDebug() << QString::fromAscii("Replaced Source Var [%1] to [%2] @Row: <%3>") .arg(szOldVarName) .arg(szVarName) .arg(nRow);
                        }
                        // Alarm Compare Variable
                        if (szOldVarName == QLatin1String(lstCTRecords[nRow].ALCompareVar))  {
                            strcpy(lstCTRecords[nRow].ALCompareVar, szVarName.toAscii().data());
                            // Aggiornamento diretto delle variabile in Grid
                            tItem = ui->tblCT->item(nRow, colCompare);
                            tItem->setText(szVarName);
                            // qDebug() << QString::fromAscii("Replaced Compare Var [%1] to [%2] @Row: <%3>") .arg(szOldVarName) .arg(szVarName) .arg(nRow);
                        }
                    }
                }
            }
        }        
        // Ricarica le combo dei nomi variabili
        fillComboVarNames(ui->cboVariable1, lstAllVarTypes, lstNoHUpdates, true);
        fillComboVarNames(ui->cboVariable2, lstAllVarTypes, lstNoHUpdates, true);
    }
    enableAndUnlockSignals(ui->txtName);
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
            QStringList tokenList;
            QFile textFile(szSourceFile);
            textFile.open(QIODevice::ReadOnly);
            QTextStream textStream(&textFile);
            while (! textStream.atEnd())
            {
                QString line = textStream.readLine();
                if (line.isNull())
                    break;
                else  {
                    line = line.trimmed();
                    if (! line.isEmpty())  {
                        stringList.append(line);
                        tokenList.clear();
                        tokenList = line.split(szSEMICOL, QString::KeepEmptyParts);
                        // qDebug() << QString::fromAscii("CT Columns: %1") .arg(tokenList.count());
                        if (tokenList.count() != nCTCols)
                            break;
                    }
                }
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
    // Di default tutte le seriali sono DISABILITATE ma EDITABLI
    // Al System_Editor si abilitano i relativi TAB con AND dei due parametri per gestire modelli in cui la Seriale è presente ed utilizzabile
    // ma con parametri FISSI non modificabil dall'utente (es. 1007_04_AE Lever o 1008_03_XX)
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
    tpRec.fastIn = 0;
    tpRec.fastOut = 0;
    tpRec.pwm = 0;
    tpRec.loadCells = 0;
    tpRec.retries = 1;
    tpRec.blacklist = 2;
    tpRec.readPeriod1 = 10;
    tpRec.readPeriod2 = 50;
    tpRec.readPeriod3 = 200;
    tpRec.fastLogPeriod = 1;
    tpRec.slowLogPeriod = 10;
    // Serial 0
    tpRec.ser0_Enabled = false;
    tpRec.ser0_Editable = true;
    tpRec.ser0_BaudRate = 38400;
    tpRec.ser0_DataBits = 8;
    tpRec.ser0_StopBits = 1;
    tpRec.ser0_TimeOut = 50;
    tpRec.ser0_Silence = 2;
    tpRec.ser0_BlockSize = 64;
    // Serial 1
    tpRec.ser1_Enabled = false;
    tpRec.ser1_Editable = true;
    tpRec.ser1_BaudRate = 38400;
    tpRec.ser1_DataBits = 8;
    tpRec.ser1_StopBits = 1;
    tpRec.ser1_TimeOut = 50;
    tpRec.ser1_Silence = 2;
    tpRec.ser1_BlockSize = 64;
    // Serial 2
    tpRec.ser2_Enabled = false;
    tpRec.ser2_Editable = true;
    tpRec.ser2_BaudRate = 38400;
    tpRec.ser2_DataBits = 8;
    tpRec.ser2_StopBits = 1;
    tpRec.ser2_TimeOut = 50;
    tpRec.ser2_Silence = 2;
    tpRec.ser2_BlockSize = 64;
    // Serial 3
    tpRec.ser3_Enabled = true;
    tpRec.ser3_Editable = true;
    tpRec.ser3_BaudRate = 38400;
    tpRec.ser3_DataBits = 8;
    tpRec.ser3_StopBits = 1;
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
    // Audio
    tpRec.audioIF = false;
    // Creazione Lista modelli
    lstTargets.clear();
    for (nModel = 0; nModel < MODEL_TOTALS; nModel++)  {
        tpRec.nModel = nModel;
        lstTargets.append(tpRec);
    }
    // Customizzazione dei modelli
    //00 AnyTPAC = 0
    lstTargets[AnyTPAC].modelName =  QLatin1String(product_name[AnyTPAC]);
    //01 TP1043_01_A
    lstTargets[TP1043_01_A].modelName =  QLatin1String(product_name[TP1043_01_A]);
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
    lstTargets[TP1043_01_B].modelName =  QLatin1String(product_name[TP1043_01_B]);
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
    //03 TP1043_02_A
    lstTargets[TP1043_02_A].modelName =  QLatin1String(product_name[TP1043_02_A]);
    lstTargets[TP1043_02_A].displayWidth =  480;
    lstTargets[TP1043_02_A].displayHeight = 272;
    lstTargets[TP1043_02_A].sdCards = 1;
    lstTargets[TP1043_02_A].digitalIN = 0;
    lstTargets[TP1043_02_A].digitalOUT = 0;
    lstTargets[TP1043_02_A].nEncoders = 0;
    lstTargets[TP1043_02_A].analogIN = 0;
    lstTargets[TP1043_02_A].analogINrowCT = -1;
    lstTargets[TP1043_02_A].analogOUT = 0;
    lstTargets[TP1043_02_A].analogOUTrowCT = -1;
    lstTargets[TP1043_02_A].tAmbient = false;
    lstTargets[TP1043_02_A].rpmPorts = 0;
    lstTargets[TP1043_02_A].ser0_Enabled = true;
    lstTargets[TP1043_02_A].ser1_Enabled = false;
    lstTargets[TP1043_02_A].ser2_Enabled = false;
    lstTargets[TP1043_02_A].ser3_Enabled = false;
    lstTargets[TP1043_02_A].can1_Enabled = false;
    // 04 TP1043_02_B
    lstTargets[TP1043_02_B].modelName =  QLatin1String(product_name[TP1043_02_B]);
    lstTargets[TP1043_02_B].displayWidth =  480;
    lstTargets[TP1043_02_B].displayHeight = 272;
    lstTargets[TP1043_02_B].sdCards = 1;
    lstTargets[TP1043_02_B].digitalIN = 0;
    lstTargets[TP1043_02_B].digitalOUT = 0;
    lstTargets[TP1043_02_B].nEncoders = 0;
    lstTargets[TP1043_02_B].analogIN = 0;
    lstTargets[TP1043_02_B].analogINrowCT = -1;
    lstTargets[TP1043_02_B].analogOUT = 0;
    lstTargets[TP1043_02_B].analogOUTrowCT = -1;
    lstTargets[TP1043_02_B].tAmbient = false;
    lstTargets[TP1043_02_B].rpmPorts = 0;
    lstTargets[TP1043_02_B].ser0_Enabled = false;
    lstTargets[TP1043_02_B].ser1_Enabled = false;
    lstTargets[TP1043_02_B].ser2_Enabled = false;
    lstTargets[TP1043_02_B].ser3_Enabled = false;
    lstTargets[TP1043_02_B].can1_Enabled = true;
    // 05 TP1070_01_A
    lstTargets[TP1070_01_A].modelName =  QLatin1String(product_name[TP1070_01_A]);
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
    lstTargets[TP1070_01_B].modelName =  QLatin1String(product_name[TP1070_01_B]);
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
    lstTargets[TP1070_01_C].modelName =  QLatin1String(product_name[TP1070_01_C]);
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
    // 08 TPAC1005
    lstTargets[TPAC1005].modelName =  QLatin1String(product_name[TPAC1005]);
    lstTargets[TPAC1005].displayWidth =  480;
    lstTargets[TPAC1005].displayHeight = 272;
    lstTargets[TPAC1005].sdCards = 0;
    lstTargets[TPAC1005].digitalIN = 0;
    lstTargets[TPAC1005].digitalOUT = 0;
    lstTargets[TPAC1005].nEncoders = 0;
    lstTargets[TPAC1005].analogIN = 0;
    lstTargets[TPAC1005].analogINrowCT = -1;
    lstTargets[TPAC1005].analogOUT = 0;
    lstTargets[TPAC1005].analogOUTrowCT = -1;
    lstTargets[TPAC1005].tAmbient = false;
    lstTargets[TPAC1005].rpmPorts = 0;
    lstTargets[TPAC1005].ser0_Enabled = true;
    lstTargets[TPAC1005].ser1_Enabled = false;
    lstTargets[TPAC1005].ser2_Enabled = false;
    lstTargets[TPAC1005].ser3_Enabled = false;
    lstTargets[TPAC1005].can1_Enabled = true;
    // 09 TPAC1007_03
    lstTargets[TPAC1007_03].modelName =  QLatin1String(product_name[TPAC1007_03]);
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
    // 10 TPAC1007_04_AA
    lstTargets[TPAC1007_04_AA].modelName =  QLatin1String(product_name[TPAC1007_04_AA]);
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
    // 11 TPAC1007_04_AB
    lstTargets[TPAC1007_04_AB].modelName =  QLatin1String(product_name[TPAC1007_04_AB]);
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
    // 12 TPAC1007_04_AC
    lstTargets[TPAC1007_04_AC].modelName =  QLatin1String(product_name[TPAC1007_04_AC]);
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
    // 13 TPAC1007_04_AD
    lstTargets[TPAC1007_04_AD].modelName =  QLatin1String(product_name[TPAC1007_04_AD]);
    lstTargets[TPAC1007_04_AD].displayWidth =  480;
    lstTargets[TPAC1007_04_AD].displayHeight = 272;
    lstTargets[TPAC1007_04_AD].sdCards = 1;
    lstTargets[TPAC1007_04_AD].digitalIN = 12;
    lstTargets[TPAC1007_04_AD].digitalOUT = 8;
    lstTargets[TPAC1007_04_AD].nEncoders = 1;
    lstTargets[TPAC1007_04_AD].analogIN = 2;
    lstTargets[TPAC1007_04_AD].analogINrowCT = 5328;
    lstTargets[TPAC1007_04_AD].analogOUT = 1;
    lstTargets[TPAC1007_04_AD].analogOUTrowCT = 5344;
    lstTargets[TPAC1007_04_AD].tAmbient = true;
    lstTargets[TPAC1007_04_AD].rpmPorts = 0;
    lstTargets[TPAC1007_04_AD].ser0_Enabled = true;
    lstTargets[TPAC1007_04_AD].ser1_Enabled = false;
    lstTargets[TPAC1007_04_AD].ser2_Enabled = false;
    lstTargets[TPAC1007_04_AD].ser3_Enabled = false;
    lstTargets[TPAC1007_04_AD].can1_Enabled = false;
    // 14 TPAC1007_04_AE (ex TPAC1007_LV)
    lstTargets[TPAC1007_04_AE].modelName =  QLatin1String(product_name[TPAC1007_04_AE]);
    lstTargets[TPAC1007_04_AE].displayWidth =  480;
    lstTargets[TPAC1007_04_AE].displayHeight = 272;
    lstTargets[TPAC1007_04_AE].sdCards = 0;
    lstTargets[TPAC1007_04_AE].digitalIN = 8;
    lstTargets[TPAC1007_04_AE].digitalOUT = 8;
    lstTargets[TPAC1007_04_AE].nEncoders = 1;
    lstTargets[TPAC1007_04_AE].analogIN = 0;
    lstTargets[TPAC1007_04_AE].analogINrowCT = -1;
    lstTargets[TPAC1007_04_AE].analogOUT = 0;
    lstTargets[TPAC1007_04_AE].analogOUTrowCT = -1;
    lstTargets[TPAC1007_04_AE].tAmbient = true;
    lstTargets[TPAC1007_04_AE].rpmPorts = 0;
    lstTargets[TPAC1007_04_AE].ser0_Enabled = true;
    lstTargets[TPAC1007_04_AE].ser1_Enabled = false;
    lstTargets[TPAC1007_04_AE].ser2_Enabled = false;
    lstTargets[TPAC1007_04_AE].ser3_Enabled = true;             // Internal Port
    lstTargets[TPAC1007_04_AE].ser3_Editable = false;
    lstTargets[TPAC1007_04_AE].can1_Enabled = false;
    // 15 TPAC1008_02_AA
    lstTargets[TPAC1008_02_AA].modelName =  QLatin1String(product_name[TPAC1008_02_AA]);
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
    lstTargets[TPAC1008_02_AB].modelName =  QLatin1String(product_name[TPAC1008_02_AB]);
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
    lstTargets[TPAC1008_02_AD].modelName =  QLatin1String(product_name[TPAC1008_02_AD]);
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
    lstTargets[TPAC1008_02_AE].modelName =  QLatin1String(product_name[TPAC1008_02_AE]);
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
    lstTargets[TPAC1008_02_AF].modelName =  QLatin1String(product_name[TPAC1008_02_AF]);
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
    // 20 TPLC050_01_AA
    lstTargets[TPLC050_01_AA].modelName =  QLatin1String(product_name[TPLC050_01_AA]);
    lstTargets[TPLC050_01_AA].displayWidth =  -1;
    lstTargets[TPLC050_01_AA].displayHeight = -1;
    lstTargets[TPLC050_01_AA].sdCards = 0;
    lstTargets[TPLC050_01_AA].digitalIN = 8;
    lstTargets[TPLC050_01_AA].digitalOUT = 8;
    lstTargets[TPLC050_01_AA].nEncoders = 1;
    lstTargets[TPLC050_01_AA].analogIN = 2;
    lstTargets[TPLC050_01_AA].analogINrowCT = 5306;
    lstTargets[TPLC050_01_AA].analogOUT = 0;
    lstTargets[TPLC050_01_AA].tAmbient = true;
    lstTargets[TPLC050_01_AA].rpmPorts = 0;
    lstTargets[TPLC050_01_AA].ser0_Enabled = true;
    lstTargets[TPLC050_01_AA].ser1_Enabled = false;
    lstTargets[TPLC050_01_AA].ser2_Enabled = false;
    lstTargets[TPLC050_01_AA].ser3_Enabled = false;
    lstTargets[TPLC050_01_AA].can1_Enabled = false;
    // 21 TPLC050_01_AB
    lstTargets[TPLC050_01_AB].modelName =  QLatin1String(product_name[TPLC050_01_AB]);
    lstTargets[TPLC050_01_AB].displayWidth =  -1;
    lstTargets[TPLC050_01_AB].displayHeight = -1;
    lstTargets[TPLC050_01_AB].sdCards = 1;
    lstTargets[TPLC050_01_AB].digitalIN = 0;
    lstTargets[TPLC050_01_AB].digitalOUT = 0;
    lstTargets[TPLC050_01_AB].nEncoders = 1;
    lstTargets[TPLC050_01_AB].analogIN = 2;
    lstTargets[TPLC050_01_AB].analogINrowCT = 5328;
    lstTargets[TPLC050_01_AB].analogOUT = 0;
    lstTargets[TPLC050_01_AB].tAmbient = true;
    lstTargets[TPLC050_01_AB].rpmPorts = 0;
    lstTargets[TPLC050_01_AB].ser0_Enabled = true;
    lstTargets[TPLC050_01_AB].ser1_Enabled = false;
    lstTargets[TPLC050_01_AB].ser2_Enabled = false;
    lstTargets[TPLC050_01_AB].ser3_Enabled = false;
    lstTargets[TPLC050_01_AB].can1_Enabled = false;
    // 22 TPLC100_01_AA
    lstTargets[TPLC100_01_AA].modelName =  QLatin1String(product_name[TPLC100_01_AA]);
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
    lstTargets[TPLC100_01_AA].ser0_Enabled = false;
    lstTargets[TPLC100_01_AA].ser1_Enabled = false;
    lstTargets[TPLC100_01_AA].ser2_Enabled = false;
    lstTargets[TPLC100_01_AA].ser3_Enabled = false;
    lstTargets[TPLC100_01_AA].can1_Enabled = true;
    // 23 TPLC100_01_AB
    lstTargets[TPLC100_01_AB].modelName =  QLatin1String(product_name[TPLC100_01_AB]);
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
    lstTargets[TPLC100_01_AB].ser0_Enabled = false;     // Possibile in futuro
    lstTargets[TPLC100_01_AB].ser1_Enabled = false;
    lstTargets[TPLC100_01_AB].ser2_Enabled = false;
    lstTargets[TPLC100_01_AB].ser3_Enabled = true;
    lstTargets[TPLC100_01_AB].can1_Enabled = false;
    // 24 TPAC1008_03_AC
    lstTargets[TPAC1008_03_AC].modelName =  QLatin1String(product_name[TPAC1008_03_AC]);
    lstTargets[TPAC1008_03_AC].displayWidth =  800;
    lstTargets[TPAC1008_03_AC].displayHeight = 480;
    lstTargets[TPAC1008_03_AC].sdCards = 0;
    lstTargets[TPAC1008_03_AC].digitalIN = 24;
    lstTargets[TPAC1008_03_AC].digitalOUT = 16;
    lstTargets[TPAC1008_03_AC].nEncoders = 5;
    lstTargets[TPAC1008_03_AC].analogIN = 4;
    lstTargets[TPAC1008_03_AC].analogINrowCT = 5325;
    lstTargets[TPAC1008_03_AC].analogOUT = 4;
    lstTargets[TPAC1008_03_AC].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_03_AC].tAmbient = true;
    lstTargets[TPAC1008_03_AC].rpmPorts = 1;
    lstTargets[TPAC1008_03_AC].fastIn = 4;
    lstTargets[TPAC1008_03_AC].fastOut = 4;
    lstTargets[TPAC1008_03_AC].pwm = 4;
    lstTargets[TPAC1008_03_AC].loadCells = 3;
    lstTargets[TPAC1008_03_AC].ser0_Enabled = true;         // Internal Port
    lstTargets[TPAC1008_03_AC].ser0_Editable = false;
    lstTargets[TPAC1008_03_AC].ser1_Enabled = false;
    lstTargets[TPAC1008_03_AC].ser2_Enabled = false;
    lstTargets[TPAC1008_03_AC].ser3_Enabled = true;
    lstTargets[TPAC1008_03_AC].can1_Enabled = false;
    // 25 TPAC1008_03_AD
    lstTargets[TPAC1008_03_AD].modelName =  QLatin1String(product_name[TPAC1008_03_AD]);
    lstTargets[TPAC1008_03_AD].displayWidth =  800;
    lstTargets[TPAC1008_03_AD].displayHeight = 480;
    lstTargets[TPAC1008_03_AD].sdCards = 0;
    lstTargets[TPAC1008_03_AD].digitalIN = 24;
    lstTargets[TPAC1008_03_AD].digitalOUT = 16;
    lstTargets[TPAC1008_03_AD].nEncoders = 5;
    lstTargets[TPAC1008_03_AD].analogIN = 4;
    lstTargets[TPAC1008_03_AD].analogINrowCT = 5325;
    lstTargets[TPAC1008_03_AD].analogOUT = 4;
    lstTargets[TPAC1008_03_AD].analogOUTrowCT = 5347;
    lstTargets[TPAC1008_03_AD].tAmbient = true;
    lstTargets[TPAC1008_03_AD].rpmPorts = 1;
    lstTargets[TPAC1008_03_AD].fastIn = 4;
    lstTargets[TPAC1008_03_AD].fastOut = 4;
    lstTargets[TPAC1008_03_AD].pwm = 4;
    lstTargets[TPAC1008_03_AD].loadCells = 3;
    lstTargets[TPAC1008_03_AD].ser0_Enabled = true;         // Internal Port
    lstTargets[TPAC1008_03_AD].ser0_Editable = false;
    lstTargets[TPAC1008_03_AD].ser1_Enabled = false;
    lstTargets[TPAC1008_03_AD].ser2_Enabled = false;
    lstTargets[TPAC1008_03_AD].ser3_Enabled = true;
    lstTargets[TPAC1008_03_AD].can1_Enabled = false;
    // 26 TP1070_02_E
    lstTargets[TP1070_02_E].modelName =  QLatin1String(product_name[TP1070_02_E]);
    lstTargets[TP1070_02_E].displayWidth =  800;
    lstTargets[TP1070_02_E].displayHeight = 480;
    lstTargets[TP1070_02_E].sdCards = 0;
    lstTargets[TP1070_02_E].digitalIN = 0;
    lstTargets[TP1070_02_E].digitalOUT = 0;
    lstTargets[TP1070_02_E].nEncoders = 0;
    lstTargets[TP1070_02_E].analogIN = 0;
    lstTargets[TP1070_02_E].analogINrowCT = -1;
    lstTargets[TP1070_02_E].analogOUT = 0;
    lstTargets[TP1070_02_E].analogOUTrowCT = -1;
    lstTargets[TP1070_02_E].tAmbient = false;
    lstTargets[TP1070_02_E].rpmPorts = 0;
    lstTargets[TP1070_02_E].ser0_Enabled = false;
    lstTargets[TP1070_02_E].ser1_Enabled = false;
    lstTargets[TP1070_02_E].ser2_Enabled = false;
    lstTargets[TP1070_02_E].ser3_Enabled = true;
    lstTargets[TP1070_02_E].can1_Enabled = false;
    // 27 TP1070_02_F
    lstTargets[TP1070_02_F].modelName =  QLatin1String(product_name[TP1070_02_F]);
    lstTargets[TP1070_02_F].displayWidth =  800;
    lstTargets[TP1070_02_F].displayHeight = 480;
    lstTargets[TP1070_02_F].sdCards = 0;
    lstTargets[TP1070_02_F].digitalIN = 0;
    lstTargets[TP1070_02_F].digitalOUT = 0;
    lstTargets[TP1070_02_F].nEncoders = 0;
    lstTargets[TP1070_02_F].analogIN = 0;
    lstTargets[TP1070_02_F].analogINrowCT = -1;
    lstTargets[TP1070_02_F].analogOUT = 0;
    lstTargets[TP1070_02_F].analogOUTrowCT = -1;
    lstTargets[TP1070_02_F].tAmbient = false;
    lstTargets[TP1070_02_F].rpmPorts = 0;
    lstTargets[TP1070_02_F].ser0_Enabled = true;
    lstTargets[TP1070_02_F].ser1_Enabled = false;
    lstTargets[TP1070_02_F].ser2_Enabled = false;
    lstTargets[TP1070_02_F].ser3_Enabled = false;
    lstTargets[TP1070_02_F].can1_Enabled = false;
    // Ciclo di verifica caricamento modelli:
    for (nModel = 0; nModel < MODEL_TOTALS; nModel++)  {
        qDebug() << QString::fromLatin1("Model: %1 - Model Name: %2") .arg(QString::number(nModel)) .arg(lstTargets[nModel].modelName);
    }
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
    if (nModel < 0 || nCur >= lstTargets.count())  {
        nModel = AnyTPAC;
        qDebug() << QLatin1String("searchModelInList: Model Code Forced to AnyTPAC");
    }
    // qDebug() << QString::fromAscii("searchModelInList: Model Code <%1> Model No <%2>") .arg(TargetConfig.modelName) .arg(nModel);
    // Return value
    return nModel;
}
bool ctedit::updateRow(int nRow, bool fMultiEdit)
// Gestisce l'aggiornamento del grid con i valori letti da interfaccia di editing
{
    bool        fRes = true;
    QStringList lstFields;
    int         nErrors = 0;
    bool        fIsSaved = false;
    QBrush      brushInputReg(Qt::SolidPattern);

    if (! isFormEmpty() && isLineModified(nRow))  {
        // Nel caso di mulitselect si sta cercando di scrivere su una riga vuota
        if (fMultiEdit && strlen(lstCTRecords[nRow].Tag) == 0)  {
            qDebug("updateRow(MultiSelect) - Row: [%d] Skipped as not used", nRow + 1);
            goto exitSave;
        }
        qDebug("updateRow() - Updating Row: %d", nRow);
        // Valori da interfaccia a Lista Stringhe
        fRes = iface2values(lstFields, fMultiEdit, nRow);
        // Primo controllo di coerenza sulla riga corrente
        nErrors = checkFormFields(nRow, lstFields, true);
        if (fRes && nErrors == 0)  {
            // Copia l'attuale CT nella lista Undo
            //(Se MultiEdit viene fatto una volta sola PRIMA di salvare tutte le righe modificate)
            if (! fMultiEdit)  {
                lstUndo.append(lstCTRecords);
            }
            qDebug("updateRow() - lstUndo added");
            // Salva Record
            fIsSaved = fieldValues2CTrecList(lstFields, lstCTRecords, nRow);
            // Aggiorna Grid Utente per riga corrente
            if (fIsSaved)  {
                fRes = list2GridRow(ui->tblCT, lstFields, lstHeadLeftCols, nRow);
                // Input Register
                if (lstFields[colInputReg] == szTRUE) {
                    brushInputReg.setColor(Qt::red);
                }
                else  {
                    brushInputReg.setColor(Qt::black);
                }
                setCellForeground(brushInputReg, ui->tblCT->model(), nRow, colRegister);
                m_isCtModified = true;
                m_rebuildDeviceTree = true;
                m_rebuildTimingTree = true;
                // Repaint Colori
                setRowsColor();
            }
        }
        else  {
            fRes = false;
        }
    }
    else  {
        qDebug("updateRow(): Row: [%d] - No Update needed!", nRow + 1);
    }

exitSave:
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
        if ((nPort > 0 && nPort <= nMax_Int16) && (panelConfig.ethPorts > 0)) {
                if (nPort != nPortFTPControl && nPort != nPortSFTP && nPort != nPortTELNET && nPort != nPortHTTP && nPort != nPortVNC)  {
                    fRes = true;
                }
            }
            break;

        case CANOPEN:
            if (nPort >= 0 && nPort <= nMaxCanPorts)  {
                if (nPort == 0)
                    fRes = panelConfig.can0_Enabled;
                else if (nPort == 1)
                    fRes = panelConfig.can1_Enabled;
            }
            break;

        case RTU:
        case MECT_PTC:
        case RTU_SRV:
            if (nPort >= 0 && nPort <= nMaxSerialPorts)  {
                if ((nPort == 0 && panelConfig.ser0_Enabled) ||
                    (nPort == 1 && panelConfig.ser1_Enabled) ||
                    (nPort == 2 && panelConfig.ser2_Enabled) ||
                    (nPort == 3 && panelConfig.ser3_Enabled) )  {
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
            m_szMsg = QString::fromAscii("Crosstable File has unsaved changes: Save?\n%1") .arg(m_szCurrentCTFile);
            if (queryUser(this, szMectTitle, m_szMsg, true))  {
                saveCTFile();
                fRes = true;
            }
        }
        m_isCtModified = false;
        // Configuration file
        if (m_isConfModified)  {
            m_szMsg = QString::fromAscii("Configuration File has unsaved changes: Save?\n%1") .arg(m_szCurrentCTPath + szINIFILE);
            if (queryUser(this, szMectTitle, m_szMsg, true))  {
                mectSet->saveMectSettings();
                fRes = true;
            }
        }
        m_isConfModified = false;
        // Trend File
        if (m_isTrendModified)  {
            m_szMsg = QString::fromAscii("Trend File has unsaved changes: Save?\n%1") .arg(trendEdit->currentTrendFile());
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
bool ctedit::getRowsFromXMLBuffer(QString &szBuffer, QList<QStringList> &lstPastedRecords, QList<int> &lstSourceRows, QList<int> &lstDestRows)
// Funzione per leggere da Buffer Clipboard o da una QString (caricata da file XML) delle righe di CT
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
                        // qDebug() << QString::fromAscii("Number of Rows: %1") .arg(nBufferRows);
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
                        nRow = (fOk && nRow) > 0 ? (nRow -1) : -1;
                    }
                    lstDestRows.append(nRow);
                    // Append Data to Pasting Lists
                    lstPastedRecords.append(lstRecordFields);
                    // qDebug() << QString::fromAscii("Row Variable: %1") .arg(lstRecordFields[colName]);
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
bool ctedit::readModelVars(const QString szModelName, QList<CrossTableRecord> &lstModelVars)
{
    QString     szFileName = szModelName + szXMLExt;
    bool        fRes = false;
    QFile       fileXML(szMODELSPATH + szFileName);
    QString     szXMLBuffer;
    QList<int>  lstSourceRows;
    QList<int>  lstDestRows;

    // Pulizia liste di destinazione
    lstSourceRows.clear();
    lstDestRows.clear();
    lstModelVars.clear();
    // Controllo esistenza file
    if (fileXML.exists())  {
        // Lettura One-Shot del file XML
        fileXML.open(QIODevice::ReadOnly | QIODevice::Text);
        szXMLBuffer = QLatin1String(fileXML.readAll().data());
        // Check Buffer Length
        if (!szXMLBuffer.isEmpty())  {
            // Load Rows from Buffer
            QList<QStringList> lstModelFields;
            fRes = getRowsFromXMLBuffer(szXMLBuffer, lstModelFields, lstSourceRows, lstDestRows);
            // Conversione da Lista Campi a Record CT
            if (fRes)  {
                int nRow = 0;
                for (nRow = 0; nRow < lstModelFields.count(); nRow++)  {
                    CrossTableRecord ctRec;
                    lstModelVars.append(ctRec);
                    freeCTrec(lstModelVars, nRow);
                    fieldValues2CTrecList(lstModelFields[nRow], lstModelVars, nRow);
                    // Aggiorna la larghezza della colonna VarName
                    lstMPNxHeadSizes[colMPNxName];
                    if (lstModelFields[nRow][colName].length() > lstMPNxHeadSizes[colMPNxName])  {
                        lstMPNxHeadSizes[colMPNxName] = lstModelFields[nRow][colName].length();
                    }
                    // Aggiorna la larghezza della colonna commento
                    if (lstModelFields[nRow][colComment].length() > lstMPNxHeadSizes[colMPNxComment])  {
                        lstMPNxHeadSizes[colMPNxComment] = lstModelFields[nRow][colComment].length();
                    }
                }
            }
        }
    }
    return fRes;
}
bool ctedit::checkFreeArea(int nStartRow, int nRows)
// Controlla che l'area di destinazione per inserire variabili sia sufficientemente capiente
{
    int         nCur = 0;
    int         nUsed = 0;
    bool        fRes = false;

    if (nStartRow + nRows > MIN_DIAG - 1)  {
        m_szMsg = QLatin1String("Pasted Rows could overlap to System Area");
        warnUser(this, szMectTitle, m_szMsg);
    }
    else  {
        for (nCur = nStartRow; nCur < nStartRow + nRows; nCur++)  {
            if (lstCTRecords[nCur].UsedEntry)  {
                nUsed++;
            }
        }
        // Almeno riga usata nel mezzo, chiedi conferma
        if (nUsed)  {
            m_szMsg = QString::fromAscii("%1 of destination rows may be used. Paste anyway ?") .arg(nUsed);
            fRes = queryUser(this, szMectTitle, m_szMsg);
        }
        else  {
            fRes = true;
        }
    }
    return fRes;
}

bool ctedit::addModelVars(const QString szModelName, int nRow, int nPort, int nNode)
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
    bool        checkRTUPort = (szModelName != szTPLC050);

    if (fileXML.exists())  {
        fileXML.open(QIODevice::ReadOnly | QIODevice::Text);
        szXMLBuffer = QLatin1String(fileXML.readAll().data());
        // Check Buffer Length
        if (! szXMLBuffer.isEmpty())  {
            // Load Rows from Buffer
            fRes = getRowsFromXMLBuffer(szXMLBuffer, lstModelRows, lstSourceRows, lstDestRows);
            if (fRes)  {
                // Sostituzione di Porta e Nodo con quelli scelti da utente
                QString szPort = QString::number(nPort);
                QString szNode = QString::number(nNode);
                for (nCur = 0; nCur < lstModelRows.count(); nCur++)  {
                    lstModelRows[nCur][colPort] = szPort;
                    lstModelRows[nCur][colNodeID] = szNode;
                }
                // Aggiunta righe in blocco a CT
                nAdded = addRowsToCT(nRow, lstModelRows, lstDestRows, checkRTUPort);
                m_szMsg = QString::fromAscii("Added %1 Rows for Model: %2") .arg(nAdded) .arg(szModelName);
            }
            else {
                m_szMsg = QString::fromAscii("Error reading Variable File for Model: %1") .arg(szModelName);
            }
        }
        else {
            m_szMsg = QString::fromAscii("Empty Variable File for Model: %1") .arg(szModelName);
        }
    }
    else {
        m_szMsg = QString::fromAscii("Variable File Not Found for Model: %1") .arg(szModelName);
    }
    // Notify to user
    displayStatusMessage(m_szMsg);
    // Return Value
    return (nAdded > 0);
}
int ctedit::addRowsToCT(int nRow, QList<QStringList > &lstRecords2Add, QList<int> &lstDestRows, bool checkRTU)
{
    int     nCur = 0;
    int     nPasted = 0;
    int     nDestRow = 0;

    // Append to Undo List
    lstUndo.append(lstCTRecords);
    qDebug() << "addRowsToCT() - lstUndo added";
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
        fieldValues2CTrecList(lstRecords2Add[nPasted], lstCTRecords, nDestRow);
        // Force Block Size to Var Size
        if (lstCTRecords[nDestRow].Protocol ==  PLC)
            lstCTRecords[nDestRow].BlockSize = 1;
        else
            lstCTRecords[nDestRow].BlockSize = varSizeInBlock(lstCTRecords[nDestRow].VarType);
        // Controllo Protocollo / Porta RTU
        int nProtocol = lstCTRecords[nDestRow].Protocol;
        if (nProtocol == RTU || nProtocol == MECT_PTC || nProtocol == RTU_SRV)  {
            // Controllo che la porta RTU selezionata sia disponibile all'utente sul modello
            // Controllo disabilitato per TPLC050
            if (checkRTU)  {
                int nPort = lstCTRecords[nDestRow].Port;
                int nTotal = 0;
                if (! isValidPort(nPort, nProtocol))  {
                    getFirstPortFromProtocol(nProtocol, nPort, nTotal);
                    if (nTotal > 0)  {
                        lstCTRecords[nDestRow].Port = nPort;
                    }
                }
            }
        }
    }
    // Restore Grid Position to First Destination Row && select first row
    ui->tblCT->selectionModel()->clearSelection();
    m_nGridRow = nCur;
    ctable2Grid();
    m_isCtModified = true;
    m_rebuildDeviceTree = true;
    m_rebuildTimingTree = true;
    jumpToGridRow(nCur + 1, true);
    jumpToGridRow(nCur, true);
    // Return value
    return (nPasted);
}

void ctedit::treeItemDoubleClicked(const QModelIndex &index)
// Passaggio da Device Tree a Variabile
{
    QVariant    vRow;
    bool        fOk;
    int         nRow;

    vRow = index.data(Qt::UserRole);
    nRow = vRow.toInt(&fOk);
//    qDebug() << QString::fromAscii("Tree Item Clicked: %1 - %2") .arg(nRow) .arg(fOk);
    if (fOk && nRow >= 0 && nRow < lstCTRecords.count())  {
        // Che fare ??
        ui->tabWidget->setCurrentIndex(TAB_CT);
        jumpToGridRow(nRow, true);
    }
}

void ctedit::on_tblCT_doubleClicked(const QModelIndex &index)
// Passaggio da Variabile a Device Tree
{
    int nRow = index.row();

    if (lstCTRecords[nRow].UsedEntry)  {
        fillDeviceTree(nRow);
        ui->tabWidget->setCurrentIndex(TAB_DEVICES);
    }
}
double  ctedit::serialCharTime_ms(int nBaudRate, int nDataBits, int nStopBits)
// Calcolo del tempo di trasmissione di 1 byte su canale seriale in funzione di Baud Rate, Data Bits, Stop Bits
{
    int nBits = 1 + nDataBits + nStopBits;
    return (((double) nBits / (double) nBaudRate) * 1000.0);
}
double  ctedit::minSerialSilenceTime(int nBaudRate, double dblCharTime_ms)
// Calcolo del minimo tempo di Silence per una linea seriale
{
    return (nBaudRate > 9600 ? 1.75 : 3.5 * dblCharTime_ms);
}

int  ctedit::blockReadTime_ms(int nType, int nRegisters, int nSilence_ms, double dblCharTime_ms)
// Calcolo del Tempo di lettura di un Blocco in funzione del numero di registri e del tempo di lettura di un char
{
    if (nType == BIT)  {
        return readCoils_ms(nRegisters, nSilence_ms, dblCharTime_ms);
    }
    else  {
        return readHoldigRegisters_ms(nRegisters, nSilence_ms, dblCharTime_ms);
    }
}
int ctedit::readHoldigRegisters_ms(int nRegisters, int nSilence_ms, double dblCharTime_ms)
// Calcolo del Tempo di lettura di un blocco di Input/Holding Registers in funzione del # Registri, Silence, charTime
{
    // <SLAVE:1> <FC:1> <ADR:2> <N REG:2> <CRC:2> + [Silence]
    double dblValue = (((double) (1 + 1 + 2 + 2 + 2)) * dblCharTime_ms) + (double) nSilence_ms;             // Modbus Request
    // <SLAVE:1> <FC:1> <DES:1> [2 * n Values] <CRC:2> + [Silence]
    dblValue += (((double) (1 + 1 + 1 + (2 * nRegisters) + 2)) * dblCharTime_ms) + (double) nSilence_ms;    // Modbus Answer
    return (int) ceil(dblValue);
}
int  ctedit::readCoils_ms(int nCoils, int nSilence_ms, double dblCharTime_ms)
// Calcolo del Tempo di lettura di un blocco di Bit Registers (Coils) in funzione del # Bit, Silence, charTime
{
    // <SLAVE:1> <FC:1> <ADR:2> <N COILS:2> <CRC:2> + [Silence]
    double dblValue = (((double) (1 + 1 + 2 + 2 + 2)) * dblCharTime_ms) + (double) nSilence_ms;             // Modbus Request
    // <SLAVE:1> <FC:1> <DES:1> [2 * n Values] <CRC:2> + [Silence]
    dblValue += (((double) (1 + 1 + 1 + ceil((double) nCoils / 8.0) + 2)) * dblCharTime_ms) + (double) nSilence_ms;    // Modbus Answer
    return (int) ceil(dblValue);
}
bool ctedit::isSilenceOk(int nSilence_ms, int nBaudRate, double dblCharTime_ms)
// Verifica che il tempo di silence specificato sia adeguato al BaudRate corrente
{
    return ((double) nSilence_ms >= minSerialSilenceTime(nBaudRate, dblCharTime_ms));
}

bool ctedit::checkServersDevicesAndNodes()
{
    int         nCur = 0;
    int         nPriority = 0;
    bool        fRes = true;
    int         nUsedVars = 0;
    int         nDiag = 0;
    QString     szDiagName;


    // Rinumerazione d'ufficio dei blocchi, poi utilizzato per il ricalcolo dei Server-Nodi...
    // TO BE CHECKED....
//    if (! riassegnaBlocchi())  {
//        m_szMsg = QLatin1String("Found Errors in Reassigning Blocks");
//        warnUser(this, szMectTitle, m_szMsg);
//    }
    // Pulizia Strutture Dati
    theServersNumber = 0;
    theDevicesNumber = 0;
    theTcpDevicesNumber = 0;
    theNodesNumber = 0;
    thePlcVarsNumber = 0;
    theBlocksNumber = 0;
    // Clean Servers
    for (nCur = 0; nCur < nMAX_SERVERS; nCur++)  {
        theServers[nCur].nProtocol = -1;
        theServers[nCur].IPAddress = 0;
        theServers[nCur].szIpAddress.clear();
        theServers[nCur].nPort = 0;
        theServers[nCur].NodeId = 0xffff;
        theServers[nCur].szServerName.clear();
        theServers[nCur].nVars = 0;
    }
    // Clean Devices
    for (nCur = 0; nCur < nMAX_DEVICES; nCur++)  {
        theDevices[nCur].nServer = 0;
        theDevices[nCur].nProtocol = -1;
        theDevices[nCur].szIpAddress.clear();
        theDevices[nCur].nPort = 0;
        theDevices[nCur].nMaxBlockSize = 0;
        theDevices[nCur].nBaudRate = 0;
        theDevices[nCur].nDataBits = 0;
        theDevices[nCur].nStopBits = 0;
        theDevices[nCur].nSilence = 0;
        theDevices[nCur].nTimeOut = 0;
        theDevices[nCur].dCharTime = 0.0;
        theDevices[nCur].dMinSilence = 0.0;
        theDevices[nCur].nVars = 0;
        for (nPriority = 0; nPriority < nNumPriority; nPriority++)  {
            theDevices[nCur].nDeviceReadTime[nPriority] = 0;
        }
        theDevices[nCur].szDeviceName.clear();
        theDevices[nCur].diagnosticAddr = 0;
        theDevices[nCur].diagnosticVarName.clear();
    }
    // Clean Nodes
    for (nCur = 0; nCur < nMAX_NODES; nCur++)  {
        theNodes[nCur].nDevice = 0xffff;
        theNodes[nCur].nNodeId = 0xffff;
        theNodes[nCur].szNodeName.clear();
        theNodes[nCur].nVars = 0;
        for (nPriority = 0; nPriority < nNumPriority; nPriority++)  {
            theNodes[nCur].nNodeReadTime[nPriority] = 0;
        }
        theNodes[nCur].diagnosticAddr = 0;
        theNodes[nCur].diagnosticVarName.clear();
    }
    // Clean Blocks
    for (nCur = 0; nCur < nMAX_BLOCKS; nCur++)  {
        theBlocks[nCur].nBlockId = 0;
        theBlocks[nCur].nPriority = 0;
        theBlocks[nCur].nDevice = 0xffff;
        theBlocks[nCur].nNode = 0xffff;
        theBlocks[nCur].nBlockSize = 0;
        theBlocks[nCur].nProtocol = 0;
        theBlocks[nCur].nRegisters = 0;
        theBlocks[nCur].nByteSize = 0;
        theBlocks[nCur].nReadTime_ms = 0;
    }
    // Clean Dev-Node Info in Variables
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        lstCTRecords[nCur].nDevice = 0xffff;
        lstCTRecords[nCur].nNode = 0xffff;
        lstCTRecords[nCur].nBlock = 0xffff;
    }
    // Spazzolamento Elementi CT per creazione Liste
    int     base = 1;
    int     block = 0;
    int     nSer = 0;
    int     nDev = 0;
    int     nNod = 0;
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        // Block Address
        if (lstCTRecords[nCur].Block != block)  {
            base = nCur;
            block = lstCTRecords[nCur].Block;
        }
        // Priority con controllo di Boundary per Array di Read Time
        nPriority = (lstCTRecords[nCur].Enable >= 0 && lstCTRecords[nCur].Enable < nNumPriority) ? lstCTRecords[nCur].Enable : 0;
        lstCTRecords[nCur].BlockBase = base;
        // Consideriamo solo gli elementi abilitati
        if (lstCTRecords[nCur].UsedEntry > 0 && lstCTRecords[nCur].Enable > 0)  {
            nUsedVars++;
            // Prima verifica per la gestione dei Server
            switch (lstCTRecords[nCur].Protocol) {
            case PLC:
                thePlcVarsNumber++;
                break;
            case RTU:
            case TCP:
            case TCPRTU:
            case CANOPEN:
            case MECT_PTC:
                // nothing to do for server
                break;
            case RTU_SRV:
                // add unique variable's server
                for (nSer = 0; nSer < theServersNumber; nSer++) {
                    if (theServers[nSer].nProtocol == RTU_SRV && lstCTRecords[nCur].Port == theServers[nSer].nPort) {
                        // already present
                        if (theServers[nSer].IPAddress != lstCTRecords[nCur].IPAddress) {
                            char str1[MAX_IPADDR_LEN];
                            char str2[MAX_IPADDR_LEN];
                            ipaddr2str(theServers[nSer].IPAddress, str1);
                            ipaddr2str(lstCTRecords[nCur].IPAddress, str2);
                            qDebug() << QString::fromAscii("checkServersDevicesAndNodes(): WARNING in variable %2 wrong 'IP Address' %3 (should be %4)") .arg(nCur) .arg(QLatin1String(str2)) .arg(QLatin1String(str1));
                        }
                        if (theServers[nSer].NodeId != lstCTRecords[nCur].NodeId) {
                            qDebug() << QString::fromAscii("checkServersDevicesAndNodes(): WARNING in variable %2 wrong 'Node ID' %3 (should be %4)") .arg(nCur) .arg(lstCTRecords[nCur].NodeId) .arg(theServers[nSer].NodeId);
                        }
                        break;
                    }
                }
                goto add_server;
                // no break;
            case TCP_SRV:
            case TCPRTU_SRV:
                // add unique variable's server
                for (nSer = 0; nSer < theServersNumber; nSer++) {
                    if (lstCTRecords[nCur].Protocol == theServers[nSer].nProtocol) {
                        // already present
                        if (theServers[nSer].IPAddress != lstCTRecords[nCur].IPAddress) {
                            char str1[MAX_IPADDR_LEN];
                            char str2[MAX_IPADDR_LEN];
                            ipaddr2str(theServers[nSer].IPAddress, str1);
                            ipaddr2str(lstCTRecords[nCur].IPAddress, str2);
                            qDebug() << QString::fromAscii("checkServersDevicesAndNodes(): WARNING in variable %2 wrong 'IP Address' %3 (should be %4)") .arg(nCur) .arg(QLatin1String(str2)) .arg(QLatin1String(str1));
                        }
                        if (theServers[nSer].nPort != lstCTRecords[nCur].Port) {
                            qDebug() << QString::fromAscii("checkServersDevicesAndNodes(): WARNING in variable %2 wrong 'Port' %3 (should be %4)") .arg(nCur) .arg(lstCTRecords[nCur].Port) .arg(theServers[nSer].nPort);
                        }
                        if (theServers[nSer].NodeId != lstCTRecords[nCur].NodeId) {
                            qDebug() << QString::fromAscii("checkServersDevicesAndNodes(): WARNING in variable %2 wrong 'Node ID' %3 (should be %4)") .arg(nCur) .arg(lstCTRecords[nCur].NodeId) .arg(theServers[nSer].NodeId);
                        }
                        break;
                    }
                }
            add_server:
                if (nSer < theServersNumber) {
                    theServers[nSer].nVars++;
                    // ok already present
                } else if (theServersNumber >= nMAX_SERVERS) {
                    qDebug() << QLatin1String("checkServersDevicesAndNodes(): ERROR: too many Servers");
                    fRes = false;
                } else {
                    // new server entry
                    nSer = theServersNumber++;
                    theServers[nSer].nProtocol = lstCTRecords[nCur].Protocol;
                    theServers[nSer].IPAddress = lstCTRecords[nCur].IPAddress;
                    theServers[nSer].szIpAddress.clear();
                    theServers[nSer].nPort = lstCTRecords[nCur].Port;
                    if (theServers[nSer].nProtocol == TCP_SRV || theServers[nSer].nProtocol == TCPRTU_SRV)  {
                        char str1[MAX_IPADDR_LEN];
                        ipaddr2str(theServers[nSer].IPAddress, str1);
                        theServers[nSer].szIpAddress = QLatin1String(str1);
                    }
                    theServers[nSer].nVars++;
                    theServers[nSer].NodeId = lstCTRecords[nCur].NodeId;
                    theServers[nSer].szServerName = QString::fromAscii("srv[%1]%2_%3_%4") .arg(nSer) .arg(lstProtocol[theServers[nSer].nProtocol]) .arg(theServers[nSer].szIpAddress) .arg(theServers[nSer].nPort);
                }

                break;
            default:
                break;
            }       // switch on Protocol 1
            // Seconda verifica per la gestione dei Device / Nodi
            // client variables =---> link to the server and add unique devices and nodes
            nDev = 0xffff;
            nNod = 0xffff;
            switch (lstCTRecords[nCur].Protocol) {
            case PLC:
                // no plc client
                lstCTRecords[nCur].nDevice = 0xffff;
                lstCTRecords[nCur].nNode = 0xffff;
                break;
            case RTU:
            case TCP:
            case TCPRTU:
            case CANOPEN:
            case MECT_PTC:
                // add unique variable's device (Protocol, IPAddress, Port, --)
                for (nDev = 0; nDev < theDevicesNumber; nDev++) {
                    if (lstCTRecords[nCur].Protocol == theDevices[nDev].nProtocol
                     && lstCTRecords[nCur].IPAddress == theDevices[nDev].IPAddress
                     && lstCTRecords[nCur].Port == theDevices[nDev].nPort) {
                        // already present
                        break;
                    }
                }
                goto add_device;
                // no break
            case RTU_SRV:
                // add unique variable's device (Protocol, --, Port, --)
                for (nDev = 0; nDev < theDevicesNumber; nDev++) {
                    if (lstCTRecords[nCur].Protocol == theDevices[nDev].nProtocol
                     && lstCTRecords[nCur].Port == theDevices[nDev].nPort) {
                        // already present
                        break;
                    }
                }
                goto add_device;
                // no break
            case TCP_SRV:
            case TCPRTU_SRV:
                // add unique variable's device (Protocol, --, --, --)
                for (nDev = 0; nDev < theDevicesNumber; nDev++) {
                    if (lstCTRecords[nCur].Protocol == theDevices[nDev].nProtocol) {
                        // already present
                        break;
                    }
                }
            add_device:
                if (nDev < theDevicesNumber) {
                    // Device Present, add Variable 2 device
                    lstCTRecords[nCur].nDevice = nDev;  // found
                    theDevices[nDev].nVars ++;       // this one, also Htype
                } else if (theDevicesNumber >= nMAX_DEVICES) {
                    // Too many Devices
                    lstCTRecords[nCur].nDevice = 0xffff; // FIXME: error
                    qDebug() << QLatin1String("checkServersDevicesAndNodes(): ERROR: too many Servers");
                    fRes = false;
                } else {
                    QString szDeviceName;
                    QString szIpAddr;
                    // new device entry
                    lstCTRecords[nCur].nDevice = theDevicesNumber;
                    nDev = theDevicesNumber++;
                    theDevices[nDev].nProtocol = lstCTRecords[nCur].Protocol;
                    theDevices[nDev].IPAddress = lstCTRecords[nCur].IPAddress;
                    theDevices[nDev].nPort = lstCTRecords[nCur].Port;
                    theDevices[nDev].nVars++;
                    char str1[MAX_IPADDR_LEN];
                    ipaddr2str(lstCTRecords[nCur].IPAddress, str1);
                    szIpAddr = QLatin1String(str1);
                    szDeviceName = lstProtocol[theDevices[nDev].nProtocol];
                    // Check Device Protocol
                    switch (theDevices[nDev].nProtocol) {
                    case PLC:
                        // FIXME: assert
                        theDevices[nDev].nMaxBlockSize = MAXBLOCKSIZE;
                        break;
                    case RTU:
                        szDeviceName.append(QString::number(theDevices[nDev].nPort));
                        // Aggiunge al Nome Device anche le info Baud Rate
                        szDeviceName.append(QString::fromAscii("\t%1") .arg(getSerialPortSpeed(theDevices[nDev].nPort)));
                        if (theDevices[nDev].nPort == 0)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser0_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser0_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser0_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser0_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser0_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser0_StopBits;
                        }
                        else if (theDevices[nDev].nPort == 1)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser1_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser1_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser1_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser1_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser1_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser1_StopBits;
                        }
                        else if (theDevices[nDev].nPort == 2)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser2_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser2_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser2_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser2_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser2_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser2_StopBits;
                        }
                        else if (theDevices[nDev].nPort == 3)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser3_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser3_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser3_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser3_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser3_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser3_StopBits;
                        }
                        break;
                    case MECT_PTC:
                        theDevices[nDev].nMaxBlockSize = MAXBLOCKSIZE;
                        theDevices[nDev].nSilence = 0;
                        theDevices[nDev].nTimeOut = 0;
                        break;
                    case TCP:
                        szDeviceName.append(QString::number(theTcpDevicesNumber));
                        ++theTcpDevicesNumber;
                        theDevices[nDev].szIpAddress = szIpAddr;
                        theDevices[nDev].nMaxBlockSize = panelConfig.tcp_BlockSize;
                        theDevices[nDev].nSilence = panelConfig.tcp_Silence;
                        theDevices[nDev].nTimeOut = panelConfig.tcp_TimeOut;
                        break;
                    case TCPRTU:
                        theDevices[nDev].szIpAddress = szIpAddr;
                        theDevices[nDev].nMaxBlockSize = panelConfig.tcp_BlockSize;
                        theDevices[nDev].nSilence = panelConfig.tcp_Silence;
                        theDevices[nDev].nTimeOut = panelConfig.tcp_TimeOut;
                        break;
                    case CANOPEN:
                        szDeviceName.append(QString::number(theDevices[nDev].nPort));
                        if (theDevices[nDev].nPort == 0)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.can0_BlockSize;
                            theDevices[nDev].nSilence = -1;
                            theDevices[nDev].nTimeOut = -1;
                            theDevices[nDev].nBaudRate = panelConfig.can0_BaudRate;
                        }
                        else if (theDevices[nDev].nPort == 1)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.can1_BlockSize;
                            theDevices[nDev].nSilence = -1;
                            theDevices[nDev].nTimeOut = -1;
                            theDevices[nDev].nBaudRate = panelConfig.can1_BaudRate;
                        }
                        break;
                    case RTU_SRV:
                        szDeviceName = QLatin1String("RTU");
                        szDeviceName.append(QString::number(theDevices[nDev].nPort));
                        // Aggiunge al Nome Device anche le info Baud Rate
                        szDeviceName.append(QString::fromAscii("\t%1") .arg(getSerialPortSpeed(theDevices[nDev].nPort)));
                        theDevices[nDev].nServer = nSer; // searched before
                        if (theDevices[nDev].nPort == 0)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser0_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser0_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser0_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser0_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser0_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser0_StopBits;
                        }
                        else if (theDevices[nDev].nPort == 1)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser1_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser1_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser1_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser1_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser1_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser1_StopBits;
                        }
                        else if (theDevices[nDev].nPort == 2)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser2_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser2_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser2_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser2_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser2_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser2_StopBits;
                        }
                        else if (theDevices[nDev].nPort == 3)  {
                            theDevices[nDev].nMaxBlockSize = panelConfig.ser3_BlockSize;
                            theDevices[nDev].nSilence = panelConfig.ser3_Silence;
                            theDevices[nDev].nTimeOut = panelConfig.ser3_TimeOut;
                            theDevices[nDev].nBaudRate = panelConfig.ser3_BaudRate;
                            theDevices[nDev].nDataBits = panelConfig.ser3_DataBits;
                            theDevices[nDev].nStopBits = panelConfig.ser3_StopBits;
                        }
                        break;
                    case TCP_SRV:
                    case TCPRTU_SRV:
                        szDeviceName = QLatin1String("TCPS");
                        theDevices[nDev].szIpAddress = szIpAddr;
                        theDevices[nDev].nServer = nSer; // searched before
                        theDevices[nDev].nMaxBlockSize = panelConfig.tcp_BlockSize;
                        theDevices[nDev].nSilence = panelConfig.tcp_Silence;
                        theDevices[nDev].nTimeOut = panelConfig.tcp_TimeOut;
                        break;
                    default:
                        ;
                    }
                    theDevices[nDev].szDeviceName = szDeviceName;
                    // Ricerca Variabile Diagnostica
                    szDiagName = szDeviceName;
                    szDiagName.append(QLatin1String("_STATUS"));
                    nDiag = varName2Row(szDiagName, lstCTRecords);
                    if (nDiag >= 0)  {
                        theDevices[nDev].diagnosticAddr = nDiag + 1;
                        theDevices[nDev].diagnosticVarName = szDiagName;
                    }
                    // Calcolo del Char Time in ms
                    if (theDevices[nDev].nProtocol == RTU || theDevices[nDev].nProtocol == RTU_SRV)  {
                        theDevices[nDev].dCharTime = serialCharTime_ms(theDevices[nDev].nBaudRate, theDevices[nDev].nDataBits, theDevices[nDev].nStopBits);
                        theDevices[nDev].dMinSilence = minSerialSilenceTime(theDevices[nDev].nBaudRate, theDevices[nDev].dCharTime);
                    }
                    // Check Block Size
                    if (nCur == base && lstCTRecords[nCur].BlockSize > theDevices[nDev].nMaxBlockSize) {
                        qDebug() << QString::fromAscii("checkServersDevicesAndNodes(): warning: variable %1 block %2 size %3, exceeding max_block_size %4 (%5)")
                                    .arg(nCur) .arg(block) .arg(lstCTRecords[nCur].BlockSize) .arg(theDevices[nDev].nMaxBlockSize) .arg(theDevices[nDev].szDeviceName);
                    }
                }
                // add unique variable's node
                for (nNod = 0; nNod < theNodesNumber; ++nNod) {
                     if (lstCTRecords[nCur].nDevice == theNodes[nNod].nDevice) {
                         if (lstCTRecords[nCur].Protocol == RTU_SRV || lstCTRecords[nCur].Protocol == TCP_SRV  || lstCTRecords[nCur].Protocol == TCPRTU_SRV) {
                             // already present (any NodeId)
                             break;
                         } else if (lstCTRecords[nCur].NodeId == theNodes[nNod].nNodeId) {
                             // already present
                             break;
                         }
                    }
                }
//            add_node:
                if (nNod < theNodesNumber) {
                    lstCTRecords[nCur].nNode = nNod; // found
                } else if (theNodesNumber >= nMAX_NODES) {
                    lstCTRecords[nCur].nNode = 0xffff;
                    qDebug() << QLatin1String("checkServersDevicesAndNodes(): ERROR: too many Nodes");
                    fRes = false;
                } else {
                    // new node entry
                    lstCTRecords[nCur].nNode = theNodesNumber++;
                    theNodes[nNod].nDevice = lstCTRecords[nCur].nDevice;
                    theNodes[nNod].nNodeId = lstCTRecords[nCur].NodeId;
                    // Ricerca Variabile Diag
                    szDiagName = QLatin1String("NODE_") + int2PaddedString(nNod+1, 2);
                    szDiagName.append(QLatin1String("_STATUS"));
                    nDiag = varName2Row(szDiagName, lstCTRecords);
                    if (nDiag >= 0)  {
                        theNodes[nNod].diagnosticAddr = nDiag + 1;
                        theNodes[nNod].diagnosticVarName = szDiagName;
                    }
                    theNodes[nNod].szNodeName = QString::fromAscii("%1 = %2 (%3)" ) .arg(lstHeadCols[colNodeID]) .arg(lstCTRecords[nCur].NodeId) .arg(szDiagName);
                    // Ricerca la riga corrente nell'elenco dei marcatori MPNx
                    int nPosMPNX = -1;
                    if (lstMPNC.count() > 0)  {
                        nPosMPNX = lstMPNC.indexOf(nCur);
                        if (nPosMPNX >= 0)  {
                            theNodes[nNod].szNodeName.append(QString::fromAscii("  %1 (%2)") .arg(szMPNC006) .arg(nPosMPNX + 1));
                        }
                    }
                    if (lstMPNE.count() > 0)  {
                        nPosMPNX = lstMPNE.indexOf(nCur);
                        if (nPosMPNX >= 0)  {
                            theNodes[nNod].szNodeName.append(QString::fromAscii("  %1 (%2)") .arg(szMPNE1001) .arg(nPosMPNX + 1));
                        }
                    }
                }
                theNodes[nNod].nVars++;
                break;
            default:
                break;
            }           // switch on Protocol 2 to Create Devices / Nodes
            // Ricerca del Blocco della variabile in lista blocchi
            if (theBlocksNumber == 0 || (theBlocksNumber > 0 && theBlocks[theBlocksNumber -1].nBlockId != block))  {
                theBlocks[theBlocksNumber].nBlockId = block;
                theBlocks[theBlocksNumber].nPriority = nPriority;
                theBlocks[theBlocksNumber].nBlockSize = lstCTRecords[nCur].BlockSize;
                theBlocks[theBlocksNumber].nProtocol = lstCTRecords[nCur].Protocol;
                theBlocks[theBlocksNumber].nDevice = nDev;
                theBlocks[theBlocksNumber].nNode = nNod;
                theBlocksNumber ++;
            }
            // Incrementing Byte Size of Block
            if (! isBitField(lstCTRecords[nCur].VarType) ||  (! isSameBitField(nCur))) {
                theBlocks[theBlocksNumber - 1].nRegisters += varSizeInBlock(lstCTRecords[nCur].VarType);
                theBlocks[theBlocksNumber - 1].nByteSize += (2 * varSizeInBlock(lstCTRecords[nCur].VarType));
            }
            // Updating Block ID for Variable
            lstCTRecords[nCur].nBlock = theBlocksNumber - 1;
        }   // Used Entry
    }       // Variables Cycle
    //---------------------------
    // Blocks Cycle for Reading times
    //---------------------------
    for (nCur = 0; nCur < theBlocksNumber; nCur++)  {
        nPriority = theBlocks[nCur].nPriority;
        // Solo per Devices Seriali aggiorna il tempo di lettura blocco sui Device e Nodi di appartenenza
        if (theBlocks[nCur].nProtocol == RTU || theBlocks[nCur].nProtocol == RTU_SRV)  {
            nDev = theBlocks[nCur].nDevice;
            // Calcolo del tempo di lettura del Blocco
            if (nDev != 0xffff)  {
                // Incremento del tempo di lettura del Device
                if (theDevices[nDev].dCharTime != 0.0)  {
                    theBlocks[nCur].nReadTime_ms = blockReadTime_ms(lstCTRecords[theBlocks[nCur].nBlockId - 1].VarType, theBlocks[nCur].nRegisters, theDevices[nDev].nSilence, theDevices[nDev].dCharTime);
                    theDevices[nDev].nDeviceReadTime[nPriority] +=  theBlocks[nCur].nReadTime_ms;
                    // Incremento del tempo di lettura del Nodo (se esiste)
                    nNod = theBlocks[nCur].nNode;
                    if (nNod != 0xffff)  {
                        theNodes[nNod].nNodeReadTime[nPriority] += theBlocks[nCur].nReadTime_ms;
                    }
                }
            }
        }
    }
//    // Debug Dump
//    QString szRow;
//    qDebug() << QString::fromAscii("Variables: %1") .arg(nUsedVars);
//    qDebug() << QString::fromAscii("Servers: %1") .arg(theServersNumber);
//    for (nCur = 0; nCur < theServersNumber; nCur++)  {
//        szRow = QString::fromAscii("\t%1 - Server Name: %2") .arg(nCur+1) .arg(theServers[nCur].szServerName);
//        szRow.append(QString::fromAscii("\tProtocol: %1") .arg(lstProtocol[theServers[nCur].nProtocol]));
//        szRow.append(QString::fromAscii("\tIp Adr: %1") .arg(theServers[nCur].IPAddress));
//        szRow.append(QString::fromAscii("\tIp Str: %1") .arg(theServers[nCur].szIpAddress));
//        szRow.append(QString::fromAscii("\tPort: %1") .arg(theServers[nCur].nPort));
//        szRow.append(QString::fromAscii("\tNode Id: %1") .arg(theServers[nCur].NodeId));
//        szRow.append(QString::fromAscii("\t# Vars: %1") .arg(theServers[nCur].nVars));
//        qDebug() << szRow;
//    }
//    // Devices List
//    qDebug() << QString::fromAscii("Devices: %1") .arg(theDevicesNumber);
//    for (nCur = 0; nCur < theDevicesNumber; nCur++)  {
//        szRow = QString::fromAscii("\t%1 - Device Name: %2") .arg(nCur+1, 4, 10) .arg(theDevices[nCur].szDeviceName);
//        szRow.append(QString::fromAscii("\tServer Id: %1") .arg(theDevices[nCur].nServer));
//        szRow.append(QString::fromAscii("\tProtocol: %1") .arg(lstProtocol[theDevices[nCur].nProtocol]));
//        szRow.append(QString::fromAscii("\tIp Str: %1") .arg(theDevices[nCur].szIpAddress));
//        szRow.append(QString::fromAscii("\tPort: %1") .arg(theDevices[nCur].nPort));
//        szRow.append(QString::fromAscii("\tBaud Rate: %1") .arg(theDevices[nCur].nBaudRate, 4, 10));
//        szRow.append(QString::fromAscii("\tData Bits: %1") .arg(theDevices[nCur].nDataBits, 2, 10));
//        szRow.append(QString::fromAscii("\tStop Bits: %1") .arg(theDevices[nCur].nStopBits, 2, 10));
//        szRow.append(QString::fromAscii("\tChar Time: %1 ms") .arg(theDevices[nCur].dCharTime, 8, cFloat, 4));
//        szRow.append(QString::fromAscii("\tMin Silence Time: %1 ms") .arg(theDevices[nCur].dMinSilence, 8, cFloat, 4));
//        szRow.append(QString::fromAscii("\tBlock Size: %1") .arg(theDevices[nCur].nMaxBlockSize, 4, 10));
//        szRow.append(QString::fromAscii("\tVars: %1") .arg(theDevices[nCur].nVars, 4, 10));
//        szRow.append(QString::fromAscii("\tDiag Adr: %1") .arg(theDevices[nCur].diagnosticAddr, 6, 10));
//        szRow.append(QString::fromAscii("\tRead Time ms: P0=%1 P1=%2 P2=%3 P3=%4") .arg(theDevices[nCur].nDeviceReadTime[0],4,10) .arg(theDevices[nCur].nDeviceReadTime[1],4,10)
//                .arg(theDevices[nCur].nDeviceReadTime[2],4,10) .arg(theDevices[nCur].nDeviceReadTime[3],4,10));
//        qDebug() << szRow;
//    }
//    qDebug() << QString::fromAscii("Nodes: %1") .arg(theNodesNumber);
//    for (nCur = 0; nCur < theNodesNumber; nCur++)  {
//        szRow = QString::fromAscii("\t%1 - Node Name: %2") .arg(nCur+1, 4, 10) .arg(theNodes[nCur].szNodeName);
//        szRow.append(QString::fromAscii("\tDevice Id: %1 - Node Id: %2") .arg(theNodes[nCur].nDevice,4,10).arg(theNodes[nCur].nNodeId,4,10));
//        szRow.append(QString::fromAscii("\t# Vars: %1") .arg(theNodes[nCur].nVars,4,10));
//        szRow.append(QString::fromAscii("\tDiag Adr: %1") .arg(theNodes[nCur].diagnosticAddr,6,10));
//        szRow.append(QString::fromAscii("\tRead Time ms: P0=%1 P1=%2 P2=%3 P3=%4") .arg(theNodes[nCur].nNodeReadTime[0],4,10) .arg(theNodes[nCur].nNodeReadTime[1],4,10)
//                .arg(theNodes[nCur].nNodeReadTime[2],4,10) .arg(theNodes[nCur].nNodeReadTime[3],4,10));
//        qDebug() << szRow;
//    }
//    qDebug() << QString::fromAscii("Blocks: %1") .arg(theBlocksNumber);
//    for (nCur = 0; nCur < theBlocksNumber; nCur++)  {
//        szRow = QString::fromAscii("\t%1 - Block Id: %2") .arg(nCur+1, 4, 10) .arg(theBlocks[nCur].nBlockId, 6, 10);
//        szRow.append(QString::fromAscii("\tProtocol: %1") .arg(lstProtocol[theBlocks[nCur].nProtocol]));
//        szRow.append(QString::fromAscii("\tBlock Size: %1") .arg(theBlocks[nCur].nBlockSize, 6, 10));
//        szRow.append(QString::fromAscii("\tNode: %1 - Device: %2") .arg(theBlocks[nCur].nNode, 4, 10) .arg(theBlocks[nCur].nDevice, 4, 10));
//        szRow.append(QString::fromAscii("\t#Registers: %1") .arg(theBlocks[nCur].nRegisters, 6, 10));
//        szRow.append(QString::fromAscii("\tByte Size: %1") .arg(theBlocks[nCur].nByteSize, 6, 10));
//        szRow.append(QString::fromAscii("\tPriority: %1 - Read Time: %2 ms") .arg(theBlocks[nCur].nPriority, 4, 10) .arg(theBlocks[nCur].nReadTime_ms, 6, 10));
//        qDebug() << szRow;
//    }
    return fRes;
}

QTreeWidgetItem *ctedit::addItem2Tree(QTreeWidgetItem *tParent, int nRole, const QString &szName, const QString &szInfo, const QString &szTimings, const QString &szToolTip)
// Aggiunta di un Item all'albero
{
    QTreeWidgetItem *tItem = new QTreeWidgetItem(tParent, nRole);
    // Adding Columns to Variable Item
    tItem->setText(colTreeName, szName);
    tItem->setText(colTreeInfo, szInfo);
    tItem->setText(colTreeTimings, szTimings);
    tItem->setToolTip(colTreeName, szToolTip);
    tItem->setToolTip(colTreeInfo, szToolTip);
    // Return Item Value
    return tItem;
}

QTreeWidgetItem *ctedit::addVariable2Tree(QTreeWidgetItem *tParent, int nRow, int nLevel)
// Aggiunge la variabile della riga nRow agganciandola al nodo tParent
{
    QTreeWidgetItem *tVariable;
    QString         szInfo(szEMPTY);
    QString         szTimings(szEMPTY);                         // Timing della variabile non riempito
    QString         szToolTip(szEMPTY);
    QColor          cSfondo = colorNonRetentive[0];
    QStringList     lstVarFields;

    // Retrieve Var Fields
    // qDebug() << QString::fromAscii("addVariable2Tree(): Processing Row: %1") .arg(nRow);
    recCT2FieldsValues(lstCTRecords, lstVarFields, nRow);

    // Tool Tip
    szToolTip.clear();
    szToolTip.append(QString::fromAscii("Row:\t%1\n") .arg(nRow + 1, nRowColWidth, 10));
    if (lstCTRecords[nRow].Protocol != PLC)  {
        szToolTip.append(QString::fromAscii("Device:\t\t%1\n") .arg(theDevices[lstCTRecords[nRow].nDevice].szDeviceName));
        szToolTip.append(QString::fromAscii("%1:\t\t%2\n") .arg(lstHeadCols[colNodeID]) .arg(theNodes[lstCTRecords[nRow].nNode].szNodeName));
    }
    // Colonna Info Variable
    szInfo.clear();
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colPriority]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colUpdate]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colType]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colDecimal]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colProtocol]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colPort]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colNodeID]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colRegister]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colBehavior]));
    szInfo.append(QString::fromAscii("%1\t") .arg(lstVarFields[colComment]));
    // Colonnna Timings Variabile NON RIEMPITA
    // Adding Variable Item to Tree
    tVariable  = addItem2Tree(tParent, nLevel, lstVarFields[colName], szInfo, szTimings, szToolTip);
    // Numero riga per Double Click
    QVariant vRow = nRow;
    tVariable->setData(colTreeName, Qt::UserRole, vRow);
    tVariable->setData(colTreeInfo, Qt::UserRole, vRow);
    // BackGround Color
    cSfondo = colorNonRetentive[0];
    // Impostazione del Backgound color in funzione della zona
    if (nRow >= 0 && nRow < MAX_RETENTIVE)  {
        cSfondo = colorRetentive[0];
    }
    else if (nRow >= MIN_NONRETENTIVE - 1 && nRow <= MAX_NONRETENTIVE -1) {
        cSfondo = colorNonRetentive[0];
    }
    else if (nRow >= MIN_DIAG - 1)  {
        cSfondo = colorSystem[0];
    }
    QBrush bCell(cSfondo, Qt::SolidPattern);
    tVariable->setBackground(colTreeName, bCell);
    tVariable->setBackground(colTreeInfo, bCell);
    tVariable->setBackground(colTreeTimings, bCell);
    return tVariable;
}
QTreeWidgetItem *ctedit::addDevice2Tree(QTreeWidgetItem *tParent, int nDevice)
// Aggiunge il Device nDevice agganciandolo al nodo tParent. Ritorna oggetto
{
    QTreeWidgetItem *tCurrentDevice;
    QString         szName(szEMPTY);
    QString         szInfo(szEMPTY);
    QString         szToolTip(szEMPTY);
    QString         szTimings(szEMPTY);
    int             nProtocol = theDevices[nDevice].nProtocol;
    int             nTotalReadTime = 0;

    if (nDevice <0)  {
        szName = QString::fromLatin1("PLC");
        szInfo = QString::fromAscii("Total Variables: %1") .arg(thePlcVarsNumber);
    }
    else if (nDevice >= 0 && nDevice < theDevicesNumber)  {
        // Colonna Nome
        szName = theDevices[nDevice].szDeviceName;
        // Colonna Info
        szInfo = QString::fromAscii("Device Variables: %1\t") .arg(theDevices[nDevice].nVars, 10, 10);
        szInfo.append(QString::fromAscii("Max Block Size: %1\t").arg(theDevices[nDevice].nMaxBlockSize, 6, 10));
        // TimeOut
        if (theDevices[nDevice].nTimeOut >= 0)
            szInfo.append(QString::fromAscii("TimeOut: %1 ms \t") .arg(theDevices[nDevice].nTimeOut, 6, 10));
        else
            szInfo.append(QLatin1String("\t\t"));
        // Silence
        if (theDevices[nDevice].nSilence >= 0)
            szInfo.append(QString::fromAscii("Silence: %1 ms \t") .arg(theDevices[nDevice].nSilence, 6, 10));
        else
            szInfo.append(QLatin1String("\t\t"));
        if (nProtocol == RTU || nProtocol == RTU_SRV)  {
            szInfo.append(QString::fromAscii("Min.Silence: %1 ms \t") .arg(theDevices[nDevice].dMinSilence));
            if ((double) theDevices[nDevice].nSilence >= theDevices[nDevice].dMinSilence)
                szInfo.append(QLatin1String("OK\t"));
            else
                szInfo.append(QLatin1String("Too Short\t"));
            // Total Read Time
            // szInfo.append(QString::fromAscii("Dev.Read Time: %1 ms\t").arg(theDevices[nDevice].nDeviceReadTime, 6, 10));
        }
        // ToolTip
        szToolTip.append(QString::fromAscii("Device ID:\t%1\n") .arg(nDevice, 8, 10));
        szToolTip.append(QString::fromAscii("Protocol:\t%1\n") .arg(lstProtocol[theDevices[nDevice].nProtocol]));
        if (nProtocol == TCP || nProtocol == TCPRTU || nProtocol == TCP_SRV || nProtocol == TCPRTU_SRV)  {
            szToolTip.append(QString::fromAscii("Ip Address:\t%1\n") .arg(theDevices[nDevice].szIpAddress));
        }
        szToolTip.append(QString::fromAscii("Port:\t%1\n") .arg(theDevices[nDevice].nPort));
        szToolTip.append(QString::fromAscii("Device Variables:\t%1\n") .arg(theDevices[nDevice].nVars, 8, 10));
        szToolTip.append(QString::fromAscii("Diag Variable Id:\t%1\n") .arg(theDevices[nDevice].diagnosticAddr, 8, 10));
        szToolTip.append(QString::fromAscii("Diag Variable Name:\t%1") .arg(theDevices[nDevice].diagnosticVarName, -60));
    }
    else  {
        szName = QLatin1String("UNDEFINED");
    }
    // Device Total Read Time
    if (nProtocol == RTU || nProtocol == RTU_SRV)  {
        int nCur = 0;
        for (nCur = nPriorityHigh; nCur <= nPriorityLow; nCur++)  {
            if (theDevices[nDevice].nDeviceReadTime[nCur] > 0)  {
                nTotalReadTime += theDevices[nDevice].nDeviceReadTime[nCur];
                szTimings.append(QString::fromAscii(" - %1: %2 ms") .arg(lstPriorityDesc[nCur]) .arg(theDevices[nDevice].nDeviceReadTime[nCur], 6, 10));
            }
        }
        szTimings.prepend(QString::fromAscii("Read Time: %1 ms") .arg(nTotalReadTime, 6, 10));
    }
    // Adding Device Item to tree
    tCurrentDevice = addItem2Tree(tParent, treeDevice, szName, szInfo, szTimings, szToolTip);
    // Return Value
    return tCurrentDevice;
}
QTreeWidgetItem *ctedit::addNode2Tree(QTreeWidgetItem *tParent, int nNode)
// Aggiunge il Nodo nNode agganciandolo al nodo tParent. Ritorna oggetto
{
    QTreeWidgetItem *tCurrentNode;
    QString         szName(szEMPTY);
    QString         szInfo(szEMPTY);
    QString         szToolTip(szEMPTY);
    QString         szTimings(szEMPTY);

    // Ricerca Info Nodo
    if (nNode >= 0 && nNode < theNodesNumber)  {
        szName = theNodes[nNode].szNodeName;
        szInfo = QString::fromAscii("Node Variables: %1") .arg(theNodes[nNode].nVars, 12, 10);
        szToolTip = QString::fromAscii("Device:\t%1\n") .arg(theDevices[theNodes[nNode].nDevice].szDeviceName,-20);
        szToolTip.append(QString::fromAscii("Node Id:\t%1\n") .arg(theNodes[nNode].nNodeId));
        szToolTip.append(QString::fromAscii("Node Variables:\t%1\n") .arg(theNodes[nNode].nVars, 12, 10));
        szToolTip.append(QString::fromAscii("Diag Variable Id:\t%1\n") .arg(theNodes[nNode].diagnosticAddr, 12, 10));
        szToolTip.append(QString::fromAscii("Diag Variable Name:\t%1") .arg(theNodes[nNode].diagnosticVarName, -20));
    }
    else  {
        szName = QLatin1String("UNDEFINED");
    }
    // Adding Node Item to Tree
    tCurrentNode = addItem2Tree(tParent, treeNode, szName, szInfo, szTimings, szToolTip);
    // Return Value
    return tCurrentNode;
}
QTreeWidgetItem *ctedit::addPriority2Tree(QTreeWidgetItem *tParent, int nPriority)
// Aggiunge la Priority nPriority agganciandolo al nodo tParent. Ritorna oggetto
{
    QTreeWidgetItem *tPriority;
    QString         szName(szEMPTY);
    QString         szInfo(szEMPTY);
    QString         szToolTip(szEMPTY);
    QString         szTimings(szEMPTY);
    int             nPeriod = 0;

    szName = priority2String(nPriority);
    // Read Period per Priorita
    if (nPriority == nPriorityHigh)
        nPeriod = panelConfig.readPeriod1;
    else if (nPriority == nPriorityMedium)
        nPeriod = panelConfig.readPeriod2;
    else if (nPriority == nPriorityLow)
        nPeriod = panelConfig.readPeriod3;
    else
        nPeriod = 0;
    szInfo = QString::fromAscii("Read Period: %1 ms") .arg(nPeriod, 10, 10);
    // Adding Node Item to Tree
    tPriority = addItem2Tree(tParent, treePriority, szName, szInfo, szTimings, szToolTip);
    // Return Value
    return tPriority;
}
int     ctedit::searchBlock(int nBlock)
// Ricerca in theBlocks del blocco nBlock
{
    int nFound = -1;
    int nCur = 0;

    for (nCur = 0; nCur < theBlocksNumber; nCur++)   {
        if (theBlocks[nCur].nBlockId == nBlock)  {
            nFound = nCur;
            break;
        }
    }
    // Return Value
    return nFound;
}

QTreeWidgetItem *ctedit::addBlock2Tree(QTreeWidgetItem *tParent, int nBlock, int nBlockSize)
// Aggiunge il blocco nBlock agganciandolo al nodo tParent. Ritorna oggetto
{
    QTreeWidgetItem *tBlock;
    QString         szName(szEMPTY);
    QString         szInfo(szEMPTY);
    QString         szToolTip(szEMPTY);
    QString         szTimings(szEMPTY);
    int             nInfoBlock = searchBlock(nBlock);

    szName = QString::fromAscii("Block_") + int2PaddedString(nBlock, 2, 10);
    szInfo = QString::fromAscii("Block Variables: %1\t") .arg(nBlockSize, 12, 10);
    if (nInfoBlock >= 0 && nInfoBlock < theBlocksNumber)  {
        if (theBlocks[nInfoBlock].nProtocol == RTU || theBlocks[nInfoBlock].nProtocol == RTU_SRV)  {
            szTimings.append(QString::fromAscii("Read Time: %1 ms\t") .arg(theBlocks[nInfoBlock].nReadTime_ms, 6, 10));
        }
    }
    // Adding Node Item to Tree
    tBlock = addItem2Tree(tParent, treeBlock, szName, szInfo, szTimings, szToolTip);
    // Return Value
    return tBlock;
}

void    ctedit::fillDeviceTree(int nCurRow)
// Riempimento Albero dei device collegati al TP
{
    QTreeWidgetItem *tItem;
    QTreeWidgetItem *tRoot;
    QTreeWidgetItem *tCurrentDevice;
    QTreeWidgetItem *tCurrentNode;
    QTreeWidgetItem *tCurrentPriority;
    QTreeWidgetItem *tCurrentBlock;
    QTreeWidgetItem *tCurrentVariable;
    QString         szName;
    QString         szTimings;
    QStringList     lstTreeHeads;
    QFont           treeFont = ui->deviceTree->font();
    int             nRow = 0;
    int             nDevice = -1;
    int             nNode = -1;
    int             nPriority = -1;
    int             nBlock = -1;
    int             nBlockSize = -1;
    int             nUsedVariables = 0;
    int             nProtocol = PLC;
    int             nReadTime = 0;
    int             nCur = 0;
    bool            isPLC = false;

    // Preparing Tree
    ui->deviceTree->clear();
    lstTreeHeads.clear();
    lstTreeHeads
            << QLatin1String("Name")
            << QLatin1String("Info")
            << QLatin1String("Timings")
        ;
    treeFont.setFixedPitch(true);
    ui->deviceTree->setFont(treeFont);
    ui->deviceTree->setColumnCount(colTreeTotals);
    tRoot = new QTreeWidgetItem(ui->deviceTree, treeRoot);
    tRoot->setText(colTreeName, m_szCurrentModel);
    tCurrentVariable = 0;
    // Build Server-Device-Nodes structures
    if (! checkServersDevicesAndNodes())  {
        m_szMsg = QString::fromAscii("Error checking Device and Nodes structure, cannot show %1 Tree !") .arg(QLatin1String("Devices"));
        warnUser(this, szMectTitle, m_szMsg);
        return;
    }
    // Variables Loop
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Considera solo le Variabili utilizzate
        if (lstCTRecords[nRow].UsedEntry > 0 && lstCTRecords[nRow].Enable > 0)  {
            nDevice = lstCTRecords[nRow].nDevice;
            nNode = lstCTRecords[nRow].nNode;
            nPriority = lstCTRecords[nRow].Enable;
            nBlock = lstCTRecords[nRow].Block;
            nBlockSize = lstCTRecords[nRow].BlockSize;
            nProtocol = lstCTRecords[nRow].Protocol;
            isPLC = (nProtocol == PLC);
            //----------------------------
            // Analisi del Device
            //----------------------------
            if (isPLC)  {
                szName = QString::fromLatin1("PLC");
                nDevice = -1;
            }
            else  {
                if (nDevice >= 0 && nDevice < nMAX_DEVICES)  {
                    szName = theDevices[nDevice].szDeviceName;
                }
                else  {
                    szName = QLatin1String("UNDEFINED");
                }
            }
            // Ricerca del Device in Tree
            tCurrentDevice = searchTreeChild(tRoot, colTreeName, szName);
            // Adding Device
            if (tCurrentDevice == 0)  {
                // Aggiunta colonna Info e Tooltip
                tCurrentDevice = addDevice2Tree(tRoot, nDevice);
            }
            tCurrentDevice->setExpanded(true);
            // If Protocol == PLC skip tree costruction, jump 2 Variable appending
            if (isPLC)  {
                tCurrentBlock = tCurrentDevice;
                tCurrentBlock->setExpanded(false);
                goto addVariable;
            }            
            //----------------------------
            // Analisi Node
            //----------------------------
            if (nNode >= 0 && nNode < theNodesNumber)  {
                szName = theNodes[nNode].szNodeName;
            }
            else  {
                szName = QLatin1String("UNDEFINED");
            }
            // Ricerca del Node in Tree
            tCurrentNode = searchTreeChild(tCurrentDevice, colTreeName, szName);
            // Adding Node
            if (tCurrentNode == 0)  {
                tCurrentNode = addNode2Tree(tCurrentDevice, nNode);
                // Calcolo del Read Time Complessivo per il Nodo
                if (nProtocol == RTU || nProtocol == RTU_SRV)  {
                    nReadTime = 0;
                    szTimings.clear();
                    for (nCur = nPriorityHigh; nCur <= nPriorityLow; nCur++)  {
                        if (theNodes[nNode].nNodeReadTime[nCur] > 0)  {
                            nReadTime += theNodes[nNode].nNodeReadTime[nCur];
                            szTimings.append(QString::fromAscii(" - %1: %2 ms") .arg(lstPriorityDesc[nCur]) .arg(theNodes[nNode].nNodeReadTime[nCur], 6, 10));
                        }
                    }
                    szTimings.prepend(QString::fromAscii("Read Time: %1 ms") .arg(nReadTime, 6, 10));
                    tCurrentNode->setText(colTreeTimings, szTimings);
                }
            }
            tCurrentNode->setExpanded(true);
            //----------------------------
            // Analisi Priority
            //----------------------------
            szName = priority2String(nPriority);
            // Ricerca del Prority in Tree
            tCurrentPriority = searchTreeChild(tCurrentNode, colTreeName, szName);
            // Adding Priority
            if (tCurrentPriority == 0)  {
                tCurrentPriority = addPriority2Tree(tCurrentNode, nPriority);
                // Calcolo del Read Time Complessivo per la priorità corrente
                if (nProtocol == RTU || nProtocol == RTU_SRV)  {
                    szTimings = QString::fromAscii("Read Time: %1 ms") .arg(theNodes[nNode].nNodeReadTime[nPriority], 6, 10);
                    tCurrentPriority->setText(colTreeTimings, szTimings);
                }
            }
            tCurrentPriority->setExpanded(false);
            //----------------------------
            // Block Name
            //----------------------------
            szName = QLatin1String("Block_") + int2PaddedString(nBlock, 2, 10);
            // Ricerca del Block in Tree
            tCurrentBlock = searchTreeChild(tCurrentPriority, colTreeName, szName);
            // Adding Block
            if (tCurrentBlock == 0)  {
                tCurrentBlock = addBlock2Tree(tCurrentPriority, nBlock, nBlockSize);
            }
            tCurrentBlock->setExpanded(false);

addVariable:
            //----------------------------
            // Adding Variable
            //----------------------------
            tItem = addVariable2Tree(tCurrentBlock, nRow);
            // Trovata riga corrente in griglia
            if (nRow == nCurRow)  {
                tCurrentVariable = tItem;
            }
            // Incremento numero variabili utilizzate
            nUsedVariables++;
        }   // Used Entry
    }
    // Aggiornamento delle Informazioni del Nodo Principale
    tRoot->setExpanded(true);
    szName = QString::fromAscii("Total Variables: %1\t") .arg(nUsedVariables);
    szName.append(QString::fromAscii("Used Devices: %1\t") .arg(theDevicesNumber));
    szName.append(QString::fromAscii("Retries: %1\t") .arg(panelConfig.retries));
    szName.append(QString::fromAscii("Black List: %1\t") .arg(panelConfig.blacklist));
    tRoot->setText(colTreeInfo, szName);
    tRoot->setToolTip(colTreeName, ui->lblModel->toolTip());
    // Seleziona l'item corrispondente alla riga corrente
    // Salto alla riga corrispondente alla variabile selezionata in griglia CT
    if (tCurrentVariable != 0)  {
//        qDebug() << QString::fromAscii("fillDeviceTree(): Selected: %1") .arg(tCurrentVariable->text(colTreeName));
        tCurrentVariable->setSelected(true);
        ui->deviceTree->scrollToItem(tCurrentVariable, QAbstractItemView::PositionAtCenter);
    }
    else {
        tRoot->setSelected(true);
    }
    // Tree Header
    ui->deviceTree->setHeaderLabels(lstTreeHeads);
    ui->deviceTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->deviceTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->deviceTree->header()->resizeSection(colTreeName, (ui->deviceTree->width() / 5) - 12);
    ui->deviceTree->header()->resizeSection(colTreeInfo, (ui->deviceTree->width() / 5) * 3 - 12);
    ui->deviceTree->header()->resizeSections(QHeaderView::Interactive);
    m_rebuildDeviceTree = false;
}
void    ctedit::fillTimingsTree(int nCurRow)
// Riempimento Albero delle variabili raggruppate per Priorità (Timings)
{
    QTreeWidgetItem *tItem;
    QTreeWidgetItem *tRoot;
    QTreeWidgetItem *tCurrentDevice;
    QTreeWidgetItem *tCurrentNode;
    QTreeWidgetItem *tCurrentPriority;
    QTreeWidgetItem *tCurrentBlock;
    QTreeWidgetItem *tCurrentVariable;
    QString         szName;
    QString         szTimings;
    QStringList     lstTreeHeads;
    QFont           treeFont = ui->timingTree->font();
    int             nRow = 0;
    int             nDevice = -1;
    int             nNode = -1;
    int             nPriority = -1;
    int             nUsedVariables = 0;
    int             nProtocol = PLC;
    bool            isPLC = false;
    int             nBlock = -1;
    int             nBlockSize = -1;

    // Preparing Tree
    ui->timingTree->clear();
    lstTreeHeads.clear();
    lstTreeHeads
            << QLatin1String("Name")
            << QLatin1String("Info")
            << QLatin1String("Timings")
        ;
    treeFont.setFixedPitch(true);
    ui->timingTree->setFont(treeFont);
    ui->timingTree->setColumnCount(colTreeTotals);
    tRoot = new QTreeWidgetItem(ui->timingTree, treeRoot);
    tRoot->setText(colTreeName, m_szCurrentModel);
    tCurrentVariable = 0;
    if (! checkServersDevicesAndNodes())  {
        m_szMsg = QString::fromAscii("Error checking Device and Nodes structure, cannot show %1 Tree !") .arg(QLatin1String("Timings"));
        warnUser(this, szMectTitle, m_szMsg);
        return;
    }
    // Creazione primo livello di Albero (Devices)
    for (nDevice = 0; nDevice < theDevicesNumber; nDevice++)  {
        tCurrentDevice = addDevice2Tree(tRoot, nDevice);
        tCurrentDevice->setExpanded(true);
    }
    // Nodo per Variabili PLC
    tCurrentDevice =  addDevice2Tree(tRoot, -1);
    // Variables Loop
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Considera solo le Variabili utilizzate
        if (lstCTRecords[nRow].UsedEntry > 0 && lstCTRecords[nRow].Enable > 0)  {
            nDevice = lstCTRecords[nRow].nDevice;
            nNode = lstCTRecords[nRow].nNode;
            nPriority = lstCTRecords[nRow].Enable;
            nBlock = lstCTRecords[nRow].Block;
            nBlockSize = lstCTRecords[nRow].BlockSize;
            nProtocol = lstCTRecords[nRow].Protocol;
            isPLC = (nProtocol == PLC);
            // Protocollo PLC raggruppato tutto insieme senza distinzione di Variabili
            if (isPLC)  {
                szName = lstProtocol[PLC];
            }
            else  {
                szName = theDevices[nDevice].szDeviceName;
            }
            //----------------------------
            // Ricerca del Device in Tree. Deve già esistere in quanto creati a monte da Lista Devices
            //----------------------------
            tCurrentDevice = searchTreeChild(tRoot, colTreeName, szName);
            if (tCurrentDevice != 0)  {
                if (! isPLC)  {
                    //----------------------------
                    // Analisi Priority
                    //----------------------------
                    szName = priority2String(nPriority);
                    // Ricerca del Prority in Tree
                    tCurrentPriority = searchTreeChild(tCurrentDevice, colTreeName, szName);
                    // Adding Priority
                    if (tCurrentPriority == 0)  {
                        tCurrentPriority = addPriority2Tree(tCurrentDevice, nPriority);
                        // Calcolo del Read Time Device-->Priorita
                        if (nProtocol == RTU || nProtocol == RTU_SRV)  {
                            szTimings = QString::fromAscii("Read Time: %1 ms") .arg(theDevices[nDevice].nDeviceReadTime[nPriority], 6, 10);
                            tCurrentPriority->setText(colTreeTimings, szTimings);
                        }
                    }
                    tCurrentPriority->setExpanded(true);
                    //----------------------------
                    // Analisi Node
                    //----------------------------
                    if (nNode >= 0 && nNode < theNodesNumber)  {
                        szName = theNodes[nNode].szNodeName;
                    }
                    else  {
                        szName = QLatin1String("UNDEFINED");
                    }
                    // Ricerca del Node in Tree
                    tCurrentNode = searchTreeChild(tCurrentPriority, colTreeName, szName);
                    // Adding Node
                    if (tCurrentNode == 0)  {
                        tCurrentNode = addNode2Tree(tCurrentPriority, nNode);
                        // Calcolo del Read Time Priorita-->Nodo
                        if (nProtocol == RTU || nProtocol == RTU_SRV)  {
                            szTimings = QString::fromAscii("Read Time: %1 ms") .arg(theNodes[nNode].nNodeReadTime[nPriority], 6, 10);
                            tCurrentNode->setText(colTreeTimings, szTimings);
                        }
                    }
                    tCurrentNode->setExpanded(nRow == nCurRow);
                    //----------------------------
                    // Block Name
                    //----------------------------
                    szName = QLatin1String("Block_") + int2PaddedString(nBlock, 2, 10);
                    // Ricerca del Block in Tree
                    tCurrentBlock = searchTreeChild(tCurrentNode, colTreeName, szName);
                    // Adding Block
                    if (tCurrentBlock == 0)  {
                        tCurrentBlock = addBlock2Tree(tCurrentNode, nBlock, nBlockSize);
                    }
                }
                else  {
                    // Se variabile PLC non resta da fare altro che aggiungere la Variabile alla lista
                    tCurrentBlock = tCurrentDevice;
                }
                //----------------------------
                // Adding Variable to Current Block
                //----------------------------
                tItem = addVariable2Tree(tCurrentBlock, nRow);
                // Trovata riga corrente in griglia
                if (nRow == nCurRow)  {
                    tCurrentVariable = tItem;
                }

            }
            // Incremento numero variabili utilizzate
            nUsedVariables++;
        }
    }
    // Aggiornamento delle Informazioni del Nodo Principale
    tRoot->setExpanded(true);
    szName = QString::fromAscii("Total Variables: %1\t") .arg(nUsedVariables);
    szName.append(QString::fromAscii("Used Devices: %1\t") .arg(theDevicesNumber));
    szName.append(QString::fromAscii("Retries: %1\t") .arg(panelConfig.retries));
    szName.append(QString::fromAscii("Black List: %1\t") .arg(panelConfig.blacklist));
    tRoot->setText(colTreeInfo, szName);
    tRoot->setToolTip(colTreeName, ui->lblModel->toolTip());
    // Salto alla riga corrispondente alla variabile selezionata in griglia CT
    if (tCurrentVariable != 0)  {
//        qDebug() << QString::fromAscii("fillTimingsTree(): Selected: %1") .arg(tCurrentVariable->text(colTreeName));
        tCurrentVariable->setSelected(true);
        ui->deviceTree->scrollToItem(tCurrentVariable, QAbstractItemView::EnsureVisible);
    }
    else {
        tRoot->setSelected(true);
    }
    // Tree Header
    ui->timingTree->setHeaderLabels(lstTreeHeads);
    ui->timingTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->timingTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->timingTree->header()->resizeSection(colTreeName, (ui->timingTree->width() / 5) - 12);
    ui->timingTree->header()->resizeSection(colTreeInfo, (ui->timingTree->width() / 5) * 3 - 12);
    ui->timingTree->header()->resizeSections(QHeaderView::Interactive);
    m_rebuildTimingTree = false;
}
void    ctedit::fillLogTree(int nCurRow)
// Riempimento Albero delle variabili raggruppate per Log Period
{
    QTreeWidgetItem *tParent;
    QTreeWidgetItem *tRoot;
    QTreeWidgetItem *tNewVariable;
    QTreeWidgetItem *tFast;
    QTreeWidgetItem *tSlow;
    QTreeWidgetItem *tOnVar;
    QTreeWidgetItem *tOnShot;
    QTreeWidgetItem *tCurrentVariable;
    QString         szName;
    QString         szInfo;
    QString         szTimings;
    QString         szToolTip;
    QStringList     lstTreeHeads;
    QFont           treeFont = ui->deviceTree->font();
    int             nRow = 0;
    int             nLogLevel = Ptype;
    int             nUsedVariables = 0;
    int             nFast = 0;
    int             nSlow = 0;
    int             nVar = 0;
    int             nShot = 0;

    // Preparing Tree
    ui->logTree->clear();
    lstTreeHeads.clear();
    lstTreeHeads.append(QLatin1String("Log Level"));
    lstTreeHeads.append(QLatin1String("Variables"));
    lstTreeHeads.append(szEMPTY);
    treeFont.setFixedPitch(true);
    ui->logTree->setFont(treeFont);
    ui->logTree->setColumnCount(colTreeTotals);
    tRoot = new QTreeWidgetItem(ui->logTree, treeRoot);
    tRoot->setText(colTreeName, m_szCurrentModel);
    tCurrentVariable = 0;
    // Check Elements
    if (countLoggedVars(lstCTRecords, nFast, nSlow, nVar, nShot) <= 0)  {
        return;
    }
    // Rebuild Server-Device-Nodes structures
    if (! checkServersDevicesAndNodes())  {
        m_szMsg = QString::fromAscii("Error checking Device and Nodes structure, cannot show %1 Tree !") .arg(QLatin1String("Devices"));
        warnUser(this, szMectTitle, m_szMsg);
        return;
    }
    // Crea i Nodi per i 4 Livelli di LOG
    szTimings.clear();
    szToolTip.clear();
    tParent = 0;
    tNewVariable = 0;
    // Fast Log
    szName = QLatin1String("F\tFast");
    szInfo = QString::fromAscii("Fast Log Period: %1\tLogged Variables: %2") .arg(panelConfig.fastLogPeriod, 6, 10) .arg(nFast, 6, 10);
    tFast = addItem2Tree(tRoot, treeDevice, szName, szInfo, szTimings, szToolTip);
    // Slow Log
    szName = QLatin1String("S\tSlow");
    szInfo = QString::fromAscii("Slow Log Period: %1\tLogged Variables: %2") .arg(panelConfig.slowLogPeriod, 6, 10) .arg(nSlow, 6, 10);
    tSlow = addItem2Tree(tRoot, treeDevice, szName, szInfo, szTimings, szToolTip);
    // On Variation
    szName = QLatin1String("V\tOn Variation");
    szInfo = QString::fromAscii("\t\tLogged Variables: %1") .arg(nVar, 6, 10);
    tOnVar = addItem2Tree(tRoot, treeDevice, szName, szInfo, szTimings, szToolTip);
    // On Shot
    szName = QLatin1String("X\tOn Shot");
    szInfo = QString::fromAscii("\t\tLogged Variables: %1") .arg(nShot, 6, 10);
    tOnShot = addItem2Tree(tRoot, treeDevice, szName, szInfo, szTimings, szToolTip);
    // Variables Loop
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Considera solo le Variabili che devono essere Loggate
        if (lstCTRecords[nRow].UsedEntry > 0 && lstCTRecords[nRow].Enable > 0)  {
            nUsedVariables++;
            nLogLevel = lstCTRecords[nRow].Update;
            tParent = 0;
            // Ignora le variabili
            if (nLogLevel > Ptype)  {
                if (nLogLevel == Ftype)  {
                    tParent = tFast;
                }
                else if (nLogLevel == Stype)  {
                    tParent = tSlow;
                }
                else if (nLogLevel == Vtype)  {
                    tParent = tOnVar;
                }
                else if (nLogLevel == Xtype)  {
                    tParent = tOnShot;
                }
                // Add Variable
                tNewVariable = 0;
                if (tParent != 0)  {
                    tNewVariable = addVariable2Tree(tParent, nRow, treeNode);
                    // Seleziona la variabile se coincide con la riga corrente
                    if (nCurRow >= 0 && nRow == nCurRow && tNewVariable != 0)  {
                        tCurrentVariable = tNewVariable;
                    }
                }
            }
        }
    }
    // Aggiornamento delle Informazioni del Nodo Principale
    tRoot->setExpanded(true);
    szInfo = QString::fromAscii("Total Variables: %1\t") .arg(nUsedVariables, 6, 10);
    szInfo.append(QString::fromAscii("Fast: %1\t") .arg(nFast, 6, 10));
    szInfo.append(QString::fromAscii("Slow: %1\t") .arg(nSlow, 6, 10));
    szInfo.append(QString::fromAscii("On Var: %1\t") .arg(nVar, 6, 10));
    szInfo.append(QString::fromAscii("On Shot: %1\t") .arg(nShot, 6, 10));
    tRoot->setText(colTreeInfo, szInfo);
    tRoot->setToolTip(colTreeName, ui->lblModel->toolTip());
    tRoot->setSelected(true);
    // Seleziona l'item corrispondente alla riga corrente
    // Salto alla riga corrispondente alla variabile selezionata in griglia CT
    if (tCurrentVariable != 0)  {
        tCurrentVariable->setSelected(true);
        ui->logTree->scrollToItem(tCurrentVariable, QAbstractItemView::PositionAtCenter);
    }
    else {
    }
    // Tree Header
    ui->logTree->setHeaderLabels(lstTreeHeads);
    ui->logTree->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->logTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->logTree->header()->resizeSection(colTreeName, (ui->logTree->width() / 5) - 12);
    ui->logTree->header()->resizeSection(colTreeInfo, (ui->logTree->width() / 5) * 3 - 12);
    ui->logTree->header()->resizeSections(QHeaderView::Interactive);
}

void    ctedit::showTabMPNC()
{
    if (lstMPNC.count() > 0)  {
        m_nMPNC = 0;
        // Ricerca della riga corrente nella dimensione dei Blocchi trovati
        for (int nItem = 0; nItem < lstMPNC.count(); nItem++)  {
            if (m_nGridRow >= lstMPNC[nItem] && m_nGridRow < lstMPNC[nItem] + lstMPNC006_Vars.count())  {
                m_nMPNC = nItem;
                break;
            }
        }
        configMPNC->localCTRecords = lstCTRecords;
        configMPNC->showTestaNodi(m_nMPNC, lstMPNC, m_nGridRow);
    }
}
void    ctedit::showTabMPNE()
{
    if (lstMPNE.count() > 0)  {
        m_nMPNE = 0;
        // Ricerca della riga corrente nella dimensione dei Blocchi trovati
        for (int nItem = 0; nItem < lstMPNE.count(); nItem++)  {
            if (m_nGridRow >= lstMPNE[nItem] && m_nGridRow < lstMPNE[nItem] + lstMPNE_Vars.count())  {
                m_nMPNE = nItem;
                break;
            }
        }
        configMPNE->localCTRecords = lstCTRecords;
        configMPNE->showTestaNodi(m_nMPNE, lstMPNE, m_nGridRow);
    }
}
void ctedit::return2GridRow(int nRow)
// Ritorno da Tab MPNC - MPNE
{
    qDebug() << QString::fromAscii("return2GridRow: Selected Row: %1 - Previous Current Row: %2") .arg(nRow) .arg(m_nGridRow);
    if (nRow > 0 && nRow <= DimCrossTable)  {
        m_nGridRow = nRow - 1;
    }
    // Jump to CT Tab
    ui->tabWidget->setCurrentIndex(TAB_CT);
}
int     ctedit::checkRegister(int nCurRow, QList<int> &lstUsingRegister)
// Controllo delle variabili che utilizzano un Registro
{
    int     nUsed  = 0;
    int     nRow = 0;

    lstUsingRegister.clear();
    // Ciclo su tutte le Variabili di CT
    for (nRow = 0; nRow < lstCTRecords.count(); nRow++)  {
        // Controlla le righe usate diverse dalla riga corrente
        if (nRow != nCurRow && lstCTRecords[nRow].UsedEntry)  {
            // Controllo se un Registro risulta già utilizzato in un altra variabile
            if (lstCTRecords[nRow].nDevice == lstCTRecords[nCurRow].nDevice &&
                lstCTRecords[nRow].nNode == lstCTRecords[nCurRow].nNode &&
                lstCTRecords[nRow].Offset == lstCTRecords[nCurRow].Offset  &&
                lstCTRecords[nRow].Decimal == lstCTRecords[nCurRow].Decimal)  {
                nUsed++;
                lstUsingRegister.append(nRow);
            }
        }
    }
    // Return Value
    return nUsed;
}

void ctedit::on_cmdApply_clicked()
{
    bool    fRes = false;

    // Salva l'attuale CT il lista Undo
    lstUndo.append(lstCTRecords);
    // Cerca il primo Item utilizzato della lista delle variabili selezionate
    for (int nItem = 0; nItem < lstSelectedRows.count(); nItem++)  {
        int nCurRow = lstSelectedRows.at(nItem);
        if (nCurRow >= 0 && nCurRow < lstCTRecords.count())  {
            // Il contenuto viene aggiornato solo se la linea risulta modificata e il form non è vuoto
            fRes = updateRow(nCurRow, true);
        }
        // Aggiornamento Riga Ko
        if (! fRes)    {
            // Disconnette segnale per evitare ricorsione
            disconnect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
                        SLOT(tableCTItemChanged(const QItemSelection &, const QItemSelection & ) ));
            // Cambia Selezione (ritorna a riga precedente)
            ui->tblCT->clearSelection();
            ui->tblCT->selectRow(nCurRow);
            m_nGridRow = nCurRow;
            // Riconnette slot gestione
            connect(ui->tblCT->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this,
                        SLOT(tableCTItemChanged(const QItemSelection &, const QItemSelection & ) ));
            return;
        }
    }
    m_isCtModified = true;
    m_rebuildDeviceTree = true;
    m_rebuildTimingTree = true;
    // Aggiorna abilitazioni (per Bottone UNDO)
    enableInterface();
}

void ctedit::on_cmdMultiEdit_clicked(bool checked)
{
    int nRow = m_nGridRow;

    if (! checked)  {
        // Esce da MultiEdit
        // Cerca il primo Item utilizzato della lista delle variabili selezionate
        for (int nItem = 0; nItem < lstSelectedRows.count(); nItem++)  {
            int nCurRow = lstSelectedRows.at(nItem);
            if (! ui->tblCT->item(nCurRow, colName)->text().trimmed().isEmpty())  {
                nRow = nCurRow;
                break;
            }
        }
        // Svuota lista elementi selezionati e toglie flag di MultiSelect
        lstSelectedRows.clear();
        m_fMultiSelect = false;
        qDebug("Cancelling MultiEdit: GotoRow: %d", nRow + 1);
        // Seleziona Riga corrente
        ui->cmdMultiEdit->setChecked(false);
        ui->cmdMultiEdit->setToolTip(QLatin1String("Switch to Multiline Edit Mode"));
        ui->tblCT->selectRow(nRow);
        enableFields();
        m_nGridRow = nRow;
        m_isCtModified = true;
        m_rebuildDeviceTree = true;
        m_rebuildTimingTree = true;
    }
    else  {
        ui->cmdMultiEdit->setToolTip(QLatin1String("Back to Single Line Edit Mode"));
    }
    m_fMultiEdit = checked;
}

void ctedit::highlightEditableFields(QListWidgetItem *itemClicked)
// Evidenzia i campi editabili (MultiEdit)
{
    // Impostazione del Colore  Background dell'item
    if (itemClicked->checkState() == Qt::Checked)  {
        itemClicked->setTextColor(QColor(QLatin1String("LemonChiffon")));
        itemClicked->setBackgroundColor(QColor(QLatin1String("DodgerBlue")));
    }
    else  {
        itemClicked->setTextColor(QColor(QLatin1String("Black")));
        itemClicked->setBackgroundColor(colorNormalEdit);
    }
    lstEditableFields.clear();
    QListWidgetItem* listItem = 0;
    for (int nItem = 0; nItem < ui->lstEditableFields->count(); nItem++)  {
        listItem = ui->lstEditableFields->item(nItem);
        if (listItem->checkState() == Qt::Checked)  {
            int nPos = lstHeadCols.indexOf(listItem->text());
            if (nPos >= 0)  {
                lstEditableFields.append(nPos);
            }
        }
    }
    qSort(lstEditableFields.begin(), lstEditableFields.end());
    ui->lblEditableFields->setText(QString(QLatin1String("Editable Fields:\n%1")) .arg(lstEditableFields.count()));
    if (m_fMultiEdit)  {
        enableFields();
    }
}

