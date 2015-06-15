#include <QtDesigner>
#include <QtGui>

#include "atcmled.h"
#include "variableandvisibilitydialog.h"
#include "atcmledtaskmenu.h"
#include "crosstableeditor.h"

	atcmledTaskMenu::atcmledTaskMenu(ATCMled *anim, QObject *parent)
: QObject(parent)
{
	led = anim;

	editStateAction = new QAction(tr("Edit ATCM Variables..."), this);
	connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmledTaskMenu::editState()
{
	variableandvisibilityDialog dialog(led);
	dialog.exec();
}

QAction *atcmledTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmledTaskMenu::taskActions() const
{
	QList<QAction *> list;
	list.append(editStateAction);
	return list;
}

	atcmledTaskMenuFactory::atcmledTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmledTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMled *anim = qobject_cast<ATCMled*>(object))
		return new atcmledTaskMenu(anim, parent);

	return 0;
}
