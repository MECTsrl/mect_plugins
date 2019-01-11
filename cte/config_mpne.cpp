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
// Posizioni nel LayOut (Riga)
const int nRowSelector = 0;
const int nRowDesc = 1;
const int nRowButtons = 2;
const int nRowFlags = 3;
const int nRowGrid = 4;
// Posizione nel LayOut (Colonna)
const int nColBase = 0;
const int nColLeft = 1;
const int nColRight = 3;
const int nColProtocol = 4;
const int nColPort = 5;
const int nColNode = 7;
const int nColLast = 9;
const int nItemWidth = 2;
// Sfondi
const QString szFileRename = szPathIMG + QString::fromAscii("Rename2.png");
// Sfondi per Moduli
const QString szFileMPNE1001 = szPathIMG + QString::fromAscii("MPNE1001.png");
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
    mainGrid->setHorizontalSpacing(2);
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
    // Label per Combo Selettore
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("MPNE1001:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nColBase);
    // Combo Selettore mpnc
    cboSelector = new QComboBox(this);
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
    mainGrid->addWidget(cmdRename, nRowDesc, nColBase);
    // Lista degli Sfondi e dei Nomi associati ai Moduli
    lstModuleName.clear();
    lstSfondi.clear();
    lstModuleCode.clear();
    // Descrizioni e sfondi dei Moduli
    lstModuleName.append(QString::fromAscii("No Module"));
    lstSfondi.append(szFileMPNE00);
    lstModuleName.append(QString::fromAscii("8 Digital Input"));
    lstSfondi.append(szFileMPNE01);
    lstModuleName.append(QString::fromAscii("8 Digital Output"));
    lstSfondi.append(szFileMPNE02);
    lstModuleName.append(QString::fromAscii("4 Digital Relays"));
    lstSfondi.append(szFileMPNE03);
    lstModuleName.append(QString::fromAscii("8 Digital Relays"));
    lstSfondi.append(szFileMPNE04);
    lstModuleName.append(QString::fromAscii("2 Analog Input 1 Analog Output"));
    lstSfondi.append(szFileMPNE05);
    // Combo per Codici Modulo
    cboLeft = new QComboBox(this);
    cboRight = new QComboBox(this);
    for (i= 0; i < nTotalModules; i++)  {
        szTemp = int2PaddedString(i, 2, 10);
        lstModuleCode.append(szTemp);
        cboLeft->addItem(lstModuleName[i], szTemp);
        cboRight->addItem(lstModuleName[i], szTemp);
    }
    mainGrid->addWidget(cboLeft, nRowDesc, nColLeft, 1, nItemWidth);
    mainGrid->addWidget(cboRight, nRowDesc, nColRight, 1, nItemWidth);
    // Frame di sfondo MPNE001
    fraMPNE = new QFrame(this);
    szFrameStyle.clear();
    szFrameStyle.append(QString::fromAscii("QFrame { \n"));
    szFrameStyle.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szFrameStyle.append(QString::fromAscii("  border-radius: 4px;\n"));
    szFrameStyle.append(QString::fromAscii("  min-width: 111px;\n"));
    szFrameStyle.append(QString::fromAscii("  max-width: 111px;\n"));
    szFrameStyle.append(QString::fromAscii("  min-height: 140px;\n"));
    szFrameStyle.append(QString::fromAscii("  max-height: 140px;\n"));
    szFrameStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szFrameStyle.append(QString::fromAscii("  background-color: transparent;\n"));
    szFrameStyle.append(QString::fromAscii("  background-image: url(%1);\n")  .arg(szFileMPNE1001));
    szFrameStyle.append(QString::fromAscii("}"));
    fraMPNE->setStyleSheet(szFrameStyle);
    mainGrid->addWidget(fraMPNE, nRowButtons, nColLeft, 1, 2 * nItemWidth);
    fraMPNE->installEventFilter(this);
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
    szButtonStyle.append(QString::fromAscii("  min-width: 52px;\n"));
    szButtonStyle.append(QString::fromAscii("  max-width: 52px;\n"));
    szButtonStyle.append(QString::fromAscii("  min-height: 118px;\n"));
    szButtonStyle.append(QString::fromAscii("  max-height: 118px;\n"));
    szButtonStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szButtonStyle.append(QString::fromAscii("}"));
    // Bottone SX
    cmdLeft = new QPushButton(fraMPNE);
    cmdLeft->setStyleSheet(szButtonStyle);
    cmdLeft->setGeometry(2, 10, 52,118);
    cmdLeft->setFlat(true);
    cmdLeft->setEnabled(false);
    // Bottone DX
    cmdRight = new QPushButton(fraMPNE);
    cmdRight->setStyleSheet(szButtonStyle);
    cmdRight->setGeometry(57, 10, 52,118);
    cmdRight->setFlat(true);
    cmdRight->setEnabled(true);
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
    mainGrid->addWidget(cmdFilter, nRowFlags, nColBase);
    // Label per Codice SX
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  font-size: 18px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    lblLeft = new QLabel(this);
    lblLeft->setText(QString(szEMPTY));
    lblLeft->setStyleSheet(szTemp);
    mainGrid->addWidget(lblLeft, nRowFlags, nColLeft, 1, nItemWidth);
    // Label per Codice DX
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  font-size: 18px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    lblRight = new QLabel(this);
    lblRight->setText(QString(szEMPTY));
    lblRight->setStyleSheet(szTemp);
    mainGrid->addWidget(lblRight, nRowFlags, nColRight, 1, nItemWidth);

    // Molla Horizontal Spacer per allineare finestra
    QSpacerItem *hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hSpacer->setAlignment(Qt::AlignHCenter);
    mainGrid->addItem(hSpacer, nRowDesc, colPort, 3, nColLast - nColPort + 2);
    // Grid per Editing
    tblCT = new QTableWidget(this);
    externalLayOut->addLayout(mainGrid);
    externalLayOut->addWidget(tblCT);
    //-------------------------------------
    // Collegamento Slot vari, etc
    //-------------------------------------
    connect(cboLeft, SIGNAL(currentIndexChanged(int)), this, SLOT(onLeftModuleChanged(int)));
    connect(cboRight, SIGNAL(currentIndexChanged(int)), this, SLOT(onRightModuleChanged(int)));
    // Bottone Switch Mode
    connect(cmdFilter, SIGNAL(clicked()), this, SLOT(changeFilter()));
    // Init variabili di gestione
    m_nTesta = -1;
    lstCapofila.clear();
    m_nBaseRow = -1;
    m_fCanRenameVars = false;
    m_nMinVarName = 0;
    m_nMaxVarName = 0;
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

void Config_MPNE::showTestaNodi(int nTesta, QList<int> lstMPNE)
{
    if (nTesta < 0)  {

    }
    else {

    }
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
bool    Config_MPNE::eventFilter(QObject *obj, QEvent *event)
// Gestore Event Handler
{
    if (obj == fraMPNE)  {
        if (event->type() == QEvent::MouseButtonRelease)  {
            qDebug() << QString::fromAscii("Mouse Release on Frame");
            return true;
        }
    }
    // Pass event to standard Event Handler
    return QObject::eventFilter(obj, event);
}
