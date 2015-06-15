#ifndef ATCMANIMATIONDIALOG_H
#define ATCMANIMATIONDIALOG_H

#include <QDialog>
#include <QTableWidget>

class QDialogButtonBox;

class ATCManimation;

class atcmanimationDialog : public QDialog
{
	Q_OBJECT

	public:
		atcmanimationDialog(ATCManimation *plugin = 0, QWidget *parent = 0);

		QSize sizeHint() const;

	private slots:
		void saveState();
		void addMapItem();
		void removeMapItem();
		void chooseResource(QTableWidgetItem* item);
        void chooseCtVariable();
        void chooseCtVisibility();

	private:
		QTableWidget *editor;
		ATCManimation *animation;
		QDialogButtonBox *buttonBox;
		QPushButton *buttonAdd;
		QPushButton *buttonRemove;
		QString m_mapping;
		QStringList m_maplist;
        QLineEdit * lineVariable;
        QLineEdit * lineVisibility;
};
#endif
