/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM spinbox base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>

#include "atcmspinbox.h"
#include "common.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#endif

ATCMspinbox::ATCMspinbox(QWidget *parent) :
    QDoubleSpinBox(parent)
{
    m_value = 0;
    m_variable = "";
    m_status = UNK;
    m_initialization = true;
    m_CtIndex = -1;
    m_CtVisibilityIndex = -1;
    m_bgcolor = QColor(230,230,230);
    m_labelcolor = QColor(230,230,230);
    m_fontcolor = QColor(10,10,10);
    m_bordercolor = QColor(0,0,0);
    m_objectstatus = false;
    m_refresh = DEFAULT_REFRESH;
    m_borderwidth = 1;
    m_borderradius = 0;
    m_visibilityvar = "";

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyle(new ATCMStyle);

    /*
     * put there a default stylesheet
     */
    setStyleSheet(
            #ifndef ENABLE_STYLESHEET
                ""
            #else
                "/*\n"
                "QDoubleSpinBox\n"
                "{\n"
                "    font: 18pt \"Ubuntu\";\n"
                "    background-color: rgb(230, 230, 230);\n"
                "    border:2px solid black;\n"
                "    border-radius:4px;\n"
                "    qproperty-alignment: AlignCenter;\n"
                "}\n"
                "QDoubleSpinBox::up-button {\n"
                "    subcontrol-origin: content;\n"
                "    subcontrol-position: right center;\n"
                "    min-height: 25px;\n"
                "    min-width: 25px;\n"
                "    background-color: rgb(230, 230, 230);\n"
                "}\n"
                "QDoubleSpinBox::up-arrow {\n"
                "    border-image: url(:/right.png);\n"
                "}\n"
                "QDoubleSpinBox::down-button {\n"
                "    subcontrol-origin: content;\n"
                "    subcontrol-position: left center;\n"
                "    min-height: 25px;\n"
                "    min-width: 25px;\n"
                "    background-color: rgb(230, 230, 230);\n"
                "}\n"
                "QDoubleSpinBox::down-arrow {\n"
                "    border-image: url(:/left.png);\n"
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

    connect( this, SIGNAL( valueChanged(double) ), this, SLOT( writeValue(double) ) );
}

ATCMspinbox::~ATCMspinbox()
{
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        delete refresh_timer;
    }
}

void ATCMspinbox::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e )
    _diameter_ = m_borderradius;
    _penWidth_ = m_borderwidth;
    QPalette palette = this->palette();


    /* font color */
    palette.setColor(QPalette::Text,m_fontcolor);
    /* label color */
    palette.setColor(QPalette::ButtonText,m_labelcolor);
    /* border color */
    palette.setColor(QPalette::Foreground, m_bordercolor);
    /* bg color */
    palette.setColor(QPalette::Button, m_bgcolor);

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

    QPainter painter(this);
    QStyleOptionSpinBox opt;
    opt.init(this);

    if (m_apparence == QFrame::Raised)
    {
        opt.state = QStyle::State_Raised;
    }
    else if (m_apparence == QFrame::Sunken)
    {
        opt.state = QStyle::State_Sunken;
    }

    opt.palette = palette;

    style()->drawComplexControl(QStyle::CC_SpinBox, &opt, &painter, this);
}

void ATCMspinbox::setViewStatus(bool status)
{
    m_viewstatus = status;
    update();
}

bool ATCMspinbox::setVisibilityVar(QString visibilityVar)
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

/* Write variable */
bool ATCMspinbox::writeValue(double value)
{
    if (m_variable.length() == 0)
    {
        m_value = (float)value;
        this->setValue(m_value);
        return false;
    }
    if (m_initialization)
    {
        m_initialization = false;
        return true;
    }
#ifdef TARGET_ARM
    if (setFormattedVar(m_variable.toAscii().data(), QString().setNum(value).toAscii().data()))
    {
        LOG_PRINT(info_e, "WRITE %f \n", m_value);
        m_value = (float)value;
        this->setValue(m_value);
        return true;
    }
    else
    {
        LOG_PRINT(info_e, "WRITE\n");
        this->setValue(m_value);
        return false;
    }
#else
    m_value = (float)value;
    this->setValue(m_value);
    return true;
#endif
}

/* Activate variable */
bool ATCMspinbox::setVariable(QString variable)
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
            else
            {
                switch (CtIndex2Type(m_CtIndex))
                {
                case intab_e:
                case intba_e:
                case uintab_e:
                case uintba_e:
                case dint_abcd_e:
                case dint_badc_e:
                case dint_cdab_e:
                case dint_dcba_e:
                case udint_abcd_e:
                case udint_badc_e:
                case udint_cdab_e:
                case udint_dcba_e:
                    QDoubleSpinBox::setDecimals(0);
                    break;
                case fabcd_e:
                case fbadc_e:
                case fcdab_e:
                case fdcba_e:
                {
                    int decimal = 0;
                    if (varNameArray[m_CtIndex].decimal > 4)
                    {
                        if (readFromDb(varNameArray[m_CtIndex].decimal, &decimal) != 0)
                        {
                            decimal = 0;
                        }
                    }
                    else if (varNameArray[m_CtIndex].decimal > 0)
                    {
                        LOG_PRINT(verbose_e, "Decimal %d\n", varNameArray[m_CtIndex].decimal);
                        decimal = varNameArray[m_CtIndex].decimal;
                    }
                    else
                    {
                        decimal = 0;
                    }

                    QDoubleSpinBox::setDecimals(decimal);
                }
                    break;
                default:
                    QDoubleSpinBox::setDecimals(0);
                    break;
                }

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

QColor ATCMspinbox::bgColor() const
{
    return m_bgcolor;
}

void ATCMspinbox::setBgColor(const QColor& color)
{
    m_bgcolor = color;
    update();
}

QColor ATCMspinbox::borderColor() const
{
    return m_bordercolor;
}

void ATCMspinbox::setBorderColor(const QColor& color)
{
    m_bordercolor = color;
    update();
}

QColor ATCMspinbox::fontColor() const
{
    return m_fontcolor;
}

void ATCMspinbox::setFontColor(const QColor& color)
{
    m_fontcolor = color;
    update();
}

QColor ATCMspinbox::labelColor() const
{
    return m_labelcolor;
}

void ATCMspinbox::setLabelColor(const QColor& color)
{
    m_labelcolor = color;
    update();
}

void ATCMspinbox::setBorderWidth(int width)
{
    m_borderwidth = width;
    update();
}

void ATCMspinbox::setBorderRadius(int radius)
{
    m_borderradius = radius;
    update();
}

bool ATCMspinbox::setRefresh(int refresh)
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
void ATCMspinbox::updateData()
{
#ifdef TARGET_ARM
    char statusMsg[TAG_LEN] = "";
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

    if (m_variable.length())
    {
        if (m_CtIndex >= 0)
        {
            if (formattedReadFromDb(m_CtIndex, value) == 0 && strlen(value) > 0)
            {
                m_status = DONE;
                m_value = atof(value);
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
            LOG_PRINT(error_e, "Invalid CtIndex %d for variable '%s'\n", m_CtIndex, m_variable.toAscii().data());
        }
    }
    LOG_PRINT(verbose_e, " %d '%s': '%s' status '%c' (BUSY '%c' - ERROR '%c' - DONE '%c')\n", m_CtIndex, m_variable.toAscii().data(), value, m_status, BUSY, ERROR, DONE);

    if (m_status == ERROR)
    {
        /* set error MSG */
        if (strlen(statusMsg) > 0)
        {
            setSpecialValueText(statusMsg);
        }
    }
    else
#endif
        if (m_status == DONE)
        {
            this->setValue(m_value);
        }
    this->update();
}

bool ATCMspinbox::startAutoReading()
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

bool ATCMspinbox::stopAutoReading()
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

enum QFrame::Shadow ATCMspinbox::apparence() const
{
    return m_apparence;
}

void ATCMspinbox::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}

void ATCMspinbox::unsetVariable()
{
    setVariable("");
}

void ATCMspinbox::unsetRefresh()
{
    setRefresh(DEFAULT_REFRESH);
}

void ATCMspinbox::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMspinbox::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCMspinbox::unsetApparence()
{
    setApparence(QFrame::Plain);
}


