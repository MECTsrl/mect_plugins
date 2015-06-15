/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief dialog box to introduce numeric string
 * it can be float int or char*
 */

#ifndef NUMPAD_H
#define NUMPAD_H

#include <QDialog>

#include "common.h"
#include <math.h>

namespace Ui {
class numpad;
}

#define INTEGER 0
#define DECIMAL 1
#define STRING  2
#define NO_DEFAULT 0xFFFF

class numpad : public QDialog
{
    Q_OBJECT
    
public:
    explicit numpad(float* value, float def = NO_DEFAULT, int decimal = 4, float min = 0.0, float max = 0.0, bool password = false, QWidget *parent = 0);
    explicit numpad(int* value, int def = NO_DEFAULT, int min = 0, int max = 0, bool password = false, QWidget *parent = 0);
    explicit numpad(char* value, char* def = NULL, char *min = NULL, char *max = NULL, bool password = false, QWidget *parent = 0);

    ~numpad();
    void reload();

private slots:
    void on_pushButton0_clicked();

    void on_pushButton1_clicked();

    void on_pushButton2_clicked();

    void on_pushButton3_clicked();

    void on_pushButton4_clicked();

    void on_pushButton5_clicked();

    void on_pushButton6_clicked();

    void on_pushButton7_clicked();

    void on_pushButton8_clicked();

    void on_pushButton9_clicked();

    void on_pushButtonClear_clicked();

    void on_pushButtonEnter_clicked();

    void on_pushButtonEsc_clicked();

    void on_pushButtonDot_clicked();

    void on_pushButtonMinus_clicked();

private:
    Ui::numpad *ui;
    float _minf;
    float _maxf;
    float* _valuef;
    int _mini;
    int _maxi;
    int* _valuei;
    char _mins[DESCR_LEN];
    char _maxs[DESCR_LEN];
    char * _values;
    int inputtype;

};

#endif // NUMPAD_H
