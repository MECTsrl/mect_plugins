#ifndef ATCMANIMATIONTASKMENU_H
#define ATCMANIMATIONTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCManimation;

class atcmanimationTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmanimationTaskMenu(ATCManimation *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

	private slots:
		void editState();

	private:
		QAction *editStateAction;
		ATCManimation *animation;
};

class atcmanimationTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmanimationTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

#endif
