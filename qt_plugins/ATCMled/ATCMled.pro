TARGET      = $$qtLibraryTarget(atcmledplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmledplugin.h \
    variableandvisibilitydialog.h \
    atcmledtaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES = \
    ../atcmpluginobject.cpp \
    atcmledplugin.cpp \
    variableandvisibilitydialog.cpp \
    atcmledtaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmled.pri)
