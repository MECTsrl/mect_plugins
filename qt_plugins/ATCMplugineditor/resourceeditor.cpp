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
    bool start = false;
    while (it.hasNext()) {
        QString item = it.next();
        if (item.compare(":/marker.png") == 0)
        {
            start = true;
        }


#if 1
        if (
                start == false
                ||
                item.startsWith(":/trolltech/")
                ||
                item.startsWith(":/core/")
                ||
                item.startsWith(":/skins/")
                ||
                item.startsWith(":/debugger/")
                ||
                item.startsWith(":/glsleditor/")
                ||
                item.startsWith(":/webkit/")
                ||
                item.startsWith(":/help/")
                ||
                item.startsWith(":/qmldesigner/")
                ||
                item.startsWith(":/qmlprofiler/")
                ||
                item.startsWith(":/qmlproject/")
                ||
                item.startsWith(":/qml/")
                ||
                item.startsWith(":/itemlibrary/")
                ||
                item.startsWith(":/wellcome/")
                ||
                item.startsWith(":/projectexplorer/")
                ||
                item.startsWith(":/extensionsystem/")
                ||
                item.startsWith(":/utils/")
                ||
                item.startsWith(":/fancyactionbar/")
                ||
                item.startsWith(":/locator/")
                ||
                item.startsWith(":/find/")
                ||
                item.startsWith(":/texteditor/")
                ||
                item.startsWith(":/codemodel/")
                ||
                item.startsWith(":/qmljs/")
                ||
                item.startsWith(":/proparser/")
                ||
                item.startsWith(":/formeditor/")
                ||
                item.startsWith(":/welcome/")
                ||
                item.startsWith(":/navigator/")
                ||
                item.startsWith(":/qt-maemo/")
                ||
                item.startsWith(":/pixmaps/qwt")
                ||
                item.startsWith(":/icon/layout/")
                ||
                item.startsWith(":/icon/tool/")
                ||
                item.startsWith(":/resourceeditor/")
                ||
                !item.endsWith(".png")
           )
        {
            continue;
        }
#endif
        QListWidgetItem * m_item = new QListWidgetItem(QIcon(item), item);
        list->addItem(m_item);
    }

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
            | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(chooseCTresource()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

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
