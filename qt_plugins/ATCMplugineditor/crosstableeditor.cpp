#include <QtGui>
#include <QtDesigner>
#include <QDirIterator>
#include <QMessageBox>
#include <QTextStream>
#include <stdio.h>
#include "common.h"

#include "crosstableeditor.h"

CrossTableEditor::CrossTableEditor(QWidget *parent, QString * selection)
    : QDialog(parent)
{
    m_selection = selection;
    table = new QTableWidget(1,5);

    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QTableWidgetItem * item;

    item = new QTableWidgetItem(tr("Priority"));
    table->setHorizontalHeaderItem(0, item);
    item = new QTableWidgetItem(tr("Name"));
    table->setHorizontalHeaderItem(1, item);
    item = new QTableWidgetItem(tr("Type"));
    table->setHorizontalHeaderItem(2, item);
    item = new QTableWidgetItem(tr("Protocol"));
    table->setHorizontalHeaderItem(3, item);
    item = new QTableWidgetItem(tr("Description"));
    table->setHorizontalHeaderItem(4, item);

    /* extract the Project dir */
    QDesignerFormWindowInterface *formWindow
            = QDesignerFormWindowInterface::findFormWindow(parent);

    QString XtableFileName = formWindow->absoluteDir().absolutePath() + QString("/config/Crosstable.csv");
    /* open and load crosstable file */
    QFile file(XtableFileName);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
    }

    QTextStream in(&file);

    int i = 0;
    int j = 0;
    while(!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(";");
#if 1
        if (j >= DB_SIZE_ELEM - 1)
        {
            //QMessageBox::critical(0, "Crosstable error", tr("Too many line into cross table. Maximum line is %1.").arg(DB_SIZE_ELEM));
            break;
        }
#endif
        if (fields.count() != 13)
        {
            QMessageBox::critical(0, "Crosstable error", tr("Wrong number of fields at line %1: found %2 expecting 13.").arg(j).arg(fields.count()));
            break;
        }
        else
        {
            if (fields.at(0).toInt() != 0)
            {
                if (i > 0)
                {
                    table->insertRow(i);
                }
                item = new QTableWidgetItem(fields.at(1));
                table->setItem(i,0,item);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item = new QTableWidgetItem(fields.at(2).simplified());
                table->setItem(i,1,item);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item = new QTableWidgetItem(fields.at(3));
                table->setItem(i,2,item);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item = new QTableWidgetItem(fields.at(5));
                table->setItem(i,3,item);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                item = new QTableWidgetItem(fields.at(12));
                table->setItem(i,4,item);
                item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                i++;
            }
            j++;
        }
    }
    file.close();

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(chooseCTVariable()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(selectCTVariable(QTableWidgetItem*)));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(table);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Choose Cross-Table variale"));

    setStyleSheet(
                "    /*background-color: rgb(221;221;221);*/\n"
                "    /*background-image: url();*/\n"
                );

    QList<QTableWidgetItem*> items = table->findItems(*m_selection, Qt::MatchExactly);
    if (items.count() > 0)
    {
       items.at(0)->setSelected(true);
       table->scrollToItem(items.at(0),QAbstractItemView::PositionAtCenter);
    }
    table->resizeColumnsToContents();
}

QSize CrossTableEditor::sizeHint() const
{
    return QSize(500, 500);
}

void CrossTableEditor::selectCTVariable(QTableWidgetItem* item)
{
    if (m_selection != NULL && table->selectedItems().count() > 0 &&
            table->currentRow() >= 0 &&
            table->item(table->currentRow(),1)->text().trimmed().length() != 0)
    {
        *m_selection = table->item(item->row(),1)->text().trimmed();
        accept();
    }
    else
    {
        reject();
    }
}

void CrossTableEditor::chooseCTVariable()
{
    if (m_selection != NULL && table->selectedItems().count() > 0 &&
            table->currentRow() >= 0 &&
            table->item(table->currentRow(),1)->text().trimmed().length() != 0)
    {
        *m_selection = table->item(table->currentRow(),1)->text().trimmed();
        accept();
    }
    else
    {
        reject();
    }
}
