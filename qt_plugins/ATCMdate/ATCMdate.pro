TARGET      = $$qtLibraryTarget(atcmdateplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS     = atcmdateplugin.h

SOURCES     = atcmdateplugin.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmdate.pri)
