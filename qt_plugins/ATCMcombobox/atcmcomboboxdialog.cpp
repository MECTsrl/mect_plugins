#include <QtGui>
#include <QtDesigner>

#include "atcmcombobox.h"
#include "atcmcomboboxdialog.h"
#include "crosstableeditor.h"

	atcmcomboboxDialog::atcmcomboboxDialog(ATCMcombobox *anim, QWidget *parent)
: QDialog(parent)
{
	combobox = anim;
    table = new QTableWidget(0,2);

	table->setEditTriggers(QAbstractItemView::DoubleClicked);

	QTableWidgetItem * item;

	item = new QTableWidgetItem(tr("Value"));
	table->setHorizontalHeaderItem(0, item);
	item = new QTableWidgetItem(tr("Label"));
	table->setHorizontalHeaderItem(1, item);

	/* Fill the table widget with the actual mapping */
	m_mapping = combobox->mapping().trimmed();

	if (m_mapping.length() > 0)
	{
		m_maplist = m_mapping.split(";");
		for (int i = 0; i < m_maplist.count() - 1; i+=2)
		{
			if (m_maplist.at(i).length() > 0 && m_maplist.at(i+1).length() > 0)
			{
				table->insertRow(i/2);
				item = new QTableWidgetItem(m_maplist.at(i));
				table->setItem(i/2,0,item);
				item = new QTableWidgetItem(m_maplist.at(i+1));
				table->setItem(i/2,1,item);
			}
		}
    }

	QLabel * labelVariable = new QLabel("Variable:");
	lineVariable = new QLineEdit(combobox->variable().trimmed());
	QPushButton * buttonVariable = new QPushButton("...");
	buttonVariable->setFixedWidth(30);
	connect(buttonVariable, SIGNAL(clicked()), this, SLOT(chooseCtVariable()));

	QLabel * labelVisibility = new QLabel("VisibilityVar:");
	lineVisibility = new QLineEdit(combobox->visibilityVar().trimmed());
	QPushButton * buttonVisibility = new QPushButton("...");
	buttonVisibility->setFixedWidth(30);
	connect(buttonVisibility, SIGNAL(clicked()), this, SLOT(chooseCtVisibility()));

	QGridLayout *variablesLayout = new QGridLayout;
	variablesLayout->addWidget(labelVariable,0,0);
	variablesLayout->addWidget(lineVariable,0,1);
	variablesLayout->addWidget(buttonVariable,0,2);
	variablesLayout->addWidget(labelVisibility,1,0);
	variablesLayout->addWidget(lineVisibility,1,1);
	variablesLayout->addWidget(buttonVisibility,1,2);

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
			| QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(saveState()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	buttonAdd = new QPushButton("+");
	connect(buttonAdd, SIGNAL(clicked()), this, SLOT(addMapItem()));
	buttonRemove = new QPushButton("-");
	connect(buttonRemove, SIGNAL(clicked()), this, SLOT(removeMapItem()));

	QHBoxLayout *addRemoveLayout = new QHBoxLayout;
	addRemoveLayout->addWidget(buttonRemove);
	addRemoveLayout->addWidget(buttonAdd);

	//connect(table, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(chooseResource(QTableWidgetItem*)));

	QVBoxLayout *groupboxLayout = new QVBoxLayout;
	groupboxLayout->addWidget(table);
	groupboxLayout->addLayout(addRemoveLayout);

	QGroupBox *group = new QGroupBox(tr("Mapping:"));
	group->setLayout(groupboxLayout);
	group->layout()->setMargin(4);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(variablesLayout);
	mainLayout->addWidget(group);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle(tr("Edit ATCM ComboBox Mapping"));
}

QSize atcmcomboboxDialog::sizeHint() const
{
	return QSize(250, 400);
}

void atcmcomboboxDialog::saveState()
{
	if (QDesignerFormWindowInterface *formWindow
			= QDesignerFormWindowInterface::findFormWindow(combobox)) {

		/* set the mapping property using the content of the table widget */
		m_mapping.clear();
		for (int i = 0; i < table->rowCount(); i++)
		{
            if (table->item(i,0) && table->item(i,0)->text().length() > 0)
			{
				if (i > 0)
				{
					m_mapping = m_mapping + ";";
				}
                if (table->item(i,1) && table->item(i,1)->text().length() > 0)
                {
                    m_mapping = m_mapping + table->item(i,0)->text() + ";" + table->item(i,1)->text();
                }
                else
                {
                    m_mapping = m_mapping + table->item(i,0)->text() + ";" + table->item(i,0)->text();
                }
			}
            else if (table->item(i,1) && table->item(i,1)->text().length() > 0)
            {
                QMessageBox::critical(this,tr("Invalid item"), tr("The item '%1' have a description ('%2') but not a value.").arg(i+1).arg(table->item(i,1)->text()));
                return;
            }
        }
		formWindow->cursor()->setProperty("mapping", m_mapping.trimmed());
		formWindow->cursor()->setProperty("variable", lineVariable->text().trimmed());
		formWindow->cursor()->setProperty("visibilityVar", lineVisibility->text().trimmed());
	}
	accept();
}

void atcmcomboboxDialog::addMapItem()
{
	QTableWidgetItem * item;
    int i;
    int nMaxItem = -1;

    //    if (table->selectionModel()->selectedRows().count() > 0) {
    //        i = table->currentRow();
    //    }
    //    else
    //    {
    //        i = table->rowCount();
    //    }
    // Searching Max Value in Table
    for (i = 0; i < table->rowCount(); i++)  {
        int nValue = table->item(i, 0)->text().toInt();
        if (nValue > nMaxItem)  {
            nMaxItem = nValue;
        }
    }
    i = table->rowCount();
    table->insertRow(i);
    item = new QTableWidgetItem(QString("%1").setNum(nMaxItem + 1));
    table->setItem(i, 0, item);
	item = new QTableWidgetItem("");
    table->setItem(i, 1, item);
}

void atcmcomboboxDialog::removeMapItem()
{
    int i = table->currentRow();
    if (i < 0)
    {
        i = table->rowCount() - 1;
    }
    table->removeRow(i);
}

void atcmcomboboxDialog::chooseCtVariable()
{
    QString value = lineVariable->text();
    CrossTableEditor dialog(combobox, &value);
    if (dialog.exec() == Accepted)
	{
		lineVariable->setText(value);
	}
}

void atcmcomboboxDialog::chooseCtVisibility()
{
    QString value = lineVisibility->text();
    CrossTableEditor dialog(combobox, &value);
    if (dialog.exec() == Accepted)
	{
		lineVisibility->setText(value);
	}
}
