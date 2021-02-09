#ifndef TIMEPOPUP_H
#define TIMEPOPUP_H

#include <QWidget>
#include <QDialog>
#include <QTableWidget>
#include <QTabWidget>
#include <QListWidgetItem>
#include <QTimeEdit>

class QVBoxLayout;
class QHBoxLayout;
class QDialogButtonBox;
class QTimeEdit;

class TimePopup :public QDialog
{
  Q_OBJECT
public:
  TimePopup(QWidget *parent=0);
  
signals:

public:


public slots:

QTime getTime();
void setTime(QTime currentTime);
void movePosition(int x, int y);


private slots:

  void setUpTableHours();
  void setUpTableMinutes();
  void setUpTableSeconds();


  void on_itemClickedHours(int row, int col);
  void on_itemClickedMin(int row,int col);
  void on_itemClickedSec(int row, int col);

  void timeAccepted();
  void timeRejected();

  void tableWidgetSetup(QTableWidget *tableWidgetXX);

  void populateTableMinSec(QTableWidget *tableWidget00,QTableWidget *tableWidget15,
                           QTableWidget *tableWidget30,QTableWidget *tableWidget45);


private:
  QDialogButtonBox* buttonOkCancel;
  QTableWidget *tableWidgetHours;
  QTabWidget *tabWidget;
  QTimeEdit *te;
  int hour;
  int min;
  int sec;
  QString colorStringSelection;
  QString colorStringBackgroundTab;
  QColor ColorTextItem;

  QTableWidget *tableWidgetMin[4];
  QTableWidget *tableWidgetSec[4];
  QString subTabName[4];

  QTabWidget *tabMinWidget;
  QTabWidget *tabSecWidget;  
};

extern TimePopup *timepopup;

#endif // TIMEPOPUP_H
