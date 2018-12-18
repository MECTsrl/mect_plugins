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
// Prefisso per Nomi Variabili
const QString szVarNamePrefix = QString::fromAscii("XX_");

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
    m_fUpdated = false;
    m_nRootPriority = nPriorityNone;
    m_nAbsPos = 0;
    m_nTotalRows = lstMPNC006_Vars.count();             // Numero di Variabili da gestire
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
    lblBox->setText(QString::fromAscii("Head:"));
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
    // Label MPNC006
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  border: 1px solid navy;\n"));
    szTemp.append(QString::fromAscii("  border-radius: 4px;\n"));
    szTemp.append(QString::fromAscii("  background-color: AliceBlue;\n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  font: 8px;\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("MPNC006"));
    lblBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseHead);
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
    // Bottoni per rimozione dei Moduli
    for (i = 0; i < nTotalGroups; i++)  {
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
    QString szRenameToolTip = QString::fromAscii("Rename Rows");
    cmdRename = new QPushButton(this);
    cmdRename->setEnabled(true);
    cmdRename->setFlat(true);
    cmdRename->setToolTip(szRenameToolTip);
    cmdRename->setStyleSheet(szTemp);
    mainGrid->addWidget(cmdRename, nRowFlags, nBaseHead);
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
    // Collegamento del Mapper Bottoni
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
    m_nMaxVarName = 0;
    for (i = 0; i < lstMPNC006_Vars.count(); i++)  {
        int nLen = lstMPNC006_Vars[i][colName].length();
        if (nLen > m_nMaxVarName)  {
            m_nMaxVarName = nLen;
        }
    }
    qDebug() << QString::fromAscii("Config_MPNC(): Max Len Var: %1") .arg(m_nMaxVarName);

}
bool    Config_MPNC::isUpdated()
// Ritorna vero se il contenuto dei nodi è stato modificato
{
    return m_fUpdated;
}

void Config_MPNC::showTestaNodi(int nTesta, QList<int> &lstCapofilaTeste)
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
    txtNode->setModified(false);
    qDebug() << QString::fromAscii("showTestaNodi(): NTesta: %1 - Teste Totali: %2") .arg(nTesta) .arg(lstCapofilaTeste.count());
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
    // Abilitazione dei bottoni "Remove"
    for (nCur = 0; nCur < nTotalGroups; nCur++)  {
        szNewStyle = szRemoveStyle.left(szRemoveStyle.length() - 1);
        curIsEnabled = false;
        szIcon.clear();
        if (nCur == 0)  {
            // Rimozione Base abilitata solo se nessun modulo è presente
            curIsEnabled = ! (lstModuleIsPresent[nBaseAnIn] || lstModuleIsPresent[nBaseAnOut] || lstModuleIsPresent[nBaseDigIn] || lstModuleIsPresent[nBaseDigOut]);
        }
        else  {
            // Se esiste almeno la base di quel gruppo abilita bottone Remove
            curIsEnabled = lstModuleIsPresent[nBaseAnIn + ((nCur -1) * nItemsPerGroup)];
        }
        // Icona Remove
        szIcon = curIsEnabled ? szFileRemove : QString::fromAscii("");
        szIcon = QString::fromAscii("    qproperty-icon: url(%1); \n") .arg(szIcon);
        if (! szIcon.isEmpty())  {
            szIcon.append(QString::fromAscii("    background-color: Cornflowerblue;\n"));
        }
        szNewStyle.append(szIcon);
        // Abilitazione bottone Remove
        lstRemove[nCur]->setEnabled(curIsEnabled);
        // Cambio StyleSheet
        szNewStyle.append(QString::fromAscii("}"));
        lstRemove[nCur]->setStyleSheet(szNewStyle);
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
    bool    okGroup = false;
    bool    okModule = false;
    bool    fUsed = false;

    qDebug() << QString::fromAscii("getUsedModules(): Main Head Row: %1") .arg(nBaseRow);
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
                int     nVarGroup = lstMPNC006_Vars[nRow][colGroup].toInt(&okGroup);
                int     nVarModule = lstMPNC006_Vars[nRow][colModule].toInt(&okModule);
                // La riga appartiene al modulo che stiamo cercando
                if (okGroup && okModule && nGroup == nVarGroup && nModule == nVarModule)  {
                    fUsed = localCTRecords[nBaseRow + nRow].UsedEntry && localCTRecords[nBaseRow + nRow].Enable > nPriorityNone;
                    lstModuleIsPresent[nCur] = fUsed;
                    qDebug() << QString::fromAscii("getUsedModules(): Module: %1 is: %2") .arg(nCur) .arg(fUsed ? QString::fromAscii("YES") : QString::fromAscii("NO"));
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
    m_nTesta = -1;
    qDebug() << QString::fromAscii("changeRootElement(): Item: %1") .arg(nItem);
    if (nItem >= 0 && nItem < lstCapofila.count())  {
        qDebug() << QString::fromAscii("changeRootElement(): Switching to Item: %1") .arg(nItem);
        m_nBaseRow = lstCapofila[nItem];
        m_nTesta = nItem;
        // Determina il Protocollo, la Porta e il Nodo dell'elemento
        if (m_nBaseRow >=  0 && m_nBaseRow < localCTRecords.count() - m_nTotalRows)  {
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
        }
    }
    else  {
        qDebug() << QString::fromAscii("changeRootElement(): Attempt to switch to wrong element: %1") .arg(nItem);
    }
    // Aggiornamento lista moduli
    getUsedModules(m_nBaseRow);
    // Abilitazione interfaccia
    customizeButtons();
    filterVariables(0, 0);

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
    qDebug() << QString::fromAscii("relative2AbsModulePos(): Group: %1 Module: %2 Abs: %3") .arg(nGroup) .arg(nModule) .arg(nAbs);
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
    qDebug() << QString::fromAscii("abs2RelativeModulePos(): AbsPos: %1 Group: %2 Module: %3") .arg(nAbs) .arg(nGroup) .arg(nModule);
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
        bool    okGroup, okModule;
        int     nVarGroup = lstMPNC006_Vars[nRow][colGroup].toInt(&okGroup);
        int     nVarModule = lstMPNC006_Vars[nRow][colModule].toInt(&okModule);
        // Confronto tra Variabile corrente e variabile paradigma in Modello (per nGroup == 0 vedi tutti)
        if ( (nGroup == 0 && nItem == 0)  ||
             (okGroup && okModule && nGroup == nVarGroup && nItem == nVarModule))  {
            // Decodifica dei valori di CT e conversione in stringa
            fRes = recCT2MPNxList(localCTRecords, lstLineValues, nRow + m_nBaseRow, lstMPNC006_Vars, nRow);
            // Aggiunta alla Table
            if (fRes)  {
                // qDebug() << QString::fromAscii("Variable: %1") .arg(lstLineValues[colMPNxName]);
                lstTableRows.append(lstLineValues);
                lstRowPriority.append(localCTRecords[nRow + m_nBaseRow].Enable);
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
//    tblCT->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    if (nRow > 0)  {
        tblCT->setSelectionBehavior(QAbstractItemView::SelectRows);
        tblCT->setSelectionMode(QAbstractItemView::ExtendedSelection);
        tblCT->setVisible(true);
        tblCT->setEnabled(true);
        tblCT->selectRow(0);
        tblCT->update();
        tblCT->setFocus();
    }    
    qDebug() << QString::fromAscii("Displayed Rows: %1") .arg(nRow);
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
    if (txtNode->isModified())  {
        bool    fOk = false;
        int nNode = txtNode->text().toInt(&fOk);
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
        disableAndBlockSignals(txtNode);
        txtNode->setText(QString::number(m_nNodeId));
        enableAndUnlockSignals(txtNode);
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
void    Config_MPNC::setGroupVars(int nGroup, int nModule, int16_t nPriority)
// Imposta la Priority per le variabili di Gruppo e Modulo
{
    int nRow = 0;

    for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
        bool    okGroup, okModule;
        int     nVarGroup = lstMPNC006_Vars[nRow][colGroup].toInt(&okGroup);
        int     nVarModule = lstMPNC006_Vars[nRow][colModule].toInt(&okModule);
        // Confronto tra Variabile corrente e variabile paradigma in Modello (per nGroup == 0 vedi tutti)
        if (okGroup && okModule && nGroup == nVarGroup && nModule == nVarModule)  {
            localCTRecords[nRow + m_nBaseRow].Enable = nPriority;
            m_fUpdated = true;
        }
    }
}
void    Config_MPNC::on_RenameVars()
// Evento Rename Clicked
{
    bool        fOk = false;
    QString     szNewSuffix = QInputDialog::getText(this, QString::fromAscii("New Var Prefix:"), QString::fromAscii("Enter new Variables Prefix to rename all Variables:"),
                                                    QLineEdit::Normal, szVarNamePrefix, &fOk, Qt::Dialog);
    if (fOk)  {
        qDebug() << QString::fromAscii("on_RenameVars(): New Var Prefix: [%1]") .arg(szNewSuffix);
    }
}
bool    Config_MPNC::canRenameRows(int nBaseRow)
// Verifica se tutto il Device può essere rinominato
{
    QString     szBaseName(szEMPTY);
    QString     szVarName(szEMPTY);

    bool        fCanRename = true;
    int         nRow = 0;

    if (m_nBaseRow >=  0 && m_nBaseRow < localCTRecords.count() - m_nTotalRows)  {
        for (nRow = 0; nRow < m_nTotalRows; nRow++)  {
            // Recupera il nome della variabile
            szBaseName = lstMPNC006_Vars[nRow][colName];
            szBaseName = szBaseName.mid(szVarNamePrefix.length());
            // qDebug() << QString::fromAscii("canRenameRows(): Base Name:%1") .arg(szBaseName);
            szVarName = QString::fromAscii(localCTRecords[nBaseRow + nRow].Tag);
            if (! szVarName.endsWith(szBaseName, Qt::CaseSensitive))  {
                qDebug() << QString::fromAscii("canRenameRows(): Failed! Row:[%1] Var Name:[%2] Base Name:[%3]") .arg(nRow) .arg(szVarName) .arg(szBaseName);
                fCanRename = false;
                break;
            }
        }
    }
    else  {
        fCanRename = false;
    }
    // Return Value
    return fCanRename;
}
