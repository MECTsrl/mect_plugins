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
    cmdLeft = 0;
    cmdRight = 0;
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
    szButtonStyle.clear();
    szButtonStyle.append(QString::fromAscii("QPushButton:disabled { \n"));
    szButtonStyle.append(QString::fromAscii("    border: 0px ;\n"));
    szButtonStyle.append(QString::fromAscii("    background-color: transparent;\n"));
    szButtonStyle.append(QString::fromAscii("    background-image: url("");\n"));
    szButtonStyle.append(QString::fromAscii("}\n"));
    szButtonStyle.append(QString::fromAscii("QPushButton:enabled { \n"));
    szButtonStyle.append(QString::fromAscii("    border: 1px solid navy;\n"));
    szButtonStyle.append(QString::fromAscii("}\n"));
    szButtonStyle.append(QString::fromAscii("QPushButton:selected, QPushButton:hover {\n"));
    szButtonStyle.append(QString::fromAscii("    border: 1px solid DarkOrange ;\n"));
    szButtonStyle.append(QString::fromAscii("}\n"));
    szButtonStyle.append(QString::fromAscii("QPushButton:pressed { \n"));
    szButtonStyle.append(QString::fromAscii("    border: 1px solid red;\n"));
    szButtonStyle.append(QString::fromAscii("}\n"));
    szButtonStyle.append(QString::fromAscii("QPushButton { \n"));
    szButtonStyle.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szButtonStyle.append(QString::fromAscii("  border-radius: 2px;\n"));
    szButtonStyle.append(QString::fromAscii("  min-width: 60px;\n"));
    szButtonStyle.append(QString::fromAscii("  max-width: 60px;\n"));
    szButtonStyle.append(QString::fromAscii("  min-height: 96px;\n"));
    szButtonStyle.append(QString::fromAscii("  max-height: 86px;\n"));
    szButtonStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szButtonStyle.append(QString::fromAscii("}"));
    // Bottone SX
    cmdLeft = new QPushButton(fraMPNE_Left);
    cmdLeft->setStyleSheet(szButtonStyle);
    cmdLeft->setGeometry(40, 16, 60, 96);
    cmdLeft->setFlat(true);
    cmdLeft->setEnabled(false);
    // Bottone DX
    cmdRight = new QPushButton(fraMPNE_Right);
    cmdRight->setStyleSheet(szButtonStyle);
    cmdRight->setGeometry(0, 16, 60, 96);
    cmdRight->setFlat(true);
    cmdRight->setEnabled(true);
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
    // Init variabili di gestione
    m_nTesta = -1;
    lstCapofila.clear();
    m_nBaseRow = -1;
    m_fCanRenameVars = false;
    m_nMinVarName = 0;
    m_nMaxVarName = 0;
    // Flag Abilitazione dei Moduli (il modulo 0 è MPNC006)
    lstModuleIsPresent.clear();
    for (i = 0; i < nTotalItems; i++)  {
        lstModuleIsPresent.append(false);
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
            lstModuleIsPresent[nCur] = false;
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
    QPushButton *btnModule = 0;
    QString     szNewStyle = szButtonStyle.left(szButtonStyle.length() - 1);

    // Selezione dei corretti elementi di interfaccia
    switch  (nPosition)   {
        case nModuleLeft:
            lblCode = lblLeft;
            btnModule = cmdLeft;
            break;
        case nModuleRight:
            lblCode = lblRight;
            btnModule = cmdRight;
            break;
        case nModuleBase:
        default:
            lblCode = 0;
            btnModule = 0;
            break;
    }
    // Aggiornamento della Label Codice
    if (lblCode != 0 && nFunction >= 0 &&  nFunction < lstModuleCode.count())  {
        lblCode->setText(lstModuleCode[nFunction]);
    }
    // Aggiornamento dello Sfondo Bottone
    if (btnModule != 0)  {
        if (nFunction == 0)  {
            btnModule->setEnabled(false);
        }
        else {
            QString szBackGround = QString::fromAscii("    background-image: url(%1);\n")  .arg(lstSfondi[nFunction]);
            szBackGround.append(QString::fromAscii("    background-color: transparent;\n"));
            szNewStyle.append(szBackGround);
            szNewStyle.append(QString::fromAscii("}"));
            btnModule->setStyleSheet(szNewStyle);
            btnModule->setEnabled(true);
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
void    Config_MPNE::filterVariables(int nPosition, int nFunction)
// Filtra le variabili specifiche del modulo identificato da Posizione e Funzione
{

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

    // Rilasciato mouse su aree visibili del Frame di base
    if (obj == fraMPNE_Left || obj == fraMPNE_Right)  {
        if (event->type() == QEvent::MouseButtonRelease)  {
            nModule = nModuleBase;
            nFunction = nUsageNone;
            qDebug() << QString::fromAscii("Mouse Release on Frame - Function: %1") .arg(nFunction);
            return true;
        }
    }
    // Rilasciato mouse su Bottone SX
    else if (obj== cmdLeft)  {
        nModule = nModuleLeft;
        nFunction = cboLeft->currentIndex();
        qDebug() << QString::fromAscii("Mouse Release on Left Button - Function: %1") .arg(nFunction);
        return true;
    }
    // Rilasciato mouse su Bottone DX
    else if (obj== cmdRight)  {
        nModule = nModuleRight;
        nFunction = cboRight->currentIndex();
        qDebug() << QString::fromAscii("Mouse Release on Right Button - Function: %1") .arg(nFunction);
        return true;
    }
    // Pass event to standard Event Handler
    return QObject::eventFilter(obj, event);
}
void    Config_MPNE::getUsedModules(int nBaseRow)
// Legge a partire dalla riga del Capofila il numero di Moduli utilizzati
{
    int     nCur = 0;
    int     nGroup = 0;
    int     nModule = 0;
    int     nRow = 0;
    bool    fUsed = false;

    // qDebug() << QString::fromAscii("getUsedModules(): Main Head Row: %1") .arg(nBaseRow);
    // Reset dell'Array dei Moduli
    for (nCur = 0; nCur < nTotalItems; nCur ++)  {
        lstModuleIsPresent[nCur] = false;
    }
    // Controlla che la Riga di Base Modulo MPNC non sia fuori Range
    if (nBaseRow > 0 &&  nBaseRow < (localCTRecords.count() - m_nTotalRows))  {
        // Interpretare la configurazione dei nodi
        for (nCur = 0; nCur < nTotalItems; nCur ++)  {
            // Ricerca della prima riga definita per Gruppo e modulo
            for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
                // La riga appartiene al modulo che stiamo cercando
                if (nGroup == lstMPNC006_Vars[nRow].Group && nModule == lstMPNC006_Vars[nRow].Module)  {
                    fUsed = localCTRecords[nBaseRow + nRow].UsedEntry && localCTRecords[nBaseRow + nRow].Enable > nPriorityNone;
                    lstModuleIsPresent[nCur] = fUsed;
                    //  qDebug() << QString::fromAscii("getUsedModules(): Module: %1 is: %2") .arg(nCur) .arg(fUsed ? QString::fromAscii("YES") : QString::fromAscii("NO"));
                    break;
                }
            }
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
// Abilitazione delle icone Bottoni in funzione della presenza dei moduli
{

}
