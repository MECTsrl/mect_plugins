#include <QtDesigner>
#include <QtGui>

#include "atcmanimation.h"
#include "atcmanimationdialog.h"
#include "atcmanimationtaskmenu.h"

	atcmanimationTaskMenu::atcmanimationTaskMenu(ATCManimation *anim, QObject *parent)
: QObject(parent)
{
	animation = anim;

	editStateAction = new QAction(tr("Edit Mapping..."), this);
	connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmanimationTaskMenu::editState()
{
	atcmanimationDialog dialog(animation);
	dialog.exec();
}

QAction *atcmanimationTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmanimationTaskMenu::taskActions() const
{
	QList<QAction *> list;
	list.append(editStateAction);
	return list;
}

	atcmanimationTaskMenuFactory::atcmanimationTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmanimationTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCManimation *anim = qobject_cast<ATCManimation*>(object))
		return new atcmanimationTaskMenu(anim, parent);

	return 0;
}
