/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief dialog box to introduce alphanumeric string
 */

#include "app_logprint.h"
#include "alphanumpad.h"
#include "ui_alphanumpad.h"

#define SET_ALPHANUMPAD_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    mystyle.append("QLineEdit"); \
    mystyle.append("{"); \
    mystyle.append("text-align: center;"); \
    mystyle.append("border-radius: 0px;"); \
    mystyle.append("border: 2px solid  rgb(0,0,255);"); \
    mystyle.append("background-color: rgb(255,255,255);"); \
    mystyle.append("color: rgb(0, 0, 0);"); \
    mystyle.append("background-repeat: no-repeat;"); \
    mystyle.append("font: 18pt \"Ubuntu\";"); \
    mystyle.append("}"); \
    mystyle.append("QMessageBox"); \
    mystyle.append("{"); \
    mystyle.append("font: 18pt \"Ubuntu\";"); \
    mystyle.append("background-color: rgb(0, 0, 0);"); \
    mystyle.append("color: red;"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton"); \
    mystyle.append("{"); \
    mystyle.append("text-align: center;"); \
    mystyle.append("border-radius: 8px;"); \
    mystyle.append("border: 2px solid  rgb(121,121,121);"); \
    mystyle.append("background-color: rgb(255, 255, 255);"); \
    mystyle.append("color: rgb(81, 81, 81);"); \
    mystyle.append("background-repeat: no-repeat;"); \
    mystyle.append("font: 26pt \"Ubuntu\";"); \
    mystyle.append("}"); \
    mystyle.append("QLabel#labelMax"); \
    mystyle.append("{"); \
    mystyle.append("    font: 18pt \"Ubuntu\";"); \
    mystyle.append("    color: rgb(255,255,255);"); \
    mystyle.append("}"); \
    mystyle.append(""); \
    mystyle.append("QLineEdit#lineEditMax"); \
    mystyle.append("{"); \
    mystyle.append("    color: rgb(255,0,0);"); \
    mystyle.append("}"); \
    mystyle.append(""); \
    mystyle.append("QLabel#labelMin"); \
    mystyle.append("{"); \
    mystyle.append("    font: 18pt \"Ubuntu\";"); \
    mystyle.append("    color: rgb(255,255,255);"); \
    mystyle.append("}"); \
    mystyle.append(""); \
    mystyle.append("QLineEdit#lineEditMin"); \
    mystyle.append("{"); \
    mystyle.append("    color: rgb(255,0,0);"); \
    mystyle.append("}"); \
    mystyle.append("QLineEdit#lineEditVal"); \
    mystyle.append("{"); \
    mystyle.append("    font: 32pt \"Ubuntu\";"); \
    mystyle.append("}"); \
    this->setStyleSheet(mystyle); \
    }

alphanumpad::alphanumpad(char* value, char* def, bool password, QWidget *parent):
    QDialog(parent),
    ui(new Ui::alphanumpad)
{
    ui->setupUi(this);
    //SET_DIALOG_STYLE();
    SET_ALPHANUMPAD_STYLE();
    
    _value = value;
    
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        if (def != NULL)
        {
            ui->lineEditVal->setText(def);
            ui->lineEditVal->selectAll();
        }
    }
    
    reload();
}

void alphanumpad::reload()
{
}

alphanumpad::~alphanumpad()
{
    delete ui;
}

void alphanumpad::on_pushButton0_clicked()
{
    ui->lineEditVal->insert("0");
}

void alphanumpad::on_pushButton1_clicked()
{
    ui->lineEditVal->insert("1");
}

void alphanumpad::on_pushButton2_clicked()
{
    ui->lineEditVal->insert("2");
}

void alphanumpad::on_pushButton3_clicked()
{
    ui->lineEditVal->insert("3");
}

void alphanumpad::on_pushButton4_clicked()
{
    ui->lineEditVal->insert("4");
}

void alphanumpad::on_pushButton5_clicked()
{
    ui->lineEditVal->insert("5");
}

void alphanumpad::on_pushButton6_clicked()
{
    ui->lineEditVal->insert("6");
}

void alphanumpad::on_pushButton7_clicked()
{
    ui->lineEditVal->insert("7");
}

void alphanumpad::on_pushButton8_clicked()
{
    ui->lineEditVal->insert("8");
}

void alphanumpad::on_pushButton9_clicked()
{
    ui->lineEditVal->insert("9");
}

void alphanumpad::on_pushButtonClear_clicked()
{
    ui->lineEditVal->clear();
}

void alphanumpad::on_pushButtonEsc_clicked()
{
    reject();
}

void alphanumpad::on_pushButtonEnter_clicked()
{
    if (ui->lineEditVal->text().length() == 0)
    {
        reject();
    }
    else
    {
        strcpy(_value, ui->lineEditVal->text().toAscii().data());
        accept();
    }
}

void alphanumpad::on_q_clicked()
{
    ui->lineEditVal->insert("Q");
}

void alphanumpad::on_w_clicked()
{
    ui->lineEditVal->insert("W");
}

void alphanumpad::on_e_clicked()
{
    ui->lineEditVal->insert("E");
}

void alphanumpad::on_r_clicked()
{
    ui->lineEditVal->insert("R");
}

void alphanumpad::on_t_clicked()
{
    ui->lineEditVal->insert("T");
}

void alphanumpad::on_y_clicked()
{
    ui->lineEditVal->insert("Y");
}

void alphanumpad::on_u_clicked()
{
    ui->lineEditVal->insert("U");
}

void alphanumpad::on_i_clicked()
{
    ui->lineEditVal->insert("I");
}

void alphanumpad::on_o_clicked()
{
    ui->lineEditVal->insert("O");
}

void alphanumpad::on_p_clicked()
{
    ui->lineEditVal->insert("P");
}

void alphanumpad::on_a_clicked()
{
    ui->lineEditVal->insert("A");
}

void alphanumpad::on_s_clicked()
{
    ui->lineEditVal->insert("S");
}

void alphanumpad::on_d_clicked()
{
    ui->lineEditVal->insert("D");
}

void alphanumpad::on_f_clicked()
{
    ui->lineEditVal->insert("F");
}

void alphanumpad::on_g_clicked()
{
    ui->lineEditVal->insert("G");
}

void alphanumpad::on_h_clicked()
{
    ui->lineEditVal->insert("H");
}

void alphanumpad::on_j_clicked()
{
    ui->lineEditVal->insert("J");
}

void alphanumpad::on_k_clicked()
{
    ui->lineEditVal->insert("K");
}

void alphanumpad::on_l_clicked()
{
    ui->lineEditVal->insert("L");
}

void alphanumpad::on_z_clicked()
{
    ui->lineEditVal->insert("Z");
}

void alphanumpad::on_x_clicked()
{
    ui->lineEditVal->insert("X");
}

void alphanumpad::on_c_clicked()
{
    ui->lineEditVal->insert("C");
}

void alphanumpad::on_v_clicked()
{
    ui->lineEditVal->insert("V");
}

void alphanumpad::on_b_clicked()
{
    ui->lineEditVal->insert("B");
}

void alphanumpad::on_n_clicked()
{
    ui->lineEditVal->insert("N");
}

void alphanumpad::on_m_clicked()
{
    ui->lineEditVal->insert("M");
}


void alphanumpad::on_pushButtonBackspace_clicked()
{
    ui->lineEditVal->backspace();
}

void alphanumpad::on_pushButtonSingleQuote_clicked()
{
    ui->lineEditVal->insert("'");
}

void alphanumpad::on_pushButtonUnderscore_clicked()
{
    ui->lineEditVal->insert("_");
}

void alphanumpad::on_pushButtonSpace_clicked()
{
    ui->lineEditVal->insert(" ");
}

