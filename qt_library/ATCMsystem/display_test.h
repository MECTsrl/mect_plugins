/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef DISPLAY_TEST_H
#define DISPLAY_TEST_H

#include "pagebrowser.h"

namespace Ui {
class display_test;
}

class display_test : public page
{
    Q_OBJECT

public:
    explicit display_test(QWidget *parent = 0);
    ~display_test();
    virtual void reload(void);
    virtual void updateData();

private slots:
#ifdef TRANSLATION
    void changeEvent(QEvent * event);
#endif
    void on_pushButton_clicked();

private:
    Ui::display_test *ui;
};

#endif // DISPLAY_TEST_H
