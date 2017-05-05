#ifndef CTEERRORLIST_H
#define CTEERRORLIST_H

#include <QDialog>
#include <QItemSelection>
#include <QList>
#include <QStringList>

#include "cteerrdef.h"

namespace Ui {
class cteErrorList;
}

class cteErrorList : public QDialog
{
    Q_OBJECT

public:
    explicit cteErrorList(QWidget *parent = 0, bool fSingleLine = false);
    ~cteErrorList();
    int     currentRow();
    int lstErrors2Grid(const QList<Err_CT> &lstErrors);      // Fill Error Grid with errors

private slots:
    void on_cmdLine_clicked();
    void on_cmdExit_clicked();
    void tableItemChanged(const QItemSelection & selected, const QItemSelection & deselected);

    void on_cmdClear_clicked();

private:
    QString list2CellValue(int nCol, const Err_CT &recErr);

    Ui::cteErrorList *ui;
    QStringList     lstCols;                            // Prompt Colonne
    QList<int>      lstSizes;                           // Width  Colonne
    int             m_nGridRow;                         // Riga corrente sul Grid;
    bool            m_fSingleLine;                      // Vero se il controllo è su una sola linea
};

#endif // CTEERRORLIST_H
