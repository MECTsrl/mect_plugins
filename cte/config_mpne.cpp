#include "config_mpne.h"
#include "utils.h"
#include "cteUtils.h"
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QColor>
#include <QIcon>
#include <QDebug>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QIntValidator>
#include <QEvent>

// Cardinalità tipi di Moduli
const int nTotalModules = 6;
// Posizioni dei Moduli
const int nModuleBase = 0;
const int nModuleLeft = 1;
const int nModuleRight = 2;
const int nTotalItems = 3;
// Posizioni nel LayOut (Riga)
const int nRowSelector = 0;
const int nRowTags = 1;
const int nRowCombo = 2;
const int nRowButtons = 3;
const int nRowFlags = 4;
const int nRowGrid = 5;
// Posizione nel LayOut (Colonna)
const int nColBase = 0;
const int nColLeft = 1;
const int nColRight = 3;
const int nColProtocol = 4;
const int nColPort = 5;
const int nColNode = 7;
const int nColLast = 9;
const int nItemWidth = 2;
const int nComboWidth = 93;
// Funzionalità della variabile (per discriminare i tipi di utilizzo del modulo)
const int nUsageNone = 0;
const int nUsageDigIn = 1;
const int nUsageDigOut = 2;
const int nUsageAnIO = 3;

// Sfondi
const QString szFileRename = szPathIMG + QString::fromAscii("Rename2.png");
// Sfondi per Moduli
const QString szFileMPNE10L = szPathIMG + QString::fromAscii("MPNE01L.png");
const QString szFileMPNE10R = szPathIMG + QString::fromAscii("MPNE01R.png");
const QString szFileMPNE00 = szEMPTY;
const QString szFileMPNE01 = szPathIMG + QString::fromAscii("MPNE01.png");
const QString szFileMPNE02 = szPathIMG + QString::fromAscii("MPNE02.png");
const QString szFileMPNE03 = szPathIMG + QString::fromAscii("MPNE03.png");
const QString szFileMPNE04 = szPathIMG + QString::fromAscii("MPNE04.png");
const QString szFileMPNE05 = szPathIMG + QString::fromAscii("MPNE05.png");
// Sfondi Filtro Visualizzazione
const QString szFilterHead = szPathIMG + QString::fromAscii("ShowHead.png");
const QString szFilterUsed = szPathIMG + QString::fromAscii("ShowUsed.png");
const QString szFilterAll = szPathIMG + QString::fromAscii("ShowAll.png");


Config_MPNE::Config_MPNE(QWidget *parent) :
    QWidget(parent)
{
    int         i = 0;
    QString     szTemp;
    QLabel      *lblBox;

    // Progenitore del Controllo
    myParent = parent;
    // Creazione del Layout principale del Form
    externalLayOut = new QVBoxLayout(this);             // Lay-Out Esterno del Frame
    mainGrid = new QGridLayout();
    mainGrid->setHorizontalSpacing(0);
    // Init valori Porte e flags
    m_nPort = -1;
    m_nNodeId = -1;
    m_nCurrentCTRow = -1;
    m_fUpdated = false;
    m_nRootPriority = nPriorityNone;
    m_nAbsPos = 0;
    m_nTotalRows = lstMPNE_Vars.count();             // Numero di Variabili da gestire
    m_nShowMode = showAll;
    lblModuleLeft = 0;
    lblModuleRight = 0;
    // Lista degli Sfondi e dei Nomi associati ai Moduli
    lstModuleName.clear();
    lstSfondi.clear();
    lstModuleCode.clear();
    lstModuleFunction.clear();      // Lista delle Funzionalità dei Moduli
    lstModuleLines.clear();         // Lista delle Linee abilitate per i Moduli
    // Descrizioni e sfondi dei Moduli
    // Module 0 - No Module
    lstModuleName.append(QString::fromAscii("No Module"));
    lstSfondi.append(szFileMPNE00);
    lstModuleFunction.append(nUsageNone);
    lstModuleLines.append(0);
    // Module 01 Dig IN
    lstModuleName.append(QString::fromAscii("8 Digital Input"));
    lstSfondi.append(szFileMPNE01);
    lstModuleFunction.append(nUsageDigIn);
    lstModuleLines.append(8);
    // Module 02 Dig OUT
    lstModuleName.append(QString::fromAscii("8 Digital Output"));
    lstSfondi.append(szFileMPNE02);
    lstModuleFunction.append(nUsageDigOut);
    lstModuleLines.append(8);
    // Module 03 Dig OUT with 4 Relays
    lstModuleName.append(QString::fromAscii("4 Digital Relays"));
    lstSfondi.append(szFileMPNE03);
    lstModuleFunction.append(nUsageDigOut);
    lstModuleLines.append(4);
    // Module 04 Dig OUT with 8 Relays
    lstModuleName.append(QString::fromAscii("8 Digital Relays"));
    lstSfondi.append(szFileMPNE04);
    lstModuleFunction.append(nUsageAnIO);
    lstModuleLines.append(3);
    // Module 05 2 AI + 1 AO
    lstModuleName.append(QString::fromAscii("2 AI - 1 AO"));
    lstSfondi.append(szFileMPNE05);
    lstModuleFunction.append(nUsageDigOut);
    lstModuleLines.append(8);
    //---------------------------
    // nRowSelector
    //---------------------------
    // Label per Combo Selettore
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  min-width: 50px;\n"));
    szTemp.append(QString::fromAscii("  max-width: 50px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("MPNE10:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nColBase);
    // Combo Selettore mpnc
    cboSelector = new QComboBox(this);
    cboSelector->setMaximumWidth(nComboWidth);
    mainGrid->addWidget(cboSelector, nRowSelector, nColLeft, 1, nItemWidth);
    // Label per Protocollo
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Protocol:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nColRight);
    lblProtocol = new QLabel(this);
    lblProtocol->setText(szEMPTY);
    lblProtocol->setStyleSheet(szTemp);
    mainGrid->addWidget(lblProtocol, nRowSelector, nColRight + 1);
    // Combo per Porta
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Port:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nColPort);
    QString szPortToolTip = QString::fromAscii("Change Serial Port");
    cboPort = new QComboBox(this);
    cboPort->setToolTip(szPortToolTip);
    for (i = 0; i <= nMaxSerialPorts; i++)  {
        cboPort->addItem(QString::number(i));
    }
    mainGrid->addWidget(cboPort, nRowSelector, nColPort + 1);
    // TextBox per Node ID
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Node Id:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nColNode);
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLineEdit { \n"));
    szTemp.append(QString::fromAscii("  min-width: 60px;\n"));
    szTemp.append(QString::fromAscii("  max-width: 60px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    txtNode = new QLineEdit(this);
    txtNode->setStyleSheet(szTemp);
    QString szNodeToolTip = QString::fromAscii("Change Node ID");
    txtNode->setToolTip(szNodeToolTip);
    txtNode->setValidator(new QIntValidator(0, nMaxNodeID, this));
    mainGrid->addWidget(txtNode, nRowSelector, nColNode + 1);
    // Molla Horizontal Spacer per allineare finestra
    QSpacerItem *colSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    colSpacer->setAlignment(Qt::AlignHCenter);
    mainGrid->addItem(colSpacer, nRowSelector, nColLast);
    //---------------------------
    // nRowTags
    //---------------------------
    // Bottone per Rename rows
    szTemp.clear();;
    szTemp.append(QString::fromAscii("QPushButton:disabled { \n"));
    szTemp.append(QString::fromAscii("  border: 0px ;\n"));
    szTemp.append(QString::fromAscii("  background-color: transparent;\n"));
    szTemp.append(QString::fromAscii("  background-image: url("");\n"));
    szTemp.append(QString::fromAscii("}\n"));
    szTemp.append(QString::fromAscii("QPushButton:enabled { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("}\n"));
    szTemp.append(QString::fromAscii("QPushButton:selected, QPushButton:hover {\n"));
    szTemp.append(QString::fromAscii("  border: 1px solid DarkOrange ;\n"));
    szTemp.append(QString::fromAscii("}\n"));
    szTemp.append(QString::fromAscii("QPushButton:pressed { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid red;\n"));
    szTemp.append(QString::fromAscii("}\n"));
    szTemp.append(QString::fromAscii("QPushButton { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szTemp.append(QString::fromAscii("  border-radius: 4px;\n"));
    szTemp.append(QString::fromAscii("  min-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  max-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  background-position: center  center;\n"));
    szTemp.append(QString::fromAscii("  background-color: transparent;\n"));
    szTemp.append(QString::fromAscii("  background-image: url(%1);\n")  .arg(szFileRename));
    szTemp.append(QString::fromAscii("}"));
    QString szRenameToolTip = QString::fromAscii("Rename Variables");
    cmdRename = new QPushButton(this);
    cmdRename->setEnabled(true);
    cmdRename->setFlat(true);
    cmdRename->setToolTip(szRenameToolTip);
    cmdRename->setStyleSheet(szTemp);
    mainGrid->addWidget(cmdRename, nRowTags, nColBase);
    // Nomi dei Moduli SX e DX
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignBottom | AlignLeft';\n"));
    szTemp.append(QString::fromAscii("  font-size: 18px;\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Exp1:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowTags, nColLeft, 1, nItemWidth);
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Exp2:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowTags, nColRight, 1, nItemWidth);
    //---------------------------
    // nRowCombo
    //---------------------------
    // Combo per Codici Modulo
    cboLeft = new QComboBox(this);
    cboRight = new QComboBox(this);
    for (i= 0; i < nTotalModules; i++)  {
        szTemp = int2PaddedString(i, 2, 10);
        lstModuleCode.append(szTemp);
        cboLeft->addItem(lstModuleName[i], szTemp);
        cboRight->addItem(lstModuleName[i], szTemp);
    }
    cboLeft->setMaximumWidth(nComboWidth);
    cboRight->setMaximumWidth(nComboWidth);
    mainGrid->addWidget(cboLeft, nRowCombo, nColLeft, 1, nItemWidth);
    mainGrid->addWidget(cboRight, nRowCombo, nColRight, 1, nItemWidth);
    //---------------------------
    // nRowButtons
    //---------------------------
    // Bottone per Filtro su Testa Nodi
    szFilterStyle.clear();
    szFilterStyle.append(QString::fromAscii("QPushButton:enabled { \n"));
    szFilterStyle.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szFilterStyle.append(QString::fromAscii("}\n"));
    szFilterStyle.append(QString::fromAscii("QPushButton:selected, QPushButton:hover {\n"));
    szFilterStyle.append(QString::fromAscii("  border: 1px solid DarkOrange ;\n"));
    szFilterStyle.append(QString::fromAscii("}\n"));
    szFilterStyle.append(QString::fromAscii("QPushButton:pressed { \n"));
    szFilterStyle.append(QString::fromAscii("  border: 1px solid red;\n"));
    szFilterStyle.append(QString::fromAscii("}\n"));
    szFilterStyle.append(QString::fromAscii("QPushButton { \n"));
    szFilterStyle.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szFilterStyle.append(QString::fromAscii("  border-radius: 4px;\n"));
    szFilterStyle.append(QString::fromAscii("  min-width: 40px;\n"));
    szFilterStyle.append(QString::fromAscii("  max-width: 40px;\n"));
    szFilterStyle.append(QString::fromAscii("  min-height: 36px;\n"));
    szFilterStyle.append(QString::fromAscii("  max-height: 36px;\n"));
    szFilterStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szFilterStyle.append(QString::fromAscii("  background-color: transparent;\n"));
    szFilterStyle.append(QString::fromAscii("}"));
    QString szSwitchToolTip = QString::fromAscii("Head Only / Head + Used / All");
    cmdFilter = new QPushButton(this);
    cmdFilter->setEnabled(true);
    cmdFilter->setFlat(true);
    cmdFilter->setToolTip(szSwitchToolTip);
    cmdFilter->setStyleSheet(szFilterStyle);
    mainGrid->addWidget(cmdFilter, nRowButtons, nColBase);
    // Frames di sfondo MPNE001
    szFrameStyle.clear();
    szFrameStyle.append(QString::fromAscii("QFrame { \n"));
//    szFrameStyle.append(QString::fromAscii("  border: 1px solid blue;\n"));
//    szFrameStyle.append(QString::fromAscii("  border-radius: 4px;\n"));
    szFrameStyle.append(QString::fromAscii("  min-width: 100px;\n"));
    szFrameStyle.append(QString::fromAscii("  max-width: 100px;\n"));
    szFrameStyle.append(QString::fromAscii("  min-height: 140px;\n"));
    szFrameStyle.append(QString::fromAscii("  max-height: 140px;\n"));
    szFrameStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szFrameStyle.append(QString::fromAscii("  background-color: transparent;\n"));
    // Frame Left
    fraMPNE_Left = new QFrame(this);
    szTemp = szFrameStyle;
    szTemp.append(QString::fromAscii("  background-image: url(%1);\n")  .arg(szFileMPNE10L));
    szTemp.append(QString::fromAscii("}"));
    fraMPNE_Left->setStyleSheet(szTemp);
    mainGrid->addWidget(fraMPNE_Left, nRowButtons, nColLeft);
    fraMPNE_Left->installEventFilter(this);
    // Frame Right
    fraMPNE_Right = new QFrame(this);
    szTemp = szFrameStyle;
    szTemp.append(QString::fromAscii("  background-image: url(%1);\n")  .arg(szFileMPNE10R));
    szTemp.append(QString::fromAscii("}"));
    fraMPNE_Right->setStyleSheet(szTemp);
    mainGrid->addWidget(fraMPNE_Right, nRowButtons, nColRight);
    fraMPNE_Right->installEventFilter(this);
    // Style per i due bottoni
    szModuleStyle.clear();
    szModuleStyle.append(QString::fromAscii("QLabel:disabled { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 0px ;\n"));
    szModuleStyle.append(QString::fromAscii("    background-color: transparent;\n"));
    szModuleStyle.append(QString::fromAscii("    background-image: url("");\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QLabel:enabled { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid navy;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QLabel:selected, QLabel:hover {\n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid DarkOrange ;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QLabel { \n"));
    szModuleStyle.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szModuleStyle.append(QString::fromAscii("  border-radius: 2px;\n"));
    szModuleStyle.append(QString::fromAscii("  min-width: 60px;\n"));
    szModuleStyle.append(QString::fromAscii("  max-width: 60px;\n"));
    szModuleStyle.append(QString::fromAscii("  min-height: 96px;\n"));
    szModuleStyle.append(QString::fromAscii("  max-height: 86px;\n"));
    szModuleStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szModuleStyle.append(QString::fromAscii("}"));
    // Bottone SX
    lblModuleLeft = new QLabel(fraMPNE_Left);
    lblModuleLeft->setStyleSheet(szModuleStyle);
    lblModuleLeft->setGeometry(40, 15, 60, 96);
    lblModuleLeft->setEnabled(false);
    // Bottone DX
    lblModuleRight = new QLabel(fraMPNE_Right);
    lblModuleRight->setStyleSheet(szModuleStyle);
    lblModuleRight->setGeometry(0, 15, 60, 96);
    lblModuleRight->setEnabled(false);
    lblModuleLeft->installEventFilter(this);
    lblModuleRight->installEventFilter(this);
    //---------------------------
    // nRowFlags
    //---------------------------
    // Label per Codice Base
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("  min-height: 24px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 24px;\n"));
    szTemp.append(QString::fromAscii("  min-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  max-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  font-size: 18px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox ->setText(QString::fromAscii("01"));
    lblBox ->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowFlags, nColBase, 1, 1, Qt::AlignHCenter);
    // Label per Codice SX
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("  min-height: 22px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 22px;\n"));
    szTemp.append(QString::fromAscii("  min-width: %1px;\n") .arg(nComboWidth));
    szTemp.append(QString::fromAscii("  max-width: %1px;\n") .arg(nComboWidth));
    szTemp.append(QString::fromAscii("  font-size: 18px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));

    lblLeft = new QLabel(this);
    lblLeft->setText(QString(szEMPTY));
    lblLeft->setStyleSheet(szTemp);
    mainGrid->addWidget(lblLeft, nRowFlags, nColLeft, 1, nItemWidth);
    // Label per Codice DX
    lblRight = new QLabel(this);
    lblRight->setText(QString(szEMPTY));
    lblRight->setStyleSheet(szTemp);
    mainGrid->addWidget(lblRight, nRowFlags, nColRight, 1, nItemWidth);
    // Molla Horizontal Spacer per allineare finestra
    QSpacerItem *hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hSpacer->setAlignment(Qt::AlignHCenter);
    mainGrid->addItem(hSpacer, nRowCombo, colPort, 3, nColLast - nColPort + 2);
    // Grid per Editing
    tblCT = new QTableWidget(this);
    externalLayOut->addLayout(mainGrid);
    externalLayOut->addWidget(tblCT);
    //-------------------------------------
    // Collegamento Slot vari, etc
    //-------------------------------------
    connect(cboLeft, SIGNAL(currentIndexChanged(int)), this, SLOT(onLeftModuleChanged(int)));
    connect(cboRight, SIGNAL(currentIndexChanged(int)), this, SLOT(onRightModuleChanged(int)));
    // Combo per cambio Modulo MPNC
    connect(cboSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(changeRootElement(int)));
    // Bottone Switch Mode
    connect(cmdFilter, SIGNAL(clicked()), this, SLOT(changeFilter()));
    // Bottoni SX e DC
//    connect(lblModuleLeft, SIGNAL(clicked()), this, SLOT(onLeftModuleClicked()));
//    connect(lblModuleRight, SIGNAL(clicked()), this, SLOT(onRightModuleClicked()));
    // Init variabili di gestione
    m_nTesta = -1;
    lstCapofila.clear();
    m_nBaseRow = -1;
    m_fCanRenameVars = false;
    m_nMinVarName = 0;
    m_nMaxVarName = 0;
    // Flag Abilitazione dei Moduli (il modulo 0 è MPNC006)
    lstModuleUsage.clear();
    for (i = 0; i < nTotalItems; i++)  {
        lstModuleUsage.append(nUsageNone);
    }
    // Filter
    m_nShowMode = showAll;
    setFilterButton(m_nShowMode);
    updateModule(nModuleLeft, 0);
    updateModule(nModuleRight, 0);
}
int     Config_MPNE::getCurrentRow()
// Restituisce la riga correntemente selezionata
{
    return m_nCurrentCTRow;
}

bool    Config_MPNE::isUpdated()
// Ritorna vero se il contenuto dei nodi è stato modificato
{
    return m_fUpdated;
}

void Config_MPNE::showTestaNodi(int nTesta, QList<int> lstMPNE, int nCurRow)
{
    int         nCur = 0;
    QString     szTemp;
    int         nBaseRow = -1;

    // Abilitazione delle entry nella Combo delle Porte
    int nPorts = enableSerialPortCombo(cboPort);
    // Blocca la combo Selettore delle Teste
    // Ricarica i valori perchè potrebbero essere cambiati da giro precedente
    disableAndBlockSignals(cboSelector);
    cboSelector->clear();
    lstCapofila.clear();
    m_fUpdated = false;
    m_nCurrentCTRow = nCurRow;          // Riporta il Numero riga corrente ottenuto da Grid Principale CT
    m_nShowMode = showAll;
    txtNode->setModified(false);
    qDebug() << QString::fromAscii("showTestaNodi(): NTesta: %1 - Teste Totali: %2 - Riga Corrente: %3") .arg(nTesta) .arg(lstMPNE.count()) .arg(nCurRow);
    if (nTesta < 0 || nTesta >= lstMPNE.count())  {
        for (nCur = 0; nCur < nTotalItems; nCur++)  {
            lstModuleUsage[nCur] = nUsageNone;
        }
        nTesta = -1;
        nBaseRow = -1;
    }
    else  {
        lstCapofila = lstMPNE;
        for (nCur = 0; nCur < lstCapofila.count(); nCur++)  {
            szTemp = QString::fromAscii("%1 - Row: %2") .arg(nCur + 1) .arg(lstCapofila[nCur] + 1);
            cboSelector->addItem(szTemp);
            if (nCur == nTesta)  {
                cboSelector->setCurrentIndex(nCur);
            }
        }
        // Leggi le caratteristiche dei moduli presenti per la testa nTesta
        nBaseRow = lstCapofila[nTesta];
    }
    // Aggiorna Le Icone dei Bottoni
    enableAndUnlockSignals(cboSelector);
    m_nTesta = nTesta;
    m_nBaseRow = nBaseRow;
    // Visualizza i dati denna testa n-Esima
    if (m_nTesta >= 0 && m_nTesta < cboSelector->count())  {
        changeRootElement(m_nTesta);
    }
    cboPort->setEnabled(nPorts > 1);
}
void    Config_MPNE::onLeftModuleChanged(int nIndex)
{
    updateModule(nModuleLeft, nIndex);
}
void    Config_MPNE::onRightModuleChanged(int nIndex)
{
    updateModule(nModuleRight, nIndex);
}

void    Config_MPNE::updateModule(int nPosition, int nFunction)
{
    QLabel      *lblCode = 0;
    QLabel      *lblModule = 0;
    QString     szNewStyle = szModuleStyle.left(szModuleStyle.length() - 1);
    int         nOldFunc = lstModuleUsage[nPosition];

    // Selezione dei corretti elementi di interfaccia
    switch  (nPosition)   {
        case nModuleLeft:
            lblCode = lblLeft;
            lblModule = lblModuleLeft;
            break;
        case nModuleRight:
            lblCode = lblRight;
            lblModule = lblModuleRight;
            break;
        case nModuleBase:
        default:
            lblCode = 0;
            lblModule = 0;
            break;
    }
    // Aggiornamento della Label Codice
    if (lblCode != 0 && nFunction >= 0 &&  nFunction < lstModuleCode.count())  {
        lblCode->setText(lstModuleCode[nFunction]);
    }
    // Aggiornamento dello Sfondo Bottone
    if (lblModule != 0)  {
        if (nOldFunc != nFunction)  {
            // Marca come non utilizzata la precedente funzione nelle variabili specifiche
            // Marca come utilizzata la nuova funzione nelle variabili specifiche
            // Aggiorna nuova funzione
            lstModuleUsage[nPosition] = nFunction;
        }
        if (nFunction == 0)  {
            lblModule->setEnabled(false);
            lblModule->lower();
        }
        else {
            QString szBackGround = QString::fromAscii("    background-image: url(%1);\n")  .arg(lstSfondi[nFunction]);
            szBackGround.append(QString::fromAscii("    background-color: transparent;\n"));
            szNewStyle.append(szBackGround);
            szNewStyle.append(QString::fromAscii("}"));
            lblModule->setStyleSheet(szNewStyle);
            lblModule->setEnabled(true);
            lblModule->raise();
        }
    }

}
void    Config_MPNE::changeFilter()
// Cambio del filtro sui moduli
{
    int nNewMode = (++m_nShowMode) % showTotals;

    m_nShowMode = nNewMode;
    setFilterButton(m_nShowMode);
    filterVariables(0, 0);
}
void    Config_MPNE::setFilterButton(int nNewMode)
// Imposta il fondo del botton cmd
{
    QString szNewStyle = szFilterStyle.left(szFilterStyle.length() - 1);
    QString szBackGround;
    switch (nNewMode)  {
        case showHead:
            // Visualizza solo elementi della Head (no Nodi)
            szBackGround = QString::fromAscii("    background-image: url(%1);\n")  .arg(szFilterHead);
            szBackGround.append(QString::fromAscii("    background-color: transparent;\n"));
            break;

        case showUsed:
            // Visualizza solo le variabili utilizzate
            szBackGround = QString::fromAscii("    background-image: url(%1);\n")  .arg(szFilterUsed);
            szBackGround.append(QString::fromAscii("    background-color: transparent;\n"));
            break;

        case showAll:
        default:
            nNewMode = showAll;
            szBackGround = QString::fromAscii("    background-image: url(%1);\n")  .arg(szFilterAll);
            szBackGround.append(QString::fromAscii("    background-color: transparent;\n"));
    }
    // Switch Mode
    szNewStyle.append(szBackGround);
    szNewStyle.append(QString::fromAscii("}"));
    cmdFilter->setStyleSheet(szNewStyle);
}
void    Config_MPNE::changeRootElement(int nItem)
// Cambio di Item della Combo dei MPNC definiti
{
    int     nGroup = 0;
    int     nFunction = 0;
    m_nTesta = -1;
    if (nItem >= 0 && nItem < lstCapofila.count())  {
        // Reperimento riga di base del Modulo selezionato
        m_nBaseRow = lstCapofila[nItem];
        m_nTesta = nItem;
        m_nShowMode = showAll;
        setFilterButton(m_nShowMode);
        // Determina il Protocollo, la Porta e il Nodo dell'elemento
        if (m_nBaseRow >=  0 && m_nBaseRow < localCTRecords.count() - m_nTotalRows)  {
            disableAndBlockSignals(cboPort);
            disableAndBlockSignals(txtNode);
            // Visualizzazione Protocollo
            int nProtocol = localCTRecords[m_nBaseRow].Protocol;
            if (nProtocol >= 0 && nProtocol < lstProtocol.count())
                lblProtocol->setText(lstProtocol[nProtocol]);
            else
                lblProtocol->setText(szEMPTY);
            // Scelta Porta Seriale
            int nPort = localCTRecords[m_nBaseRow].Port;
            m_nPort = -1;
            if (nPort >= 0 && nPort <= nMaxSerialPorts)  {
                m_nPort = nPort;
            }
            cboPort->setCurrentIndex(m_nPort);
            // Node Id
            int nNode = localCTRecords[m_nBaseRow].NodeId;
            txtNode->setText(QString::number(nNode));
            txtNode->setModified(false);
            m_nNodeId = nNode;
            // Priorità del Gruppo
            m_nRootPriority = localCTRecords[m_nBaseRow].Enable;
            // Abilitazione Rename delle Variabili
            cmdRename->setEnabled(canRenameRows(m_nBaseRow));
            enableAndUnlockSignals(cboPort);
            enableAndUnlockSignals(txtNode);
            // Aggiornamento lista moduli
            getUsedModules(m_nBaseRow);
            // Riga corrente non compresa nella definizione del modulo
            if (m_nCurrentCTRow < m_nBaseRow || m_nCurrentCTRow > m_nBaseRow + m_nTotalRows)  {
                qDebug() << QString::fromAscii("changeRootElement(): Forced current Row[%1] To Base Element [%2]") .arg(m_nCurrentCTRow) .arg(m_nBaseRow);
                m_nCurrentCTRow = m_nBaseRow;
            }
            // Abilitazione interfaccia
            customizeButtons();
            filterVariables(nGroup, nFunction);
        }
    }
    else  {
        qDebug() << QString::fromAscii("changeRootElement(): Attempt to switch to wrong element: %1") .arg(nItem);
    }

}
bool    Config_MPNE::eventFilter(QObject *obj, QEvent *event)
// Gestore Event Handler
{
    int nModule = 0;
    int nFunction = 0;

    // Gestione dell'evento Rilascio Mouse per Frame - Bottoni SX e DX
    if (event->type() == QEvent::MouseButtonRelease)  {
        // Rilasciato mouse su aree visibili del Frame di base
        if (obj == fraMPNE_Left || obj == fraMPNE_Right)  {
                nModule = nModuleBase;
                nFunction = nUsageNone;
                qDebug() << QString::fromAscii("Mouse Release on Frame - Function: %1") .arg(nFunction);
                filterVariables(nModule, nFunction);
                return true;
        }
        // Rilasciato mouse su Bottone SX
        else if (obj== lblModuleLeft)  {
            nModule = nModuleLeft;
            nFunction = cboLeft->currentIndex();
            // Modulo non usato, filtra su base
            if (nFunction == nUsageNone)  {
                nModule = nModuleBase;
            }
            filterVariables(nModule, nFunction);
            qDebug() << QString::fromAscii("Mouse Release on Left Button - Function: %1") .arg(nFunction);
            return true;
        }
        // Rilasciato mouse su Bottone DX
        else if (obj== lblModuleRight)  {
            nModule = nModuleRight;
            nFunction = cboRight->currentIndex();
            // Modulo non usato, filtra su base
            if (nFunction == nUsageNone)  {
                nModule = nModuleBase;
            }
            filterVariables(nModule, nFunction);
            qDebug() << QString::fromAscii("Mouse Release on Right Button - Function: %1") .arg(nFunction);
            return true;
        }
    }
    // Pass event to standard Event Handler
    return QObject::eventFilter(obj, event);
}
void    Config_MPNE::getUsedModules(int nBaseRow)
// Legge a partire dalla riga del Capofila il numero di Moduli utilizzati
{
    int     nGroup = 0;
    int     nFunction = 0;
    int     nRow = 0;
    bool    fUsed = false;

    // qDebug() << QString::fromAscii("getUsedModules(): Main Head Row: %1") .arg(nBaseRow);
    // Reset dell'Array dei Moduli
    for (nGroup = 0; nGroup < nTotalItems; nGroup ++)  {
        lstModuleUsage[nGroup] = nUsageNone;
    }
    // Controlla che la Riga di Base Modulo MPNC non sia fuori Range
    if (nBaseRow > 0 &&  nBaseRow < (localCTRecords.count() - m_nTotalRows))  {
        // Ricerca della prima funzionalità definita per il Modulo (SX o DX)
        for (nGroup = nModuleLeft; nGroup < nTotalItems; nGroup ++)  {
            nFunction = nUsageNone;
            // Ricerca della prima riga utilizzata del Modulo
            for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
                // La riga appartiene al modulo che stiamo cercando
                if (nGroup == lstMPNC006_Vars[nRow].Group)  {
                    fUsed = localCTRecords[nBaseRow + nRow].UsedEntry && localCTRecords[nBaseRow + nRow].Enable > nPriorityNone;
                    // Se la riga è utilizzata prende per buona la Funzione dal modello
                    if (fUsed)  {
                        nFunction = lstMPNC006_Vars[nRow].Module;
                    }
                    break;
                }
            }
            lstModuleUsage[nGroup] = nFunction;
            qDebug() << QString::fromAscii("getUsedModules(): Module: %1 Function: %2") .arg(nGroup) .arg(nFunction);
        }
    }
    else  {
        m_szMsg = QString::fromAscii("The Selected Module is Out of Range!");
        notifyUser(this, szMectTitle, m_szMsg);
    }
}
bool    Config_MPNE::canRenameRows(int nBaseRow)
// Verifica se tutto il Device può essere rinominato
{
    QString     szVarName(szEMPTY);
    QStringList lstVarNames;
    bool        fCanRename = true;
    int         nRow = 0;
    int         nCol = 0;

    if (m_nBaseRow >=  0 && m_nBaseRow < localCTRecords.count() - m_nTotalRows)  {
        lstVarNames.clear();
        m_nMinVarName = MAX_IDNAME_LEN;
        m_nMaxVarName = 0;
        m_szVarNamePrefix.clear();
        // Determinazione Minima e Massima lunghezza del nome Variabile
        for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
            szVarName = QString::fromAscii(localCTRecords[nBaseRow + nRow].Tag);
            int nLen = szVarName.trimmed().length();
            if (nLen > 0)  {
                if (nLen > m_nMaxVarName) m_nMaxVarName = nLen;
                if (nLen < m_nMinVarName) m_nMinVarName = nLen;
                lstVarNames.append(szVarName);
            }
        }
        // Ordinamento della lista di variabili
        lstVarNames.sort();
        QString szTemp;
        bool    isEqual = true;
        // Ciclo sulle Variabili ordinate
        for (nCol = 1; nCol <= m_nMinVarName; nCol++)  {
            // Take part of name from first variable
            szTemp = lstVarNames[0].left(nCol);
            for (nRow = 1; nRow < lstVarNames.count(); nRow++)  {
                if (! lstVarNames.at(nRow).startsWith(szTemp))  {
                    isEqual = false;
                    break;
                }
            }
            // Condizione di terminazione della scansione dei nomi
            if (! isEqual)  {
                m_szVarNamePrefix = szTemp.left(nCol - 1);
                break;
            }
        }
        // Condizione di Rename: Suffisso presente tra i nomi di Variabili
        fCanRename = m_szVarNamePrefix.length() > 0;
    }
    else  {
        fCanRename = false;
    }
    qDebug() << QString::fromAscii("canRenameRows(): Variables Prefix: [%1] Min Var: [%2] Max Var: [%3]") .arg(m_szVarNamePrefix) .arg(m_nMinVarName) .arg(m_nMaxVarName);
    // Return Value
    return fCanRename;
}
void    Config_MPNE::customizeButtons()
// Abilitazione delle icone Bottoni in funzione della presenza dei moduli (Lettura della configurazione attuale Variabili-->Interfaccia)
{
    int     nGroup = nModuleLeft;
    if (lstModuleFunction[nGroup] >= 0 && lstModuleFunction[nGroup] < cboLeft->count())  {
        cboLeft->setCurrentIndex(lstModuleFunction[nGroup]);
    }
    nGroup = nModuleRight;
    if (lstModuleFunction[nGroup] >= 0 && lstModuleFunction[nGroup] < cboRight->count())  {
        cboRight->setCurrentIndex(lstModuleFunction[nGroup]);
    }
}
void    Config_MPNE::filterVariables(int nPosition, int nFunction)
// Filtra le variabili specifiche del modulo identificato da Posizione e Funzione
{
    QStringList         lstLineValues;
    QList<QStringList > lstTableRows;
    QList<int16_t>      lstRowPriority;
    int                 nRow = 0;
    int                 nCurrentRow = 0;
    bool                fRes = false;

    qDebug() << QString::fromAscii("filterVariables(): Filter Variables for Position: %1 Function: %2") .arg(nPosition) .arg(nFunction);
    // Preparazione tabella
    this->setCursor(Qt::WaitCursor);
    lstLineValues.clear();
    lstTableRows.clear();
    lstRowPriority.clear();
    tblCT->setVisible(false);
    tblCT->setEnabled(false);
    tblCT->clearSelection();
    tblCT->setRowCount(0);
    tblCT->setColumnCount(0);
    tblCT->clearContents();
    tblCT->clear();
    // Ciclo di Lettura
    for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
        if ( (nPosition == 0 && nFunction == 0)  ||
             (nPosition == lstMPNE_Vars[nRow].Group && nFunction == lstMPNE_Vars[nRow].Module))  {
            bool    fShow = true;
            if (nPosition == 0 && nFunction == 0)  {
                switch (m_nShowMode)  {
                    case showHead:
                        // Visualizza solo elementi della Head (no Nodi)
                        fShow = (nPosition == lstMPNE_Vars[nRow].Group && nFunction == lstMPNE_Vars[nRow].Module);
                        break;

                    case showUsed:
                        // Visualizza solo le variabili utilizzate
                        fShow = localCTRecords[m_nBaseRow + nRow].Enable > nPriorityNone;

                    break;
                    case showAll:
                    default:
                        fShow = true;
                        break;
                }
            }
            // La riga deve essere visualizzata
            if (fShow)  {
                // Decodifica dei valori di CT e conversione in stringa
                fRes = recCT2MPNxFieldsValues(localCTRecords, lstLineValues, nRow + m_nBaseRow, lstMPNE_Vars, nRow);
                // Aggiunta alla Table
                if (fRes)  {
                    lstTableRows.append(lstLineValues);
                    lstRowPriority.append(localCTRecords[nRow + m_nBaseRow].Enable);
                    // Riga da selezionare in Grid
                    if (nRow + m_nBaseRow == m_nCurrentCTRow)  {
                        nCurrentRow = lstTableRows.count() - 1;
                    }
                }
            }
        }
    }
    // Dimensionamento Tabella
    tblCT->setRowCount(lstTableRows.count());
    tblCT->setColumnCount(colMPNxTotals);
    // Caricamento in Tabella
    for (nRow = 0; nRow < lstTableRows.count(); nRow++)  {
        list2GridRow(tblCT, lstTableRows[nRow], lstMPNxHeadLeftCols, nRow);
    }
    setGridParams(tblCT, lstMPNxCols, lstMNPxHeadSizes, QAbstractItemView::SingleSelection);
    // Colore di Sfondo
    for (nRow = 0; nRow < lstTableRows.count(); nRow++)  {
        setRowColor(tblCT, nRow, 0, 1, lstRowPriority[nRow], m_nBaseRow);
    }
    // Ci sono elementi in griglia
    if (lstTableRows.count() > 0)  {
        tblCT->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblCT->setSelectionMode(QAbstractItemView::SingleSelection);
        tblCT->setVisible(true);
        tblCT->setEnabled(true);
        if (nCurrentRow < 0 || nCurrentRow >= lstTableRows.count())  {
            qDebug() << QString::fromAscii("filterVariables(): Table Row: %1 Forced to 0") .arg(nCurrentRow);
            nCurrentRow = 0;
        }
        tblCT->selectRow(nCurrentRow);
        tblCT->update();
        tblCT->setFocus();
    }
    qDebug() << QString::fromAscii("filterVariables(): Displayed Rows: %1 - Current Relative: %2 - CT Row: %3 - Show Mode: %4")
                .arg(nRow) .arg(nCurrentRow) .arg(m_nCurrentCTRow) .arg(m_nShowMode);
    this->setCursor(Qt::ArrowCursor);
}
