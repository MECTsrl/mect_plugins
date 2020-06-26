#include "searchvariable.h"
#include "ui_searchvariable.h"
#include "cteUtils.h"
#include "utils.h"

#include <QIntValidator>
#include <QPushButton>

SearchVariable::SearchVariable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchVariable)
{
    int nCol = 0;

    this->setVisible(false);
    ui->setupUi(this);
    isResettingFilter = true;
    // Combo Sections (bloccata per evitare Side Effects al currentIndex)
    for (nCol = 0; nCol < regTotals; nCol++)  {
        ui->cboSections->addItem(lstRegions[nCol]);
    }
    ui->cboSections->setCurrentIndex(-1);
    // Combo Priority
    for  (nCol = 0; nCol < nNumPriority; nCol++)   {
        ui->cboPriority->addItem(lstPriority[nCol], lstPriority[nCol]);
    }
    ui->cboPriority->setCurrentIndex(-1);
    // Combo Types
    for  (nCol=0; nCol<lstTipi.count(); nCol++)   {
        ui->cboType->addItem(lstTipi[nCol], lstTipi[nCol]);
    }
    ui->cboType->setCurrentIndex(-1);
    // Combo Protocols
    for  (nCol=0; nCol<lstProtocol.count(); nCol++)   {
        ui->cboProtocol->addItem(lstProtocol[nCol], lstProtocol[nCol]);
    }
    ui->cboProtocol->setCurrentIndex(-1);
    // Validator per txtNode
    ui->txtNode->setValidator(new QIntValidator(0, nMaxNodeID, this));
    // Reset dei filtri a carattere
    ui->txtVarname->clear();
    ui->txtNode->clear();
    // Lista Colonne Visibili
    lstVisibleCols.clear();
    lstVisibleCols.append(colPriority);
    lstVisibleCols.append(colUpdate);
    lstVisibleCols.append(colName);
    lstVisibleCols.append(colType);
    lstVisibleCols.append(colDecimal);
    lstVisibleCols.append(colProtocol);
    lstVisibleCols.append(colPort);
    lstVisibleCols.append(colNodeID);
    lstVisibleCols.append(colRegister);
    lstVisibleCols.append(colBehavior);
    lstVisibleCols.append(colComment);
    // Righe presenti
    lstRowNumbers.clear();
    // Azioni collegate ai pulsanti
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QPushButton *resetButton = ui->buttonBox->button(QDialogButtonBox::Reset);
    connect(resetButton, SIGNAL(clicked()), this, SLOT(resetFilters()));
    // Row Double Clicled
    connect(ui->tblVariables, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onRowDoubleClicked(QModelIndex)));
    connect(ui->tblVariables, SIGNAL(clicked(QModelIndex)), this, SLOT(onRowClicked(QModelIndex)));
    nSelectedRow = -1;
    fCaseSensitive = false;
    fNameStartsWith = false;
    resetFilters();
}

SearchVariable::~SearchVariable()
{
    delete ui;
}

int SearchVariable::getSelectedVariable()
{
    return     nSelectedRow;
}

bool SearchVariable::filterCTVars()
{
    bool                fRes = false;
    int                 nCur = 0;
    int                 nCol = 0;
    int                 nFound = 0;
    QStringList         lstLineValues;
    QList<QStringList > lstTableRows;
    QList<int16_t>      lstRowPriority;
    QList<int16_t>      lstRows;

    // Reset dei filtri in corso, non deve fare nulla
    if (isResettingFilter)  {
        qWarning("Reset Filtro in corso, exit");
        return true;
    }
    // Preparazione tabella
    this->setCursor(Qt::WaitCursor);
    disableAndBlockSignals(ui->tblVariables);
    // ui->tblVariables->setVisible(false);
    ui->tblVariables->setEnabled(false);
    ui->tblVariables->clearSelection();
    ui->tblVariables->setRowCount(0);
    ui->tblVariables->setColumnCount(0);
    ui->tblVariables->clearContents();
    ui->tblVariables->clear();
    nSelectedRow = -1;
    // Verifica elementi di CT
    lstRowNumbers.clear();
    lstTableRows.clear();
    lstRowPriority.clear();
    lstRows.clear();
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        // Convert CT Record 2 User Values
        fRes = recCT2FieldsValues(lstCTRecords, lstLineValues, nCur);
        if (fRes)  {
            // Controllo se la riga corrente supera il filtro
            if (rec2show(lstLineValues, nCur))  {
                lstTableRows.append(lstLineValues);
                lstRowPriority.append(lstCTRecords[nCur].Enable);
                lstRows.append(nCur);
                nFound++;
            }
        }
    }
    // Dimensionamento e caricamento elementi in Tabella
    ui->tblVariables->setRowCount(lstTableRows.count());
    ui->tblVariables->setColumnCount(colTotals);
    // Caricamento in Tabella
    for (nCur = 0; nCur < lstTableRows.count(); nCur++)  {
        // Imposta Numero Riga
        lstRowNumbers.append(QString::fromAscii("%1") .arg(lstRows[nCur] + 1, 5, 10));
        // Aggiunge elemento in Tabella
        fRes = list2GridRow(ui->tblVariables, lstTableRows[nCur], lstHeadLeftCols, nCur);
    }
    // Impostazione parametri TableView
    ui->tblVariables->setVerticalHeaderLabels(lstRowNumbers);
    ui->tblVariables->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tblVariables->setSelectionMode(QAbstractItemView::SingleSelection);
    setGridParams(ui->tblVariables, lstHeadCols, lstHeadSizes, QAbstractItemView::SingleSelection);
    // Colore di Sfondo delle righe
    for (nCur = 0; nCur < lstTableRows.count(); nCur++)  {
        setRowColor(ui->tblVariables, lstRows[nCur], 0, 1, lstRowPriority[nCur], 0);
    }
    // Nascondi Colonne non visibili
    for (nCol = 0; nCol < colTotals; nCol++)  {
        // Se la colonna non è tra quelle visibili la nasconde
        if (lstVisibleCols.indexOf(nCol) < 0)  {
            ui->tblVariables->setColumnHidden(nCol, true);
        }
    }
    // Numero di Elementi trovato
    ui->lblFound->setText(QString::number(nFound));

    // Return value
    this->setCursor(Qt::ArrowCursor);
    qDebug("ctable2Filter(): Found %d", nFound);
    if (nFound)  {
        ui->tblVariables->selectRow(0);
        fRes = true;
    }
    // ui->tblVariables->setVisible(true);
    ui->tblVariables->setEnabled(true);
    ui->tblVariables->update();
    // ui->tblVariables->setFocus();
    enableAndUnlockSignals(ui->tblVariables);
    return fRes;
}

bool    SearchVariable::rec2show(QStringList &lstFields, int nRow)
{
    bool    f2Show = false;
    QString szNameFilter = ui->txtVarname->text().trimmed();
    QString szNode = ui->txtNode->text().trimmed();
    int     nSection = ui->cboSections->currentIndex();
    Qt::CaseSensitivity caseCompare = fCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;

    // Entry non utilizzata
    if (lstFields[colName].trimmed().isEmpty())  {
        goto endCheck;
    }
    // Controllo sulla Sezione
    if (nSection >= 0)  {
        // Ritentive
        if (nSection == regRetentive)  {
            if (nRow >= MAX_RETENTIVE)  {
                goto endCheck;
            }
        }
        // Non Ritentive
        else if (nSection == regNonRetentive)  {
            if (nRow < MAX_RETENTIVE || nRow >= MAX_NONRETENTIVE)  {
                goto endCheck;
            }
        }
        // Diags
        else if (nSection == regDiagnostic)  {
            if (nRow < MAX_NONRETENTIVE || nRow >= MAX_NODE)  {
                goto endCheck;
            }
        }
        // LocalIO
        else if (nSection == regLocalIO)  {
            if (nRow < MAX_NODE || nRow >= MIN_SYSTEM - 1)  {
                goto endCheck;
            }
        }
        // System
        else if (nSection == regSystem)  {
            if (nRow < MIN_SYSTEM - 1)  {
                goto endCheck;
            }
        }
    }
    // Controllo su Priorità
    if (ui->cboPriority->currentIndex() >= 0)  {
        if (lstFields[colPriority] != ui->cboPriority->currentText())  {
            goto endCheck;
        }
    }
    // Controllo sul Tipo
    if (ui->cboType->currentIndex() >= 0)  {
        if (lstFields[colType] != ui->cboType->currentText())  {
            goto endCheck;
        }
    }
    // Controllo sul Protocollo
    if (ui->cboProtocol->currentIndex() >= 0)  {
        if (lstFields[colProtocol] != ui->cboProtocol->currentText())  {
            goto endCheck;
        }
    }
    // Controllo sul numero di Nodo
    if (! szNode.isEmpty())  {
        if (lstFields[colNodeID] != szNode)  {
            goto endCheck;
        }
    }
    // Filtro sui Nomi delle variabili
    if (! szNameFilter.isEmpty())  {
        if (fNameStartsWith)  {
            if (! lstFields[colName].startsWith(szNameFilter, caseCompare)) {
                goto endCheck;
            }
        }
        else  {
            if (! lstFields[colName].contains(szNameFilter, caseCompare))  {
                goto endCheck;
            }
        }
    }
    // Tutti i controlli sono superati, il record deve essere mostrato
    f2Show = true;

endCheck:
    // Return value
    return f2Show;
}

void SearchVariable::on_cboSections_currentIndexChanged(int index)
{
    if (! isResettingFilter)  {
        if (index >= 0)  {
            qDebug("New Filter on Section: %s", lstRegions[index].toLatin1().data());
        }
        else  {
            qDebug("Cleared Section Filter");
        }
        filterCTVars();
    }
}

void SearchVariable::resetFilters()
{
    isResettingFilter = true;
    ui->cboSections->setCurrentIndex(-1);
    ui->cboPriority->setCurrentIndex(-1);
    ui->cboType->setCurrentIndex(-1);
    ui->cboProtocol->setCurrentIndex(-1);
    ui->txtNode->clear();
    ui->txtVarname->clear();
    ui->chkCase->setChecked(false);
    ui->chkStartsWith->setChecked(false);
    ui->lblFound->setText(szZERO);
    ui->tblVariables->setRowCount(0);
    nSelectedRow = -1;
    isResettingFilter = false;
}

void SearchVariable::on_cboPriority_currentIndexChanged(int index)
{
    if (! isResettingFilter)  {
        if (index >= 0)  {
            qDebug("New Filter on Priority: %s", lstPriority[index].toLatin1().data());
        }
        else  {
            qDebug("Cleared Priority Filter");
        }
        filterCTVars();
    }
}

void SearchVariable::on_cboType_currentIndexChanged(int index)
{
    if (! isResettingFilter)  {
        if (index >= 0)  {
            qDebug("New Filter on Type: %s", lstTipi[index].toLatin1().data());
        }
        else  {
            qDebug("Cleared Var Type Filter");
        }
        filterCTVars();
    }
}

void SearchVariable::on_cboProtocol_currentIndexChanged(int index)
{
    if (! isResettingFilter)  {
        if (index >= 0)  {
            qDebug("New Filter on Protocol: %s", lstProtocol[index].toLatin1().data());
        }
        else  {
            qDebug("Cleared Protocol Filter");
        }
        filterCTVars();
    }
}

void SearchVariable::on_chkCase_clicked(bool checked)
{
    fCaseSensitive = checked;
    if (! isResettingFilter && ! ui->txtVarname->text().trimmed().isEmpty())  {
        filterCTVars();
    }
}

void SearchVariable::on_chkStartsWith_clicked(bool checked)
{
    fNameStartsWith = checked;
    if (! isResettingFilter && ! ui->txtVarname->text().trimmed().isEmpty())  {
        filterCTVars();
    }
}

void SearchVariable::on_txtVarname_textChanged(const QString &arg1)
{
    qDebug("New Filter on Var Name: %s", arg1.toLatin1().data());
    if (! isResettingFilter)  {
        filterCTVars();
    }
}

void SearchVariable::on_txtNode_textChanged(const QString &arg1)
{
    qDebug("New Filter on Node ID: %s", arg1.toLatin1().data());
    if (! isResettingFilter)  {
        filterCTVars();
    }
}

void    SearchVariable::onRowClicked(const QModelIndex &index)
{
    int nRow = index.row();

    if (nRow >= 0)  {
        // Deduce il Numero di Riga assoluto in CT dal verticalHeader della griglia
        QTableWidgetItem    *tItem = ui->tblVariables->verticalHeaderItem(nRow);
        if (tItem != 0)  {
            bool fOk = false;
            nRow = tItem->text().toInt(&fOk);
            if (fOk)  {
                nSelectedRow = nRow;
                qDebug("SearchVariable::onRowClicked(): Row %d", nRow);
            }
        }
    }
}

void    SearchVariable::onRowDoubleClicked(const QModelIndex &index)
// Evento Row Double Clicked
{
    int nRow = index.row();

    if (nRow >= 0)  {
        // Deduce il Numero di Riga assoluto in CT dal verticalHeader della griglia
        QTableWidgetItem    *tItem = ui->tblVariables->verticalHeaderItem(nRow);
        if (tItem != 0)  {
            bool fOk = false;
            nRow = tItem->text().toInt(&fOk);
            if (fOk)  {
                nSelectedRow = nRow;
                qDebug("SearchVariable::onRowDoubleClicked(): Row %d", nRow);
                this->accept();
            }
        }
    }
}

