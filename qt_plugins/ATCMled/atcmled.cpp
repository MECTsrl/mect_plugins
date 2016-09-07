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
#include "protocol.h"

#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_var.h"
#endif

ATCMled::ATCMled(QWidget *parent) :
    QLabel(parent)
{
    m_lastVisibility = false;
    m_value = 0;
    m_variable = "";
    m_status = STATUS_ENABLED;
    m_CtIndex = -1;
    m_CtVisibilityIndex = -1;
    m_onicon = QIcon(":/on.png");
    m_officon = QIcon(":/off.png");
    m_objectstatus = false;
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

    m_parent = parent;
    connect(m_parent, SIGNAL(varRefresh()), this, SLOT(updateData()));
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

    if (m_viewstatus == true &&( m_status & STATUS_ERR))
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
    /* if the acual variable is different from actual variable, deactivate it */
    if (m_variable.length() != 0 && variable.trimmed().compare(m_variable) != 0)
    {
        m_variable.clear();
        m_CtIndex = -1;
    }

    /* if the acual variable is empty activate it */
    if (variable.trimmed().length() > 0)
    {
#ifdef TARGET_ARM
        m_variable = variable.trimmed();
        if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
        {
            LOG_PRINT(error_e, "cannot extract ctIndex\n");
            m_status = STATUS_ERR;
            //m_value = VAR_UNKNOWN;
            m_CtIndex = -1;
        }
        LOG_PRINT(verbose_e, "'%s' -> ctIndex %d\n", m_variable.toAscii().data(), m_CtIndex);
#else
        m_variable = variable.trimmed();
#endif
    }

    if (!(m_status & STATUS_ERR))
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

/* read variable */
void ATCMled::updateData()
{
#ifdef TARGET_ARM
    u_int32_t value = 0;

    if (!m_parent->isVisible())
    {
        incdecHvar(isVisible(), m_CtIndex);
        return;
    }

    if (m_visibilityvar.length() > 0 && m_CtVisibilityIndex >= 0)
    {
        if (ioComm->readUdpReply(m_CtVisibilityIndex, &value) == 0)
        {
            m_status = STATUS_OK;
            setVisible(value != 0);
        }
    }

    incdecHvar(isVisible(), m_CtIndex);

    if (this->isVisible() == false)
    {
        return;
    }

    if (m_variable.length() > 0 && m_CtIndex > 0)
    {
        if (ioComm->readUdpReply(m_CtIndex, &value) == 0)
        {
            m_value = (value != 0);
            m_status = STATUS_OK;
        }
        else
        {
            //m_value = -1;
            m_status = STATUS_ERR;
        }
    }
    else
    {
        m_status = STATUS_ERR;
        LOG_PRINT(verbose_e, "[%s] Invalid CtIndex %d for variable '%s'\n", objectName().toAscii().data(), m_CtIndex, m_variable.toAscii().data());
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
