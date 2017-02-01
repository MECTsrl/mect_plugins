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
    m_CtIndex = 0;
    m_CtVisibilityIndex = 0;
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

	m_parent = parent;
    connect(m_parent, SIGNAL(varRefresh()), this, SLOT(updateData()));

    /*
     * put there a default stylesheet
     *led->setStyleSheet("padding: 5px;");
     */

#ifdef TARGET_ARM
    if (m_refresh > 0)
    {
    }
    else
#endif
    {
    }
}

ATCMled::~ATCMled()
{

}

void ATCMled::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    QIcon::State state;
    QIcon::Mode mode;
    QIcon icon = QIcon(":/unk.png");

    switch(m_value)
    {
    case 0:
        icon = m_officon;
        mode = QIcon::Normal;
        state = QIcon::Off;
        break;
    case 1:
        icon = m_onicon;
        mode = QIcon::Normal;
        state = QIcon::On;
        break;
    default:
        icon = QIcon(":/unk.png");
        state = QIcon::Off;
        mode = QIcon::Disabled;
        break;
    }

    if (m_viewstatus == true && m_status == ERROR)
    {
        mode = QIcon::Disabled;
    }
    icon.paint(&p,this->rect(), Qt::AlignCenter, mode, state);

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
    /* if the acual variable is empty activate it */
    if (variable.trimmed().length() > 0)
    {
#ifdef TARGET_ARM
        m_variable = variable.trimmed();
        if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot extract ctIndex\n");
            m_status = ERROR;
            //m_value = VAR_UNKNOWN;
            m_CtIndex = 0;
        }
        LOG_PRINT(verbose_e, "'%s' -> ctIndex %d\n", m_variable.toAscii().data(), m_CtIndex);
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

bool ATCMled::setRefresh(int refresh)
{
    m_refresh = refresh;
    return true;
}

/* read variable */
void ATCMled::updateData()
{
#ifdef TARGET_ARM
    if (m_CtVisibilityIndex > 0) {
        uint32_t visible = 0;
        if (readFromDb(m_CtVisibilityIndex, &visible) == 0) {
            m_status = DONE;
            if (visible && ! this->isVisible()) {
                this->setVisible(true);
            }
            else if (! visible && this->isVisible()) {
                this->setVisible(false);
            }
        }
    }

    if (! this->isVisible()) {
        return;
    }

    if (m_CtIndex > 0)
    {
        int ivalue;
        if (formattedReadFromDb_int(m_CtIndex, &ivalue) == 0)
        {
            m_value = (ivalue != 0);
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
    }
#endif
    this->update();
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
