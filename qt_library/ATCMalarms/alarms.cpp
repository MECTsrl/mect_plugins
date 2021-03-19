/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Alarm page
 */
#include <pthread.h>
#include "app_logprint.h"
#include "alarms.h"
#include "ui_alarms.h"


/* this define set the window title */
#define WINDOW_TITLE "ALARM"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the current page style.
 * the syntax is html stylesheet-like
 */
#define SET_ALARMS_STYLE() { \
    QString mystyle; \
    mystyle.append(this->styleSheet()); \
    /* add there the page stylesheet customization */ \
    mystyle.append(""); \
    this->setStyleSheet(mystyle); \
    }

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
alarms::alarms(QWidget *parent) :
    page(parent),
    ui(new Ui::alarms)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    
    /* if exist and is not empty enable the WINDOW_ICON */
    if (strlen(WINDOW_ICON) > 0)
    {
        /* enable this line, define the WINDOW_ICON and put a Qlabel named labelIcon in your ui file if you want have a window icon */
        labelIcon = ui->labelIcon;
    }
    /* if exist and is not empty enable the WINDOW_TITLE */
    if (strlen(WINDOW_TITLE) > 0)
    {
        /* enable this line, define the WINDOW_TITLE and put a Qlabel named labelTitle in your ui file if you want have a window title */
        labelTitle = ui->labelTitle;
    }
    
    /* set up the page style */
    //setStyle::set(this);
    /* set the style described into the macro SET_ALARMS_STYLE */
    SET_ALARMS_STYLE();
    
    /* connect the label that show the date and the time to the timer of the parent updateData */
    labelDataOra = ui->labelDataOra;
    /* connect the label that show the actual user name */
    //labelUserName = ui->labelUserName;
    
#ifdef LEVEL_TYPE
    ui->comboBoxLevel->clear();
    for (int i = level_all_e; i < nb_of_level_e; i++)
    {
        ui->comboBoxLevel->addItem(QString("%1").arg(i));
    }
#endif
    
    reload();
    /*
       connect(logger, SIGNAL(new_alarm(info_msg_e *)), this, SLOT(addEvent(info_msg_e *)));
       connect(logger, SIGNAL(new_event(info_msg_e *)), this, SLOT(addEvent(info_msg_e *)));
     */
    connect(logger, SIGNAL(new_alarm(char *)), this, SLOT(receiveEvent(char *)));
    connect(logger, SIGNAL(new_event(char *)), this, SLOT(receiveEvent(char *)));

    // connect signal new_ack to logger::dumpAck()
    logger->connectToPage(this);
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void alarms::reload()
{
    _alarm = true;
    _event = true;
    _level = level_all_e;
    
    /* clean the list Widget from the old values */
    QListWidgetItem * item;
    do
    {
        item = ui->listWidget->takeItem ( 0 );
        if (item !=  0)
        {
            delete item;
        }
    }
    while (item != 0);
    refreshEvent();
    /* rotateShowError is the function for the alarm banner.
     * the "ui->lineEditStatus" is the line edit in your custom page used to diplay the error.
     * the ERROR_LABEL_PERIOD_MS is the feedrate.
     * is enought to use this funtion to make the banner working.
     */
    rotateShowError(ui->lineEditStatus, ERROR_LABEL_PERIOD_MS);
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void alarms::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }
    refreshEvent();
    /* call the parent updateData member */
    page::updateData();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void alarms::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}
#endif

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
alarms::~alarms()
{
    delete ui;
}

/**
 * @brief Add a new element into the Alarm list box
 */
void alarms::addEvent(event_descr_e * msg, bool visibility)
{
    char line[DESCR_LEN * 2];
    char event[DESCR_LEN];
    char type[DESCR_LEN];
    
    if (msg->isack)
    {
        strcpy(event, TAG_ACK);
    }
    else if (msg->status == alarm_rise_e)
    {
        strcpy(event, TAG_RISE);
    }
    else if (msg->status == alarm_fall_e)
    {
        strcpy(event, TAG_FALL);
    }
    else
    {
        strcpy(event, TAG_UNK);
        LOG_PRINT(verbose_e, "Unknown status '%s' [%s|%s|%s] for variable '%s'\n", event, TAG_ACK, TAG_RISE, TAG_FALL, msg->tag);
        return;
    }
    
    event_t * evt = EventHash.find(msg->tag).value();
    if (evt->type == ALARM)
    {
        strcpy(type, TAG_ALARM);
    }
    else if (evt->type == EVENT)
    {
        strcpy(type, TAG_EVENT);
    }
    else
    {
        strcpy(event, TAG_UNK);
        LOG_PRINT(verbose_e, "Unknown event type '%d' [%s|%s] for variable '%s'\n", evt->type, TAG_ALARM, TAG_EVENT, msg->tag);
        return;
    }
    
    /* description [date - time] */
#ifdef DATETIME_IS_ALARM_RISE
    char * description = getDescription(msg->tag);
    sprintf(line, "%s [%s]",
            ((description == NULL) ? "-" : description),
            (msg->begin.isValid()) ? msg->begin.toString("yyyy/MM/dd-HH:mm:ss").toAscii().data() : "-"
                                   );
#else
    if (msg->ack == NULL)
    {
        sprintf(line, "%s [%s]",
                ((description == NULL) ? "-" : description),
                msg->ack->toString("yyyy/MM/dd-HH:mm:ss").toAscii().data()
                );
    }
    else
    {
        sprintf(line, "%s [%s]",
                ((description == NULL) ? "-" : description),
                (msg->begin == NULL) ? "-" : msg->begin->toString("yyyy/MM/dd-HH:mm:ss").toAscii().data()
                                       );
    }
    
#endif
#if 0
    sprintf(line, "%s START:[%s] STOP:[%s]",
            getDescription(msg->tag),
            (msg->begin == NULL) ? "-" : msg->begin->toString("yyyy/MM/dd-HH:mm:ss").toAscii().data(),
            (msg->end == NULL) ? "-" : msg->end->toString("yyyy/MM/dd-HH:mm:ss").toAscii().data()
                                 );
#endif
    LOG_PRINT(verbose_e, "STYLE INDEX %d STYLE '%s'\n", msg->styleindex, StatusColorTable[msg->styleindex]);
    addEvent(line, visibility, description, StatusColorTable[msg->styleindex]);
    return;
}

void alarms::addEvent(char * line, bool visibility, char * id, char * style)
{
    QString text = QString(line);
#if 0
    if (ui->listWidget->findItems(text,Qt::MatchCaseSensitive).count() != 0)
    {
        LOG_PRINT(verbose_e, "event alredy present '%s'\n", line);
        return;
    }
#endif
    if (style != NULL)
    {
        int i;
        QListWidgetItem * item = NULL;
        for (i = 0; i < ui->listWidget->count(); i++)
        {
            if (ui->listWidget->item(i)->text().startsWith(id) == true)
            {
                item = ui->listWidget->item(i);
                break;
            }
        }

        if (item != NULL)
        {
            if (strcasecmp(style, INVISIBLE) != 0)
            {
                if (item->text() != text)
                {
                    item->setText(text);
                    LOG_PRINT(verbose_e, "Adding updating item '%s'\n", item->text().toAscii().data());
                }
                else
                {
                    LOG_PRINT(verbose_e, "Already existing item '%s'\n", item->text().toAscii().data());
                }
            }
            else
            {
                LOG_PRINT(verbose_e, "Hide item '%s'\n", item->text().toAscii().data());
                QListWidgetItem* item = ui->listWidget->takeItem(i);
                delete item;
                return;
            }
        }
        else if (strcasecmp(style, INVISIBLE) != 0)
        {
            item = new QListWidgetItem();
            item->setText(text.trimmed());
#ifdef ALARM_FIRST_TO_LAST
            ui->listWidget->addItem(item);
#else
            ui->listWidget->insertItem(0,item);
#endif
            LOG_PRINT(verbose_e, "Adding new item '%s'\n", item->text().toAscii().data());
        }
        else
        {
            return;
        }
        LOG_PRINT(verbose_e, "'%s'\n",style);
        int r = 0, g = 0, b = 0, ret = 0;
        ret = sscanf(style, "rgb(%d,%d,%d)", &r,&g,&b);
        if (ret == 3)
        {
            LOG_PRINT(verbose_e, "setting color %d %d %d\n",r,g,b)
                    item->setTextColor(QColor(r,g,b));
            LOG_PRINT(verbose_e, "set color %d %d %d\n",r,g,b)
        }
        else
        {
            LOG_PRINT(error_e, "cannot set color for message '%s'\n", line);
        }
        item->setHidden(!visibility);
    }
    else
    {
        LOG_PRINT(verbose_e, "Not visible into the banner\n");
    }
}

void alarms::on_pushButtonHome_clicked()
{
    go_home();
}

void alarms::on_pushButtonBack_clicked()
{
    go_back();
}

void alarms::on_pushButtonHistory_clicked()
{
    goto_page("alarms_history");
}

#ifdef LEVEL_TYPE
void alarms::on_comboBoxLevel_currentIndexChanged(int index)
{
    /* 0 is all level */
    if (_level != index && index < nb_of_level_e)
    {
        _level = index;
        refreshEvent();
    }
}
#endif

void alarms::on_comboBoxType_currentIndexChanged(int index)
{
    /* 0 is all
     * 1 is alarms
     * 2 is event
     */
    switch(index)
    {
    case 0:
        _alarm = true;
        _event = true;
        break;
    case 1:
        _alarm = true;
        _event = false;
        break;
    case 2:
        _alarm = false;
        _event = true;
        break;
    default:
        _alarm = true;
        _event = true;
        break;
    }
    LOG_PRINT(verbose_e, "alarm %d event %d\n", _alarm, _event);
    refreshEvent();
}

void alarms::receiveEvent(__attribute__((unused)) char * msg)
{
    LOG_PRINT(verbose_e, "alarm %s\n", msg);
    if (this->isVisible() == false)
    {
        return;
    }
    refreshEvent();
}

void alarms::refreshEvent()
{
    event_t * event;
    bool visibility = true;
    
    /* to avoid flikering don't remove and refill all the widget but update only the difference */
    //ui->listWidget->clear();
    
    LOG_PRINT(verbose_e,"%d\n", _active_alarms_events_.count());
    
    pthread_mutex_lock(&alarmevents_list_mutex);
    {
        int found = 0;

        /* remove the non active alarm */
        for (int i = 0; i < ui->listWidget->count(); i++)
        {
            for (int j = 0; j < _active_alarms_events_.count(); j++)
            {
                char * description = getDescription(_active_alarms_events_.at(j)->tag);
                LOG_PRINT(verbose_e, "'%s' vs '%s'\n", ui->listWidget->item(i)->text().toAscii().data(), (description == NULL) ? "-" : description);
                if (ui->listWidget->item(i)->text().startsWith((description == NULL) ? "-" : description) == true)
                {
                    found = 1;
                    break;
                }
            }
            if (found == 0)
            {
                LOG_PRINT(verbose_e, "NOT FOUND %s\n", ui->listWidget->item(i)->text().toAscii().data());
                QListWidgetItem * item = ui->listWidget->takeItem(i);
                delete item;
            }
            else
            {
                LOG_PRINT(verbose_e, "FOUND %s\n", ui->listWidget->item(i)->text().toAscii().data());
                found = 0;
            }
        }

        /* update the active alarm if it is necessary */
        for (int i = 0; i < _active_alarms_events_.count(); i++)
        {
            visibility = true;
            LOG_PRINT(verbose_e,"%s\n", _active_alarms_events_.at(i)->tag);
            event = EventHash.find(_active_alarms_events_.at(i)->tag).value();

            /* looking into event db this event to get the level and the type if it is necessary */
            if (event->type == ALARM && _alarm == false)
            {
                LOG_PRINT(verbose_e, "Hide event '%s'\n", event->description);
                visibility = false;
            }
            if (event->type == EVENT && _event == false)
            {
                LOG_PRINT(verbose_e, "Hide event '%s'\n", event->description);
                visibility = false;
            }
            if (_level != level_all_e && event->level > _level)
            {
                LOG_PRINT(verbose_e, "Hide event '%s'\n", event->description);
                visibility = false;
            }
            addEvent(_active_alarms_events_.at(i), visibility);
        }
    }
    pthread_mutex_unlock(&alarmevents_list_mutex);
}

void alarms::on_pushButtonACK_clicked()
{
    if (ui->listWidget->selectedItems().count() > 0)
    {
        int current_index = ui->listWidget->currentRow();
        if (current_index < 0)
        {
            return;
        }
        pthread_mutex_lock(&alarmevents_list_mutex);
        {
            _active_alarms_events_.at(current_index)->isack = true;
            _active_alarms_events_.at(current_index)->ack = QDateTime::currentDateTime();
        }
        pthread_mutex_unlock(&alarmevents_list_mutex);

        emit new_ack(NULL);
        if (ui->listWidget->currentItem())
            ui->listWidget->currentItem()->setSelected(false);
        refreshEvent();
    }
}

void alarms::on_pushButtonACKall_clicked()
{
    bool found = false;

    pthread_mutex_lock(&alarmevents_list_mutex);
    {
        for (int i = 0; i < _active_alarms_events_.count(); i++)
        {
            if (! _active_alarms_events_.at(i)->isack) {
                found = true;
                _active_alarms_events_.at(i)->isack = true;
                _active_alarms_events_.at(i)->ack = QDateTime::currentDateTime();
            }
        }
    }
    pthread_mutex_unlock(&alarmevents_list_mutex);

    if (found)
        emit new_ack(NULL);
    if (ui->listWidget->currentItem())
        ui->listWidget->currentItem()->setSelected(false);
    refreshEvent();
}

void alarms::on_pushButtonHornACK_clicked()
{
    HornACK = true;
}

