#ifndef ATCMSPINBOX_H
#define ATCMSPINBOX_H

#include <QtGui/QWidget>
#include <QLocale>
#include <QtGui/QDoubleSpinBox>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QFrame>
#include "atcmpluginobject.h"

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMspinbox : public QDoubleSpinBox, public ATCMpluginObject
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
        Q_PROPERTY(ButtonSymbols buttonSymbols READ buttonSymbols WRITE setButtonSymbols DESIGNABLE false)
		Q_PROPERTY(QString specialValueText READ specialValueText WRITE setSpecialValueText DESIGNABLE false)
		Q_PROPERTY(CorrectionMode correctionMode READ correctionMode WRITE setCorrectionMode DESIGNABLE false)
		Q_PROPERTY(bool keyboardTracking READ keyboardTracking WRITE setKeyboardTracking DESIGNABLE false)
		Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement DESIGNABLE false)
		Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy DESIGNABLE false)
		Q_PROPERTY(Qt::ContextMenuPolicy contextMenuPolicy READ contextMenuPolicy WRITE setContextMenuPolicy DESIGNABLE false)
		Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops DESIGNABLE false)
		Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip DESIGNABLE false)
		Q_PROPERTY(bool autoFillBackground READ autoFillBackground WRITE setAutoFillBackground DESIGNABLE false)
		Q_PROPERTY(QLocale locale READ locale WRITE setLocale DESIGNABLE false)
		Q_PROPERTY(Qt::InputMethodHints inputMethodHints READ inputMethodHints WRITE setInputMethodHints DESIGNABLE false)
        Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly DESIGNABLE false)
        Q_PROPERTY(bool frame READ hasFrame WRITE setFrame DESIGNABLE false)
		/************* new property ************ */
		/* name of the cross table variable associated */
        Q_PROPERTY(QString variable READ variable WRITE setVariable RESET unsetVariable)
		/* set if the the status of the associated variable have an visible feedback */
        Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
		/* set the crosstable variable to associate the  visibility */
        Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
		/* set the background color */
		Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
		/* set the border color */
		Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
		/* set the font color */
		Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
        /* set the label color */
        Q_PROPERTY(QColor labelColor READ labelColor WRITE setLabelColor)
        /* set the border width */
		Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
		/* set the border radius */
		Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
		/* set the apparence */
        Q_PROPERTY(enum QFrame::Shadow apparence READ apparence WRITE setApparence DESIGNABLE true RESET unsetApparence)
#endif
	public:
		ATCMspinbox(QWidget *parent = 0);
		~ATCMspinbox();
		float value()    const { return m_value; }
		QString variable() const { return m_variable; }
		bool viewStatus()  const { return m_viewstatus; }
		QString visibilityVar()  const { return m_visibilityvar; }
		char statusComm()      const { return m_status; }
		char statusButton()      const { return m_objectstatus; }
		int borderWidth()      const { return m_borderwidth; }
		int borderRadius()      const { return m_borderradius; }
		QColor bgColor() const;
		QColor borderColor() const;
		QColor fontColor() const;
        QColor labelColor() const;
		bool startAutoReading();

        enum QFrame::Shadow apparence() const;

		bool stopAutoReading();

        void unsetVariable();
        void unsetViewStatus();
        void unsetVisibilityVar();
        void unsetApparence();

public Q_SLOTS:
		bool writeValue(double);
		bool setVariable(QString);
        bool setRefresh(int){return true;}
		void setViewStatus(bool);
		bool setVisibilityVar(QString);
		void setBgColor(const QColor& bgColor);
		void setBorderColor(const QColor& borderColor);
		void setFontColor(const QColor& fontColor);
        void setLabelColor(const QColor& bgColor);

		void setBorderWidth(int width);
		void setBorderRadius(int radius);

        void setApparence(const enum QFrame::Shadow apparence);

	protected Q_SLOTS:
		void updateData();

	protected:
		float m_value;
		QString m_variable;
		QString m_visibilityvar;
		char m_status;
		bool m_viewstatus;
		bool m_objectstatus;
		int m_CtIndex;
		int m_CtVisibilityIndex;
		QColor m_bgcolor;
		QColor m_bordercolor;
		QColor m_fontcolor;
        QColor m_labelcolor;

		int m_borderwidth;
		int m_borderradius;

        enum QFrame::Shadow m_apparence;

		bool m_initialization;

	protected:
		void paintEvent(QPaintEvent *event);

	private:
        QWidget *m_parent;
        bool m_lastVisibility;
};

#endif
