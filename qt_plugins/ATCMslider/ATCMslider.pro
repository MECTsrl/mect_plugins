TARGET      = $$qtLibraryTarget(atcmsliderplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmsliderplugin.h \
    variableandvisibilitydialog.h \
    atcmslidertaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES = \
    ../atcmpluginobject.cpp \
    atcmsliderplugin.cpp \
    variableandvisibilitydialog.cpp \
    atcmslidertaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmslider.pri)
