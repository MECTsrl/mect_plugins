#ifndef ATCMVARIABLEANDVISIBILITYDIALOG_H
#define ATCMVARIABLEANDVISIBILITYDIALOG_H

#include <QDialog>
#include <QTableWidget>

class QDialogButtonBox;

class ATCMbutton;

class variableandvisibilityDialog : public QDialog
{
	Q_OBJECT

	public:
		variableandvisibilityDialog(ATCMbutton *plugin = 0, QWidget *parent = 0);

		QSize sizeHint() const;

	private slots:
		void saveState();
        void chooseCtVariable();
        void chooseCtVisibility();
        void chooseCtPassword();

	private:
		ATCMbutton *m_widget;
		QDialogButtonBox *buttonBox;
        QLineEdit * lineVariable;
        QLineEdit * lineVisibility;
        QLineEdit * linePassword;
};
#endif
