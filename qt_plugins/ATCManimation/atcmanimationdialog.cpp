#include <QtGui>
#include <QtDesigner>

#include "atcmanimation.h"
#include "atcmanimationdialog.h"
#include "resourceeditor.h"
#include "crosstableeditor.h"

	atcmanimationDialog::atcmanimationDialog(ATCManimation *anim, QWidget *parent)
: QDialog(parent)
{
	animation = anim;
    editor = new QTableWidget(0,2);

	editor->setIconSize(QSize(30,30));

	QTableWidgetItem * item;

	item = new QTableWidgetItem(tr("Value"));
	editor->setHorizontalHeaderItem(0, item);
	item = new QTableWidgetItem(tr("Picture"));
	editor->setHorizontalHeaderItem(1, item);

	editor->setColumnWidth(0,30);
	editor->setColumnWidth(1,300);
	this->setGeometry(
			this->geometry().x(),
			this->geometry().y(),
			400,
			this->geometry().height()
			);

	/* Fill the table widget with the actual mapping */
	m_mapping = animation->mapping().trimmed();

	if (m_mapping.length() > 0)
	{
		m_maplist = m_mapping.split(";");
		for (int i = 0; i < m_maplist.count() - 1; i+=2)
		{
			if (m_maplist.at(i).length() > 0 && m_maplist.at(i+1).length() > 0)
			{
				editor->insertRow(i/2);
				item = new QTableWidgetItem(m_maplist.at(i));
				editor->setItem(i/2,0,item);
				item = new QTableWidgetItem(QIcon(m_maplist.at(i+1)), m_maplist.at(i+1));
				editor->setItem(i/2,1,item);
			}
		}
    }

	QLabel * labelVariable = new QLabel("Variable:");
	lineVariable = new QLineEdit(animation->variable().trimmed());
	QPushButton * buttonVariable = new QPushButton("...");
	buttonVariable->setFixedWidth(30);
	connect(buttonVariable, SIGNAL(clicked()), this, SLOT(chooseCtVariable()));

	QLabel * labelVisibility = new QLabel("VisibilityVar:");
	lineVisibility = new QLineEdit(animation->visibilityVar().trimmed());
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

	connect(editor, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(chooseResource(QTableWidgetItem*)));

	QVBoxLayout *groupboxLayout = new QVBoxLayout;
	groupboxLayout->addWidget(editor);
	groupboxLayout->addLayout(addRemoveLayout);

	QGroupBox *group = new QGroupBox(tr("Mapping:"));
	group->setLayout(groupboxLayout);
	group->layout()->setMargin(4);

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->addLayout(variablesLayout);
	mainLayout->addWidget(group);
	mainLayout->addWidget(buttonBox);

	setLayout(mainLayout);
	setWindowTitle(tr("Edit Animation Mapping"));
}

QSize atcmanimationDialog::sizeHint() const
{
	return QSize(250, 250);
}

void atcmanimationDialog::saveState()
{
	if (QDesignerFormWindowInterface *formWindow
			= QDesignerFormWindowInterface::findFormWindow(animation)) {

		/* set the mapping property using the content of the table widget */
		m_mapping.clear();
		//printf("%d\n", table->rowCount());
		for (int i = 0; i < editor->rowCount(); i++)
		{
		//printf("%d\n", table->rowCount());
            if (editor->item(i,0) && editor->item(i,0)->text().length() > 0 && editor->item(i,1)->text().length() > 0)
			{
				if (i > 0)
				{
					m_mapping = m_mapping + ";";
				}
				m_mapping = m_mapping + editor->item(i,0)->text() + ";" + editor->item(i,1)->text();
			}
		}
		formWindow->cursor()->setProperty("mapping", m_mapping.trimmed());
        formWindow->cursor()->setProperty("variable", lineVariable->text().trimmed());
        formWindow->cursor()->setProperty("visibilityVar", lineVisibility->text().trimmed());
    }
	accept();
}

void atcmanimationDialog::addMapItem()
{
    QTableWidgetItem * item;

    int i;
    if (editor->selectionModel()->selectedRows().count() > 0) {
        i = editor->currentRow();
    }
    else
    {
        i = editor->rowCount();
    }

    editor->insertRow(i);
    item = new QTableWidgetItem(QString("%1").setNum(i+1));
    editor->setItem(i,0,item);
    item = new QTableWidgetItem("");
    editor->setItem(i,1,item);
}

void atcmanimationDialog::removeMapItem()
{
    int i = editor->currentRow();
    if (i < 0)
    {
        i = editor->rowCount() - 1;
    }
    editor->removeRow(i);
}

void atcmanimationDialog::chooseResource(QTableWidgetItem* item)
{
	if (item->column() == 0)
	{
		return;
	}
	QString selection;
	ResourceEditor r(this, &selection);
	if (r.exec() == QDialog::Accepted)
	{
		item->setText(selection);
		item->setIcon(QIcon(selection));
	}
	/* create the listbox */
	/* populate the listbox */
	/* show the listbox */
}

void atcmanimationDialog::chooseCtVariable()
{
	QString value;
    CrossTableEditor dialog(animation, &value);
    if (dialog.exec() == Accepted)
	{
		lineVariable->setText(value);
	}
}

void atcmanimationDialog::chooseCtVisibility()
{
	QString value;
    CrossTableEditor dialog(animation, &value);
    if (dialog.exec() == Accepted)
	{
		lineVisibility->setText(value);
	}
}
