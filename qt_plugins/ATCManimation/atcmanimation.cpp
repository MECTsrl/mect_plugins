/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM animation base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>

#include "atcmanimation.h"
#include "common.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#endif

ATCManimation::ATCManimation(QWidget *parent) :
	QLabel(parent)
{
	m_value = VAR_UNKNOWN;
	m_variable = "";
	m_status = UNK;
	m_CtIndex = -1;
	m_CtVisibilityIndex = -1;
	m_refresh = DEFAULT_REFRESH;
	m_visibilityvar = "";
	m_viewstatus = false;

    setFrameShape(QFrame::Box);
    //setMinimumSize(QSize(150,50));
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setMinimumSize(15,15);

	/*
	 * put there a default stylesheet
	 *led->setStyleSheet("padding: 5px;");
	 */

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

ATCManimation::~ATCManimation()
{
	if (refresh_timer != NULL)
	{
		refresh_timer->stop();
		delete refresh_timer;
	}
}

void ATCManimation::paintEvent(QPaintEvent * e)
{
	QPainter p(this);

#ifdef TARGET_ARM
	if (m_viewstatus)
	{
		/* draw the background color in funtion of the status */
		QBrush brush(Qt::red);
		switch(m_status)
		{
			case DONE:
				brush.setColor(Qt::green);
				break;
			case BUSY:
				brush.setColor(Qt::yellow);
				break;
			case ERROR:
				brush.setColor(Qt::red);
				break;
			default /*UNKNOWN*/:
				brush.setColor(Qt::gray);
				break;
		}
		p.setBrush(brush);
		p.drawRect(this->rect());
	}

	LOG_PRINT(info_e, "LOADING '%s' %d %d\n", m_value.toAscii().data(), m_maplist.count(), m_maplist.indexOf(m_value));
	if (m_value.compare(VAR_UNKNOWN) != 0 && m_maplist.count() > 0 && m_maplist.indexOf(m_value) >= 0 && m_maplist.indexOf(m_value) <= m_maplist.count() - 2)
	{
		LOG_PRINT(info_e, "LOADING '%s'\n", m_value.toAscii().data());
		LOG_PRINT(info_e, "LOADING '%s'\n", m_maplist.at(m_maplist.indexOf(m_value) + 1).toAscii().data());
		p.drawPixmap(this->rect(), QPixmap(m_maplist.at(m_maplist.indexOf(m_value) + 1)), QPixmap(m_maplist.at(m_maplist.indexOf(m_value) + 1)).rect());
	}
#else
	if (m_maplist.count() >= 2)
	{
		p.drawPixmap(this->rect(), QPixmap(m_maplist.at(1)), QPixmap(m_maplist.at(1)).rect());
	}
#endif
	/* propagate the stylesheet set by QtCreator */
	QStyleOption opt;
	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	/* propagate the paint event to the parent widget */
	QLabel::paintEvent(e);
}

void ATCManimation::setViewStatus(bool status)
{
	m_viewstatus = status;
	update();
}

bool ATCManimation::setVisibilityVar(QString visibilityVar)
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
			if (m_refresh == 0)
			{
				setRefresh(DEFAULT_REFRESH);
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
bool ATCManimation::setVariable(QString variable)
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
    if (variable.trimmed() > 0)
    {
#ifdef TARGET_ARM
        if (activateVar(variable.trimmed().toAscii().data()) == 0)
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
#endif
        return true;
    }
    else
    {
        return false;
    }
}

void ATCManimation::unsetVariable()
{
    setVariable("");
}

void ATCManimation::unsetRefresh()
{
    setRefresh(DEFAULT_REFRESH);
}

void ATCManimation::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCManimation::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCManimation::unsetMapping()
{
    setMapping("");
}

bool ATCManimation::setRefresh(int refresh)
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
void ATCManimation::updateData()
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
            m_value = value;
            m_status = DONE;
}
		else
		{
			m_value = VAR_UNKNOWN;
			m_status = ERROR;
		}
	}
	else
	{
		m_status = ERROR;
		m_value = VAR_UNKNOWN;
		LOG_PRINT(info_e, "Invalid CtIndex %d for variable '%s'\n", m_CtIndex, m_variable.toAscii().data());
	}
	LOG_PRINT(verbose_e, "'%s': '%s' status '%c' \n", m_variable.toAscii().data(), value, m_status);
#endif
	this->update();
}

bool ATCManimation::startAutoReading()
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

bool ATCManimation::stopAutoReading()
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

bool ATCManimation::setMapping(QString mapping)
{
	m_mapping = mapping;
	if (m_mapping.length() > 0)
	{
		m_maplist = m_mapping.split(";");
#ifdef TARGET_ARM
#if 0
		LOG_PRINT(info_e, "%d\n", m_maplist.count());
		for (int i = 0; i < m_maplist.count() - 1; i+=2)
		{
			LOG_PRINT(info_e, "%d - '%s' -> '%s'\n", i, m_maplist.at(i).toAscii().data(),  m_maplist.at(i+1).toAscii().data());
		}
#endif
#endif
    }
	else
	{
		m_maplist.clear();
		return false;
	}
	return true;
}