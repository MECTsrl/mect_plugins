/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM progressbar base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <stdio.h>

#include "atcmprogressbar.h"
#include "common.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_functions.h"
#include "global_functions.h"
#endif

ATCMprogressbar::ATCMprogressbar(QWidget *parent) :
	QProgressBar(parent)
{
    m_value = -1;
	m_variable = "";
	m_status = UNK;
    m_CtIndex = 0;
    m_CtVisibilityIndex = 0;
	m_barColor = QColor(255,127,80);
	m_visibilityvar = "";
	m_viewstatus = false;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

    m_bgcolor = BG_COLOR_DEF;
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
			"QProgressBar\n"
			"{\n"
			"    /*text color*/\n"
			"    color: rgb(255, 255, 255);\n"
			"    /* Horizontal() or Vertical() */\n"
			"    orientation:Horizontal();\n"
			"    /* this line set the background colour */ \n"
			"    background-color: rgb(0,0,0);\n"
			"    /* this line set the border width in px with a solid style and a desired colour */\n"
			"    border: 1px solid  rgb(160,160,160);\n"
			"    /* this line set the border radius to in px. */\n"
			"    border-radius: 5px;\n"
			"}\n"
			"/* this section set the Chunk style of the generic QProgressBar in this page */ \n"
			"QProgressBar::chunk\n"
			"{\n"
			"    /* this line set the background colour */\n"
			"    background-color: qlineargradient(spread:reflect, x1:0.528429, y1:0.535, x2:1, y2:1, stop:0.309524 rgba(47, 241, 0, 255), stop:1 rgba(194, 194, 194, 255));\n"
			"    /* this line set the border radius to in px. */\n"
			"    border-radius: 5px;\n"
			"}\n"
			"*/\n"
#endif
			);

#ifdef TARGET_ARM
#endif

}

ATCMprogressbar::~ATCMprogressbar()
{
}

void ATCMprogressbar::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter painter(this);
    QPalette palette = this->palette();

	QStyleOptionProgressBar option;
	option.init(this);
	QStyleOptionProgressBarV2 opt(option);

	/* bar color */
	palette.setColor(QPalette::Button, m_barColor);
	/* border color */
	palette.setColor(QPalette::Foreground, m_bordercolor);
	/* bg color */
	palette.setColor(QPalette::Background, m_bgcolor);

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

	opt.orientation = QProgressBar::orientation();
	opt.minimum = QProgressBar::minimum();
	opt.maximum = QProgressBar::maximum();
	opt.progress = QProgressBar::value();

	style()->drawControl(QStyle::CE_ProgressBar, &opt, &painter);
}

void ATCMprogressbar::setViewStatus(bool status)
{
	m_viewstatus = status;
	update();
}

bool ATCMprogressbar::setVisibilityVar(QString visibilityVar)
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

/* Activate variable */
bool ATCMprogressbar::setVariable(QString variable)
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
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }
    setToolTip("");
#else
    setToolTip(m_variable);
#endif

    return true;
}

QColor ATCMprogressbar::bgColor() const
{
	return m_bgcolor;
}

void ATCMprogressbar::setBgColor(const QColor& color)
{
	m_bgcolor = color;
	update();
}

QColor ATCMprogressbar::borderColor() const
{
	return m_bordercolor;
}

void ATCMprogressbar::setBorderColor(const QColor& color)
{
	m_bordercolor = color;
	update();
}

QColor ATCMprogressbar::barColor() const
{
	return m_barColor;
}

void ATCMprogressbar::setBarColor(const QColor& color)
{
	m_barColor = color;
	update();
}

void ATCMprogressbar::setBorderWidth(int width)
{
	m_borderwidth = width;
	update();
}

void ATCMprogressbar::setBorderRadius(int radius)
{
	m_borderradius = radius;
	update();
}

bool ATCMprogressbar::setRefresh(int refresh)
{
	m_refresh = refresh;
	return true;
}

/* read variable */
void ATCMprogressbar::updateData()
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
        this->setValue(m_value);
        this->update();
    }
#endif
}


enum QFrame::Shadow ATCMprogressbar::apparence() const
{
    return m_apparence;
}

void ATCMprogressbar::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}

void ATCMprogressbar::unsetVariable()
{
    setVariable("");
}

void ATCMprogressbar::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
}

void ATCMprogressbar::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMprogressbar::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCMprogressbar::unsetApparence()
{
    setApparence(QFrame::Plain);
}
