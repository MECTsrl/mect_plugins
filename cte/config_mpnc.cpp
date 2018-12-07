#include "config_mpnc.h"
#include "utils.h"
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPixmap>
#include <QPalette>
#include <QColor>
#include <QIcon>
#include <QDebug>

const int nItemsPerGroup = 4;
const int nTotalItems = 17;
// Elementi MPNC
const int nTotalRows = 212;
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
const QString szFileMPNC006 = szPathIMG + QString::fromAscii("MPNC006_R.png");
const QString szFileMPNC030 = szPathIMG + QString::fromAscii("MPNC030_R.png");
const QString szFileMPNC035 = szPathIMG + QString::fromAscii("MPNC030_R.png");
const QString szFileMPNC020_01 = szPathIMG + QString::fromAscii("MPNC020_R.png");
const QString szFileMPNC020_02 = szPathIMG + QString::fromAscii("MPNC020_R.png");


Config_MPNC::Config_MPNC(QWidget *parent) :
    QWidget(parent)
{
    int         i = 0;
    QString     szTemp;
    QLabel      *lblBox;

    // Creazione del Layout principale del Form
    externalLayOut = new QVBoxLayout(this);             // Lay-Out Esterno del Frame
    mainGrid = new QGridLayout();
    mainGrid->setHorizontalSpacing(2);
    mapButtonClicked = new QSignalMapper(this);         // Signal Mapper per Bottone Clicked
    // Labels per Flags
    QStringList lstFlags;
    for (i = 0; i < nItemsPerGroup; i++)  {
        QChar chBase = QChar::fromAscii(65 + i);
        lstFlags.append(QString(1, chBase));
    }
    // Lista degli Sfondi associati ai Moduli
    lstSfondi.clear();
    // MPNC006
    lstSfondi.append(szFileMPNC006);
    // MPNC030
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC030);
    }
    // MPNC035
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC035);
    }
    // MPNC020_01
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC020_01);
    }
    // MPNC020_02
    for (i = 0; i < nItemsPerGroup; i ++)  {
        lstSfondi.append(szFileMPNC020_02);
    }
    // Flag Abilitazione dei Moduli (il modulo 0 è MPNC006)
    lstModuleIsPresent.clear();
    for (i = 0; i < nTotalItems; i++)  {
        lstModuleIsPresent.append(false);
    }
    // Labels per Combo Selettore
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setText(QString::fromAscii("Head:"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowSelector, nBaseHead);
    // Combo Selettore mpnc
    cboSelector = new QComboBox(this);
    mainGrid->addWidget(cboSelector, nRowSelector, nBaseAnIn, 1, nItemsPerGroup);
    // Label MPNC006
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  font: 8px;\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lblBox->setText(QString::fromAscii("MPNC006"));
    lblBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseHead);
    // Labels per i 4 gruppi
    // ANIN
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 36px;\n"));
    szTemp.append(QString::fromAscii("  font: 14px;\n"));
    szTemp.append(QString::fromAscii("}"));
    lblBox = new QLabel(this);
    lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC030\n4 Analog Input"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseAnIn, 1, nItemsPerGroup);
    // ANOUT
    lblBox = new QLabel(this);
    lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC035\n4 Analog Output"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseAnOut, 1, nItemsPerGroup);
    // DIGIN
    lblBox = new QLabel(this);
    lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC020 01\n16 Digital Input"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseDigIn, 1, nItemsPerGroup);
    // DIGOUT
    lblBox = new QLabel(this);
    lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
    lblBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lblBox->setText(QString::fromAscii("MPNC020 02\n16 Digital Output"));
    lblBox->setStyleSheet(szTemp);
    mainGrid->addWidget(lblBox, nRowDesc, nBaseDigOut, 1, nItemsPerGroup);
    // Bottoni per gestione dei moduli
    // StyleSheet di base per ogni bottone
    szModuleStyle.clear();
    szModuleStyle.append(QString::fromAscii("QPushButton:disabled { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid darkGray;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QPushButton:enabled { \n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid navy;\n"));
    szModuleStyle.append(QString::fromAscii("}\n"));
    szModuleStyle.append(QString::fromAscii("QPushButton:selected, QPushButton:hover {\n"));
    szModuleStyle.append(QString::fromAscii("    border: 1px solid yellow;\n"));
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
    for (i = 0; i < nTotalItems; i++)  {
        QPushButton *module = new QPushButton(this);
        module->setEnabled(false);
        module->setFlat(true);
        module->setStyleSheet(szModuleStyle);
        mapButtonClicked->setMapping(module, int(i));
        connect(module, SIGNAL(clicked()), mapButtonClicked, SLOT(map()));
        mainGrid->addWidget(module, nRowButtons, i);
        lstPulsanti.append(module);
    }
    // Horizontal Spacer per allineare finestra
    QSpacerItem *hSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hSpacer->setAlignment(Qt::AlignHCenter);
    mainGrid->addItem(hSpacer, nRowButtons, nTotalItems);
    // Label Gruppo (A..D)
    szTemp.clear();
    szTemp.append(QString::fromAscii("QLabel { \n"));
    szTemp.append(QString::fromAscii("  min-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  max-width: 40px;\n"));
    szTemp.append(QString::fromAscii("  min-height: 20px;\n"));
    szTemp.append(QString::fromAscii("  max-height: 20px;\n"));
    szTemp.append(QString::fromAscii("  font: 14px;\n"));
    szTemp.append(QString::fromAscii("}"));
    for (i = 0; i < nTotalItems - 1; i++)  {
        lblBox = new QLabel(this);
        lblBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        lblBox->setText(lstFlags[i % nItemsPerGroup]);
        lblBox->setFrameStyle(QFrame::Box | QFrame::Sunken);
        lblBox->setStyleSheet(szTemp);
        mainGrid->addWidget(lblBox, nRowFlags, nBaseAnIn + i);
    }
    // Grid per Editing
    tblCT = new QTableWidget(this);
    externalLayOut->addLayout(mainGrid);
    externalLayOut->addWidget(tblCT);
    //-------------------------------------
    // Collegamento del Mapper Bottoni
    //-------------------------------------
    connect(mapButtonClicked, SIGNAL(mapped(int)), this, SLOT(buttonClicked(int)));
    // Combo per cambio Modulo MPNC
    connect(cboSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(changeRootElement(int)));
    // Init variabili di gestione
    m_nTesta = -1;
    lstCapofila.clear();
    m_nBaseRow = -1;
}

void Config_MPNC::showTestaNodi(int nTesta, QList<int> &lstCapofilaTeste, QList<CrossTableRecord> &lstRows)
{
    int         nCur = 0;
    QString     szTemp;
    int         nBaseRow = -1;

    // Blocca la combo Selettore delle Teste
    // Ricarica i valori perchè potrebbero essere cambiati da giro precedente
    disableAndBlockSignals(cboSelector);
    cboSelector->clear();
    lstCapofila.clear();
    lstCTUserRows.clear();
    lstCTUserRows = lstRows;
    qDebug() << QString::fromAscii("showTestaNodi(): NBase: %1 - CT Rows: %2") .arg(nBaseRow) .arg(lstCTUserRows.count());
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
            szTemp = QString::fromAscii("%1 - Row: %2") .arg(nCur + 1) .arg(lstCapofila[nCur]);
            cboSelector->addItem(szTemp);
            if (nCur == nTesta)  {
                cboSelector->setCurrentIndex(nCur);
            }
        }
        // Leggi le caratteristiche dei moduli presenti per la testa nTesta
        nBaseRow = lstCapofila[nTesta];
    }
    // Aggiorna Le Icone dei Bottoni
    qDebug() << QString::fromAscii("showTestaNodi(): NBase before getUsedModules()  %1") .arg(nBaseRow);
    getUsedModules(nBaseRow);
    customizeButtons();
    enableAndUnlockSignals(cboSelector);
    m_nTesta = nTesta;
    m_nBaseRow = nBaseRow;
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
    if (! lstModuleIsPresent[nBaseHead])  {
        lstModuleIsPresent[nBaseHead] = true;
        lstModuleIsPresent[nBaseAnIn] = true;
        lstModuleIsPresent[nBaseAnOut] = true;
        lstModuleIsPresent[nBaseAnOut + 1] = true;
        lstModuleIsPresent[nBaseDigIn] = true;
        lstModuleIsPresent[nBaseDigIn + 1] = true;
        lstModuleIsPresent[nBaseDigIn + 2] = true;
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
        // Impostazione valori
        if (! szIcon.isEmpty())  {
            szIcon = QString::fromAscii("    qproperty-icon: url(%1); \n") .arg(szIcon);
            szIcon.append(QString::fromAscii("    background-color: Cornflowerblue;\n"));
            szNewStyle.append(szIcon);
        }
        else if (! szBackGround.isEmpty())  {
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
void    Config_MPNC::buttonClicked(int nButton)
// Gestore della pressione dei bottoni
{
    qDebug() << QString::fromAscii("buttonClicked(): Clicked Button: %1") .arg(nButton);
}
void    Config_MPNC::getUsedModules(int nRow)
// Legge a partire dalla riga del Capofila il numero di Moduli utilizzati
{
    int nCur = 0;

    qDebug() << QString::fromAscii("getUsedModules(): Main Head Row: %1") .arg(nRow);
    // Riga fuori Range
    if (nRow < 1 || nRow > (MIN_DIAG - nTotalRows))  {
        for (nCur = 0; nCur < nTotalItems; nCur ++)  {
            lstModuleIsPresent[nCur] = false;
        }
    }
    else  {
        for (nCur = 0; nCur < nTotalItems; nCur ++)  {
            lstModuleIsPresent[nCur] = true;
        }
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
    }
    else  {
        qDebug() << QString::fromAscii("changeRootElement(): Attempt to switch to wrong element: %1") .arg(nItem);
    }
    // Aggiornamento lista moduli
    getUsedModules(m_nBaseRow);
    // Abilitazione interfaccia
    customizeButtons();

}
