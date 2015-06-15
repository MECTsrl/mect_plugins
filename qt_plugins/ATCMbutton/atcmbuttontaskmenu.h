#ifndef ATCMLABELTASKMENU_H
#define ATCMLABELTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMbutton;

class atcmbuttonTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmbuttonTaskMenu(ATCMbutton *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

    private slots:
        void editState();

	private:
        QAction *editStateAction;
        ATCMbutton *label;
};

class atcmbuttonTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmbuttonTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmbuttonTaskMenu);
#endif
