/**
 * @file
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief Main page
 */
#ifndef ALARMS_HISTORY_H
#define ALARMS_HISTORY_H

#include "pagebrowser.h"

namespace Ui {
class alarms_history;
}

class alarms_history : public page
{
    Q_OBJECT
    
public:
    explicit alarms_history(QWidget *parent = 0);
    ~alarms_history();
    virtual void reload(void);
    virtual void updateData();

private slots:
    void changeEvent(QEvent * event);

    void on_pushButtonHome_clicked();
    void on_pushButtonBack_clicked();
    void on_pushButtonPrevious_clicked();
    void on_pushButtonNext_clicked();
    void on_comboBoxDate_currentIndexChanged(int index);
    void on_pushButtonSave_clicked();

private:
    bool loadLogFile(int fileNb, bool alarm = true, bool event = true, int level = level_all_e);
    bool loadLogFile(const char * filename, bool alarm = true, bool event = true, int level = level_all_e);

private:
    Ui::alarms_history *ui;
    int _current;
    int _file_nb;
    bool _alarm;
    bool _event;
    int _level;
    QStringList logFileList;
};

#endif // ALARMS_HISTORY_H

