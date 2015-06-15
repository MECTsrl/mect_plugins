#include <QtDesigner>
#include <QtGui>

#include "atcmgraph.h"
#include "atcmgraphdialog.h"
#include "atcmgraphtaskmenu.h"
#include "crosstableeditor.h"

	atcmgraphTaskMenu::atcmgraphTaskMenu(ATCMgraph *anim, QObject *parent)
: QObject(parent)
{
	graph = anim;

	editStateAction = new QAction(tr("Edit ATCM Graph..."), this);
	connect(editStateAction, SIGNAL(triggered()), this, SLOT(editState()));
}

void atcmgraphTaskMenu::editState()
{
	atcmgraphDialog dialog(graph);
	dialog.exec();
}

QAction *atcmgraphTaskMenu::preferredEditAction() const
{
	return editStateAction;
}

QList<QAction *> atcmgraphTaskMenu::taskActions() const
{
	QList<QAction *> list;
	list.append(editStateAction);
	return list;
}

	atcmgraphTaskMenuFactory::atcmgraphTaskMenuFactory(QExtensionManager *parent)
: QExtensionFactory(parent)
{
}

QObject *atcmgraphTaskMenuFactory::createExtension(QObject *object,
		const QString &iid,
		QObject *parent) const
{
	if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
		return 0;

    if (ATCMgraph *anim = qobject_cast<ATCMgraph*>(object))
		return new atcmgraphTaskMenu(anim, parent);

    return 0;
}
