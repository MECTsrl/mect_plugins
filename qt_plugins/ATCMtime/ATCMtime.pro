TARGET      = $$qtLibraryTarget(atcmtimeplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS = \
    ../atcmplugin.h \
    atcmtimeplugin.h

SOURCES = \
    ../atcmplugin.cpp \
    atcmtimeplugin.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmtime.pri)
