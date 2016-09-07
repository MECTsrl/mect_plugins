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
#include "protocol.h"

#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_var.h"
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
    m_lastVisibility = false;
    m_value = "";
    m_variable = "";
    m_status = STATUS_ENABLED;
    m_CtIndex = -1;
    m_CtVisibilityIndex = -1;
    m_objectstatus = false;
    m_visibilityvar = "";
    m_writeAcknowledge = false;

    m_bgcolor = BG_COLOR_DEF;
    m_fontcolor = FONT_COLOR_DEF;
    m_bordercolor = BORDER_COLOR_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;

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
#ifdef TARGET_ARM
    setToolTip("");
#endif

    setMapping(m_mapping);
    m_remapping = false;

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

    m_parent = parent;
    connect(m_parent, SIGNAL(varRefresh()), this, SLOT(updateData()));
    //connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
}

ATCMcombobox::~ATCMcombobox()
{
}

void ATCMcombobox::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
    QPainter painter(this);
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
    if (m_viewstatus) {
        /* draw the background color in funtion of the status */
        palette.setColor(QPalette::Foreground, Qt::red);
        if (m_status & STATUS_OK)
            palette.setColor(QPalette::Foreground, Qt::green);
        else if (m_status & (STATUS_BUSY_R | STATUS_BUSY_W))
            palette.setColor(QPalette::Foreground, Qt::yellow);
        else if (m_status & (STATUS_FAIL_W | STATUS_ERR))
            palette.setColor(QPalette::Foreground, Qt::red);
        else
            palette.setColor(QPalette::Foreground, Qt::gray);
    }
#endif

    opt.palette = palette;
    _diameter_ = m_borderradius;
    _penWidth_ = m_borderwidth;
    opt.currentText = currentText();

    style()->drawComplexControl(QStyle::CC_ComboBox, &opt, &painter, this);
}

void ATCMcombobox::unsetVariable()
{
    setVariable("");
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
            LOG_PRINT(verbose_e,"visibilityVar '%s', CtIndex %d\n", m_visibilityvar.trimmed().toAscii().data(), m_CtVisibilityIndex);
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

/* Write variable */
bool ATCMcombobox::writeValue(QString value)
{
    if (m_variable.length() == 0)
    {
        return false;
    }
#ifdef TARGET_ARM
    bool ret_val = true;

    if (m_writeAcknowledge == false || QMessageBox::question(this, trUtf8("Conferma Scrittura"), trUtf8("Si vuole procedere alla scrittura del valore '%1'?").arg(value), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Ok)
    {
        m_value = mapped2value(value);
        ret_val =  ioComm->sendUdpWriteCmd(m_CtIndex, m_value.toAscii().data());
        //fprintf(stderr, "WRITING %d %s -> %s\n", m_CtIndex, value.toAscii().data(), m_value.toAscii().data());
    }

    setcomboValue();

    return ret_val;
#else
    Q_UNUSED( value );
    return true;
#endif
}

/* Activate variable */
bool ATCMcombobox::setVariable(QString variable)
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
            m_value = VAR_UNKNOWN;
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

/* read variable */
void ATCMcombobox::updateData()
{
#ifdef TARGET_ARM
    u_int32_t value;

    if (!m_parent->isVisible())
    {
        incdecHvar(isVisible(), m_CtIndex);
        return;
    }

    if (m_visibilityvar.length() > 0 && m_CtVisibilityIndex >= 0)
    {
        if (ioComm->readUdpReply(m_CtVisibilityIndex, &value) == 0)
        {
            m_status = STATUS_ENABLED;
            setVisible(value != 0);
        }
    }

    incdecHvar(isVisible(), m_CtIndex);

    if (this->isVisible() == false)
    {
        return;
    }

    if (m_variable.length() > 0)
    {
        if (m_CtIndex > 0)
        {
            char valuestr[32];
            if (ioComm->valFromIndex(m_CtIndex, valuestr) == 0)
            {
                m_status = STATUS_OK;
                m_value = valuestr;
            }
            else
            {
                m_value = VAR_UNKNOWN;
                m_status = STATUS_ERR;
            }
        }
        else
        {
            m_status = STATUS_ERR;
            m_value = VAR_UNKNOWN;
            LOG_PRINT(verbose_e, "Invalid CtIndex %d for variable '%s'\n", m_CtIndex, m_variable.toAscii().data());
        }
    }
#endif
    if (m_status & STATUS_OK)
    {
        setcomboValue();
    }
    this->update();
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
    this->clear();
    if (m_mapping.length() > 0)
    {
        QStringList map = m_mapping.split(";");
        if (maxCount() < map.count()/2)
        {
            setMaxCount(map.count()/2);
        }
        for (int i = 1; i < map.count(); i+=2)
        {
#ifdef TARGET_ARM
            LOG_PRINT(verbose_e, "@@@@@@@@@@@@@@@ '%d' [%s]\n", i, map.at(i).toAscii().data());
#endif
            this->addItem(map.at(i));
        }
    }
    else
    {
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
        if (map.at(i).trimmed().compare(value) == 0)
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
    int index = this->findText(mapped);

    /* code to manage a remapping value */
    if (index >= 0)
    {
        disconnect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
        if (m_remapping == true)
        {
            m_remapping = false;
#ifdef TARGET_ARM
            LOG_PRINT(verbose_e, "Remapping...\n");
#endif
            setMapping(m_mapping);
        }
        this->setCurrentIndex(index);
        connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
    }

    if (index == this->currentIndex() )
    {
        return true;
    }
    if (index >= 0)
    {
        this->setEditable(false);
        disconnect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
        this->setCurrentIndex(index);
        connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
        setcomboValue();
    }
    else
    {
#ifdef TARGET_ARM
        LOG_PRINT(verbose_e,"unkown value '%s'\n", m_value.toAscii().data());
#endif
        /* if is not managed, put an empty string */
        /* if the actual status is an error, display error message */
        if (m_status & STATUS_ERR)
        {
            this->setEditable(true);
            this->setEditText(mapped);
#ifdef TARGET_ARM
            LOG_PRINT(verbose_e,"unkown value '%s'\n", m_value.toAscii().data());
#endif
        }
        /* if the actual status is not expected, display the value */
        else
        {
#ifdef TARGET_ARM
            LOG_PRINT(error_e,"unkown value '%s' for variable '%s'\n", m_value.toAscii().data(), m_variable.toAscii().data());
#endif
            index = this->findText(m_value);
            if (index < 0)
            {
                this->addItem(m_value);
                index = this->findText(m_value);
            }
            disconnect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
            this->setCurrentIndex(index);
            connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
            m_remapping = true;
            //this->setEditText(m_value);
        }
        return false;
    }
    return true;
}

#if 0
void ATCMcombobox::setProva(const atcmcomboboxTaskMenu * prova)
{
    m_prova = (atcmcomboboxTaskMenu *)prova;
}
#endif
