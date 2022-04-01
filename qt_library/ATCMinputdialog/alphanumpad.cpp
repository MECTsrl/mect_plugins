/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief dialog box to introduce alphanumeric string
 */

#include "app_logprint.h"
#include "alphanumpad.h"
#include "ui_alphanumpad.h"
#include "setstyle.h"

#define SET_ALPHANUMPAD_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    mystyle.append("QWidget"); \
    mystyle.append("{"); \
    mystyle.append("    background-color: rgb(10, 10, 10);"); \
    mystyle.append("}"); \
    mystyle.append("QLineEdit"); \
    mystyle.append("{"); \
    mystyle.append("text-align: center;"); \
    mystyle.append("border-radius: 0px;"); \
    mystyle.append("border: 2px solid  LemonChiffon;"); \
    mystyle.append("color: Cornsilk;"); \
    mystyle.append("background-color: DimGray;"); \
    mystyle.append("background-repeat: no-repeat;"); \
    mystyle.append("selection-background-color: rgb(87, 130, 193);"); \
    mystyle.append("selection-color: red;"); \
    mystyle.append("font: 30pt \"DejaVu Sans Mono\";"); \
    mystyle.append("}"); \
    mystyle.append("QMessageBox"); \
    mystyle.append("{"); \
    mystyle.append("    font: 12pt \"DejaVu Sans Mono\";"); \
    mystyle.append("    background-color: rgb(10, 10, 10);"); \
    mystyle.append("    color: red;"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton"); \
    mystyle.append("{"); \
    mystyle.append("text-align: center;"); \
    mystyle.append("border-radius: 8px;"); \
    mystyle.append("border: 2px solid  LemonChiffon;"); \
    mystyle.append("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 DarkGray, stop:1 rgb(22, 22, 22));"); \
    mystyle.append("color: LemonChiffon;"); \
    mystyle.append("background-repeat: no-repeat;"); \
    mystyle.append("font: 22pt \"DejaVu Sans Mono\";"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton:pressed"); \
    mystyle.append("{"); \
    mystyle.append("border: 2px solid  rgb(194, 194, 194);"); \
    mystyle.append("background-color:  rgb(255, 255, 127);"); \
    mystyle.append("color: rgb(70,70,80);"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton#pushButtonPassword"); \
    mystyle.append("{"); \
    mystyle.append("	image-position: center; "); \
    mystyle.append("	image: url(:/libicons/img/eye_invisible_32.png); "); \
    mystyle.append("	qproperty-focusPolicy: NoFocus;"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton#pushButtonPassword:checked"); \
    mystyle.append("{"); \
    mystyle.append("	image: url(:/libicons/img/eye_visible_32.png);"); \
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

    ui->pushButtonPassword->setChecked(false);
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
        ui->pushButtonPassword->setVisible(true);
    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        ui->pushButtonPassword->setVisible(false);
    }

    if (def != NULL)
    {
        ui->lineEditVal->setText(def);
    }

    capsLock = false;
    reload();
}



alphanumpad::alphanumpad(char* value, bool showSpecialChars, char* def, bool password, QWidget *parent):
    QDialog(parent),
    ui(new Ui::alphanumpad)
{
    ui->setupUi(this);
    //SET_DIALOG_STYLE();
    SET_ALPHANUMPAD_STYLE();
    
    _value = value;


    // Abilitazione del bottone per visualizzare i caratteri speciali
    ui->showSymbols->setVisible(showSpecialChars);
    ui->pushButtonPassword->setChecked(false);
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
        ui->pushButtonPassword->setVisible(true);
    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        ui->pushButtonPassword->setVisible(false);
    }

    if (def != NULL)
    {
        ui->lineEditVal->setText(def);
    }
    capsLock = false;
    reload();
}

void alphanumpad::reload()
{
    showUpper(capsLock);
    ui->showSymbols->setChecked(false);
    ui->lineEditVal->setFocus();
    clean = false;
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
    clean = true;
}

void alphanumpad::on_pushButtonEsc_clicked()
{
    reject();
}

void alphanumpad::on_pushButtonEnter_clicked()
{
    if (ui->lineEditVal->text().length() == 0 && clean == false)
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

void alphanumpad::on_pushButtonDot_clicked()
{
    ui->lineEditVal->insert(".");
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
        ui->u->setText("U");
        ui->v->setText("V");
        ui->x->setText("X");
        ui->y->setText("Y");
        ui->w->setText("W");
        ui->z->setText("Z");
    }
    else
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
        ui->u->setText("u");
        ui->v->setText("v");
        ui->x->setText("x");
        ui->y->setText("y");
        ui->w->setText("w");
        ui->z->setText("z");
    }
}

void alphanumpad::on_showSymbols_toggled(bool checked)
{
    if (checked) {
        ui->showSymbols->setText("2/2");
        ui->stackedKeys->setCurrentIndex(1);
    }
    else  {
        ui->showSymbols->setText("1/2");
        ui->stackedKeys->setCurrentIndex(0);
    }
}

void alphanumpad::on_pushButtonMinus_clicked()
{
    ui->lineEditVal->insert("-");
}

void alphanumpad::on_pushButtonPlus_clicked()
{
    ui->lineEditVal->insert("+");
}

void alphanumpad::on_pushButtonColon_clicked()
{
    ui->lineEditVal->insert(":");
}

void alphanumpad::on_pushButtonAsterisk_clicked()
{
    ui->lineEditVal->insert("*");
}

void alphanumpad::on_pushButtonEquals_clicked()
{
    ui->lineEditVal->insert("=");
}


void alphanumpad::on_pushButtonDollar_clicked()
{
    ui->lineEditVal->insert("$");
}

void alphanumpad::on_pushButtonCaret_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(94)));
}


void alphanumpad::on_pushButtonPipe_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(124)));
}

void alphanumpad::on_pushButtonSemicolon_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(59)));
}

void alphanumpad::on_pushButtonQuestionMark_clicked()
{
    ui->lineEditVal->insert("?");
}

void alphanumpad::on_pushButtonExclamationMark_clicked()
{
    ui->lineEditVal->insert("!");
}

void alphanumpad::on_pushButtonAtSign_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(64)));
}

void alphanumpad::on_pushButtonPercent_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(37)));
}

void alphanumpad::on_pushButtonLessThan_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(60)));
}

void alphanumpad::on_pushButtonMoreThan_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(62)));
}

void alphanumpad::on_pushButtonHash_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(35)));
}

void alphanumpad::on_pushButtonAmpersand_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(38)));
}

void alphanumpad::on_pushButtonSlash_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(47)));
}

void alphanumpad::on_pushButtonComma_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(44)));
}

void alphanumpad::on_pushButtonBackSlash_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(92)));
}

void alphanumpad::on_pushButtonOpenGraph_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(123)));
}

void alphanumpad::on_pushButtonCloseGraph_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(125)));
}

void alphanumpad::on_pushButtonOpenSquare_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(91)));
}

void alphanumpad::on_pushButtonCloseSquare_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(93)));
}

void alphanumpad::on_pushButtonOpenBracket_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(40)));
}

void alphanumpad::on_pushButtonCloseBracket_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(41)));
}

void alphanumpad::on_pushButtonTilde_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(126)));
}

void alphanumpad::on_pushButtonBacktick_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(96)));
}

void alphanumpad::on_pushButtonDoubleQuote_clicked()
{
    ui->lineEditVal->insert(QString(QChar::fromAscii(34)));
}

void alphanumpad::on_pushButtonPassword_clicked(bool checked)
{
    if (checked)  {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
    }
    else  {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
    }
}
