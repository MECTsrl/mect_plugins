/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * CopyLeft Mect s.r.l. 2013
 *
 * @brief Main page
 */
#include "app_logprint.h"
#include "global_functions.h"
#include "trend.h"
#include "item_selector.h"
#include "ui_trend.h"
#include "utility.h"
#include "alphanumpad.h"

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

#undef STATIC_AXES

#define HORIZ_TICKS 5
#define VERT_TICKS  4

#define INCREMENT 1.0
#define OVERLOAD_FACTOR 2
#define SHIFT_FACTOR 10
#define AXIS_STEP_DIVISOR 2
#define DELTA_TIME_FACTOR (AXIS_STEP_DIVISOR * 2)
#define DELTA_VALUE_FACTOR 4

#define DATE_TIME_FMT "yyyy/MM/dd HH:mm:ss"

#define DEFAULT_TREND QDate::currentDate().toString("yyyy_MM_dd.log").toAscii().data()

#define LINE2STR(line) \
{ \
    if (strchr(line, '\r')) \
    { \
        *strchr(line, '\r') = '\0'; \
    } \
    else if (strchr(line, '\n')) \
    { \
        *strchr(line, '\n') = '\0'; \
    } \
}

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
    mystyle.append("  font: 12pt \"DejaVu Sans Mono\";"); \
    d_qwtplot->setStyleSheet(mystyle); \
    }

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
        pens[i].x = NULL;
        pens[i].y = NULL;
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
    overloadActualTzero = true;

    popup = new trend_other(this);
    popup_visible = false;
    popup->hide();
    actualPen = 0;
}

#undef WINDOW_TITLE
#undef WINDOW_ICON

/**
 * @brief This is the reload member. The operation written here, are executed every time this page is shown.
 */
void trend::reload()
{
    LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
    disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));

    LOG_PRINT(verbose_e, " RELOADING\n");
    reloading = true;
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
    if (_trend_data_reload_ == true)
    {
        if (strlen(_actual_trend_) == 0)
        {
            LOG_PRINT(warning_e, "No trend selected.\n");
            LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
            disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
            refresh_timer->stop();
            errormsg = (trUtf8("No trend description specified."));
            //QMessageBox::critical(this,trUtf8("Invalid data"), trUtf8("No trend description specified."));
            force_back = true;
            reloading = false;
            LOG_PRINT(verbose_e, " RELOADED\n");
            return;
        }

        LOG_PRINT(verbose_e, "_trend_data_reload_\n");
        
        d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotCurve);
        d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotScale);
        d_qwtplot->detachItems(QwtPlotItem::Rtti_PlotUserItem);
        char filename[FILENAME_MAX];
        if (QFileInfo(_actual_trend_).suffix().compare("csv") == 0)
        {
            sprintf(filename, "%s/%s", CUSTOM_TREND_DIR, _actual_trend_);
        }
        else
        {
            sprintf(filename, "%s/%s.csv", CUSTOM_TREND_DIR, _actual_trend_);
        }
        
        /* load the actual trend info */
        if (LoadTrend(filename, &errormsg) == false)
        {
            force_back = true;
            reloading = false;
            LOG_PRINT(verbose_e, " RELOADED\n");
            return;
        }
        else
        {
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
                pens[z].y = new double [MAX_SAMPLE_NB + 1];
                pens[z].x = new double [MAX_SAMPLE_NB + 1];

                for (int i = 0; i < MAX_SAMPLE_NB; i++)
                {
                    pens[z].y[i] = /*pens[rownb].yMax*/NAN;
                    pens[z].x[i] = /*pens[rownb].yMax*/NAN;
                }
                pens[z].sample = 0;
            }
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
    TzeroLoaded = Tzero = QDateTime::currentDateTime();
    VisibleWindowSec = MaxWindowSec;
    
    /* at first time we are in online mode, so the window time start at the same of window loaded time */
    if (actualVisibleWindowSec == 0)
    {
        actualTzero = TzeroLoaded;
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
    
    LoadedWindowSec = OVERLOAD_FACTOR * VisibleWindowSec;
    
    /* calculate the TrendPeriodSec */
    /* if it is less than LogPeriodSec, set at LogPeriodSec */
    LogPeriodSec = ((LogPeriodSecF>LogPeriodSecS)?LogPeriodSecF:LogPeriodSecS);
    TrendPeriodSec = (int)(LoadedWindowSec / MAX_SAMPLE_NB);
    TrendPeriodSec = (TrendPeriodSec < LogPeriodSec) ? LogPeriodSec : TrendPeriodSec;
    /* set TzeroLoaded in the future to force the data load */
    TzeroLoaded = actualTzero.addSecs(TrendPeriodSec * 2);
    LOG_PRINT(verbose_e, "actualVisibleWindowSec %d VisibleWindowSec %d actualTzero '%s' Tzero '%s' TzeroLoaded '%s' TrendPeriodSec %d LogPeriodSec %d\n",
              actualVisibleWindowSec,
              VisibleWindowSec,
              actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
              Tzero.toString(DATE_TIME_FMT).toAscii().data(),
              TzeroLoaded.toString(DATE_TIME_FMT).toAscii().data(),
              TrendPeriodSec,
              LogPeriodSec
              );
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
    d_qwtplot->setAxisScaleDraw(timeAxisId, new TimeScaleDraw(TzeroLoaded.time()));
#endif
    
    first_time = true;
    reloading = false;
    LOG_PRINT(verbose_e, " RELOADED\n");
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
    //page::updateData();

    if ((reloading == true || _trend_data_reload_ == false ) && first_time == false)
    {
        return;
    }

    if (force_back == true)
    {
        /* select a new trend from CUSTOM_TREND_DIR directory */
        item_selector * sel;
        QString value;
        QStringList trendList;
        QDir trendDir(CUSTOM_TREND_DIR, "*.csv");

        trendList = trendDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
        if (trendList.count() > 0)
        {
            sel = new item_selector(trendList, &value, trUtf8("TREND SELECTOR"));
            sel->showFullScreen();

            if (sel->exec() == QDialog::Accepted)
            {
                strcpy(_actual_trend_, QFileInfo(value).baseName().toAscii().data());
                LOG_PRINT(verbose_e, "FULL NAME %s\n", QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_).toAscii().data());
                if (LoadTrend(QString("%1/%2.csv").arg(CUSTOM_TREND_DIR).arg(_actual_trend_).toAscii().data(), &errormsg) == false)
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
                        pens[z].y = new double [MAX_SAMPLE_NB + 1];
                        pens[z].x = new double [MAX_SAMPLE_NB + 1];

                        for (int i = 0; i < MAX_SAMPLE_NB; i++)
                        {
                            pens[z].y[i] = /*pens[rownb].yMax*/NAN;
                            pens[z].x[i] = /*pens[rownb].yMax*/NAN;
                        }
                        pens[z].sample = 0;
                    }
                    reload();
                    refresh_timer->start(REFRESH_MS);
                    return;
                }
            }
            else
            {
                errormsg = trUtf8("No trend selected");
                go_back();
            }
            delete sel;
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

    if(first_time == true)
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
        if (overloadActualTzero == true)
        {
            actualVisibleWindowSec = VisibleWindowSec;
            LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), Tzero.toString().toAscii().data());
            actualTzero = Tzero;
        }
        
        loadOrientedWindow();

        actualPen--;
        on_pushButtonSelect_clicked();

        LOG_PRINT(verbose_e, "DISCONNECT refreshEvent\n");
        disconnect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)));
        LOG_PRINT(verbose_e, "CONNECT refreshEvent\n");
        connect(logger, SIGNAL(new_trend(trend_msg_t)), this, SLOT(refreshEvent(trend_msg_t)),Qt::QueuedConnection);
    }
    
    LOG_PRINT(verbose_e, "UPDATE\n");
    if (popup_visible != false)
    {
        popup->reload();
        popup->show();
        popup->raise();
    }
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
    LOG_PRINT(verbose_e, "MUOIO\n");
    delete ui;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void trend::refreshEvent(trend_msg_t item_trend)
{
    LOG_PRINT(verbose_e,"NEW SAMPLE\n");
    static int last_ctindex = -1;
    
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
        if (pens[i].visible == 0)
        {
            /* skip invisible pen */
            continue;
        }
        /* found actual pen */
        if (pens[i].CtIndex == item_trend.CtIndex)
        {
            LOG_PRINT(verbose_e,"NEW SAMPLE ADD '%d' -> '%f' time: '%s' SAMPLE %d\n",  item_trend.CtIndex, item_trend.value,  item_trend.timestamp.toString(DATE_TIME_FMT).toAscii().data(), pens[i].sample);
            if (last_ctindex == -1)
            {
                last_ctindex = item_trend.CtIndex;
            }
            /* add a new sample to the plot */
            if (pens[i].sample >= MAX_SAMPLE_NB) {
                if (_online_){
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
            if (pens[i].y[pens[i].sample] > pens[i].yMaxActual)
            {
                usleep(1);
                LOG_PRINT(warning_e, "out of range %f > %f.\n", pens[i].y[pens[i].sample], pens[i].yMaxActual);
            }
            pens[i].sample++;
#ifdef SHOW_ACTUAL_VALUE
            updatePenLabel();
#endif
        }
    }
    
    if (item_trend.CtIndex == last_ctindex)
    {
        if (getOnline()){
            static bool rounding = false;
            if (rounding)
            {
                ui->pushButtonOnline->setStyleSheet("QPushButton"
                                                    "{"
                                                    "border-image: url(:/libicons/img/Chess1.png);"
                                                    "qproperty-focusPolicy: NoFocus;"
                                                    "}");
            }
            else
            {
                ui->pushButtonOnline->setStyleSheet("QPushButton"
                                                    "{"
                                                    "border-image: url(:/libicons/img/Chess2.png);"
                                                    "qproperty-focusPolicy: NoFocus;"
                                                    "}");
            }
            rounding = !rounding;
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
        
        /* update the graph only if the status is online or if the new sample are visible */
        if (
                _online_
                ||
                item_trend.timestamp < actualTzero.addSecs(actualVisibleWindowSec)
                ||
                pens[actualPen].x[pens[actualPen].sample] < actualVisibleWindowSec
                )
        {
            if (_online_ == false)
            {
                LOG_PRINT(verbose_e,"OFFLINE BUT ARE VISIBLE CURRENT DATA: UPDATE IT _online_ %d timestamp '%s' - '%s' -> '%s'\n",
                          _online_,
                          item_trend.timestamp.toString("HH:mm:ss").toAscii().data(),
                          actualTzero.toString("HH:mm:ss").toAscii().data(),
                          actualTzero.addSecs(actualVisibleWindowSec).toString("HH:mm:ss").toAscii().data()
                          );
            }
            else
            {
                /* online mode: if necessary center the actual data */
                LOG_PRINT(verbose_e,"ONLINE: CENTER to actual data actual sample %s actual Tmax %s\n",
                          item_trend.timestamp.toString(DATE_TIME_FMT).toAscii().data(),
                          actualTzero.addSecs(actualVisibleWindowSec).toString(DATE_TIME_FMT).toAscii().data()
                          );
                if (_layout_ == PORTRAIT)
                {
                    LOG_PRINT(verbose_e,"ONLINE ACTUAL T0 from '%s' to '%s\n",
                              actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                              item_trend.timestamp.toString(DATE_TIME_FMT).toAscii().data()
                              );
                    LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), item_trend.timestamp.toString().toAscii().data());
                    actualTzero = item_trend.timestamp;
                }
                else
                {
                    int increment = (actualVisibleWindowSec/SHIFT_FACTOR < LogPeriodSec) ? LogPeriodSec : actualVisibleWindowSec/SHIFT_FACTOR;
                    
                    QDateTime limit;
                    limit = actualTzero.addSecs(actualVisibleWindowSec);
                    
                    while (item_trend.timestamp > limit)
                    {
                        limit = actualTzero.addSecs(actualVisibleWindowSec);
                        LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), actualTzero.addSecs(increment).toString().toAscii().data());
                        actualTzero = actualTzero.addSecs(increment);
                        LOG_PRINT(verbose_e,"ONLINE: CENTER to actual data actual sample %s actual Tmin %s Tmax %s increment %d\n",
                                  item_trend.timestamp.toString(DATE_TIME_FMT).toAscii().data(),
                                  actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                                  actualTzero.addSecs(actualVisibleWindowSec).toString(DATE_TIME_FMT).toAscii().data(),
                                  increment
                                  );
                    }
                }
            }
            LOG_PRINT(verbose_e,"NEW SAMPLE DRAWN '%s' -> '%f' time: '%s'\n",  varNameArray[item_trend.CtIndex].tag, item_trend.value,  item_trend.timestamp.toString(DATE_TIME_FMT).toAscii().data());
            
            loadOrientedWindow();
        }
        else
        {
            LOG_PRINT(verbose_e,"OFFLINE AND NOT VISIBLE CURRENT DATA: DO NOTHING _online_ %d timestamp '%s' - '%s' -> '%s'\n", _online_, item_trend.timestamp.toString("HH:mm:ss").toAscii().data(), actualTzero.toString("HH:mm:ss").toAscii().data(), actualTzero.addSecs(actualVisibleWindowSec).toString("HH:mm:ss").toAscii().data());
        }
    }
    LOG_PRINT(verbose_e,"NEW SAMPLE\n");
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
    
    loadOrientedWindow();
    
    // Calling setOnline
    //setOnline(true);
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
    if (_load_window_busy == true)
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
    QDateTime iterator;
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
    QStringList logFileList = logDir.entryList(QDir::Files|QDir::NoDotAndDotDot);
    for (int i = 0; i < logFileList.count(); i++)
    {
        // check if the log time is into the interval
        if (
                (
                    begin <= QDateTime::fromString(QFileInfo(logFileList.at(i)).baseName(),"yyyy_MM_dd_HH_mm_ss")
                    ||
                    (
                        i + 1 < logFileList.count()
                        &&
                        begin < QDateTime::fromString(QFileInfo(logFileList.at(i + 1)).baseName(),"yyyy_MM_dd_HH_mm_ss")
                        )
                    ||
                    (
                        i + 1 == logFileList.count()
                        &&
                        begin < QDateTime::currentDateTime()
                        )
                    )
                &&
                end >= QDateTime::fromString(QFileInfo(logFileList.at(i)).baseName(),"yyyy_MM_dd_HH_mm_ss")
                )
        {
            LOG_PRINT(verbose_e, "PARSING TREND FILE '%s'\n", logFileList.at(i).toAscii().data());
            if (Load(QString("%1/%2").arg(STORE_DIR).arg(logFileList.at(i)).toAscii().data(), &begin, &end, skip) == false)
            {
                LOG_PRINT(verbose_e, "Cannot load '%s'\n", logFileList.at(i).toAscii().data());
                return false;
            }
            LOG_PRINT(verbose_e, "PARSED TREND FILE '%s'\n", logFileList.at(i).toAscii().data());
            logfound++;
        }
    }

    if (logfound == 0)
    {
        LOG_PRINT(verbose_e, "cannot found any sample from begin '%s' to '%s'\n",
                  begin.toString("yyyy/MM/dd HH:mm:ss").toAscii().data(),
                  end.toString("yyyy/MM/dd HH:mm:ss").toAscii().data()
                  );
        return false;
    }
    else
    {
        LOG_PRINT(verbose_e, "found '%d' logs\n", logfound);
        return true;
    }
}

bool trend::Load(const char * filename, QDateTime * begin, QDateTime * end, int skip)
{
    char line[LINE_SIZE] = "";
    int found = -1;
    char * p = NULL, * r = NULL;
    char token[LINE_SIZE] = "";

    LOG_PRINT(verbose_e, "LOG_FILE '%s'\n", filename);
    LOG_PRINT(verbose_e, "#######'%s' -> '%s'\n",
              begin->toString("yyyy/MM/dd HH:mm:ss").toAscii().data(),
              end->toString("yyyy/MM/dd HH:mm:ss").toAscii().data()
              );
    
    FILE * fp = NULL;
    fp = fopen (filename, "r");
    if (fp != 0)
    {
        LOG_PRINT(verbose_e, "FOUND LOG FILE '%s'\n", filename);
        /*
         * File format:
         * <yyyy/MM/dd>; <HH:mm:ss>; <tag1> ... <tagN>
         */
        
        /* extract title */
        if (fgets(line, LINE_SIZE, fp) == NULL)
        {
            fclose(fp);
            LOG_PRINT(error_e, "Cannot extract the title from '%s'\n", filename);
            return false;
        }
        
        /* date */
        p = strtok_csv(line, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0' || strcmp(p, "date") != 0)
        {
            LOG_PRINT(error_e, "Invalid date field '%s' '%s'\n", line, p);
            fclose(fp);
            return false;
        }
        
        /* time */
        p = strtok_csv(NULL, SEPARATOR, &r);
        if (p == NULL || p[0] == '\0' || strcmp(p, "time") != 0)
        {
            LOG_PRINT(error_e, "Invalid time field '%s' '%s'\n", line, p);
            fclose(fp);
            return false;
        }
        
        QList<int> filter;

        while ((p = strtok_csv(NULL, SEPARATOR, &r)) != NULL)
        {
            found = -1;
            for (int i = 0; i < PEN_NB; i++)
            {
                if (pens[i].curve == NULL)
                {
                    continue;
                }
                LOG_PRINT(verbose_e, "looking for '%s'' vs '%s'\n", pens[i].tag, p);
                if (strcmp(pens[i].tag, p) == 0)
                {
                    if (found == -1)
                    {
                        found = 0x0;
                    }
                    found |=(0x1<< i);
                    
                    LOG_PRINT(verbose_e, "found tag '%s' filter %x\n", p, found);
                }
            }
            
            filter.append(found);
        }

        /* extract data */
        QString dateandtime;
        int count = 0;
        int rownb = 0;
        char beginstr [20];
        char endstr [20];
        char enddatestr [20];
        strcpy(beginstr, begin->toString("HH:mm:ss").toAscii().data());
        strcpy(endstr, end->toString("HH:mm:ss").toAscii().data());
        strcpy(enddatestr, end->toString("yyyy/MM/dd").toAscii().data());
        while (fgets(line, LINE_SIZE, fp) != NULL)
        {
            if (count < skip)
            {
                LOG_PRINT(verbose_e, "Skip\n");
                count++;
                continue;
            }
            bool valid = false;
            bool finish = false;
            count = 0;
            
            LINE2STR(line);
            int i;
            int len;
            for (i = 0, p = line; p != NULL; p = strchr(p, ';'), i++)
            {
                if (p != NULL)
                {
                    if (p != line)
                    {
                        p++;
                    }
                    token[0] = '\0';
                    if (strchr(p, ';'))
                    {
                        len = strchr(p, ';')-p;
                        strncpy(token, p, len);
                        token[len] = '\0';
                    }
                    else
                    {
                        strcpy(token, p);
                    }
                    sscanf(token, "%s", token);
                    if (isspace(token[0]))
                    {
                        token[0] = '\0';
                    }
                    /* date */
                    if(i == 0)
                    {
                        if (begin != NULL && strcmp(token, enddatestr) < 0)
                        {
                            valid = true;
                        }
                        dateandtime.clear();
                        dateandtime.append(token);
                        LOG_PRINT(verbose_e, "valid log '%s' < '%s' -> '%s'\n", token , enddatestr, dateandtime.toAscii().data());
                    }
                    /* time */
                    else if(i == 1)
                    {
                        if (valid == false)
                        {
                            if (begin != NULL && strcmp(token, beginstr) < 0)
                            {
                                LOG_PRINT(verbose_e, "Skip old log ('%s') '%s'\n", begin->toString("HH:mm:ss").toAscii().data(), token);
                                count = skip;
                                break;
                            }
                            if (end != NULL && strcmp(token, endstr) > 0)
                            {
                                finish = true;
                                LOG_PRINT(verbose_e, "last valid log '%s' ('%s')\n", token, end->toString("HH:mm:ss").toAscii().data());
                                LOG_PRINT(verbose_e, "load DONE\n");
                                break;
                            }
                        }
                        dateandtime.append(" ");
                        dateandtime.append(token);
                        LOG_PRINT(verbose_e, "valid log '%s' < '%s' -> '%s'\n", token , enddatestr, dateandtime.toAscii().data());
                    }
                    else if (i > 1)
                    {
                        if (i-2 < filter.count() && filter.at(i-2) != -1)
                        {
                            for( int j = 0; j < PEN_NB; j++)
                            {
                                if (((filter.at(i-2) >> j) & 0x1) == 0x1 && token[0] != '-')
                                {
                                    LOG_PRINT(verbose_e, "tag '%s', '%d'\n", token, j);
                                    pens[j].y[pens[j].sample] = atof(token);
                                    pens[j].x[pens[j].sample] = TzeroLoaded.secsTo(QDateTime::fromString(dateandtime, DATE_TIME_FMT));
                                    LOG_PRINT(verbose_e, "%d %s : Y[%d]=%f\n", j, pens[j].tag, pens[j].sample, pens[j].y[pens[j].sample-1]);
                                    pens[j].sample++;
                                    if (pens[j].sample > MAX_SAMPLE_NB)
                                    {
                                        LOG_PRINT(verbose_e, "fatal error max sample %d reach! TrendPeriodSec %d LoadedWindowSec %d\n", MAX_SAMPLE_NB, TrendPeriodSec, LoadedWindowSec);
                                        LOG_PRINT(verbose_e, "begin %s end %s skip %d\n", begin->toString(DATE_TIME_FMT).toAscii().data(), end->toString(DATE_TIME_FMT).toAscii().data(), skip);
                                        //LOG_PRINT(verbose_e, "actual_date_time %s end %s\n", actual_date_time.toString(DATE_TIME_FMT).toAscii().data(), end->toString(DATE_TIME_FMT).toAscii().data());
                                        LOG_PRINT(error_e, "Too many sample\n");
                                        fclose(fp);
                                        return false;
                                    }
                                }
                            }
                        }
                    }
                    LOG_PRINT(verbose_e, "%d -> '%s'\n", i++, token);
                }
            }
            if (finish == true)
            {
                LOG_PRINT(verbose_e, "load DONE\n");
                break;
            }
            if (count != skip)
            {
                rownb++;
            }
        }
        fclose(fp);
        LOG_PRINT(verbose_e, "CLOSE TREND FILE '%s'\n", filename);
    }
    else
    {
        LOG_PRINT(error_e, "no trend file '%s'\n", filename);
        return false;
    }
    LOG_PRINT(verbose_e, "LOADED TREND FILE '%s'\n", filename);
    return true;
}

void trend::incrementTime(int direction)
{
    setOnline(false);
    
    int increment = ((actualVisibleWindowSec / DELTA_TIME_FACTOR) < LogPeriodSec) ? LogPeriodSec : (actualVisibleWindowSec / DELTA_TIME_FACTOR);
    LOG_PRINT(verbose_e, "actualTzero '%s', actualVisibleWindowSec %d -> %d -> actualTzero '%s', actualVisibleWindowSec %d\n",
              actualTzero.toString("HH:mm:ss").toAscii().data(),
              actualVisibleWindowSec,
              increment,
              actualTzero.addSecs(increment * direction).toString("HH:mm:ss").toAscii().data(),
              actualVisibleWindowSec + (increment * direction)
              );
    
    /* update the actual window parameters */
    LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), actualTzero.addSecs(increment * direction).toString().toAscii().data());
    actualTzero = actualTzero.addSecs(increment * direction);
    if (actualTzero > QDateTime::currentDateTime())
    {
        actualTzero = QDateTime::currentDateTime();
    }
    
    loadOrientedWindow();
}

void trend::incrementValue(int direction)
{
    for (int i = 0; i < PEN_NB; i++)
    {
        float delta = (pens[i].yMaxActual - pens[i].yMinActual) / DELTA_VALUE_FACTOR;
        LOG_PRINT(verbose_e, "DOWN %f * %f = %f\n",
                  pens[i].yMinActual,
                  delta,
                  pens[i].yMinActual - delta
                  );
        pens[i].yMinActual += (delta * direction);
        pens[i].yMaxActual += (delta * direction);
    }
    
    loadOrientedWindow();
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
    
    /* the data loaded will be not under-sampled */
    if (sample_to_skip > 0)
    {
        LOG_PRINT(warning_e, "the data loaded will be under-sampled sample_to_skip %d sample_to_load %d\n", sample_to_skip, (int)(LoadedWindowSec / TrendPeriodSec));
    }
    /* the data loaded will be under-sampled */
    else
    {
        LOG_PRINT(verbose_e, "the data loaded will be not under-sampled sample_to_skip %d sample_to_load %d\n", sample_to_skip, (int)(LoadedWindowSec / TrendPeriodSec));
    }
    
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
    d_qwtplot->setAxisScaleDraw(timeAxisId, new TimeScaleDraw(TzeroLoaded.time()));
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

    arrayTimeTicks << sinint;
    for (int i = 0; i < HORIZ_TICKS; i++)
    {
        arrayTimeTicks << sinint + (sfinal - sinint) * i / HORIZ_TICKS;
    }
    arrayTimeTicks << sfinal;
    
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
            int XindexIn = 0, XindexFin = 0, nsec = 0;
            nsec = TzeroLoaded.secsTo(Tmin);
            for (XindexIn = 0; pens[pen_index].x[XindexIn] < nsec && XindexIn < pens[pen_index].sample; XindexIn++)
            {
                LOG_PRINT(verbose_e, "pens[%d].x[%d] = %f ?> nsec %d\n",
                          pen_index,
                          XindexIn,
                          pens[pen_index].x[XindexIn],
                          nsec
                          );
            }
            XindexIn = ((XindexIn > 0 && pens[pen_index].x[XindexIn] > nsec) ? (XindexIn - 1): XindexIn);
            LOG_PRINT(verbose_e, "pens[%d].x[%d] = %f > nsec %d\n",
                      pen_index,
                      XindexIn,
                      pens[pen_index].x[XindexIn],
                      nsec
                      );
            
            int decimal = getVarDecimal(pens[pen_index].CtIndex);
            
            float n = (pens[pen_index].yMaxActual - pens[pen_index].yMinActual) / VERT_TICKS;

            /* FIXME : write a better code to extract the decimal's number */
            char tmp[32];
            sprintf (tmp, "%.6f",n);
            char *p = strchr(tmp, '.');
            int mindecimal = 0;
            for (mindecimal = 6; mindecimal > 0; mindecimal--)
            {
                    if(p[mindecimal] != '0')
                    {
                            break;
                    }
            }

            decimal = (decimal < mindecimal) ? mindecimal : decimal;
#ifdef STATIC_AXES
            if (valueScale[pen_index]->getDecimalNb() != decimal)
            {
                valueScale[pen_index]->setDecimalNb(decimal);
            }
#else
            d_qwtplot->setAxisScaleDraw(QwtAxisId( valueAxisId, pen_index ), new NormalScaleDraw(decimal));
#endif
            /* prepare the value axis tick */
            LOG_PRINT(verbose_e, "SCALE %d, min %f max %f decimal %d\n", valueAxisId + pen_index, pens[pen_index].yMinActual, pens[pen_index].yMaxActual, decimal);
            QList<double> arrayTicks;
            arrayTicks << pens[pen_index].yMinActual;
            for (int i = 0; i < VERT_TICKS; i++)
            {
                arrayTicks << pens[pen_index].yMinActual + (pens[pen_index].yMaxActual - pens[pen_index].yMinActual) * i / VERT_TICKS;
            }
            arrayTicks << pens[pen_index].yMaxActual;
            QwtScaleDiv scale = QwtScaleDiv(pens[pen_index].yMinActual,pens[pen_index].yMaxActual);
            //scale = d_qwtplot->axisScaleDiv(valueAxisId + pen_index);
            scale.setTicks(QwtScaleDiv::MajorTick, (arrayTicks));
            d_qwtplot->setAxisAutoScale(QwtAxisId( valueAxisId, pen_index ), false);
            d_qwtplot->setAxisScaleDiv( QwtAxisId( valueAxisId, pen_index ), scale);
            
            /* if online, the last sample is pens[pen_index].sample */
            if (_online_)
            {
                XindexFin = pens[pen_index].sample;
                LOG_PRINT(verbose_e, "pens[%d].x[%d] = %f > nsec %d\n",
                          pen_index,
                          XindexFin,
                          pens[pen_index].x[XindexFin],
                          nsec
                          );
            }
            /* if offline, looking for the last sample */
            else
            {
                int nsec = TzeroLoaded.secsTo(Tmax);
                for (XindexFin = pens[pen_index].sample; pens[pen_index].x[XindexFin] > nsec && XindexFin > XindexIn; XindexFin--);
                XindexFin = ((pens[pen_index].x[XindexFin] < nsec) ? (XindexFin + 1): XindexFin);
                LOG_PRINT(verbose_e, "pens[%d].x[%d] = %f > nsec %d\n",
                          pen_index,
                          XindexFin,
                          pens[pen_index].x[XindexFin],
                          nsec
                          );
            }
            
            if (pens[pen_index].visible == true)
            {
                if (XindexFin > XindexIn)
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

                    if (_layout_ == PORTRAIT)
                    {
                        LOG_PRINT(verbose_e, "PORTRAIT\n");
                        pens[pen_index].curve->setRawSamples(&(pens[pen_index].y[XindexIn]), &(pens[pen_index].x[XindexIn]), XindexFin - XindexIn);
                    }
                    else
                    {
                        pens[pen_index].curve->setRawSamples(&(pens[pen_index].x[XindexIn]), &(pens[pen_index].y[XindexIn]), XindexFin - XindexIn);
                        LOG_PRINT(verbose_e, "LANDSCAPE \n");
                    }

                    LOG_PRINT(verbose_e, "NEW SAMPLE LAST '%s'  %f %f in %d fin %d sample %d\n", pens[pen_index].tag, pens[pen_index].x[XindexFin-1], pens[pen_index].y[XindexFin-1], XindexIn, XindexFin, pens[pen_index].sample);
                }
                else
                {
                    LOG_PRINT(warning_e, "no point to show\n");
                    showStatus(trUtf8("No point to show"), true);
                    //return false;
                }
            }
        }
    }
    
    /* update the plot */
    LOG_PRINT(verbose_e,"before replot\n");
    d_qwtplot->updateAxes();
    d_qwtplot->replot();
    LOG_PRINT(verbose_e, "UPDATE AXES AND REPLOT!\n");
    
    /* update the actual date label */
    if (this->isVisible())
    {
        if (ui->labelDate->text().compare(actualTzero.toString("yyyy/MM/dd")) != 0)
        {
            showStatus(actualTzero.toString("yyyy/MM/dd"), false);
        }
    }
    LOG_PRINT(verbose_e, "return!\n");
    
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
        LOG_PRINT(warning_e, "Tmin in the future! %s vs %s\n",Tmin.toString(DATE_TIME_FMT).toAscii().data(), Tmax.toString(DATE_TIME_FMT).toAscii().data());
    }
    if (Tmin <= QDateTime::currentDateTime())
    {
        /* set the actual Tzero and the actual VisibleWindowSec */
        if (_layout_ == LANDSCAPE)
        {
            LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), Tmin.toString().toAscii().data());
            actualTzero = Tmin;
        }
        else
        {
            LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), Tmax.toString().toAscii().data());
            actualTzero = Tmax;
        }
        actualVisibleWindowSec = Tmin.secsTo(Tmax);
        LOG_PRINT(verbose_e, "UPDATE actualVisibleWindowSec\n");
        LOG_PRINT(verbose_e, "@@@@@@@ NEW actualTzero %s actualVisibleWindowSec %d current '%s'\n",
                  actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                  actualVisibleWindowSec,
                  QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data()
                  );
        
        
        /* if for some reason the actual VisibleWindowSec is more little then the minimum sample
         * distance, set it at minimum sample distance
         */
        if (actualVisibleWindowSec < LogPeriodSec)
        {
            //warning
            LOG_PRINT(verbose_e, "Zoom to big! %d vs %d\n", actualVisibleWindowSec, LogPeriodSec);
            actualVisibleWindowSec = LogPeriodSec;
            LOG_PRINT(verbose_e, "UPDATE actualVisibleWindowSec\n");
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
        TrendPeriodSec = (TrendPeriodSec < LogPeriodSec) ? LogPeriodSec : TrendPeriodSec;
        
        int skip = (int)(TrendPeriodSec/LogPeriodSec)
                +
                ((TrendPeriodSec%LogPeriodSec == 0)  ? 0 : 1)
                -
                1;
        
        if (
                (_layout_ == LANDSCAPE && (actualTzero < TzeroLoaded || actualVisibleWindowSec > LoadedWindowSec))
                ||
                (_layout_ == PORTRAIT && (actualTzero.addSecs(-actualVisibleWindowSec) < TzeroLoaded || actualVisibleWindowSec > LoadedWindowSec))
                ||
                sample_to_skip > skip
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
            
            popup->enableButtonUp(false);
            popup->enableButtonDown(false);
            popup->enableButtonLeft(false);
            popup->enableButtonRight(false);

            showStatus(trUtf8("Loading..."), false);

            if (VisibleWindowSec != actualVisibleWindowSec)
            {
                VisibleWindowSec = actualVisibleWindowSec;
            }
            LoadedWindowSec = OVERLOAD_FACTOR * VisibleWindowSec;
            
            LOG_PRINT(verbose_e, "actualTzero '%s' LoadedWindowSec '%d' actualVisibleWindowSec '%d' -> loadedTzero %s\n",
                      actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                      LoadedWindowSec,
                      actualVisibleWindowSec,
                      actualTzero.addSecs(- actualVisibleWindowSec -(int)((LoadedWindowSec - actualVisibleWindowSec) / 2)).toString(DATE_TIME_FMT).toAscii().data()
                      );
            
            QDateTime Ti;
            if (_layout_ == LANDSCAPE)
            {
                Ti = actualTzero.addSecs(-(int)((LoadedWindowSec - actualVisibleWindowSec) / 2));
            }
            else
            {
                Ti = actualTzero.addSecs( - actualVisibleWindowSec -(int)((LoadedWindowSec - actualVisibleWindowSec) / 2));
            }
            
            if (loadFromFile(Ti) == false)
            {
                errormsg = trUtf8("Cannot found any data from %1 to %2").arg(actualTzero.toString().toAscii().data()).arg(QDateTime::currentDateTime().toString().toAscii().data());
                showStatus(errormsg, true);
                errormsg.clear();
                LOG_PRINT(warning_e, "Cannot found any data from %s to %s\n", actualTzero.toString().toAscii().data(), QDateTime::currentDateTime().toString().toAscii().data());
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
                LOG_PRINT(verbose_e, "UPDATE actualTzero from %s to %s\n", actualTzero.toString().toAscii().data(), TzeroLoaded.toString().toAscii().data());
                actualTzero = TzeroLoaded;
                d_qwtplot->setAxisVisible( QwtAxisId( timeAxisId, 0 ), true);
            }
        }
        else
        {
            LOG_PRINT(verbose_e, "The new Tzero (%s) is in bounds %s and actual window %d is more little then the loaded window %d \n",
                      actualTzero.toString(DATE_TIME_FMT).toAscii().data(),
                      TzeroLoaded.toString(DATE_TIME_FMT).toAscii().data(),
                      actualVisibleWindowSec,
                      LoadedWindowSec
                      );
        }
    }
    else
    {
        LOG_PRINT(warning_e, "Tmin in the future! %s vs %s\n",Tmin.toString(DATE_TIME_FMT).toAscii().data(), QDateTime::currentDateTime().toString(DATE_TIME_FMT).toAscii().data());
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
    QString x, y;
    if (_layout_ == PORTRAIT)
    {
        x = QString::number(d_qwtplot->invTransform(QwtAxisId( valueAxisId, actualPen ), pos.x()), 'f', decimal);
        y = TzeroLoaded.addSecs((int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pos.y()))).toString("HH:mm:ss");
    }
    else
    {
        x = TzeroLoaded.addSecs((int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pos.x()))).toString("HH:mm:ss");
        y = QString::number(d_qwtplot->invTransform(QwtAxisId( valueAxisId, actualPen ), pos.y()), 'f');
    }
    
    ui->labelvalue->setText(x + "; " + y);
    ui->labelvalue->setStyleSheet(QString("border: 2px solid #%1;" "font: 14pt \"DejaVu Sans Mono\";").arg(pens[actualPen].color));
    ui->labelvalue->setVisible(true);

#ifdef MARKER
    d_marker->setValue(pos.x(), pos.y());
    LOG_PRINT(verbose_e, "############ MARKER %d\n", pos.x());
#endif
}

void trend::selected(const QPolygon &pol)
{
    int myXin, myXfin;
    
    LOG_PRINT(verbose_e, "x %d y %d h %d w %d\n", pol.boundingRect().x(), pol.boundingRect().y(), pol.boundingRect().height(), pol.boundingRect().width());
    
    if (_zoom)
    {
        if (_layout_ == PORTRAIT)
        {
            myXin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pol.boundingRect().y()));
            myXfin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pol.boundingRect().y() + pol.boundingRect().height()));
        }
        else
        {
            myXin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pol.boundingRect().x()));
            myXfin = (int)(d_qwtplot->invTransform(QwtAxisId( timeAxisId, 0 ), pol.boundingRect().x() + pol.boundingRect().width()));
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
                pens[i].yMinActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), pol.boundingRect().x());
                pens[i].yMaxActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), pol.boundingRect().x() + pol.boundingRect().width());
            }
            else
            {
                pens[i].yMinActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), pol.boundingRect().y());
                pens[i].yMaxActual = d_qwtplot->invTransform(QwtAxisId( valueAxisId, i ), pol.boundingRect().y() + pol.boundingRect().height());
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
        
        loadOrientedWindow();
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
    pthread_mutex_lock(&mutex);
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
    pthread_mutex_unlock(&mutex);
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
                                            "border-image: url(:/libicons/img/Chess.png);"
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
        int decimal = getVarDecimal(pens[actualPen].CtIndex);
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
