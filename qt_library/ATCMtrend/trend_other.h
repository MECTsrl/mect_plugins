/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef TREND_OTHER_H
#define TREND_OTHER_H

#include <QMouseEvent>

#include "pagebrowser.h"

namespace Ui {
class trend_other;
}

class trend_other : public page
{
    Q_OBJECT

public:
    explicit trend_other(QWidget *parent = 0);
    ~trend_other();
    virtual void reload(void);
    virtual void updateData();
    void enableButtonUp(bool status);
    void enableButtonDown(bool status);
    void enableButtonLeft(bool status);
    void enableButtonRight(bool status);

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonOnline_clicked();
    void on_pushButtonScreenshot_clicked();
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();
    void on_pushButtonLeft_clicked();
    void on_pushButtonRight_clicked();
private:
    void mouseMoveEvent(QMouseEvent * e);

private:
    Ui::trend_other *ui;
    QWidget * tp;
};

#endif // TREND_OTHER_H
