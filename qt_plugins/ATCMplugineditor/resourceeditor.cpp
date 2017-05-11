#include <QtGui>
#include <QtDesigner>
#include <QDirIterator>

#include "resourceeditor.h"

ResourceEditor::ResourceEditor(QWidget *parent, QString * selection)
: QDialog(parent)
{
	list = new QListWidget(this);
    list->setIconSize(QSize(30,30));
    m_selection = selection;

    QDirIterator it(":", QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString item = it.next();

        if (item.endsWith(".png") && (item.startsWith(":/icons/") || item.startsWith(":/systemicons/")))
        {
             QListWidgetItem * m_item = new QListWidgetItem(QIcon(item), item);
             list->addItem(m_item);
        }
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
            | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(chooseCTresource()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(chooseCTresource()));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(list);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Resource Editor"));
}

QSize ResourceEditor::sizeHint() const
{
	return QSize(500, 250);
}

void ResourceEditor::chooseCTresource()
{
    if (m_selection != NULL)
    {
        *m_selection = list->currentItem()->text();
        accept();
    }
    else
    {
        reject();
    }
}
