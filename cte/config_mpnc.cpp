#include "config_mpnc.h"
#include "utils.h"
#include "cteUtils.h"
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPixmap>
#include <QPalette>
#include <QColor>
#include <QIcon>
#include <QDebug>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFrame>
#include <QIntValidator>

const int nItemsPerGroup = 4;
const int nTotalGroups = 5;
const int nTotalItems = 17;
// Posizioni nel LayOut (Colonna o indice di Bottoni)
const int nBaseHead = 0;
const int nBaseAnIn = 1;
const int nBaseAnOut = 5;
const int nBaseDigIn = 9;
const int nBaseDigOut = 13;
// Posizioni nel LayOut (Riga)
const int nRowSelector = 0;
const int nRowDesc = 1;
const int nRowButtons = 2;
const int nRowFlags = 3;
const int nRowGrid = 4;
// Sfondi per Moduli
const QString szFileAdd = szPathIMG + QString::fromAscii("Add_32.png");
const QString szFileRemove = szPathIMG + QString::fromAscii("Remove_32.png");
const QString szFileRename = szPathIMG + QString::fromAscii("Rename2.png");
const QString szFileMPNC006 = szPathIMG + QString::fromAscii("MPNC006_R.png");
const QString szFileMPNC030 = szPathIMG + QString::fromAscii("MPNC030_R.png");
const QString szFileMPNC035 = szPathIMG + QString::fromAscii("MPNC030_R.png");
const QString szFileMPNC020_01 = szPathIMG + QString::fromAscii("MPNC020_R.png");
const QString szFileMPNC020_02 = szPathIMG + QString::fromAscii("MPNC020_R.png");
const QString szFilterHead = szPathIMG + QString::fromAscii("ShowHead.png");
const QString szFilterUsed = szPathIMG + QString::fromAscii("ShowUsed.png");
const QString szFilterAll = szPathIMG + QString::fromAscii("ShowAll.png");

Config_MPNC::Config_MPNC(QWidget *parent) :
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
    mapRemoveClicked = new QSignalMapper(this);         // Signal Mapper per Remove Module Clicked
    mapModuleClicked = new QSignalMapper(this);         // Signal Mapper per Module Clicked
    lstModuleName.clear();
    lstSfondi.clear();
    m_nPort = -1;
    m_nNodeId = -1;
    m_nCurrentCTRow = -1;
    m_fUpdated = false;
    m_nRootPriority = nPriorityNone;
    m_nAbsPos = 0;
    m_nTotalRows = lstMPNC006_Vars.count();             // Numero di Variabili da gestire
    m_nShowMode = showAll;
//    QFrame*         boxSeparator = 0;
    // Labels per Posizioni (A B C D)
    for (i = 0; i < nItemsPerGroup; i++)  {
        QChar chBase = QChar::fromAscii(65 + i);
        lstPosFlags.append(QString(1, chBase));
    }
    // Lista degli Sfondi e dei Nomi associati ai Moduli
    // MPNC006
    lstSfondi.append(szFileMPNC006);
    lstModuleName.append(QString::fromAscii("MPNC006"));
    // MPNC030
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC030);
    }
    lstModuleName.append(QString::fromAscii("MPNC030"));
    // MPNC035
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC035);
    }
    lstModuleName.append(QString::fromAscii("MPNC035"));
    // MPNC020_01
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC020_01);
    }
    lstModuleName.append(QString::fromAscii("MPNC020_01"));
    // MPNC020_02
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC020_02);
    }
    lstModuleName.append(QString::fromAscii("MPNC020_02"));
    // Flag Abilitazione dei Moduli (il modulo 0 è MPNC006)
    lstModuleIsPresent.clear();
    for (i = 0; i < nTotalItems; i++)  {
        lstModuleIsPresent.append(false);
    }
    // Label per Combo Selettore
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  qproperty-alignment: 'AlignVCenter | AlignHCenter';\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("MPNC006:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nBaseHead);
    // Combo Selettore mpnc
    cboSelector = new QComboBox(this);
    mainGrid->addWidget(cboSelector, nRowSelector, nBaseAnIn, 1, nItemsPerGroup);
    // Label per Protocollo
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Protocol:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nBaseAnOut, 1, 2);
    lblProtocol = new QLabel(this);
    lblProtocol->setText(szEMPTY);
    lblProtocol->setStyleSheet(szTemp);
    mainGrid->addWidget(lblProtocol, nRowSelector, nBaseAnOut + 2, 1, 2);
    // Combo per Porta
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Port:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nBaseDigIn, 1, 2);
    QString szPortToolTip = QString::fromAscii("Change Serial Port");
    cboPort = new QComboBox(this);
    cboPort->setToolTip(szPortToolTip);
    for (i = 0; i <= nMaxSerialPorts; i++)  {
        cboPort->addItem(QString::number(i));
    }
    mainGrid->addWidget(cboPort, nRowSelector, nBaseDigIn + 2, 1, 2);
    // TextBox per Node ID
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Node Id:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nBaseDigOut, 1, 2);
    QString szNodeToolTip = QString::fromAscii("Change Node ID");
    txtNode = new QLineEdit(this);
    txtNode->setToolTip(szNodeToolTip);
    txtNode->setStyleSheet(szTemp);
    txtNode->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    txtNode->setValidator(new QIntValidator(0, nMaxNodeID, this));
    mainGrid->addWidget(txtNode, nRowSelector, nBaseDigOut + 2, 1, 2);
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
    mainGrid->addWidget(cmdRename, nRowDesc, nBaseHead);
    // Labels per i 4 gruppi
    // ANIN
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("  border-radius: 4px;\n"));
    szTemp.append(QString::fromAscii("  background-color: AliceBlue;\n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  font: 14px;\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC030\n4 Analog Input"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseAnIn, 1, nItemsPerGroup);
    // ANOUT
    lblBox = new QLabel(this);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC035\n4 Analog Output"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseAnOut, 1, nItemsPerGroup);
    // DIGIN
    lblBox = new QLabel(this);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC020 01\n16 Digital Input"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseDigIn, 1, nItemsPerGroup);
    // DIGOUT
    lblBox = new QLabel(this);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC020 02\n16 Digital Output"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseDigOut, 1, nItemsPerGroup);
    //---------------------------------
    // Bottoni per eliminazione Moduli o MPNC006
    //---------------------------------
    szRemoveStyle.clear();
    szRemoveStyle.append(QString::fromAscii("QPushButton:disabled { \n"));
    szRemoveStyle.append(QString::fromAscii("    border: 0px ;\n"));
    szRemoveStyle.append(QString::fromAscii("    background-color: transparent;\n"));
    szRemoveStyle.append(QString::fromAscii("    background-image: url("");\n"));
    szRemoveStyle.append(QString::fromAscii("}\n"));
    szRemoveStyle.append(QString::fromAscii("QPushButton:enabled { \n"));
    szRemoveStyle.append(QString::fromAscii("    border: 1px solid navy;\n"));
    szRemoveStyle.append(QString::fromAscii("}\n"));
    szRemoveStyle.append(QString::fromAscii("QPushButton:selected, QPushButton:hover {\n"));
    szRemoveStyle.append(QString::fromAscii("    border: 1px solid DarkOrange ;\n"));
    szRemoveStyle.append(QString::fromAscii("}\n"));
    szRemoveStyle.append(QString::fromAscii("QPushButton:pressed { \n"));
    szRemoveStyle.append(QString::fromAscii("    border: 1px solid red;\n"));
    szRemoveStyle.append(QString::fromAscii("}\n"));
    szRemoveStyle.append(QString::fromAscii("QPushButton { \n"));
    szRemoveStyle.append(QString::fromAscii("  border: 0px solid blue;\n"));
    szRemoveStyle.append(QString::fromAscii("  border-radius: 4px;\n"));
    szRemoveStyle.append(QString::fromAscii("  min-height: 36px;\n"));
    szRemoveStyle.append(QString::fromAscii("  max-height: 36px;\n"));
    szRemoveStyle.append(QString::fromAscii("  min-width: 40px;\n"));
    szRemoveStyle.append(QString::fromAscii("  max-width: 40px;\n"));
    szRemoveStyle.append(QString::fromAscii("  background-color: AliceBlue;\n"));
    szRemoveStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szRemoveStyle.append(QString::fromAscii("}"));
    QString szRemoveToolTip = QString::fromAscii("Remove Module");
    // Bottoni per rimozione dei Moduli (1..4)
    for (i = 1; i < nTotalGroups; i++)  {
        QPushButton *remove = new QPushButton(this);
        remove->setEnabled(false);
        remove->setFlat(true);
        remove->setVisible(true);
        remove->setToolTip(szRemoveToolTip);
        remove->setStyleSheet(szRemoveStyle);
        mapRemoveClicked->setMapping(remove, int(i));
        connect(remove, SIGNAL(clicked()), mapRemoveClicked, SLOT(map()));
        mainGrid->addWidget(remove, nRowDesc, (nBaseHead) + (i * nItemsPerGroup));
        lstRemove.append(remove);
    }
    //---------------------------------
    // Bottoni per la gestione del singolo Modulo
    //---------------------------------
    // StyleSheet di base per ogni bottone
    szModuleStyle.clear();
    szModuleStyle.append(QString::fromAscii("QPushButton:disabled { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid darkGray;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QPushButton:enabled { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid navy;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QPushButton:selected, QPushButton:hover {\n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid DarkOrange ;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QPushButton:pressed { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid red;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QPushButton { \n"));
    szModuleStyle.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szModuleStyle.append(QString::fromAscii("  border-radius: 4px;\n"));
    szModuleStyle.append(QString::fromAscii("  min-height: 140px;\n"));
    szModuleStyle.append(QString::fromAscii("  max-height: 140px;\n"));
    szModuleStyle.append(QString::fromAscii("  min-width: 40px;\n"));
    szModuleStyle.append(QString::fromAscii("  max-width: 40px;\n"));
    szModuleStyle.append(QString::fromAscii("  background-position: center  center;\n"));
    szModuleStyle.append(QString::fromAscii("}"));
    QString szModuleToolTip = QString::fromAscii("Add/View Module");
    for (i = 0; i < nTotalItems; i++)  {
        QPushButton *module = new QPushButton(this);
        module->setEnabled(false);
        module->setFlat(true);
        module->setToolTip(szModuleToolTip);
        module->setStyleSheet(szModuleStyle);
        mapModuleClicked->setMapping(module, int(i));
        connect(module, SIGNAL(clicked()), mapModuleClicked, SLOT(map()));
        mainGrid->addWidget(module, nRowButtons, i);
        lstPulsanti.append(module);
    }
    // Molla Horizontal Spacer per allineare finestra
    QSpacerItem *hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hSpacer->setAlignment(Qt::AlignHCenter);
    mainGrid->addItem(hSpacer, nRowButtons, nTotalItems);
    // Bottone per Filtro su Testa Nodi
    szFilterStyle.clear();;
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
    mainGrid->addWidget(cmdFilter, nRowFlags, nBaseHead);
    // Label Gruppo (A..D)
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel:disabled {\n"));
    szTemp.append(QString::fromAscii("  border: 1px solid darkGray ;\n"));
    szTemp.append(QString::fromAscii("  background-color: transparent;\n"));
    szTemp.append(QString::fromAscii("  color: darkGray;\n"));
    szTemp.append(QString::fromAscii("}\n"));
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  max-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  border: 1px solid blue;\n"));
    szTemp.append(QString::fromAscii("  border-radius: 4px;\n"));
    szTemp.append(QString::fromAscii("  background-color: Cornflowerblue;\n"));
    szTemp.append(QString::fromAscii("  color: DarkOrange;\n"));
    szTemp.append(QString::fromAscii("  font: 14px;\n"));
    szTemp.append(QString::fromAscii("}"));
    for (i = 0; i < nTotalItems - 1; i++)  {
        lblBox = new QLabel(this);
        lblBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        lblBox->setText(lstPosFlags[i % nItemsPerGroup]);
        lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
        lblBox->setEnabled(false);
        lblBox->setStyleSheet(szTemp);
        mainGrid->addWidget(lblBox, nRowFlags, nBaseAnIn + i);
        lstPosMarker.append(lblBox);
    }
    // Grid per Editing
    tblCT = new QTableWidget(this);
    externalLayOut->addLayout(mainGrid);
    externalLayOut->addWidget(tblCT);
    //-------------------------------------
    // Collegamento del Mapper Bottoni - Slot vari, etc
    //-------------------------------------
    connect(mapModuleClicked, SIGNAL(mapped(int)), this, SLOT(buttonClicked(int)));
    connect(mapRemoveClicked, SIGNAL(mapped(int)), this, SLOT(groupItemRemove(int)));
    // Combo per cambio Modulo MPNC
    connect(cboSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(changeRootElement(int)));
    // Combo per modulo Porta
    connect(cboPort, SIGNAL(currentIndexChanged(int)), this, SLOT(on_changePort(int)));
    // Text Box Editing Node Id
    connect(txtNode, SIGNAL(editingFinished()), this, SLOT(on_changeNode()));
    // Bottone Rename CT
    connect(cmdRename, SIGNAL(clicked()), this, SLOT(on_RenameVars()));
    // Bottone Switch Mode
    connect(cmdFilter, SIGNAL(clicked()), this, SLOT(changeFilter()));
    // Row Double Clicled
    connect(tblCT, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRowDoubleClicked(QModelIndex)));
    // Row Clicled
    connect(tblCT, SIGNAL(clicked(QModelIndex)), this, SLOT(onRowClicked(QModelIndex)));
    // Combo Cambio porta Modulo
//    // Spacers tra Gruppo (solo 4 Items)
//    szTemp.clear();
//    szTemp.append(QString::fromAscii("  border: 1px solid red;\n"));
//    szTemp.append(QString::fromAscii("  background-color: red;\n"));
//    for (i = 0; i < nTotalGroups - 1; i++)  {
//        boxSeparator = new QFrame(this);
//        boxSeparator->setFrameShape(QFrame::VLine);
//        boxSeparator->setStyleSheet(szTemp);
//        mainGrid->addWidget(boxSeparator, nRowDesc, nBaseAnIn + (i * nItemsPerGroup), 2, 1);
//    }
    // Init variabili di gestione
    m_nTesta = -1;
    lstCapofila.clear();
    m_nBaseRow = -1;
    m_fCanRenameVars = false;
    m_nMinVarName = 0;
    m_nMaxVarName = 0;

}
int     Config_MPNC::getCurrentRow()
// Restituisce la riga correntemente selezionata
{
    return m_nCurrentCTRow;
}

bool    Config_MPNC::isUpdated()
// Ritorna vero se il contenuto dei nodi è stato modificato
{
    return m_fUpdated;
}

void Config_MPNC::showTestaNodi(int nTesta, QList<int> &lstCapofilaTeste, int nCurRow)
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
    qDebug() << QString::fromAscii("showTestaNodi(): NTesta: %1 - Teste Totali: %2 - Riga Corrente: %3") .arg(nTesta) .arg(lstCapofilaTeste.count()) .arg(nCurRow);
    if (nTesta < 0 || nTesta >= lstCapofilaTeste.count())  {
        for (nCur = 0; nCur < nTotalItems; nCur++)  {
            lstModuleIsPresent[nCur] = false;
        }
        nTesta = -1;
        nBaseRow = -1;
    }
    else  {
        lstCapofila = lstCapofilaTeste;
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
void    Config_MPNC::customizeButtons()
// Abilitazione delle icone Bottoni in funzione della presenza dei moduli
{
    int         nCur = 0;
    QString     szNewStyle;
    QString     szBackGround;
    QString     szIcon;
    bool        prevIsEnabled = false;
    bool        curIsEnabled = false;

    // Testa Nodi non presente, disabilito tutti i moduli
    prevIsEnabled = lstModuleIsPresent[nBaseHead];
    // Abilitazione dei bottoni "Remove" 1..4
    for (nCur = 1; nCur < nTotalGroups; nCur++)  {
        szNewStyle = szRemoveStyle.left(szRemoveStyle.length() - 1);
        curIsEnabled = false;
        szIcon.clear();
        // Se esiste almeno la base di quel gruppo abilita bottone Remove
        curIsEnabled = lstModuleIsPresent[nBaseAnIn + ((nCur -1) * nItemsPerGroup)];
        // Icona Remove
        szIcon = curIsEnabled ? szFileRemove : QString::fromAscii("");
        szIcon = QString::fromAscii("    qproperty-icon: url(%1); \n") .arg(szIcon);
        if (! szIcon.isEmpty())  {
            szIcon.append(QString::fromAscii("    background-color: Cornflowerblue;\n"));
        }
        szNewStyle.append(szIcon);
        // Abilitazione bottone Remove
        lstRemove[nCur - 1]->setEnabled(curIsEnabled);
        // Cambio StyleSheet
        szNewStyle.append(QString::fromAscii("}"));
        lstRemove[nCur - 1]->setStyleSheet(szNewStyle);
    }
    for (nCur = 0; nCur < nTotalItems; nCur++)  {
        // Recupero dello Style generale per i bottoni
        szNewStyle = szModuleStyle.left(szModuleStyle.length() - 1);
        szBackGround.clear();
        szIcon.clear();
        // Il bottone corrente è abilitato se il precedente modulo è presente e se la Root è presente
        // Oppure se è il primo bottone del Gruppo e se la Root è presente
        if ((nCur % nItemsPerGroup ) == 1)  {
            curIsEnabled = lstModuleIsPresent[nBaseHead];
        }
        else  {
            curIsEnabled = lstModuleIsPresent[nCur] || prevIsEnabled;
        }
        // Se il modulo è abilitato si imposta lo sfondo tipico del Device associato
        if (lstModuleIsPresent[nCur])  {
            szBackGround = QString::fromAscii("    background-image: url(%1);\n")  .arg(lstSfondi[nCur]);
            szBackGround.append(QString::fromAscii("    background-color: transparent;\n"));
        }
        else  {
            if (curIsEnabled)  {
                szIcon = szFileAdd;
            }
        }
        szIcon = QString::fromAscii("    qproperty-icon: url(%1); \n") .arg(szIcon);
        szIcon.append(QString::fromAscii("    background-color: Cornflowerblue;\n"));
        szNewStyle.append(szIcon);
        if (! szBackGround.isEmpty())  {
            szNewStyle.append(szBackGround);
        }
        // Cambio StyleSheet
        szNewStyle.append(QString::fromAscii("}"));
        lstPulsanti[nCur]->setStyleSheet(szNewStyle);
        // Abilitazione
        lstPulsanti[nCur]->setEnabled(curIsEnabled);
        // qDebug() << QString::fromAscii("customizeButtons(): Button: %1 - Enabled %2 - Style: %3") .arg(nCur) .arg(curIsEnabled ? 1 : 0) .arg(szNewStyle);
        // Stato del bottone corrente propagato per decidere bottone successivo
        prevIsEnabled = lstModuleIsPresent[nCur];
    }
}
void    Config_MPNC::getUsedModules(int nBaseRow)
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
            // Conversione da Posizione a Gruppo e Modulo
            abs2RelativeModulePos(nCur, nGroup, nModule);
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
void    Config_MPNC::changeRootElement(int nItem)
// Cambio di Item della Combo dei MPNC definiti
{
    int     nGroup = 0;
    int     nModule = 0;

    m_nTesta = -1;
    if (nItem >= 0 && nItem < lstCapofila.count())  {
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
            // Ricerca del Gruppo/Modulo di appartenenza della riga di CT. Se il modulo è utilizzato salta al modulo
            if (localCTRecords[m_nCurrentCTRow].Enable > nPriorityNone)  {
                nGroup = lstMPNC006_Vars[m_nCurrentCTRow - m_nBaseRow].Group;
                nModule = lstMPNC006_Vars[m_nCurrentCTRow - m_nBaseRow].Module;
            }
            int nABS = relative2AbsModulePos(nGroup, nModule);
            if (! lstModuleIsPresent[nABS])  {
                nGroup = 0;
                nModule = 0;
            }
            qDebug() << QString::fromAscii("changeRootElement(): Switching to Item: %1 - Row: %2 - Group: %3 - Module: %4")
                        .arg(nItem) .arg(m_nCurrentCTRow) .arg(nGroup) .arg(nModule);
            // Abilitazione interfaccia
            customizeButtons();
            filterVariables(nGroup, nModule);
        }
    }
    else  {
        qDebug() << QString::fromAscii("changeRootElement(): Attempt to switch to wrong element: %1") .arg(nItem);
    }
}
int     Config_MPNC::relative2AbsModulePos(int nGroup, int nModule)
// Calcola la posizione assoluta del Modulo
{
    int nAbs = -1;

    if (nGroup == 0)  {
        nAbs = 0;
    }
    else if (nGroup > 0 && nGroup < nTotalGroups && nModule >= 0 && nModule < nItemsPerGroup)  {
        nAbs = nBaseAnIn + ((nGroup - 1 )* nItemsPerGroup) + nModule;
    }
    // qDebug() << QString::fromAscii("relative2AbsModulePos(): Group: %1 Module: %2 Abs: %3") .arg(nGroup) .arg(nModule) .arg(nAbs);
    return nAbs;
}
void    Config_MPNC::abs2RelativeModulePos(int nAbs, int &nGroup, int &nModule)
// Calcola la posizione assoluta del Modulo
{
    nGroup = 0;
    nModule = 0;
    // Calcolo dei bottoni ulterioni
    if (nAbs > 0 && nAbs < nTotalItems)  {
        nGroup =  ((nAbs - 1) / nItemsPerGroup) + 1;
        nModule = ((nAbs - 1) % nItemsPerGroup);
    }
    // qDebug() << QString::fromAscii("abs2RelativeModulePos(): AbsPos: %1 Group: %2 Module: %3") .arg(nAbs) .arg(nGroup) .arg(nModule);
}

int     Config_MPNC::getLastModuleUsed(int nGroup)
// Ricerca dell'ultimo modulo usato nel gruppo (da 1 a 4)
// Ritorna -1 se il gruppo è fuori Range
// Ritorna 0 se non esiste nessun modulo utilizzato per il gruppo, oppure la posizione 1..nItemsPerGroup
{
    int nItem = -1;

    if (nGroup > 0 && nGroup < nTotalGroups)  {
        for (nItem = nItemsPerGroup - 1; nItem >= 0; nItem--)  {
            int nPos = relative2AbsModulePos(nGroup, nItem);
            if (nPos >= 0 && nPos < lstModuleIsPresent.count() && lstModuleIsPresent[nPos])  {
                break;
            }
        }
    }
    // Return value
    return nItem;
}
void    Config_MPNC::buttonClicked(int nButton)
// Gestore della pressione dei bottoni
{
    qDebug() << QString::fromAscii("buttonClicked(): Clicked Button: %1") .arg(nButton);
    int nGroup = 0;
    int nItem = 0;
    int nLastUsed = 0;

    abs2RelativeModulePos(nButton, nGroup, nItem);
    // Premuto un bottone di un gruppo
    if (nGroup > 0 && nItem >= 0)  {
        nLastUsed = getLastModuleUsed(nGroup);
        // Premuto in modalità aggiunta ?
        if (nItem > nLastUsed)  {
            m_szMsg = QString::fromAscii("Are you sure you want to Add a module [%1] at position [%2]?") .arg(lstModuleName[nGroup]) .arg(lstPosFlags[nItem]);
            if (queryUser(this, szMectTitle, m_szMsg))  {
                lstModuleIsPresent[nButton] = true;
                // Aggiornamento delle Variabili di CT relative all'oggetto
                setGroupVars(nGroup, nItem, m_nRootPriority);
                // Refresh Bottoni
                customizeButtons();
            }
        }
    }
    // Filter Variables of a Group / Item
    filterVariables(nGroup, nItem);
}
void    Config_MPNC::groupItemRemove(int nGroup)
// Rimozione elemento da gruppo
{
    // Controllo Gruppo 1..4
    if (nGroup <= 0 || nGroup >= nTotalGroups)
        return;

    int nItem = getLastModuleUsed(nGroup);

    qDebug() << QString::fromAscii("groupItemRemove(): Group Clicked: %1 - Name: %2 - Pos: %3") .arg(nGroup) .arg(lstModuleName[nGroup]) .arg(nItem);
    if (nItem >= 0)  {
        m_szMsg = QString::fromAscii("Are you sure you want to remove the module [%1] at position [%2]?") .arg(lstModuleName[nGroup]) .arg(lstPosFlags[nItem]);
        if (queryUser(this, szMectTitle, m_szMsg))  {
            int nAbsPos = relative2AbsModulePos(nGroup, nItem);
            lstModuleIsPresent[nAbsPos] = false;
            // Aggiornamento delle Variabili di CT relative all'oggetto
            setGroupVars(nGroup, nItem, nPriorityNone);
            // Refresh Bottoni
            m_nAbsPos = 0;
            customizeButtons();
            // Refresh Root
            filterVariables(0, 0);
        }
    }

}
void    Config_MPNC::filterVariables(int nGroup, int nItem)
// Filtra le variabili specifiche del modulo identificato da Gruppo e Posizione
{
    QStringList         lstLineValues;
    QList<QStringList > lstTableRows;
    QList<int16_t>      lstRowPriority;
    int                 nRow = 0;
    int                 nCurrentRow = 0;
    bool                fRes = false;

    qDebug() << QString::fromAscii("filterVariables(): Group: %1 - Name: %2 - Item: %3") .arg(nGroup) .arg(lstModuleName[nGroup]) .arg(nItem);
    // Preparazione tabella
    this->setCursor(Qt::WaitCursor);
    tblCT->setVisible(false);
    tblCT->setEnabled(false);
    tblCT->clearSelection();
    tblCT->setRowCount(0);
    tblCT->setColumnCount(0);
    tblCT->clearContents();
    tblCT->clear();
    lstTableRows.clear();
    lstRowPriority.clear();
    // Ciclo di Lettura
    for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
        // Confronto tra Variabile corrente e variabile paradigma in Modello (per nGroup == 0 vedi tutti)
        if ( (nGroup == 0 && nItem == 0)  ||
             (nGroup == lstMPNC006_Vars[nRow].Group && nItem == lstMPNC006_Vars[nRow].Module))  {
            bool    fShow = true;
            // Ulteriori selezioni per la Testa dei Nodi in funzione della modalità di visualizzazione
            if (nGroup == 0 && nItem == 0)  {
                switch (m_nShowMode)  {
                    case showHead:
                        // Visualizza solo elementi della Head (no Nodi)
                        fShow = (nGroup == lstMPNC006_Vars[nRow].Group && nItem == lstMPNC006_Vars[nRow].Module);
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
                fRes = recCT2MPNxFieldsValues(localCTRecords, lstLineValues, nRow + m_nBaseRow, lstMPNC006_Vars, nRow);
                // Aggiunta alla Table
                if (fRes)  {
                    // qDebug() << QString::fromAscii("Variable: %1") .arg(lstLineValues[colMPNxName]);
                    lstTableRows.append(lstLineValues);
                    lstRowPriority.append(localCTRecords[nRow + m_nBaseRow].Enable);
                    // Riga da selezionare in Grid
                    if (nRow + m_nBaseRow == m_nCurrentCTRow)  {
                        nCurrentRow = lstTableRows.count() - 1;
                        qDebug() << QString::fromAscii("filterVariables(): Current Table Row: %1 - CT Row: %2") .arg(nCurrentRow) .arg(nRow + m_nBaseRow);
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
    // Refresh delle Labels Marcatori del Gruppo/Item corrente
    m_nAbsPos = relative2AbsModulePos(nGroup, nItem);
    for (nRow = 0; nRow < lstPosMarker.count(); nRow++)  {
        lstPosMarker[nRow]->setEnabled(nRow == (m_nAbsPos -1));
    }
    this->setCursor(Qt::ArrowCursor);
}
void    Config_MPNC::on_changePort(int nPort)
// Evento cambio Porta RTU
{
    qDebug() << QString::fromAscii("changePort(): New Port %1") .arg(nPort);
    if (nPort != m_nPort)  {
        m_szMsg = QString::fromAscii("Confirm Serial Port change from [%1] to [%2] ?") .arg(m_nPort) .arg(nPort);
        if (queryUser(this, szMectTitle, m_szMsg))  {
            // Ciclo per riassegnare la porta Seriale
            int nRow = 0;
            for (nRow = m_nBaseRow; nRow < m_nBaseRow + m_nTotalRows; nRow++)  {
                if (nRow >= 0 && nRow < localCTRecords.count())  {
                    localCTRecords[nRow].Port = nPort;
                }
            }
            m_nPort = nPort;
            m_fUpdated = true;
        }
        // Aggiorna numero di porta
        disableAndBlockSignals(cboPort);
        cboPort->setCurrentIndex(m_nPort);
        enableAndUnlockSignals(cboPort);
        // Refresh interfaccia su nodo corrente
        if (m_nAbsPos >= 0 && m_nAbsPos <= nTotalItems)  {
            int nGroup = 0;
            int nModule = 0;
            abs2RelativeModulePos(m_nAbsPos, nGroup, nModule);
            if ((nGroup >= 0 && nGroup < nTotalGroups) && (nModule >= 0 && nModule < nItemsPerGroup))  {
                filterVariables(nGroup, nModule);
            }
        }
    }
}
void    Config_MPNC::on_changeNode()
// Evento Cambio Nodo
{
    disableAndBlockSignals(txtNode);
    if (txtNode->isModified())  {
        bool    fOk = false;
        QString szNewValue = txtNode->text().trimmed();
        if (szNewValue != QString::number(m_nNodeId))  {
            int nNode = szNewValue.toInt(&fOk);
            if (fOk && nNode != m_nNodeId)  {
                m_szMsg = QString::fromAscii("Confirm Node ID change from [%1] to [%2] ?") .arg(m_nNodeId) .arg(nNode);
                if (queryUser(this, szMectTitle, m_szMsg))  {
                    // Ciclo per riassegnare la porta Seriale
                    int nRow = 0;
                    for (nRow = m_nBaseRow; nRow < m_nBaseRow + m_nTotalRows; nRow++)  {
                        if (nRow >= 0 && nRow < localCTRecords.count())  {
                            localCTRecords[nRow].NodeId = nNode;
                        }
                    }
                    m_nNodeId = nNode;
                    m_fUpdated = true;
                }
            }
            // Aggiorna Numero di Nodo
            txtNode->setText(QString::number(m_nNodeId));
            txtNode->setModified(false);
            // Forza Repaint della Radice del Nodo
            m_nAbsPos = 0;
        }
        // Refresh interfaccia su nodo corrente
        if (m_nAbsPos >= 0 && m_nAbsPos <= nTotalItems)  {
            int nGroup = 0;
            int nModule = 0;
            abs2RelativeModulePos(m_nAbsPos, nGroup, nModule);
            if ((nGroup >= 0 && nGroup < nTotalGroups) && (nModule >= 0 && nModule < nItemsPerGroup))  {
                filterVariables(nGroup, nModule);
            }
        }
    }
    enableAndUnlockSignals(txtNode);
}
void    Config_MPNC::setGroupVars(int nGroup, int nModule, int16_t nPriority)
// Imposta la Priority per le variabili di Gruppo e Modulo
{
    int nRow = 0;

    for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
        // Confronto tra Variabile corrente e variabile paradigma in Modello (per nGroup == 0 vedi tutti)
        if (nGroup == lstMPNC006_Vars[nRow].Group && nModule == lstMPNC006_Vars[nRow].Module)  {
            localCTRecords[nRow + m_nBaseRow].Enable = nPriority;
            m_fUpdated = true;
        }
    }
}
void    Config_MPNC::on_RenameVars()
// Evento Rename Clicked
{
    bool        fOk = false;
    QString     szNewPrefix = QInputDialog::getText(this, QString::fromAscii("New Var Prefix:"), QString::fromAscii("Enter new Variables Prefix to rename all Variables:"),
                                                    QLineEdit::Normal, m_szVarNamePrefix, &fOk, Qt::Dialog);
    if (fOk)  {
        // Validazione del risultato
        m_szMsg.clear();
        szNewPrefix = szNewPrefix.trimmed();
        qDebug() << QString::fromAscii("on_RenameVars(): New Var Prefix: [%1]") .arg(szNewPrefix);
        // Controllo sulla lunghezza complessiva delle variabili rinominate
        if (szNewPrefix.length() + m_nMaxVarName > MAX_IDNAME_LEN)  {
            fOk = false;
            m_szMsg.append(QString::fromAscii("The Prefix [%1] is Too Long.\nThe resulting length of the Variable Names exceeds the limit of [%2] characters\n")
                    .arg(szNewPrefix) .arg(MAX_IDNAME_LEN));
        }
        // Prefisso Vuoto
        if (szNewPrefix.isEmpty())  {
            fOk = false;
            m_szMsg.append(QString::fromAscii("The Prefix is Empty.\n"));
        }
        // Prefisso non valido
        if (! isValidVarName(szNewPrefix))  {
            fOk = false;
            m_szMsg.append(QString::fromAscii("The prefix [%1] is not valid\nif used as the beginning of a variable name\n") .arg(szNewPrefix));
        }
        if (fOk)  {
            // Controlli superati, procedere al rename delle variabili
            QString szNewVarName(szEMPTY);
            int     nVar = 0;
            for (nVar = 0; nVar < m_nTotalRows; nVar++)  {
                // Generazione del nuovo nome variabile
                szNewVarName = QString::fromAscii(localCTRecords[m_nBaseRow + nVar].Tag);
                szNewVarName = szNewVarName.mid(m_szVarNamePrefix.length());
                szNewVarName.prepend(szNewPrefix);
                strcpy(localCTRecords[m_nBaseRow + nVar].Tag, szNewVarName.toAscii().data());
            }
            m_szVarNamePrefix = szNewPrefix;
            m_fUpdated = true;
            filterVariables(0, 0);
        }
        else {
            warnUser(this, szMectTitle, m_szMsg);
        }
    }
}
bool    Config_MPNC::canRenameRows(int nBaseRow)
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
void    Config_MPNC::onRowClicked(const QModelIndex &index)
// Evento Row Clicked
{
    int nRow = index.row();

    if (nRow >= 0)  {
        QTableWidgetItem    *tItem = tblCT->item(nRow, colMPNxRowNum);
        if (tItem != 0)  {
            bool fOk = false;
            nRow = tItem->text().toInt(&fOk);
            if (fOk)  {
                m_nCurrentCTRow = nRow;
                qDebug() << QString::fromAscii("onRowClicked(): CT Row:[%1]") .arg(m_nCurrentCTRow);
            }
        }
    }
}
void    Config_MPNC::onRowDoubleClicked(const QModelIndex &index)
// Evento Row Double Clicked
{
    int nRow = index.row();

    if (nRow >= 0 && m_nCurrentCTRow )  {
        qDebug() << QString::fromAscii("onRowDoubleClicked(): CT Row:[%1]") .arg(m_nCurrentCTRow);
        emit varClicked(m_nCurrentCTRow);
    }
}
void    Config_MPNC::changeFilter()
// Cambio del filtro sui moduli
{
    int nNewMode = (++m_nShowMode) % showTotals;

    m_nShowMode = nNewMode;
    setFilterButton(m_nShowMode);
    filterVariables(0, 0);
}
void    Config_MPNC::setFilterButton(int nNewMode)
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
