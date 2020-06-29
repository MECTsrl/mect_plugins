/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef STORE_H
#define STORE_H

#include "pagebrowser.h"

#define TMPDIR "/tmp/ram"

namespace Ui {
class store;
}

class store : public page
{
    Q_OBJECT
    
public:
    explicit store(QWidget *parent = 0);
    ~store();
    virtual void reload(void);
    virtual void updateData();

private slots:
#ifdef TRANSLATION
    void changeEvent(QEvent * event);
#endif
    void on_pushButtonHome_clicked();
    
    void on_pushButtonBack_clicked();
    
    void on_pushButtonUp_clicked();
    
    void on_pushButtonDown_clicked();
    
    void on_pushButtonLeft_clicked();
    
    void on_pushButtonRight_clicked();
    
    void on_pushButtonSaveUSB_clicked();
    
#if 0
    void on_pushButtonPrevious_clicked();
    
    void on_pushButtonNext_clicked();
#endif

    void on_pushButtonFilter_clicked();

private:
    void showLogHeder();
    void showLogRead(char ** outstruct);

private:
    Ui::store *ui;
    int status;
    int current_row;
    int current_column;
    char outputfile[FILENAME_MAX];
    time_t ti, tf;
    FILE * fpin;
private:
};

#endif // STORE_H


