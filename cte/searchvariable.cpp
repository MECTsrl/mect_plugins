#include "searchvariable.h"
#include "ui_searchvariable.h"
#include "cteUtils.h"
#include "utils.h"

#include <QIntValidator>

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
    lstVisibleCols.append(colComment);
    // Righe presenti
    lstRowNumbers.clear();
    // Azioni collegate ai pulsanti
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    resetFilters();
}

SearchVariable::~SearchVariable()
{
    delete ui;
}

int SearchVariable::getSelectedVariable()
{
    return -1;
}

bool SearchVariable::ctable2Filter()
{
    bool            fRes = false;
    int             nCur = 0;
    int             nCol = 0;
    QStringList     lstFields;

    // Reset dei filtri in corso, non deve fare nulla
    if (isResettingFilter)  {
        qWarning("Reset Filtro in corso, exit");
        return true;
    }
    // Preparazione tabella
    this->setCursor(Qt::WaitCursor);
    disableAndBlockSignals(ui->tblVariables);
    ui->tblVariables->setEnabled(false);
    ui->tblVariables->clearSelection();
    ui->tblVariables->setRowCount(0);
    ui->tblVariables->clear();
    ui->tblVariables->setColumnCount(colTotals);
    // Caricamento elementi
    lstRowNumbers.clear();
    for (nCur = 0; nCur < lstCTRecords.count(); nCur++)  {
        // Convert CT Record 2 User Values
        fRes = recCT2FieldsValues(lstCTRecords, lstFields, nCur);
        if (fRes)  {
            // Controllo se la riga corrente supera il filtro
            if (rec2show(lstFields, nCur))  {
                ui->tblVariables->insertRow(nCur);
                fRes = list2GridRow(ui->tblVariables, lstFields, lstHeadLeftCols, nCur);
                // Imposta Numero Riga
                lstRowNumbers.append(QString::fromAscii("%1") .arg(nCur + 1, 5, 10));
            }
        }
    }
    // Impostazione parametri TableView
    ui->tblVariables->setVerticalHeaderLabels(lstRowNumbers);
    setGridParams(ui->tblVariables, lstHeadCols, lstHeadSizes, QAbstractItemView::SingleSelection);
    // Nascondi Colonne non visibili
    for (nCol = 0; nCol < colTotals; nCol++)  {
        // Se la colonna non è tra quelle visibili la nasconde
        if (lstVisibleCols.indexOf(nCol) < 0)  {
            ui->tblVariables->setColumnHidden(nCol, true);
        }
    }
    // Return value
    enableAndUnlockSignals(ui->tblVariables);
    this->setCursor(Qt::ArrowCursor);
    qDebug("ctable2Grid(): Result %d", fRes);
    return fRes;
}

bool    SearchVariable::rec2show(QStringList &lstFields, int nRow)
{
    bool    f2Show = false;
    QString szNameFilter = ui->txtVarname->text().trimmed();
    QString szNode = ui->txtNode->text().trimmed();
    int     nSection = ui->cboSections->currentIndex();

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

    }
    // Tutti i controlli sono superati, il record deve essere mostrato
    f2Show = true;

endCheck:
    // Return value
    return f2Show;
}

void SearchVariable::on_cboSections_currentIndexChanged(int index)
{
    if (index >= 0)  {
        qDebug("New Filter on Section: %s", lstRegions[index].toLatin1().data());
    }
    else  {
        qDebug("Cleared Section Filter");
    }
    ctable2Filter();
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
    ui->chkWhole->setChecked(false);
    isResettingFilter = false;
}
