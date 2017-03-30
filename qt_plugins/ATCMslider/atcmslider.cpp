/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM slider base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <stdio.h>

#include "atcmslider.h"
#include "common.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_functions.h"
#endif

ATCMslider::ATCMslider(QWidget *parent) :
	QSlider(parent)
{
	m_value = 0;
	m_variable = "";
	m_status = UNK;
    m_CtIndex = 0;
    m_CtVisibilityIndex = 0;
	m_handlerColor = QColor(128,128,128);
	m_addColor = QColor(255,127,80);
	m_subColor = QColor(255,127,80);
	m_icon = QIcon();
    m_visibilityvar = "";
	m_viewstatus = false;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

    m_bordercolor = BORDER_COLOR_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;

    //setMinimumSize(QSize(150,50));
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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
			"/*\n"
			"QSlider::groove:vertical  {\n"
			"    background: transparent;\n"
			"/* absolutely position 4px from the left and right of the widget. setting margins on the widget should work too... */\n"
			"    position: absolute;\n"
			"    left: 4px; right: 4px;\n"
			"}\n"
			"QSlider::handle:vertical  {\n"
			"    height: 31px;\n"
			"    background: transparent;\n"
			"    margin: 0 -4px; /* expand outside the groove */\n"
			"    border-image: url(:/marker.png);\n"
			"}\n"
			"QSlider::add-page:vertical  {\n"
			"    background: transparent;\n"
			"}\n"
			"QSlider::sub-page:vertical  {\n"
			"    background: transparent;\n"
			"}\n"
			"*/\n"
#endif
			);
#ifdef TARGET_ARM
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }
    connect(this, SIGNAL( valueChanged(int) ), this, SLOT( writeValue(int) ));
#endif
}

ATCMslider::~ATCMslider()
{
}

void ATCMslider::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter painter(this);
	QPalette palette = this->palette();

	QStyleOptionSlider opt;
	opt.init(this);

	/* sub color */
	palette.setColor(QPalette::ButtonText, m_subColor);
	/* add color */
	palette.setColor(QPalette::Background, m_addColor);
	/* handler color */
	palette.setColor(QPalette::Button, m_handlerColor);
	/* border color */
	palette.setColor(QPalette::Foreground, m_bordercolor);

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

	if (m_apparence == QFrame::Raised)
	{
        opt.state = QStyle::State_Raised;
	}
	else if (m_apparence == QFrame::Sunken)
	{
        opt.state = QStyle::State_Sunken;
	}

	opt.orientation = QSlider::orientation();
	opt.maximum = QSlider::maximum();
	opt.minimum = QSlider::minimum();
	opt.sliderValue = QSlider::value();
	opt.rect = QSlider::rect();
	opt.pageStep = QSlider::pageStep();
	opt.upsideDown = QSlider::invertedAppearance();
	_icon_ = m_icon;

	style()->drawComplexControl(QStyle::CC_Slider, &opt, &painter, this);
}

void ATCMslider::setViewStatus(bool status)
{
	m_viewstatus = status;
	update();
}

bool ATCMslider::setVisibilityVar(QString visibilityVar)
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
            LOG_PRINT(verbose_e,"visibilityVar '%s', CtIndex %d\n", m_visibilityvar.toAscii().data(), m_CtVisibilityIndex);
            m_CtVisibilityIndex = CtIndex;
            this->setVisible(false); // to avoid initial splash
#endif
            m_visibilityvar = visibilityVar.trimmed();
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

/* Write variable */
bool ATCMslider::writeValue(int value)
{
#ifdef TARGET_ARM
    if (m_CtIndex <= 0 || m_status == UNK) {
        return false;
    }

    // do_write
    m_value = value;
    setFormattedVarByCtIndex(m_CtIndex, QString::number(m_value).toAscii().data());

    // do_update
    disconnect( this, SIGNAL( valueChanged(int) ), this, SLOT( writeValue(int) ) );
    this->setValue(m_value);
    connect( this, SIGNAL( valueChanged(int) ), this, SLOT( writeValue(int) ) );
#else
    Q_UNUSED( value );
#endif
    return true;
}

/* Activate variable */
bool ATCMslider::setVariable(QString variable)
{
    m_variable = variable.trimmed();
#ifdef TARGET_ARM
    if (m_variable.isEmpty()) {
        m_status = ERROR;
        m_value = minimum();
        m_CtIndex = 0;
        LOG_PRINT(verbose_e, "empty variable\n");
    }
    else if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
    {
        m_status = ERROR;
        m_value = minimum();
        m_CtIndex = 0;
        LOG_PRINT(error_e, "unknown variable '%s'\n", variable.trimmed().toAscii().data());
    }
    else
    {
        m_status = UNK; // not read yet
        m_value = minimum();
        LOG_PRINT(info_e, "set variable #%d '%s'\n", m_CtIndex, m_variable.toAscii().data());
    }
    setToolTip("");
#else
    setToolTip(m_variable);
#endif

    return true;
}

QColor ATCMslider::borderColor() const
{
	return m_bordercolor;
}

void ATCMslider::setBorderColor(const QColor& color)
{
	m_bordercolor = color;
	update();
}

QColor ATCMslider::handlerColor() const
{
	return m_handlerColor;
}

void ATCMslider::setHandlerColor(const QColor& color)
{
	m_handlerColor = color;
	update();
}

QColor ATCMslider::subColor() const
{
	return m_subColor;
}

void ATCMslider::setSubColor(const QColor& color)
{
	m_subColor = color;
	update();
}

QColor ATCMslider::addColor() const
{
	return m_addColor;
}

void ATCMslider::setAddColor(const QColor& color)
{
	m_addColor = color;
	update();
}

void ATCMslider::setBorderWidth(int width)
{
	m_borderwidth = width;
	update();
}

void ATCMslider::setBorderRadius(int radius)
{
	m_borderradius = radius;
	update();
}

bool ATCMslider::setRefresh(int refresh)
{
	m_refresh = refresh;
	return true;
}

/* read variable */
void ATCMslider::updateData()
{
#ifdef TARGET_ARM
    bool do_update = false;

    if (m_CtVisibilityIndex > 0) {
        int ivalue;
        switch (readFromDbQuick(m_CtVisibilityIndex, &ivalue)) {
        case DONE:
        case BUSY:
            if (ivalue && ! this->isVisible()) {
                this->setVisible(true);
                m_status = UNK;
            }
            else if (! ivalue && this->isVisible()) {
                this->setVisible(false);
                m_status = UNK;
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

    if (m_CtIndex > 0) {
        int ivalue;
        register char status = readFromDbQuick(m_CtIndex, &ivalue);

        do_update = true; // (m_status == UNK) || (ivalue != m_iprevious) || (status != m_sprevious)
        if (do_update) {
            switch (status) {
            case DONE:
            case BUSY:
            case ERROR: {
                register int decimal = getVarDecimalByCtIndex(m_CtIndex); // locks only if it's from another variable
                int new_value = int_fromValue(m_CtIndex, ivalue, decimal);

                if (new_value > this->maximum()) {
                    new_value = this->maximum();
                } else if (new_value < this->minimum()) {
                    new_value = this->minimum();
                }
                do_update = (m_status != status) || (m_value != new_value);
                if (do_update) {
                    m_status = status;
                    m_value = new_value;
                }
              } break;
            default:
                do_update = (m_status != ERROR);
                m_status = ERROR;
                m_value = -1;
            }
        }
    }

    if (do_update) {
        disconnect( this, SIGNAL( valueChanged(int) ), this, SLOT( writeValue(int) ) );
        this->setValue(m_value);
        connect( this, SIGNAL( valueChanged(int) ), this, SLOT( writeValue(int) ) );
        this->update();
    }
#endif
}

QIcon ATCMslider::icon() const
{
	return m_icon;
}

void ATCMslider::setIcon(const QIcon& icon)
{
	m_icon = icon;
	update();
}

enum QFrame::Shadow ATCMslider::apparence() const
{
    return m_apparence;
}

void ATCMslider::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}

void ATCMslider::unsetVariable()
{
    setVariable("");
}

void ATCMslider::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
}

void ATCMslider::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMslider::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCMslider::unsetApparence()
{
    setApparence(QFrame::Plain);
}
