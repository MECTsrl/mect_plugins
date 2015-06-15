TARGET      = $$qtLibraryTarget(atcmlcdnumberplugin)

include (../qt_plugins.pri)

HEADERS     = atcmlcdnumberplugin.h
SOURCES     = atcmlcdnumberplugin.cpp
RESOURCES   = icons.qrc

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(atcmlcdnumber.pri)
