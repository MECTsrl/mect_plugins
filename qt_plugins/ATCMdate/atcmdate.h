#ifndef ATCMDATE_H
#define ATCMDATE_H

#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QLocale>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QTimer>
#include <QFrame>
#include "atcmpluginobject.h"

#define ITALIAN_DATE "dd/MM/yyyy"
#define ENGLISH_DATE "yyyy/MM/dd"
#define DEFAULT_DATE ITALIAN_DATE

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMdate : public QPushButton, public ATCMpluginObject
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
        Q_PROPERTY(QKeySequence	shortcut READ shortcut WRITE setShortcut DESIGNABLE false)
		Q_PROPERTY(bool	autoExclusive READ autoExclusive WRITE setAutoExclusive DESIGNABLE false)
		Q_PROPERTY(bool	autoRepeat READ autoRepeat WRITE setAutoRepeat DESIGNABLE false)
		Q_PROPERTY(int	autoRepeatDelay READ autoRepeatDelay WRITE setAutoRepeatDelay DESIGNABLE false)
		Q_PROPERTY(int	autoRepeatInterval READ autoRepeatInterval WRITE setAutoRepeatInterval DESIGNABLE false)
		Q_PROPERTY(bool	autoDefault READ autoDefault WRITE setAutoDefault DESIGNABLE false)
		Q_PROPERTY(bool	default READ isDefault WRITE setDefault DESIGNABLE false)
		Q_PROPERTY(bool	flat READ isFlat WRITE setFlat DESIGNABLE false)
		Q_PROPERTY(QIcon icon READ icon WRITE setIcon DESIGNABLE false)
		Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize DESIGNABLE false)
		Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement DESIGNABLE false)
		Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy DESIGNABLE false)
		Q_PROPERTY(Qt::ContextMenuPolicy contextMenuPolicy READ contextMenuPolicy WRITE setContextMenuPolicy DESIGNABLE false)
		Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops DESIGNABLE false)
		Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip DESIGNABLE false)
		Q_PROPERTY(bool autoFillBackground READ autoFillBackground WRITE setAutoFillBackground DESIGNABLE false)
		Q_PROPERTY(QLocale locale READ locale WRITE setLocale DESIGNABLE false)
		Q_PROPERTY(Qt::InputMethodHints inputMethodHints READ inputMethodHints WRITE setInputMethodHints DESIGNABLE false)
		Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable DESIGNABLE false)
		Q_PROPERTY(bool checked READ isChecked WRITE setChecked DESIGNABLE false)
        Q_PROPERTY(const QString text READ text DESIGNABLE false)
		/************* new property ************ */
		/* set the date background color */
		Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
		/* set the date border color */
		Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
		/* set the date font color */
		Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
		/* set the date background color */
		Q_PROPERTY(QColor bgSelectColor READ bgSelectColor WRITE setBgSelectColor)
		/* set the date border color */
		Q_PROPERTY(QColor borderSelectColor READ borderSelectColor WRITE setBorderSelectColor)
		/* set the date font color */
		Q_PROPERTY(QColor fontSelectColor READ fontSelectColor WRITE setFontSelectColor)
		/* set the border width */
		Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
		/* set the border radius */
		Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
		/* set the apparence */
        Q_PROPERTY(enum QFrame::Shadow apparence READ apparence WRITE setApparence)
		/* set the format */
		Q_ENUMS(ATCMDateFormat)
        Q_PROPERTY(enum ATCMDateFormat format READ format WRITE setFormat)
 #endif
	public:
        enum ATCMDateFormat
        {
            YYYY_MM_DD,
            DD_MM_YYYY
        };
        ATCMdate(QWidget *parent = 0);
		~ATCMdate();
		int borderWidth()      const { return m_borderwidth; }
		int borderRadius()      const { return m_borderradius; }
		QColor bgColor() const;
		QColor borderColor() const;
		QColor fontColor() const;
		QColor bgSelectColor() const;
		QColor borderSelectColor() const;
		QColor fontSelectColor() const;
		bool startAutoReading();
        
		enum QFrame::Shadow apparence() const;
        enum ATCMDateFormat format() const
        {
            if (m_format.compare(ITALIAN_DATE) == 0)
            {
                return DD_MM_YYYY;
            }
            else
            {
                return YYYY_MM_DD;
            }
        }

		bool stopAutoReading();

	public Q_SLOTS:
		void setBgColor(const QColor& bgColor);
		void setBorderColor(const QColor& borderColor);
		void setFontColor(const QColor& fontColor);
		void setBgSelectColor(const QColor& bgColor);
		void setBorderSelectColor(const QColor& borderColor);
		void setFontSelectColor(const QColor& fontColor);

		void setBorderWidth(int width);
		void setBorderRadius(int radius);

        void setApparence(const enum QFrame::Shadow apparence);
        void setFormat(const enum ATCMDateFormat format);

    protected Q_SLOTS:
		void updateData();

	protected:
		QColor m_bgcolor;
		QColor m_bordercolor;
		QColor m_fontcolor;

		QColor m_bgcolor_select;
		QColor m_bordercolor_select;
		QColor m_fontcolor_select;

		int m_borderwidth;
		int m_borderradius;

        enum QFrame::Shadow m_apparence;
        QString m_format;

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		QTimer * refresh_timer;
};

#endif
