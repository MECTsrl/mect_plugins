#ifndef ATCMVARIABLEANDVISIBILITYDIALOG_H
#define ATCMVARIABLEANDVISIBILITYDIALOG_H

#include <QDialog>
#include <QTableWidget>

class QDialogButtonBox;

class ATCMspinbox;

class variableandvisibilityDialog : public QDialog
{
	Q_OBJECT

	public:
		variableandvisibilityDialog(ATCMspinbox *plugin = 0, QWidget *parent = 0);

		QSize sizeHint() const;

	private slots:
		void saveState();
        void chooseCtVariable();
        void chooseCtVisibility();

	private:
		ATCMspinbox *m_widget;
		QDialogButtonBox *buttonBox;
        QLineEdit * lineVariable;
        QLineEdit * lineVisibility;
};
#endif
