#include <QtDesigner>
#include <QtGui>

#include "atcmslider.h"
#include "variableandvisibilitydialog.h"
#include "atcmslidertaskmenu.h"
#include "crosstableeditor.h"

	atcmsliderTaskMenu::atcmsliderTaskMenu(ATCMslider *anim, QObject *parent)
: QObject(parent)
{
	slider = anim;

    editStateAction = new QAction(tr("Edit ATCM Variables..."), this);
    connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmsliderTaskMenu::editState()
{
    variableandvisibilityDialog dialog(slider);
	dialog.exec();
}

QAction *atcmsliderTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmsliderTaskMenu::taskActions() const
{
	QList<QAction *> list;
    list.append(editStateAction);
    return list;
}

	atcmsliderTaskMenuFactory::atcmsliderTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmsliderTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMslider *anim = qobject_cast<ATCMslider*>(object))
		return new atcmsliderTaskMenu(anim, parent);

	return 0;
}
