/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef RECIPE_H
#define RECIPE_H

#include "pagebrowser.h"
#include <QList>
#include <QTableWidgetItem>
#include <global_var.h>

namespace Ui {
class recipe;
}

class recipe : public page
{
    Q_OBJECT
    
public:
    explicit recipe(QWidget *parent = 0);
    ~recipe();
    virtual void reload(void);
    virtual void updateData();

private slots:
#ifdef TRANSLATION
    void changeEvent(QEvent * event);
#endif
    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonRead_clicked();
    void on_pushButtonLoad_clicked();
    void on_pushButtonSave_clicked();
    void on_pushButtonUp_clicked();
    void on_pushButtonDown_clicked();
    void on_pushButtonLeft_clicked();
    void on_pushButtonRight_clicked();
    void on_tableWidget_itemClicked(QTableWidgetItem *item);
    void horizontalHeaderClicked(int column);
private:
    bool showRecipe(const char * familyName, const char * recipeName);
    bool getFamilyRecipe(const char * filename, char * familyName, char * recipeName);
private:
    Ui::recipe *ui;
    int current_row;
    int current_column;
    char _familyName[LINE_SIZE];
    char _recipeName[LINE_SIZE];
    int state;
};

#endif // RECIPE_H
