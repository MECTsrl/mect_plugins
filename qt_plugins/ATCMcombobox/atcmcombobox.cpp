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
#include <QStyledItemDelegate>

#include "atcmcombobox.h"
#include "common.h"
#include "atcmstyle.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#include "global_functions.h"
#endif

ATCMcombobox::ATCMcombobox(QWidget *parent) :
    QComboBox(parent)
{
    m_fBusy = false;
    m_value = "";
    m_variable = "";
    m_status = UNK;
    m_CtIndex = -1;
    m_CtVisibilityIndex = 0;
    m_objectstatus = false;
    m_visibilityvar = "";
    m_writeAcknowledge = false;

    m_bgcolor = BG_COLOR_DEF;
    m_fontcolor = FONT_COLOR_DEF;
    m_bordercolor = BORDER_COLOR_DEF;
    m_borderwidth = BORDER_WIDTH_DEF;
    m_borderradius = BORDER_RADIUS_DEF;
    m_refresh = DEFAULT_PLUGIN_REFRESH;

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setEditable(false);
    QStyledItemDelegate *itDelegate = new QStyledItemDelegate();
    setItemDelegate(itDelegate);
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
#ifdef TARGET_ARM
    QObject *ancestor = getPage((QObject *)this);

    if (ancestor != NULL) {
        connect(ancestor, SIGNAL(varRefresh()), this, SLOT(updateData()));
    }
    connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
#endif
}

ATCMcombobox::~ATCMcombobox()
{
}

void ATCMcombobox::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e );
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
    setRefresh(DEFAULT_PLUGIN_REFRESH);
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

/* Write variable */
bool ATCMcombobox::writeValue(QString value)
{
#ifdef TARGET_ARM
    if (m_CtIndex <= 0 || m_status == UNK) {
        return false;
    }

    m_fBusy = true;
    if (m_writeAcknowledge && QMessageBox::question(this, trUtf8("Confirm Writing"), trUtf8("Do you want to save new value: '%1'?").arg(value), QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel)
    {
        return false;
    }
    m_value = mapped2value(value);
    setFormattedVarByCtIndex(m_CtIndex, m_value.toAscii().data());
    setcomboValue();
    m_fBusy = false;

#else
    Q_UNUSED( value );
#endif
    return true;
}

/* Activate variable */
bool ATCMcombobox::setVariable(QString variable)
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
    return true;
}

/* read variable */
void ATCMcombobox::updateData()
{
#ifdef TARGET_ARM
    bool do_update = false;

    if (m_fBusy)
        return;

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
            }   break;
            default:
                do_update = (m_status != ERROR);
                m_status = ERROR;
                m_value = "";
            }
        }
    }

    if (do_update) {
        setcomboValue();
        this->update();
    }
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
    int index = this->findText(mapped, Qt::MatchExactly);

#ifdef TARGET_ARM
    if (m_status == DONE)
    {
        // (index >= 0) or -1 from findText
        disconnect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
        this->setCurrentIndex(index);
        connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
        return true;
    }
    else if (this->currentIndex() >= 0) // when in ERROR or UNK
    {
        disconnect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
        this->setCurrentIndex(-1);
        connect( this, SIGNAL( currentIndexChanged(QString) ), this, SLOT( writeValue(QString) ) );
    }
#else
    /* code to manage a remapping value */
    if (index >= 0)
    {
        if (m_remapping == true)
        {
            m_remapping = false;
            setMapping(m_mapping);
            setcomboValue();
        }
        this->setCurrentIndex(index);
    }
    if (index == this->currentIndex())
    {
        return true;
    }
    else if (index >= 0)
    {
        this->setCurrentIndex(index);
        return true;
    }
    /* if the actual status is not expected, display the value */
    this->addItem(m_value);
    index = this->findText(m_value);
    this->setCurrentIndex(index);
    m_remapping = true;
    //this->setEditText(m_value);
#endif
    return false;
}

