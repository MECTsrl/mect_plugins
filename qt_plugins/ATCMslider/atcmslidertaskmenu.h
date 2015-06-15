#ifndef ATCMSLIDERTASKMENU_H
#define ATCMSLIDERTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMslider;

class atcmsliderTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmsliderTaskMenu(ATCMslider *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

    private slots:
        void editState();

	private:
        QAction *editStateAction;
        ATCMslider *slider;
};

class atcmsliderTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmsliderTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmsliderTaskMenu);
#endif
