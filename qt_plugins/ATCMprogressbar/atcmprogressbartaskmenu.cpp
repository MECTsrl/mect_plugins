#include <QtDesigner>
#include <QtGui>

#include "atcmprogressbar.h"
#include "variableandvisibilitydialog.h"
#include "atcmprogressbartaskmenu.h"
#include "crosstableeditor.h"

	atcmprogressbarTaskMenu::atcmprogressbarTaskMenu(ATCMprogressbar *anim, QObject *parent)
: QObject(parent)
{
	progressbar = anim;

    editStateAction = new QAction(tr("Edit ATCM Variables..."), this);
    connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmprogressbarTaskMenu::editState()
{
    variableandvisibilityDialog dialog(progressbar);
	dialog.exec();
}

QAction *atcmprogressbarTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmprogressbarTaskMenu::taskActions() const
{
	QList<QAction *> list;
    list.append(editStateAction);
    return list;
}

	atcmprogressbarTaskMenuFactory::atcmprogressbarTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmprogressbarTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

	if (ATCMprogressbar *anim = qobject_cast<ATCMprogressbar*>(object))
		return new atcmprogressbarTaskMenu(anim, parent);

	return 0;
}
