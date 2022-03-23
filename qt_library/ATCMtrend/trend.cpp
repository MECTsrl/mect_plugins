/**
 * @file
 *
 * @section LICENSE
 * CopyLeft Mect s.r.l. 2013
 *
 * @brief Main page
 */
#define __USE_XOPEN // strptime
#include "app_logprint.h"
#include "global_functions.h"
#include "trend.h"
#include "item_selector.h"
#include "ui_trend.h"
#include "utility.h"

#include <QList>
#include <QDate>
#include <QDir>
#include <QStringList>
#include <QFileInfo>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_plot_layout.h>
#include <pthread.h>
#include <ctype.h>
#include <QMessageBox>
#include <unistd.h>
#include <QDesktopWidget>

#include <time.h>

#define OVERLOAD_SECONDS(Visible) (Visible * 12 / 10) // 20%

#define DATE_TIME_FMT "yyyy-MM-dd HH:mm:ss"

#define DEFAULT_TREND QDate::currentDate().toString("yyyy_MM_dd.log").toAscii().data()

/* this define set the window title */
#define WINDOW_TITLE "TREND"
/* this define set the window icon the file can have a path into resource file or into the file system */
#define WINDOW_ICON ""

/**
 * @brief this macro is used to set the PAGE67a style.
 * the syntax is html stylesheet-like
 */
#define SET_TREND() { \
    QString mystyle; \
    mystyle.append(d_qwtplot->styleSheet()); \
    mystyle.append("  background-color: rgb(255, 255, 255);"); \
    mystyle.append("  color: rgb(0, 0, 0);"); \
    mystyle.append("  background-image: url();"); \
    mystyle.append("  font: 9pt \"DejaVu Sans Mono\";"); \
    d_qwtplot->setStyleSheet(mystyle); \
    mystyle.clear(); \
    mystyle.append(ui->labelDate->styleSheet()); \
    mystyle.append("  font: 10pt \"DejaVu Sans Mono\";"); \
    d_qwtplot->setStyleSheet(mystyle); \
    }

// FIXME: if we can change interfaces then transform the following in class members

static bool do_refresh_plot;
static bool do_pan;
static int incrementTimeDirection = 0;
static int incrementValueDirection = 0;

static QRect zoomRect;
static bool do_zoom;
static QString lastLogFileName;
static fpos_t lastLogFilePos;
static time_t lastLog_time;

static bool unzoom_enabled;
static QDateTime unzoom_actualTzero;
static int unzoom_actualVisibleWindowSec;

static DateTimeScaleDraw * theDateTimeScaleDraw;
static int max_sample_nb = MAX_SAMPLE_NB;

static int datetime_ticks(char layout, int width, int height)
{
    int ticks;

    if (layout == LANDSCAPE) {
        // width -> horiz_ticks -> datetime side by side
        if (width <= 320) { // 240 272 320
            ticks = 2;
        } else if (width <= 480) {
            ticks = 5;
        } else if (width <= 800) {
            ticks = 10;
        } else {
            ticks = 10;
        }
    } else {
        // height -> horiz_ticks -> datetime stacked
        if (height <= 320) { // 240 272 320
            ticks = 6;
        } else if (height <= 480) {
            ticks = 10;
        } else if (height <= 800) {
            ticks = 16;
        } else {
            ticks = 16;
        }
    }
    return ticks;
}

static int values_ticks(char layout, int width, int height)
{
    int ticks;

    if (layout == LANDSCAPE) {
        // height -> vert_ticks -> values stacked
        if (height <= 320) { // 240 272 320
            ticks = 8;
        } else if (height <= 480) {
            ticks = 16;
        } else if (height <= 800) {
            ticks = 16;
        } else {
            ticks = 16;
        }
    } else {
        // width -> vert_ticks -> values side by side
        if (width <= 320) { // 240 272 320
            ticks = 2;
        } else if (width <= 480) {
            ticks = 8;
        } else if (width <= 800) {
            ticks = 12;
        } else {
            ticks = 12;
        }
    }
    return ticks;
}

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
trend::trend(QWidget *parent) :
    page(parent),
    ui(new Ui::trend)
{
    ui->setupUi(this);
    ui->pushButtonPen->setVisible(false);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    //setStyle::set(this);

    /* initialization */
    //max_sample_nb = parent->geometry().width(); // 240 272 320 480 600 800
    max_sample_nb = QApplication::desktop()->screenGeometry().width();

    if (max_sample_nb < 42 || max_sample_nb > MAX_SAMPLE_NB) {
        max_sample_nb = MAX_SAMPLE_NB;
    }
    for (int i = 0; i < PEN_NB; i++)
    {
        pens[i].x = new double [max_sample_nb + 1];
        pens[i].y = new double [max_sample_nb + 1];
        pens[i].curve = NULL; // new InterruptedCurve(); // QwtPlotCurve();
    }
    
    actualPen = 0;
    
    d_qwtplot = NULL;
    d_picker = NULL;
    valueAxisId = 0;
    timeScale  = NULL;

    d_qwtplot = ui->qwtPlot;
    d_qwtplot->hide();

    //    d_qwtplot->plotLayout()->setAlignCanvasToScales(true);
    //    d_qwtplot->plotLayout()->setCanvasMargin 	( 40, -1 );
    d_qwtplot->setAxesCount( QwtPlot::xBottom, 1 );
    d_qwtplot->setAxesCount( QwtPlot::xTop, PEN_NB );
    d_qwtplot->setAxesCount( QwtPlot::yLeft, PEN_NB );
    theDateTimeScaleDraw = NULL;

    /* set the trend stylesheet */
    SET_TREND();
    
    /* setup the picker in order to capture a rect */
    d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                 QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                 d_qwtplot->canvas());
    d_picker->setStateMachine(new QwtPickerDragRectMachine());
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setRubberBandPen(QColor(0,0,0));
    d_picker->setTrackerPen(QColor(255,255,255,0)); // transparent
    d_picker->setTrackerMode(QwtPicker::ActiveOnly);
    d_picker->setEnabled(true);
    
    /* connect the picker signals */
    connect(d_picker, SIGNAL(moved(const QPoint &)),
            SLOT(moved(const QPoint &)));
    connect(d_picker, SIGNAL(selected(const QPolygon &)),
            SLOT(selected(const QPolygon &)));
    
    /* setup the grid */
    grid = new QwtPlotGrid;
    grid->enableX(true);
    grid->enableXMin(true);
    grid->enableY(true);
    grid->enableYMin(true);
    grid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
    grid->attach(d_qwtplot);
    d_qwtplot->setAutoReplot(true);
    
    _trend_data_reload_ = true;
    _load_window_busy = false;
    reloading = false;
    force_back = false;
    first_time = false;
    overloadActualTzero = false;
    lastLogFileName.clear();
    lastLog_time = 0;

    actualVisibleWindowSec = 0;

    popup = new trend_other(this);
    popup_visible = false;
    popup->hide();
    actualPen = 0;

    do_refresh_plot = false;
    do_pan = false;
    unzoom_enabled = false;
    incrementTimeDirection = 0;
    incrementValueDirection = 0;
    do_zoom = false;
    zoomRect.setX(0);
    zoomRect.setY(0);
    zoomRect.setHeight(0);
    zoomRect.setWidth(0);
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void trend::reload()
{
    reloading = true;
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void trend::updateData()
{
    if (! this->isVisible())
    {
        return;
    }

    /* call the father update data  */
    page::updateData();

    if (reloading)
    {
        reloading = false;
        showStatus(trUtf8("Loading..."), false);

        /* disable all buttons during loading */
        ui->pushButtonPan->setEnabled(false);
        popup->enableButtonUp(false);
        popup->enableButtonDown(false);
        popup->enableButtonLeft(false);
        popup->enableButtonRight(false);

        d_qwtplot->hide(); // setVisible(false);
        QCoreApplication::processEvents();

        if (_trend_data_reload_)
        {
            actualPen = 0;
            for (int z = 0; z < PEN_NB; z++)
            {
                pens[z].sample = 0;
            }
            d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotCurve);
            d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotScale);
            d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotUserItem);
            if (strlen(_actual_trend_) == 0)
            {
                force_back = true;
                return;
            }
            if (LoadTrend(_actual_trend_, &errormsg) == false)
            {
                force_back = true;
                return;
            }
            for (int z = 0; z < PEN_NB; z++)
            {
                // must do this
                pens[z].curve = new InterruptedCurve(); // QwtPlotCurve();
            }
            _trend_data_reload_ = false;
        }

        // reset all        
        if (actualVisibleWindowSec == 0)
        {
            Tzero = QDateTime::currentDateTime().addSecs(-LogPeriodSecF);
            VisibleWindowSec = MaxWindowSec;

            actualTzero = Tzero;
            actualVisibleWindowSec = VisibleWindowSec;

            enableZoomMode(false);
            setOnline(true);
        }

        VisibleWindowSec = actualVisibleWindowSec;
        LoadedWindowSec = OVERLOAD_SECONDS(VisibleWindowSec);
        TrendPeriodSec = LoadedWindowSec / max_sample_nb;
        if (TrendPeriodSec < LogPeriodSecF)
            TrendPeriodSec = LogPeriodSecF;

        /* set TzeroLoaded to force the data load */
        TzeroLoaded = Tzero.addSecs(-1);

        if (_layout_ == PORTRAIT)
        {
            timeAxisId = QwtPlot::yLeft;
            valueAxisId = QwtPlot::xTop;
        }
        else
        {
            timeAxisId = QwtPlot::xBottom;
            valueAxisId = QwtPlot::yLeft;
        }

        for (int i = 0; i < PEN_NB; i++)
        {
            LOG_PRINT(verbose_e, "SCALE %d, min %f max %f\n", valueAxisId + i, pens[i].yMin, pens[i].yMax);
            if (pens[i].curve != NULL)
            {
                pens[i].curve->attach(d_qwtplot);
                if (_layout_ == PORTRAIT)
                {
                    LOG_PRINT(verbose_e, "PORTRAIT\n");
                    pens[i].curve->setAxes(QwtAxisId( valueAxisId, i ), QwtAxisId( timeAxisId, 0 ));
                }
                else
                {
                    LOG_PRINT(verbose_e, "LANDSCAPE\n");
                    pens[i].curve->setAxes(QwtAxisId( timeAxisId, 0 ), QwtAxisId( valueAxisId, i ));
                }
                pens[i].curve->setPen(QPen(QColor(QString("#%1").arg(pens[i].color)),2));
            }
            LOG_PRINT(verbose_e, "####### Resetting axis curve %d\n", i);
            for ( int axis = 0; axis < QwtAxis::PosCount; axis++ )
            {
                d_qwtplot->setAxisVisible( axis, false );
            }
            d_qwtplot->setAxisVisible( QwtAxisId( QwtPlot::xBottom, i ), false);
            d_qwtplot->setAxisScaleDraw(QwtPlot::xBottom + i, NULL);
            d_qwtplot->setAxisMaxMajor(QwtPlot::xBottom + i, 0);
            d_qwtplot->setAxisMaxMinor(QwtPlot::xBottom + i, 0);

            d_qwtplot->setAxisVisible( QwtAxisId( QwtPlot::xTop, i ), false);
            d_qwtplot->setAxisScaleDraw(QwtPlot::xTop + i, NULL);
            d_qwtplot->setAxisMaxMajor(QwtPlot::xTop + i, 0);
            d_qwtplot->setAxisMaxMinor(QwtPlot::xTop + i, 0);

            d_qwtplot->setAxisVisible( QwtAxisId( QwtPlot::yLeft, i ), false);
            d_qwtplot->setAxisScaleDraw(QwtPlot::yLeft + i, NULL);
            d_qwtplot->setAxisMaxMajor(QwtPlot::yLeft + i, 0);
            d_qwtplot->setAxisMaxMinor(QwtPlot::yLeft + i, 0);

            d_qwtplot->setAxisVisible( QwtAxisId( QwtPlot::yRight, i ), false);
            d_qwtplot->setAxisScaleDraw(QwtPlot::yRight + i, NULL);
            d_qwtplot->setAxisMaxMajor(QwtPlot::yRight + i, 0);
            d_qwtplot->setAxisMaxMinor(QwtPlot::yRight + i, 0);
        }
        theDateTimeScaleDraw = new DateTimeScaleDraw(TzeroLoaded); // QDateTime::currentDateTime());
        d_qwtplot->setAxisScaleDraw(timeAxisId, theDateTimeScaleDraw);

        QCoreApplication::processEvents();
        d_qwtplot->show(); // setVisible(true);

        first_time = true;
    }

    if (force_back)
    {
        force_back = false;

        /* select a new trend from CUSTOM_TREND_DIR directory */
        QStringList trendList;
        QDir trendDir(CUSTOM_TREND_DIR, "*.csv");

        trendList = trendDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
        if (trendList.count() > 0)
        {
            QString value;
            item_selector * sel = new item_selector(trendList, &value, trUtf8("TREND SELECTOR"));
            sel->showFullScreen();

            if (sel->exec() == QDialog::Accepted)
            {
                strcpy(_actual_trend_, QFileInfo(value).baseName().toAscii().data());
                if (LoadTrend(_actual_trend_, &errormsg) == false)
                {
                    force_back = true;
                    errormsg = trUtf8("The trend description (%1) is not valid. %2").arg(_actual_trend_).arg(errormsg);
                    showStatus(errormsg, true);
                    errormsg.clear();
                } else {
                    reloading = true;
                }
                delete sel;
                return;
            }
            delete sel;
            errormsg = trUtf8("No trend selected");
            go_back();
            return;
        }
        errormsg = trUtf8("No trend to show");
        showStatus(errormsg, true);
        errormsg.clear();

        actualVisibleWindowSec = 0;
        go_back();
        return;
    }

    if (first_time)
    {
        first_time = false;
                
        actualPen = actualPen - 1;
        on_pushButtonSelect_clicked();

        do_refresh_plot = true;
        incrementTimeDirection = 0;
        incrementValueDirection = 0;

        disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
        connect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)),Qt::QueuedConnection);
    }
    
    LOG_PRINT(verbose_e, "UPDATE\n");
    if (popup_visible)
    {
        popup->reload();
        popup->show();
        popup->raise();
    }

    // from UP, DOWN, LEFT, RIGHT buttons
    if (incrementTimeDirection != 0)
    {
        if (incrementTimeDirection < 0)
            setOnline(false);

        int increment = actualVisibleWindowSec / datetime_ticks(_layout_, this->width(), this->height());
        if (increment < LogPeriodSecF)
            increment = LogPeriodSecF;

        /* update the actual window parameters */
        actualTzero = actualTzero.addSecs(increment * incrementTimeDirection);
        if (actualTzero > QDateTime::currentDateTime())
        {
            actualTzero = QDateTime::currentDateTime();
        }

        do_refresh_plot = true;
        incrementTimeDirection = 0;
    }
    if (incrementValueDirection != 0)
    {
        for (int i = 0; i < PEN_NB; i++)
        {
            float delta = (pens[i].yMaxActual - pens[i].yMinActual) / values_ticks(_layout_, this->width(), this->height());
            pens[i].yMinActual += (delta * incrementValueDirection);
            pens[i].yMaxActual += (delta * incrementValueDirection);
        }

        do_refresh_plot = true;
        incrementValueDirection = 0;
    }

    // zoomed
    if (do_zoom)
    {
        int myXin, myXfin;

        if (_layout_ == PORTRAIT)
        {
            myXin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), zoomRect.y()));
            myXfin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), zoomRect.y() + zoomRect.height()));
        }
        else
        {
            myXin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), zoomRect.x()));
            myXfin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), zoomRect.x() + zoomRect.width()));
        }

        /* if necessary swap the selection */
        if (myXin > myXfin)
        {
            int tmp = myXfin;
            myXfin = myXin;
            myXin = tmp;
        }

        if ((myXfin - myXin) / TrendPeriodSec <= 2)
        {
            LOG_PRINT(warning_e, "zoom too big. number of sample %d\n", (myXfin - myXin) / TrendPeriodSec);
            return;
        }

        for (int i = 0; i < PEN_NB; i++)
        {
            if (_layout_ == PORTRAIT)
            {
                pens[i].yMinActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), zoomRect.x());
                pens[i].yMaxActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), zoomRect.x() + zoomRect.width());
            }
            else
            {
                pens[i].yMinActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), zoomRect.y());
                pens[i].yMaxActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), zoomRect.y() + zoomRect.height());
            }

            /* if necessary swap the selection */
            if (pens[i].yMinActual > pens[i].yMaxActual)
            {
                double tmp = pens[i].yMaxActual;
                pens[i].yMaxActual = pens[i].yMinActual;
                pens[i].yMinActual = tmp;
            }
        }
        LOG_PRINT(verbose_e, "ZOOOOOOOOM Tmin %d - %s Tmax %d - %s Current %s\n",
                  myXin, TzeroLoaded.addSecs(myXin).toString(DATE_TIME_FMT).toAscii().data(),
                  myXfin, TzeroLoaded.addSecs(myXfin).toString(DATE_TIME_FMT).toAscii().data(),
                  QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data()
                  );

        if (TzeroLoaded.addSecs(myXfin) < QDateTime::currentDateTime())
        {
            setOnline(false);
        }

        /* update the actual window parameters */
        if (! unzoom_enabled)
        {
            unzoom_enabled = true;
            unzoom_actualTzero = actualTzero;
            unzoom_actualVisibleWindowSec = actualVisibleWindowSec;
        }
        ui->pushButtonPan->setEnabled(true);

        actualVisibleWindowSec = myXfin - myXin;
        if (_layout_ == PORTRAIT)
        {
            actualTzero = TzeroLoaded.addSecs(myXfin);
        }
        else
        {
            actualTzero = TzeroLoaded.addSecs(myXin);
        }

        do_refresh_plot = true;
        do_zoom = false;
    }

    // disable zoom mode
    if (do_pan)
    {
        /* window pan ->  Tzero, VisibleWindowSec, pens[actualPen].yMin and pens[actualPen].yMax */
        enableZoomMode(false);

        for (int z = 0; z < PEN_NB; z++)
        {
            pens[z].yMinActual = pens[z].yMin;
            pens[z].yMaxActual = pens[z].yMax;
        }

        if (unzoom_enabled && unzoom_actualTzero.isValid())
        {
            actualTzero = unzoom_actualTzero;
            actualVisibleWindowSec = unzoom_actualVisibleWindowSec;
        }
        else
        {
            actualTzero = TzeroLoaded;
            actualVisibleWindowSec = VisibleWindowSec;
        }
        unzoom_enabled = false;
        ui->pushButtonPan->setEnabled(false);

        do_refresh_plot = true;
        do_pan = false;
    }

    /* update the graph only if the status is online or if the new samples are visible */
    static int tic = 0;
    if (_online_ || do_refresh_plot)
    {    
        if (_online_)
        {
            if (tic % 2 == 0) // each second
            {
                do_refresh_plot = true;
                /* online mode: if necessary center the actual data */
                QDateTime now =  QDateTime::currentDateTime();

                actualTzero = now.addSecs(- actualVisibleWindowSec);
            }
            if (tic % 4 == 0) // each even second
            {
                ui->pushButtonOnline->setStyleSheet("QPushButton {"
                                                    "border-image: url(:/libicons/img/Chess1.png);"
                                                    "qproperty-focusPolicy: NoFocus;"
                                                    "}");
                ui->pushButtonOnline->repaint();
            } else if (tic % 4 == 2) { // each odd second
                ui->pushButtonOnline->setStyleSheet("QPushButton {"
                                                    "border-image: url(:/libicons/img/Chess2.png);"
                                                    "qproperty-focusPolicy: NoFocus;"
                                                    "}");
                ui->pushButtonOnline->repaint();
            }
        }

        if (do_refresh_plot)
        {
            do_refresh_plot = false;
            loadOrientedWindow();
        }
    }
    else if (tic % 2 == 0) // each second
    {
        ui->pushButtonOnline->setStyleSheet("QPushButton {"
                                            "border-image: url(:/libicons/img/Chess.png);"
                                            "qproperty-focusPolicy: NoFocus;"
                                            "}");
        ui->pushButtonOnline->repaint();
    }
    ++tic;
}

void trend::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

/**
 * @brief This is the distructor member. The operation written here, are executed only one time when the page will be deleted.
 */
trend::~trend()
{
    delete ui;
    for (int i = 0; i < PEN_NB; i++)
    {
        if (pens[i].x != NULL)
            delete pens[i].x;
        if (pens[i].y != NULL)
            delete pens[i].y;
        if (pens[i].curve != NULL)
            delete pens[i].curve;
        pens[i].x = NULL;
        pens[i].y = NULL;
        pens[i].curve = NULL;
    }
}

void trend::refreshEvent(trend_msg_t item_trend)
{   
    for (int i = 0; i < PEN_NB; i++)
    {
        if (! pens[i].visible)
        {
            continue;
        }
        if (pens[i].CtIndex == item_trend.CtIndex)
        {
            if ( _online_
              || item_trend.timestamp < actualTzero.addSecs(actualVisibleWindowSec)
              || pens[i].x[pens[i].sample] < actualVisibleWindowSec
               )
            {
                do_refresh_plot = true;
            }
            break;
        }
    }
    return;
}

void trend::on_pushButtonPen_clicked()
{
    //    disableUpdate();
    //    goto_page("trend_option");
}

void trend::on_pushButtonSelect_clicked()
{
    ui->pushButtonSelect->setEnabled(false);
    
    LOG_PRINT(verbose_e, "actual pen %d\n", actualPen);
    
    /* show the next trace */
    actualPen = ((actualPen + 1) % PEN_NB);
    if (pens[actualPen].visible)
    {
        bringFront(actualPen);
    }
    
    ui->labelDate->setText("     ");
    ui->labelDate->repaint();
    updatePenLabel();
    
    /* update the pushButtonPenColor text with the current pen color */
    if (strlen(pens[actualPen].color) != 0)
    {
        LOG_PRINT(verbose_e, "COLOR %s\n", pens[actualPen].color);
        ui->pushButtonSelect->setStyleSheet(
                    QString(
                        "QPushButton"
                        "{"
                        "border: 20px solid  #%1;"
                        "qproperty-focusPolicy: NoFocus;"
                        "}"
                        ).arg(pens[actualPen].color)
                    );

    }
    
    /* enable the actual axis */
    for (int i = 0; i < PEN_NB; i++)
    {
        LOG_PRINT(verbose_e, "####### axis curve %d set status %d\n", i, (i == actualPen));
        d_qwtplot->setAxisVisible( QwtAxisId( valueAxisId, i ), (i == actualPen));
    }
    //    d_qwtplot->plotLayout()->setAlignCanvasToScales(true);
    //    d_qwtplot->plotLayout()->setCanvasMargin 	( 40, -1 );
    d_qwtplot->replot();
    ui->pushButtonSelect->setEnabled(true);
}

void trend::on_pushButtonZoom_toggled(bool checked)
{
    enableZoomMode(checked);
}

void trend::on_pushButton_clicked()
{
    if (!popup->isVisible())
    {
        popup->move(this->width() - (2 * popup->width()), 0);
        popup->reload();
        popup->show();
        popup->raise();
        popup_visible = true;
    }
    else if (popup->isVisible())
    {
        popup->hide();
        popup_visible = false;
        return;
    }
}

void trend::setPan()
{    
    do_pan = true;
}

void trend::enableZoomMode(bool on)
{
    _zoom = on;
    if (_zoom)
    {
        d_picker->setRubberBand(QwtPicker::RectRubberBand);
    }
    else
    {
        d_picker->setRubberBand(QwtPicker::CrossRubberBand);
        ui->labelvalue->setText("");
        ui->labelvalue->setStyleSheet("");
        ui->labelvalue->setVisible(false);
        ui->labelPen->setVisible(true);
    }
    ui->pushButtonZoom->setChecked(_zoom);
}

bool trend::setOnline(bool status)
{
    _online_ = status;
    LOG_PRINT(verbose_e, "set %s\n", (_online_ == true) ? "ONLINE": "OFFLINE");
    if (popup != NULL && popup->isVisible())
    {
        LOG_PRINT(verbose_e, "RELOAD POPUP\n");
        popup->reload();
    }
    return _online_;
}

bool trend::getOnline()
{
    LOG_PRINT(verbose_e, "_online_ %d\n", _online_);
    return _online_;
}

bool trend::setRange()
{
    disableUpdate();
    return goto_page("trend_range");
}

bool trend::printGraph()
{
    setOnline(false);
    /* Edit the selected item */
    char fullfilename[FILENAME_MAX] = "";
    char value[DESCR_LEN] = "";
    alphanumpad * dk;
    
    sprintf(fullfilename, "%s", actualTzero.toString("yyyy_MM_dd_HH_mm_ss").toAscii().data());
    
    dk = new alphanumpad(value, fullfilename);
    dk->showFullScreen();
    
    if (dk->exec() == QDialog::Accepted && strlen(value) != 0)
    {
        sprintf(fullfilename, "%s/%s/%s.png", SCREENSHOT_DIR, _actual_trend_, value);
        if (QDir().exists(QString("%1/%2").arg(SCREENSHOT_DIR).arg(_actual_trend_)) == false)
        {
            QDir().mkdir(QString("%1/%2").arg(SCREENSHOT_DIR).arg(_actual_trend_));
        }
        LOG_PRINT(verbose_e, "Saving to '%s'\n", value);
        QPixmap::grabWidget(this->d_qwtplot).save(fullfilename);
        QMessageBox::information(this,trUtf8("Saved data"), trUtf8("The trend graph is saved to '%1'.").arg(QFileInfo(fullfilename).baseName()));
        delete dk;
        return true;
    }
    else
    {
    }
    delete dk;
    return false;
}

bool trend::bringFront(int pen)
{
    if (_load_window_busy)
    {
        return false;
    }
    
    _load_window_busy = true;
    if (pens[pen].curve != NULL)
    {
        pens[pen].curve->detach();
        pens[pen].curve->attach(d_qwtplot);
        _load_window_busy = false;
        return true;
    }
    else
    {
        _load_window_busy = false;
        return false;
    }
}

void trend::disableUpdate()
{
    LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
    disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
}

bool trend::Load(QDateTime begin, QDateTime end, int skip)
{
    int logfound = 0;
    
    LOG_PRINT(verbose_e, "begin '%s' end '%s' skip '%d'\n", begin.toString(DATE_TIME_FMT).toAscii().data(), end.toString(DATE_TIME_FMT).toAscii().data(), skip);
    
    /* the store are in STORE_DIR and they have a name like yyyy_MM_dd_HH_mm_ss.log */
    /* get the list of the file and parse the file between time begin and time end */
    QDir logDir(STORE_DIR);
    QStringList logFileList = logDir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);

    for (int i = 0; i < logFileList.count(); i++)
    {
        QDateTime lastLogDateTime = QDateTime::fromTime_t(lastLog_time);

        if (i + 1 < logFileList.count())
        {
            QDateTime nextLogFileBegin = QDateTime::fromString(QFileInfo(logFileList.at(i + 1)).baseName(), "yyyy_MM_dd_HH_mm_ss");

            if (nextLogFileBegin <= begin || (! lastLogFileName.isEmpty() && nextLogFileBegin <= lastLogDateTime) )
            {
                // too early
                continue;
            }
        }
        QDateTime logFileBegin = QDateTime::fromString(QFileInfo(logFileList.at(i)).baseName(), "yyyy_MM_dd_HH_mm_ss");
        if (end < logFileBegin)
        {
            // too late
            break;
        }

        // QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 500); // max 500 ms
        QCoreApplication::processEvents();

        if ( logFileBegin <= begin || logFileBegin <= end )
        {
            if (lastLogFileName == logFileList.at(i))
            {
                // update from file
                if (! Load(NULL, &begin, &end, skip))
                {
                    lastLogFileName.clear();
                }
            }
            else
            {
                // load from file
                lastLog_time = logFileBegin.toTime_t();
                if (! Load(QString("%1/%2").arg(STORE_DIR).arg(logFileList.at(i)).toAscii().data(), &begin, &end, skip))
                {
                    lastLogFileName.clear();
                }
                else
                {
                    lastLogFileName = logFileList.at(i);
                }
            }
            logfound++;
        }
    }

    return (logfound > 0);
}

bool trend::Load(const char * filename, QDateTime * begin, QDateTime * end, int skip)
{
    bool retval = false;
    char line[LINE_SIZE] = "";
    char * p = NULL, * r = NULL;
    time_t t0 = TzeroLoaded.toTime_t();
    time_t ti = begin->toTime_t();
    time_t tf = end->toTime_t();
    char buf[42];
    QList<int> filter;
    FILE * fp = NULL;
    int samples[PEN_NB] = {0, 0, 0, 0};
    bool updating = false;
    char pathname[FILENAME_MAX];

    if (filename == NULL)
    {
        updating = true;
        strcpy(pathname, QString("%1/%2").arg(STORE_DIR).arg(lastLogFileName).toAscii().data());
    }
    else
    {
        strcpy(pathname, filename);
    }

    fp = fopen (pathname, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "no trend file '%s'\n", pathname);
        return false;
    }

    LOG_PRINT(verbose_e, "FOUND LOG FILE '%s'\n", pathname);
    // fprintf(stderr, "trend: loading (%s)\n", (filename ? filename : "(null)"));

    /*
     * File format:
     * date      ;     time;   tag1 ; ...;   tagN
     * yyyy/MM/dd; HH:mm:ss; 123.456; ...; 123.456
     */

    /* extract header */
    int row = 0;
    if (fgets(line, LINE_SIZE, fp) == NULL)
    {
        LOG_PRINT(error_e, "Cannot read the header from '%s'\n", pathname);
        rewind(fp);
        goto exit_function;
    }

    /* date */
    p = strtok_csv(line, SEPARATOR, &r);
    if (p == NULL || p[0] == '\0' || strcmp(p, "date") != 0)
    {
        LOG_PRINT(error_e, "Invalid date field '%s' '%s'\n", line, p);
        rewind(fp);
        goto exit_function;
    }

    /* time */
    p = strtok_csv(NULL, SEPARATOR, &r);
    if (p == NULL || p[0] == '\0' || strcmp(p, "time") != 0)
    {
        LOG_PRINT(error_e, "Invalid time field '%s' '%s'\n", line, p);
        rewind(fp);
        goto exit_function;
    }

    // tags
    filter.clear();
    while ((p = strtok_csv(NULL, SEPARATOR, &r)) != NULL)
    {
        bool found = false;
        for (int i = 0; i < PEN_NB; i++)
        {
            if (! pens[i].visible)
            {
                continue;
            }
            if (strcmp(pens[i].tag, p) == 0)
            {
                filter.append(i);
                found = true;
                break;
            }
        }
        if (! found)
            filter.append(-1);
    }

    if (updating)
    {
        fsetpos(fp, &lastLogFilePos);

        // remove the nan
        for (int j = 0; j < PEN_NB; ++j)
        {
            if (pens[j].sample > 0 && isnan(pens[j].y[pens[j].sample - 1]))
            {
                --pens[j].sample;
            }
        }
    }
    else
    {
        fgetpos(fp, &lastLogFilePos);
    }

    /* extract data */
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        struct tm tfile;
        time_t t;

        ++row;

        /*date*/
        p = strtok_csv(line, SEPARATOR, &r);
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Cannot get date token\n");
            break;
        }
        sprintf(buf, "%s_", p);

        /*time*/
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL)
        {
            LOG_PRINT(error_e, "Cannot get time token\n");
            break;
        }
        strcat(buf, p);

        // check datetime
        strptime(buf, "%Y/%m/%d_%H:%M:%S", &tfile);
        tfile.tm_isdst = 0;
        t = mktime(&tfile);
        lastLog_time = t;

        // update screen counter
        if (row % 750 == 0)
        {
            ui->labelDate->setText(p); // HH:MM:SS
            ui->labelDate->setStyleSheet("color: rgb(0,0,255);");
            ui->labelDate->repaint();
            QCoreApplication::processEvents();
        }

        // skip past and future samples
        if ( t < ti )
        {
            /* too early */
            continue;
        }
        if ( tf < t )
        {
            /* too late */
            break;
        }

        // adding sample, skipping samples for large time intervals (must be separately checked for each pen)
        for (int i = 0; i < filter.count() && (p = strtok_csv(NULL, SEPARATOR, &r)) != NULL; ++i)
        {
            int j = filter.at(i);
            if (j < 0 || (p[0] == '-' && p[1] == 0))
            {                
                continue;
            }
            // skipping if last sample is too near
            if (pens[j].sample > 0) {
                double x = (t - t0);
                double delta_t = x - pens[j].x[pens[j].sample - 1];

                if (delta_t < skip) {
                    continue;
                }
            }
            // make space, losing the older one
            if (pens[j].sample == max_sample_nb) {

                //fprintf(stderr, "trend: %s#%d %s pen[%d] making space\n", (filename ? filename : "(null)"), row, buf, j);
                memcpy(&pens[j].x[0], &pens[j].x[1], ((max_sample_nb - 1) * sizeof(double)));
                memcpy(&pens[j].y[0], &pens[j].y[1], ((max_sample_nb - 1) * sizeof(double)));
                pens[j].sample = max_sample_nb - 1;
            }
            // add the sample
            if (pens[j].sample < max_sample_nb) {
                char *s = NULL;
                double d;
                d = strtod(p, &s);
                if (s == p)
                {
                    pens[j].y[pens[j].sample] = NAN;
                } else {
                    pens[j].y[pens[j].sample] = d; //atof(p);
                }
                pens[j].x[pens[j].sample] = (t - t0);
                pens[j].sample++;

                ++samples[j]; // only for NANs

                //fpos_t pos;
                //fgetpos(fp, &pos);
                //fprintf(stderr, "trend: pos=%u %s pen[%d].sample=%d/%d %f\n", pos.__pos, buf, j, pens[j].sample, max_sample_nb, d);
            }
        }

    }
    retval = true;

exit_function:
    fgetpos(fp, &lastLogFilePos);
    fclose(fp);

    // for each curve, if there were samples, we add a NAN (i.e. force a discontinuity) at the end of each log
    for (int j = 0; j < PEN_NB; ++j)
    {
        if (samples[j] > 0 && pens[j].sample < max_sample_nb)
        {
            pens[j].y[pens[j].sample] = NAN;
            pens[j].x[pens[j].sample] = pens[j].x[pens[j].sample - 1]; // ok: samples[j] > 0
            pens[j].sample++;
        }
    }
    return retval;
}

void trend::incrementTime(int direction)
{
    incrementTimeDirection = direction;
}

void trend::incrementValue(int direction)
{
    incrementValueDirection = direction;
}

bool trend::loadFromFile(QDateTime Ti)
{   
    Q_UNUSED(Ti);
    // manage online status
    QDateTime Tfin;
    if(TzeroLoaded.addSecs(LoadedWindowSec) > QDateTime::currentDateTime())
    {
        Tfin = QDateTime::currentDateTime();
    }
    else
    {
        Tfin = TzeroLoaded.addSecs(LoadedWindowSec);
    }
    
    /* load data from Ti to Ti + MaxWindowsec */
    return Load(TzeroLoaded, Tfin, TrendPeriodSec);
}

#define ABS(a) (((a) > 0) ? (a) : -(a))

bool trend::showWindow(QDateTime Tmin, QDateTime Tmax, double ymin, double ymax, int pen)
{
    /* prepare the time axis tick */
    //double sinint = timeScale->getBaseTime().secsTo(Tmin.time());
    //double sfinal = timeScale->getBaseTime().secsTo(Tmax.time()
    double sinint = TzeroLoaded.secsTo(Tmin);
    double sfinal = TzeroLoaded.secsTo(Tmax);

    QList<double> arrayTimeTicks;
    
    LOG_PRINT(verbose_e, "Current '%s' Tmin '%s' Tmax '%s' ymin %f ymax %f pen %d sinint %f sfinal %f TzeroLoaded %s\n",
              QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data(),
              Tmin.toString(DATE_TIME_FMT).toAscii().data(),
              Tmax.toString(DATE_TIME_FMT).toAscii().data(),
              ymin,
              ymax,
              pen,
              sinint,
              sfinal,
              TzeroLoaded.toString().toAscii().data()
              );
    double horiz_ticks = (double)datetime_ticks(_layout_, this->width(), this->height());
    double vert_ticks = (double)values_ticks(_layout_, this->width(), this->height());
    double deltax = (sfinal - sinint) / horiz_ticks;
    for (int i = 0; i < (horiz_ticks + 1); i++)
    {
        arrayTimeTicks.append(sinint + i * deltax);
    }    
    QwtScaleDiv scale = QwtScaleDiv(sinint,sfinal);
    scale.setTicks(QwtScaleDiv::MajorTick, (arrayTimeTicks));
    d_qwtplot->setAxisScaleDiv( QwtAxisId( timeAxisId, 0 ), scale);
    d_qwtplot->setAxisVisible( QwtAxisId( timeAxisId, 0 ), true);

    /* prepare the values */
    for (int pen_index = 0; pen_index < PEN_NB; pen_index++)
    {
        if (pens[pen_index].visible)
        {
            /* calculate the initial index and the final index visible into the current window */
            int XindexIn, XindexFin;

            int xmin = TzeroLoaded.secsTo(Tmin);
            XindexIn = 0;
            while (XindexIn < pens[pen_index].sample && pens[pen_index].x[XindexIn] < xmin)
            {
                if (XindexIn < (pens[pen_index].sample - 1))
                    ++XindexIn;
                else
                    break;
            }
            if (XindexIn > 0 && pens[pen_index].x[XindexIn] > xmin)
            {
                    --XindexIn;
            }
            
            int decimal;
            switch (varNameArray[pens[pen_index].CtIndex].type)
            {
            case uintab_e:
            case uintba_e:
            case intab_e:
            case intba_e:
            case udint_abcd_e:
            case udint_badc_e:
            case udint_cdab_e:
            case udint_dcba_e:
            case dint_abcd_e:
            case dint_badc_e:
            case dint_cdab_e:
            case dint_dcba_e:
                decimal = getVarDecimalByCtIndex(pens[pen_index].CtIndex);
                break;

            case fabcd_e:
            case fbadc_e:
            case fcdab_e:
            case fdcba_e:
                decimal = getVarDecimalByCtIndex(pens[pen_index].CtIndex); // may change
                break;

            case byte_e:
                decimal = getVarDecimalByCtIndex(pens[pen_index].CtIndex);
                break;

            case bit_e:
            case bytebit_e:
            case wordbit_e:
            case dwordbit_e:
                decimal = 2; // scale (0, 1) --> labels(0.00 0.25 0.50 0.75 1.00)
                break;

            default:
                decimal = 0;
            }
            
            d_qwtplot->setAxisScaleDraw(QwtAxisId( valueAxisId, pen_index ), new NormalScaleDraw(decimal));
            /* prepare the value axis tick */
            QList<double> arrayTicks;
            double deltay = (pens[pen_index].yMaxActual - pens[pen_index].yMinActual) / vert_ticks;
            for (int i = 0; i < (vert_ticks + 1); i++)
            {
                arrayTicks.append(pens[pen_index].yMinActual + i * deltay);
            }
            QwtScaleDiv scale = QwtScaleDiv(pens[pen_index].yMinActual,pens[pen_index].yMaxActual);
            scale.setTicks(QwtScaleDiv::MajorTick, (arrayTicks));
            d_qwtplot->setAxisAutoScale(QwtAxisId( valueAxisId, pen_index ), false);
            d_qwtplot->setAxisScaleDiv( QwtAxisId( valueAxisId, pen_index ), scale);
            // we do not want many Y scales: d_qwtplot->setAxisVisible( QwtAxisId( valueAxisId, pen_index ), true );
            
            /* if online, the last sample is pens[pen_index].sample */
            if (_online_)
            {
                XindexFin = pens[pen_index].sample;
            }
            /* if offline, looking for the last sample */
            else
            {
                int xmax = TzeroLoaded.secsTo(Tmax);
                XindexFin = pens[pen_index].sample - 1;
                while (XindexFin > XindexIn && pens[pen_index].x[XindexFin] > xmax)
                {
                    if (XindexFin > (XindexIn + 1))
                        --XindexFin;
                    else
                        break;
                }
                if (XindexFin < (pens[pen_index].sample - 1) && pens[pen_index].x[XindexFin] < xmax)
                {
                    ++XindexFin;
                }
            }
            
            if (pens[pen_index].visible)
            {
                if (XindexIn < XindexFin)
                {
                    /* set the y scale in according with the new window and the new scale of the activa pen */
                    if (ymin != ymax)
                    {
                        LOG_PRINT(verbose_e, "NEW RANGE pen %d %f %f from %f %f\n", pen_index, ymin, ymax, pens[pen_index].yMin, pens[pen_index].yMax);
                        pens[pen_index].yMinActual =
                                pens[pen_index].yMin
                                +
                                (pens[pen_index].yMax - pens[pen_index].yMin)
                                /
                                (pens[pen].yMax - pens[pen].yMin)
                                +
                                (ymin - pens[pen].yMin);
                        pens[pen_index].yMaxActual =
                                pens[pen_index].yMin
                                +
                                (pens[pen_index].yMax - pens[pen_index].yMin)
                                /
                                (pens[pen].yMax - pens[pen].yMin)
                                +
                                (ymax - pens[pen].yMin);
                        LOG_PRINT(verbose_e, "NEW RANGE pen %d %f %f from %f %f to %f %f\n", pen_index, ymin, ymax, pens[pen_index].yMin, pens[pen_index].yMax, pens[pen_index].yMinActual, pens[pen_index].yMaxActual );
                    }

                    // set the new samples
                    if (pens[pen_index].curve != NULL) {
                        if (_layout_ == PORTRAIT)
                        {
                            pens[pen_index].curve->setRawSamples(&(pens[pen_index].y[XindexIn]), &(pens[pen_index].x[XindexIn]), XindexFin - XindexIn);
                        }
                        else
                        {
                            pens[pen_index].curve->setRawSamples(&(pens[pen_index].x[XindexIn]), &(pens[pen_index].y[XindexIn]), XindexFin - XindexIn);
                        }
                        pens[pen_index].curve->attach(d_qwtplot);
                    }

                    // update value (QLabel)
                    if (pen_index == pen && this->isVisible())
                    {
                        double v = NAN;
                        for (int i = XindexFin - 1; i > 0; --i)
                        {
                            if (! isnan(pens[pen].y[i]))
                            {
                                v = pens[pen].y[i];
                                break;
                            }
                        }
                        ui->labelDate->setText(QString(" %1 ").arg(v, 7, 'f', decimal));
                        ui->labelDate->setStyleSheet("color: rgb(0,0,0);");
                        ui->labelDate->repaint();
                    }

                    LOG_PRINT(verbose_e, "NEW SAMPLE LAST '%s'  %f %f in %d fin %d sample %d\n", pens[pen_index].tag, pens[pen_index].x[XindexFin-1], pens[pen_index].y[XindexFin-1], XindexIn, XindexFin, pens[pen_index].sample);
                }
            }
        }
    }
    
    /* update the plot */
    LOG_PRINT(verbose_e,"before replot\n");
    d_qwtplot->updateAxes();
    d_qwtplot->replot();
    d_qwtplot->repaint();
    LOG_PRINT(verbose_e, "UPDATE AXES AND REPLOT!\n");
       
    return true;
}

bool trend::loadWindow(QDateTime Tmin, QDateTime Tmax, double ymin, double ymax, int pen)
{
    if (_load_window_busy || ! this->isVisible())
    {
        LOG_PRINT(verbose_e, "BUSY\n");
        return false;
    }

    _load_window_busy = true;

    TrendPeriodSec = LoadedWindowSec / max_sample_nb;
    if (TrendPeriodSec < LogPeriodSecF)
        TrendPeriodSec = LogPeriodSecF;

    // (1/3) out of bounds, so we need to load all data from file?
    if ( _trend_data_reload_
       || actualTzero < TzeroLoaded
       || TzeroLoaded.addSecs(LoadedWindowSec) < actualTzero
       || sample_to_skip != TrendPeriodSec )
    {
        showStatus(trUtf8("Loading..."), false);
        // clear whole plot area
        for (int i = 0; i < PEN_NB; i++)
        {
            pens[i].sample = 0;
            if (pens[i].visible && pens[i].curve != NULL)
            {
                pens[i].curve->detach();
            }
        }

        // new buffer limits
#if 1
        TzeroLoaded = actualTzero;
#else
        if (_layout_ == LANDSCAPE)
            TzeroLoaded = actualTzero.addSecs(-(int)((LoadedWindowSec - actualVisibleWindowSec) / 2));
        else
            TzeroLoaded = actualTzero.addSecs( - actualVisibleWindowSec -(int)((LoadedWindowSec - actualVisibleWindowSec) / 2));
#endif
        VisibleWindowSec = actualVisibleWindowSec;
        LoadedWindowSec = OVERLOAD_SECONDS(VisibleWindowSec);
        sample_to_skip = TrendPeriodSec;

        theDateTimeScaleDraw = new DateTimeScaleDraw(TzeroLoaded); // QDateTime::currentDateTime());
        d_qwtplot->setAxisScaleDraw(timeAxisId, theDateTimeScaleDraw);
        for (int i = 0; i < PEN_NB; i++)
        {
            int decimal = 0;
            if (pens[i].visible)
            {
                decimal =  getVarDecimalByName(pens[i].tag);
            }
            d_qwtplot->setAxisScaleDraw(QwtAxisId( valueAxisId, i ), new NormalScaleDraw(decimal));
        }

        lastLogFileName.clear();
        loadFromFile(TzeroLoaded);
    }

    // (2/3) crossing bounds, so we need to shift and reload some new data from file?
    else if (TzeroLoaded.addSecs(LoadedWindowSec) < actualTzero.addSecs(actualVisibleWindowSec))
    {
        showStatus(trUtf8("Shift..."), false);
        // VisibleWindowSec = actualVisibleWindowSec;

        // shift samples
        int deltaSec = TzeroLoaded.secsTo(actualTzero);
        TzeroLoaded = actualTzero;

        // clear some of the plot area, reuse as much as possible
        for (int i = 0; i < PEN_NB; i++)
        {
            int j = 0;

            if (! pens[i].visible)
            {
                pens[i].sample = 0;
            }
            // find the first sample in the new time range
            while (j < pens[i].sample && pens[i].x[j] < deltaSec)
            {
                ++j;
            }
            if (j < pens[i].sample)
            {
                // shift left the samples (position and time)
                memcpy(&pens[i].x[0], &pens[i].x[j], ((pens[i].sample - j) * sizeof(double)));
                memcpy(&pens[i].y[0], &pens[i].y[j], ((pens[i].sample - j) * sizeof(double)));
                // and reduce the size
                pens[i].sample = (pens[i].sample - j);
            }
            else
            {
                // sorry, nothing to reuse
                pens[i].sample = 0;
            }

            // rebase all on new T0
            for (int k = 0; k < pens[i].sample; ++k)
            {
                pens[i].x[k] = pens[i].x[k] - deltaSec;
            }
        }

        loadFromFile(TzeroLoaded);
    }

    // (3/3) online, so just watch for new samples?
    else if (_online_)
    {
        // showStatus(trUtf8("Updating..."), false);

        loadFromFile(TzeroLoaded);
    }

    // anyhow redraw
    theDateTimeScaleDraw->setBaseDateTime(TzeroLoaded);
    bool ret_val = showWindow(Tmin, Tmax, ymin, ymax, pen);

    /* enable up and down arrow in according with the available upper or lower point */
    if (pens[actualPen].yMax > pens[actualPen].yMaxActual)
    {
        if (_layout_ == LANDSCAPE) popup->enableButtonUp(true);     else popup->enableButtonRight(true);
    } else {
        if (_layout_ == LANDSCAPE) popup->enableButtonUp(false);    else popup->enableButtonRight(false);
    }

    if (pens[actualPen].yMin < pens[actualPen].yMinActual)
    {
        if (_layout_ == LANDSCAPE) popup->enableButtonDown(true);   else popup->enableButtonLeft(true);
    } else {
        if (_layout_ == LANDSCAPE) popup->enableButtonDown(false);  else popup->enableButtonLeft(false);
    }
    
    if (! _online_) // actualTzero < QDateTime::currentDateTime())
    {
        if (_layout_ == LANDSCAPE) popup->enableButtonRight(true);  else popup->enableButtonUp(true);
    } else {
        if (_layout_ == LANDSCAPE) popup->enableButtonRight(false); else popup->enableButtonUp(false);
    }
    
    if (_layout_ == LANDSCAPE) { popup->enableButtonLeft(true); }   else { popup->enableButtonDown(true); }
    
    _load_window_busy = false;
    return ret_val;
}

void trend::moved(const QPoint &pos)
{
    int decimal =  getVarDecimalByName(pens[actualPen].tag);
    QString datetime, y;
    if (_layout_ == PORTRAIT)
    {
        y = QString::number(d_qwtplot->invTransform(QwtAxisId( valueAxisId, actualPen ), pos.x()), 'f', decimal);
        datetime = TzeroLoaded.addSecs((int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pos.y()))).toString("yyyy-MM-dd HH:mm:ss");
    }
    else
    {
        datetime = TzeroLoaded.addSecs((int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pos.x()))).toString("yyyy-MM-dd HH:mm:ss");
        y = QString::number(d_qwtplot->invTransform(QwtAxisId( valueAxisId, actualPen ), pos.y()), 'f', decimal);
    }
    
    ui->labelvalue->setText(y + " @ " + datetime); // for watching the value on narrow displays
    ui->labelvalue->setStyleSheet(QString("border: 2px solid #%1;" "font: 14pt \"DejaVu Sans Mono\";").arg(pens[actualPen].color));
    ui->labelPen->setVisible(false);
    ui->labelvalue->setVisible(true);
}

void trend::selected(const QPolygon &pol)
{
    if (_zoom)
    {
        zoomRect = pol.boundingRect();
        do_zoom = true;
    }
    ui->labelvalue->setText("");
    ui->labelvalue->setStyleSheet("");
    ui->labelvalue->setVisible(false);
    ui->labelPen->setVisible(true);
}

void InterruptedCurve::drawCurve( QPainter *painter, __attribute__((unused))int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to ) const
{
    // marker of line
    if (!isnan(data()->sample(from).x()) && !isnan(data()->sample(from).y()))
    {
        drawDots(painter, xMap, yMap, canvasRect, from, from);
    }

    // draw lines with gaps
    for (int i = from + 1; i <= to; i++)
    {        
        if (!isnan(data()->sample(i).x()) && !isnan(data()->sample(i).y()))
        {
            if (!isnan(data()->sample(i - 1).x()) && !isnan(data()->sample(i - 1).y()))
            {
                drawLines(painter, xMap, yMap, canvasRect, (i - 1), i);
            }
            else
            {
                drawDots(painter, xMap, yMap, canvasRect, i, i);
            }

        }
    }
}

void trend::loadOrientedWindow()
{
#if 0
    LOG_PRINT(verbose_e,"loadWindow\n");
    if (_layout_ == PORTRAIT)
    {
        LOG_PRINT(verbose_e, "Current '%s' Tmin '%s' Tmax '%s' VisibleWindowSec %d\n",
                  QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data(),
                  actualTzero.addSecs(actualVisibleWindowSec).toString(DATE_TIME_FMT).toAscii().data(),
                  actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                  actualVisibleWindowSec
                  );
        loadWindow(
                    actualTzero.addSecs(-actualVisibleWindowSec),
                    actualTzero
                    );
    }
    else
    {
        LOG_PRINT(verbose_e, "Current '%s' Tmin '%s' Tmax '%s' VisibleWindowSec %d\n",
                  QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data(),
                  actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                  actualTzero.addSecs(actualVisibleWindowSec).toString(DATE_TIME_FMT).toAscii().data(),
                  actualVisibleWindowSec
                  );
        loadWindow(
                    actualTzero,
                    actualTzero.addSecs(actualVisibleWindowSec)
                    );
    }
#else
    loadWindow(
                actualTzero,
                actualTzero.addSecs(actualVisibleWindowSec)
                );
#endif
}


void trend::on_pushButtonOnline_clicked()
{
    setOnline(!(getOnline()));
}

void trend::showStatus(QString message, bool iserror)
{
    ui->labelDate->setText(message);
    if (iserror)
    {
        ui->labelDate->setStyleSheet("color: rgb(255,0,0);");
        ui->pushButtonOnline->setStyleSheet("QPushButton"
                                            "{"
                                            "border-image: url(:/libicons/img/Warning.png);"
                                            "qproperty-focusPolicy: NoFocus;"
                                            "}");
        ui->labelDate->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
    else
    {
        ui->labelDate->setStyleSheet("");
        ui->pushButtonOnline->setStyleSheet("QPushButton"
                                            "{"
                                            "border-image: url(:/libicons/img/Hourglass.png);"
                                            "qproperty-focusPolicy: NoFocus;"
                                            "}");
        ui->labelDate->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    ui->labelPen->setVisible(!iserror);
    ui->labelDate->repaint();
}

void trend::on_pushButtonHome_clicked()
{
    _trend_data_reload_ = true;
    LOG_PRINT(verbose_e, "_trend_data_reload_ %d\n",  _trend_data_reload_);
    disableUpdate();
    go_home();
}

void trend::on_pushButtonBack_clicked()
{
    _trend_data_reload_ = true;
    LOG_PRINT(verbose_e, "_trend_data_reload_ %d\n",  _trend_data_reload_);
    disableUpdate();
    go_back();
}

void trend::on_pushButtonTime_clicked()
{
    setRange();
}

void trend::on_pushButtonPan_clicked()
{
    setPan();
}

void trend::updatePenLabel()
{
    /* update the pushButtonPen text with the current pen description */
    if (pens[actualPen].visible && strlen(pens[actualPen].tag) != 0)
    {
#ifdef SHOW_ACTUAL_VALUE
        int decimal = getVarDecimalByCtIndex(pens[actualPen].CtIndex);
#endif
        if (strlen(pens[actualPen].description) > 0)
        {
            ui->labelPen->setText(
                        pens[actualPen].description
            #ifdef SHOW_ACTUAL_VALUE
                        +
                        QString(" [x:%1; y:%2]")
                        .arg(TzeroLoaded.addSecs(pens[actualPen].x[pens[actualPen].sample - 1]).toString("HH:mm:ss"))
                    .arg(QString::number(pens[actualPen].y[pens[actualPen].sample - 1], 'f', decimal))
        #endif
                    );
        }
        else
        {
            ui->labelPen->setText(
                        pens[actualPen].tag
            #ifdef SHOW_ACTUAL_VALUE
                        +
                        QString(" [x:%1; y:%2]")
                        .arg(TzeroLoaded.addSecs(pens[actualPen].x[pens[actualPen].sample - 1]).toString("HH:mm:ss"))
                    .arg(QString::number(pens[actualPen].y[pens[actualPen].sample - 1], 'f', decimal))
        #endif
                    );
        }
    }
    else if (strlen(pens[actualPen].tag) != 0)
    {
        if (strlen(pens[actualPen].description) > 0)
        {
            ui->labelPen->setText(trUtf8("Trace '%1' is not visible").arg(pens[actualPen].description));
        }
        else
        {
            ui->labelPen->setText(trUtf8("Trace '%1' is not visible").arg(pens[actualPen].tag));
        }
    }
    else
    {
        if (strlen(pens[actualPen].description) > 0)
        {
            ui->labelPen->setText(trUtf8("Trace '%1' is empty").arg(pens[actualPen].description));
        }
        else
        {
            ui->labelPen->setText(trUtf8("Trace '%1' is empty").arg(actualPen + 1));
        }
    }
    return;
}
