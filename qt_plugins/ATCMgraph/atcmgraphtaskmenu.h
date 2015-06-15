#ifndef ATCMGRAPHTASKMENU_H
#define ATCMGRAPHTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMgraph;

class atcmgraphTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmgraphTaskMenu(ATCMgraph *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

    private slots:
        void editState();

	private:
        QAction *editStateAction;
        ATCMgraph *graph;
};

class atcmgraphTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmgraphTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmgraphTaskMenu);
#endif
