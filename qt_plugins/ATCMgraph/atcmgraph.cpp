/**
 * @file
 * @author Emiliano Bergamaschini <ebergamaschini@mect.it>
 *
 * @section LICENSE
 * Copyright Mect s.r.l. 2013
 *
 * @brief ATCM graph base class
 */
#include <QPainter>
#include <QBoxLayout>
#include <QStyleOption>
#include <QMessageBox>

#include "atcmgraph.h"
#include "common.h"
#ifdef TARGET_ARM
#include "app_logprint.h"
#include "cross_table_utility.h"
#endif

ATCMgraph::ATCMgraph(QWidget *parent) :
    MAINWIGET(parent)
{
    m_refresh = DEFAULT_PLUGIN_REFRESH;
    m_x1Variable = "PLC_time";
    m_CtX1Index = 5390;
    m_x1status = UNK;
    m_x1MinVariable = "PLC_timeMin";
    m_CtX1MinIndex = 5391;
    m_x1MinValue = 0;
    m_x1MaxVariable = "PLC_timeMax";
    m_CtX1MaxIndex = 5392;
    m_x1MaxValue = 100;
    m_x1step = 10;
    m_x1label = "";
    m_x1ticklabelvisible = false;

    m_y1Variable = "";
    m_CtY1Index = -1;
    m_y1status = UNK;
    m_y1MinVariable = "0";
    m_CtY1MinIndex = -1;
    m_y1MinValue = 0;
    m_y1MaxVariable = "100";
    m_CtY1MaxIndex = -1;
    m_y1MaxValue = 100;
    m_y1step = 10;
    m_y1label = "";
    m_y1ticklabelvisible = false;

    m_display1 = "";
    m_sample_nb1 = 0;
    m_current_sample1 = 0;

    m_x2Variable = "PLC_time";
    m_CtX2Index = 5390;
    m_x2status = UNK;
    m_x2MinVariable = "PLC_timeMin";
    m_CtX2MinIndex = 5391;
    m_x2MinValue = 0;
    m_x2MaxVariable = "PLC_timeMax";
    m_CtX2MaxIndex = 5392;
    m_x2MaxValue = 100;
    m_x2step = 10;
    m_x2label = "";
    m_x2ticklabelvisible = false;

    m_y2Variable = "";
    m_CtY2Index = -1;
    m_y2status = UNK;
    m_y2MinVariable = "0";
    m_CtY2MinIndex = -1;
    m_y2MinValue = 0;
    m_y2MaxVariable = "100";
    m_CtY2MaxIndex = -1;
    m_y2MaxValue = 100;
    m_y2step = 10;
    m_y2label = "";
    m_y2ticklabelvisible = false;

    m_display2 = "";
    m_sample_nb2 = 0;
    m_current_sample2 = 0;

    m_gridvisible = true;
    m_legendvisible = false;
    m_bgcolor = QColor(255,255,255);
    m_pen1color = QColor(255,0,0);
    m_pen2color = QColor(0,255,0);

    m_run_stop = true;

    m_x1ArrayValue = (double*)calloc(MAX_SAMPLES + 1, sizeof (double));
    m_x2ArrayValue = (double*)calloc(MAX_SAMPLES + 1, sizeof (double));
    m_y1ArrayValue = (double*)calloc(MAX_SAMPLES + 1, sizeof (double));
    m_y2ArrayValue = (double*)calloc(MAX_SAMPLES + 1, sizeof (double));

    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        m_x1ArrayValue[i] = VAR_NAN;
        m_y1ArrayValue[i] = VAR_NAN;
        m_x2ArrayValue[i] = VAR_NAN;
        m_y2ArrayValue[i] = VAR_NAN;
    }

    m_x2step = 20;
    m_x1step = 20;
    m_y1step = 20;
    m_y2step = 20;

    //setMinimumSize(QSize(150,50));
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //setGeometry(0,0,sizeHint().width(),sizeHint().height());

#ifdef FRAMEPLOT
    PLOT = new QwtPlot(this);
    //PLOT->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //PLOT->setGeometry(0,0,sizeHint().width(),sizeHint().height());
    mainLayout = new QGridLayout;
    mainLayout->setMargin(1);
    setLayout(mainLayout);

    mainLayout->addWidget(PLOT,0,0);
#else
#warning PLOT
#endif

    //setFixedSize(sizeHint());

    /* setup the grid */
    grid = new QwtPlotGrid;
    grid->enableXMin(true);
    grid->enableYMin(true);
#if QWT_VERSION >= 0x060100
    grid->setMajorPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinorPen(QPen(Qt::gray, 0 , Qt::DotLine));
#else
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::gray, 0 , Qt::DotLine));
#endif
    grid->attach(PLOT);

#ifdef TARGET_ARM
    curve1 = new ATCMInterruptedCurve();
#else
    curve1 = new QwtPlotCurve();
#endif
    curve1->attach(PLOT);
    curve1->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
    //curve1->setCurveAttribute(QwtPlotCurve::Fitted);

#ifdef TARGET_ARM
    curve2 = new ATCMInterruptedCurve();
#else
    curve2 = new QwtPlotCurve();
#endif
    curve2->attach(PLOT);
    curve2->setAxes(QwtPlot::xTop, QwtPlot::yRight);
    //curve2->setCurveAttribute(QwtPlotCurve::Fitted);

    PLOT->setAxisAutoScale(QwtPlot::xBottom,true);
    PLOT->setAxisAutoScale(QwtPlot::yLeft,true);
    PLOT->setAxisAutoScale(QwtPlot::xTop,true);
    PLOT->setAxisAutoScale(QwtPlot::yRight,true);

    PLOT->setAxisScale(QwtPlot::xBottom, m_x1MinValue, m_x1MaxValue, m_x1step);
    PLOT->setAxisScale(QwtPlot::yLeft, m_y1MinValue, m_y1MaxValue, m_y1step);
    PLOT->setAxisScale(QwtPlot::xTop, m_x2MinValue, m_x2MaxValue, m_x2step);
    PLOT->setAxisScale(QwtPlot::yRight, m_y2MinValue, m_y2MaxValue, m_y2step);

    PLOT->enableAxis(QwtPlot::xBottom, m_x1ticklabelvisible);
    PLOT->enableAxis(QwtPlot::yLeft, m_y1ticklabelvisible);
    PLOT->enableAxis(QwtPlot::xTop, m_x2ticklabelvisible);
    PLOT->enableAxis(QwtPlot::yRight, m_y2ticklabelvisible);

    PLOT->updateAxes();

    curve1->setPen(QPen(m_pen1color,2));
    curve2->setPen(QPen(m_pen2color,2));

#ifndef TARGET_ARM
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        m_x1ArrayValue[i] = m_x2ArrayValue[i] = i;
        m_y1ArrayValue[i] = 50 + 50 * sin((double)i/10);
        m_y2ArrayValue[i] = 50 + 50 * cos((double)i/10);
    }
    curve1->setRawSamples(m_x1ArrayValue, m_y1ArrayValue, MAX_SAMPLES);
    curve2->setRawSamples(m_x2ArrayValue, m_y2ArrayValue, MAX_SAMPLES);

    PLOT->replot();
#endif

    /*
     * put there a default stylesheet
     *label->setStyleSheet("padding: 5px;");
     */
#ifdef TARGET_ARM
    if (m_refresh > 0)
    {
        refresh_timer = new QTimer(this);
        connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));
        refresh_timer->start(m_refresh);
    }
    else
#endif
    {
        refresh_timer = NULL;
    }
}

QSize ATCMgraph::sizeHint() const
{
    return QSize(200, 100);
}

ATCMgraph::~ATCMgraph()
{
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        delete refresh_timer;
    }
    free(m_x1ArrayValue);
    m_x1ArrayValue = NULL;
    free(m_x2ArrayValue);
    m_x2ArrayValue = NULL;
    free(m_y1ArrayValue);
    m_y1ArrayValue = NULL;
    free(m_y2ArrayValue);
    m_y2ArrayValue = NULL;
}

#if 0
void ATCMgraph::paintEvent(QPaintEvent * e)
{
    Q_UNUSED( e )

#if 0
    QPainter p(this);

    if (m_viewstatus)
    {
        /* draw the background color in funtion of the status */
        QBrush brush(Qt::red);
        if (m_x1status == DONE && m_y1status == DONE && m_x2status == DONE && m_y2status == DONE)
        {
            brush.setColor(Qt::green);
        }
        if (m_x1status == BUSY || m_y1status == BUSY || m_x2status == BUSY || m_y2status == BUSY)
        {
            brush.setColor(Qt::yellow);
        }
        if (m_x1status == ERROR || m_y1status == ERROR || m_x2status == ERROR || m_y2status == ERROR)
        {
            brush.setColor(Qt::red);
        }

        p.setBrush(brush);
        p.drawRect(this->rect());
    }
#if 0
    /* propagate the stylesheet set by QtCreator */
    QStyleOption opt;
    opt.init(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
#endif
    /* propagate the paint event to the parent widget */
    paintEvent(e);
#endif
}
#endif

/* Activate variable */
bool ATCMgraph::setX1Variable(QString variable)
{
    if (setVariable(variable, &m_x1Variable, &m_CtX1Index) == false)
    {
        m_x1status = ERROR;
    }
    else
    {
        if (m_x1label.length() == 0)
        {
            setX1Label(variable);
        }
    }
    return (m_x1status != ERROR);
}

bool ATCMgraph::setX1Min(QString variable)
{
    if (setVariable(variable, &m_x1MinVariable, &m_CtX1MinIndex) == false)
    {
        m_x1status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::xBottom, m_x1MinValue, m_x1MaxValue, m_x1step);
    return (m_x1status != ERROR);
}

bool ATCMgraph::setX1Max(QString variable)
{
    if (setVariable(variable, &m_x1MaxVariable, &m_CtX1MaxIndex) == false)
    {
        m_x1status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::xBottom, m_x1MinValue, m_x1MaxValue, m_x1step);
    return (m_x1status != ERROR);
}

void ATCMgraph::setX1Step(double x1Step)
{
    m_x1step = x1Step;
    PLOT->setAxisScale(QwtPlot::xBottom, m_x1MinValue, m_x1MaxValue, m_x1step);
    PLOT->updateAxes();
}

void ATCMgraph::setX1Label(QString x1Label)
{
    m_x1label = x1Label;
    PLOT->setAxisTitle(QwtPlot::xBottom, m_x1label);
}

/* Activate variable */
bool ATCMgraph::setY1Variable(QString variable)
{
    if (setVariable(variable, &m_y1Variable, &m_CtY1Index) == false)
    {
        m_y1status = ERROR;
    }
    else
    {
        if (m_y1label.length() == 0)
        {
            setY1Label(variable);
        }
    }
    return (m_y1status != ERROR);
}

bool ATCMgraph::setY1Min(QString variable)
{
    if (setVariable(variable, &m_y1MinVariable, &m_CtY1MinIndex) == false)
    {
        m_y1status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::yLeft, m_y1MinValue, m_y1MaxValue, m_y1step);
    return (m_y1status != ERROR);
}

bool ATCMgraph::setY1Max(QString variable)
{
    if (setVariable(variable, &m_y1MaxVariable, &m_CtY1MaxIndex) == false)
    {
        m_y1status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::yLeft, m_y1MinValue, m_y1MaxValue, m_y1step);
    return (m_y1status != ERROR);
}

void ATCMgraph::setY1Step(double y1Step)
{
    m_y1step = y1Step;
    PLOT->setAxisScale(QwtPlot::yLeft, m_y1MinValue, m_y1MaxValue, m_y1step);
    PLOT->updateAxes();
}

void ATCMgraph::setY1Label(QString y1Label)
{
    m_y1label = y1Label;
    curve1->setTitle(m_y1label);
    PLOT->setAxisTitle(QwtPlot::yLeft, m_y1label);
    setLegendVisible(m_legendvisible);
}

/* Activate variable */
bool ATCMgraph::setX2Variable(QString variable)
{
    if (setVariable(variable, &m_x2Variable, &m_CtX2Index) == false)
    {
        m_x2status = ERROR;
    }
    else
    {
        if (m_x2label.length() == 0)
        {
            setX2Label(variable);
        }
    }
    return (m_x2status != ERROR);
}

bool ATCMgraph::setX2Min(QString variable)
{
    if (setVariable(variable, &m_x2MinVariable, &m_CtX2MinIndex) == false)
    {
        m_x2status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::xTop, m_x2MinValue, m_x2MaxValue, m_x2step);
    return (m_x2status != ERROR);
}

bool ATCMgraph::setX2Max(QString variable)
{
    if (setVariable(variable, &m_x2MaxVariable, &m_CtX2MaxIndex) == false)
    {
        m_x2status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::xTop, m_x2MinValue, m_x2MaxValue, m_x2step);
    return (m_x2status != ERROR);
}

void ATCMgraph::setX2Step(double x2Step)
{
    m_x2step = x2Step;
    PLOT->setAxisScale(QwtPlot::xTop, m_x2MinValue, m_x2MaxValue, m_x2step);
    PLOT->updateAxes();
}

void ATCMgraph::setX2Label(QString x2Label)
{
    m_x2label = x2Label;
    PLOT->setAxisTitle(QwtPlot::xTop, m_x2label);
}

/* Activate variable */
bool ATCMgraph::setY2Variable(QString variable)
{
    if (setVariable(variable, &m_y2Variable, &m_CtY2Index) == false)
    {
        m_y2status = ERROR;
    }
    else
    {
        if (m_y2label.length() == 0)
        {
            setY2Label(variable);
        }
    }
    return (m_y2status != ERROR);
}

bool ATCMgraph::setY2Min(QString variable)
{
    if (setVariable(variable, &m_y2MinVariable, &m_CtY2MinIndex) == false)
    {
        m_y2status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::yRight, m_y2MinValue, m_y2MaxValue, m_y2step);
    return (m_y2status != ERROR);
}

bool ATCMgraph::setY2Max(QString variable)
{
    if (setVariable(variable, &m_y2MaxVariable, &m_CtY2MaxIndex) == false)
    {
        m_y2status = ERROR;
    }
    PLOT->setAxisScale(QwtPlot::yRight, m_y2MinValue, m_y2MaxValue, m_y2step);
    return (m_y2status != ERROR);
}

void ATCMgraph::setY2Step(double y2Step)
{
    m_y2step = y2Step;
    PLOT->setAxisScale(QwtPlot::yRight, m_y2MinValue, m_y2MaxValue, m_y2step);
    PLOT->updateAxes();
}

void ATCMgraph::setY2Label(QString y2Label)
{
    m_y2label = y2Label;
    curve2->setTitle(m_y2label);
    PLOT->setAxisTitle(QwtPlot::yRight, m_y2label);
    setLegendVisible(m_legendvisible);
}

bool ATCMgraph::setRefresh(int refresh)
{
    m_refresh = refresh;
    if (refresh_timer == NULL && m_refresh > 0)
    {
        refresh_timer = new QTimer(this);

        connect(refresh_timer, SIGNAL(timeout()), this, SLOT(updateData()));

        refresh_timer->start(m_refresh);
    }
    else if (m_refresh > 0)
    {
        refresh_timer->start(m_refresh);
    }
    else if (refresh_timer != NULL)
    {
        refresh_timer->stop();
    }
    return true;
}

void ATCMgraph::setTitle(QString title)
{
    m_title = title;
    PLOT->setTitle(title);
}

void ATCMgraph::setPen1Color(QColor penColor)
{
    m_pen1color = penColor;
    curve1->setPen(QPen(m_pen1color,2));
    PLOT->replot();
}

void ATCMgraph::setPen2Color(QColor penColor)
{
    m_pen2color = penColor;
    curve2->setPen(QPen(m_pen2color,2));
    PLOT->replot();
}

void ATCMgraph::setBgColor(QColor bgColor)
{
    m_bgcolor = bgColor;
    PLOT->setAutoFillBackground( true );
    PLOT->setCanvasBackground(QBrush(m_bgcolor));
    PLOT->replot();
}

/* read variable */
void ATCMgraph::updateData()
{
    bool pen1 = true;
    bool pen2 = true;

    if (PLOT->isVisible() == false)
    {
        return;
    }

#ifdef  TARGET_ARM
    /* read data */
    double x1 = VAR_NAN, y1 = VAR_NAN;
    pen1 = false;
    if (readData(m_CtX1Index, m_x1Variable, &x1)
            && readData(m_CtY1Index, m_y1Variable, &y1)
            && readData(m_CtX1MinIndex, m_x1MinVariable, &m_x1MinValue)
            && readData(m_CtX1MaxIndex, m_x1MaxVariable, &m_x1MaxValue)
            && readData(m_CtY1MinIndex, m_y1MinVariable, &m_y1MinValue)
            && readData(m_CtY1MaxIndex, m_y1MaxVariable, &m_y1MaxValue))
    {
        pen1 = true;
    }

    double x2 = VAR_NAN, y2 = VAR_NAN;
    pen2 = false;
    if (readData(m_CtX2Index, m_x2Variable, &x2)
            && readData(m_CtY2Index, m_y2Variable, &y2)
            && readData(m_CtX2MinIndex, m_x2MinVariable, &m_x2MinValue)
            && readData(m_CtX2MaxIndex, m_x2MaxVariable, &m_x2MaxValue)
            && readData(m_CtY2MinIndex, m_y2MinVariable, &m_y2MinValue)
            && readData(m_CtY2MaxIndex, m_y2MaxVariable, &m_y2MaxValue))
    {
        pen2 = true;
    }

    /* add sample */
    if (pen1 == false || addSample(m_x1ArrayValue, m_y1ArrayValue, x1, y1, m_x1MinValue, m_x1MaxValue, &m_current_sample1) == false)
    {
        pen1 = false;
        LOG_PRINT(error_e, "Problem PEN 1\n");
    }
    else
#endif
    {
        curve1->setRawSamples(m_x1ArrayValue, m_y1ArrayValue, MAX_SAMPLES);
        PLOT->setAxisScale(QwtPlot::xBottom, m_x1MinValue, m_x1MaxValue, m_x1step);
        PLOT->setAxisScale(QwtPlot::yLeft, m_y1MinValue, m_y1MaxValue, m_y1step);
    }
#ifdef  TARGET_ARM
    if (pen2 == false || addSample(m_x2ArrayValue, m_y2ArrayValue, x2, y2, m_x2MinValue, m_x2MaxValue, &m_current_sample2) == false)
    {
        pen2 = false;
        LOG_PRINT(error_e, "Problem PEN 2\n");
    }
    else
#endif
    {
        curve2->setRawSamples(m_x2ArrayValue, m_y2ArrayValue, MAX_SAMPLES);
        PLOT->setAxisScale(QwtPlot::xTop, m_x2MinValue, m_x2MaxValue, m_x2step);
        PLOT->setAxisScale(QwtPlot::yRight, m_y2MinValue, m_y2MaxValue, m_y2step);
    }

    /* show data */
    if (pen1 != false || pen2 != false )
    {
        PLOT->updateAxes();
        PLOT->replot();
    }
}

bool ATCMgraph::startAutoReading()
{
    if (refresh_timer != NULL && m_refresh > 0)
    {
        refresh_timer->start(m_refresh);
        return true;
    }
    return false;
}

bool ATCMgraph::stopAutoReading()
{
    if (refresh_timer != NULL)
    {
        refresh_timer->stop();
        return true;
    }
    return false;
}

bool ATCMgraph::setViewStatus(bool status)
{
    return (m_viewstatus = status);
}

void ATCMgraph::setLegendVisible(bool status)
{
    if (status)
    {
        //legend->setGeometry(QRect());
        legend = new QwtLegend;
        legend->setFrameStyle(QFrame::Box|QFrame::Sunken);
        PLOT->insertLegend(legend, QwtPlot::BottomLegend);
    }
    else
    {
        //legend->setGeometry(legendRect);

        PLOT->insertLegend( NULL );
    }
    m_legendvisible = status;
}

#ifdef TARGET_ARM
bool ATCMgraph::addSample(double *samples_x, double *samples_y, double x, double y, double min_x, double max_x, int * sample)
{
    if (samples_x != NULL && samples_y != NULL)
    {
        double temp_x[MAX_SAMPLES];
        double temp_y[MAX_SAMPLES];

        sample_mutex.lock();

        /* if the new sample is in bound add it */
        int j = MAX_SAMPLES - 1;
        if (x != VAR_NAN && min_x <= x && x <= max_x) {
            temp_x[j] = x;
            temp_y[j] = y;
            --j;
        }

        /* skip the sample out of bound */
        for (int i = MAX_SAMPLES - 1; i >= 0 && j >= 0; --i) {
            if (samples_x[i] != VAR_NAN && min_x <= samples_x[i] && samples_x[i] <= max_x) {
                temp_x[j] = samples_x[i];
                temp_y[j] = samples_y[i];
                --j;
            }
        }
        /* fill the other point with NAN value */
        for (; j >= 0; --j) {
            temp_x[j] = VAR_NAN;
            temp_y[j] = VAR_NAN;
        }

        /* update the pen data */
        memcpy(samples_x, temp_x, MAX_SAMPLES * sizeof(double));
        memcpy(samples_y, temp_y, MAX_SAMPLES * sizeof(double));

        sample_mutex.unlock();

        return true;
    }
    else
    {
        return false;
    }
}
#endif

void ATCMgraph::setGridVisible(bool visible)
{
    m_gridvisible = visible;
    grid->enableX(m_gridvisible);
    grid->enableXMin(m_gridvisible);
    grid->enableY(m_gridvisible);
    grid->enableYMin(m_gridvisible);
    PLOT->replot();
}

void ATCMgraph::setX1TickLabelVisible(bool visible)
{
    m_x1ticklabelvisible = visible;
    PLOT->enableAxis(QwtPlot::xBottom, m_x1ticklabelvisible);
    PLOT->updateAxes();
}

void ATCMgraph::setY1TickLabelVisible(bool visible)
{
    m_y1ticklabelvisible = visible;
    PLOT->enableAxis(QwtPlot::yLeft, m_y1ticklabelvisible);
    PLOT->updateAxes();
}

void ATCMgraph::setX2TickLabelVisible(bool visible)
{
    m_x2ticklabelvisible = visible;
    PLOT->enableAxis(QwtPlot::xTop, m_x2ticklabelvisible);
    PLOT->updateAxes();
}

void ATCMgraph::setY2TickLabelVisible(bool visible)
{
    m_y2ticklabelvisible = visible;
    PLOT->enableAxis(QwtPlot::yRight, m_y2ticklabelvisible);
    PLOT->updateAxes();
}

void ATCMgraph::setX1TickLabelFormat(enum ATCMAxisFormat fmt)
{
    m_x1ticklabelformat = fmt;
    if (m_x1ticklabelformat == date)
    {
        PLOT->setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw(QTime()));
    }
    else
    {
#ifdef TARGET_ARM
        //int decimal =  getVarDecimal(m_x1Variable);
        int decimal =  0;
#else
        int decimal =  0;
#endif
        PLOT->setAxisScaleDraw(QwtPlot::xBottom, new NormalScaleDraw(decimal));
    }
    PLOT->updateAxes();
}

void ATCMgraph::setY1TickLabelFormat(enum ATCMAxisFormat fmt)
{
    m_y1ticklabelformat = fmt;
    if (m_y1ticklabelformat == date)
    {
        PLOT->setAxisScaleDraw(QwtPlot::yLeft, new TimeScaleDraw(QTime()));
    }
    else
    {
#ifdef TARGET_ARM
        //int decimal =  getVarDecimal(m_y1variable);
        int decimal =  0;
#else
        int decimal =  0;
#endif
        PLOT->setAxisScaleDraw(QwtPlot::yLeft, new NormalScaleDraw(decimal));
    }
    PLOT->updateAxes();
}

void ATCMgraph::setX2TickLabelFormat(enum ATCMAxisFormat fmt)
{
    m_x2ticklabelformat = fmt;
    if (m_x2ticklabelformat == date)
    {
        PLOT->setAxisScaleDraw(QwtPlot::xTop, new TimeScaleDraw(QTime()));
    }
    else
    {
#ifdef TARGET_ARM
        //int decimal =  getVarDecimal(m_x2variable);
        int decimal =  0;
#else
        int decimal =  0;
#endif
        PLOT->setAxisScaleDraw(QwtPlot::xTop, new NormalScaleDraw(decimal));
    }
    PLOT->updateAxes();
}

void ATCMgraph::setY2TickLabelFormat(enum ATCMAxisFormat fmt)
{
    m_y2ticklabelformat = fmt;
    if (m_y2ticklabelformat == date)
    {
        PLOT->setAxisScaleDraw(QwtPlot::yRight, new TimeScaleDraw(QTime()));
    }
    else
    {
#ifdef TARGET_ARM
        //int decimal =  getVarDecimal(m_y1variable);
        int decimal =  0;
#else
        int decimal =  0;
#endif
        PLOT->setAxisScaleDraw(QwtPlot::yRight, new NormalScaleDraw(decimal));
    }
    PLOT->updateAxes();
}

void ATCMgraph::unsetX1Variable()
{
    setX1Variable("");
}

void ATCMgraph::unsetX1Min()
{
    setX1Min("0");
}

void ATCMgraph::unsetX1Max()
{
    setX1Max("100");
}

void ATCMgraph::unsetX1Step()
{
    setX1Step(10);
}

void ATCMgraph::unsetX1TickLabelVisible()
{
    setX1TickLabelVisible(true);
}

void ATCMgraph::unsetX1TickLabelFormat()
{
    setX1TickLabelFormat(number);
}

void ATCMgraph::unsetX1Label()
{
    setX1Label("");
}

void ATCMgraph::unsetY1Variable()
{
    setY1Label("");
}

void ATCMgraph::unsetY1Min()
{
    setY1Min("0");
}

void ATCMgraph::unsetY1Max()
{
    setY1Max("100");
}

void ATCMgraph::unsetY1Step()
{
    setY1Step(10);
}

void ATCMgraph::unsetY1TickLabelVisible()
{
    setY1TickLabelVisible(true);
}

void ATCMgraph::unsetY1TickLabelFormat()
{
    setY1TickLabelFormat(number);
}

void ATCMgraph::unsetY1Label()
{
    setY1Label("");
}

void ATCMgraph::unsetX2Variable()
{
    setX2Variable("");
}

void ATCMgraph::unsetX2Min()
{
    setX2Min("0");
}

void ATCMgraph::unsetX2Max()
{
    setX2Max("100");
}

void ATCMgraph::unsetX2Step()
{
    setX2Step(10);
}

void ATCMgraph::unsetX2TickLabelVisible()
{
    setX2TickLabelVisible(true);
}

void ATCMgraph::unsetX2TickLabelFormat()
{
    setX2TickLabelFormat(number);
}

void ATCMgraph::unsetX2Label()
{
    setX2Label("");
}

void ATCMgraph::unsetY2Variable()
{
    setY2Variable("");
}

void ATCMgraph::unsetY2Min()
{
    setY2Min("0");
}

void ATCMgraph::unsetY2Max()
{
    setY2Max("100");
}

void ATCMgraph::unsetY2Step()
{
    setY2Step(10);
}

void ATCMgraph::unsetY2TickLabelVisible()
{
    setY2TickLabelVisible(true);
}

void ATCMgraph::unsetY2TickLabelFormat()
{
    setY2TickLabelFormat(number);
}

void ATCMgraph::unsetY2Label()
{
    setY2Label("");
}

void ATCMgraph::unsetGridVisible()
{
    setGridVisible(true);
}

void ATCMgraph::unsetLegendVisible()
{
    setLegendVisible(false);
}

void ATCMgraph::unsetTitle()
{
    setTitle("");
}

void ATCMgraph::unsetRefresh()
{
    setRefresh(DEFAULT_PLUGIN_REFRESH);
}

void ATCMgraph::unsetViewStatus()
{
    setViewStatus(false);
}

void ATCMgraph::setDisplay1(QString display)
{
    if (setVariable(display, &m_display1, &m_CtDisplay1Index) == false)
    {
        m_display1 = "";
    }
}

void ATCMgraph::unsetDisplay1()
{
    m_display1 = "";
}

void ATCMgraph::setDisplay2(QString display)
{
    if (setVariable(display, &m_display2, &m_CtDisplay2Index) == false)
    {
        m_display2 = "";
    }
}

void ATCMgraph::unsetDisplay2()
{
    m_display2 = "";
}

bool ATCMgraph::setVariable(QString variable, QString * destination, int * CtIndex)
{
    /* reset the value */
    if (variable.trimmed().length() == 0)
    {
        if (destination->length() != 0)
        {
#ifdef TARGET_ARM
            if (deactivateVar(destination->trimmed().toAscii().data()) == 0)
            {
#endif
                destination->clear();
                *CtIndex = -1;
                return true;
#ifdef TARGET_ARM
            }
            else
            {
                return false;
            }
#endif
        }
        return false;
    }

#ifdef TARGET_ARM
    /* check if the variable is a constant number */
    bool valid;
    variable.toDouble(&valid);
    if (valid)
    {
        *CtIndex = -1;
        *destination = variable;
        return true;
    }
    /* activate the variable */
    if (activateVar(variable.trimmed().toAscii().data()) == 0)
    {
        int myCtIndex = -1;
        LOG_PRINT(verbose_e, "extracting ctIndex of '%s'\n", variable.trimmed().toAscii().data());
        if (Tag2CtIndex(variable.trimmed().toAscii().data(), &myCtIndex) != 0)
        {
            LOG_PRINT(error_e,"variable '%s', CtIndex %d\n", destination->toAscii().data(), *CtIndex);
            return false;
        }
        *destination = variable.trimmed();
        *CtIndex = myCtIndex;
        LOG_PRINT(info_e, "'%s' -> ctIndex %d\n", destination->toAscii().data(), *CtIndex);

        return true;
    }
    *CtIndex = -1;
    *destination = variable;
#else
    *CtIndex = -1;
    *destination = variable;
#endif
    return true;
}

#ifdef TARGET_ARM
char ATCMgraph::readVariable(QString variable, int CtIndex, double * valuef)
{
    char value[16];
    if (formattedReadFromDb(CtIndex, value) == 0 && strlen(value) > 0)
    {
        *valuef = atof(value);
        return DONE;
    }
    else
    {
        *valuef = VAR_NAN;
        return ERROR;
        LOG_PRINT(error_e, "Invalid value '%s'\n", value);
    }
}
#if 0
bool ATCMgraph::read4Variables(int CtIndex1, int CtIndex1, int CtIndex1, int CtIndex4,
                               double * valuef1, double * valuef2, double * valuef3, double * valuef4)
{
    char value[16];
    char StatusData = ioDataComm->getStatusIO();
    char StatusSyncro = ioSyncroComm->getStatusIO();
    bool status = true;

    *valuef1 = VAR_NAN;
    *valuef2 = VAR_NAN;
    *valuef3 = VAR_NAN;
    *valuef4 = VAR_NAN;

    if (StatusData == ERROR || StatusSyncro == ERROR)
    {
        LOG_PRINT(error_e, "ERROR PROBLEM INTO SET COMMUNICATION StatusData %X StatusSyncro %X ERROR %X\n", StatusData, StatusSyncro, ERROR);
        return false;
    }
    else if (StatusData == BUSY || StatusSyncro == BUSY)
    {
        LOG_PRINT(info_e, "'%s' StatusData or StatusSyncro id BUSY\n", variable.toAscii().data());
        return false;
    }

    if (CtIndex1 >= 0)
    {
        if (formattedReadFromDb(CtIndex1, value) == 0 && strlen(value) <= 0)
        {
            *valuef1 = VAR_NAN;
            status = false;
        }
    }
    if (CtIndex2 >= 0)
    {
        if (formattedReadFromDb(CtIndex2, value) == 0 && strlen(value) <= 0)
        {
            *valuef2 = VAR_NAN;
            status = false;
        }
    }
    if (CtIndex3 >= 0)
    {
        if (formattedReadFromDb(CtIndex3, value) == 0 && strlen(value) <= 0)
        {
            *valuef3 = VAR_NAN;
            status = false;
        }
    }
    if (CtIndex4 >= 0)
    {
        if (formattedReadFromDb(CtIndex4, value) == 0 && strlen(value) <= 0)
        {
            *valuef4 = VAR_NAN;
            status = false;
        }
    }
    return status;
}
#endif
bool ATCMgraph::readData(int CtIndex, QString variable, double * value)
{
    double myvalue;
    /* it is empty */
    if (variable.length() == 0)
    {
        *value = VAR_NAN;
        return false;
    }
    /* it is a constant value */
    else if (CtIndex < 0)
    {
        bool valid;
        myvalue = variable.toDouble(&valid);
        if (valid)
        {
            *value = myvalue;
            return true;
        }
#ifdef TARGET_ARM
        else
        {
            LOG_PRINT(error_e, "cannot read the variable '%s'\n", variable.toAscii().data());
            *value = VAR_NAN;
            return false;
        }
#endif
    }
#ifdef TARGET_ARM
    /* it is a variable value */
    else if (readVariable(variable, CtIndex, &myvalue) == DONE)
    {
        *value = myvalue;
        return true;
    }
    /* it is a variable value but some error occurred during value reading */
    else
    {
        *value = VAR_NAN;
        return false;
    }
#endif
}

void ATCMgraph::RunStop()
{
    m_run_stop = !m_run_stop;
    if (refresh_timer != NULL)
    {
        if (m_run_stop)
        {
            if (m_refresh > 0)
            {
                refresh_timer->start(m_refresh);
            }
        }
        else
        {
            refresh_timer->stop();
        }
    }
}

void ATCMInterruptedCurve::drawCurve( QPainter *painter, __attribute__((unused))int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to ) const
{
    int preceding_from = from;
    bool is_gap = true;

    // Scan all data to identify gaps
    for (int i = from; i <= to; i++)
    {
#if ( QWT_VERSION >= 0x060001 )
        const QPointF sample = data()->sample(i);
#else
        const QPointF sample = d_series->sample(i);
#endif

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

#endif
