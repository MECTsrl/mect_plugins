#ifndef ATCMGRAPH_H
#define ATCMGRAPH_H

#include <QtGui/QWidget>
#include <QLocale>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QTimer>
#include <QTime>
#include <QGridLayout>
#include <QMutex>

#define FRAMEPLOT
#ifdef FRAMEPLOT
#include <QFrame>
#define MAINWIGET QFrame
#define PLOT plot
#else
#define MAINWIGET QwtPlot
#define PLOT this
#endif

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_legend.h>

#define MAX_SAMPLES 1024

class ATCMInterruptedCurve : public QwtPlotCurve
{
protected:
virtual void drawCurve( QPainter *p, int style, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect, int from, int to ) const;
};

class TimeScaleDraw: public QwtScaleDraw
{
public:
    TimeScaleDraw(const QTime &base):
        baseTime(base)
    {
    }
    virtual QwtText label(double v) const
    {
        QTime upTime = baseTime.addSecs((int)v);
        //fprintf(stderr, "%s + %f = %s\n", baseTime.toString().toAscii().data(), v, upTime.toString().toAscii().data());
        return upTime.toString();
        //return QString("%1").arg(v);
    }
    void setBaseTime(const QTime &base)
    {
        baseTime = base;
    }
private:
    QTime baseTime;
};

class NormalScaleDraw: public QwtScaleDraw
{
public:
    NormalScaleDraw(const int &decimals = 0)
    {
        setDecimalNb(decimals);
    }
    virtual QwtText label(double v) const
    {
        //fprintf(stderr, "%s + %f = %s\n", baseTime.toString().toAscii().data(), v, upTime.toString().toAscii().data());
        return QString().setNum(v,'f',decimalNb);
    }
    void setDecimalNb(const int &decimals)
    {
        decimalNb = decimals;
    }
    int getDecimalNb()
    {
        return decimalNb;
    }
private:
    int decimalNb;
};
#if 0
namespace descriptor
{
class atcm_axisDescriptor
{
public:
    atcm_axisDescriptor()
    {
        this->label = "label";
    }

    atcm_axisDescriptor(const atcm_axisDescriptor &other)
    {
        label = other.label;
    }

    ~atcm_axisDescriptor()
    {
    }

private:
     QString label;
    QColor penColor;
    QString Variable;
    bool visible;
    bool tickLabelVisible;
    double Min;
    double Max;
    double Step;
};
}

//using namespace descriptor;
Q_DECLARE_METATYPE(descriptor::atcm_axisDescriptor)
#endif

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMgraph : public MAINWIGET
{
    Q_OBJECT
#ifndef TARGET_ARM
    /************* property to hide *************/
	Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled DESIGNABLE false)
    // Q_PROPERTY(QSizePolicy sizePolicy READ sizePolicy WRITE setSizePolicy DESIGNABLE false)
    Q_PROPERTY(bool mouseTracking READ hasMouseTracking WRITE setMouseTracking DESIGNABLE false)
    Q_PROPERTY(QPalette palette READ palette WRITE setPalette DESIGNABLE false)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip DESIGNABLE false)
    Q_PROPERTY(QCursor cursor READ cursor WRITE setCursor DESIGNABLE false)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis DESIGNABLE false)
    Q_PROPERTY(QSize baseSize READ baseSize WRITE setBaseSize DESIGNABLE false)
#ifdef _WIN32
    Q_PROPERTY(QString accessibleName READ accessibleName WRITE setAccessibleName DESIGNABLE false)
    Q_PROPERTY(QString accessibleDescription READ accessibleDescription WRITE setAccessibleDescription DESIGNABLE false)
#endif
    Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection DESIGNABLE false)
    Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement DESIGNABLE false)
    Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy DESIGNABLE false)
    Q_PROPERTY(Qt::ContextMenuPolicy contextMenuPolicy READ contextMenuPolicy WRITE setContextMenuPolicy DESIGNABLE false)
    Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops DESIGNABLE false)
    Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip DESIGNABLE false)
    Q_PROPERTY(bool autoFillBackground READ autoFillBackground WRITE setAutoFillBackground DESIGNABLE false)
	Q_PROPERTY(QLocale locale READ locale WRITE setLocale DESIGNABLE false)
    Q_PROPERTY(Qt::InputMethodHints inputMethodHints READ inputMethodHints WRITE setInputMethodHints DESIGNABLE false)
    Q_PROPERTY(Shape frameShape READ frameShape WRITE setFrameShape DESIGNABLE false)
    Q_PROPERTY(Shadow frameShadow READ frameShadow WRITE setFrameShadow DESIGNABLE false)
    Q_PROPERTY(int lineWidth READ lineWidth WRITE setLineWidth DESIGNABLE false)
    Q_PROPERTY(int midLineWidth READ midLineWidth WRITE setMidLineWidth DESIGNABLE false)

    /************* new property ************ */
    Q_ENUMS(ATCMAxisFormat)
#if 0
    /* name of the variable to assign at x axes */
    Q_PROPERTY(descriptor::atcm_axisDescriptor x1axis READ x1axis WRITE setX1axis USER true DESIGNABLE true)
#endif
    /*
        PEN 1
    */
    /* name of the variable to assign at x axes */
    Q_PROPERTY(QString x1Variable READ x1Variable WRITE setX1Variable RESET unsetX1Variable)
    /* min x value */
    Q_PROPERTY(QString x1Min READ x1Min WRITE setX1Min RESET unsetX1Min)
    /* max x value */
    Q_PROPERTY(QString x1Max READ x1Max WRITE setX1Max RESET unsetX1Max)
    /* step x value */
    Q_PROPERTY(double x1Step READ x1Step WRITE setX1Step RESET unsetX1Step)
    /* x value visiblity */
    Q_PROPERTY(bool x1TickLabelVisible READ x1TickLabelVisible WRITE setX1TickLabelVisible RESET unsetX1TickLabelVisible)
    /* x value format */
    Q_ENUMS(ATCMAxisFormat)
    Q_PROPERTY(ATCMAxisFormat x1TickLabelFormat READ x1TickLabelFormat WRITE setX1TickLabelFormat RESET unsetX1TickLabelFormat)
    /* x label */
    Q_PROPERTY(QString x1Label READ x1Label WRITE setX1Label RESET unsetX1Label)
    /* name of the variable to assign at y axes */
    Q_PROPERTY(QString y1Variable READ y1Variable WRITE setY1Variable RESET unsetY1Variable)
    /* min y value */
    Q_PROPERTY(QString y1Min READ y1Min WRITE setY1Min RESET unsetY1Min)
    /* max y value */
    Q_PROPERTY(QString y1Max READ y1Max WRITE setY1Max RESET unsetY1Max)
    /* step y value */
    Q_PROPERTY(double y1Step READ y1Step WRITE setY1Step RESET unsetY1Step)
    /* y value visiblity */
    Q_PROPERTY(bool y1TickLabelVisible READ y1TickLabelVisible WRITE setY1TickLabelVisible RESET unsetY1TickLabelVisible)
    /* y value format */
    Q_PROPERTY(ATCMAxisFormat y1TickLabelFormat READ y1TickLabelFormat WRITE setY1TickLabelFormat RESET unsetY1TickLabelFormat)
    /* y label */
    Q_PROPERTY(QString y1Label READ y1Label WRITE setY1Label RESET unsetY1Label RESET unsetY1Label)
    /* set pen */
    Q_PROPERTY(QColor pen1Color READ pen1Color WRITE setPen1Color)
    /* display trigger */
    Q_PROPERTY(QString Display1 READ Display1 WRITE setDisplay1 RESET unsetDisplay1)
    /*
        PEN 2
    */
    /* name of the variable to assign at x axes */
    Q_PROPERTY(QString x2Variable READ x2Variable WRITE setX2Variable RESET unsetX2Variable)
    /* min x value */
    Q_PROPERTY(QString x2Min READ x2Min WRITE setX2Min RESET unsetX2Min)
    /* max x value */
    Q_PROPERTY(QString x2Max READ x2Max WRITE setX2Max RESET unsetX2Max)
    /* step x value */
    Q_PROPERTY(double x2Step READ x2Step WRITE setX2Step RESET unsetX2Step)
    /* x value visiblity */
    Q_PROPERTY(bool x2TickLabelVisible READ x2TickLabelVisible WRITE setX2TickLabelVisible RESET unsetX2TickLabelVisible)
    /* x value format */
    Q_PROPERTY(ATCMAxisFormat x2TickLabelFormat READ x2TickLabelFormat WRITE setX2TickLabelFormat RESET unsetX2TickLabelFormat)
    /* x label */
    Q_PROPERTY(QString x2Label READ x2Label WRITE setX2Label RESET unsetX2Label)
    /* name of the variable to assign at y axes */
    Q_PROPERTY(QString y2Variable READ y2Variable WRITE setY2Variable RESET unsetY2Variable)
    /* min y value */
    Q_PROPERTY(QString y2Min READ y2Min WRITE setY2Min RESET unsetY2Min)
    /* max x value */
    Q_PROPERTY(QString y2Max READ y2Max WRITE setY2Max RESET unsetY2Max)
    /* step y value */
    Q_PROPERTY(double y2Step READ y2Step WRITE setY2Step RESET unsetY2Step)
    /* y value visiblity */
    Q_PROPERTY(bool y2TickLabelVisible READ y2TickLabelVisible WRITE setY2TickLabelVisible RESET unsetY2TickLabelVisible)
    /* y value format */
    Q_PROPERTY(ATCMAxisFormat y2TickLabelFormat READ y2TickLabelFormat WRITE setY2TickLabelFormat RESET unsetY2TickLabelFormat)
    /* y label */
    Q_PROPERTY(QString y2Label READ y2Label WRITE setY2Label RESET unsetY2Label)
    /* set pen */
    Q_PROPERTY(QColor pen2Color READ pen2Color WRITE setPen2Color)
    /* display trigger */
    Q_PROPERTY(QString Display2 READ Display2 WRITE setDisplay2 RESET unsetDisplay2)
    /*
        CANVAS OPTION
    */
    /* Grid visibility */
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible RESET unsetGridVisible)
    /* Grid color */
    //        Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor)
    /* set if the legend is visible */
    Q_PROPERTY(bool legendVisible READ legendVisible WRITE setLegendVisible RESET unsetLegendVisible)
    /* Title */
    Q_PROPERTY(QString title READ title WRITE setTitle RESET unsetTitle)
    /* refresh time of the crosstable variables */
    Q_PROPERTY(int refresh READ refresh WRITE setRefresh RESET unsetRefresh)
    /* set if the the status of the associated variable have an visible feedback */
    Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
    /* set background color */
    Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
#endif
public:
    enum ATCMAxisFormat
    {
        number,
        date
    };
    ATCMgraph(QWidget *parent = 0);
    ~ATCMgraph();

    QString x1Variable() const { return m_x1Variable; }
    QString x1Min() const {
        if (m_x1MinVariable.length() > 0) return m_x1MinVariable;
        else return QString().setNum(m_x1MinValue,'f');
    }
    QString x1Max() const {
        if (m_x1MaxVariable.length() > 0) return m_x1MaxVariable;
        else return QString().setNum(m_x1MaxValue,'f');
    }
    double x1Step() const { return m_x1step; }
    QString x1Label() const { return m_x1label; }
    bool x1TickLabelVisible() const { return m_x1ticklabelvisible; }
    enum ATCMAxisFormat x1TickLabelFormat() const { return m_x1ticklabelformat; }

    QString y1Variable() const { return m_y1Variable; }
    QString y1Min() const {
        if (m_y1MinVariable.length() > 0) return m_y1MinVariable;
        else return QString().setNum(m_y1MinValue,'f');
    }
    QString y1Max() const {
        if (m_y1MaxVariable.length() > 0) return m_y1MaxVariable;
        else return QString().setNum(m_y1MaxValue,'f');
    }
    double y1Step() const { return m_y1step; }
    QString y1Label() const { return m_y1label; }
    bool y1TickLabelVisible() const { return m_y1ticklabelvisible; }
    enum ATCMAxisFormat y1TickLabelFormat() const { return m_y1ticklabelformat; }

    QString Display1() const { return m_display1; }

    QString x2Variable() const { return m_x2Variable; }
    QString x2Min() const {
        if (m_x2MinVariable.length() > 0) return m_x2MinVariable;
        else return QString().setNum(m_x2MinValue,'f');
    }
    QString x2Max() const {
        if (m_x2MaxVariable.length() > 0) return m_x2MaxVariable;
        else return QString().setNum(m_x2MaxValue,'f');
    }
    double x2Step() const { return m_x2step; }
    QString x2Label() const { return m_x2label; }
    bool x2TickLabelVisible() const { return m_x2ticklabelvisible; }
    enum ATCMAxisFormat x2TickLabelFormat() const { return m_x2ticklabelformat; }

    QString y2Variable() const { return m_y2Variable; }
    QString y2Min() const {
        if (m_y2MinVariable.length() > 0) return m_y2MinVariable;
        else return QString().setNum(m_y2MinValue,'f');
    }
    QString y2Max() const {
        if (m_y2MaxVariable.length() > 0) return m_y2MaxVariable;
        else return QString().setNum(m_y2MaxValue,'f');
    }
    double y2Step() const { return m_y2step; }
    QString y2Label() const { return m_y2label; }
    bool y2TickLabelVisible() const { return m_y2ticklabelvisible; }
    enum ATCMAxisFormat y2TickLabelFormat() const { return m_y2ticklabelformat; }

    QString Display2() const { return m_display2; }

    QColor bgColor() const { return m_bgcolor; }
    QColor pen1Color() const { return m_pen1color; }
    QColor pen2Color() const { return m_pen2color; }
#if 0
    descriptor::atcm_axisDescriptor x1axis() const { return m_x1axis; }
#endif

    bool gridVisible() const { return m_gridvisible; }
    QString title() const { return m_title; }

    int sample1Nb() const { return m_sample_nb1; }
    int sample2Nb() const { return m_sample_nb2; }

    int refresh()      const { return m_refresh; }
    char status()      const { return m_x1status && m_y1status && m_x2status && m_y2status; }
    bool viewStatus()  const { return m_viewstatus; }
    bool legendVisible()  const { return m_legendvisible; }
    bool startAutoReading();
    bool stopAutoReading();
    void RunStop();

public slots:

    bool setX1Variable(QString x1variable);
    bool setX1Min(QString variable);
    bool setX1Max(QString variable);
    void setX1Step(double x1Step);
    void setX1Label(QString x1Label);
    void setX1TickLabelVisible(bool visible);
    void setX1TickLabelFormat(enum ATCMAxisFormat x1TickLabelFormat);

    bool setY1Variable(QString variable);
    bool setY1Min(QString variable);
    bool setY1Max(QString variable);
    void setY1Step(double y1Step);
    void setY1Label(QString y1Label);
    void setY1TickLabelVisible(bool visible);
    void setY1TickLabelFormat(enum ATCMAxisFormat y1TickLabelFormat);

    bool setX2Variable(QString variable);
    bool setX2Min(QString variable);
    bool setX2Max(QString variable);
    void setX2Step(double x2Step);
    void setX2Label(QString x2Label);
    void setX2TickLabelVisible(bool visible);
    void setX2TickLabelFormat(enum ATCMAxisFormat x2TickLabelFormat);

    bool setY2Variable(QString y2variable);
    bool setY2Min(QString variable);
    bool setY2Max(QString variable);
    void setY2Step(double y2Step);
    void setY2Label(QString y2Label);
    void setY2TickLabelVisible(bool visible);
    void setY2TickLabelFormat(enum ATCMAxisFormat y2TickLabelFormat);

    void setBgColor(QColor bgcolor);
    void setPen1Color(QColor pencolor);
    void setPen2Color(QColor pencolor);
    void setTitle(QString title);
    void setGridVisible(bool visible);

#if 0
    void setX1axis(descriptor::atcm_axisDescriptor xaxis);
#endif

    bool setRefresh(int);
    bool setViewStatus(bool);
    void setLegendVisible(bool);
#ifdef TARGET_ARM
    bool addSample(double *samples_x, double *samples_y, double x, double y, double min_x, double max_x, int * sample);
    bool readData(int CtIndex, QString variable, double * value);
#endif
    void setDisplay1(QString);
    void setDisplay2(QString);

    QSize sizeHint() const;

    /* name of the variable to assign at x axes */
    void unsetX1Variable();
    void unsetX1Min();
    void unsetX1Max();
    void unsetX1Step();
    void unsetX1TickLabelVisible();
    void unsetX1TickLabelFormat();
    void unsetX1Label();
    void unsetY1Variable();
    void unsetY1Min();
    void unsetY1Max();
    void unsetY1Step();
    void unsetY1TickLabelVisible();
    void unsetY1TickLabelFormat();
    void unsetY1Label();
    void unsetX2Variable();
    void unsetX2Min();
    void unsetX2Max();
    void unsetX2Step();
    void unsetX2TickLabelVisible();
    void unsetX2TickLabelFormat();
    void unsetX2Label();
    void unsetY2Variable();
    void unsetY2Min();
    void unsetY2Max();
    void unsetY2Step();
    void unsetY2TickLabelVisible();
    void unsetY2TickLabelFormat();
    void unsetY2Label();
    void unsetGridVisible();
    void unsetLegendVisible();
    void unsetTitle();
    void unsetRefresh();
    void unsetViewStatus();
    void unsetDisplay1();
    void unsetDisplay2();
protected slots:
    void updateData();

protected:
#ifdef FRAMEPLOT
    QwtPlot * plot;
#endif
    int m_refresh;
    bool m_viewstatus;
    bool m_legendvisible;
    QwtPlotGrid *grid;

    QwtPlotCurve *curve1;
    QString m_display1;
    double m_trigger1;
    int m_CtDisplay1Index;

    double m_x1step;
    QString m_x1label;
    bool m_x1ticklabelvisible;
    enum ATCMAxisFormat m_x1ticklabelformat;
    char m_x1status;

    QString m_x1Variable;
    double *m_x1ArrayValue;
    int m_CtX1Index;

    QString m_x1MinVariable;
    double m_x1MinValue;
    int m_CtX1MinIndex;

    QString m_x1MaxVariable;
    double m_x1MaxValue;
    int m_CtX1MaxIndex;

    double m_y1step;
    QString m_y1label;
    bool m_y1ticklabelvisible;
    enum ATCMAxisFormat m_y1ticklabelformat;
    char m_y1status;

    QString m_y1Variable;
    double * m_y1ArrayValue;
    int m_CtY1Index;

    QString m_y1MinVariable;
    double m_y1MinValue;
    int m_CtY1MinIndex;

    QString m_y1MaxVariable;
    double m_y1MaxValue;
    int m_CtY1MaxIndex;

    int m_current_sample1;
    int m_sample_nb1;

    QwtPlotCurve *curve2;
    char m_y2status;

    double m_x2step;
    QString m_x2label;
    bool m_x2ticklabelvisible;
    enum ATCMAxisFormat m_x2ticklabelformat;
    char m_x2status;

    QString m_x2Variable;
    double *m_x2ArrayValue;
    int m_CtX2Index;

    QString m_x2MinVariable;
    double m_x2MinValue;
    int m_CtX2MinIndex;

    QString m_x2MaxVariable;
    double m_x2MaxValue;
    int m_CtX2MaxIndex;

    double m_y2step;
    QString m_y2label;
    bool m_y2ticklabelvisible;
    enum ATCMAxisFormat m_y2ticklabelformat;
    QString m_display2;
    int m_CtDisplay2Index;
    double m_trigger2;

    QString m_y2Variable;
    double *m_y2ArrayValue;
    int m_CtY2Index;

    QString m_y2MinVariable;
    double m_y2MinValue;
    int m_CtY2MinIndex;

    QString m_y2MaxVariable;
    double m_y2MaxValue;
    int m_CtY2MaxIndex;

    int m_current_sample2;
    int m_sample_nb2;

    QColor m_pen1color;
    QColor m_pen2color;

    QColor m_bgcolor;

    QString m_title;
    bool m_gridvisible;

#ifdef FRAMEPLOT
    QGridLayout *mainLayout;
#endif

    QwtLegend *legend;
    QRect legendRect;
#if 0
    descriptor::atcm_axisDescriptor m_x1axis;
#endif

#if 0
protected:
	void paintEvent(QPaintEvent *event);
#endif

private:
    bool setVariable(QString variable, QString * destination, int * CtIndex);
#ifdef TARGET_ARM
    char readVariable(QString variable, int CtIndex, double * value);
#endif
private:
    QTimer * refresh_timer;
    bool m_run_stop;
    QMutex sample_mutex;
};

#endif
