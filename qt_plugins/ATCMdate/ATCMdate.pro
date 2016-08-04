TARGET      = $$qtLibraryTarget(atcmdateplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmdateplugin.h

SOURCES = \
    ../atcmplugin.cpp \
    atcmdateplugin.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmdate.pri)
