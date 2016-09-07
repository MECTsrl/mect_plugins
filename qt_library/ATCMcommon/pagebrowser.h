/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Page browswer base class
 */
#ifndef _PAGE_BROWSER_H_
#define _PAGE_BROWSER_H_

#include <QHash>
#ifdef HW_KEY_ENABLED
#include <QKeyEvent>
#endif
#include <QPainter>
#include <QMessageBox>
#include <QStack>
#include <QStyleOption>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QHash>

#include "setstyle.h"
#include "common.h"
#include "cross_table_utility.h"
#if defined(ENABLE_ALARMS) || defined(ENABLE_TREND) || defined(ENABLE_STORE)
#include "hmi_logger.h"
#endif

#include "numpad.h"
#include "alphanumpad.h"

#ifdef TARGET
#define SHOW showFullScreen
#else
#define SHOW show
#endif

#define PAGE_PREFIX "page"

/**
 * @brief This defines describe the LED stylesheets
 */
#define STYLE_UNKNOWN    "background-color: rgb(211, 211, 211);"
#define STYLE_PROGRESS   "background-color: rgb(255, 255,   0);"
#define STYLE_ERROR      "background-color: rgb(255,   0,   0);"
#define STYLE_READY      "background-color: rgb(  0, 255,   0);"
#define STYLE_DISCONNECT "background-color: rgb(170, 170, 255);"

#define ERROR_LABEL_PERIOD_MS 500
#define ERROR_SEPARATOR " - "

class page : public QWidget
{
    Q_OBJECT

public:
    explicit page(QWidget *parent = 0);
    ~page();
    virtual void reload(void) = 0;
protected slots:
    virtual void updateData();
    void refreshPage();
    bool go_back(void);
    bool go_home(void);
    bool goto_page(const char * page_name, bool remember = true);
    int getPageNb();
protected:
    bool setTag(QLabel * label, QString value = "");
    bool setTag(QString * label, QString value = "");
    void translateFontSize( QWidget *ui );
protected:
    /* this function is necessary to update the background */
    void paintEvent (QPaintEvent *)
    {
        QStyleOption opt;
        opt.init (this);
        QPainter p (this);
        style ()->drawPrimitive (QStyle::PE_Widget, &opt, &p, this);
    };
    bool create_next_page(page ** p, const char * t);
#ifdef HW_KEY_ENABLED
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent *event);
    void KeyPress(const char * Id);
    void KeyRelease(const char * Id);
#endif
    /* FIXME MTL Currently not used. To convert to new HMI<=>4C interface. */
#if 0
    bool getFormattedVar(const char * varname, bool * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, short int * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, unsigned short int * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, int * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, unsigned int * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, float * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QString * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QLabel * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QLineEdit * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QPushButton * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QSpinBox * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QDoubleSpinBox * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QComboBox * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QComboBox * formattedVar, QStringList map, QLabel * led = NULL);
#if 0
    bool getFormattedVar(const char * varname, QLCDNumber * formattedVar, QLabel * led = NULL);
    bool getFormattedVar(const char * varname, QwtTextLabel * formattedVar, QLabel * led = NULL);
#endif
#endif
    bool setFormattedVar(const char * varname, char * formattedVar);
    char getStatusVar(const char * varname, char * msg = NULL);
    bool setStatusVar(const char * varname, char Status);
    bool setStatusVar(int SynIndex, char Status);

    bool setAsStartPage(char * window);

    int checkLicence(QString * message);
    int checkSDusage();

    int countLine(const char * filename);
    bool zipAndSave(QStringList sourcefiles, QString destfile, bool junkdir = false, QString basedir = "");
    bool signFile(QString srcfile, QString destfile);

#ifdef ENABLE_ALARMS
    char * getDescription(char* tag);
    void setAlarmsBuzzer(int period_ms = ERROR_LABEL_PERIOD_MS);
    void sequentialShowError(QLineEdit * line, int period_ms = ERROR_LABEL_PERIOD_MS);
    bool rotateShowError(QLineEdit * line, int period_ms = ERROR_LABEL_PERIOD_MS);
protected slots:
    void rotateShowErrorSlot();
signals:
    void new_ack(event_msg_t * msg);

#endif
signals:
    /* widget refresh AND request for new value for H-type variables */
    void varRefresh();
protected:
    QTimer * refresh_timer;
    QLabel * labelIcon;
    QLabel * labelTitle;
    QLabel * labelDataOra;
    QLabel * labelUserName;
    int protection_level;
    QStringList variableList;
    bool hideAll(void);
private:
    QLineEdit * _line;
    int _period_ms;

};

extern QHash<QString, page *> ScreenHash;
extern QStack<QString> History;
extern int create_page_nb(page ** p, int pageNb);
extern void printVncDisplayString(char * vncString);

#endif
