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
	mystyle.append("QLineEdit"); \
	mystyle.append("{"); \
	mystyle.append("text-align: center;"); \
	mystyle.append("border-radius: 0px;"); \
	mystyle.append("border: 2px solid  rgb(0,0,255);"); \
	mystyle.append("background-color: rgb(255,255,255);"); \
	mystyle.append("color: rgb(0, 255, 255);"); \
	mystyle.append("background-repeat: no-repeat;"); \
    mystyle.append("font: 18pt \""FONT_TYPE"\";"); \
	mystyle.append("}"); \
	mystyle.append("QMessageBox"); \
	mystyle.append("{"); \
    mystyle.append("font: 18pt \""FONT_TYPE"\";"); \
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
    mystyle.append("font: 26pt \""FONT_TYPE"\";"); \
	mystyle.append("}"); \
	mystyle.append("QLabel#labelMax"); \
	mystyle.append("{"); \
    mystyle.append("    font: 18pt \""FONT_TYPE"\";"); \
	mystyle.append("    color: rgb(0,0,0);"); \
	mystyle.append("}"); \
	mystyle.append(""); \
	mystyle.append("QLineEdit#lineEditMax"); \
	mystyle.append("{"); \
	mystyle.append("    color: rgb(255,0,0);"); \
	mystyle.append("}"); \
	mystyle.append(""); \
	mystyle.append("QLabel#labelMin"); \
	mystyle.append("{"); \
    mystyle.append("    font: 18pt \""FONT_TYPE"\";"); \
	mystyle.append("    color: rgb(0,0,0);"); \
	mystyle.append("}"); \
	mystyle.append(""); \
	mystyle.append("QLineEdit#lineEditMin"); \
	mystyle.append("{"); \
	mystyle.append("    color: rgb(255,0,0);"); \
	mystyle.append("}"); \
	mystyle.append("QLineEdit#lineEditVal"); \
	mystyle.append("{"); \
    mystyle.append("    font: 32pt \""FONT_TYPE"\";"); \
	mystyle.append("    color: rgb(0,0,0);"); \
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
	_minf = 0;
	_maxf = 0;

	if (password)
	{
		ui->lineEditVal->setEchoMode(QLineEdit::Password);
		_minf = 0;
		_maxf = 0;
		_valuef = value;
		*_valuef = ERROR_VALUE;
	}
	else
	{
		ui->lineEditVal->setEchoMode(QLineEdit::Normal);
		_minf = min;
		_maxf = max;
		_valuef = value;
        if (def != NO_DEFAULT)
        {
            ui->lineEditVal->setText(QString("%1").setNum(def, 'f', decimal));
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
	_minf = 0;
	_maxf = 0;

	if (password)
	{
		ui->lineEditVal->setEchoMode(QLineEdit::Password);
		_mini = 0;
		_maxi = 0;
		_valuei = value;
		*_valuei = ERROR_VALUE;
	}
	else
	{
		ui->lineEditVal->setEchoMode(QLineEdit::Normal);
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
	_minf = 0;
	_maxf = 0;

	if (password)
	{
		ui->lineEditVal->setEchoMode(QLineEdit::Password);
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

	}
	else
	{
		ui->lineEditVal->setEchoMode(QLineEdit::Normal);
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

void numpad::reload()
{
	if (_mini < _maxi || _minf < _maxf)
	{
		if (inputtype == INTEGER)
		{
			ui->lineEditMax->setText(QString("%1").arg(_maxi));
			ui->lineEditMin->setText(QString("%1").arg(_mini));
		}
		else if (inputtype == DECIMAL)
		{
			ui->lineEditMax->setText(QString("%1").arg(_maxf));
			ui->lineEditMin->setText(QString("%1").arg(_minf));
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
    ui->lineEditVal->selectAll();
}

numpad::~numpad()
{
	delete ui;
}

void numpad::on_pushButton0_clicked()
{
	LOG_PRINT(info_e, "0\n");
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
#if 0
	if (ui->lineEditVal->text().toFloat() < _min || ui->lineEditVal->text().toFloat() > _max)
	{
		reject();
	}
	else
#endif
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
	if (inputtype != INTEGER)
	{
		ui->lineEditVal->insert(".");
	}
}

void numpad::on_pushButtonMinus_clicked()
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
