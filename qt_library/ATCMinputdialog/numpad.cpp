/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief dialog box to introduce numeric string
 * it can be float int or char*
 */

#include "app_logprint.h"
#include "numpad.h"
#include "ui_numpad.h"
#include "setstyle.h"

/**
 * @brief this macro is used to set the numpad style.
 * the syntax is html stylesheet-like
 */

#define SET_NUMPAD_STYLE() { \
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
    mystyle.append("color: LemonChiffon;"); \
    mystyle.append("background-color: DimGray;"); \
    mystyle.append("background-repeat: no-repeat;"); \
    mystyle.append("selection-background-color: DarkSlateGray;"); \
    mystyle.append("selection-color: Yellow;"); \
    mystyle.append("font: 18pt \""FONT_TYPE"\";"); \
    mystyle.append("}"); \
    mystyle.append("QMessageBox"); \
    mystyle.append("{"); \
    mystyle.append("    font: 18pt \""FONT_TYPE"\";"); \
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
    mystyle.append("font: 20pt \""FONT_TYPE"\";"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton:pressed"); \
    mystyle.append("{"); \
    mystyle.append("border: 2px solid  rgb(194, 194, 194);"); \
    mystyle.append("background-color:  rgb(255, 255, 127);"); \
    mystyle.append("color: rgb(70,70,80);"); \
    mystyle.append("}"); \
    mystyle.append("QPushButton#pushButtonPassword"); \
    mystyle.append("{"); \
    mystyle.append("	qproperty-focusPolicy: NoFocus;"); \
    mystyle.append("	background-repeat: no-repeat;"); \
    mystyle.append("	background-position: center;"); \
    mystyle.append("	background-image: url(:/libicons/img/eye_invisible_32.png); "); \
    mystyle.append("}"); \
    mystyle.append("QPushButton#pushButtonPassword:checked"); \
    mystyle.append("{"); \
    mystyle.append("	background-image: url(:/libicons/img/eye_visible_32.png);"); \
    mystyle.append("}"); \
    mystyle.append("QLabel#labelMax, QLabel#labelMin, QLabel#labelPrompt"); \
    mystyle.append("{"); \
    mystyle.append("    font: 18pt \""FONT_TYPE"\";"); \
    mystyle.append("    color: rgb(255, 237, 0);"); \
    mystyle.append("}"); \
    mystyle.append("QLineEdit#lineEditMax, QLineEdit#lineEditMin"); \
    mystyle.append("{"); \
    mystyle.append("    color: LawnGreen;"); \
    mystyle.append("}"); \
    mystyle.append("QLineEdit#lineEditVal"); \
    mystyle.append("{"); \
    mystyle.append("    font: 30pt \""FONT_TYPE"\";"); \
    mystyle.append("}"); \
    this->setStyleSheet(mystyle); \
    }




/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
numpad::numpad(float* value, float def, int decimal, float min, float max, bool password, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::numpad)
{
    ui->setupUi(this);

    //SET_DIALOG_STYLE();
    SET_NUMPAD_STYLE();

    ui->pushButtonA->hide();
    ui->pushButtonB->hide();
    ui->pushButtonC->hide();
    ui->pushButtonD->hide();
    ui->pushButtonE->hide();
    ui->pushButtonF->hide();

    base = 10;

    inputtype = DECIMAL;
    _mini = 0;
    _maxi = 0;
    _minuint = 0;
    _maxuint = 0;
    _minf = 0;
    _maxf = 0;

    ui->pushButtonPassword->setChecked(false);
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
        ui->pushButtonPassword->setVisible(true);
        _minf = 0;
        _maxf = 0;
        _valuef = value;
        *_valuef = ERROR_VALUE;
    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        ui->pushButtonPassword->setVisible(false);
        _minf = min;
        _maxf = max;
        _valuef = value;
        if (def != NO_DEFAULT)
        {
            ui->lineEditVal->setText(QString("%1").number(def, 'f', decimal));
        }
    }

    reload();
}

numpad::numpad(int* value, int def, int min, int max, enum input_fmt_e fmt, bool password, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::numpad)
{
    ui->setupUi(this);

    //SET_DIALOG_STYLE();
    SET_NUMPAD_STYLE();

    ui->pushButtonA->setVisible(fmt == input_hex);
    ui->pushButtonB->setVisible(fmt == input_hex);
    ui->pushButtonC->setVisible(fmt == input_hex);
    ui->pushButtonD->setVisible(fmt == input_hex);
    ui->pushButtonE->setVisible(fmt == input_hex);
    ui->pushButtonF->setVisible(fmt == input_hex);

    switch (fmt)
    {
    case input_dec:
        base = 10;
        break;
    case input_hex:
        base = 16;
        break;
    case input_bin:
        base = 2;
        break;
    default:
        base = 10;
        break;
    }

    inputtype = INTEGER;
    _mini = 0;
    _maxi = 0;
    _minuint = 0;
    _maxuint = 0;
    _minf = 0;
    _maxf = 0;

    ui->pushButtonPassword->setChecked(false);
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
        ui->pushButtonPassword->setVisible(true);
        _mini = 0;
        _maxi = 0;
        _valuei = value;
        *_valuei = ERROR_VALUE;
    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        ui->pushButtonPassword->setVisible(false);
        _mini = min;
        _maxi = max;
        _valuei = value;
        if (def != NO_DEFAULT)
        {
            ui->lineEditVal->setText(QString("%1").setNum(def, base));
        }
    }

    reload();
}

numpad::numpad(unsigned * value, unsigned  def, unsigned  min, unsigned  max, enum input_fmt_e fmt, bool password, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::numpad)
{
    ui->setupUi(this);

    //SET_DIALOG_STYLE();
    SET_NUMPAD_STYLE();

    ui->pushButtonA->setVisible(fmt == input_hex);
    ui->pushButtonB->setVisible(fmt == input_hex);
    ui->pushButtonC->setVisible(fmt == input_hex);
    ui->pushButtonD->setVisible(fmt == input_hex);
    ui->pushButtonE->setVisible(fmt == input_hex);
    ui->pushButtonF->setVisible(fmt == input_hex);
    ui->pushButtonMinus->setEnabled(false);

    switch (fmt)
    {
    case input_dec:
        base = 10;
        break;
    case input_hex:
        base = 16;
        break;
    case input_bin:
        base = 2;
        break;
    default:
        base = 10;
        break;
    }

    inputtype = UINTEGER;
    _mini = 0;
    _maxi = 0;
    _minuint = 0;
    _maxuint = 0;
    _minf = 0;
    _maxf = 0;

    ui->pushButtonPassword->setChecked(false);
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
        ui->pushButtonPassword->setVisible(true);
        _minuint = 0;
        _maxuint = 0;
        _valueuint = value;
        *_valueuint = ERROR_VALUE;
    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        ui->pushButtonPassword->setVisible(false);
        _minuint = min;
        _maxuint = max;
        _valueuint = value;
        if (def != NO_UINTDEFAULT)
        {
            ui->lineEditVal->setText(QString("%1").setNum(def, base));
        }
    }

    reload();

}


numpad::numpad(char* value, char* def, char* min, char* max, bool password, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::numpad)
{
    ui->setupUi(this);

    //SET_DIALOG_STYLE();
    SET_NUMPAD_STYLE();

    ui->pushButtonA->hide();
    ui->pushButtonB->hide();
    ui->pushButtonC->hide();
    ui->pushButtonD->hide();
    ui->pushButtonE->hide();
    ui->pushButtonF->hide();

    base = 10;

    inputtype = STRING;
    _mini = 0;
    _maxi = 0;
    _minuint = 0;
    _maxuint = 0;
    _minf = 0;
    _maxf = 0;

    ui->pushButtonPassword->setChecked(false);
    if (password)
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
        ui->pushButtonPassword->setVisible(true);
        if (min == NULL)
        {
            strcpy(_mins, "");
        }
        else
        {
            strcpy(_mins, min);
        }
        if (max == NULL)
        {
            strcpy(_maxs, "");
        }
        else
        {
            strcpy(_maxs, max);
        }
        if (value == NULL)
        {
            strcpy(_values, "");
        }
        else
        {
            strcpy(_values, value);
            _values[0] = '\0';
        }
        ui->lineEditVal->setText("");

    }
    else
    {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
        ui->pushButtonPassword->setVisible(false);
        if (min == NULL)
        {
            strcpy(_mins, "");
        }
        else
        {
            strcpy(_mins, min);
        }
        if (max == NULL)
        {
            strcpy(_maxs, "");
        }
        else
        {
            strcpy(_maxs, max);
        }
        _values = value;
        if (def != NULL)
        {
            ui->lineEditVal->setText(def);
        }
    }

    reload();
}

numpad::numpad(char*  value, int type, char* def, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::numpad)
{
    ui->setupUi(this);

    //SET_DIALOG_STYLE();
    SET_NUMPAD_STYLE();

    ui->pushButtonA->hide();
    ui->pushButtonB->hide();
    ui->pushButtonC->hide();
    ui->pushButtonD->hide();
    ui->pushButtonE->hide();
    ui->pushButtonF->hide();

    inputtype = type;

    ui->pushButtonPassword->setVisible(false);
    if (inputtype == DIALNB)
    {
        ui->pushButtonMinus->setText("*");
        ui->pushButtonDot->setText("#");
    }
    else
    {
        ui->pushButtonMinus->setText("-");
        ui->pushButtonDot->setText(".");
    }

    base = 10;
    _mini = 0;
    _maxi = 0;
    _minuint = 0;
    _maxuint = 0;
    _minf = 0;
    _maxf = 0;

    _values = value;
    if (def != NULL)
    {
        ui->lineEditVal->setText(def);
    }

    reload();
}

void numpad::reload()
{
    if (_mini < _maxi || _minf < _maxf || _minuint < _maxuint)
    {
        if (inputtype == INTEGER)
        {
            ui->lineEditMax->setText(QString("%1").arg(_maxi));
            ui->lineEditMin->setText(QString("%1").arg(_mini));
        }
        else if (inputtype == UINTEGER)
        {
            ui->lineEditMax->setText(QString("%1").arg(_maxuint));
            ui->lineEditMin->setText(QString("%1").arg(_minuint));
        }
        else if (inputtype == DECIMAL)
        {
            ui->lineEditMax->setText(QString("%1").arg(_maxf));
            ui->lineEditMin->setText(QString("%1").arg(_minf));
        }
        else if (inputtype == DIALNB)
        {
            ui->lineEditMax->setText(QString("%1").arg(_maxs));
            ui->lineEditMin->setText(QString("%1").arg(_mins));
        }
        else
        {
            ui->lineEditMax->setText(QString("%1").arg(_maxs));
            ui->lineEditMin->setText(QString("%1").arg(_mins));
        }
    }
    else
    {
        ui->lineEditMax->hide();
        ui->lineEditMin->hide();
        ui->labelMax->hide();
        ui->labelMin->hide();
    }
    ui->lineEditVal->setFocus();
}

numpad::~numpad()
{
    delete ui;
}

void numpad::on_pushButton0_clicked()
{
    LOG_PRINT(verbose_e, "0\n");
    ui->lineEditVal->insert("0");
}

void numpad::on_pushButton1_clicked()
{
    ui->lineEditVal->insert("1");
}

void numpad::on_pushButton2_clicked()
{
    ui->lineEditVal->insert("2");
}

void numpad::on_pushButton3_clicked()
{
    ui->lineEditVal->insert("3");
}

void numpad::on_pushButton4_clicked()
{
    ui->lineEditVal->insert("4");
}

void numpad::on_pushButton5_clicked()
{
    ui->lineEditVal->insert("5");
}

void numpad::on_pushButton6_clicked()
{
    ui->lineEditVal->insert("6");
}

void numpad::on_pushButton7_clicked()
{
    ui->lineEditVal->insert("7");
}

void numpad::on_pushButton8_clicked()
{
    ui->lineEditVal->insert("8");
}

void numpad::on_pushButton9_clicked()
{
    ui->lineEditVal->insert("9");
}

void numpad::on_pushButtonClear_clicked()
{
    ui->lineEditVal->backspace();
}

void numpad::on_pushButtonEsc_clicked()
{
    reject();
}

void numpad::on_pushButtonEnter_clicked()
{
        if (ui->lineEditVal->text().length() == 0)
        {
            reject();
        }
        else
        {
            bool ok = true;
            if (inputtype == DECIMAL && _valuef != NULL)
            {
                *_valuef = ui->lineEditVal->text().toFloat(&ok);
            }
            else if (inputtype == INTEGER && _valuei != NULL)
            {
                *_valuei = ui->lineEditVal->text().toInt(&ok, base);
            }
            else if (inputtype == UINTEGER && _valueuint != NULL)
            {
                *_valueuint = ui->lineEditVal->text().toUInt(&ok, base);
            }
            else if (inputtype == IPADDR)
            {
                QStringList ipaddr = ui->lineEditVal->text().split(".");
                if (ipaddr.count() != 4)
                {
                    ok = false;
                }
                else
                {
                    ok = true;
                    for (int i = 0; i < 4; i++)
                    {
                        if (ipaddr.at(i).toInt() < 0 || ipaddr.at(i).toInt() > 255)
                        {
                            ok = false;
                        }
                    }
                    if (ok)
                    {
                        strcpy(_values, ui->lineEditVal->text().toAscii().data());
                    }
                }
            }
            else
            {
                strcpy(_values, ui->lineEditVal->text().toAscii().data());
            }
            if (ok)
            {
                accept();
            }
            else
            {
                reject();
            }
        }
}

void numpad::on_pushButtonDot_clicked()
{
    if (inputtype == DIALNB)
    {
        ui->lineEditVal->insert("#");
    }
    else if (inputtype != INTEGER)
    {
        ui->lineEditVal->insert(".");
    }
}

void numpad::on_pushButtonMinus_clicked()
{
    if (inputtype == DIALNB)
    {
        ui->lineEditVal->insert("*");
    }
    else if (inputtype == STRING)
    {
        ui->lineEditVal->insert("-");
    }
    else if (inputtype == IPADDR)
    {
        return;
    }
    else
    {
        if (ui->lineEditVal->text().startsWith("-"))
        {
            ui->lineEditVal->setText(ui->lineEditVal->text().remove("-"));
        }
        else
        {
            ui->lineEditVal->setText("-" + ui->lineEditVal->text());
        }
    }
}

void numpad::on_pushButtonA_clicked()
{
    ui->lineEditVal->insert("A");
}

void numpad::on_pushButtonB_clicked()
{
    ui->lineEditVal->insert("B");
}

void numpad::on_pushButtonC_clicked()
{
    ui->lineEditVal->insert("C");
}

void numpad::on_pushButtonD_clicked()
{
    ui->lineEditVal->insert("D");
}

void numpad::on_pushButtonE_clicked()
{
    ui->lineEditVal->insert("E");
}

void numpad::on_pushButtonF_clicked()
{
    ui->lineEditVal->insert("F");
}

void numpad::on_pushButtonPassword_clicked(bool checked)
{
    if (checked)  {
        ui->lineEditVal->setEchoMode(QLineEdit::Normal);
    }
    else  {
        ui->lineEditVal->setEchoMode(QLineEdit::Password);
    }
}

int numpad::baseOfInputFormat(enum input_fmt_e fmt)
{
    int retVal = 10;

    switch (fmt)
    {
    case input_dec:
        retVal = 10;
        break;
    case input_hex:
        retVal = 16;
        break;
    case input_bin:
        retVal = 2;
        break;
    default:
        retVal = 10;
        break;
    }
    return retVal;
}

void    numpad::setPrompt(const QString &labelTitle)
{
    ui->labelPrompt->setText(labelTitle);
    ui->labelPrompt->setVisible(not labelTitle.isEmpty());
}
