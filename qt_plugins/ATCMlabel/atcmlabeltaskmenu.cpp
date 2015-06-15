#include <QtDesigner>
#include <QtGui>

#include "atcmlabel.h"
#include "variableandvisibilitydialog.h"
#include "atcmlabeltaskmenu.h"
#include "crosstableeditor.h"

	atcmlabelTaskMenu::atcmlabelTaskMenu(ATCMlabel *anim, QObject *parent)
: QObject(parent)
{
	label = anim;

	editStateAction = new QAction(tr("Edit ATCM Variables..."), this);
	connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmlabelTaskMenu::editState()
{
	variableandvisibilityDialog dialog(label);
	dialog.exec();
}

QAction *atcmlabelTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmlabelTaskMenu::taskActions() const
{
	QList<QAction *> list;
	list.append(editStateAction);
	return list;
}

	atcmlabelTaskMenuFactory::atcmlabelTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmlabelTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMlabel *anim = qobject_cast<ATCMlabel*>(object))
		return new atcmlabelTaskMenu(anim, parent);

	return 0;
}
