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
#include "global_functions.h"
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
#ifdef TARGET_ARM
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }
#endif
}

ATCMlcdnumber::~ATCMlcdnumber()
{
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
    m_variable = variable.trimmed();
#ifdef TARGET_ARM
//    if (m_variable.isEmpty()) {
//        m_status = ERROR;
//        m_value = "";
//        m_CtIndex = 0;
//        LOG_PRINT(verbose_e, "empty variable\n");
//    }
//    else if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
//    {
//        m_status = ERROR;
//        m_value = "";
//        m_CtIndex = 0;
//        LOG_PRINT(error_e, "unknown variable '%s'\n", variable.trimmed().toAscii().data());
//    }
//    else
//    {
        m_status = UNK; // not read yet
        m_value =  "";
//        LOG_PRINT(info_e, "set variable #%d '%s'\n", m_CtIndex, m_variable.toAscii().data());
//    }
    setToolTip("");
#else
    setToolTip(m_variable);
#endif
    return false;
}

bool ATCMlcdnumber::setRefresh(int refresh)
{
	m_refresh = refresh;
	return true;
}

/* read variable */
void ATCMlcdnumber::updateData()
{
#ifdef TARGET_ARM
    bool do_update = false;
    int m_CtIndex; // <---=

    if (! this->isVisible() || m_variable.length() == 0) {
		return;
	}

    if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0) {
		LOG_PRINT(error_e, "cannot extract ctIndex\n");
        do_update = (m_status != ERROR);
        m_status = ERROR;
        m_value = "";
	}
	else
	{
        int ivalue;
        register char status = readFromDbQuick(m_CtIndex, &ivalue);

        do_update = true; // (m_status == UNK) || (ivalue != m_iprevious) || (status != m_sprevious)
        if (do_update) {
            switch (status) {
            case DONE:
            case BUSY: {
                char svalue[42] = "";
                register int decimal = getVarDecimalByCtIndex(m_CtIndex); // locks only if it's from another variable

                sprintf_fromValue(svalue, m_CtIndex, ivalue, decimal, 10);
                do_update = (m_status != DONE) || (m_value.compare(QString(svalue)) != 0);
                m_status = DONE;
                if (do_update) {
                    m_value = svalue;
                }
            }   break;
            case ERROR:
            default:
                do_update = (m_status != ERROR);
                m_status = ERROR;
                m_value = "";
            }
        }
	}
    if (do_update) {
        this->display(m_value);
        this->update();
    }
#endif
}

bool ATCMlcdnumber::setViewStatus(bool status)
{
	return (m_viewstatus = status);
}
