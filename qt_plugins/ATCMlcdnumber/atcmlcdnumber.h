#ifndef ATCMLCDNUMBER_H
#define ATCMLCDNUMBER_H

#include <QtGui/QWidget>
#include <QtGui/QLCDNumber>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QTimer>

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMlcdnumber : public QLCDNumber
{
    Q_OBJECT
#ifndef TARGET_ARM
    Q_PROPERTY(QString variable READ variable WRITE setVariable)
    Q_PROPERTY(int refresh READ refresh WRITE setRefresh)
    Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus)
#endif

public:
    ATCMlcdnumber(QWidget *parent = 0);
    ~ATCMlcdnumber();
    QString value()    const { return m_value; }
    QString variable() const { return m_variable; }
    int refresh()      const { return m_refresh; }
    char status()      const { return m_status; }
    bool viewStatus()  const { return m_viewstatus; }
    bool startAutoReading();
    bool stopAutoReading();

public Q_SLOTS:
    bool writeValue(QString);
    bool setVariable(QString);
    bool setRefresh(int);
    bool setViewStatus(bool);

protected Q_SLOTS:
    void updateData();

protected:
    QString m_value;
    QString m_variable;
    int m_refresh;
    char m_status;
    bool m_viewstatus;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QTimer * refresh_timer;
};

#endif
