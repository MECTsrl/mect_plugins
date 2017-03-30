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
#include "global_functions.h"
#endif

ATCManimation::ATCManimation(QWidget *parent) :
	QLabel(parent)
{
	m_value = VAR_UNKNOWN;
	m_variable = "";
	m_status = UNK;
    m_CtIndex = 0;
    m_CtVisibilityIndex = 0;
	m_visibilityvar = "";
	m_viewstatus = false;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

    setFrameShape(QFrame::Box);
    //setMinimumSize(QSize(150,50));
	setFocusPolicy(Qt::NoFocus);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setMinimumSize(15,15);

#ifdef TARGET_ARM
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }
#endif
}

ATCManimation::~ATCManimation()
{

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

	LOG_PRINT(verbose_e, "LOADING '%s' %d %d\n", m_value.toAscii().data(), m_maplist.count(), m_maplist.indexOf(m_value));
	if (m_value.compare(VAR_UNKNOWN) != 0 && m_maplist.count() > 0 && m_maplist.indexOf(m_value) >= 0 && m_maplist.indexOf(m_value) <= m_maplist.count() - 2)
	{
		LOG_PRINT(verbose_e, "LOADING '%s'\n", m_value.toAscii().data());
		LOG_PRINT(verbose_e, "LOADING '%s'\n", m_maplist.at(m_maplist.indexOf(m_value) + 1).toAscii().data());
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
			LOG_PRINT(error_e,"visibilityVar '%s', CtIndex %d\n", visibilityVar.trimmed().toAscii().data(), CtIndex);
			return false;
		}
#endif
    }
}

/* Activate variable */
bool ATCManimation::setVariable(QString variable)
{
    m_variable = variable.trimmed();
#ifdef TARGET_ARM
    if (m_variable.isEmpty()) {
        m_status = ERROR;
        m_value = "";
        m_CtIndex = 0;
        LOG_PRINT(verbose_e, "empty variable\n");
    }
    else if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
    {
        m_status = ERROR;
        m_value = "";
        m_CtIndex = 0;
        LOG_PRINT(error_e, "unknown variable '%s'\n", variable.trimmed().toAscii().data());
    }
    else
    {
        m_status = UNK; // not read yet
        m_value =  "";
        LOG_PRINT(info_e, "set variable #%d '%s'\n", m_CtIndex, m_variable.toAscii().data());
    }
    setToolTip("");
#else
    setToolTip(m_variable);
#endif

    return true;
}

void ATCManimation::unsetVariable()
{
    setVariable("");
}

void ATCManimation::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
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
	return true;
}

/* read variable */
void ATCManimation::updateData()
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
                char svalue[42] = "";
                register int decimal = getVarDecimalByCtIndex(m_CtIndex); // locks only if it's from another variable
                sprintf_fromValue(svalue, m_CtIndex, ivalue, decimal, 10);

                do_update = (m_status != status) || (m_value.compare(QString(svalue)) != 0);
                if (do_update) {
                    m_status = status;
                    m_value = svalue;
                }
             } break;
            default:
                do_update = (m_status != ERROR);
                m_status = ERROR;
                m_value = "";
            }
        }
    }

    if (do_update) {
        this->update();
    }
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
		LOG_PRINT(verbose_e, "%d\n", m_maplist.count());
		for (int i = 0; i < m_maplist.count() - 1; i+=2)
		{
			LOG_PRINT(verbose_e, "%d - '%s' -> '%s'\n", i, m_maplist.at(i).toAscii().data(),  m_maplist.at(i+1).toAscii().data());
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
