/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
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
#endif

ATCMprogressbar::ATCMprogressbar(QWidget *parent) :
	QProgressBar(parent)
{
	m_value = 0;
	m_variable = "";
	m_status = UNK;
	m_CtIndex = -1;
	m_CtVisibilityIndex = -1;
	m_barColor = QColor(255,127,80);
	m_visibilityvar = "";
	m_viewstatus = false;

    m_bgcolor = BG_COLOR_DEF;
    m_bordercolor = BORDER_COLOR_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

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

}

ATCMprogressbar::~ATCMprogressbar()
{
	if (refresh_timer != NULL)
	{
		refresh_timer->stop();
		delete refresh_timer;
	}
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
        m_CtVisibilityIndex = -1;
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
            LOG_PRINT(error_e,"visibilityVar '%s', CtIndex %d\n", visibilityVar.trimmed().toAscii().data(), CtIndex);
            return false;
        }
#endif
    }
}

/* Activate variable */
bool ATCMprogressbar::setVariable(QString variable)
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
        if (activateVar(variable.trimmed().toAscii().data()) == 0)
        {
            m_variable = variable.trimmed();
            if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
            {
                LOG_PRINT(error_e, "cannot extract ctIndex\n");
                m_status = ERROR;
                //m_value = VAR_UNKNOWN;
                m_CtIndex = -1;
            }
            LOG_PRINT(verbose_e, "'%s' -> ctIndex %d\n", m_variable.toAscii().data(), m_CtIndex);
        }
        else
        {
            m_status = ERROR;
            //m_value = VAR_UNKNOWN;
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
void ATCMprogressbar::updateData()
{
#ifdef TARGET_ARM
	char value[TAG_LEN] = "";

	if (m_visibilityvar.length() > 0 && m_CtVisibilityIndex >= 0)
	{
		if (formattedReadFromDb(m_CtVisibilityIndex, value) == 0 && strlen(value) > 0)
        {
            m_status = DONE;
            LOG_PRINT(verbose_e, "VISIBILITY %d\n", atoi(value));
            setVisible(atoi(value) != 0);
		}
		LOG_PRINT(verbose_e, "'%s': '%s' visibility status '%c' \n", m_variable.toAscii().data(), value, m_status);
	}
	if (this->isVisible() == false)
	{
		return;
	}

	if (m_variable.length() > 0 && m_CtIndex >= 0)
	{
		if (formattedReadFromDb(m_CtIndex, value) == 0 && strlen(value) > 0)
		{
            m_status = DONE;
            m_value = atoi(value);
		}
		else
		{
			//m_value = VAR_UNKNOWN;
			m_status = ERROR;
		}
	}
	else
	{
		m_status = ERROR;
		m_value = -1;
		LOG_PRINT(verbose_e, "Invalid CtIndex %d for variable '%s'\n", m_CtIndex, m_variable.toAscii().data());
	}
	LOG_PRINT(verbose_e, " %d '%s': '%s' status '%c' (BUSY '%c' - ERROR '%c' - DONE '%c')\n", m_CtIndex, m_variable.toAscii().data(), value, m_status, BUSY, ERROR, DONE);
#endif
	if (m_status == DONE)
	{
		this->setValue(m_value);
	}
	this->update();
}

bool ATCMprogressbar::startAutoReading()
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

bool ATCMprogressbar::stopAutoReading()
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
