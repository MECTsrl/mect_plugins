#ifndef CROSSTABLEEDITOR_H
#define CROSSTABLEEDITOR_H

#include <QDialog>
#include <QTableWidget>

class QDialogButtonBox;

class CrossTableEditor : public QDialog
{
	Q_OBJECT

	public:
		CrossTableEditor(QWidget *parent = 0, QString * selection = NULL);

		QSize sizeHint() const;

	private slots:
        void selectCTVariable(QTableWidgetItem* item);
        void chooseCTVariable();

	private:
		QTableWidget *table;
		QDialogButtonBox *buttonBox;
		QString * m_selection;
};

#endif // CROSSTABLEEDITOR_H
