#ifndef SEARCHVARIABLE_H
#define SEARCHVARIABLE_H

#include <QDialog>
#include <QList>
#include <QStringList>

namespace Ui {
class SearchVariable;
}

class SearchVariable : public QDialog
{
    Q_OBJECT

public:
    explicit SearchVariable(QWidget *parent = 0);
    ~SearchVariable();
    int getSelectedVariable();

private slots:
    void on_cboSections_currentIndexChanged(int index);
    void resetFilters();

private:
    bool    ctable2Filter();
    bool    rec2show(QStringList &lstFields, int nRow);
    Ui::SearchVariable *ui;
    QList<int>      lstVisibleCols;
    QStringList     lstRowNumbers;
    bool            isResettingFilter;
};

#endif // SEARCHVARIABLE_H
