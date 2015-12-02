#ifndef ATCMCOMBOBOX_H
#define ATCMCOMBOBOX_H

#include <QtGui/QWidget>
#include <QLocale>
#include <QtGui/QComboBox>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include <QTimer>
#include <QFrame>
//#include "atcmcomboboxtaskmenu.h"

class
#ifndef TARGET_ARM
 QDESIGNER_WIDGET_EXPORT
#endif
 ATCMcombobox : public QComboBox
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
		Q_PROPERTY(bool editable READ isEditable WRITE setEditable DESIGNABLE false)
		Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize DESIGNABLE false)
		Q_PROPERTY(QSize sizeIncrement READ sizeIncrement WRITE setSizeIncrement DESIGNABLE false)
		Q_PROPERTY(bool frame READ hasFrame WRITE setFrame DESIGNABLE false)
		Q_PROPERTY(Qt::FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy DESIGNABLE false)
		Q_PROPERTY(Qt::ContextMenuPolicy contextMenuPolicy READ contextMenuPolicy WRITE setContextMenuPolicy DESIGNABLE false)
		Q_PROPERTY(bool acceptDrops READ acceptDrops WRITE setAcceptDrops DESIGNABLE false)
		Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip DESIGNABLE false)
		Q_PROPERTY(bool autoFillBackground READ autoFillBackground WRITE setAutoFillBackground DESIGNABLE false)
		Q_PROPERTY(QLocale locale READ locale WRITE setLocale DESIGNABLE false)
		Q_PROPERTY(Qt::InputMethodHints inputMethodHints READ inputMethodHints WRITE setInputMethodHints DESIGNABLE false)
		Q_PROPERTY(int modelColumn READ modelColumn WRITE setModelColumn DESIGNABLE false)
		Q_PROPERTY(InsertPolicy insertPolicy READ insertPolicy WRITE setInsertPolicy DESIGNABLE false)
		/************* new property ************ */
		/* name of the cross table variable associated */
		Q_PROPERTY(QString variable READ variable WRITE setVariable RESET unsetVariable)
		/* refresh time of the crosstable variables */
		Q_PROPERTY(int refresh READ refresh WRITE setRefresh RESET unsetRefresh)
		/* set if the the status of the associated variable have an visible feedback */
		Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
		/* set if the a confirmation will be appear before each writing */
		Q_PROPERTY(bool writeAcknowledge READ writeAcknowledge WRITE setWriteAcknowledge RESET unsetwriteAcknowledge)
		/* set the crosstable variable to associate the combobox visibility */
		Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
		/* set the combobox background color */
		Q_PROPERTY(QColor bgColor READ bgColor WRITE setBgColor)
		/* set the combobox border color */
		Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)
		/* set the combobox font color */
		Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
		/* set the border width */
		Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth)
		/* set the border radius */
		Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
		/* set the apparence */
		Q_PROPERTY(enum QFrame::Shadow apparence READ apparence WRITE setApparence RESET unsetApparence)
		/* set mapping string */
		Q_PROPERTY(QString mapping READ mapping WRITE setMapping RESET unsetMapping)
		//Q_PROPERTY(atcmcomboboxTaskMenu * prova READ prova WRITE setProva)
#endif
	public:
		ATCMcombobox(QWidget *parent = 0);
		~ATCMcombobox();
		QString value()    const { return m_value; }
		QString variable() const { return m_variable; }
		QString mapping()  const { return m_mapping; }
		int refresh()      const { return m_refresh; }
		bool viewStatus()  const { return m_viewstatus; }
		bool writeAcknowledge()  const { return m_writeAcknowledge; }
		QString visibilityVar()  const { return m_visibilityvar; }
		char statusComm()      const { return m_status; }
		char statusButton()      const { return m_objectstatus; }
		int borderWidth()      const { return m_borderwidth; }
		int borderRadius()      const { return m_borderradius; }
		QColor bgColor() const;
		QColor borderColor() const;
		QColor fontColor() const;
		bool startAutoReading();
		//atcmcomboboxTaskMenu * prova() { return m_prova; }

		enum QFrame::Shadow apparence() const;

		bool stopAutoReading();

		public Q_SLOTS:
			bool writeValue(QString);
		bool setVariable(QString);
		bool setMapping(QString);
		bool setRefresh(int);
		void setViewStatus(bool);
		void setWriteAcknowledge(bool);
		bool setVisibilityVar(QString);
		void setBgColor(const QColor& bgColor);
		void setBorderColor(const QColor& borderColor);
		void setFontColor(const QColor& fontColor);

		void setBorderWidth(int width);
		void setBorderRadius(int radius);

		void setApparence(const enum QFrame::Shadow apparence);

		void unsetMapping();
		void unsetVariable();
		void unsetRefresh();
		void unsetViewStatus();
		void unsetVisibilityVar();
		void unsetwriteAcknowledge();
		void unsetApparence();

		protected Q_SLOTS:
			void updateData();

	protected:
		QString m_value;
		QString m_variable;
		QString m_mapping;
		QString m_visibilityvar;
		int m_refresh;
		char m_status;
		bool m_viewstatus;
		bool m_writeAcknowledge;
		bool m_objectstatus;
		int m_CtIndex;
		int m_CtVisibilityIndex;
		QColor m_bgcolor;
		QColor m_bordercolor;
		QColor m_fontcolor;

		int m_borderwidth;
		int m_borderradius;

		enum QFrame::Shadow m_apparence;

		bool m_initialization;

		//atcmcomboboxTaskMenu * m_prova;
	protected:
		void paintEvent(QPaintEvent *event);
		bool setcomboValue();
		QString value2mapped( QString value );
		QString mapped2value( QString mapped );

	private:
		QTimer * refresh_timer;
};

#endif
