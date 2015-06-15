#include <QtDesigner>
#include <QtGui>

#include "atcmcombobox.h"
#include "atcmcomboboxdialog.h"
#include "atcmcomboboxtaskmenu.h"

	atcmcomboboxTaskMenu::atcmcomboboxTaskMenu(ATCMcombobox *anim, QObject *parent)
: QObject(parent)
{
	combobox = anim;

	editStateAction = new QAction(tr("Edit Mapping..."), this);
	connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmcomboboxTaskMenu::editState()
{
	atcmcomboboxDialog dialog(combobox);
	dialog.exec();
}

QAction *atcmcomboboxTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmcomboboxTaskMenu::taskActions() const
{
	QList<QAction *> list;
	list.append(editStateAction);
	return list;
}

	atcmcomboboxTaskMenuFactory::atcmcomboboxTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmcomboboxTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMcombobox *anim = qobject_cast<ATCMcombobox*>(object))
		return new atcmcomboboxTaskMenu(anim, parent);

	return 0;
}
