#ifndef ATCMSTYLE_H
#define ATCMSTYLE_H

/* this define enable the example stylesheet */
#undef ENABLE_STYLESHEET

#include <QMotifStyle>
//#include <QWindowsStyle>

//class ATCMStyle : public QWindowsStyle
class ATCMStyle : public QMotifStyle
{
    Q_OBJECT

    public:
        ATCMStyle() {}

        void polish(QPalette &palette);
        void polish(QWidget *widget);
        void unpolish(QWidget *widget);
        int styleHint(StyleHint which, const QStyleOption *option,
                const QWidget *widget = 0,
                QStyleHintReturn *returnData = 0) const;
        int pixelMetric(PixelMetric which, const QStyleOption *option,
                const QWidget *widget = 0) const;
        void drawPrimitive(PrimitiveElement which,
                const QStyleOption *option, QPainter *painter,
                const QWidget *widget = 0) const;
        void drawControl(ControlElement element,
                 const QStyleOption *option, QPainter *painter,
                 const QWidget *widget) const;
        void drawComplexControl(ComplexControl which,
                const QStyleOptionComplex *option,
                QPainter *painter,
                const QWidget *widget = 0) const;
        QRect subControlRect(ComplexControl whichControl,
                const QStyleOptionComplex *option,
                SubControl whichSubControl,
                const QWidget *widget = 0) const;

        public slots:
            QIcon standardIconImplementation(StandardPixmap which,
                    const QStyleOption *option,
                    const QWidget *widget = 0) const;

    private:

        void drawATCMPanelLineEdit(const QStyleOption *option,
            QPainter *painter) const;
        void drawATCMFrameDefaultButton(const QStyleOption *option,
            QPainter *painter) const;
        void drawATCMFrame(const QStyleOption *option,
            QPainter *painter) const;
        void drawATCMVBevel(const QStyleOption *option,
                QPainter *painter) const;
        void drawATCMHBevel(const QStyleOption *option,
                QPainter *painter) const;
        void drawATCMCheckBoxIndicator(const QStyleOption *option,
                QPainter *painter) const;
        void drawATCMSpinBoxButton(SubControl which,
                const QStyleOptionComplex *option,
                QPainter *painter) const;
};

extern int _diameter_;
extern double _penWidth_;
extern QIcon _icon_;

#define BG_COLOR_DEF        QColor(255,255,255)
#define BG_COLOR_SEL_DEF    QColor(230,230,230)
#define FONT_COLOR_DEF      QColor(0,0,0)
#define FONT_COLOR_SEL_DEF  QColor(10,10,10)
#define BORDER_COLOR_DEF    QColor(0,0,0)
#define BORDER_COLOR_SEL_DEF QColor(0,0,0)
#define BORDER_WIDTH_DEF    0
#define BORDER_RADIUS_DEF   0

#endif
