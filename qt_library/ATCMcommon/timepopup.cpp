#include "timepopup.h"
#include <QtGui>

#define SIZE_X 250 //230
#define SIZE_Y 250 //230
#define SIZE_TAB_X 245
#define SIZE_TAB_Y 245
#define SIZE_TABLE_X 240
#define SIZE_TABLE_Y 240

#define COL_HOUR 6
#define ROW_HOUR 4
#define COL_MINUTE_SECOND 5
#define ROW_MINUTE_SECOND 3

#define SIZE_CELL_HOUR_X 40
#define SIZE_CELL_HOUR_Y 46
#define SIZE_CELL_MINUTE_SECOND_X 47
#define SIZE_CELL_MINUTE_SECOND_Y 54

TimePopup *timepopup = NULL;

TimePopup::TimePopup(QWidget *parent) :
    QDialog(parent, Qt::Popup)
{
    setSizeGripEnabled(false);
    QRect currentPos(0,0,SIZE_X,SIZE_Y);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(currentPos);

    //set background color
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::lightGray);
    this->setAutoFillBackground(true);
    this->setPalette(pal);

    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;

    verticalLayout = new QVBoxLayout();
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    horizontalLayout= new QHBoxLayout();
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    tabWidget=new QTabWidget();
    tabWidget->setObjectName(QString::fromUtf8("tabwidget"));
    tabWidget->setGeometry(QRect(0, 0, SIZE_TAB_X, SIZE_TAB_Y));

    te = new QTimeEdit();

    buttonOkCancel = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    horizontalLayout->addWidget(te);
    horizontalLayout->addWidget(buttonOkCancel);

    verticalLayout->addLayout(horizontalLayout);
    verticalLayout->addWidget(tabWidget);
    this->setLayout(verticalLayout);

    hour = 0;
    min = 0;
    sec = 0;

    subTabName[0]= "   00 ";
    subTabName[1]= "   15 ";
    subTabName[2]= "   30 ";
    subTabName[3]= "   45 ";

    colorStringSelection = "green";
    colorStringBackgroundTab = "green";
    ColorTextItem = Qt::red;

    setUpTableHours();
    setUpTableMinutes();
    setUpTableSeconds();

    connect( buttonOkCancel, SIGNAL(accepted()), this, SLOT(timeAccepted()));
    connect( buttonOkCancel, SIGNAL(rejected()), this, SLOT(timeRejected()));
}

void TimePopup::movePosition(int x, int y)
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
    this->update();
}

void TimePopup::timeAccepted()
{
    this->setVisible(false);
    this->accept();
}
void TimePopup::timeRejected()
{
    this->setVisible(false);
    this->reject();
}

///////////////
/// GetTime////
///////////////
QTime TimePopup::getTime()
{
    return te->time();
}

///////////////
/// SetTime////
///////////////
void TimePopup::setTime(QTime currentTime)
{
    hour = currentTime.hour();
    min = currentTime.minute();
    sec = currentTime.second();

    te->setTime(QTime(hour, min, sec));

    int tab, row, col;

    row = hour / COL_HOUR;
    col = hour % COL_HOUR;
    tableWidgetHours->setCurrentCell(row, col);

    tab = min / (ROW_MINUTE_SECOND * COL_MINUTE_SECOND);
    row = (min - tab * (ROW_MINUTE_SECOND * COL_MINUTE_SECOND)) / COL_MINUTE_SECOND;
    col = min % COL_MINUTE_SECOND;
    tabMinWidget->setCurrentIndex(tab);
    tableWidgetMin[tab]->setCurrentCell(row, col);

    tab = sec / (ROW_MINUTE_SECOND * COL_MINUTE_SECOND);
    row = (sec - tab * (ROW_MINUTE_SECOND * COL_MINUTE_SECOND)) / COL_MINUTE_SECOND;
    col = sec % COL_MINUTE_SECOND;
    tabSecWidget->setCurrentIndex(tab);
    tableWidgetSec[tab]->setCurrentCell(row, col);
}

///////////////////////
/// setUpTableHours////
///////////////////////
void TimePopup::setUpTableHours()
{
    QWidget *TabHours = new QWidget(tabWidget);
    tabWidget->addTab(TabHours, "Hour");
    tableWidgetHours=new QTableWidget(TabHours);
    tableWidgetHours->setGeometry(QRect(0, 0, SIZE_TAB_X, SIZE_TAB_Y-40));
    tableWidgetHours->setRowCount(ROW_HOUR);
    tableWidgetHours->setColumnCount(COL_HOUR);
    tableWidgetHours->verticalHeader()->setVisible(false);
    tableWidgetHours->horizontalHeader()->setVisible(false);
    tableWidgetHours->setSelectionMode( QAbstractItemView::SingleSelection );
    tableWidgetHours->setShowGrid(false);

    for(int i=0;i<ROW_HOUR;i++)
    {
        tableWidgetHours->setRowHeight(i,SIZE_CELL_HOUR_Y);
    }
    for(int j=0;j<COL_HOUR;j++)
    {
        tableWidgetHours->setColumnWidth(j,SIZE_CELL_HOUR_X);
    }

    int hours=0;
    for(int i=0;i<ROW_HOUR;i++)
    {
        for(int j=0;j<COL_HOUR;j++)
        {
            QTableWidgetItem *item=new QTableWidgetItem(QString::number(hours));
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setTextAlignment(Qt::AlignCenter);
            if((j & 1)==0)
            {
                item->setTextColor(Qt::red);
            }
            tableWidgetHours->setItem(i,j,item);
            hours++;
        }
    }
    tableWidgetHours->setStyleSheet("selection-background-color:"+ colorStringSelection);
    connect(tableWidgetHours, SIGNAL(cellClicked(int,int)),this, SLOT(on_itemClickedHours(int,int)));
}

/////////////////////////
/// setUpTableMinutes////
/////////////////////////
void TimePopup::setUpTableMinutes()
{
    QWidget *TabMin = new QWidget(tabWidget);
    tabWidget->addTab(TabMin, "Minute");

    tabMinWidget=new QTabWidget(TabMin);
    tabMinWidget->setGeometry(QRect(0, 0, SIZE_TAB_X, SIZE_TAB_Y));


    QWidget *TabTime[4];
    for(int tabNumber=0;tabNumber<4;tabNumber++)
    {
        TabTime[tabNumber] = new QWidget(TabMin);
        tabMinWidget->addTab(TabTime[tabNumber],subTabName[tabNumber] );
        tableWidgetMin[tabNumber]=new QTableWidget(TabTime[tabNumber]);
        tableWidgetSetup(tableWidgetMin[tabNumber]);
        connect(tableWidgetMin[tabNumber],SIGNAL(cellClicked(int,int)),this,SLOT(on_itemClickedMin(int,int)));

    }
    populateTableMinSec(tableWidgetMin[0],tableWidgetMin[1],tableWidgetMin[2],tableWidgetMin[3]);
}

/////////////////////////
/// setUpTableSeconds////
/////////////////////////
void TimePopup::setUpTableSeconds()
{
    QWidget *TabSec = new QWidget(tabWidget);
    tabWidget->addTab(TabSec, "Second");

    tabWidget->setStyleSheet("QTabBar::tab::selected { background-color : "+colorStringBackgroundTab+"; }");

    tabSecWidget=new QTabWidget(TabSec);
    tabSecWidget->setGeometry(QRect(0, 0, SIZE_TAB_X, SIZE_TAB_Y));

    QWidget *TabTime[4];
    for(int tabNumber=0;tabNumber<4;tabNumber++)
    {
        TabTime[tabNumber] = new QWidget(TabSec);
        tabSecWidget->addTab(TabTime[tabNumber],subTabName[tabNumber] );
        tableWidgetSec[tabNumber]=new QTableWidget(TabTime[tabNumber]);
        tableWidgetSetup(tableWidgetSec[tabNumber]);
        connect(tableWidgetSec[tabNumber],SIGNAL(cellClicked(int,int)),this,SLOT(on_itemClickedSec(int,int)));

    }
    populateTableMinSec(tableWidgetSec[0],tableWidgetSec[1],tableWidgetSec[2],tableWidgetSec[3]);

}

////////////////////////
/// TableWidgetSetup////
////////////////////////
void TimePopup::tableWidgetSetup(QTableWidget *tableWidgetXX)
{
    tableWidgetXX->setGeometry(QRect(0, 0,SIZE_TABLE_X,SIZE_TABLE_Y-5));
    tableWidgetXX->setRowCount(ROW_MINUTE_SECOND);
    tableWidgetXX->setColumnCount(COL_MINUTE_SECOND);
    tableWidgetXX->verticalHeader()->setVisible(false);
    tableWidgetXX->horizontalHeader()->setVisible(false);
    tableWidgetXX->setSelectionMode( QAbstractItemView::SingleSelection );
    tableWidgetXX->setShowGrid(false);
    tableWidgetXX->setStyleSheet("selection-background-color:" + colorStringSelection);

    /*    for(int i=0;i<ROW_MINUTE_SECOND;i++)
        {
          tableWidgetXX->setRowHeight(i,SIZE_CELL_MINUTE_SECOND_Y);

        }
    for(int j=0;j<COL_MINUTE_SECOND;j++)
        {

           tableWidgetXX->setColumnWidth(j,SIZE_CELL_MINUTE_SECOND_X);
        }*/
}

/////////////////////
/// populateTable////
/////////////////////
void TimePopup::populateTableMinSec(QTableWidget *tableWidget00,QTableWidget *tableWidget15,
                                    QTableWidget *tableWidget30,QTableWidget *tableWidget45)
{
    int minsec=0;
    while(minsec!=60)
    {
        for(int i=0;i<ROW_MINUTE_SECOND;i++)
        {
            for(int j=0;j<COL_MINUTE_SECOND;j++)
            {
                QTableWidgetItem *item=new QTableWidgetItem(QString::number(minsec));
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                item->setTextAlignment(Qt::AlignCenter);
                if((j & 1)==0)
                {
                    item->setTextColor(ColorTextItem);
                }
                if(minsec<15)
                {
                    tableWidget00->setItem(i,j,item);

                }
                if(minsec>14&&minsec<30)
                {
                    tableWidget15->setItem(i,j,item);


                }
                if(minsec>29&&minsec<45)
                {
                    tableWidget30->setItem(i,j,item);

                }
                if(minsec>44)
                {
                    tableWidget45->setItem(i,j,item);

                }
                minsec++;
            }
        }
    }
    for(int i=0;i<ROW_MINUTE_SECOND;i++)
    {
        tableWidget00->setRowHeight(i,SIZE_CELL_MINUTE_SECOND_Y);
        tableWidget15->setRowHeight(i,SIZE_CELL_MINUTE_SECOND_Y);
        tableWidget30->setRowHeight(i,SIZE_CELL_MINUTE_SECOND_Y);
        tableWidget45->setRowHeight(i,SIZE_CELL_MINUTE_SECOND_Y);

    }
    for(int j=0;j<COL_MINUTE_SECOND;j++)
    {
        tableWidget45->setColumnWidth(j,SIZE_CELL_MINUTE_SECOND_X);
        tableWidget30->setColumnWidth(j,SIZE_CELL_MINUTE_SECOND_X);
        tableWidget15->setColumnWidth(j,SIZE_CELL_MINUTE_SECOND_X);
        tableWidget00->setColumnWidth(j,SIZE_CELL_MINUTE_SECOND_X);
    }
}

///////////////////////
/// itemHourClicked////
///////////////////////
void TimePopup::on_itemClickedHours(int row, int col)
{
    hour = tableWidgetHours->item(row,col)->text().toInt();
    te->setTime(QTime(hour, min, sec));
}

/////////////////////////
/// itemMinuteClicked////
/////////////////////////
void TimePopup::on_itemClickedMin(int row, int col)
{
    min = tableWidgetMin[tabMinWidget->currentIndex()]->item(row,col)->text().toInt();
    te->setTime(QTime(hour, min, sec));
}

/////////////////////////
/// itemSecondClicked////
/////////////////////////
void TimePopup::on_itemClickedSec(int row, int col)
{
    sec = tableWidgetSec[tabSecWidget->currentIndex() ]->item(row,col)->text().toInt();
    te->setTime(QTime(hour, min, sec));
}
