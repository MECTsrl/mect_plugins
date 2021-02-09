#ifndef CALENDAR_H
#define CALENDAR_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QDateEdit>
#include <QPushButton>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QTimeEdit>
#include <QApplication>
#include <QHeaderView>
#include <QDesktopWidget>

class QVBoxLayout;
class QHBoxLayout;
class QDialogButtonBox;
class QDateEdit;
class QPushButton;
class QFrame;
class QTableWidget;

class Calendar : public QDialog
{
        Q_OBJECT
    public:
        Calendar(QWidget *parent = 0);
        
    signals:
        
    public slots:
        QDate getDate();
        void setDate(QDate currentDate);
        void movePosition(int x, int y);




    private slots:
        void setTableYears();
        void setYearName();

        void setMonthName();
        void setTableMonth();
        void showMonthTable();

        void setTableDay();
        void setDayName(int year, int month);
        void showDayTable();

        void on_clicked_pushYearAndMonth();
        void setYear();
        void setMonth();
        void setDay();

        void on_clicked_arrowUP();
        void on_clicked_arrowDOWN();


    private :


        QPushButton *pushYearAndMonth;
        QPushButton *arrowUP;
        QPushButton *arrowDOWN;

        QTableWidget *tableWidgetYear;
        QTableWidget *tableWidgetMonth;
        QTableWidget *tableWidgetDay;

        int selectedYear;
        int selectedMonth;
        int selectedDay;
};

extern Calendar *calendarpopup;

#endif // CALENDAR_H
