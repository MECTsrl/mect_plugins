#include "queryportnode.h"
#include "ui_queryportnode.h"
#include "utils.h"
#include "ctecommon.h"
#include "cteUtils.h"

#include <QIntValidator>
#include <QLatin1String>

queryPortNode::queryPortNode(const QString &szTitle, QString &szMessage, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::queryPortNode)
{
    int i = 0;
    int nPorts = 0;
    int nFirstPort = -1;

    ui->setupUi(this);
    // Window Title
    this->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint| Qt::CustomizeWindowHint);
    this->setWindowTitle(szTitle);
    // Window Message
    ui->lblMessage->setText(szMessage);
    // Items in Combo Port
    for (i = _serial0; i < _serialMax; i++)  {
        ui->cboPort->addItem(QString::fromLatin1("%1") .arg(i, 10, 10));
    }
    // Ricerca delle Porte libere per il protocollo RTU
    getFirstPortFromProtocol(RTU, nFirstPort, nPorts);
    ui->cboPort->setCurrentIndex(nFirstPort);
    // Abilitazione delle porte definite
    if (nPorts > 0)  {
        nPorts = enableSerialPortCombo(ui->cboPort);
    }
    ui->cboPort->setEnabled(nPorts > 1);
    // Default Node to 1
    ui->txtNode->setText(QLatin1String("1"));
    // Int Validator
    ui->txtNode->setValidator(new QIntValidator(1, nMaxRTUNodeID, this));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(validatePort()));
}

queryPortNode::~queryPortNode()
{
    delete ui;
}

void queryPortNode::getPortNode(int &nPort, int&nNode)
{
    bool    fOk = false;
    int     nUserNode = ui->txtNode->text().trimmed().toInt(&fOk);

    nNode = fOk ? nUserNode : 1;
    nPort = ui->cboPort->currentIndex();
}

void queryPortNode::validatePort()

{
    QString szTemp;
    bool    fOk = false;
    int     nUserNode = ui->txtNode->text().trimmed().toInt(&fOk);

    if ((nUserNode >= 1 && nUserNode <= nMaxRTUNodeID) && ui->cboPort->currentIndex() >= 0) {
        this->accept();
    }
    else  {
        szTemp = QLatin1String("Port Number Must be from 1 to ") + QString::number(nMaxRTUNodeID);
        warnUser(this, this->windowTitle(), szTemp);
        this->reject();
    }
}
