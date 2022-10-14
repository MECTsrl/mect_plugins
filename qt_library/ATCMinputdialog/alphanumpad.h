/**
 * @file
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
    explicit alphanumpad(char* value, bool showSpecialChars, char* def = NULL, bool password = false, QWidget *parent = 0);

    // char[] setter and getter
    void setValue(char* value);
    void getValue(char* value);
    // QString setter and getter
    void    setQStringValue(const QString &stringValue);
    QString getQStringValue() const { return ui->lineEditVal->text().trimmed(); }

    ~alphanumpad();
    void reload();

private slots:

    void on_showSymbols_toggled(bool checked);
    void showUpper(bool checked);

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

    void on_pushButtonDot_clicked();
    void on_pushButtonMinus_clicked();
    void on_pushButtonPlus_clicked();
    void on_pushButtonColon_clicked();
    void on_pushButtonAsterisk_clicked();
    void on_pushButtonEquals_clicked();
    void on_pushButtonDollar_clicked();
    void on_pushButtonCaret_clicked();
    void on_pushButtonPipe_clicked();
    void on_pushButtonSemicolon_clicked();
    void on_pushButtonQuestionMark_clicked();
    void on_pushButtonExclamationMark_clicked();
    void on_pushButtonAtSign_clicked();
    void on_pushButtonPercent_clicked();
    void on_pushButtonLessThan_clicked();
    void on_pushButtonMoreThan_clicked();
    void on_pushButtonHash_clicked();
    void on_pushButtonAmpersand_clicked();
    void on_pushButtonSlash_clicked();
    void on_pushButtonComma_clicked();
    void on_pushButtonBackSlash_clicked();
    void on_pushButtonOpenGraph_clicked();
    void on_pushButtonCloseGraph_clicked();
    void on_pushButtonOpenSquare_clicked();
    void on_pushButtonCloseSquare_clicked();
    void on_pushButtonOpenBracket_clicked();
    void on_pushButtonCloseBracket_clicked();
    void on_pushButtonTilde_clicked();
    void on_pushButtonBacktick_clicked();
    void on_pushButtonDoubleQuote_clicked();
    void on_pushButtonPassword_toggled(bool checked);
    void on_pushButtonCapsLock_toggled(bool checked);

private:
    Ui::alphanumpad *ui;
    char * _value;
    bool capsLock;
    bool clean;
};

#endif // alphanumpad
