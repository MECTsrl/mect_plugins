#ifndef ATCMCOMBOBOXDIALOG_H
#define ATCMCOMBOBOXDIALOG_H

#include <QDialog>
#include <QTableWidget>

class QDialogButtonBox;

class ATCMcombobox;

class atcmcomboboxDialog : public QDialog
{
	Q_OBJECT

	public:
		atcmcomboboxDialog(ATCMcombobox *plugin = 0, QWidget *parent = 0);

		QSize sizeHint() const;

	private slots:
		void saveState();
		void addMapItem();
		void removeMapItem();
        void chooseCtVariable();
        void chooseCtVisibility();

	private:
		QTableWidget *table;
		ATCMcombobox *combobox;
		QDialogButtonBox *buttonBox;
		QPushButton *buttonAdd;
		QPushButton *buttonRemove;
		QString m_mapping;
		QStringList m_maplist;
        QLineEdit * lineVariable;
        QLineEdit * lineVisibility;
};
#endif
