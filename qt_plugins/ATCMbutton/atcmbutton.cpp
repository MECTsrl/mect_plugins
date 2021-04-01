/**
 * @file
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
#include <QString>

#include "atcmbutton.h"
#include "common.h"
#include "numpad.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include <stdio.h>
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_functions.h"
extern bool _trend_data_reload_;
extern char _actual_trend_[FILENAME_MAX];
extern char _actual_store_[FILENAME_MAX];
#endif

ATCMbutton::ATCMbutton(QWidget * parent):
    QPushButton(parent)
{
    m_pagename = "";
    m_remember = true;
    m_status = UNK;
    m_CtIndex = 0;
    m_CtVisibilityIndex = 0;
    m_CtPasswordVarIndex = 0;
    m_text = "";
    m_text_press = "";
    m_icon = QIcon();
    m_icon_press = QIcon();
    m_visibilityvar = "";
    m_passwordVar = "";
    m_passwordValue = "";
    m_viewstatus = false;
    m_statuspressval = "1";
    m_statusreleaseval = "0";
    m_justchanged = 0;
    m_forcedAction = false;
    m_statusactualval = m_statusreleaseval;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

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

    m_fBusy = false;
#ifdef TARGET_ARM
    // at this time "checkable" is not yet determined
#endif
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

    if (isDown() || isChecked())
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
            opt.state = QStyle::State_On | QStyle::State_Sunken;
        }
        else if (m_apparence == QFrame::Sunken)
        {
            opt.state = QStyle::State_On | QStyle::State_Raised;
        }
        else
        {
            opt.state = QStyle::State_On;
        }
    }
    else
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
        case UNK:
        default /*UNKNOWN*/:
            palette.setColor(QPalette::Foreground, Qt::gray);
        }
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
            this->setVisible(false); // to avoid initial splash
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

void ATCMbutton::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
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
    m_statusvar = variable.trimmed();
#ifdef TARGET_ARM
    m_text = "";
    if (m_statusvar.isEmpty()) {
        m_status = ERROR;
        m_CtIndex = 0;
        LOG_PRINT(verbose_e, "empty variable\n");
    }
    else if (Tag2CtIndex(m_statusvar.toAscii().data(), &m_CtIndex) != 0)
    {
        m_status = ERROR;
        m_CtIndex = 0;
        LOG_PRINT(error_e, "unknown variable '%s'\n", m_statusvar.trimmed().toAscii().data());
    }
    else
    {
        m_status = UNK; // not read yet
        LOG_PRINT(info_e, "set variable #%d '%s'\n", m_CtIndex, m_statusvar.toAscii().data());
    }

    // anyway for updating passwords
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        // avoid multiple connects after QEvent::LanguageChange
        disconnect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }

    // here because in the constructor checkable is not yet determined
    if (isCheckable())
    {
        // avoid multiple connects after QEvent::LanguageChange
        disconnect( this, SIGNAL( toggled(bool) ), this, SLOT( toggleAction(bool) ));
        connect( this, SIGNAL( toggled(bool) ), this, SLOT( toggleAction(bool) ) , Qt::DirectConnection);
    }
    else
    {
        // avoid multiple connects after QEvent::LanguageChange
        disconnect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ));
        disconnect( this, SIGNAL( released() ), this, SLOT( releaseAction() ));
        connect( this, SIGNAL( pressed() ), this, SLOT( pressAction() ) , Qt::DirectConnection);
        connect( this, SIGNAL( released() ), this, SLOT( releaseAction() ) , Qt::DirectConnection);
    }
    setToolTip("");
#else
    setToolTip(m_statusvar);
#endif

    return true;
}

bool ATCMbutton::setStatusPressedValue(QString variable)
{
    if (m_statusactualval.compare(m_statuspressval) == 0) {
        m_statusactualval = variable;
    }
    m_statuspressval = variable;
    return true;
}

bool ATCMbutton::setStatusReleasedValue(QString variable)
{
    if (m_statusactualval.compare(m_statusreleaseval) == 0) {
        m_statusactualval = variable;
    }
    m_statusreleaseval = variable;
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

bool ATCMbutton::setRefresh(int refresh)
{
    Q_UNUSED( refresh );
    m_refresh = refresh;
    return true;
}

/* read variable */
void ATCMbutton::updateData()
{
#ifdef TARGET_ARM
    bool do_update = false;

    if (m_fBusy)
        return;

    if (m_CtVisibilityIndex > 0) {
        int ivalue;
        switch (readFromDbQuick(m_CtVisibilityIndex, &ivalue)) {
        case DONE:
        case BUSY:
            if (ivalue && ! this->isVisible()) {
                this->setVisible(true);
                m_status = UNK;
                do_update = true;
            }
            else if (! ivalue && this->isVisible()) {
                this->setVisible(false);
                m_status = UNK;
                do_update = true; // useless
            }
            break;
        case ERROR:
        default:
            ; // do nothing
        }
    }

    if (! this->isVisible()) {
        return;
    }

    if (m_CtPasswordVarIndex > 0) {
        int ivalue = 0;
        switch (readFromDbQuick(m_CtPasswordVarIndex, &ivalue)) {
        case DONE:
        case BUSY: {
            char svalue[42] = "";
            register int decimal = getVarDecimalByCtIndex(m_CtPasswordVarIndex); // locks only if it's from another variable
            sprintf_fromValue(svalue, m_CtPasswordVarIndex, ivalue, decimal, 10);

            // only password status, no: m_status = DONE;
            m_passwordValue = QString(svalue).trimmed();
          } break;
        case ERROR:
        default:
            ; // do nothing
        }
    } // else m_passwordValue already set

    if (m_CtIndex > 0) {
        int ivalue;
        register char status = readFromDbQuick(m_CtIndex, &ivalue);

        do_update = true; // (m_status == UNK) || (ivalue != m_iprevious) || (status != m_sprevious)
        if (do_update) {
            switch (status) {
            case DONE:
            case BUSY:
            case ERROR: {
                char svalue[42] = "";
                register int decimal = getVarDecimalByCtIndex(m_CtIndex); // locks only if it's from another variable
                sprintf_fromValue(svalue, m_CtIndex, ivalue, decimal, 10);
                QString Svalue(svalue);

                bool isPressed = (Svalue == m_statuspressval);
                if (isCheckable()) {
                    do_update = (m_status != status) || (m_statusactualval != Svalue)
                             || (isPressed && ! isChecked()) || (! isPressed && isChecked());
                } else {
                    do_update = (m_status != status) || (m_statusactualval != Svalue)
                             || (isPressed && ! isDown()) || (! isPressed && isDown());
                }
                if (do_update) {
                    m_status = status;
                    m_statusactualval = Svalue;
                    isPressed = (m_statusactualval == m_statuspressval);
                    if (isCheckable()) {
                        bool wasBlocked = this->blockSignals(true);
                        setChecked(isPressed);
                        this->blockSignals(wasBlocked);
                    } else {
                        bool wasBlocked = this->blockSignals(true);
                        setDown(isPressed);
                        this->blockSignals(wasBlocked);
                    }
                }
              } break;
            default:
                do_update = (m_status != ERROR);
                m_status = ERROR;
                m_statusactualval = "";
            }
        }
    }

    if (do_update) {
        this->update();
    }
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
        if (m_pagename.startsWith("trend"))
        {
            strncpy(_actual_trend_, m_pagename.toAscii().data(), FILENAME_MAX);
            _trend_data_reload_ = true;
            LOG_PRINT(verbose_e, "Going to page 'trend' loading file '%s'\n", m_pagename.toAscii().data());
            emit newPage("trend", m_remember);
        }
        else if (m_pagename.startsWith("store"))
        {
            strncpy(_actual_store_, m_pagename.toAscii().data(), FILENAME_MAX);
            LOG_PRINT(verbose_e, "Going to page 'store' loading file '%s'\n", m_pagename.toAscii().data());
            emit newPage("store", m_remember);
        }
        else
        {
            LOG_PRINT(verbose_e, "Going to page %s\n", m_pagename.toAscii().data());
            this->setDown(false); // in case the page doesn't exist
            emit newPage(m_pagename.toAscii().data(), m_remember);
        }
    }
#endif
}

bool ATCMbutton::checkPassword()
{
    bool retval = true;

#ifdef TARGET_ARM
    if (!m_passwordValue.isEmpty()) // not only (m_CtPasswordVarIndex > 0)
    {
        numpad * dk;
        int value = 0;

        dk = new numpad(&value, NO_DEFAULT, 0, 0, input_dec, true, 0);

        dk->showFullScreen();

        if (dk->exec() == QDialog::Accepted)
        {
            //LOG_PRINT(verbose_e, "Password '%s' vs '%d'\n", m_passwordValue.toAscii().data(), value);
            if (m_passwordValue != QString::number(value))
            {
                QMessageBox::critical(this, trUtf8("Invalid Password"), trUtf8("The inserted password is wrong!!!"));
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
            setFormattedVarByCtIndex(m_CtIndex, m_statuspressval.toAscii().data());
        }
        goToPage();
    }
    else
    {
        // Password Failed
        if (isCheckable())
        {
            bool wasBlocked = this->blockSignals(true);
            setChecked(false);
            this->blockSignals(wasBlocked);
        }
    }
#endif
}

void ATCMbutton::releaseFunction()
{
#ifdef TARGET_ARM
    if (isCheckable())
    {
        if (checkPassword())
        {
            if (m_CtIndex > 0)
            {
                setFormattedVarByCtIndex(m_CtIndex, m_statusreleaseval.toAscii().data());
            }
        }
        else
        {
            bool wasBlocked = this->blockSignals(true);
            setChecked(true);
            this->blockSignals(wasBlocked);
        }
    }
    else
    {
        if (m_CtIndex > 0)
        {
            setFormattedVarByCtIndex(m_CtIndex, m_statusreleaseval.toAscii().data());
        }
    }

#endif
}

void ATCMbutton::pressAction()
{
#ifdef TARGET_ARM
    m_fBusy = true;
    pressFunction();
    m_fBusy = false;
#endif
}

void ATCMbutton::releaseAction()
{
#ifdef TARGET_ARM
    m_fBusy = true;
    releaseFunction();
    m_fBusy = false;
#endif
}

void ATCMbutton::toggleAction(bool status)
{
#ifdef TARGET_ARM
    m_fBusy = true;
    if (status)
    {
        pressFunction();
    }
    else
    {
        releaseFunction();
    }
    m_fBusy = false;
#endif
}

bool ATCMbutton::setPasswordVar(QString password)
{
    QString tPassword = password.trimmed();

    if (tPassword.isEmpty())
    {
#ifdef TARGET_ARM
        m_CtPasswordVarIndex = 0;
        m_passwordValue.clear();
#endif
        m_passwordVar.clear();
        return true;
    }
#ifdef TARGET_ARM
    int CtIndex;
    if (Tag2CtIndex(tPassword.toAscii().data(), &CtIndex) == 0)
    {
        m_CtPasswordVarIndex = CtIndex;
        m_passwordValue.clear();
        m_passwordVar = tPassword;
        return true;
    }
    bool ok;
    tPassword.toInt(&ok);
    if (ok)
    {
        m_CtPasswordVarIndex = 0;
        m_passwordValue = tPassword;
        m_passwordVar.clear();
        return true;
    }
    else
    {
        LOG_PRINT(error_e, "wrong password value '%s'\n", tPassword.toAscii().data());
        m_CtPasswordVarIndex = 0;
        m_passwordValue.clear();
        m_passwordVar.clear();
        return false;
    }
#else
    m_passwordVar = tPassword;
    return true;
#endif
}
