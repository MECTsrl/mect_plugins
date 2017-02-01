#ifndef ATCMPROGRESSBAR_H
#define ATCMPROGRESSBAR_H

#include <QtGui/QWidget>
#include <QtGui/QProgressBar>
#include <QLocale>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QTimer>
#include <QFrame>

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMprogressbar : public QProgressBar
{
	Q_OBJECT
#ifndef TARGET_ARM
		/************* property to hide *************/
        Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet DESIGNABLE false)
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
		Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment DESIGNABLE false)
		Q_PROPERTY(bool textVisible READ isTextVisible WRITE setTextVisible DESIGNABLE false)
		Q_PROPERTY(bool invertedAppearance READ invertedAppearance WRITE setInvertedAppearance DESIGNABLE false)
		Q_PROPERTY(QProgressBar::Direction textDirection READ textDirection WRITE setTextDirection DESIGNABLE false)
		Q_PROPERTY(QString format READ format WRITE setFormat DESIGNABLE false)
		Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement DESIGNABLE false)
		Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy DESIGNABLE false)
		Q_PROPERTY(Qt::ContextMenuPolicy contextMenuPolicy READ contextMenuPolicy WRITE setContextMenuPolicy DESIGNABLE false)
		Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops DESIGNABLE false)
		Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip DESIGNABLE false)
		Q_PROPERTY(bool autoFillBackground READ autoFillBackground WRITE setAutoFillBackground DESIGNABLE false)
		Q_PROPERTY(QLocale locale READ locale WRITE setLocale DESIGNABLE false)
		Q_PROPERTY(Qt::InputMethodHints inputMethodHints READ inputMethodHints WRITE setInputMethodHints DESIGNABLE false)
        Q_PROPERTY(QFont font READ font WRITE setFont DESIGNABLE false)
		/************* new property ************ */
		/* name of the cross table variable associated */
        Q_PROPERTY(QString variable READ variable WRITE setVariable RESET unsetVariable)
		/* refresh time of the crosstable variables */
        Q_PROPERTY(int refresh READ refresh WRITE setRefresh RESET unsetRefresh)
		/* set if the the status of the associated variable have an visible feedback */
        Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
		/* set the crosstable variable to associate the button visibility */
        Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
		/* set the bar color */
		Q_PROPERTY(QColor barColor READ barColor WRITE setBarColor)
		/* set the background color */
		Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
		/* set the border color */
		Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
		/* set the border width */
		Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
		/* set the border radius */
		Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
		/* set the apparence */
        Q_PROPERTY(enum QFrame::Shadow apparence READ apparence WRITE setApparence RESET unsetApparence)
#endif
	public:
		ATCMprogressbar(QWidget *parent = 0);
		~ATCMprogressbar();
		int value()    const { return m_value; }
		QString variable() const { return m_variable; }
		int refresh()      const { return m_refresh; }
		char status()      const { return m_status; }
		bool viewStatus()  const { return m_viewstatus; }
		QString visibilityVar()  const { return m_visibilityvar; }
		int borderWidth()      const { return m_borderwidth; }
		int borderRadius()      const { return m_borderradius; }
		QColor bgColor() const;
		QColor borderColor() const;
		QColor barColor() const;

        bool startAutoReading() const { return true; }
        bool stopAutoReading()  const { return true; }

        enum QFrame::Shadow apparence() const;


	public Q_SLOTS:
		bool setVariable(QString);
		bool setRefresh(int);
		void setViewStatus(bool);
		bool setVisibilityVar(QString);
		void setBarColor(const QColor& barColor);
		void setBgColor(const QColor& bgColor);
		void setBorderColor(const QColor& borderColor);

		void setBorderWidth(int width);
		void setBorderRadius(int radius);

        void setApparence(const enum QFrame::Shadow apparence);

        void unsetVariable();
        void unsetRefresh();
        void unsetViewStatus();
        void unsetVisibilityVar();
        void unsetApparence();

    protected Q_SLOTS:
		void updateData();

	protected:
		int m_value;
		QString m_variable;
		QString m_visibilityvar;
		int m_refresh;
		char m_status;
		bool m_viewstatus;
		int m_CtIndex;
		int m_CtVisibilityIndex;
		QColor m_bgcolor;
		QColor m_bordercolor;
		QColor m_barColor;

		int m_borderwidth;
		int m_borderradius;

        enum QFrame::Shadow m_apparence;

	protected:
		void paintEvent(QPaintEvent *event);

	private:
        QWidget *m_parent;
};

#endif
