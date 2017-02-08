#include "qrcode.h"
#include "ui_qrcode.h"

#include <QFileInfo>
#include <QPixmap>

qrcode::qrcode(QString szFileQRC, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qrcode)
{
    ui->setupUi(this);
    this->setVisible(false);
    this->setWindowState(Qt::WindowFullScreen);
    QFileInfo fInfo(szFileQRC);
    if (fInfo.exists())  {
        QPixmap myPix;
        myPix.load(szFileQRC);
        ui->lblQrCode->setPixmap(myPix);
    }
}

qrcode::~qrcode()
{
    delete ui;
}

void qrcode::on_pushButtonBack_clicked()
{
    this->reject();
}
