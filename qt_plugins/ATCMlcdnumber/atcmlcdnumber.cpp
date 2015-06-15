/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM lcdnumber base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>

#include "atcmlcdnumber.h"
#include "common.h"
#ifdef TARGET_ARM
#include "cross_table_utility.h"
#include "app_logprint.h"
#endif

ATCMlcdnumber::ATCMlcdnumber(QWidget *parent) :
	QLCDNumber(parent)
{
	m_value = "";
	m_variable = "";
	m_status = UNK;

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	/*
	 * put there a default stylesheet
	 */
	setStyleSheet(
#ifndef ENABLE_STYLESHEET
			""
#else
			"QLCDNumber {\n"
			"    background-color: rgb(230, 230, 230);\n"
			"    border:2px solid black;\n"
			"    border-radius:4px;\n"
			"}"
#endif
			);

	if (m_refresh > 0)
	{
		refresh_timer = new QTimer(this);
		connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
		refresh_timer->start(m_refresh);
	}
	else
	{
		refresh_timer = NULL;
	}
}

ATCMlcdnumber::~ATCMlcdnumber()
{
	if (refresh_timer != NULL)
	{
		refresh_timer->stop();
		delete refresh_timer;
	}
}

void ATCMlcdnumber::paintEvent(QPaintEvent * e)
{
	QPainter p(this);

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
	/* propagate the stylesheet set by QtCreator */
	QStyleOption opt;
	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
	/* propagate the paint event to the parent widget */
	QLCDNumber::paintEvent(e);
}

/* Write variable */
bool ATCMlcdnumber::writeValue(QString value)
{
	m_value = value;
	this->display(m_value);
	if (m_value.length() == 0 || m_variable.length() == 0)
	{
		return true;
	}
#ifdef TARGET_ARM
	return setFormattedVar(m_variable.toAscii().data(), m_value.toAscii().data());
#else
	return true;
#endif
}

/* Activate variable */
bool ATCMlcdnumber::setVariable(QString variable)
{
	m_variable = variable;
#ifdef TARGET_ARM
	int SynIndex = 0;
	int CtIndex = 0;
	char blockhead[TAG_LEN] = "";
	int retval;

	LOG_PRINT(verbose_e, "Activating '%s'\n", m_variable.toAscii().data());

	/* check if the block of the variable is already active */
	retval = isBlockActive(m_variable.toAscii().data(), blockhead);
	if (retval == 1)
	{
		LOG_PRINT(warning_e, "The variable '%s' come from a block already active\n", m_variable.toAscii().data());
		return true;
	}
	else if (retval == 0)
	{
		/* looking the variable index from the crosstable structure */
		/* insert it into the syncrovector */
		if (addSyncroElement(blockhead, &CtIndex) == 0)
		{
			if (CtIndex2SynIndex(CtIndex, &SynIndex) != 0)
			{
				LOG_PRINT(error_e, "Cannot find the variable '%d'' into the syncro vector\n", CtIndex);
				return false;
			}
			LOG_PRINT(verbose_e, "Set reading flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
			/* enable it in reading */
			SET_SYNCRO_FLAG(SynIndex, READ_MASK);
			LOG_PRINT(verbose_e, "Set reading flag  pIOSyncroAreaO[%d] '%X'\n",SynIndex, pIOSyncroAreaO[SynIndex]);
			return true;
		}
		LOG_PRINT(error_e, "Cannot Activate '%s'\n", m_variable.toAscii().data());
	}
	LOG_PRINT(error_e, "IMPOSSIBLE RET VALUE '%d'\n", retval);
	m_status = ERROR;
	m_value = VAR_UNKNOWN;
#endif
	return false;
}

bool ATCMlcdnumber::setRefresh(int refresh)
{
	m_refresh = refresh;
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
	return true;
}

/* read variable */
void ATCMlcdnumber::updateData()
{
#ifdef TARGET_ARM
	int CtIndex;
	char value[TAG_LEN] = "";

	if (this->isVisible() == false)
	{
		return;
	}

	if (m_variable.length() == 0)
	{
		return;
	}

	if (Tag2CtIndex(m_variable.toAscii().data(), &CtIndex) != 0)
	{
		LOG_PRINT(error_e, "cannot extract ctIndex\n");
		m_status = ERROR;
		m_value = VAR_UNKNOWN;
	}
	else
	{
		if (formattedReadFromDb(CtIndex, value) == 0 && strlen(value) > 0)
		{
			char statusMsg[TAG_LEN];
			m_status = getStatusVarByCtIndex(CtIndex, statusMsg);
			switch (m_status)
			{
				case BUSY:
					m_value = value;
					break;
				case ERROR:
					if (strlen(statusMsg) > 0) m_value = statusMsg;
					break;
				case DONE:
					m_value = value;
					break;
				default:
					m_value = VAR_UNKNOWN;
					break;
			}
		}
		else
		{
			m_value = VAR_UNKNOWN;
			m_status = ERROR;
		}
	}
	LOG_PRINT(verbose_e, "'%s': '%s' status '%c' (BUSY '%c' - ERROR '%c' - DONE '%c')\n", m_variable.toAscii().data(), value, m_status, BUSY, ERROR, DONE);
#endif
	this->update();
	this->display(m_value);
}

bool ATCMlcdnumber::startAutoReading()
{
	if (refresh_timer != NULL && m_refresh > 0)
	{
		refresh_timer->start(m_refresh);
		return true;
	}
	return false;
}

bool ATCMlcdnumber::stopAutoReading()
{
	if (refresh_timer != NULL)
	{
		refresh_timer->stop();
		return true;
	}
	return false;
}

bool ATCMlcdnumber::setViewStatus(bool status)
{
	return (m_viewstatus = status);
}
