#ifndef QUERYPORTNODE_H
#define QUERYPORTNODE_H

#include <QDialog>
#include <QString>

namespace Ui {
class queryPortNode;
}

class queryPortNode : public QDialog
{
    Q_OBJECT

public:
    explicit queryPortNode(const QString &szTitle, QString &szMessage, QWidget *parent = 0);
    ~queryPortNode();
    void getPortNode(int &nPort, int&nNode);

private:
    Ui::queryPortNode *ui;
};

#endif // QUERYPORTNODE_H
