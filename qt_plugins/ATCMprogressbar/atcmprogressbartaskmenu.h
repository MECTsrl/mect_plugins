#ifndef ATCMPROGRESSBARTASKMENU_H
#define ATCMPROGRESSBARTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMprogressbar;

class atcmprogressbarTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmprogressbarTaskMenu(ATCMprogressbar *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

    private slots:
        void editState();

	private:
        QAction *editStateAction;
        ATCMprogressbar *progressbar;
};

class atcmprogressbarTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmprogressbarTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmprogressbarTaskMenu);
#endif
