#ifndef ATCMLABEL_H
#define ATCMLABEL_H

#include <QLocale>
#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QFrame>
#include "atcmpluginobject.h"

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMlabel : public QPushButton, public ATCMpluginObject
{
	Q_OBJECT
#ifndef TARGET_ARM
		/************* property to hide *************/
        Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet DESIGNABLE false)
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
		/************* new property ************ */
		/* name of the cross table variable associated */
        Q_PROPERTY(QString variable READ variable WRITE setVariable RESET unsetVariable)
		/* prefix to show */
        Q_PROPERTY(QString prefix READ prefix WRITE setPrefix RESET unsetPrefix)
		/* suffix to show */
        Q_PROPERTY(QString suffix READ suffix WRITE setSuffix RESET unsetSuffix)
		/* maximum value allowed of the associated crosstable variable */
        Q_PROPERTY(QString max READ max WRITE setMax RESET unsetMax)
        /* minumum value allowed of the associated crosstable variable */
        Q_PROPERTY(QString min READ min WRITE setMin RESET unsetMin)
        /* set if the the status of the associated variable have an visible feedback */
        Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
		/* set the crosstable variable to associate the label visibility */
        Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
		/* set the label background color */
		Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
		/* set the label border color */
		Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
		/* set the label font color */
		Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
		/* set the label background color */
		Q_PROPERTY(QColor bgSelectColor READ bgSelectColor WRITE setBgSelectColor)
		/* set the label border color */
		Q_PROPERTY(QColor borderSelectColor READ borderSelectColor WRITE setBorderSelectColor)
		/* set the label font color */
		Q_PROPERTY(QColor fontSelectColor READ fontSelectColor WRITE setFontSelectColor)
		/* set the border width */
		Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
		/* set the border radius */
		Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
		/* set the apparence */
        Q_PROPERTY(enum QFrame::Shadow apparence READ apparence WRITE setApparence RESET unsetApparence)
        Q_ENUMS(ATCMLabelFormat)
        Q_PROPERTY(enum ATCMLabelFormat format READ format WRITE setFormat)
#endif

    public:
        enum ATCMLabelFormat
        {
            Dec,
            Hex,
            Bin
        };
        ATCMlabel(QWidget *parent = 0);
		~ATCMlabel();
        enum ATCMLabelFormat format() const
        {
            return m_format;
        }
        QString value()    const { return m_value; }
		QString variable() const { return m_variable; }
		QString prefix() { return m_prefix; }
		QString suffix() { return m_suffix; }
		QString min()      const { return m_min; }
		QString max()      const { return m_max; }
		bool viewStatus()  const { return m_viewstatus; }
		QString visibilityVar()  const { return m_visibilityvar; }
		char statusComm()      const { return m_status; }
		int borderWidth()      const { return m_borderwidth; }
		int borderRadius()      const { return m_borderradius; }
		QColor bgColor() const;
		QColor borderColor() const;
		QColor fontColor() const;
		QColor bgSelectColor() const;
		QColor borderSelectColor() const;
		QColor fontSelectColor() const;
        
		enum QFrame::Shadow apparence() const;

	public Q_SLOTS:
		bool writeValue(QString);
		bool setVariable(QString);
        bool setRefresh(int) const {return true;}
        void setPrefix(QString);
		void setSuffix(QString);
		bool setMin(QString);
		bool setMax(QString);
		void setViewStatus(bool);
		bool setVisibilityVar(QString);
		void setBgColor(const QColor& bgColor);
		void setBorderColor(const QColor& borderColor);
		void setFontColor(const QColor& fontColor);
		void setBgSelectColor(const QColor& bgColor);
		void setBorderSelectColor(const QColor& borderColor);
		void setFontSelectColor(const QColor& fontColor);

		void setBorderWidth(int width);
		void setBorderRadius(int radius);

        void setApparence(const enum QFrame::Shadow apparence);
        void setFormat(const enum ATCMLabelFormat format);

        void unsetVariable();
        void unsetPrefix();
        void unsetSuffix();
        void unsetMin();
        void unsetMax();
        void unsetViewStatus();
        void unsetVisibilityVar();
        void unsetApparence();

    protected Q_SLOTS:
		void updateData();
        void writeAction();
        void releaseAction();

	protected:
		QString m_value;
		QString m_min;
		QString m_max;
		QString m_variable;
		QString m_prefix;
		QString m_suffix;
		QString m_visibilityvar;
		char m_status;
		bool m_viewstatus;
		bool m_objectstatus;
		int m_CtIndex;
		int m_CtVisibilityIndex;
		QColor m_bgcolor;
		QColor m_bordercolor;
		QColor m_fontcolor;

		QColor m_bgcolor_select;
		QColor m_bordercolor_select;
		QColor m_fontcolor_select;

		int m_borderwidth;
		int m_borderradius;

        enum QFrame::Shadow m_apparence;
        enum ATCMLabelFormat m_format;

        QWidget *m_parent;
        bool m_lastVisibility;
protected:
		void paintEvent(QPaintEvent *event);

};

#endif
