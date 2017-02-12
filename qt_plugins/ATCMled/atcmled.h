#ifndef ATCMLED_H
#define ATCMLED_H

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QLocale>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QIcon>
#include <QTimer>

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMled : public QLabel
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
		/* hide QLabel property */
		Q_PROPERTY(const QPixmap* pixmap READ pixmap DESIGNABLE false)
		Q_PROPERTY(const QString text READ text DESIGNABLE false)
		Q_PROPERTY(Qt::TextFormat textFormat READ textFormat DESIGNABLE false)
		Q_PROPERTY(bool scaledContents READ hasScaledContents DESIGNABLE false)
		Q_PROPERTY(Qt::Alignment alignment READ alignment DESIGNABLE false)
		Q_PROPERTY(bool wordWrap READ wordWrap DESIGNABLE false)
		Q_PROPERTY(int margin READ margin DESIGNABLE false)
		Q_PROPERTY(int indent READ indent DESIGNABLE false)
		Q_PROPERTY(bool openExternalLinks READ openExternalLinks DESIGNABLE false)
		Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags DESIGNABLE false)
		Q_PROPERTY(QFont font READ font WRITE setFont DESIGNABLE false)
		/* hide QFrame property */
		Q_PROPERTY(Shape frameShape READ frameShape DESIGNABLE false)
		Q_PROPERTY(Shadow frameShadow READ frameShadow DESIGNABLE false)
		Q_PROPERTY(int lineWidth READ lineWidth DESIGNABLE false)
		Q_PROPERTY(int midLineWidth READ midLineWidth DESIGNABLE false)
		Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet DESIGNABLE false)
		Q_PROPERTY(QWidget * buddy READ buddy WRITE setBuddy DESIGNABLE false)
		/************* new property ************ */
		/* name of the cross table variable associated */
		Q_PROPERTY(QString variable READ variable WRITE setVariable RESET unsetVariable)
		/* refresh time of the crosstable variables */
		Q_PROPERTY(int refresh READ refresh WRITE setRefresh RESET unsetRefresh)
		/* set if the the status of the associated variable have an visible feedback */
		Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
		/* set the crosstable variable to associate the led visibility */
		Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
		/* set the icon led when it is on */
		Q_PROPERTY(QIcon onIcon READ onIcon WRITE setOnIcon RESET unsetOnIcon )
		/* set the icon led when it is off */
		Q_PROPERTY(QIcon offIcon READ offIcon WRITE setOffIcon RESET unsetOffIcon)
#endif
	public:
		ATCMled(QWidget *parent = 0);
		~ATCMled();
		QString variable() const { return m_variable; }
		int refresh()      const { return m_refresh; }
		bool viewStatus()  const { return m_viewstatus; }
		QString visibilityVar()  const { return m_visibilityvar; }

		QIcon onIcon() const;
		QIcon offIcon() const;
		int value()    const { return m_value; }
        bool startAutoReading() const { return true; }
        bool stopAutoReading()  const { return true; }
		virtual QSize 	sizeHint () { return QSize(15,15); }

	public Q_SLOTS:
		bool setVariable(QString);
        bool setRefresh(int);
        void unsetRefresh();
        void setViewStatus(bool);
		bool setVisibilityVar(QString);
		void setOffIcon(const QIcon& icon);
		void setOnIcon(const QIcon& icon);

		void unsetVariable();
		void unsetViewStatus();
		void unsetVisibilityVar();
		void unsetOnIcon();
		void unsetOffIcon();

	protected Q_SLOTS:
		void updateData();

	protected:
		int m_value;
		QString m_variable;
		QString m_visibilityvar;
		int m_refresh;
		char m_status;
		bool m_viewstatus;
		bool m_objectstatus;
		int m_CtIndex;
		int m_CtVisibilityIndex;

		QIcon m_onicon;
		QIcon m_officon;

	protected:
		void paintEvent(QPaintEvent *event);

	private:
};

#endif
