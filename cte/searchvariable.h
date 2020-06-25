#ifndef SEARCHVARIABLE_H
#define SEARCHVARIABLE_H

#include <QDialog>

namespace Ui {
class SearchVariable;
}

class SearchVariable : public QDialog
{
    Q_OBJECT

public:
    explicit SearchVariable(QWidget *parent = 0);
    ~SearchVariable();

private:
    Ui::SearchVariable *ui;
};

#endif // SEARCHVARIABLE_H
