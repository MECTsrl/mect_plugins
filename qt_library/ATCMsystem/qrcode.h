#ifndef QRCODE_H
#define QRCODE_H
#include <QString>
#include <QDialog>

namespace Ui {
class qrcode;
}

class qrcode : public QDialog
{
    Q_OBJECT
    
public:
    explicit qrcode(QString szFileQRC, QWidget *parent = 0);
    ~qrcode();
    
private slots:

    void on_pushButtonBack_clicked();


private:
    Ui::qrcode *ui;
};

#endif // QRCODE_H
