/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM button base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QMessageBox>

#include "atcmbutton.h"
#include "common.h"
#include "numpad.h"
#include "atcmstyle.h"
#include "protocol.h"

#ifdef TARGET_ARM
#include "global_var.h"
#include <stdio.h>
#include "app_logprint.h"
#include "cross_table_utility.h"
#ifdef ENABLE_TREND
extern bool _trend_data_reload_;
extern char _actual_trend_[FILENAME_MAX];
#endif
#ifdef ENABLE_STORE
extern char _actual_store_[FILENAME_MAX];
#endif
#endif

#define RETRY_NB 20

ATCMbutton::ATCMbutton(QWidget * parent):
    QPushButton(parent)
{
    m_pagename = "";
    m_remember = true;
    m_status = STATUS_ENABLED;
    m_CtIndex = -1;
    m_CtVisibilityIndex = -1;
    m_CtPasswordVarIndex = -1;
    m_text = "";
    m_text_press = "";
    m_icon = QIcon();
    m_icon_press = QIcon();
    m_visibilityvar = "";
    m_passwordVar = "";
    m_passwordValue = "";
    m_viewstatus = false;
    m_statuspressval = 1;
    m_statusreleaseval = 0;
    m_justchanged = 0;
    m_forcedAction = false;
    m_statusactualval = m_statusreleaseval;

    m_bgcolor = BG_COLOR_SEL_DEF;
    m_bgcolor_press = BG_COLOR_SEL_DEF;
    m_fontcolor = FONT_COLOR_DEF;
    m_fontcolor_press = FONT_COLOR_SEL_DEF;
    m_bordercolor = BORDER_COLOR_DEF;
    m_bordercolor_press = BORDER_COLOR_SEL_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;

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
                "/* property into checked status */\n"
                "QPushButton:checked {\n"
                "    /* set the background color */\n"
                "    /*background-color: rgb(130, 130, 130);*/\n"
                "    /* set the border width and color  */\n"
                "    /*border:2px solid black;*/\n"
                "    /* set the border radius */\n"
                "    /*border-radius:4px;*/\n"
                "    /* set the text at cecked status */\n"
                "    /*qproperty-text: \"text checked\";*/\n"
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

    m_parent = parent;
    connect(m_parent, SIGNAL(varRefresh()), this, SLOT(updateData()));
    /* connect spostate in setStatusVar */
}

ATCMbutton::~ATCMbutton()
{
}

void ATCMbutton::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter painter(this);
    QPalette palette = this->palette();

    QStyleOptionButton opt;
    opt.init(this);

    if (isDown() == false && isChecked() == false)
    {
        /* button color */
        palette.setColor(QPalette::Button, m_bgcolor);
        /* font color */
        palette.setColor(QPalette::ButtonText, m_fontcolor);
        /* border color */
        palette.setColor(QPalette::Foreground, m_bordercolor);
        /* icon */
        opt.icon = m_icon;
        opt.iconSize = iconSize();
        /* text */
        opt.text = m_text;

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
        palette.setColor(QPalette::Button, m_bgcolor_press);
        /* font color */
        palette.setColor(QPalette::ButtonText, m_fontcolor_press);
        /* border color */
        palette.setColor(QPalette::Foreground, m_bordercolor_press);
        /* icon */
        opt.icon = m_icon_press;
        opt.iconSize = iconSize();
        /* text */
        opt.text = m_text_press;

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
    if (m_viewstatus) {
        /* draw the background color in funtion of the status */
        palette.setColor(QPalette::Foreground, Qt::red);
        if (m_status & STATUS_OK)
            palette.setColor(QPalette::Foreground, Qt::green);
        else if (m_status & (STATUS_BUSY_R | STATUS_BUSY_W))
            palette.setColor(QPalette::Foreground, Qt::yellow);
        else if (m_status & (STATUS_FAIL_W | STATUS_ERR))
            palette.setColor(QPalette::Foreground, Qt::red);
        else
            palette.setColor(QPalette::Foreground, Qt::gray);
    }
#endif

    opt.palette = palette;
    _diameter_ = m_borderradius;
    _penWidth_ = m_borderwidth;

    style()->drawControl(QStyle::CE_PushButton, &opt, &painter);
}

void ATCMbutton::setViewStatus(bool status)
{
    m_viewstatus = status;
    update();
}

bool ATCMbutton::setVisibilityVar(QString visibilityVar)
{
    if (visibilityVar.trimmed().length() == 0)
    {
        m_visibilityvar.clear();
        m_CtVisibilityIndex = -1;
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
            return true;
#ifdef TARGET_ARM
        }
        else
        {
            LOG_PRINT(verbose_e,"visibilityVar '%s', CtIndex %d\n", visibilityVar.trimmed().toAscii().data(), CtIndex);
            return false;
        }
#endif
    }
}

void ATCMbutton::unsetPageName()
{
    setPageName("");
}

void ATCMbutton::unsetRemember()
{
    setRemember(false);
}

void ATCMbutton::unsetPasswordVar()
{
    setPasswordVar("");
}

void ATCMbutton::unsetStatusvar()
{
    setStatusvar("");
}

void ATCMbutton::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMbutton::unsetVisibilityVar()
{
    setVisibilityVar("");
}

/* Activate variable */
bool ATCMbutton::setStatusvar(QString variable)
{
#ifdef TARGET_ARM
    disconnect( this, SIGNAL( toggled(bool) ), this, SLOT( toggleAction(bool) ));
    disconnect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) );
    disconnect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) );
    if (isCheckable())
    {
        connect( this, SIGNAL( toggled(bool) ), this, SLOT( toggleAction(bool) ) , Qt::DirectConnection);
    }
    else
    {
        connect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) , Qt::DirectConnection);
        connect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) , Qt::DirectConnection);
    }
#endif
    /* if the actual variable is different from actual variable, deactivate it */
    if (m_statusvar.length() != 0 && variable.trimmed().compare(m_statusvar) != 0)
    {
        m_statusvar.clear();
        m_CtIndex = -1;
    }

    /* if the acual variable is empty activate it */
    if (variable.trimmed().length() > 0)
    {
#ifdef TARGET_ARM
        m_statusvar = variable.trimmed();
        if (Tag2CtIndex(m_statusvar.toAscii().data(), &m_CtIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot extract ctIndex\n");
            m_status = STATUS_ERR;
            m_CtIndex = -1;
        }
        LOG_PRINT(verbose_e, "'%s' -> ctIndex %d\n", m_statusvar.toAscii().data(), m_CtIndex);
#else
        m_statusvar = variable.trimmed();
#endif
    }

    if (!(m_status & STATUS_ERR))
    {
#ifndef TARGET_ARM
        setToolTip(m_statusvar);
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

bool ATCMbutton::setStatusPressedValue(QString variable)
{
    m_statuspressval = (u_int32_t)variable.toInt();
    if (isChecked()) {
        m_statusactualval = m_statuspressval;
    }
    return true;
}

bool ATCMbutton::setStatusReleasedValue(QString variable)
{
    m_statusreleaseval = (u_int32_t)variable.toInt();
    if (!isChecked()) {
        m_statusactualval = m_statusreleaseval;
    }
    return true;
}

QColor ATCMbutton::bgColor() const
{
    return m_bgcolor;
}

void ATCMbutton::setBgColor(const QColor& color)
{
    m_bgcolor = color;
    update();
}

QColor ATCMbutton::borderColor() const
{
    return m_bordercolor;
}

void ATCMbutton::setBorderColor(const QColor& color)
{
    m_bordercolor = color;
    update();
}

QColor ATCMbutton::fontColor() const
{
    return m_fontcolor;
}

void ATCMbutton::setFontColor(const QColor& color)
{
    m_fontcolor = color;
    update();
}

QColor ATCMbutton::bgPressColor() const
{
    return m_bgcolor_press;
}

void ATCMbutton::setBorderWidth(int width)
{
    m_borderwidth = width;
    update();
}

void ATCMbutton::setBorderRadius(int radius)
{
    m_borderradius = radius;
    update();
}

/* read variable */
void ATCMbutton::updateData()
{
#ifdef TARGET_ARM
    u_int32_t value;

    if (!m_parent->isVisible())
    {
        incdecHvar(isVisible(), m_CtIndex, m_CtPasswordVarIndex);
        goto exit_function;
    }

    if (m_visibilityvar.length() > 0 && m_CtVisibilityIndex >= 0)
    {
        if (ioComm->readUdpReply(m_CtVisibilityIndex, &value) == 0)
        {
            m_status = STATUS_OK;
            setVisible(value != 0);
        }
    }

    incdecHvar(isVisible(), m_CtIndex, m_CtPasswordVarIndex);

    if (this->isVisible() == false)
    {
        goto exit_function;
    }

    if (m_passwordVar.length() > 0)
    {
        if (m_CtPasswordVarIndex >= 0)
        {
            LOG_PRINT(verbose_e, "password var %s, index %d\n", m_passwordVar.toAscii().data(), m_CtPasswordVarIndex);
            char valuestr[32];
            if (ioComm->valFromIndex(m_CtPasswordVarIndex, valuestr) == 0)
            {
                m_status = STATUS_OK;
                m_passwordValue = valuestr;
            }
            LOG_PRINT(verbose_e, "T: '%s' - password var %s, index %d\n", m_text.toAscii().data(), m_passwordVar.toAscii().data(), m_CtPasswordVarIndex);
        }
        else
        {
            /* the password is a static password */
            //LOG_PRINT(error_e, "T: '%s' - NO password var %s, index %d\n", m_text.toAscii().data(), m_passwordVar.toAscii().data(), m_CtPasswordVarIndex);
        }
    }

    if (m_statusvar.length() > 0 && m_CtIndex > 0)
    {
        if (!(ioComm->getStatusVar(m_CtIndex, NULL) & (STATUS_BUSY_R | STATUS_BUSY_W)))
        {
            if (ioComm->readUdpReply(m_CtIndex, &value) == 0)
            {
                if (m_statusactualval != value
                        ||
                        (m_statusactualval == m_statuspressval && isDown() == false)
                        ||
                        (m_statusactualval != m_statuspressval && isDown() == true)
                        )
                {
                    m_statusactualval = value;
                    bool press = (m_statusactualval == m_statuspressval);
                    {
                        if (isCheckable())
                        {
                            disconnect( this, SIGNAL( toggled(bool) ), this, SLOT( toggleAction(bool) ));
                            setChecked(press); // emit toggled(true);
                            LOG_PRINT(verbose_e, "setChecked(%d)\n", press);
                            connect( this, SIGNAL( toggled(bool) ), this, SLOT( toggleAction(bool) ) , Qt::DirectConnection);
                        }
                        else
                        {
                            disconnect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) );
                            disconnect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) );
                            setDown(press); // emit pressed();
                            LOG_PRINT(verbose_e, "setDown(%d)\n", press);
                            connect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) , Qt::DirectConnection);
                            connect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) , Qt::DirectConnection);
                        }
                    }
                }
            }
        }
    }
    else
    {
        m_status = STATUS_ERR;
        LOG_PRINT(verbose_e, "Invalid CtIndex %d for variable '%s'\n", m_CtIndex, m_statusvar.toAscii().data());
    }

exit_function:
    this->update();
#endif
}

void ATCMbutton::setBgPressColor(const QColor& color)
{
    m_bgcolor_press = color;
    update();
}

QColor ATCMbutton::borderPressColor() const
{
    return m_bordercolor_press;
}

void ATCMbutton::setBorderPressColor(const QColor& color)
{
    m_bordercolor_press = color;
    update();
}

QColor ATCMbutton::fontPressColor() const
{
    return m_fontcolor_press;
}

void ATCMbutton::setFontPressColor(const QColor& color)
{
    m_fontcolor_press = color;
    update();
}

QString ATCMbutton::text()
{
    return m_text;
}
void ATCMbutton::setText(QString text)
{
    if (m_text_press.length() == 0 || m_text_press == m_text)
    {
        m_text_press = text;
    }
    m_text = text;
    update();
}

QString ATCMbutton::pressText()
{
    return m_text_press;
}

void ATCMbutton::setPressText(QString text)
{
    m_text_press = text;
    update();
}

QIcon ATCMbutton::icon() const
{
    return m_icon;
}

void ATCMbutton::setIcon(const QIcon& icon)
{
    if (m_icon_press.isNull())
    {
        m_icon_press = icon;
    }
    m_icon = icon;
    update();
}

QIcon ATCMbutton::pressIcon() const
{
    return m_icon_press;
}

void ATCMbutton::setPressIcon(const QIcon& icon)
{
    m_icon_press = icon;
    update();
}

enum QFrame::Shadow ATCMbutton::apparence() const
{
    return m_apparence;
}

void ATCMbutton::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}

void ATCMbutton::setPageName(QString pagename)
{
    m_pagename = pagename;
#ifndef TARGET_ARM
    setToolTip(QString("page: %1  status: %2").arg(m_pagename).arg(m_statusvar));
#else
    setToolTip("");
#endif
    update();
}

void  ATCMbutton::goToPage()
{
#ifdef TARGET_ARM
    if (m_pagename.length() > 0)
    {
#ifdef ENABLE_TREND
        if (m_pagename.startsWith("trend"))
        {
            strncpy(_actual_trend_, m_pagename.toAscii().data(), FILENAME_MAX);
            _trend_data_reload_ = true;
            LOG_PRINT(verbose_e, "Going to page 'trend' loading file '%s'\n", m_pagename.toAscii().data());
            emit newPage("trend", m_remember);
        }
        else
#endif
#ifdef ENABLE_STORE
            if (m_pagename.startsWith("store"))
            {
                strncpy(_actual_store_, m_pagename.toAscii().data(), FILENAME_MAX);
                LOG_PRINT(verbose_e, "Going to page 'store' loading file '%s'\n", m_pagename.toAscii().data());
                emit newPage("store", m_remember);
            }
            else
#endif
            {
                LOG_PRINT(verbose_e, "Going to page %s\n", m_pagename.toAscii().data());
                emit newPage(m_pagename.toAscii().data(), m_remember);
            }
    }
#endif
}

bool ATCMbutton::checkPassword()
{
    bool retval = true;

#ifdef TARGET_ARM
    if (m_passwordVar.length() > 0 && m_passwordValue.length() > 0)
    {
        numpad * dk;
        int value = 0;

        dk = new numpad(&value, NO_DEFAULT, 0, 0, input_dec, true, 0);

        dk->showFullScreen();

        if (dk->exec() == QDialog::Accepted)
        {
            //LOG_PRINT(verbose_e, "Password '%s' vs '%d'\n", m_passwordValue.toAscii().data(), value);
            if (m_passwordValue.compare(QString("%1").arg(value)) != 0)
            {
                QMessageBox::critical(this, trUtf8("Password non valida"), trUtf8("La password inserita non è valida."));
                retval = false;
            }
            else
            {
                retval = true;
            }
        }
        else
        {
            retval = false;
        }
        delete dk;
    }
    else
    {
        retval = true;
    }
#endif
    return retval;
}

void ATCMbutton::setRemember(bool status)
{
    m_remember = status;
    update();
}

void ATCMbutton::pressFunction()
{
#ifdef TARGET_ARM
    if (checkPassword())
    {
        if (m_CtIndex > 0)
        {
            ioComm->sendUdpWriteCmd(m_CtIndex, (void*)(m_statuspressval));
        }
        goToPage();
    }
#endif
}

void ATCMbutton::releaseFunction()
{
#ifdef TARGET_ARM
    if (m_CtIndex > 0)
    {
        ioComm->sendUdpWriteCmd(m_CtIndex, (void*)(m_statusreleaseval));
    }
#endif
}

void ATCMbutton::pressAction()
{
#ifdef TARGET_ARM
    //setEnabled(false);
#if 0
    if (!isCheckable())
    {
        disconnect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) );
    }
#endif
    pressFunction();
#endif
}

void ATCMbutton::releaseAction()
{
#ifdef TARGET_ARM
#if 0
    if (!isCheckable())
    {
        disconnect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) );
    }
#endif
    releaseFunction();
#if 0
    //setEnabled(true);
    if (!isCheckable())
    {
        connect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) , Qt::DirectConnection);
        connect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) , Qt::DirectConnection);
    }
#endif
#endif
}

void ATCMbutton::toggleAction(bool status)
{
#ifdef TARGET_ARM
    if (status)
    {
        pressAction();
    }
    else
    {
        releaseAction();
    }
#endif
}

bool ATCMbutton::setPasswordVar(QString password)
{
    if (password.trimmed().length() == 0)
    {
        m_passwordVar.clear();
        m_CtPasswordVarIndex = -1;
        return true;
    }
    else
    {
#ifdef TARGET_ARM
        int CtIndex;
        if (Tag2CtIndex(password.trimmed().toAscii().data(), &CtIndex) == 0)
        {
            LOG_PRINT(verbose_e,"password '%s', CtIndex %d\n", m_passwordVar.toAscii().data(), m_CtPasswordVarIndex);
            m_CtPasswordVarIndex = CtIndex;
#endif
            m_passwordVar = password.trimmed();
            return true;
#ifdef TARGET_ARM
        }
        else
        {
            m_passwordValue = password.trimmed();
            m_passwordVar = password.trimmed();
            LOG_PRINT(verbose_e,"password '%s', CtIndex %d\n", password.trimmed().toAscii().data(), CtIndex);
            return false;
        }
#endif
    }
}
