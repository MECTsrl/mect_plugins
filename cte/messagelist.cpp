#include "messagelist.h"
#include "ui_messagelist.h"
#include "utils.h"

#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFont>
#include <QFontMetrics>
#include <QChar>
#include <QColor>
#include <QBrush>

messageList::messageList(const QString &szTitle, QString &szMessage, QList<QStringList> &lstTableRows, QList<int> colSizes, int  nButtons, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::messageList)
{
    int         nRow = 0;
    int         nCol = 0;
    QString     szTemp;
    QStringList lstRow;
    QColor      cSfondo = QColor(210,255,255,255);           // Azzurro
    QBrush      bCell(cSfondo, Qt::SolidPattern);

    ui->setupUi(this);
    ui->buttonBox->setStandardButtons((QDialogButtonBox::StandardButtons) nButtons);
    // Window Title
    this->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint| Qt::CustomizeWindowHint);
    this->setWindowTitle(szTitle);
    // Window Message
    ui->lblMessage->setText(szMessage);
    // Window Grid
    if (lstTableRows.count() > 0)  {
        for (nRow = 0; nRow < lstTableRows.count(); nRow++)  {
            lstRow.clear();
            lstRow = lstTableRows[nRow];
            // Title of Grid
            if (nRow == 0)  {
                // La prima riga contiene le intestazioni del Grid
                ui->tblMessageList->setColumnCount(lstRow.count() );
                ui->tblMessageList->setRowCount(lstTableRows.count() - 1);
                ui->tblMessageList->setHorizontalHeaderLabels(lstRow);
            }
            else {
                // Caricamento degli Item in Grid
                for (nCol = 0; nCol < lstRow.count(); nCol++)  {
                    szTemp = lstRow[nCol];
                    int nColSize = nCol >= 0 && nCol < colSizes.count() ? colSizes[nCol] : 20;
                    QTableWidgetItem    *tItem = new QTableWidgetItem(szTemp);
                    // Item Allineato a Sx o Dx in funzione del segno di Size
                    if (nColSize > 0)
                        tItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                    else
                        tItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
                    // Rende il valore non Editabile
                    tItem->setFlags(tItem->flags() ^ Qt::ItemIsEditable);
                    // Aggiunge Item a Grid
                    ui->tblMessageList->setItem(nRow -1, nCol, tItem);
                }
                // Impostazione del colore di sfondo
                setRowBackground(bCell, ui->tblMessageList->model(), nRow - 1);

            }
        }
        // Redim Colonne Grid
        QFontMetrics fm(ui->tblMessageList->font());
        for (nCol = 0; nCol < colSizes.count(); nCol++)  {
            if (nCol < ui->tblMessageList->columnCount())  {
                int nColWidth = abs(colSizes[nCol]);
                // Larghezza fissa per alcune colonne
                QString szTemp;
                szTemp.fill(QChar::fromAscii(88), nColWidth);
                nColWidth = fm.width(szTemp) * 1.2;
                ui->tblMessageList->setColumnWidth(nCol, nColWidth);
            }
        }
        int nColHeight = fm.height() * 2;
        QHeaderView *verticalHeader = ui->tblMessageList->verticalHeader();
        verticalHeader->setResizeMode(QHeaderView::Fixed);
        verticalHeader->setDefaultSectionSize(nColHeight);
        // Caratteristiche del Grid
        ui->tblMessageList->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->tblMessageList->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->tblMessageList->setAlternatingRowColors(true);
    }
    // Selezione della prima riga se esiste
    if (ui->tblMessageList->rowCount() > 0)
        ui->tblMessageList->selectRow(0);

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(button_clicked(QAbstractButton *)));
}

messageList::~messageList()
{
    delete ui;
}

void messageList::button_clicked(QAbstractButton* cmdButton)
{
    qDebug() << QString::fromAscii("messageList::button_clicked: %1") .arg(cmdButton->text());
    // Add here filter on Buttons
}
