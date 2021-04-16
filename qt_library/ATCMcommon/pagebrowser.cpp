#include <QDateTime>
#include <QLineEdit>
#include <QMessageBox>
#include <QTimer>
#include <math.h>
#include <QFile>
#include <QSettings>
#include <QDebug>

#include <errno.h>
#include <pthread.h>

#include "utility.h"
#include "app_logprint.h"
#include "pagebrowser.h"
#include "app_config.h"

#include "ATCMsystem/system_ini.h"
#include "ATCMsystem/net_conf.h"
#include "ATCMsystem/data_manager.h"
#include "ATCMsystem/display_settings.h"
#include "ATCMsystem/display_test.h"
#include "ATCMsystem/info.h"
#include "ATCMsystem/menu.h"
#include "ATCMsystem/options.h"
#include "ATCMsystem/time_set.h"
#include "page0.h"
#include "defines.h"
#include "global_functions.h"

#include "alarms.h"
#include "alarms_history.h"

#include "store.h"
#include "store_filter.h"

#include "trend.h"
#include "trend_other.h"
#include "trend_option.h"
#include "trend_range.h"

#include "recipe.h"
#include "recipe_select.h"

/** @brief variables used for the change page management */
QStack<QString> History;
QHash<QString, page *> ScreenHash;

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
page::page(QWidget *parent) :
    QWidget(parent)
{
    /* set up the time for set the refresh timeout */
    refresh_timer = new QTimer(this);
    connect(refresh_timer, SIGNAL(timeout()), this, SLOT(refreshPage()));
    refresh_timer->start(REFRESH_MS);
    labelDataOra = NULL;
    labelUserName = NULL;
    labelIcon = NULL;
    labelTitle = NULL;
    /* set the default protection level */
    protection_level = pwd_operator_e;
    //char vncDisplay[64];
    //printVncDisplayString(vncDisplay);
}

void page::refreshPage()
{
    if (! isVisible()) {
        // should never happen
        refresh_timer->stop();
        return;
    }
    updateData();
    if (! isVisible()) {
        // gone to another page ...
        return;
    }
    emit varRefresh();
}

const char *pageNames[] =
{
    "page0",            //   0 unused
    "alarms",           //  -1
    "alarms_history",   //  -2
    "recipe",           //  -3
    "recipe_select",    //  -4
    "store",            //  -5
    "store_filter",     //  -6
    "data_manager",     //  -7
    "display_settings", //  -8
    "display_test",     //  -9
    "info",             // -10
    "item_selector",    // -11
    "menu",             // -12
    "net_conf",         // -13
    "options",          // -14
    "system_ini",       // -15
    "time_set",         // -16
    "trend",            // -17
    "trend_option",     // -18
    "trend_range"       // -19

    // dialogs:
    //         "alphanumpad", "numpad", "qrcode",
    // widgets:
    //         "trend_other",
};

char *getPageName(int pagenum, char *buf)
{
    if (pagenum == 0 || buf == NULL)
    {
        return NULL; // no "page0"
    }
    else if (pagenum > 0)
    {
        sprintf(buf, "page%x", pagenum);
        return buf;
    }
    else if ((-pagenum) < sizeof(pageNames)/sizeof(char *))
    {
        strcpy(buf, pageNames[(-pagenum)]);
        return buf;
    }
    else
        return NULL;
}

int getPageNumber(const char * pagename)
{
    int retval = 0;
    unsigned n = 0;

    if (sscanf(pagename, "page%x", &n) == 1)
    {
        retval = n;
    }
    else
    {
        for (unsigned i = 1; i < sizeof(pageNames)/sizeof(char *); ++i)
        {
            if (strcmp(pagename, pageNames[i]) == 0)
            {
                retval = 0 - i;
                break;
            }
        }
    }
    return retval;
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void page::updateData()
{
    if (! this->isVisible())
    {
        LOG_PRINT(warning_e, "page '%s' not visible but running.\n", this->windowTitle().toAscii().data());
        return;
    }

    int plc_hmi_page = 0;
    readFromDbQuick(ID_PLC_HMI_PAGE, &plc_hmi_page);

    if (plc_hmi_page != 0)
    {
        int current_page = getPageNb(); // returns 0 if qdialog (numpad & c)

        if (current_page && (current_page != plc_hmi_page))
        {
            char buf[42];
            char * s = getPageName(plc_hmi_page, buf);
            if (s) {
                goto_page(s);
            } else {
                doWrite(ID_PLC_HMI_PAGE, &current_page);
            }
        }
    }

    setAlarmsBuzzer();

    int buzzerOn;

    readFromDbQuick(ID_PLC_buzzerOn, &buzzerOn);
    if (buzzerOn != 0) {
        int volume = 100;
        unsigned on_cs = BUZZER_DURATION_MS / 10;
        unsigned off_cs = 0;
        unsigned replies = 1;
        unsigned value;

        if (varNameArray[ID_PLC_BUZZER].tag[0]) {
            // new buzzer management
            readFromDbQuick(ID_PLC_BEEP_VOLUME, &volume);
            value = volume + (on_cs << 8) + (off_cs << 16) + (replies << 24);
            doWrite(ID_PLC_BUZZER, &value);
        } else {
            // workaround for old projects
            beep(on_cs * 10);
        }
    }

    /* check the password timeout in order to logout */
    if (PwdTimeoutSec > 0 && active_password != pwd_operator_e)
    {
        if ((1000 * PwdTimeoutSec) < LastTouch.elapsed())
        {
            active_password = pwd_operator_e;
            if (windowTitle().compare(PwdLogoutPage))
            {
                LOG_PRINT(verbose_e, "password expire after %d seconds. going to '%s' page\n", PwdTimeoutSec, PwdLogoutPage);
                goto_page(PwdLogoutPage);
            }
        }
    }
    
    /* add here te code to be execute everytime */
    if (labelDataOra)
    {
        labelDataOra->setText(QDateTime::currentDateTime().toString("yyyy/MM/dd - HH:mm:ss"));
    }
    if (labelUserName)
    {
        labelUserName->setText(trUtf8("User: %1").arg(PasswordsString[active_password]));
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
page::~page()
{
}

/**
 * @brief hide all pages
 */
bool page::hideAll(void)
{
    /* chek if some windows is still open */
    QHash<QString, page *>::const_iterator i;
    for ( i = ScreenHash.begin(); i != ScreenHash.end() && i.value() != NULL ; i++)
    {
        if ( i.value()->isVisible())
        {
            LOG_PRINT(verbose_e,"hiding (%s)...\n", i.value()->windowTitle().toAscii().data() );
            i.value()->hide();
            LOG_PRINT(verbose_e,"done!\n" );
        }
    }
    LOG_PRINT(verbose_e,"Everything hide.\n" );
    return 0;
}

/**
 * @brief go to a page named page_name
 */
bool page::goto_page(const char * page_name, bool remember)
{
    page * p = NULL;
    // QComboBox workaround
    QList <QComboBox *>list;
    QComboBox *qcb;
    static QString qstr(" ");
    int page_number;

    // qDebug() << QString("goto_page(%1) ... ").arg(page_name);

    if (strcmp(page_name, "BACK") == 0)
    {
        return go_back();
    }
    if (strcmp(page_name, "HOME") == 0)
    {
        return go_home();
    }
    if (strcmp(page_name, "page0") == 0)
    {
        return goto_page(StartPage, false);
    }
    
    LOG_PRINT(verbose_e, "page_name %s\n", page_name);
    
    refresh_timer->stop();
    LOG_PRINT(verbose_e, " %s TIMER STOP\n", this->windowTitle().toAscii().data());
    
    /* destroy trend page */
    if (strcmp(page_name, "trend") == 0 && _last_layout_ != _layout_)
    {
        _last_layout_ = _layout_;
        if(ScreenHash.contains(page_name))
        {
            p = ScreenHash.take(page_name);
            //p->destroy();
            delete p;
            p = NULL;
        }
    }

    /*prepare the next page */
    if(!ScreenHash.contains(page_name))
    {
        create_next_page(&p, page_name);
        if (p == NULL)
        {
            LOG_PRINT(error_e,"Fail to create page '%s'\n", page_name);
            //mymutex.unlock();
            QMessageBox::critical(this,trUtf8("Access Denied"), trUtf8("The requested page '%1' doesn't exist.").arg(page_name));
            refresh_timer->start(REFRESH_MS);
            LOG_PRINT(verbose_e, " %s TIMER START\n", this->windowTitle().toAscii().data());
            goto exit_failure;
        }
        ScreenHash.insert(page_name, p);
        LOG_PRINT(verbose_e,"CREATA NUOVA PAGINA %s\n", page_name);
    }
    else
    {
        p = ScreenHash.value(page_name);
        p->refresh_timer->stop();
        LOG_PRINT(verbose_e, " %s TIMER STOP\n", p->windowTitle().toAscii().data());
        LOG_PRINT(verbose_e,"reload existing page '%s'\n", page_name);
    }

    if (active_password > p->protection_level)
    {
        QMessageBox::critical(this,trUtf8("Access Denied"), trUtf8("Impossible access to '%1': inappropriate privilegies.").arg(p->windowTitle()));
        LOG_PRINT(verbose_e,"active %d, protection %d\n", active_password, p->protection_level);
        goto exit_failure;
    }

    page_number = getPageNumber(page_name);
    doWrite(ID_PLC_HMI_PAGE, &page_number);

    if (remember)
    {
        /* update history */
        History.push(this->windowTitle());
    }
    p->reload();

    // QComboBox workaround
    list = p->findChildren<QComboBox *>();
    foreach (qcb, list)
    {
        qcb->addItem(qstr);
        qcb->removeItem(qcb->count() - 1);
    }

    if (p != this) {
        /* deactivate the old variables */
        if (this->variableList.count() == 0)
        {
            LOG_PRINT(verbose_e, "No variable to deactivate.\n");
        }
        else if (deactivateVarList(this->variableList) == false)
        {
            LOG_PRINT(error_e, "cannot deactivate the variable list\n");
        }

        /* activate the new variables */
        /* send to the plc the active flag for the active variable */
        /* this code will be active only if the variable VAR_TO_DISPLAY is not empty */
        if (p->variableList.count() == 0)
        {
            LOG_PRINT(verbose_e, "No variable to activate.\n");
        }
        else if (activateVarList(p->variableList) == false)
        {
            LOG_PRINT(error_e, "cannot activate the variable list\n");
        }
    }
    hideAll();
    //hideOthers(p);


    p->refresh_timer->start(REFRESH_MS);
    LOG_PRINT(verbose_e, " %s TIMER START\n", p->windowTitle().toAscii().data());
    p->SHOW();
    LOG_PRINT(verbose_e, "New page '%s' (%s) is shown.\n", p->windowTitle().toAscii().data(), page_name );
    // qDebug() << QString("... goto_page(%1) success PLC_HMI_PAGE=%2").arg(page_name).arg(page_number);
    return true;

exit_failure:
    int this_page = getPageNumber(this->windowTitle().toAscii().data());
    doWrite(ID_PLC_HMI_PAGE, &this_page);
    // qDebug() << QString("... goto_page(%1) failure PLC_HMI_PAGE=%2").arg(page_name).arg(this_page);
    return false;
}

/**
 * @brief go to the home page
 */
bool page::go_home(void)
{
    History.clear();
    _trend_data_reload_ = true;
    if (goto_page(HomePage) == false)
    {
        return goto_page(HOMEPAGE_DEF);
    }
    return true;
}

/**
 * @brief go to the previous page
 */
bool page::go_back()
{
    if (isVisible() == false)
    {
        return false;
    }
    if (!History.isEmpty())
    {
        QString page_name = History.pop();
        LOG_PRINT(verbose_e, " FROM (%s) BACK TO (%s)\n", this->windowTitle().toAscii().data(), page_name.toAscii().data());
        return goto_page (page_name.toAscii().data(),false);
    }
    else
    {
        LOG_PRINT(warning_e, "History empty\n");
        go_home();
        return false;
    }
}

/* this section is used for hardwares with phisical key */
#ifdef HW_KEY_ENABLED
/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* for some reason chis event is assocated when the key is pressed!!! */
void page::keyReleaseEvent(QKeyEvent *event)
{
    fprintf(stderr, "A KEY IS PRESSED: 1 - %d\n", event->key());
    //LOG_PRINT(verbose_e, "A KEY IS PRESSED: 1 - %d\n", event->key());
#if 0
    int i;
    switch (event->key() - 0x1000000 + 11)
    {
    case F01:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 1 - %d\n", event->key());
        i = 0;
        break;
    case F02:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 2 - %d\n", event->key());
        i = 1;
        break;
    case F03:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 3 - %d\n", event->key());
        i = 2;
        break;
    case F04:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 4 - %d\n", event->key());
        i = 3;
        break;
    case F05:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 5 - %d\n", event->key());
        i = 4;
        break;
    case F06:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 6 - %d\n", event->key());
        i = 5;
        break;
    case F07:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 7 - %d\n", event->key());
        i = 6;
        break;
    case F08:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 8 - %d\n", event->key());
        i = 7;
        break;
    case F09:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 9 - %d\n", event->key());
        i = 8;
        break;
    case F10:
        //LOG_PRINT(verbose_e,"A KEY IS PRESSED: 0 - %d\n", event->key());
        i = 9;
        break;
    default:
        //LOG_PRINT(verbose_e,"UNKNOWN KEY IS PRESSED: 0 - %d\n", event->key() - 0x1000000);
        return;
        break;
    }
    KeyPress(keymap[i]);
#endif
}

/* WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING */
/* for some reason chis event is assocated when the key is released!!! */
void page::keyPressEvent(QKeyEvent *event)
{
    fprintf(stderr, "A KEY IS RELEASED: 1 - %d\n", event->key());
    LOG_PRINT(verbose_e, "A KEY IS RELEASED: 1 - %d\n", event->key());
#if 0
    int i;
    switch (event->key() - 0x1000000 + 11)
    {
    case F01:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 1 - %d\n", event->key());
        i = 0;
        break;
    case F02:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 2 - %d\n", event->key());
        i = 1;
        break;
    case F03:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 3 - %d\n", event->key());
        i = 2;
        break;
    case F04:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 4 - %d\n", event->key());
        i = 3;
        break;
    case F05:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 5 - %d\n", event->key());
        i = 4;
        break;
    case F06:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 6 - %d\n", event->key());
        i = 5;
        break;
    case F07:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 7 - %d\n", event->key());
        i = 6;
        break;
    case F08:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 8 - %d\n", event->key());
        i = 7;
        break;
    case F09:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 9 - %d\n", event->key());
        i = 8;
        break;
    case F10:
        //LOG_PRINT(verbose_e,"A KEY IS RELEASED: 0 - %d\n", event->key());
        i = 9;
        break;
    default:
        //LOG_PRINT(verbose_e,"UNKNOWN KEY IS RELEASED: 0 - %d\n", event->key() - 0x1000000);
        return;
        break;
    }
    KeyRelease(keymap[i]);
#endif
}

void page::KeyRelease(const char * Id)
{
    if (Id == NULL)
    {
        return;
    }
    //LOG_PRINT(verbose_e, "RELEASE ##################################### '%s'\n", Id);
}

void page::KeyPress(const char * Id)
{
    if (Id == NULL)
    {
        return;
    }
    //LOG_PRINT(verbose_e, "PRESS ##################################### '%s'\n", Id);
}
#endif


/**
 * @brief call the activateVar function for each variable into listVarname
 */
bool page::activateVarList(const QStringList listVarname)
{
    QString varname;
    bool ret = true;
    
    foreach(varname, listVarname)
    {
        if (activateVar(varname.toAscii().data()) != 0)
        {
            LOG_PRINT(error_e, "cannot activate '%s'\n", varname.toAscii().data());
            ret = false;
        }
        else
        {
            LOG_PRINT(verbose_e, "activated '%s'\n", varname.toAscii().data());
        }
    }
    
    return ret;
}

/**
 * @brief call the deactivateVar function for each variable into listVarname
 */
bool page::deactivateVarList(const QStringList listVarname)
{
    QString varname;
    bool ret = true;
    
    foreach(varname, listVarname)
    {
        if (deactivateVar(varname.toAscii().data()) != 0)
        {
            LOG_PRINT(error_e, "cannot deactivate '%s'\n", varname.toAscii().data());
            ret = false;
        }
        else
        {
            LOG_PRINT(verbose_e, "deactivated '%s'\n", varname.toAscii().data());
        }
    }
    
    return ret;
}

/**
 * @brief set the actual status.
 * The status could be:
 * - ERROR:  error flag is set to 1
 * - DONE:   error flag is set to 0
 * - BUSY:   error flag is set to 0
 */
bool page::setStatusVar(int SynIndex, char Status)
{
    Q_UNUSED(SynIndex);
    Q_UNUSED(Status);
    LOG_PRINT(error_e, "called  page::setStatusVar()\n");
    return false;
}

/**
 * @brief set the actual page as start page
 */
bool page::setAsStartPage(char * window)
{
    QSettings settings(CONFIG_FILE, QSettings::IniFormat);
    strcpy(StartPage, window);
    settings.setValue(STARTPAGE_TAG, StartPage);
    settings.sync();
    return true;
}

void page::setAlarmsBuzzer(int period_ms)
{
    int index = 0;
    static int counter = 0;
    
#ifdef BUZZER
    if (! BuzzerAlarm || HornACK)
    {
        LOG_PRINT(verbose_e, "BuzzerAlarm '%d' HornACK '%d'\n", BuzzerAlarm, HornACK);
        return;
    }
    
    /* do the alarm check only any period_ms */
    if (counter * REFRESH_MS < period_ms)
    {
        counter++;
        return;
    }
    counter = 0;

    bool do_beep = false;
    pthread_mutex_lock(&alarmevents_list_mutex);
    {
        for (index = 0; index < _active_alarms_events_.count(); ++index)
        {
            if (_active_alarms_events_.at(index)->type == ALARM && _active_alarms_events_.at(index)->isack == false)
            {
                do_beep = true;
                break;
            }
        }
    }
    pthread_mutex_unlock(&alarmevents_list_mutex);
    if (do_beep)
    {
        int volume = 100;
        unsigned on_cs = (period_ms / 2) / 10;
        unsigned off_cs = 0;
        unsigned replies = 1;
        unsigned value;

        if (varNameArray[ID_PLC_BUZZER].tag[0]) {
            // new buzzer management
            readFromDbQuick(ID_PLC_ALARM_VOLUME, &volume);
            value = volume + (on_cs << 8) + (off_cs << 16) + (replies << 24);
            doWrite(ID_PLC_BUZZER, &value);
        } else {
            // workaround for old projects
            beep(on_cs * 10);
        }
    }
#endif
}

void page::sequentialShowError(QLineEdit * line, int period_ms)
{
    static int index = 0;
    static int counter = 0;
    
    if (_active_alarms_events_.count() == 0)
    {
        return;
    }
    
    if (counter * REFRESH_MS < period_ms)
    {
        counter++;
        return;
    }
    counter = 0;
    
    pthread_mutex_lock(&alarmevents_list_mutex);
    {
        for (; index < _active_alarms_events_.count(); index++)
        {
            /* skip the acknowledged error */
            if (ISBANNER(_active_alarms_events_.at(index)->styleindex) == 0)
            {
                continue;
            }
        }

        if (index >= _active_alarms_events_.count())
        {
            LOG_PRINT(verbose_e, "rewind\n");
            index = 0;
        }
        if (_active_alarms_events_.at(index)->isack)
        {
            line->clear();
            LOG_PRINT(verbose_e, "nothing to show\n");
        }
        else
        {
            line->setText((EventHash.find(_active_alarms_events_.at(index)->tag).value())->description);
            LOG_PRINT(verbose_e, "show '%s'\n", line->text().toAscii().data());
        }
    }
    pthread_mutex_unlock(&alarmevents_list_mutex);
}

void page::rotateShowErrorSlot()
{
    if (rotateShowError(_line, _period_ms))
    {
    }
}

bool page::rotateShowError(QLineEdit * line, int period_ms)
{
    static int shift = 0;
    static int counter = 0;
    QString bannerStr;
    static QString bannerStrOld;
    static QFontMetrics fm(line->font());
    
    _line = line;
    _period_ms = period_ms;
    
    counter = 0;
    bannerStr.clear();

    pthread_mutex_lock(&alarmevents_list_mutex);
    {
        for (int index = 0; index < _active_alarms_events_.count(); index++)
        {
            /* skip the invisible error */
            if (ISBANNER(_active_alarms_events_.at(index)->styleindex) == 0)
            {
                continue;
            }
            if (bannerStr.length() > 0)
            {
                bannerStr.append( ERROR_SEPARATOR );
            }
            bannerStr.append((EventHash.find(_active_alarms_events_.at(index)->tag).value())->description);
        }
    }
    pthread_mutex_unlock(&alarmevents_list_mutex);

    if (bannerStrOld.compare(bannerStr) != 0)
    {
        LOG_PRINT(verbose_e, "new error\n");
        bannerStrOld = bannerStr;
        shift = 0;
    }
    else if (bannerStr.length() != 0)
    {
        LOG_PRINT(verbose_e, "rewind %d >= %d\n", line->width(), fm.width(bannerStr.right(bannerStr.length() - shift)));
        if (line->width() < fm.width(bannerStr.right(bannerStr.length() - shift)))
        {
            LOG_PRINT(verbose_e, "rewind %d >= %d\n", line->width(), fm.width(bannerStr.right(bannerStr.length() - shift)));
            shift++;
        }
        else
        {
            shift = 0;
        }
    }
    if (bannerStr.length() == 0)
    {
        line->clear();
        LOG_PRINT(verbose_e, "nothing to show\n");
    }
    else
    {
        line->setText(bannerStr.right(bannerStr.length() - shift));
        line->setCursorPosition(0);
        LOG_PRINT(verbose_e, "show '%s'\n", line->text().toAscii().data());
    }
    QTimer::singleShot(_period_ms, this, SLOT(rotateShowErrorSlot()));
    return true;
}

char * page::getDescription(char* tag)
{
    if (tag != NULL)
    {
        QHash<QString, event_t *>::const_iterator i = EventHash.find(tag);
        
        if (i != EventHash.end())
        {
            return EventHash.find(tag).value()->description;
        }
    }
    return NULL;
}

int page::countLine(const char * filename)
{
    FILE * fp = NULL;
    int i = 0;
    char line[LINE_SIZE];
    
    fp = fopen (filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "cannot open file '%s'\n", filename);
        return false;
    }
    for (i = 0; fgets(line, LINE_SIZE, fp) != NULL; i++);
    fclose (fp);
    return i;
}

bool page::isBlockFullUsed(int block, QStringList variablelist)
{
    int i;
    for (i = 0; i < DB_SIZE_ELEM && varNameArray[i].block != block; i++)
        ;
    for (; i < DB_SIZE_ELEM && varNameArray[i].block == block; i++)
    {
        if (variablelist.indexOf(varNameArray[i].tag) == -1)
        {
            return false;
        }
    }
    return true;
}

QStringList page::getBlocksToFill(QStringList variablelist)
{
    char varblockhead[TAG_LEN] = "";
    QStringList list;
    int i;
    for (i = 0; i < variablelist.count(); i++)
    {
        int block = getHeadBlockName(variablelist.at(i).toAscii().data(), varblockhead);
        if (block >= 0)
        {
            if (isBlockFullUsed(block, variablelist) == false)
            {
                list << varblockhead;
            }
        }
    }
    return list;
}


void page::translateFontSize( QWidget *ui )
{
    QObjectList l;
    QObject *w;
    
    if (!ui)
        return;
    
    QFont font = QFont(ui->font());
    font.setPointSize( (font.pointSize() *5) / 3);
    ui->setFont(font);
    
    l = ui->children();
    
    foreach (w, l) {
        QWidget *ww = dynamic_cast<QWidget *>(w);
        if (ww) {
            QFont font = QFont(ww->font());
            font.setPointSize( (font.pointSize() *5) / 3);
            ww->setFont(font);
        }
    }
}

int page::getPageNb()
{
    return getPageNumber(this->windowTitle().toAscii().data());
}

bool page::zipAndSave(QStringList sourcefiles, QString destfile, bool junkdir, QString basedir)
{
    if (destfile.length() == 0)
    {
        LOG_PRINT(error_e, "no zip file found \n");
        return false;
    }
    if (!QFile::exists(ZIP_BIN))
    {
        LOG_PRINT(error_e, "no zip command found\n");
        return false;
    }
    for (int i = 0; i < sourcefiles.count(); i++)
    {
        if (!QFile::exists(sourcefiles.at(i)) && sourcefiles.at(i).compare("*") != 0)
        {
            LOG_PRINT(error_e, "cannot find the file to zip '%s'\n", sourcefiles.at(i).toAscii().data());
            return false;
        }
    }

    char command[1024];

    if (basedir.length())
    {
        if (junkdir)
        {
            sprintf(command, "cd %s && %s -r -j %s %s >/dev/null 2>&1", basedir.toAscii().data(), ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
        else
        {
            sprintf(command, "cd %s && %s -r %s %s >/dev/null 2>&1", basedir.toAscii().data(), ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
    }
    else
    {
        if (junkdir)
        {
            sprintf(command, "%s -r -j %s %s >/dev/null 2>&1", ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
        else
        {
            sprintf(command, "%s -r %s %s >/dev/null 2>&1", ZIP_BIN, destfile.toAscii().data(), sourcefiles.join(" ").toAscii().data());
        }
    }

    if (system(command))
    {
        LOG_PRINT(error_e, "cannot execute command: '%s'\n", command);
        return false;
    }
    return true;
}

bool page::signFile(QString srcfile, QString destfile)
{
    char line[STR_LEN];
    /* create the signature file */
    /* Open the command for reading. */
    sprintf(line, "%s %s", APP_SIGN, srcfile.toAscii().data());
    FILE * fp = popen(line, "r");
    if (fp == NULL) {
        LOG_PRINT(error_e,"Failed to run command '%s'\n", line );
        return false;
    }

    char sign[LINE_SIZE];

    /* Read the output a line at a time - output it. */
    if (fscanf(fp, "%s", sign) > 0) {
        LOG_PRINT(verbose_e,"SIGN: '%s'\n", sign);
    }

    /* close */
    pclose(fp);

    if (sign[0] == '\0')
    {
        LOG_PRINT(error_e,"Failed read sign\n");
        return false;
    }

    FILE * fpout = fopen(destfile.toAscii().data(), "w");
    if (fpout == NULL)
    {
        LOG_PRINT(error_e, "cannot open '%s'\n", destfile.toAscii().data());
        return false;
    }
    fprintf(fpout, "%s\n", sign);
    fclose(fpout);
    return true;
}

bool page::setTag(QString * label, QString value)
{
    QHash<QString, QString>::const_iterator i;
    LOG_PRINT(verbose_e,"LOOKING FOR %s\n", value.toAscii().data());
    if (value.length() == 0)
    {
        return false;
    }
    else
    {
        i = TagTable.find(value);
    }
    if (i != TagTable.end())
    {
        *label = i.value();
        LOG_PRINT(verbose_e,"FOUND %s\n", i.value().toAscii().data());
        return true;
    }
    return false;
}

int page::checkSDusage()
{
    if (SDCheck() == 0)
    {
        char filename[256];
        sprintf(filename, "%s/%s", SD_MOUNT_POINT, ".application");
        if (QFile::exists(filename))
        {
            return 1;
        }
        sprintf(filename, "%s/%s", SD_MOUNT_POINT, ".extrastorage");
        if (QFile::exists(filename))
        {
            return 2;
        }
        return 0;
    }
    return -1;
}

/*
   -1 error
   0 no sdcard
   1 sdcard without licence
   2 sdcard with storage licence
   3 sdcard with system licence
 */
int page::checkLicence(QString * message)
{
    if (SDCheck() == 0)
    {
        char cid[CID_LEN + 1];
        message->clear();
        if (getSdCID(cid) != 0)
        {
            LOG_PRINT(error_e, "cannot open CID file '%s'\n", CID_FILE );
            *message = "Invalid SD card detected.";
            return -1;
        }
        
        char command[1024];
        
        /* read licence */
        char licence[CID_LEN + 1];
        /* Open the command for reading. */
        sprintf(command, "%s/%s", SD_MOUNT_POINT, LICENCE_FILE);
        
        FILE * fp = fopen(command, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Failed to run open '%s'\n", command );
            *message = "SD card detected, no licence found.";
            return 1;
        }
        
        /* Read the output a line at a time - output it. */
        if (fscanf(fp, "%s", licence) == 1)
        {
            LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        }
        else
        {
            fclose(fp);
            LOG_PRINT(error_e, "Failed to run command '%s': '%s'\n", command, licence );
            *message = "SD card detected, invalid licence found.";
            return 1;
        }
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* close */
        fclose(fp);
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* compose licence */
        char signedcode[LINE_SIZE];
        /* Open the command for reading. */
        sprintf(command, "echo %s%s | %s", cid, STORAGE_PERMISSION, APP_SIGN);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Failed to run command '%s'\n", command );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* Read the output a line at a time - output it. */
        if (fgets(signedcode, LINE_SIZE, fp) != NULL)
        {
            sscanf(signedcode, "%s", signedcode);
            LOG_PRINT(verbose_e, "signedcode: '%s'\n", signedcode);
        }
        else
        {
            pclose(fp);
            LOG_PRINT(error_e, "Failed to run command '%s': '%s'\n", command, signedcode );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        LOG_PRINT(verbose_e, "licence: '%s'\n", licence);
        /* close */
        pclose(fp);
        
        if (strcmp(licence, signedcode) == 0)
        {
            *message = "SD card detected, storage licence found.";
            LOG_PRINT(verbose_e, "SD card detected, storage licence found.\n");
            return 2;
        }
        
        /* Open the command for reading. */
        sprintf(command, "echo %s%s | %s", cid, APPLICATION_PERMISSION, APP_SIGN);
        fp = popen(command, "r");
        if (fp == NULL)
        {
            LOG_PRINT(error_e, "Failed to run command '%s'\n", command );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        /* Read the output a line at a time - output it. */
        if (fgets(signedcode, LINE_SIZE, fp) != NULL)
        {
            sscanf(signedcode, "%s", signedcode);
            LOG_PRINT(verbose_e, "signedcode: '%s'\n", signedcode);
        }
        else
        {
            pclose(fp);
            LOG_PRINT(error_e, "Failed to run command '%s': '%s'\n", command, signedcode );
            QMessageBox::critical(0,trUtf8("Internal error"), trUtf8("cannot create the code."));
            return -1;
        }
        
        /* close */
        pclose(fp);
        
        if (strcmp(licence, signedcode) == 0)
        {
            *message = "SD card detected, application licence found.";
            LOG_PRINT(verbose_e, "SD card detected, application licence found.\n");
            return 3;
        }
        else
        {
            LOG_PRINT(verbose_e, "'%s'\n'%s'\n", licence, signedcode);
            *message = "SD card detected, No valid licence found.";
            return 1;
        }
        
    }
    else
    {
        *message = "No SD card detected.";
        return 0;
    }
}

/**
 * @brief create a new page
 */
bool page::create_next_page(page ** p, const char * t)
{
    *p = NULL;
    /* special pages */
    if (strncmp(t, PAGE_PREFIX, strlen(PAGE_PREFIX)) != 0)
    {
        if (strcmp(t, "system_ini") == 0)
        {
            *p = (page *)(new system_ini);
        }
        else if (strcmp(t, "net_conf") == 0)
        {
            *p = (page *)(new net_conf);
        }
        else if (strcmp(t, "info") == 0)
        {
            *p = (page *)(new info);
        }
        else if (strcmp(t, "data_manager") == 0)
        {
            *p = (page *)(new data_manager);
        }
        else if (strcmp(t, "display_settings") == 0)
        {
            *p = (page *)(new display_settings);
        }
        else if (strcmp(t, "display_test") == 0)
        {
            *p = (page *)(new display_test);
        }
        else if (strcmp(t, "menu") == 0)
        {
            *p = (page *)(new menu);
        }
        else if (strcmp(t, "options") == 0)
        {
            *p = (page *)(new options);
        }
        else if (strcmp(t, "time_set") == 0)
        {
            *p = (page *)(new time_set);
        }
        else if (strcmp(t, "alarms") == 0)
        {
            *p = (page *)(new alarms);
        }
        else if (strcmp(t, "alarms_history") == 0)
        {
            *p = (page *)(new alarms_history);
        }
        else if (strcmp(t, "store") == 0)
        {
            *p = (page *)(new store);
        }
        else if (strcmp(t, "store_filter") == 0)
        {
            *p = (page *)(new store_filter);
        }
        else if (strcmp(t, "trend") == 0)
        {
            *p = (page *)(new trend);
        }
        else if (strcmp(t, "trend_other") == 0)
        {
            *p = (page *)(new trend_other);
        }
        else if (strcmp(t, "trend_option") == 0)
        {
            *p = (page *)(new trend_option);
        }
        else if (strcmp(t, "trend_range") == 0)
        {
            *p = (page *)(new trend_range);
        }
        else if (strcmp(t, "recipe") == 0)
        {
            *p = (page *)(new recipe);
        }
        else if (strcmp(t, "recipe_select") == 0)
        {
            *p = (page *)(new recipe_select);
        }
        else
        {
            LOG_PRINT(error_e, "cannot create special page %s\n", t);
            return false;
        }
    }
    /* it is an ordinary page */
    else
    {
        int pageNb = getPageNumber(t);

        if (create_page_nb(p, pageNb)!= 0)
        {
            LOG_PRINT(error_e, "cannot create ordinary page %d=0x%x (%s)\n", pageNb, pageNb, t);
            return false;
        }
    }
    return true;
}
