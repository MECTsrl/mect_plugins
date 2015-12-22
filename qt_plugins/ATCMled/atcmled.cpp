/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM led base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>

#include "atcmled.h"
#include "common.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#endif

ATCMled::ATCMled(QWidget *parent) :
	QLabel(parent)
{
    m_value = 0;
	m_variable = "";
	m_status = UNK;
	m_CtIndex = -1;
	m_CtVisibilityIndex = -1;
	m_onicon = QIcon(":/on.png");
	m_officon = QIcon(":/off.png");
	m_objectstatus = false;
	m_refresh = 0;
	m_visibilityvar = "";
	m_viewstatus = false;

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

ATCMled::~ATCMled()
{
	if (refresh_timer != NULL)
	{
		refresh_timer->stop();
		delete refresh_timer;
	}
}

void ATCMled::paintEvent(QPaintEvent * e)
{
	QPainter p(this);

#if 1
	switch(m_value)
	{
		case 0:
			m_officon.paint(&p,this->rect());
			break;
		case 1:
			m_onicon.paint(&p,this->rect());
			break;
		default:
            return;
			m_officon.paint(&p,this->rect());
			break;
	}
#endif
	/* propagate the stylesheet set by QtCreator */
	QStyleOption opt;
	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

	/* propagate the paint event to the parent widget */
	QLabel::paintEvent(e);
}

void ATCMled::setViewStatus(bool status)
{
	m_viewstatus = status;
	update();
}

/* Activate variable */
bool ATCMled::setVariable(QString variable)
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
#endif
        return true;
    }
    else
    {
        return false;
    }
}

void ATCMled::unsetVariable()
{
    setVariable("");
}

void ATCMled::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
}

void ATCMled::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMled::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCMled::unsetOnIcon()
{
    m_onicon = QIcon(":/on.png");
}

void ATCMled::unsetOffIcon()
{
    m_officon = QIcon(":/off.png");
}

bool ATCMled::setVisibilityVar(QString visibilityVar)
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


bool ATCMled::setRefresh(int refresh)
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
void ATCMled::updateData()
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
		LOG_PRINT(info_e, "'%s': '%s' visibility status '%c' \n", m_variable.toAscii().data(), value, m_status);
	}
	if (this->isVisible() == false)
	{
		return;
	}

	if (m_variable.length() > 0 && m_CtIndex >= 0)
	{
		if (formattedReadFromDb(m_CtIndex, value) == 0 && strlen(value) > 0)
		{
            m_value = atoi(value) != 0;
            m_status = DONE;
        }
		else
		{
            //m_value = -1;
			m_status = ERROR;
		}
	}
	else
	{
		m_status = ERROR;
        LOG_PRINT(info_e, "[%s] Invalid CtIndex %d for variable '%s'\n", objectName().toAscii().data(), m_CtIndex, m_variable.toAscii().data());
	}
	LOG_PRINT(verbose_e, " %d '%s': '%s' status '%c' (BUSY '%c' - ERROR '%c' - DONE '%c')\n", m_CtIndex, m_variable.toAscii().data(), value, m_status, BUSY, ERROR, DONE);
#endif
	this->update();
}

bool ATCMled::startAutoReading()
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

bool ATCMled::stopAutoReading()
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

QIcon ATCMled::onIcon() const
{
	return m_onicon;
}

void ATCMled::setOnIcon(const QIcon& icon)
{
	m_onicon = icon;
	update();
}

QIcon ATCMled::offIcon() const
{
	return m_officon;
}

void ATCMled::setOffIcon(const QIcon& icon)
{
	m_officon = icon;
	update();
}
