#include <QtDesigner>
#include <QtGui>

#include "atcmbutton.h"
#include "variableandvisibilitydialog.h"
#include "atcmbuttontaskmenu.h"
#include "crosstableeditor.h"

	atcmbuttonTaskMenu::atcmbuttonTaskMenu(ATCMbutton *anim, QObject *parent)
: QObject(parent)
{
	label = anim;

    editStateAction = new QAction(tr("Edit ATCM Variables..."), this);
    connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmbuttonTaskMenu::editState()
{
    variableandvisibilityDialog dialog(label);
    dialog.exec();
}

QAction *atcmbuttonTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmbuttonTaskMenu::taskActions() const
{
	QList<QAction *> list;
    list.append(editStateAction);
    return list;
}

	atcmbuttonTaskMenuFactory::atcmbuttonTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmbuttonTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMbutton *anim = qobject_cast<ATCMbutton*>(object))
		return new atcmbuttonTaskMenu(anim, parent);

	return 0;
}
