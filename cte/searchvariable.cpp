#include "searchvariable.h"
#include "ui_searchvariable.h"

SearchVariable::SearchVariable(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchVariable)
{
    this->setVisible(false);
    ui->setupUi(this);
}

SearchVariable::~SearchVariable()
{
    delete ui;
}
