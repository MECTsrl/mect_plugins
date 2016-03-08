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
    
    capsLock = false;
    reload();
}

void alphanumpad::reload()
{
    showUpper(capsLock);
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
    ui->lineEditVal->insert(ui->q->text());
}

void alphanumpad::on_w_clicked()
{
    ui->lineEditVal->insert(ui->w->text());
}

void alphanumpad::on_e_clicked()
{
    ui->lineEditVal->insert(ui->e->text());
}

void alphanumpad::on_r_clicked()
{
    ui->lineEditVal->insert(ui->r->text());
}

void alphanumpad::on_t_clicked()
{
    ui->lineEditVal->insert(ui->t->text());
}

void alphanumpad::on_y_clicked()
{
    ui->lineEditVal->insert(ui->y->text());
}

void alphanumpad::on_u_clicked()
{
    ui->lineEditVal->insert(ui->u->text());
}

void alphanumpad::on_i_clicked()
{
    ui->lineEditVal->insert(ui->i->text());
}

void alphanumpad::on_o_clicked()
{
    ui->lineEditVal->insert(ui->o->text());
}

void alphanumpad::on_p_clicked()
{
    ui->lineEditVal->insert(ui->p->text());
}

void alphanumpad::on_a_clicked()
{
    ui->lineEditVal->insert(ui->a->text());
}

void alphanumpad::on_s_clicked()
{
    ui->lineEditVal->insert(ui->s->text());
}

void alphanumpad::on_d_clicked()
{
    ui->lineEditVal->insert(ui->d->text());
}

void alphanumpad::on_f_clicked()
{
    ui->lineEditVal->insert(ui->f->text());
}

void alphanumpad::on_g_clicked()
{
    ui->lineEditVal->insert(ui->g->text());
}

void alphanumpad::on_h_clicked()
{
    ui->lineEditVal->insert(ui->h->text());
}

void alphanumpad::on_j_clicked()
{
    ui->lineEditVal->insert(ui->j->text());
}

void alphanumpad::on_k_clicked()
{
    ui->lineEditVal->insert(ui->k->text());
}

void alphanumpad::on_l_clicked()
{
    ui->lineEditVal->insert(ui->l->text());
}

void alphanumpad::on_z_clicked()
{
    ui->lineEditVal->insert(ui->z->text());
}

void alphanumpad::on_x_clicked()
{
    ui->lineEditVal->insert(ui->x->text());
}

void alphanumpad::on_c_clicked()
{
    ui->lineEditVal->insert(ui->c->text());
}

void alphanumpad::on_v_clicked()
{
    ui->lineEditVal->insert(ui->v->text());
}

void alphanumpad::on_b_clicked()
{
    ui->lineEditVal->insert(ui->b->text());
}

void alphanumpad::on_n_clicked()
{
    ui->lineEditVal->insert(ui->n->text());
}

void alphanumpad::on_m_clicked()
{
    ui->lineEditVal->insert(ui->m->text());
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

void alphanumpad::on_pushButtonCapsLock_clicked(bool checked)
{
    capsLock = checked;
    showUpper(checked);
}

void alphanumpad::showUpper(bool checked)
{
    if (checked)
    {
        ui->a->setText("a");
        ui->b->setText("b");
        ui->c->setText("c");
        ui->d->setText("d");
        ui->e->setText("e");
        ui->f->setText("f");
        ui->g->setText("g");
        ui->h->setText("h");
        ui->i->setText("i");
        ui->j->setText("j");
        ui->k->setText("k");
        ui->l->setText("l");
        ui->m->setText("m");
        ui->n->setText("n");
        ui->o->setText("o");
        ui->p->setText("p");
        ui->q->setText("q");
        ui->r->setText("r");
        ui->s->setText("s");
        ui->t->setText("t");
        ui->t->setText("u");
        ui->t->setText("v");
        ui->x->setText("x");
        ui->y->setText("y");
        ui->w->setText("w");
        ui->z->setText("z");
    }
    else
    {
        ui->a->setText("A");
        ui->b->setText("B");
        ui->c->setText("C");
        ui->d->setText("D");
        ui->e->setText("E");
        ui->f->setText("F");
        ui->g->setText("G");
        ui->h->setText("H");
        ui->i->setText("I");
        ui->j->setText("J");
        ui->k->setText("K");
        ui->l->setText("L");
        ui->m->setText("M");
        ui->n->setText("N");
        ui->o->setText("O");
        ui->p->setText("P");
        ui->q->setText("Q");
        ui->r->setText("R");
        ui->s->setText("S");
        ui->t->setText("T");
        ui->t->setText("U");
        ui->t->setText("V");
        ui->x->setText("X");
        ui->y->setText("Y");
        ui->w->setText("W");
        ui->z->setText("Z");
    }
}
