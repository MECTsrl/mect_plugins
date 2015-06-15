/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief dialog box to introduce alphanumeric string
 */

#ifndef ALPHANUMPAD_H
#define ALPHANUMPAD_H

#include <QDialog>

namespace Ui {
class alphanumpad;
}

class alphanumpad : public QDialog
{
    Q_OBJECT
    
public:
    explicit alphanumpad(char* value, char* def = NULL, bool password = false, QWidget *parent = 0);

    ~alphanumpad();
    void reload();

private slots:
    void on_pushButton1_clicked();
    void on_pushButton2_clicked();
    void on_pushButton3_clicked();
    void on_pushButton4_clicked();
    void on_pushButton5_clicked();
    void on_pushButton6_clicked();
    void on_pushButton7_clicked();
    void on_pushButton8_clicked();
    void on_pushButton9_clicked();
    void on_pushButton0_clicked();

    void on_pushButtonClear_clicked();
    void on_pushButtonEnter_clicked();
    void on_pushButtonEsc_clicked();

    void on_q_clicked();
    void on_w_clicked();
    void on_e_clicked();
    void on_r_clicked();
    void on_t_clicked();
    void on_y_clicked();
    void on_u_clicked();
    void on_s_clicked();
    void on_d_clicked();
    void on_f_clicked();
    void on_g_clicked();
    void on_h_clicked();
    void on_j_clicked();
    void on_k_clicked();
    void on_l_clicked();
    void on_z_clicked();
    void on_x_clicked();
    void on_c_clicked();
    void on_v_clicked();
    void on_b_clicked();
    void on_n_clicked();
    void on_m_clicked();
    void on_i_clicked();
    void on_o_clicked();
    void on_p_clicked();
    void on_a_clicked();
    void on_pushButtonBackspace_clicked();

    void on_pushButtonSingleQuote_clicked();

    void on_pushButtonUnderscore_clicked();

    void on_pushButtonSpace_clicked();

private:
    Ui::alphanumpad *ui;
    char * _value;
};

#endif // alphanumpad
