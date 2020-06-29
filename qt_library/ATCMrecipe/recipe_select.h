/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef RECIPE_SELECT_H
#define RECIPE_SELECT_H

#include "pagebrowser.h"
#include <QListWidgetItem>

namespace Ui {
class recipe_select;
}

class recipe_select : public page
{
    Q_OBJECT
    
public:
    explicit recipe_select(QWidget *parent = 0);
    ~recipe_select();
    virtual void reload(void);
    virtual void updateData();

private slots:
#ifdef TRANSLATION
    void changeEvent(QEvent * event);
#endif
    void on_pushButtonHome_clicked();

    void on_pushButtonBack_clicked();

    void on_pushButtonSaveUSB_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

private:
    Ui::recipe_select *ui;
    char _recipe_dir_to_browse[FILENAME_MAX];
};

#endif // RECIPE_SELECT_H

