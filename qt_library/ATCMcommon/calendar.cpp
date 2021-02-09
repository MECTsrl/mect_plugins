#include "calendar.h"
#include <QtGui>

#define SIZE_X 250
#define SIZE_Y 250

#define ROW_YEARS 3
#define COL_YEARS 4
#define SIZE_CELL_YEARS_X 61 //56
#define SIZE_CELL_YEARS_Y 66 //60

#define ROW_MONTH 3
#define COL_MONTH 4
#define SIZE_CELL_MONTH_X 61
#define SIZE_CELL_MONTH_Y 66

#define ROW_DAY 7
#define COL_DAY 7
#define SIZE_CELL_DAYNAME_X 22
#define SIZE_CELL_DAY_X 35 //32
#define SIZE_CELL_DAY_Y 30 //28

Calendar *calendarpopup = NULL;

Calendar::Calendar(QWidget *parent) :
 QDialog(parent, Qt::Popup)
{
    setSizeGripEnabled(false);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(QRect(0,0, SIZE_X, SIZE_Y));

    // set black background
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::lightGray);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;

    verticalLayout = new QVBoxLayout();
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    horizontalLayout = new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    pushYearAndMonth =new QPushButton();
    pushYearAndMonth->setObjectName(QString::fromUtf8("pushYearAndMonth"));
    pushYearAndMonth->setContentsMargins(0, 0, 0, 0);
    pushYearAndMonth->setFixedWidth(120);
    connect(pushYearAndMonth,SIGNAL(clicked()),this,SLOT(on_clicked_pushYearAndMonth()));

    arrowDOWN =new QPushButton();
    arrowDOWN->setObjectName(QString::fromUtf8("arrowDOWN"));
    arrowDOWN->setContentsMargins(0, 0, 0, 0);
    arrowDOWN->setText("<");
    arrowDOWN->setFixedWidth(50);
    connect(arrowDOWN,SIGNAL(clicked()),this,SLOT(on_clicked_arrowDOWN()));


    arrowUP =new QPushButton();
    arrowUP->setObjectName(QString::fromUtf8("arrowUP"));
    arrowUP->setContentsMargins(0, 0, 0, 0);
    arrowUP->setText(">");
    arrowUP->setFixedWidth(50);
    connect(arrowUP,SIGNAL(clicked()),this,SLOT(on_clicked_arrowUP()));

    horizontalLayout->addWidget (arrowDOWN,0,Qt::AlignLeft);
    horizontalLayout->addWidget (pushYearAndMonth,0,Qt::AlignCenter);
    horizontalLayout->addWidget (arrowUP,0,Qt::AlignRight);

    QFrame *separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine); // Horizontal line
    separatorLine->setFrameShadow(QFrame::Sunken);
    separatorLine->setLineWidth(1);

    tableWidgetYear = new QTableWidget();
    tableWidgetYear->setObjectName(QString::fromUtf8("TableWidgetYear"));
    tableWidgetYear->setContentsMargins(0, 0, 0, 0);

    tableWidgetMonth= new QTableWidget();
    tableWidgetMonth->setObjectName(QString::fromUtf8("TableWidgetMonth"));
    tableWidgetMonth->setContentsMargins(0, 0, 0, 0);



    tableWidgetDay= new QTableWidget();
    tableWidgetDay->setObjectName(QString::fromUtf8("TableWidgetDay"));
    tableWidgetDay->setContentsMargins(0, 0, 0, 0);

    verticalLayout->addLayout (horizontalLayout);
    verticalLayout->addWidget (separatorLine);
    verticalLayout->addWidget (tableWidgetYear);
    verticalLayout->addWidget (tableWidgetMonth);
    verticalLayout->addWidget (tableWidgetDay);
    this->setLayout(verticalLayout);

    selectedYear=0;
    selectedMonth=0;
    selectedYear=0;

    setTableYears();
    setTableMonth();
    setTableDay ();

    tableWidgetYear->setVisible (false);
    tableWidgetMonth->setVisible (false);
    tableWidgetDay->setVisible (true);
}

void Calendar::movePosition(int x, int y)
{

    QRect rec = QApplication::desktop()->screenGeometry();
    int height = rec.height() - 6;
    int width = rec.width() - 6;
    if(x + SIZE_X > width)
    {
        x = width - SIZE_X;
    }
    if(y + SIZE_Y > height)
    {
        y = height - SIZE_Y;
    }
    this->move(x,y);
    this->show();
    this->update();
}

void Calendar::setYear()
{

    QPushButton *pB= (QPushButton*)(sender());
    selectedYear=pB->text ().toInt ();
    arrowUP->setEnabled(true);
    arrowDOWN->setEnabled(true);
    pushYearAndMonth->setText(pB->text ());
}

void Calendar::setMonth()
{

    QPushButton *pB= (QPushButton*)(sender());
    selectedMonth=QDate::fromString( pB->text (),"MMM").month();
    setDayName(selectedYear,selectedMonth);
    tableWidgetYear->setVisible (false);
    tableWidgetMonth->setVisible (true);
    arrowUP->setEnabled(true);
    arrowDOWN->setEnabled(true);
    pushYearAndMonth->setText (QDate::longMonthName (selectedMonth)+" "+QString::number (selectedYear));

}

void Calendar::setDay()
{

    QPushButton *pB= (QPushButton*)(sender());
    selectedDay=pB->text ().toInt ();
    this->setVisible(false);
    accept();
}

QDate Calendar::getDate()
{

    return QDate(selectedYear, selectedMonth, selectedDay);
}

void Calendar::setDate(QDate currentDate)
{
    selectedYear=currentDate.year();
    selectedMonth=currentDate.month();
    selectedDay=currentDate.day();
    pushYearAndMonth->setText (QDate::longMonthName (selectedMonth)+" "+QString::number (selectedYear));
    setYearName();
    setDayName(selectedYear,selectedMonth);
}

void Calendar::on_clicked_arrowUP()
{
    if(!(selectedYear == 2038 && selectedMonth == 12) ) {
        if(tableWidgetDay->isVisible()){

            selectedMonth++;
            if(selectedMonth==13){
                selectedMonth=1;
                selectedYear++;
            }
            setDayName(selectedYear,selectedMonth);
            pushYearAndMonth->setText (QDate::longMonthName (selectedMonth)+" "+QString::number (selectedYear));

        }
        else if(tableWidgetMonth->isVisible()){
            if(selectedYear < 2038 ) {
                selectedYear++;
                setMonthName();
                pushYearAndMonth->setText (QString::number (selectedYear));
            }
        }
        else if( tableWidgetYear->isVisible()){
            if(selectedYear < 2038 ) {
                selectedYear= selectedYear+12;
                setYearName();
                pushYearAndMonth->setText (QString::number (selectedYear-11)+" - "+QString::number (selectedYear));
            }
        }
    }
}

void Calendar::on_clicked_arrowDOWN()
{
    if(!(selectedYear == 1970 && selectedMonth == 1 )) {
        if(tableWidgetDay->isVisible()){
            selectedMonth--;
            if(selectedMonth==0){
                selectedMonth=12;
                selectedYear--;
            }
            setDayName(selectedYear,selectedMonth);
            pushYearAndMonth->setText (QDate::longMonthName (selectedMonth)+" "+QString::number (selectedYear));
        }
        else if(tableWidgetMonth->isVisible()){
            if(selectedYear > 1970 ) {
                selectedYear--;
                setMonthName();
                pushYearAndMonth->setText (QString::number (selectedYear));
            }
        }
        else if( tableWidgetYear->isVisible()){
            if((selectedYear -12) > 1970 ) {
                selectedYear= selectedYear-12;
                setYearName();
                pushYearAndMonth->setText (QString::number (selectedYear-11)+" - "+QString::number (selectedYear));
            }
        }
    }
}

void Calendar::setTableYears()
{

    tableWidgetYear->setRowCount(ROW_YEARS);

    tableWidgetYear->setColumnCount(COL_YEARS);

    tableWidgetYear->verticalHeader()->setVisible(false);

    tableWidgetYear->horizontalHeader()->setVisible(false);

    tableWidgetYear->setSelectionMode( QAbstractItemView::SingleSelection );

    tableWidgetYear->setShowGrid(false);

    for(int i=0;i<ROW_YEARS;i++)
    {
        tableWidgetYear->setRowHeight(i,SIZE_CELL_YEARS_Y);

    }
    for(int i=0;i<COL_YEARS;i++)
    {
        tableWidgetYear->setColumnWidth(i,SIZE_CELL_YEARS_X);
    }

    for(int i=0;i<ROW_YEARS;i++)
    {
         for(int j=0;j<COL_YEARS;j++){
             QPushButton *cellButton=new QPushButton(tableWidgetYear);
             cellButton->setObjectName(QString::fromUtf8("YearButton(%1,%2)").arg (i).arg (j));
             tableWidgetYear->setCellWidget (i,j,cellButton);
             connect(cellButton,SIGNAL(clicked()),this,SLOT(setYear()));
         }
    }
    tableWidgetYear->setVisible (true);

}

void Calendar::setYearName()
{
    int currentYear;
    if(selectedYear==0){
        currentYear= (QDate::currentDate ().year())-(ROW_YEARS*COL_YEARS)+1;
    }
    else{
        currentYear=selectedYear-(ROW_YEARS*COL_YEARS)+1;
    }
    for(int i=0;i<ROW_YEARS;i++)
    {
        for(int j=0;j<COL_YEARS;j++){

            QPushButton *pB;
            pB=(QPushButton*) tableWidgetYear->cellWidget (i,j);
            pB->setText (QString::number (currentYear++));
            pB->setStyleSheet("Text-align:center");
            if(pB->text().toInt() < 1970 || currentYear > 2039 ) {
                pB->setEnabled(false);
            }else {
                pB->setEnabled(true);
                connect(pB,SIGNAL(clicked()),this,SLOT(showMonthTable()));
            }
        }
    }
}


void Calendar::setTableMonth()
{

    tableWidgetMonth->setVisible (false);

    tableWidgetMonth->setRowCount(ROW_MONTH);

    tableWidgetMonth->setColumnCount(COL_MONTH);

    tableWidgetMonth->verticalHeader()->setVisible(false);

    tableWidgetMonth->horizontalHeader()->setVisible(false);

    tableWidgetMonth->setSelectionMode( QAbstractItemView::SingleSelection );

    tableWidgetMonth->setShowGrid(false);

    for(int i=0;i<ROW_MONTH;i++)
    {
        tableWidgetMonth->setRowHeight(i,SIZE_CELL_MONTH_Y);

    }
    for(int i=0;i<COL_MONTH;i++)
    {
        tableWidgetMonth->setColumnWidth(i,SIZE_CELL_MONTH_X);
    }

    for(int i=0;i<ROW_MONTH;i++)
    {
         for(int j=0;j<COL_MONTH;j++){
             QPushButton *cellButton=new QPushButton(tableWidgetMonth);
             cellButton->setObjectName(QString::fromUtf8("MonthButton(%1,%2)").arg (i).arg (j));
             tableWidgetMonth->setCellWidget (i,j,cellButton);
             connect(cellButton,SIGNAL(clicked()),this,SLOT(setMonth()));
         }
    }
    setMonthName();
}


void Calendar::setMonthName()
{
int listMonthNumber=1;

    for(int i=0;i<ROW_MONTH;i++)
    {
         for(int j=0;j<COL_MONTH;j++){

            QPushButton *pB;
            pB=(QPushButton*) tableWidgetMonth->cellWidget (i,j);
            pB->setText (QDate::shortMonthName (listMonthNumber++));
            pB->setStyleSheet("Text-align:center");
            connect(pB,SIGNAL(clicked()),this,SLOT(showDayTable()));
         }
    }
}
void Calendar::showMonthTable()
{

    tableWidgetYear->setVisible (false);
    tableWidgetMonth->setVisible (true);
}

void Calendar::showDayTable()
{
    tableWidgetMonth->setVisible (false);
    tableWidgetDay->setVisible (true);
}
void Calendar::setTableDay()
{

    tableWidgetDay->setRowCount(ROW_DAY);

    tableWidgetDay->setColumnCount(COL_DAY);

    tableWidgetDay->verticalHeader()->setVisible(false);

    tableWidgetDay->horizontalHeader()->setVisible(false);

    tableWidgetDay->setSelectionMode( QAbstractItemView::SingleSelection );

    tableWidgetDay->setShowGrid(false);

    for(int i=0;i<ROW_DAY;i++)
    {
        if(i==0){
            tableWidgetDay->setRowHeight(i,SIZE_CELL_DAYNAME_X);
        }else {
            tableWidgetDay->setRowHeight(i,SIZE_CELL_DAY_Y);
        }
    }
    for(int i=0;i<COL_DAY;i++)
    {
        tableWidgetDay->setColumnWidth(i,SIZE_CELL_DAY_X);
    }

    int counter=0;
    for(int i=0;i<ROW_DAY;i++)
    {
        for(int j=0;j<COL_DAY;j++){
            if(counter>6){

                QPushButton *cellButton=new QPushButton(tableWidgetDay);
                cellButton->setObjectName(QString::fromUtf8("DayButton(%1,%2)").arg (i).arg (j));
                cellButton->setStyleSheet("");
                // cellButton->setFlat(true);
                tableWidgetDay->setCellWidget (i,j,cellButton);
                connect(cellButton,SIGNAL(clicked()),this,SLOT(setDay()));

            }else{
                counter++;
            }
        }
    }
}


void Calendar::setDayName(int year,int month)
{
    int listDayNumber=1;
    QDate date;

    for(int i=0;i<ROW_DAY;i++)
    {
        for(int j=0;j<COL_DAY;j++){

            if(i==0){
                //Days Names
                QTableWidgetItem *item=new QTableWidgetItem();
                item->setTextAlignment(Qt::AlignCenter);
                item->setText(QDate::shortDayName (j+1));
                item->setFont(QFont("Sans Serif",11, QFont::Bold));
                if(j==6){
                    item->setTextColor(QColor(Qt::red));

                }
                tableWidgetDay->setItem(i,j,item);
            }
            else{

                date.setDate (year,month,listDayNumber);



                if( date.isValid ()){
                    if(listDayNumber==1){
                        for(int k=0;k<date.dayOfWeek ();k++){
                            QPushButton *pB;
                            pB=(QPushButton*) tableWidgetDay->cellWidget (i,k);
                            pB->setEnabled (false);
                            pB->setStyleSheet("background-color: ");
                            pB->setText(" ");
                        }
                    }
                    QPushButton *pB;
                    pB=(QPushButton*) tableWidgetDay->cellWidget (i,date.dayOfWeek ()-1);
                    pB->setStyleSheet("Text-align:center");
                    pB->setVisible (true);
                    pB->setEnabled (true);
                    pB->setText (QString::number (listDayNumber));
                    if(selectedDay == listDayNumber){
                        tableWidgetDay->setCurrentCell(i,j);
                        pB->setStyleSheet("background-color: green; font: bold ");
                    }
                    listDayNumber++;
                    if(date.dayOfWeek ()==7)
                        break;

                }else{
                    QPushButton *pB;
                    pB=(QPushButton*) tableWidgetDay->cellWidget (i,j);
                    pB->setStyleSheet("background-color: ");
                    pB->setEnabled (false);
                    pB->setText(" ");
                }
            }
        }
    }
}


void Calendar::on_clicked_pushYearAndMonth()
{

    if(tableWidgetDay->isVisible ()){

        tableWidgetDay->setVisible (false);
        tableWidgetMonth->setVisible (true);
        pushYearAndMonth->setText (QString::number (selectedYear));

    }
    else if(tableWidgetMonth->isVisible ()){
        arrowDOWN->setEnabled(true);
        arrowUP->setEnabled(true);
        tableWidgetMonth->setVisible (false);
        tableWidgetYear->setVisible (true);
        pushYearAndMonth->setText (QString::number (selectedYear-11)+" - "+QString::number (selectedYear));
    }
}
