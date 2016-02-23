#ifndef ATCMBUTTON_H
#define ATCMBUTTON_H

#include <QtGui/QWidget>
#include <QtGui/QPushButton>
#include <QLocale>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QTimer>
#include <QFrame>
#include <QMutex>

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMbutton : public QPushButton
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
#ifndef TARGET_ARM
		Q_PROPERTY(QString accessibleName READ accessibleName WRITE setAccessibleName DESIGNABLE false)
		Q_PROPERTY(QString accessibleDescription READ accessibleDescription WRITE setAccessibleDescription DESIGNABLE false)
#endif
		Q_PROPERTY(Qt::LayoutDirection layoutDirection READ layoutDirection WRITE setLayoutDirection DESIGNABLE false)
        Q_PROPERTY(QKeySequence	shortcut READ shortcut WRITE setShortcut DESIGNABLE false)
		Q_PROPERTY(bool	autoExclusive READ autoExclusive WRITE setAutoExclusive DESIGNABLE false)
		Q_PROPERTY(bool	autoDefault READ autoDefault WRITE setAutoDefault DESIGNABLE false)
		Q_PROPERTY(bool	default READ isDefault WRITE setDefault DESIGNABLE false)
		Q_PROPERTY(bool	flat READ isFlat WRITE setFlat DESIGNABLE false)
		Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement DESIGNABLE false)
		Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy DESIGNABLE false)
		Q_PROPERTY(Qt::ContextMenuPolicy contextMenuPolicy READ contextMenuPolicy WRITE setContextMenuPolicy DESIGNABLE false)
		Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops DESIGNABLE false)
		Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip DESIGNABLE false)
		Q_PROPERTY(bool autoFillBackground READ autoFillBackground WRITE setAutoFillBackground DESIGNABLE false)
		Q_PROPERTY(QLocale locale READ locale WRITE setLocale DESIGNABLE false)
		Q_PROPERTY(Qt::InputMethodHints inputMethodHints READ inputMethodHints WRITE setInputMethodHints DESIGNABLE false)
		/************* new property ************ */
		/* name of the page to go when the button is pressed */
		Q_PROPERTY(QString pageName READ pageName WRITE setPageName RESET unsetPageName)
		/* add or not the target page into the history */
		Q_PROPERTY(bool remember READ remember WRITE setRemember RESET unsetRemember)
		/* password to ask before perform the other operation when the button is pressed */
		Q_PROPERTY(QString passwordVar READ passwordVar WRITE setPasswordVar RESET unsetPasswordVar)
        /* name of the cross table variable associated to the status of button (pressed/checked or released/uncheked) */
        Q_PROPERTY(QString statusvar READ statusvar WRITE setStatusvar RESET unsetStatusvar)
        /* value to set into statusvar when the button is released  */
        Q_PROPERTY(QString statusReleasedValue READ statusReleasedValue WRITE setStatusReleasedValue)
        /* value to set into statusvar when the button is pressed */
        Q_PROPERTY(QString statusPressedValue READ statusPressedValue WRITE setStatusPressedValue)
        /* set if the the status of the associated variable have an visible feedback */
		Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
        /* refresh time of the crosstable variables */
        Q_PROPERTY(int refresh READ refresh WRITE setRefresh RESET unsetRefresh)
        /* set the crosstable variable to associate the button visibility */
		Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
		/* set the button background color */
		Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
		/* set the button border color */
		Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
		/* set the button font color */
		Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
		/* set the button background color */
		Q_PROPERTY(QColor bgPressColor READ bgPressColor WRITE setBgPressColor)
		/* set the button border color */
		Q_PROPERTY(QColor borderPressColor READ borderPressColor WRITE setBorderPressColor)
		/* set the button font color */
		Q_PROPERTY(QColor fontPressColor READ fontPressColor WRITE setFontPressColor)
		/* set the text button */
		Q_PROPERTY(QString text READ text WRITE setText)
		/* set the text button when the status is press */
		Q_PROPERTY(QString pressText READ pressText WRITE setPressText)
		/* set the icon button */
		Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
		/* set the icon button when the status is press */
		Q_PROPERTY(QIcon pressIcon READ pressIcon WRITE setPressIcon)
		/* set the border width */
		Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
		/* set the border radius */
		Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
		/* set checkable */
		//Q_PROPERTY(bool checkable READ checkable WRITE setCheckable)
		/* setChecked */
		//Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus)
		/* set the apparence */
		Q_PROPERTY(enum QFrame::Shadow apparence READ apparence WRITE setApparence)
#endif
	public:
		ATCMbutton(QWidget * parent = 0);
		~ATCMbutton();
		QString pageName() const { return m_pagename; }
		bool remember()      const { return m_remember; }
		QString passwordVar() const { return m_passwordVar; }
        QString statusvar() const { return m_statusvar; }
        QString statusPressedValue() const { return m_statuspressval; }
        QString statusReleasedValue() const { return m_statusreleaseval; }
        int refresh()      const { return m_refresh; }
		bool viewStatus()  const { return m_viewstatus; }
		QString visibilityVar()  const { return m_visibilityvar; }
		char statusComm()      const { return m_status; }
        char statusButton()      const { return QPushButton::isDown(); }
		int borderWidth()      const { return m_borderwidth; }
		int borderRadius()      const { return m_borderradius; }
		QColor bgColor() const;
		QColor borderColor() const;
		QColor fontColor() const;
		QColor bgPressColor() const;
		QColor borderPressColor() const;
		QColor fontPressColor() const;

		QString text();
		QString pressText();

		QIcon icon() const;
		QIcon pressIcon() const;

		enum QFrame::Shadow apparence() const;

		bool startAutoReading();
		bool stopAutoReading();

	public Q_SLOTS:
		void setPageName(QString);
		void setRemember(bool);
		bool setPasswordVar(QString);
		bool setStatusvar(QString);
        bool setStatusPressedValue(QString);
        bool setStatusReleasedValue(QString);
        bool setRefresh(int);
		void setViewStatus(bool);
		bool setVisibilityVar(QString);
		void setBgColor(const QColor& bgColor);
		void setBorderColor(const QColor& borderColor);
		void setFontColor(const QColor& fontColor);
		void setBgPressColor(const QColor& bgColor);
		void setBorderPressColor(const QColor& borderColor);
		void setFontPressColor(const QColor& fontColor);

		void setText(const QString text);
		void setPressText(const QString text);

		void setIcon(const QIcon& icon);
		void setPressIcon(const QIcon& icon);

		void setBorderWidth(int width);
		void setBorderRadius(int radius);

		void setApparence(const enum QFrame::Shadow apparence);

		void unsetPageName();
		void unsetRemember();
		void unsetPasswordVar();
		void unsetStatusvar();
		void unsetRefresh();
		void unsetViewStatus();
		void unsetVisibilityVar();
        bool checkPassword();

		protected Q_SLOTS:
			void updateData();
        void pressFunction();
        void releaseFunction();
        void pressAction();
		void releaseAction();
		void toggleAction(bool);
        void goToPage();

		protected Q_SLOTS:

	Q_SIGNALS:
        void newPage( const char * pagename, bool remember);

	protected:
		QString m_pagename;
		bool m_remember;
		QString m_passwordValue;
		QString m_passwordVar;
		QString m_visibilityvar;
		QString m_statusvar;
        QString m_statuspressval;
        QString m_statusreleaseval;
        QString m_statusactualval;
        int m_justchanged;
        bool m_forcedAction;
        int m_refresh;
		char m_status;
		bool m_viewstatus;
		int m_CtIndex;
		int m_CtVisibilityIndex;
        int m_CtPasswordVarIndex;
        QColor m_bgcolor;
		QColor m_bordercolor;
		QColor m_fontcolor;

		QColor m_bgcolor_press;
		QColor m_bordercolor_press;
		QColor m_fontcolor_press;

		QString m_text;
		QString m_text_press;

		QIcon m_icon;
		QIcon m_icon_press;

		int m_borderwidth;
		int m_borderradius;

		enum QFrame::Shadow m_apparence;

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		QTimer * refresh_timer;
        QMutex theMutex;
};

#endif
