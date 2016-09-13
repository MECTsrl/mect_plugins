TARGET      = $$qtLibraryTarget(atcmsliderplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS     = atcmsliderplugin.h \
    variableandvisibilitydialog.h \
    atcmslidertaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES     = atcmsliderplugin.cpp \
    variableandvisibilitydialog.cpp \
    atcmslidertaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmslider.pri)
