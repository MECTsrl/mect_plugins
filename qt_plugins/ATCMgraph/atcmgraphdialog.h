#ifndef ATCMGRAPHDIALOG_H
#define ATCMGRAPHDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QSignalMapper>

class QDialogButtonBox;

class ATCMgraph;

class atcmgraphDialog : public QDialog
{
	Q_OBJECT

	public:
		atcmgraphDialog(ATCMgraph *plugin = 0, QWidget *parent = 0);

		QSize sizeHint() const;

	private slots:
		void saveState();
        void chooseVariable(int index);

	private:
        QSignalMapper* mapper;
        ATCMgraph *m_widget;
		QDialogButtonBox *buttonBox;

        QLineEdit * lineX1Variable;
        QLineEdit * lineX1minVariable;
        QLineEdit * lineX1maxVariable;

        QLineEdit * lineY1Variable;
        QLineEdit * lineY1minVariable;
        QLineEdit * lineY1maxVariable;

        QLineEdit * lineDisplay1Variable;

        QLineEdit * lineX2Variable;
        QLineEdit * lineX2minVariable;
        QLineEdit * lineX2maxVariable;

        QLineEdit * lineY2Variable;
        QLineEdit * lineY2minVariable;
        QLineEdit * lineY2maxVariable;

        QLineEdit * lineDisplay2Variable;
};
#endif
