/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM date base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QDate>

#include "atcmdate.h"
#include "common.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "global_functions.h"
#endif

ATCMdate::ATCMdate(QWidget *parent) :
    QPushButton(parent)
{

    m_bgcolor_select = BG_COLOR_SEL_DEF;
    m_fontcolor_select = FONT_COLOR_SEL_DEF;
    m_bordercolor_select = BORDER_COLOR_SEL_DEF;
    m_bgcolor = BG_COLOR_DEF;
    m_fontcolor = FONT_COLOR_DEF;
    m_bordercolor = BORDER_COLOR_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;

    m_format = DEFAULT_DATE;

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
#else
    refresh_timer = new QTimer(this);
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
    refresh_timer->start(1000);
#endif
}

ATCMdate::~ATCMdate()
{
#ifdef TARGET_ARM
#else
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        delete refresh_timer;
    }
#endif
}

void ATCMdate::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter painter(this);
    QPalette palette = this->palette();

    QStyleOptionButton opt;
    opt.init(this);

    /* text */
    opt.text = text();

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

    opt.palette = palette;
    _diameter_ = m_borderradius;
    _penWidth_ = m_borderwidth;

    style()->drawControl(QStyle::CE_PushButton, &opt, &painter);
}

QColor ATCMdate::bgColor() const
{
    return m_bgcolor;
}

void ATCMdate::setBgColor(const QColor& color)
{
    m_bgcolor = color;
    update();
}

QColor ATCMdate::borderColor() const
{
    return m_bordercolor;
}

void ATCMdate::setBorderColor(const QColor& color)
{
    m_bordercolor = color;
    update();
}

QColor ATCMdate::fontColor() const
{
    return m_fontcolor;
}

void ATCMdate::setFontColor(const QColor& color)
{
    m_fontcolor = color;
    update();
}

QColor ATCMdate::bgSelectColor() const
{
    return m_bgcolor_select;
}

void ATCMdate::setBorderWidth(int width)
{
    m_borderwidth = width;
    update();
}

void ATCMdate::setBorderRadius(int radius)
{
    m_borderradius = radius;
    update();
}

/* read variable */
void ATCMdate::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }
    this->setText(QDate::currentDate().toString(m_format));
    this->update();
}

void ATCMdate::setBgSelectColor(const QColor& color)
{
    m_bgcolor_select = color;
    update();
}

QColor ATCMdate::borderSelectColor() const
{
    return m_bordercolor_select;
}

void ATCMdate::setBorderSelectColor(const QColor& color)
{
    m_bordercolor_select = color;
    update();
}

QColor ATCMdate::fontSelectColor() const
{
    return m_fontcolor_select;
}

void ATCMdate::setFontSelectColor(const QColor& color)
{
    m_fontcolor_select = color;
    update();
}

enum QFrame::Shadow ATCMdate::apparence() const
{
    return m_apparence;
}

void ATCMdate::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}


void ATCMdate::setFormat(const enum ATCMDateFormat format)
{
    if (format == DD_MM_YYYY)
    {
        m_format = ITALIAN_DATE;
    }
    else
    {
        m_format = ENGLISH_DATE;
    }
#ifdef TARGET_ARM
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }
    setToolTip("");
#endif
    update();
}
