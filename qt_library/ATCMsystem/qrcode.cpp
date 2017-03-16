#include "qrcode.h"
#include "ui_qrcode.h"

#include <QFileInfo>
#include <QPixmap>
#include <QRect>

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

qrcode::qrcode(QString szFileQRC, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qrcode)
{
    ui->setupUi(this);
    this->setVisible(false);
    this->setWindowState(Qt::WindowFullScreen);
    QFileInfo fInfo(szFileQRC);
    if (fInfo.exists())  {
        QRect parentSize = parent->geometry();

        // get Parent Screen Dimensions
        int w = parentSize->width();
        int h = parentSize->height();

        // Find minimal size for Label
        int minSize = MIN(w, h);
        bool fLandScape = w > h;

        if (! fLandScape)  {
            minSize = minSize - ui->pushButtonBack->width();
        }
        // 2 Pixels space
        minSize -= 4;
        ui->lblQrCode->setMinimumSize(minSize, -minSize);
        ui->lblQrCode->updateGeometry();
        // Get final geometry of label
        w = ui->lblQrCode->width();
        h = ui->lblQrCode->height();
        // Loading QR Code Pixmap
        QPixmap myPix;
        myPix.load(szFileQRC);
        // set a scaled pixmap to a w x h window keeping its aspect ratio
        ui->lblQrCode->setPixmap(myPix.scaled(w,h,Qt::KeepAspectRatio));
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
