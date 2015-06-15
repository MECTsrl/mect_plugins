#ifndef ATCMVARIABLEANDVISIBILITYDIALOG_H
#define ATCMVARIABLEANDVISIBILITYDIALOG_H

#include <QDialog>
#include <QTableWidget>

class QDialogButtonBox;

class ATCMprogressbar;

class variableandvisibilityDialog : public QDialog
{
	Q_OBJECT

	public:
		variableandvisibilityDialog(ATCMprogressbar *plugin = 0, QWidget *parent = 0);

		QSize sizeHint() const;

	private slots:
		void saveState();
        void chooseCtVariable();
        void chooseCtVisibility();

	private:
		ATCMprogressbar *m_widget;
		QDialogButtonBox *buttonBox;
        QLineEdit * lineVariable;
        QLineEdit * lineVisibility;
};
#endif
