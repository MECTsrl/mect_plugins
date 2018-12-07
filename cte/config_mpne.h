#ifndef CONFIG_MPNE_H
#define CONFIG_MPNE_H

#include "parser.h"
#include "ctecommon.h"

#include <QWidget>
#include <QPushButton>
#include <QLabel>

class Config_MPNE : public QWidget
{
    Q_OBJECT
public:
    explicit Config_MPNE(QWidget *parent = 0);

signals:

public slots:
    void    showTestaNodi(int nTesta, QList<int> lstMPNE);

private slots:

private:
    //---------------------------------------------------------------------
    // Funzioni locali al modulo
    //---------------------------------------------------------------------

    //---------------------------------------------------------------------
    // Variabili varie
    //---------------------------------------------------------------------


};

#endif // CONFIG_MPNE_H
