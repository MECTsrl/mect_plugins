#ifndef MESSAGELIST_H
#define MESSAGELIST_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QStringList>


namespace Ui {
class messageList;
}

class messageList : public QDialog
{
    Q_OBJECT

public:
    explicit messageList(const QString &szTitle, QString &szMessage, QList<QStringList> &lstTableRows, QList<int> colSizes, QWidget *parent = 0);
    ~messageList();

private:
    Ui::messageList *ui;
};

#endif // MESSAGELIST_H
