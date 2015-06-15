#ifndef ATCMCOMBOBOXTASKMENU_H
#define ATCMCOMBOBOXTASKMENU_H

#include <QDesignerTaskMenuExtension>
#include <QExtensionFactory>

class QAction;
class QExtensionManager;

class ATCMcombobox;

class atcmcomboboxTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
	Q_OBJECT
		Q_INTERFACES(QDesignerTaskMenuExtension)

	public:
		atcmcomboboxTaskMenu(ATCMcombobox *anim, QObject *parent);

		QAction *preferredEditAction() const;
		QList<QAction *> taskActions() const;

    private slots:
        void editState();

	private:
        QAction *editStateAction;
        ATCMcombobox *combobox;
};

class atcmcomboboxTaskMenuFactory : public QExtensionFactory
{
	Q_OBJECT

	public:
		atcmcomboboxTaskMenuFactory(QExtensionManager *parent = 0);

	protected:
		QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

//Q_DECLARE_METATYPE(atcmcomboboxTaskMenu);
#endif
