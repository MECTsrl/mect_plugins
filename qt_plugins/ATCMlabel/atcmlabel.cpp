/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM label base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QMessageBox>
#include <stdlib.h>

#include "atcmlabel.h"
#include "common.h"
#include "numpad.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#endif

ATCMlabel::ATCMlabel(QWidget *parent) :
    QPushButton(parent)
{
    m_value = "";
    m_variable = "";
    m_prefix = "";
    m_suffix = "";
    m_min = "NAN";
    m_max = "NAN";
    m_status = UNK;
    m_CtIndex = -1;
    m_CtVisibilityIndex = 0;
    m_objectstatus = false;
    m_visibilityvar = "";
    m_format = Dec;

    m_bgcolor = BG_COLOR_DEF;
    m_bgcolor_select = BG_COLOR_SEL_DEF;
    m_fontcolor = FONT_COLOR_DEF;
    m_fontcolor_select = FONT_COLOR_SEL_DEF;
    m_bordercolor = BORDER_COLOR_DEF;
    m_bordercolor_select = BORDER_COLOR_SEL_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setFlat(true);
    setStyle(new ATCMStyle);
#ifdef TARGET_ARM
    setToolTip("");
#endif

    /*
     * put there a default stylesheet
     */
    setStyleSheet(
            #ifndef ENABLE_STYLESHEET
                ""
            #else
                "/* property into normal status */\n"
                "QPushButton {\n"
                "    /* set the border width and color  */\n"
                "    /*border:2px solid black;*/\n"
                "    /* set the border radius */\n"
                "    /*border-radius:4px;*/\n"
                "    /* set the background image */\n"
                "    /*background-image: url();*/\n"
                "    /* set the image */\n"
                "    /*image: url();*/\n"
                "    /* set the icon image */\n"
                "    /*qproperty-icon: url();*/\n"
                "    /* set the icon image size */\n"
                "    /*qproperty-iconSize: 24px;*/\n"
                "    /* set the background color */\n"
                "    /*background-color: rgb(230, 230, 230);*/\n"
                "    /* set the text at normal status */\n"
                "    /*qproperty-text: \"text\";*/\n"
                "    /* set the font color */\n"
                "    /*color: black;*/\n"
                "}\n"
                "/* property into pressed status */\n"
                "QPushButton:pressed {\n"
                "    /* set the background color */\n"
                "    /*background-color: rgb(230, 230, 130);*/\n"
                "    /* set the border width and color  */\n"
                "    /*border:2px solid black;*/\n"
                "    /* set the border radius */\n"
                "    /*border-radius:4px;*/\n"
                "    /* set the text at pressed status */\n"
                "    /*qproperty-text: \"text pressed\";*/\n"
                "    /* set the font color */\n"
                "    /*color: black;*/\n"
                "    /* set the background image */\n"
                "    /*background-image: url();*/\n"
                "    /* set the image */\n"
                "    /*image: url();*/\n"
                "    /* set the icon image */\n"
                "    /*qproperty-icon: url();*/\n"
                "    /* set the icon image size */\n"
                "    /*qproperty-iconSize: 24px;*/\n"
                "}\n"
                "/* property into disabled status */\n"
                "QPushButton:disabled {\n"
                "    /* set the background color */\n"
                "    /*background-color: rgb(255, 255, 255);*/\n"
                "    /* set the border width and color  */\n"
                "    /*border:2px solid black;*/\n"
                "    /* set the border radius */\n"
                "    /*border-radius:4px;*/\n"
                "    /* set the text at pressed status */\n"
                "    /*qproperty-text: \"text disabled\";*/\n"
                "    /* set the font color */\n"
                "    /*color: rgb(155, 155, 155);*/\n"
                "    /* set the background image */\n"
                "    /*background-image: url();*/\n"
                "    /* set the image */\n"
                "    /*image: url();*/\n"
                "    /* set the icon image */\n"
                "    /*qproperty-icon: url();*/\n"
                "    /* set the icon image size */\n"
                "    /*qproperty-iconSize: 24px;*/\n"
                "}\n"
            #endif
                );

#ifdef TARGET_ARM
    if (m_refresh > 0)
    {
        refresh_timer = new QTimer(this);
        connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
        refresh_timer->start(m_refresh);
    }
    else
#endif
    {
        refresh_timer = NULL;
    }

    connect( this, SIGNAL( pressed() ), this, SLOT( writeAction() ) );
    connect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) );
}

ATCMlabel::~ATCMlabel()
{
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        delete refresh_timer;
    }
}

void ATCMlabel::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter painter(this);
    QPalette palette = this->palette();

    QStyleOptionButton opt;
    opt.init(this);

    /* text */
    opt.text = m_prefix + text() + m_suffix;

    if (m_objectstatus == false)
    {
        /* button color */
        palette.setColor(QPalette::Button, m_bgcolor);
        /* font color */
        palette.setColor(QPalette::ButtonText, m_fontcolor);
        /* border color */
        palette.setColor(QPalette::Foreground, m_bordercolor);

        if (m_apparence == QFrame::Raised)
        {
            opt.state = QStyle::State_Off | QStyle::State_Raised;
        }
        else if (m_apparence == QFrame::Sunken)
        {
            opt.state = QStyle::State_Off | QStyle::State_Sunken;
        }
        else
        {
            opt.state = QStyle::State_Off;
        }
    }
    else
    {
        /* button color */
        palette.setColor(QPalette::Button, m_bgcolor_select);
        /* font color */
        palette.setColor(QPalette::ButtonText, m_fontcolor_select);
        /* border color */
        palette.setColor(QPalette::Foreground, m_bordercolor_select);

        if (m_apparence == QFrame::Raised)
        {
            opt.state = QStyle::State_Off | QStyle::State_Sunken;
        }
        else if (m_apparence == QFrame::Sunken)
        {
            opt.state = QStyle::State_Off | QStyle::State_Raised;
        }
        else
        {
            opt.state = QStyle::State_Off;
        }
    }

#ifdef TARGET_ARM
    if (m_viewstatus)
    {
        /* draw the background color in funtion of the status */
        palette.setColor(QPalette::Foreground, Qt::red);
        switch(m_status)
        {
        case DONE:
            palette.setColor(QPalette::Foreground, Qt::green);
            break;
        case BUSY:
            palette.setColor(QPalette::Foreground, Qt::yellow);
            break;
        case ERROR:
            palette.setColor(QPalette::Foreground, Qt::red);
            break;
        default /*UNKNOWN*/:
            palette.setColor(QPalette::Foreground, Qt::gray);
            break;
        }
    }
#endif

    opt.palette = palette;
    _diameter_ = m_borderradius;
    _penWidth_ = m_borderwidth;

    style()->drawControl(QStyle::CE_PushButton, &opt, &painter);
}

void ATCMlabel::setViewStatus(bool status)
{
    m_viewstatus = status;
    update();
}

bool ATCMlabel::setVisibilityVar(QString visibilityVar)
{
    if (visibilityVar.trimmed().length() == 0)
    {
        m_visibilityvar.clear();
        m_CtVisibilityIndex = 0;
        return true;
    }
    else
    {
#ifdef TARGET_ARM
        int CtIndex;
        if (Tag2CtIndex(visibilityVar.trimmed().toAscii().data(), &CtIndex) == 0)
        {
            LOG_PRINT(verbose_e,"visibilityVar '%s', CtIndex %d\n", m_visibilityvar.trimmed().toAscii().data(), m_CtVisibilityIndex);
            m_CtVisibilityIndex = CtIndex;
#endif
            m_visibilityvar = visibilityVar.trimmed();
            if (m_refresh == 0)
            {
                setRefresh(DEFAULT_PLUGIN_REFRESH);
            }
            return true;
#ifdef TARGET_ARM
        }
        else
        {
            m_CtVisibilityIndex = 0;
            LOG_PRINT(error_e,"visibilityVar '%s', CtIndex %d\n", visibilityVar.trimmed().toAscii().data(), CtIndex);
            return false;
        }
#endif
    }
}

/* Activate variable */
bool ATCMlabel::setVariable(QString variable)
{
    /* if the acual variable is different from actual variable, deactivate it */
    if (m_variable.length() != 0 && variable.trimmed().compare(m_variable) != 0)
    {
#ifdef TARGET_ARM
        if (deactivateVar(m_variable.trimmed().toAscii().data()) == 0)
        {
#endif
            m_variable.clear();
            m_CtIndex = -1;
#ifdef TARGET_ARM
        }
#endif
    }

    /* if the acual variable is empty activate it */
    if (variable.trimmed().length() > 0)
    {
#ifdef TARGET_ARM
        if (true) // Patch for H Vars 2.0.12rc2  activateVar(variable.trimmed().toAscii().data()) == 0)
        {
            m_variable = variable.trimmed();
            if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
            {
                LOG_PRINT(error_e, "cannot extract ctIndex\n");
                m_status = ERROR;
                m_value = VAR_UNKNOWN;
                m_CtIndex = -1;
            }
            LOG_PRINT(verbose_e, "'%s' -> ctIndex %d\n", m_variable.toAscii().data(), m_CtIndex);
        }
        else
        {
            m_status = ERROR;
            m_value = VAR_UNKNOWN;
        }
#else
        m_variable = variable.trimmed();
#endif
    }

    if (m_status != ERROR)
    {
#ifndef TARGET_ARM
        setToolTip(m_variable);
#else
        setToolTip("");
#endif
        return true;
    }
    else
    {
        return false;
    }
}

QColor ATCMlabel::bgColor() const
{
    return m_bgcolor;
}

void ATCMlabel::setBgColor(const QColor& color)
{
    m_bgcolor = color;
    update();
}

QColor ATCMlabel::borderColor() const
{
    return m_bordercolor;
}

void ATCMlabel::setBorderColor(const QColor& color)
{
    m_bordercolor = color;
    update();
}

QColor ATCMlabel::fontColor() const
{
    return m_fontcolor;
}

void ATCMlabel::setFontColor(const QColor& color)
{
    m_fontcolor = color;
    update();
}

QColor ATCMlabel::bgSelectColor() const
{
    return m_bgcolor_select;
}

void ATCMlabel::setBorderWidth(int width)
{
    m_borderwidth = width;
    update();
}

void ATCMlabel::setBorderRadius(int radius)
{
    m_borderradius = radius;
    update();
}

bool ATCMlabel::setRefresh(int refresh)
{
    m_refresh = refresh;
#ifdef TARGET_ARM
    if (refresh_timer == NULL && m_refresh > 0)
    {
        refresh_timer = new QTimer(this);
        connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
        refresh_timer->start(m_refresh);
    }
    else if (m_refresh > 0)
    {
        refresh_timer->start(m_refresh);
    }
    else if (refresh_timer != NULL)
    {
        refresh_timer->stop();
    }
#endif
    return true;
}

/* read variable */
void ATCMlabel::updateData()
{
#ifdef TARGET_ARM
    char value[TAG_LEN] = "";

    if (m_CtVisibilityIndex > 0) {
        uint32_t visible = 0;
        if (readFromDb(m_CtVisibilityIndex, &visible) == 0) {
            m_status = DONE;
            if (visible && ! this->isVisible()) {
                this->setVisible(true);
            }
            else if (! visible && this->isVisible()) {
                this->setVisible(false);
            }
        }
    }
    if (! this->isVisible()) {
        return;
    }

    if (m_variable.length() == 0)
    {
        m_status = DONE;
        m_value = VAR_UNKNOWN;
    }
    else if (m_variable.length() > 0 && m_CtIndex >= 0)
    {
        if (formattedReadFromDb(m_CtIndex, value) == 0)
        {
            m_status = DONE;
            if (m_format == Bin)
            {
                m_value = QString::number(atoi(value), 2) + QString("b");

            }
            else if (m_format == Hex)
            {
                switch (CtIndex2Type(m_CtIndex))
                {
                case intab_e:
                case intba_e:
                {
                    int16_t val = strtol(value, NULL, 10);
                    m_value = QString("0x") + QString::number(val, 16);
                    break;
                }
                case uintab_e:
                case uintba_e:
                {
                    uint16_t val = strtoul(value, NULL, 10);
                    m_value = QString("0x") + QString::number(val, 16);
                    break;
                }
                case dint_abcd_e:
                case dint_badc_e:
                case dint_cdab_e:
                case dint_dcba_e:
                {
                    int32_t val = strtol(value, NULL, 10);
                    m_value = QString("0x") + QString::number(val, 16);
                    break;
                }
                case udint_abcd_e:
                case udint_badc_e:
                case udint_cdab_e:
                case udint_dcba_e:
                {
                    uint32_t val = strtoul(value, NULL, 10);
                    m_value = QString("0x") + QString::number(val, 16);
                    break;
                }
                case fabcd_e:
                case fbadc_e:
                case fcdab_e:
                case fdcba_e:
                    m_value = value;
                    break;
                default:
                    m_value = QString("0x") + QString::number(atoi(value), 16);
                    break;
                }
            }
            else
            {
                m_value = value;
            }
        }
        else
        {
            m_value = VAR_UNKNOWN;
            m_status = ERROR;
            LOG_PRINT(verbose_e,"variable '%s', CtIndex %d\n", m_variable.toAscii().data(), m_CtIndex);
        }
        this->setText(m_value);
    }
    else
    {
        m_status = ERROR;
        m_value = VAR_UNKNOWN;
        LOG_PRINT(error_e, "Invalid CtIndex %d for variable '%s' object '%s'\n", m_CtIndex, m_variable.toAscii().data(), this->objectName().toAscii().data());
    }
    LOG_PRINT(verbose_e, " %d '%s': '%s' status '%c' (BUSY '%c' - ERROR '%c' - DONE '%c')\n", m_CtIndex, m_variable.toAscii().data(), value, m_status, BUSY, ERROR, DONE);
#endif
    this->update();
}

bool ATCMlabel::startAutoReading()
{
#ifdef TARGET_ARM
    if (refresh_timer != NULL && m_refresh > 0)
    {
        refresh_timer->start(m_refresh);
        return true;
    }
    return false;
#else
    return true;
#endif
}

bool ATCMlabel::stopAutoReading()
{
#ifdef TARGET_ARM
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        return true;
    }
    return false;
#else
    return true;
#endif
}

void ATCMlabel::setBgSelectColor(const QColor& color)
{
    m_bgcolor_select = color;
    update();
}

QColor ATCMlabel::borderSelectColor() const
{
    return m_bordercolor_select;
}

void ATCMlabel::setBorderSelectColor(const QColor& color)
{
    m_bordercolor_select = color;
    update();
}

QColor ATCMlabel::fontSelectColor() const
{
    return m_fontcolor_select;
}

void ATCMlabel::setFontSelectColor(const QColor& color)
{
    m_fontcolor_select = color;
    update();
}


bool ATCMlabel::setMin(QString min)
{
    if (m_max.compare("NAN") == 0 || min.toFloat() < m_max.toFloat())
    {
        m_min = min;
        if (m_max.compare("NAN") == 0)
        {
            m_max = min;
        }
        return true;
    }
    else if (min.toFloat() == m_max.toFloat())
    {
        m_min = "0";
        m_max = "0";
        return true;
    }
    else
    {
        return false;
    }
}

bool ATCMlabel::setMax(QString max)
{
    if (m_min.compare("NAN") == 0 || max.toFloat() > m_min.toFloat())
    {
        m_max = max;
        if (m_min.compare("NAN") == 0)
        {
            m_min = max;
        }
        return true;
    }
    else if (max.toFloat() == m_min.toFloat())
    {
        m_min = "0";
        m_max = "0";
        return true;
    }
    else
    {
        return false;
    }
}

enum QFrame::Shadow ATCMlabel::apparence() const
{
    return m_apparence;
}

void ATCMlabel::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}

void ATCMlabel::setPrefix(QString prefix)
{
    m_prefix = prefix;
    update();
}

void ATCMlabel::setSuffix(QString suffix)
{
    m_suffix = suffix;
    update();
}

void ATCMlabel::writeAction()
{
    m_objectstatus = true;
    update();
#ifdef TARGET_ARM
    QString strvalue;
    if (m_CtIndex > 0)
    {
        int decimal = getVarDecimal(m_CtIndex);
        numpad * dk;
        switch (CtIndex2Type(m_CtIndex))
        {
        case intab_e:
        case intba_e:
        case uintab_e:
        case uintba_e:
        case dint_abcd_e:
        case dint_badc_e:
        case dint_cdab_e:
        case dint_dcba_e:
        case udint_abcd_e:
        case udint_badc_e:
        case udint_cdab_e:
        case udint_dcba_e:
        {
            if (decimal == 0)
            {
                int value  = 0, min = m_min.toInt(), max = m_max.toInt();
                dk = new numpad(&value, m_value.toInt(), min, max, (enum  input_fmt_e)m_format);
                dk->showFullScreen();

                if (dk->exec() == QDialog::Accepted)
                {
                    if (min < max && (value < min || value > max))
                    {
                        QMessageBox::critical(this,trUtf8("Invalid Data"), trUtf8("The value'%1' inserted is invalid.\nThe value must be between '%2' and '%3'").arg(value).arg(m_min).arg(m_max));
                        delete dk;
                        return;
                    }
                    strvalue = QString::number(value);

                }
                else
                {
                    delete dk;
                    return;
                }
                LOG_PRINT(verbose_e,"intero %s = %d\n", m_variable.toAscii().data(), value);
            }
            else
            {
                float value  = 0, min = m_min.toFloat(), max = m_max.toFloat();
                dk = new numpad(&value, m_value.toFloat(), decimal, min, max);
                dk->showFullScreen();

                if (dk->exec() == QDialog::Accepted)
                {
                    if (min < max && (value < min || value > max))
                    {
                        QMessageBox::critical(this,trUtf8("Invalid Data"), trUtf8("The value'%1' inserted is invalid.\nThe value must be between '%2' and '%3'").arg(value).arg(m_min).arg(m_max));
                        delete dk;
                        return;
                    }
                    strvalue = QString::number(value);

                }
                else
                {
                    delete dk;
                    return;
                }
                LOG_PRINT(verbose_e,"decimale %s = %f decimali %d\n", m_variable.toAscii().data(), value,decimal);
            }
        }
            break;
        case fabcd_e:
        case fbadc_e:
        case fcdab_e:
        case fdcba_e:
        {
            float value  = 0, min = m_min.toFloat(), max = m_max.toFloat();
            dk = new numpad(&value, m_value.toFloat(), decimal, min, max);
            dk->showFullScreen();

            if (dk->exec() == QDialog::Accepted)
            {
                if (min < max && (value < min || value > max))
                {
                    QMessageBox::critical(this,trUtf8("Invalid Data"), trUtf8("The value'%1' inserted is invalid.\nThe value must be between '%2' and '%3'").arg(value).arg(m_min).arg(m_max));
                    delete dk;
                    return;
                }
                strvalue = QString::number(value);

            }
            else
            {
                delete dk;
                return;
            }
            LOG_PRINT(verbose_e,"decimale %s = %f\n", m_variable.toAscii().data(), value);
        }
            break;
        default:
        {
            int value  = 0, min = 0, max = 1;
            dk = new numpad(&value, m_value.toInt(), min, max, (enum  input_fmt_e)m_format);
            dk = new numpad(&value, m_value.toInt(), min, max, (enum  input_fmt_e)m_format);
            dk->showFullScreen();

            if (dk->exec() == QDialog::Accepted)
            {
                if (min < max && (value < min || value > max))
                {
                    QMessageBox::critical(this,trUtf8("Invalid Data"), trUtf8("The value'%1' inserted is invalid.\nThe value must be between '%2' and '%3'").arg(value).arg(m_min).arg(m_max));
                    delete dk;
                    return;
                }
                strvalue = QString::number(value);

            }
            else
            {
                delete dk;
                return;
            }
            LOG_PRINT(verbose_e,"bit %s = %d\n", m_variable.toAscii().data(), value);
        }
            break;
        }
        writeValue(strvalue);
        delete dk;
    }
#endif
}

void ATCMlabel::releaseAction()
{
    m_objectstatus = false;
    update();
}

/* Write variable */
bool ATCMlabel::writeValue(QString value)
{
    if (m_variable.length() == 0)
    {
        m_value = value;
        this->setText(m_value);
        return false;
    }
#ifdef TARGET_ARM
    if (m_CtIndex >= 0 && setFormattedVarByCtIndex(m_CtIndex, value.toAscii().data()) == 0)
    {
        m_value = value;
        this->setText(m_value);
        return true;
    }
    else
    {
        this->setText(value);
        return false;
    }
#else
    m_value = value;
    this->setText(m_value);
    return true;
#endif
}

void ATCMlabel::unsetVariable()
{
    setVariable("");
}

void ATCMlabel::unsetPrefix()
{
    setPrefix("");
}

void ATCMlabel::unsetSuffix()
{
    setSuffix("");
}

void ATCMlabel::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
}

void ATCMlabel::unsetMin()
{
    setMin(0);
}

void ATCMlabel::unsetMax()
{
    setMax(0);
}

void ATCMlabel::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMlabel::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCMlabel::unsetApparence()
{
    setApparence(QFrame::Plain);
}

void ATCMlabel::setFormat(const enum ATCMLabelFormat format)
{
    m_format = format;
    update();
}
