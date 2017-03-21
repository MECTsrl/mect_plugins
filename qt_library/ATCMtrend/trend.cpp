/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
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

#include <time.h>

#undef STATIC_AXES

#define HORIZ_TICKS 5
#define VERT_TICKS  4

#define INCREMENT 1.0
#define OVERLOAD_SECONDS(Visible) (Visible * 20 / 10) // 800 + 100% = 1600
#define SHIFT_FACTOR 10
#define AXIS_STEP_DIVISOR 2
#define DELTA_TIME_FACTOR (AXIS_STEP_DIVISOR * 2)
#define DELTA_VALUE_FACTOR 4

#define DATE_TIME_FMT "yyyy/MM/dd HH:mm:ss"

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

static bool do_refresh_plot;
static bool do_pan;
static int incrementTimeDirection = 0;
static int incrementValueDirection = 0;

static QRect zoomRect;
static bool do_zoom;

/**
 * @brief This is the constructor. The operation written here, are executed only one time: at the instanziation of the page.
 */
trend::trend(QWidget *parent) :
    page(parent),
    ui(new Ui::trend)
{
    ui->setupUi(this);
    /* set here the protection level (pwd_admin_e, pwd_super_user_e, pwd_user_e, pwd_operator_e), default is pwd_operator_e
     * protection_level = pwd_operator_e;
     */
    //setStyle::set(this);

    /* initialization */
    for (int i = 0; i < PEN_NB; i++)
    {
        pens[i].x = new double [MAX_SAMPLE_NB + 1];
        pens[i].y = new double [MAX_SAMPLE_NB + 1];
        pens[i].curve = NULL;
    }
    
    actualPen = 0;
    
    d_qwtplot = NULL;
    d_picker = NULL;
    valueAxisId = 0;
#ifdef MARKER
    d_marker = NULL;
#endif
    timeScale  = NULL;
#ifdef STATIC_AXES
#ifdef VALUE_TIME_SCALE
    for (int i = 0; i < PEN_NB; i++)
    {
        valueScale[i] = NULL;
    }
#endif
#endif

    d_qwtplot = ui->qwtPlot;
    d_qwtplot->hide();

    //    d_qwtplot->plotLayout()->setAlignCanvasToScales(true);
    //    d_qwtplot->plotLayout()->setCanvasMargin 	( 40, -1 );
    d_qwtplot->setAxesCount( QwtPlot::xBottom, 1 );
    d_qwtplot->setAxesCount( QwtPlot::xTop, PEN_NB );
    d_qwtplot->setAxesCount( QwtPlot::yLeft, PEN_NB );

    /* set the trend stylesheet */
    SET_TREND();
    
    /* setup the picker in order to capture a rect */
    d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                 QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                 d_qwtplot->canvas());
    d_picker->setStateMachine(new QwtPickerDragRectMachine());
    d_picker->setRubberBand(QwtPicker::CrossRubberBand);
    d_picker->setRubberBandPen(QColor(0,0,0));
    d_picker->setTrackerPen(QColor(255,255,255));
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
    overloadActualTzero = false;

    popup = new trend_other(this);
    popup_visible = false;
    popup->hide();
    actualPen = 0;

    do_refresh_plot = false;
    do_pan = false;
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
    LOG_PRINT(verbose_e, " RELOADING\n");
    reloading = true;
}

/**
 * @brief This is the updateData member. The operation written here, are executed every REFRESH_MS milliseconds.
 */
void trend::updateData()
{
    if (this->isVisible() == false)
    {
        return;
    }

    /* call the father update data  */
    page::updateData();

    if (reloading)
    {
        LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
        disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));

        d_qwtplot->hide();

        /* disable all button during loading */
        popup->enableButtonUp(false);
        popup->enableButtonDown(false);
        popup->enableButtonLeft(false);
        popup->enableButtonRight(false);
        ui->pushButtonPen->setText("");
        /* set the label as loading */
        showStatus(trUtf8("Loading..."), false);

        force_back = false;

        /* actualVisibleWindowSec is not null, skip the initialization */
        if (_trend_data_reload_)
        {
            if (strlen(_actual_trend_) == 0)
            {
                disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
                force_back = true;
                reloading = false;
                return;
            }

            LOG_PRINT(verbose_e, "_trend_data_reload_\n");

            d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotCurve);
            d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotScale);
            d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotUserItem);

            /* load the actual trend info */
            if (LoadTrend(_actual_trend_, &errormsg) == false)
            {
                force_back = true;
                reloading = false;
                return;
            }

            for (int z = 0; z < PEN_NB; z++)
            {
#ifdef STATIC_AXES
                if (valueScale[z] == NULL)
                {
                    int decimal = 0;
                    if (strlen(pens[z].tag))
                    {
                        decimal = getVarDecimalByName(pens[z].tag);
                    }
                    LOG_PRINT(verbose_e, " valueScale null pointer for z='%d'\n", z);
                    valueScale[z] = new NormalScaleDraw(decimal);
                }
#endif
                pens[z].curve = new InterruptedCurve();
                //pens[rownb].curve = new QwtPlotCurve();
                for (int i = 0; i < MAX_SAMPLE_NB; i++)
                {
                    pens[z].y[i] = /*pens[rownb].yMax*/NAN;
                    pens[z].x[i] = /*pens[rownb].yMax*/NAN;
                }
                pens[z].sample = 0;
            }
    #ifdef MARKER
            if (d_marker != NULL)
            {
                delete d_marker;
            }
            d_marker = new QwtPlotMarker();
            d_marker->setValue(0.0, 0.0);
            d_marker->setLineStyle(QwtPlotMarker::VLine);
            //d_marker->setLinePen(QPen(QColor(QString("#%1").arg(pens[actualPen].color)), 0, Qt::DashDotLine));
            d_marker->setLinePen(QPen(Qt::green, 0, Qt::DashDotLine));
            d_marker->attach(d_qwtplot);
    #endif
        }

        d_qwtplot->show();

        LOG_PRINT(verbose_e, "Calling setOnline  _trend_data_reload_ %d\n",  _trend_data_reload_);
        enableZoomMode(false);

        /* set the Tzero and TzeroLoaded */
        Tzero = QDateTime::currentDateTime().addSecs(-LogPeriodSec);
        VisibleWindowSec = MaxWindowSec;

        /* at first time we are in online mode, so the window time start at the same of window loaded time */
        if (actualVisibleWindowSec == 0)
        {
            actualTzero = Tzero;
            actualVisibleWindowSec = VisibleWindowSec;
            setOnline(true);
            LOG_PRINT(verbose_e, "initialize actualTzero '%s' and actualVisibleWindowSec %d\n", actualTzero.toString(DATE_TIME_FMT).toAscii().data(), actualVisibleWindowSec);
            overloadActualTzero = true;
        }
        else
        {
            LOG_PRINT(verbose_e, "initialize actualTzero '%s' and actualVisibleWindowSec %d\n", actualTzero.toString(DATE_TIME_FMT).toAscii().data(), actualVisibleWindowSec);
            VisibleWindowSec = actualVisibleWindowSec;
            // setOnline(false); /* maintain the last status */
            overloadActualTzero = false;
        }

        LoadedWindowSec = OVERLOAD_SECONDS(VisibleWindowSec);

        /* calculate the TrendPeriodSec */
        /* if it is less than LogPeriodSec, set at LogPeriodSec */
        LogPeriodSec = ((LogPeriodSecF < LogPeriodSecS) ? LogPeriodSecF : LogPeriodSecS);
        TrendPeriodSec = (int)(LoadedWindowSec / MAX_SAMPLE_NB);
        TrendPeriodSec = (TrendPeriodSec < LogPeriodSec) ? LogPeriodSec : TrendPeriodSec;
        /* set TzeroLoaded to force the data load */
        TzeroLoaded = Tzero.addSecs(-1);
        _trend_data_reload_ = false;

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

            LOG_PRINT(verbose_e, "pen %d yMin %f yMax %f tmin %d tmax %d\n", i, pens[i].yMin, pens[i].yMax, 0, MaxWindowSec);
        }

    #ifdef STATIC_AXES
        if (timeScale == NULL)
        {
            timeScale = new TimeScaleDraw(TzeroLoaded.time());
            d_qwtplot->setAxisScaleDraw(QwtAxisId( timeAxisId, 0 ), timeScale);
        }
        else
        {
            timeScale->setBaseTime(TzeroLoaded.time());
        }
        d_qwtplot->setAxisVisible( QwtAxisId( timeAxisId, 0 ), false);
        LOG_PRINT(verbose_e, "####### Setting time axis\n");
    #else
        d_qwtplot->setAxisScaleDraw(timeAxisId, new DateTimeScaleDraw(TzeroLoaded));
    #endif

        first_time = true;
        reloading = false;
        LOG_PRINT(verbose_e, " RELOADED\n");
    }

    if (force_back)
    {
        /* select a new trend from CUSTOM_TREND_DIR directory */
        item_selector * sel;
        QString value;
        QStringList trendList;
        QDir trendDir(CUSTOM_TREND_DIR, "*.csv");

        force_back = false;

        trendList = trendDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
        if (trendList.count() > 0)
        {
            sel = new item_selector(trendList, &value, trUtf8("TREND SELECTOR"));
            sel->showFullScreen();

            if (sel->exec() == QDialog::Accepted)
            {
                strcpy(_actual_trend_, QFileInfo(value).baseName().toAscii().data());
                if (LoadTrend(_actual_trend_, &errormsg) == false)
                {
                    refresh_timer->stop();
                    errormsg = trUtf8("The trend description (%1) is not valid. %2").arg(_actual_trend_).arg(errormsg);
                    showStatus(errormsg, true);
                    errormsg.clear();
                    force_back = true;
                    delete sel;
                    return;
                }
                else
                {
                    delete sel;
                    for (int z = 0; z < PEN_NB; z++)
                    {
#ifdef STATIC_AXES
                        if (valueScale[z] == NULL)
                        {
                            int decimal = 0;
                            if (strlen(pens[z].tag))
                            {
                                decimal = getVarDecimalByName(pens[z].tag);
                            }
                            LOG_PRINT(verbose_e, " valueScale null pointer for z='%d'\n", z);
                            valueScale[z] = new NormalScaleDraw(decimal);
                        }
#endif
                        pens[z].curve = new InterruptedCurve();
                        //pens[rownb].curve = new QwtPlotCurve();
                        for (int i = 0; i < MAX_SAMPLE_NB; i++)
                        {
                            pens[z].y[i] = /*pens[rownb].yMax*/NAN;
                            pens[z].x[i] = /*pens[rownb].yMax*/NAN;
                        }
                        pens[z].sample = 0;
                    }
                    reloading = true;
                    refresh_timer->start(REFRESH_MS);
                    return;
                }
            }
            else
            {
                delete sel;
                errormsg = trUtf8("No trend selected");
                go_back();
            }
        }
        else
        {
            errormsg = trUtf8("No trend to show");
        }

        actualVisibleWindowSec = 0;
        showStatus(errormsg, true);
        errormsg.clear();
        //go_back();
        return;
    }

    if (first_time)
    {
#if 0
        d_qwtplot->setGeometry(ui->frame->rect());
        if (_layout_ == PORTRAIT)
        {
            ui->pushButtonSelect->setGeometry(0,0,ui->frame->width(), 30);
            LOG_PRINT(verbose_e, "GEOMETRY: 0,0,%d,30\n", ui->frame->width());
            ui->labelvalue->setGeometry(
                        ui->frame->width() - ui->labelvalue->width(),
                        ui->frame->height() - ui->labelvalue->height(),
                        ui->labelvalue->width(),
                        ui->labelvalue->height()
                        );
        }
        else
        {
            ui->pushButtonSelect->setGeometry(0,0,80, ui->frame->height());
            LOG_PRINT(verbose_e, "GEOMETRY: 0,0,80,%d\n", ui->frame->height());
            ui->labelvalue->setGeometry(
                        ui->frame->width() - ui->labelvalue->width(),
                        ui->frame->height() + ui->pushButtonSelect->height(),
                        ui->labelvalue->width(),
                        ui->labelvalue->height()
                        );
        }
#endif
        first_time = false;
        /* connecting new_trend signal */
        LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
        disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
        
        /* update the actual window parameters */

        LOG_PRINT(verbose_e, "UPDATE actualVisibleWindowSec\n");
        if (overloadActualTzero)
        {
            overloadActualTzero = false;
            actualTzero = Tzero;
            actualVisibleWindowSec = VisibleWindowSec;
        }
        
        actualPen = -1;
        on_pushButtonSelect_clicked();

        do_refresh_plot = true;
        incrementTimeDirection = 0;
        incrementValueDirection = 0;

        LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
        disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
        LOG_PRINT(verbose_e, "CONNECT refreshEvent\n");
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
        setOnline(false);

        int increment = ((actualVisibleWindowSec / DELTA_TIME_FACTOR) < LogPeriodSec) ? LogPeriodSec : (actualVisibleWindowSec / DELTA_TIME_FACTOR);

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
            float delta = (pens[i].yMaxActual - pens[i].yMinActual) / DELTA_VALUE_FACTOR;
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
        else
        {
            LOG_PRINT(verbose_e, "ZOOOOOOOOM NUBER OF SAMPLE %d\n", (myXfin - myXin) / TrendPeriodSec);
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
        actualVisibleWindowSec = myXfin - myXin;
        LOG_PRINT(verbose_e, "UPDATE actualVisibleWindowSec\n");
        if (_layout_ == PORTRAIT)
        {
            LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), TzeroLoaded.addSecs(myXfin).toString().toAscii().data());
            actualTzero = TzeroLoaded.addSecs(myXfin);
        }
        else
        {
            LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), TzeroLoaded.addSecs(myXin).toString().toAscii().data());
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

        /*actualTzero = TzeroLoaded = Tzero = QDateTime::currentDateTime()*/;
        LOG_PRINT(verbose_e, "UPDATE actualVisibleWindowSec\n");
        actualVisibleWindowSec = VisibleWindowSec;

        /* set TzeroLoaded in the future to force the data load */
        TzeroLoaded = actualTzero.addSecs(TrendPeriodSec * 2);

        // Calling setOnline
        //setOnline(true);

        do_refresh_plot = true;
        do_pan = false;
    }

    if (do_refresh_plot)
    {
        /* update the graph only if the status is online or if the new sample are visible */
        do_refresh_plot = false;

        if (_online_)
        {
            /* online mode: if necessary center the actual data */
            QDateTime now =  QDateTime::currentDateTime();
            if (_layout_ == PORTRAIT)
            {
                actualTzero = now;
            }
            else if (_layout_ == LANDSCAPE)
            {
                actualTzero = now.addSecs(-actualVisibleWindowSec);
            }
        }

        // refresh the plot
        loadOrientedWindow();
    }

    if (getOnline()){
        static int counter = 0;
        if (counter % 4 == 0)
        {
            ui->pushButtonOnline->setStyleSheet("QPushButton"
                                                "{"
                                                "border-image: url(:/libicons/img/Chess1.png);"
                                                "qproperty-focusPolicy: NoFocus;"
                                                "}");
        }
        else if (counter % 4 == 2)
        {
            ui->pushButtonOnline->setStyleSheet("QPushButton"
                                                "{"
                                                "border-image: url(:/libicons/img/Chess2.png);"
                                                "qproperty-focusPolicy: NoFocus;"
                                                "}");
        }
        ++counter;
    }
    else
    {
        ui->pushButtonOnline->setStyleSheet("QPushButton"
                                            "{"
                                            "border-image: url(:/libicons/img/Chess.png);"
                                            "qproperty-focusPolicy: NoFocus;"
                                            "}");
    }
    ui->pushButtonOnline->repaint();
}

#ifdef TRANSLATION
/**
 * @brief This is the event slot to detect new language translation.
 */
void trend::changeEvent(QEvent * event)
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
trend::~trend()
{
    delete ui;
    for (int i = 0; i < PEN_NB; i++)
    {
        if (pens[i].x != NULL)
            delete pens[i].x;
        if (pens[i].y != NULL)
            delete pens[i].y;
        pens[i].x = NULL;
        pens[i].y = NULL;
        pens[i].curve = NULL;
    }
}

void trend::refreshEvent(trend_msg_t item_trend)
{   
    if (reloading || _trend_data_reload_ || first_time || force_back || _load_window_busy) {
        return;
    }

    if (item_trend.timestamp.isValid() == false)
    {
        LOG_PRINT(error_e,"invalid sample '%s'\n",  varNameArray[item_trend.CtIndex].tag);
        return;
    }
    
    LOG_PRINT(verbose_e,"NEW SAMPLE '%s' -> '%f' time: '%s'\n",  varNameArray[item_trend.CtIndex].tag, item_trend.value,  item_trend.timestamp.toString(DATE_TIME_FMT).toAscii().data());
    /* set the new sample to the actual array */
    for (int i = 0; i < PEN_NB; i++)
    {
        if (pens[i].curve == NULL)
        {
            /* skip empty pen */
            continue;
        }
        if (! pens[i].visible)
        {
            /* skip invisible pen */
            continue;
        }
        /* found actual pen */
        if (pens[i].CtIndex == item_trend.CtIndex)
        {            
            /* update samples only if the status is online or if the new sample are visible */
            if ( _online_
              || item_trend.timestamp < actualTzero.addSecs(actualVisibleWindowSec)
              || pens[actualPen].x[pens[actualPen].sample] < actualVisibleWindowSec
               )
            {
                if (pens[i].sample >= MAX_SAMPLE_NB)
                {
                    if (_online_)
                    {
                        /* the buffer is full, shift all values of all pens, but each pen has a different period */
                        double first_good = pens[i].x[MAX_SAMPLE_NB/SHIFT_FACTOR];

                        /* maintain only the samples that are newer than the first good */
                        for (int z = 0; z < PEN_NB; z++)
                        {
                            if (pens[z].curve != NULL)
                            {
                                int sample = 0;
                                for (int j = 0; j < MAX_SAMPLE_NB; ++j)
                                {
                                    if (pens[z].x[j] >= first_good) {
                                        pens[z].x[sample] = pens[z].x[j];
                                        pens[z].y[sample] = pens[z].y[j];
                                        ++sample;
                                    }
                                }
                                pens[z].sample = sample;
                            }
                        }
                    } else {
                        /* not enough space, we accept to lose samples */
                        LOG_PRINT(warning_e, "we are off line and the new data is out of range.");
                        break;
                    }
                }
                /* update the actual x and y arrays */
                pens[i].y[pens[i].sample] = item_trend.value;
                pens[i].x[pens[i].sample] = TzeroLoaded.secsTo(item_trend.timestamp);
                pens[i].sample++;
#ifdef SHOW_ACTUAL_VALUE
                updatePenLabel();
#endif
                /* will refresh the plot at the next updateData() */
                do_refresh_plot = true;

            }
            break;
        }
    }    
}

void trend::on_pushButtonPen_clicked()
{
    disableUpdate();
    goto_page("trend_option");
}

void trend::on_pushButtonSelect_clicked()
{
    ui->pushButtonSelect->setEnabled(false);
    
    LOG_PRINT(verbose_e, "actual pen %d\n", actualPen);
    
    /* show the next trace */
    actualPen = ((actualPen + 1) % PEN_NB);
    if (pens[actualPen].curve != NULL)
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
        ui->pushButtonPen->setVisible(true);
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
    setOnline(false);
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
    
    sprintf(fullfilename, "%s", QDate::fromString(ui->labelDate->text(), "yyyy/MM/dd").toString("yyyy_MM_dd").toAscii().data());
    
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
        LOG_PRINT(verbose_e, "detach %d\n", pen);
        pens[pen].curve->detach();
        LOG_PRINT(verbose_e, "attach %d\n", pen);
        pens[pen].curve->attach(d_qwtplot);
#ifdef MARKER
        d_marker->setLinePen(QPen(QColor(QString("#%1").arg(pens[pen].color)), 0, Qt::DashDotLine));
#endif
        LOG_PRINT(verbose_e, "done %d\n", pen);
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
    
    for (int rownb = 0; rownb < PEN_NB; rownb++)
    {
        if (pens[rownb].y != NULL)
        {
            for (int i = 0; i < MAX_SAMPLE_NB; i++)
            {
                pens[rownb].y[i] = /*pens[rownb].yMax*/NAN;
                pens[rownb].x[i] = 0;
            }
        }
        pens[rownb].sample = 0;
    }
    
    LOG_PRINT(verbose_e, "begin '%s' end '%s' skip '%d'\n", begin.toString(DATE_TIME_FMT).toAscii().data(), end.toString(DATE_TIME_FMT).toAscii().data(), skip);
    
    /* the store are in STORE_DIR and they have a name like yyyy_MM_dd_HH_mm_ss.log */
    /* get the list of the file and parse the file between time begin and time end */
    QDir logDir(STORE_DIR);
    QStringList logFileList = logDir.entryList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name);
    for (int i = 0; i < logFileList.count(); i++)
    {
        if (  i + 1 < logFileList.count()
           && QDateTime::fromString(QFileInfo(logFileList.at(i + 1)).baseName(), "yyyy_MM_dd_HH_mm_ss") <= begin)
        {
            // too early
            continue;
        }
        QDateTime logFileBegin = QDateTime::fromString(QFileInfo(logFileList.at(i)).baseName(), "yyyy_MM_dd_HH_mm_ss");
        if (end < logFileBegin)
        {
            // too late
            break;
        }
        if ( logFileBegin <= begin || logFileBegin <= end )
        {
            if (Load(QString("%1/%2").arg(STORE_DIR).arg(logFileList.at(i)).toAscii().data(), &begin, &end, skip) == false)
            {
                LOG_PRINT(warning_e, "Cannot load '%s'\n", logFileList.at(i).toAscii().data());
                return false;
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
    int count = 0;
    time_t ti = begin->toTime_t();
    time_t tf = end->toTime_t();
    char buf[42];
    int samples[PEN_NB] = {0, 0, 0, 0};
    QList<int> filter;
    FILE * fp = NULL;

    LOG_PRINT(verbose_e, "'%s', %s, %s, %d\n",
              filename,
              begin->toString("yyyy-MM-ddTHH:mm:ss").toAscii().data(),
              end->toString("yyyy-MM-ddTHH:mm:ss").toAscii().data(),
              skip
              );

    fp = fopen (filename, "r");
    if (fp == NULL)
    {
        LOG_PRINT(error_e, "no trend file '%s'\n", filename);
        return false;
    }

    LOG_PRINT(verbose_e, "FOUND LOG FILE '%s'\n", filename);
    /*
     * File format:
     * date      ;     time;   tag1 ; ...;   tagN
     * yyyy/MM/dd; HH:mm:ss; 123.456; ...; 123.456
     */

    /* extract header */
    int row = 0;
    if (fgets(line, LINE_SIZE, fp) == NULL)
    {
        LOG_PRINT(error_e, "Cannot read the header from '%s'\n", filename);
        goto exit_function;
    }

    /* date */
    p = strtok_csv(line, SEPARATOR, &r);
    if (p == NULL || p[0] == '\0' || strcmp(p, "date") != 0)
    {
        LOG_PRINT(error_e, "Invalid date field '%s' '%s'\n", line, p);
        goto exit_function;
    }

    /* time */
    p = strtok_csv(NULL, SEPARATOR, &r);
    if (p == NULL || p[0] == '\0' || strcmp(p, "time") != 0)
    {
        LOG_PRINT(error_e, "Invalid time field '%s' '%s'\n", line, p);
        goto exit_function;
    }

    // tags
    filter.clear();
    while ((p = strtok_csv(NULL, SEPARATOR, &r)) != NULL)
    {
        bool found = false;
        for (int i = 0; i < PEN_NB; i++)
        {
            if (pens[i].curve == NULL)
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

    /* extract data */
    while (fgets(line, LINE_SIZE, fp) != NULL)
    {
        struct tm tfile;
        time_t t;

        ++row;
        // skip samples for large time intervals
        if (count < skip)
        {
            LOG_PRINT(verbose_e, "Skip\n");
            count++;
            continue;
        }
        count = 0;

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

        if (row % 1000 == 0) {
            ui->labelDate->setText(QString("%1").arg(t, 5, 16));
            ui->labelDate->setStyleSheet("color: rgb(0,0,255);");
            ui->labelDate->repaint();
        }

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

        for (int i = 0; i < filter.count() && (p = strtok_csv(NULL, SEPARATOR, &r)) != NULL; ++i)
        {
            int j = filter.at(i);
            if (j < 0 || (p[0] == '-' && p[1] == 0))
            {                
                continue;
            }
            if (pens[j].sample < MAX_SAMPLE_NB) {
                pens[j].y[pens[j].sample] = atof(p);
                pens[j].x[pens[j].sample] = TzeroLoaded.secsTo(QDateTime::fromTime_t(t));
                pens[j].sample++;

                ++samples[j];
            }
        }

    }
    retval = true;

exit_function:
    fclose(fp);
    for (int j = 0; j < PEN_NB; ++j) {
        LOG_PRINT(verbose_e, "[%d] %d samples\n", j, samples[j]);
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
    LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
    disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
    
    TzeroLoaded = Ti;

    LOG_PRINT(verbose_e, "TzeroLoaded %s\n", TzeroLoaded.toString(DATE_TIME_FMT).toAscii().data());
    if (actualVisibleWindowSec > LoadedWindowSec)
    {
        LoadedWindowSec = actualVisibleWindowSec;
    }

    /* calculate the TrendPeriodSec */
    /* if it is less than LogPeriodSec, set at LogPeriodSec */
    TrendPeriodSec = (int)ceil(((float)(LoadedWindowSec - (2 * LogPeriodSec)) / MAX_SAMPLE_NB));
    TrendPeriodSec = (TrendPeriodSec < LogPeriodSec) ? LogPeriodSec : TrendPeriodSec;

    sample_to_skip =
            (int)(TrendPeriodSec/LogPeriodSec)
            +
            ((TrendPeriodSec%LogPeriodSec == 0)  ? 0 : 1)
            -
            1;

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
    if (Load(TzeroLoaded, Tfin, sample_to_skip) == false)
    {
        LOG_PRINT(verbose_e, "Cannot load data\n");
        LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
        disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
        LOG_PRINT(verbose_e, "CONNECT refreshEvent\n");
        connect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)),Qt::QueuedConnection);
        return false;
    }
    LOG_PRINT(verbose_e, "PARSED2 TREND FILE\n");

#ifdef STATIC_AXES
    if (timeScale == NULL)
    {
        timeScale = new TimeScaleDraw(TzeroLoaded.time());
        d_qwtplot->setAxisScaleDraw(QwtAxisId( timeAxisId, 0 ), timeScale);
    }
    else
    {
        timeScale->setBaseTime(TzeroLoaded.time());
    }
    d_qwtplot->setAxisVisible( QwtAxisId( timeAxisId, 0 ), true);
    LOG_PRINT(verbose_e, "Setting time axis\n");
#else
    d_qwtplot->setAxisScaleDraw(timeAxisId, new DateTimeScaleDraw(TzeroLoaded));
#endif
#ifdef VALUE_TIME_SCALE
    for (int i = 0; i < PEN_NB; i++)
    {
        int decimal = 0;
        if (pens[i].visible)
        {
#ifdef STATIC_AXES
#if 0 //moved into LoadTRend since if no data are available valueScale is null and you get SIG FAULT
            decimal =  getVarDecimalByName(pens[i].tag);
            if (valueScale[i] == NULL)
            {
                valueScale[i] = new NormalScaleDraw(decimal);
            }
#endif

#else
            decimal =  getVarDecimalByName(pens[i].tag);
#endif
        }
#ifdef STATIC_AXES
        d_qwtplot->setAxisScaleDraw(, valueScale[i]);
#else
        d_qwtplot->setAxisScaleDraw(QwtAxisId( valueAxisId, i ), new NormalScaleDraw(decimal));
#endif
        //LOG_PRINT(verbose_e, "decimals %d\n", decimal);
    }
#endif
    LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
    disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
    LOG_PRINT(verbose_e, "CONNECT refreshEvent\n");
    connect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)),Qt::QueuedConnection);
    return true;
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
    double horiz_ticks = HORIZ_TICKS;
    if (this->width() <= 320){
        horiz_ticks = 3;
    } else if (this->width() <= 480){
        horiz_ticks = 5;
    } else if (this->width() <= 800){
        horiz_ticks = 9;
    }
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
        if (pens[pen_index].curve != NULL)
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
            
#ifdef STATIC_AXES
            if (valueScale[pen_index]->getDecimalNb() != decimal)
            {
                valueScale[pen_index]->setDecimalNb(decimal);
            }
#else
            d_qwtplot->setAxisScaleDraw(QwtAxisId( valueAxisId, pen_index ), new NormalScaleDraw(decimal));
#endif
            /* prepare the value axis tick */
            QList<double> arrayTicks;
            double deltay = (pens[pen_index].yMaxActual - pens[pen_index].yMinActual) / VERT_TICKS;
            for (int i = 0; i < (VERT_TICKS +1); i++)
            {
                arrayTicks.append(pens[pen_index].yMinActual + i * deltay);
            }
            QwtScaleDiv scale = QwtScaleDiv(pens[pen_index].yMinActual,pens[pen_index].yMaxActual);

            //scale = d_qwtplot->axisScaleDiv(valueAxisId + pen_index);
            scale.setTicks(QwtScaleDiv::MajorTick, (arrayTicks));
            d_qwtplot->setAxisAutoScale(QwtAxisId( valueAxisId, pen_index ), false);
            d_qwtplot->setAxisScaleDiv( QwtAxisId( valueAxisId, pen_index ), scale);
            
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
                    if (_layout_ == PORTRAIT)
                    {
                        pens[pen_index].curve->setRawSamples(&(pens[pen_index].y[XindexIn]), &(pens[pen_index].x[XindexIn]), XindexFin - XindexIn);
                    }
                    else
                    {
                        pens[pen_index].curve->setRawSamples(&(pens[pen_index].x[XindexIn]), &(pens[pen_index].y[XindexIn]), XindexFin - XindexIn);
                    }
                    pens[pen_index].curve->attach(d_qwtplot);

                    // update value (QLabel)
                    if (pen_index == pen && this->isVisible())
                    {
                        ui->labelDate->setText(QString(" %1 ").arg(pens[pen].y[XindexFin - 1], 7, 'f', decimal));
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
    if (_load_window_busy || this->isVisible() == false)
    {
        LOG_PRINT(verbose_e, "BUSY\n");
        return false;
    }
    _load_window_busy = true;
    LOG_PRINT(verbose_e, "Current '%s' Tmin '%s' Tmax '%s' ymin %f ymax %f pen %d\n",
              QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data(),
              Tmin.toString(DATE_TIME_FMT).toAscii().data(),
              Tmax.toString(DATE_TIME_FMT).toAscii().data(),
              ymin,
              ymax,
              pen
              );
    if (Tmax < Tmin)
    {
        //warning
        LOG_PRINT(error_e, "Tmin in the future! %s vs %s\n",Tmin.toString(DATE_TIME_FMT).toAscii().data(), Tmax.toString(DATE_TIME_FMT).toAscii().data());
    }

    //verbose
    LOG_PRINT(verbose_e, "actualVisibleWindowSec %d VisibleWindowSec %d actualTzero %s Tzero %s TzeroLoaded %s\n",
              actualVisibleWindowSec,
              VisibleWindowSec,
              actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
              Tzero.toString(DATE_TIME_FMT).toAscii().data(),
              TzeroLoaded.toString(DATE_TIME_FMT).toAscii().data());

    /* calculate the TrendPeriodSec */
    /* if it is less than LogPeriodSec, set at LogPeriodSec */
    TrendPeriodSec = (int)ceil(((float)(LoadedWindowSec - (2 * LogPeriodSec)) / MAX_SAMPLE_NB));
    if (TrendPeriodSec < LogPeriodSec)
        TrendPeriodSec = LogPeriodSec;

    int skip = (int)(TrendPeriodSec/LogPeriodSec)
            +
            ((TrendPeriodSec%LogPeriodSec == 0)  ? 0 : 1)
            -
            1;

    // need to reload data from file?
    if ( _trend_data_reload_
       || (  _layout_ == LANDSCAPE
         && (  actualTzero < TzeroLoaded
            || actualTzero.addSecs(actualVisibleWindowSec) > TzeroLoaded.addSecs(LoadedWindowSec)
            )
         )
       || ( _layout_ == PORTRAIT
          && (  actualTzero.addSecs(-actualVisibleWindowSec) < TzeroLoaded
             || actualVisibleWindowSec > LoadedWindowSec
             )
          )
       || sample_to_skip != skip
       )
    {

        LOG_PRINT(verbose_e, "The new Tzero (%s) is out of bounds (%s ... %s), reload the data from the files (VisibleWindowSec %d)\n",
                  actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                  TzeroLoaded.toString(DATE_TIME_FMT).toAscii().data(),
                  TzeroLoaded.addSecs(LoadedWindowSec).toString(DATE_TIME_FMT).toAscii().data(),
                  actualVisibleWindowSec
                  );
        LOG_PRINT(verbose_e, "The new time window (%d) is too big (%d), reload the data for the new window -> new Tzero will be %s\n",
                  actualVisibleWindowSec,
                  LoadedWindowSec,
                  actualTzero.addSecs(-(int)((LoadedWindowSec - actualVisibleWindowSec) / 2)).toString(DATE_TIME_FMT).toAscii().data()
                  );

        if ( ! _online_ )
        {
            for (int i = 0; i < PEN_NB; i++)
            {
                if (pens[i].curve != NULL)
                {
                    pens[i].curve->detach();
                }
            }
        }
        popup->enableButtonUp(false);
        popup->enableButtonDown(false);
        popup->enableButtonLeft(false);
        popup->enableButtonRight(false);

        showStatus(trUtf8("Loading..."), false);

        if (VisibleWindowSec != actualVisibleWindowSec)
        {
            VisibleWindowSec = actualVisibleWindowSec;
        }
        LoadedWindowSec = OVERLOAD_SECONDS(VisibleWindowSec);

        LOG_PRINT(verbose_e, "actualTzero '%s' LoadedWindowSec '%d' actualVisibleWindowSec '%d' -> loadedTzero %s\n",
                  actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                  LoadedWindowSec,
                  actualVisibleWindowSec,
                  actualTzero.addSecs(- actualVisibleWindowSec -(int)((LoadedWindowSec - actualVisibleWindowSec) / 2)).toString(DATE_TIME_FMT).toAscii().data()
                  );

//        QDateTime Ti;
//        if (_layout_ == LANDSCAPE)
//        {
//            Ti = actualTzero.addSecs(-(int)((LoadedWindowSec - actualVisibleWindowSec) / 2));
//        }
//        else
//        {
//            Ti = actualTzero.addSecs( - actualVisibleWindowSec -(int)((LoadedWindowSec - actualVisibleWindowSec) / 2));
//        }

        if (loadFromFile(actualTzero) == false)
        {
//                errormsg = trUtf8("Cannot found any data from %1 to %2").arg(actualTzero.toString().toAscii().data()).arg(QDateTime::currentDateTime().toString().toAscii().data());
//                showStatus(errormsg, true);
//                errormsg.clear();
            LOG_PRINT(warning_e, "Cannot find any data from %s to %s\n", actualTzero.toString().toAscii().data(), QDateTime::currentDateTime().toString().toAscii().data());
            //LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), QDateTime::currentDateTime().toString().toAscii().data());
            //actualTzero = QDateTime::currentDateTime();
        }
        else if (actualTzero < TzeroLoaded || actualVisibleWindowSec > LoadedWindowSec)
        {
            LOG_PRINT(warning_e, "The new Tzero (%s) is still out of bounds (%s ... %s - VisibleWindowSec %d). Force the TzeroLoaded into the bound\n",
                      actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                      TzeroLoaded.toString(DATE_TIME_FMT).toAscii().data(),
                      TzeroLoaded.addSecs(LoadedWindowSec).toString(DATE_TIME_FMT).toAscii().data(),
                      actualVisibleWindowSec
                      );
            LOG_PRINT(error_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), TzeroLoaded.toString().toAscii().data());
            actualTzero = TzeroLoaded;
            d_qwtplot->setAxisVisible( QwtAxisId( timeAxisId, 0 ), true);
        }
    }

    bool ret_val = showWindow(Tmin, Tmax, ymin, ymax, pen);
    LOG_PRINT(verbose_e, "UUUUUUUUP pen %d Min %f Actual Min %f\n", actualPen, pens[actualPen].yMin, pens[actualPen].yMinActual);
    /* enable or disable up and down arrow in according with the available upper or lower point */
    if (pens[actualPen].yMax > pens[actualPen].yMaxActual)
    {
        if (_layout_ == LANDSCAPE)
        {
            popup->enableButtonUp(true);
        }
        else
        {
            popup->enableButtonLeft(true);
        }
    }
    else
    {
        if (_layout_ == LANDSCAPE)
        {
            popup->enableButtonUp(false);
        }
        else
        {
            popup->enableButtonLeft(false);
        }
    }
    LOG_PRINT(verbose_e, "DOWN pen %d Max %f Actual Max %f\n", actualPen, pens[actualPen].yMax, pens[actualPen].yMaxActual);
    if (pens[actualPen].yMin < pens[actualPen].yMinActual)
    {
        if (_layout_ == LANDSCAPE)
        {
            popup->enableButtonDown(true);
        }
        else
        {
            popup->enableButtonRight(true);
        }
    }
    else
    {
        if (_layout_ == LANDSCAPE)
        {
            popup->enableButtonDown(false);
        }
        else
        {
            popup->enableButtonRight(false);
        }
    }
    
    if (QDateTime::currentDateTime() > Tmax.addSecs(TrendPeriodSec))
    {
        if (_layout_ == LANDSCAPE)
        {
            popup->enableButtonRight(true);
        }
        else
        {
            popup->enableButtonUp(true);
        }
    }
    else
    {
        if (_layout_ == LANDSCAPE)
        {
            popup->enableButtonRight(false);
        }
        else
        {
            popup->enableButtonUp(false);
        }
    }
    
    if (_layout_ == LANDSCAPE)
    {
        popup->enableButtonLeft(true);
    }
    else
    {
        popup->enableButtonDown(true);
    }
    
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
        datetime = TzeroLoaded.addSecs((int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pos.y()))).toString("yyyy/MM/dd HH:mm:ss");
    }
    else
    {
        datetime = TzeroLoaded.addSecs((int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pos.x()))).toString("yyyy/MM/dd HH:mm:ss");
        y = QString::number(d_qwtplot->invTransform(QwtAxisId( valueAxisId, actualPen ), pos.y()), 'f', decimal);
    }
    
    ui->labelvalue->setText(datetime + "  " + y);
    ui->labelvalue->setStyleSheet(QString("border: 2px solid #%1;" "font: 14pt \"DejaVu Sans Mono\";").arg(pens[actualPen].color));
    ui->pushButtonPen->setVisible(false);
    ui->labelvalue->setVisible(true);

#ifdef MARKER
    d_marker->setValue(pos.x(), pos.y());
    LOG_PRINT(verbose_e, "############ MARKER %d\n", pos.x());
#endif
}

void trend::selected(const QPolygon &pol)
{
    if (_zoom)
    {
        zoomRect = pol.boundingRect();
        do_zoom = true;
    }
    else
    {
#ifdef MARKER
        d_marker->setValue(myYfin, 1.0);
        LOG_PRINT(verbose_e, "############ MARKER %f\n", myYfin);
        //d_qwtplot->replot();
#endif
    }
    ui->labelvalue->setText("");
    ui->labelvalue->setStyleSheet("");
    ui->labelvalue->setVisible(false);
    ui->pushButtonPen->setVisible(true);
}

void InterruptedCurve::drawCurve( QPainter *painter, __attribute__((unused))int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to ) const
{
    int preceding_from = from;
    bool is_gap = true;
    
    // Scan all data to identify gaps
    for (int i = from; i <= to; i++)
    {
        const QPointF sample = data()->sample(i);

        LOG_PRINT(verbose_e, "sample.y() %f -> %d, sample.x() %f -> %d is_gap %s\n", sample.y(), isnormal(sample.y()), sample.x(), isnormal(sample.x()), is_gap ? "true" : "false");
        
        // In a gap normal floating point number
        if(((isnormal(sample.y()) != 0 || sample.y() == 0) && (isnormal(sample.x()) != 0 || sample.x() == 0)) && is_gap)
        {
            preceding_from = i;
            LOG_PRINT(verbose_e, "SETTING GAP FALSE sample.y() %f, sample.x() %f is_gap %s\n", sample.y(), sample.x(), is_gap ? "true" : "false");
            is_gap = false;
        }
        
        // At the beginning of a gap (or the end of the serie) : draw the preceding interval
        // the number is not a floating point valid value and we are not in a gap or if it's the last normal value and
        // is a good value
        if(
                (((isnormal(sample.y()) == 0 && sample.y() != 0) || (isnormal(sample.x()) == 0 && sample.x() != 0)) && !is_gap)
                ||
                (i == to && ((isnormal(sample.y()) != 0 || sample.y() == 0) && (isnormal(sample.x()) != 0 || sample.x() == 0)))
                )
        {
            // or drawSteps, drawLines, drawSticks, drawDots
            drawLines(painter, xMap, yMap, canvasRect, preceding_from, ((i>from) ? (i-1) : i));
            LOG_PRINT(verbose_e, "SETTING GAP TRUE sample.y() %f, sample.x() %f is_gap %s i %d to %d\n", sample.y(), sample.x(), is_gap ? "true" : "false", i, to);
            is_gap = true;
        }
    }
}

void trend::loadOrientedWindow()
{
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

    ui->pushButtonPen->setVisible(!iserror);
    ui->labelDate->repaint();
}

void trend::on_pushButtonHome_clicked()
{
    go_home();
    _trend_data_reload_ = true;
    LOG_PRINT(verbose_e, "_trend_data_reload_ %d\n",  _trend_data_reload_);
    disableUpdate();
    hide();
}

void trend::on_pushButtonBack_clicked()
{
    go_back();
    _trend_data_reload_ = true;
    LOG_PRINT(verbose_e, "_trend_data_reload_ %d\n",  _trend_data_reload_);
    disableUpdate();
    hide();
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
            ui->pushButtonPen->setText(
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
            ui->pushButtonPen->setText(
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
            ui->pushButtonPen->setText(trUtf8("Trace '%1' is not visible").arg(pens[actualPen].description));
        }
        else
        {
            ui->pushButtonPen->setText(trUtf8("Trace '%1' is not visible").arg(pens[actualPen].tag));
        }
    }
    else
    {
        if (strlen(pens[actualPen].description) > 0)
        {
            ui->pushButtonPen->setText(trUtf8("Trace '%1' is empty").arg(pens[actualPen].description));
        }
        else
        {
            ui->pushButtonPen->setText(trUtf8("Trace '%1' is empty").arg(actualPen + 1));
        }
    }
    return;
}
