#ifndef ATCMLABELTASKMENU_H
#define ATCMLABELTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMlabel;

class atcmlabelTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmlabelTaskMenu(ATCMlabel *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

	private slots:
		void editState();

	private:
		QAction *editStateAction;
		ATCMlabel *label;
};

class atcmlabelTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmlabelTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmlabelTaskMenu);
#endif
