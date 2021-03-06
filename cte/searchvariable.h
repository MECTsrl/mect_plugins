#ifndef SEARCHVARIABLE_H
#define SEARCHVARIABLE_H

#include <QDialog>
#include <QString>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QModelIndex>

namespace Ui {
class SearchVariable;
}

class SearchVariable : public QDialog
{
    Q_OBJECT

public:
    explicit SearchVariable(QWidget *parent = 0);
    ~SearchVariable();
    int     getSelectedVariable();
    void    refreshFilters(int nMainGridRow);

private slots:
    void on_cboSections_currentIndexChanged(int index);
    void on_cboPriority_currentIndexChanged(int index);
    void on_cboType_currentIndexChanged(int index);
    void on_cboProtocol_currentIndexChanged(int index);
    void on_chkCase_clicked(bool checked);
    void on_chkStartsWith_clicked(bool checked);
    void on_txtPort_textChanged(const QString &arg1);
    void on_txtNode_textChanged(const QString &arg1);
    void on_txtVarname_textChanged(const QString &arg1);
    void onRowClicked(const QModelIndex &index);
    void onRowDoubleClicked(const QModelIndex &index);
    void sortVarColumn(int nColumn);
    void resetFilters();
    void rowChanged();


    void on_cboUpdate_currentIndexChanged(int index);

private:
    bool    filterCTVars();
    bool    rec2show(QStringList &lstFields, int nRow);
    Ui::SearchVariable *ui;
    QList<int>      lstVisibleCols;
    // QStringList     lstRowNumbers;
    bool            isResettingFilter;
    int             nSelectedRow;
    bool            fCaseSensitive;
    bool            fNameStartsWith;
    QHash<QString, int> hashVariables;
};

#endif // SEARCHVARIABLE_H
