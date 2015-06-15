#include <QtDesigner>
#include <QtGui>

#include "atcmspinbox.h"
#include "variableandvisibilitydialog.h"
#include "atcmspinboxtaskmenu.h"
#include "crosstableeditor.h"

	atcmspinboxTaskMenu::atcmspinboxTaskMenu(ATCMspinbox *anim, QObject *parent)
: QObject(parent)
{
	spinbox = anim;

    editStateAction = new QAction(tr("Edit ATCM Variables..."), this);
    connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmspinboxTaskMenu::editState()
{
    variableandvisibilityDialog dialog(spinbox);
	dialog.exec();
}

QAction *atcmspinboxTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmspinboxTaskMenu::taskActions() const
{
	QList<QAction *> list;
    list.append(editStateAction);
    return list;
}

	atcmspinboxTaskMenuFactory::atcmspinboxTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmspinboxTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMspinbox *anim = qobject_cast<ATCMspinbox*>(object))
		return new atcmspinboxTaskMenu(anim, parent);

	return 0;
}
