/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM led base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QFileInfo>

#include "atcmled.h"
#include "common.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_functions.h"
#endif

ATCMled::ATCMled(QWidget *parent) :
    QLabel(parent)
{
    m_value = 0; // -1;
    m_variable = "";
    m_status = UNK;
    m_CtIndex = 0;
    m_CtVisibilityIndex = 0;
    m_onicon = QIcon(":/on.png");
    m_officon = QIcon(":/off.png");
    m_objectstatus = false;
    m_visibilityvar = "";
    m_viewstatus = false;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumSize(15,15);

#ifdef TARGET_ARM
#endif
}

ATCMled::~ATCMled()
{

}

void ATCMled::paintEvent(QPaintEvent * e)
{
    QPainter p(this);
    QIcon::State state;
    QIcon::Mode mode;
    QIcon icon;
    static QIcon unk(":/unk.png");

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
    case -1:
    default:
        icon = unk;
        mode = QIcon::Normal;
        state = QIcon::Off;
    }

    if (m_viewstatus == true && m_status != DONE)
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
    m_variable = variable.trimmed();
#ifdef TARGET_ARM
    if (m_variable.isEmpty()) {
        m_status = ERROR;
        m_value = 0; // -1;
        m_CtIndex = 0;
        LOG_PRINT(verbose_e, "empty variable\n");
    }
    else if (Tag2CtIndex(m_variable.toAscii().data(), &m_CtIndex) != 0)
    {
        m_status = ERROR;
        m_value = 0; //-1;
        m_CtIndex = 0;
        LOG_PRINT(error_e, "unknown variable '%s'\n", variable.trimmed().toAscii().data());
    }
    else
    {
        m_status = UNK; // not read yet
        m_value =  0; // -1;
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

bool ATCMled::setRefresh(int refresh)
{
    m_refresh = refresh;
    return true;
}

/* read variable */
void ATCMled::updateData()
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
                // no decimals check, only exact zero and non zero values
                if (ivalue != 0) {
                    ivalue = 1;
                }
                do_update = (m_status != status) || (m_value != ivalue);
                if (do_update) {
                    m_status = status;
                    m_value = ivalue;
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
        this->update();
    }
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
