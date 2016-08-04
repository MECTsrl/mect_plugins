#ifndef ATCMANIMATION_H
#define ATCMANIMATION_H

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QLocale>
#ifndef TARGET_ARM
#include <QtDesigner/QDesignerExportWidget>
#endif
#include "atcmplugin.h"

class
        #ifndef TARGET_ARM
        QDESIGNER_WIDGET_EXPORT
        #endif

        ATCManimation : public QLabel, public ATCMplugin
{
    Q_OBJECT
#ifndef TARGET_ARM
    /************* property to hide *************/
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled DESIGNABLE false)
    //Q_PROPERTY(QSizePolicy sizePolicy READ sizePolicy WRITE setSizePolicy DESIGNABLE false)
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
    Q_PROPERTY(Qt::TextFormat textFormat READ textFormat WRITE setTextFormat DESIGNABLE false)
    Q_PROPERTY(bool openExternalLinks READ openExternalLinks WRITE setOpenExternalLinks DESIGNABLE false)
    Q_PROPERTY(Qt::TextInteractionFlags textInteractionFlags READ textInteractionFlags WRITE setTextInteractionFlags DESIGNABLE false)
    Q_PROPERTY(bool wordWrap READ wordWrap WRITE setWordWrap DESIGNABLE false)
    Q_PROPERTY(int indent READ indent WRITE setIndent DESIGNABLE false)
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
    Q_PROPERTY(Qt::Alignment alignment READ alignment DESIGNABLE false)
    Q_PROPERTY(int margin READ margin DESIGNABLE false)
    Q_PROPERTY(QFont font READ font WRITE setFont DESIGNABLE false)
    Q_PROPERTY(bool scaledContents READ hasScaledContents WRITE setScaledContents DESIGNABLE false)
    Q_PROPERTY(QWidget * buddy READ buddy WRITE setBuddy DESIGNABLE false)
    /************* new property ************ */
    /* mapping string */
    Q_PROPERTY(QString mapping READ mapping WRITE setMapping RESET unsetMapping)
    /* name of the cross table variable associated */
    Q_PROPERTY(QString variable READ variable WRITE setVariable RESET unsetVariable)
    /* set if the the status of the associated variable have an visible feedback */
    Q_PROPERTY(bool viewStatus READ viewStatus WRITE setViewStatus RESET unsetViewStatus)
    /* set the crosstable variable to associate the object visibility */
    Q_PROPERTY(QString visibilityVar READ visibilityVar WRITE setVisibilityVar RESET unsetVisibilityVar)
#endif
public:
    ATCManimation(QWidget *parent = 0);
    ~ATCManimation();
    QString value()    const { return m_value; }
    QString variable() const { return m_variable; }
    QString mapping()  const { return m_mapping; }
    char status()      const { return m_status; }
    bool viewStatus()  const { return m_viewstatus; }
    QString visibilityVar()  const { return m_visibilityvar; }
    bool startAutoReading();
    bool stopAutoReading();
    virtual QSize 	sizeHint () { return QSize(50,50); }

public Q_SLOTS:
    bool setVariable(QString);
    bool setMapping(QString);
    bool setRefresh(int){return true;}
    void setViewStatus(bool);
    bool setVisibilityVar(QString);

    void unsetMapping();
    void unsetVariable();
    void unsetViewStatus();
    void unsetVisibilityVar();

protected Q_SLOTS:
    void updateData();

protected:
    bool m_lastVisibility;
    QString m_value;
    QString m_variable;
    QString m_mapping;
    QString m_visibilityvar;
    char m_status;
    bool m_viewstatus;
    int m_CtIndex;
    int m_CtVisibilityIndex;
    QStringList m_maplist;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QWidget *m_parent;
};

#endif
