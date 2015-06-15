TARGET      = $$qtLibraryTarget(atcmspinboxplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS     = atcmspinboxplugin.h \
				atcmspinboxtaskmenu.h \
				variableandvisibilitydialog.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES     = atcmspinboxplugin.cpp \
				atcmspinboxtaskmenu.cpp \
				variableandvisibilitydialog.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(atcmspinbox.pri)
