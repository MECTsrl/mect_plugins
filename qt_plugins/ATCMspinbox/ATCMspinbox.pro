TARGET      = $$qtLibraryTarget(atcmspinboxplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmspinboxplugin.h \
    atcmspinboxtaskmenu.h \
    variableandvisibilitydialog.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES = \
    ../atcmplugin.cpp \
    atcmspinboxplugin.cpp \
    atcmspinboxtaskmenu.cpp \
    variableandvisibilitydialog.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmspinbox.pri)
