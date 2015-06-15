#ifndef ATCMSPINBOXTASKMENU_H
#define ATCMSPINBOXTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMspinbox;

class atcmspinboxTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmspinboxTaskMenu(ATCMspinbox *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

    private slots:
        void editState();

	private:
        QAction *editStateAction;
        ATCMspinbox *spinbox;
};

class atcmspinboxTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmspinboxTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmspinboxTaskMenu);
#endif
