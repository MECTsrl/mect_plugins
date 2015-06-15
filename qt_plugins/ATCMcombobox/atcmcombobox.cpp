/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM combobox base class
 */
#include <QPainter>
#include <QVBoxLayout>
#include <QStyleOption>
#include <QMessageBox>

#include "atcmcombobox.h"
#include "common.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#else
#if 0
#include <QtProperty>
#include "crosstableeditfactory.h"
#include "crosstablemanager.h"
#endif
#endif

ATCMcombobox::ATCMcombobox(QWidget *parent) :
    QComboBox(parent)
{
    m_value = "";
    m_variable = "";
    m_status = UNK;
    m_initialization = true;
    m_CtIndex = -1;
    m_CtVisibilityIndex = -1;
    m_bgcolor = QColor(230,230,230);
    m_fontcolor = QColor(10,10,10);
    m_bordercolor = QColor(0,0,0);
    m_objectstatus = false;
    m_refresh = DEFAULT_REFRESH;
    m_borderwidth = 1;
    m_borderradius = 0;
    m_visibilityvar = "";
    m_writeAcknowledge = false;
#if 0
#ifndef TARGET_ARM
    CrossTableManager *filePathManager;
    CrossTableEditFactory *fileEditFactory;
    QtProperty *example;

    filePathManager = new CrossTableManager;
    example = filePathManager->addProperty("Example");

    filePathManager->setValue(example, "main.cpp");
    filePathManager->setFilter(example, "Source files (*.cpp *.c)");

    fileEditFactory = new CrossTableEditFactory;
    browser->setFactoryForManager(filePathManager, fileEditFactory);
    addSubProperty(example);
#endif
#endif

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setStyle(new ATCMStyle);

    setMapping(m_mapping);

    /*
     * put there a default stylesheet
     */
    setStyleSheet(
            #ifndef ENABLE_STYLESHEET
                ""
            #else
                "/*"
                "QAbstractItemView {\n"
                "    padding: 5 5 16 5;\n"
                "    font-size: 20px;\n"
                "}\n"
                "QScrollBar:vertical {\n"
                "    width: 30px;\n"
                "    margin: 0px 0px 0px 0px;\n"
                "}\n"
                "QScrollBar::handle:vertical {\n"
                "    min-height:30px;\n"
                "    background-color: rgb(0, 0, 0);\n"
                "}\n"
                "QScrollBar::add-line:vertical {\n"
                "    height: 0px;\n"
                "    subcontrol-position: bottom;\n"
                "    subcontrol-origin: margin;\n"
                "}\n"
                "QScrollBar::sub-line:vertical {\n"
                "    height: 0px;\n"
                "    subcontrol-position: top;\n"
                "    subcontrol-origin: margin;\n"
                "}\n"
                "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\n"
                "    background: none;\n"
                "}\n"
                "QComboBox {\n"
                "    padding-left: 5px;\n"
                "    font-size: 20px;\n"
                "    border:2px solid black;\n"
                "    border-radius:4px;\n"
                "    background-color: rgb(230, 230, 230);\n"
                "}\n"
                "QComboBox::drop-down\n"
                "{\n"
                "    image: url(:/down.png);\n"
                "}\n"
                "*/"
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

    connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
}

ATCMcombobox::~ATCMcombobox()
{
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        delete refresh_timer;
    }
}

void ATCMcombobox::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e )
    QPalette palette = this->palette();

    QStyleOptionComboBox opt;
    opt.init(this);

    /* button color */
    palette.setColor(QPalette::Button, m_bgcolor);
    /* font color */
    palette.setColor(QPalette::ButtonText, m_fontcolor);
    /* border color */
    palette.setColor(QPalette::Foreground, m_bordercolor);

    if (m_apparence == QFrame::Raised)
    {
        opt.state = QStyle::State_Raised;
    }
    else if (m_apparence == QFrame::Sunken)
    {
        opt.state = QStyle::State_Sunken;
    }

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
    opt.currentText = currentText();

    QPainter painter(this);
    style()->drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);
}

void ATCMcombobox::unsetVariable()
{
    setVariable("");
}

void ATCMcombobox::unsetRefresh()
{
    setRefresh(DEFAULT_REFRESH);
}

void ATCMcombobox::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMcombobox::unsetVisibilityVar()
{
    setVisibilityVar("");
}

void ATCMcombobox::unsetMapping()
{
    setMapping("");
}

void ATCMcombobox::unsetwriteAcknowledge()
{
    setWriteAcknowledge(false);
}

void ATCMcombobox::unsetApparence()
{
    setApparence(QFrame::Plain);
}

void ATCMcombobox::setViewStatus(bool status)
{
    m_viewstatus = status;
}

void ATCMcombobox::setWriteAcknowledge(bool status)
{
    m_writeAcknowledge = status;
}

bool ATCMcombobox::setVisibilityVar(QString visibilityVar)
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
bool ATCMcombobox::writeValue(QString value)
{
    if (m_variable.length() == 0)
    {
        return false;
    }
    if (m_initialization)
    {
        m_initialization = false;
        return true;
    }
#ifdef TARGET_ARM
    bool ret_val = true;
    refresh_timer->stop();

    if (m_writeAcknowledge == false || QMessageBox::question(this, tr("Conferma Scrittura"), tr("Si vuole procedere alla scrittura del valore '%1'?").arg(value), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        m_value = mapped2value(value);
        ret_val =  setFormattedVar(m_variable.toAscii().data(), m_value.toAscii().data());
    }

    setcomboValue();

    refresh_timer->start(m_refresh);
    return ret_val;
#else
    Q_UNUSED( value )
    return true;
#endif
}

/* Activate variable */
bool ATCMcombobox::setVariable(QString variable)
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

QColor ATCMcombobox::bgColor() const
{
    return m_bgcolor;
}

void ATCMcombobox::setBgColor(const QColor& color)
{
    m_bgcolor = color;
    update();
}

QColor ATCMcombobox::borderColor() const
{
    return m_bordercolor;
}

void ATCMcombobox::setBorderColor(const QColor& color)
{
    m_bordercolor = color;
    update();
}

QColor ATCMcombobox::fontColor() const
{
    return m_fontcolor;
}

void ATCMcombobox::setFontColor(const QColor& color)
{
    m_fontcolor = color;
    update();
}

void ATCMcombobox::setBorderWidth(int width)
{
    m_borderwidth = width;
    update();
}

void ATCMcombobox::setBorderRadius(int radius)
{
    m_borderradius = radius;
    update();
}

bool ATCMcombobox::setRefresh(int refresh)
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
void ATCMcombobox::updateData()
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

    if (m_variable.length() > 0)
    {
        if (m_CtIndex >= 0)
        {
            if (formattedReadFromDb(m_CtIndex, value) == 0 && strlen(value) > 0)
            {
                m_status = DONE;
                m_value = value;
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
    }
    LOG_PRINT(verbose_e, "'%s': '%s' status '%c' \n", m_variable.toAscii().data(), value, m_status);
#endif
    if (m_status == DONE)
    {
        setcomboValue();
    }
    this->update();
}

bool ATCMcombobox::startAutoReading()
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

bool ATCMcombobox::stopAutoReading()
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

enum QFrame::Shadow ATCMcombobox::apparence() const
{
    return m_apparence;
}

void ATCMcombobox::setApparence(const enum QFrame::Shadow apparence)
{
    m_apparence = apparence;
    update();
}

bool ATCMcombobox::setMapping(QString mapping)
{
    m_mapping = mapping;
    if (m_mapping.length() > 0)
    {
        QStringList map = m_mapping.split(";");
        for (int i = 1; i < map.count(); i+=2)
        {
#ifdef TARGET_ARM
            LOG_PRINT(info_e, "@@@@@@@@@@@@@@@ '%d' [%s]\n", i, map.at(i).toAscii().data());
#endif
            this->addItem(map.at(i));
        }
    }
    else
    {
        this->clear();
        return false;
    }
    return true;
}

QString ATCMcombobox::value2mapped( QString value )
{
    int i;
    QStringList map = m_mapping.split(";");
#ifdef TARGET_ARM
    LOG_PRINT(verbose_e, "'%d' [%s]\n", map.count(), m_mapping.toAscii().data());
#endif
    for (i = 0; i + 1 < map.count(); i+=2)
    {
#ifdef TARGET_ARM
        LOG_PRINT(verbose_e, "compare '%s' [%s] vs '%s'\n", map.at(i).toAscii().data(), map.at(i+1).toAscii().data(), value.toAscii().data());
#endif
        if (map.at(i).compare(value) == 0)
        {
#ifdef TARGET_ARM
            LOG_PRINT(verbose_e, "Found mapping '%s' -> '%s'.\n", map.at(i).toAscii().data(), map.at(i + 1).toAscii().data());
#endif
            return map.at(i + 1);
        }
    }
    return QString("");
}

QString ATCMcombobox::mapped2value( QString mapped )
{
    int i;
    QStringList map = m_mapping.split(";");
#ifdef TARGET_ARM
    LOG_PRINT(verbose_e, "'%d' [%s]\n", map.count(), m_mapping.toAscii().data());
#endif
    for (i = 0; i + 1 < map.count(); i+=2)
    {
#ifdef TARGET_ARM
        LOG_PRINT(verbose_e, "compare '%s' [%s] vs '%s'\n", map.at(i).toAscii().data(), map.at(i+1).toAscii().data(), mapped.toAscii().data());
#endif
        if (map.at(i + 1).compare(mapped) == 0)
        {
#ifdef TARGET_ARM
            LOG_PRINT(verbose_e, "Found mapping '%s' -> '%s'.\n", map.at(i).toAscii().data(), map.at(i + 1).toAscii().data());
#endif
            return map.at(i);
        }
    }
    return QString("");
}

bool ATCMcombobox::setcomboValue()
{
    QString mapped = value2mapped(m_value);
#ifdef TARGET_ARM
    /* no mapping */
    if (mapped.length() == 0)
    {
        int index = this->findText(m_value);
        if (index >= 0)
        {
            this->setEditable(false);
            if(this->currentIndex() != index)
            {
                m_initialization = true;
                this->setCurrentIndex(index);
                m_initialization = false;
            }
            else
            {
                LOG_PRINT(verbose_e,"Value not changed\n");
            }
        }
        else
        {
            LOG_PRINT(info_e,"unkown value '%s'\n", m_value.toAscii().data());
            /* if is not managed, put an empty string */
            this->setEditable(true);
            /* if the actual status is an error, display error message */
            if (m_status == ERROR)
            {
                this->setEditText(m_value);
                LOG_PRINT(info_e,"unkown value '%s'\n", m_value.toAscii().data());
            }
            /* if the actual status is not expected, display an empty value */
            else
            {
                LOG_PRINT(warning_e, "Cannot found data '%s' into selection '%s'.\n", m_value.toAscii().data(), m_mapping.toAscii().data());
                this->setEditText("");
                LOG_PRINT(info_e,"unkown value '%s'\n", m_value.toAscii().data());
            }
            return false;
        }
    }
    /* mapping */
    else
    {
        int index = this->findText(mapped);
        if (index >= 0)
        {
            this->setEditable(false);
            m_initialization = true;
            this->setCurrentIndex(index);
            m_initialization = false;
        }
        else
        {
            LOG_PRINT(info_e,"unkown value '%s'\n", m_value.toAscii().data());
            /* if is not managed, put an empty string */
            this->setEditable(true);
            /* if the actual status is an error, display error message */
            if (m_status == ERROR)
            {
                this->setEditText(mapped);
                LOG_PRINT(info_e,"unkown value '%s'\n", m_value.toAscii().data());
            }
            /* if the actual status is not expected, display an empty value */
            else
            {
                LOG_PRINT(warning_e, "Cannot found data '%s' into selection '%s'.\n", mapped.toAscii().data(), m_mapping.toAscii().data());
                this->setEditText("");
                LOG_PRINT(info_e,"unkown value '%s'\n", m_value.toAscii().data());
            }
            return false;
        }
    }
#endif
    return true;
}

#if 0
void ATCMcombobox::setProva(const atcmcomboboxTaskMenu * prova)
{
    m_prova = (atcmcomboboxTaskMenu *)prova;
}
#endif
