TARGET      = $$qtLibraryTarget(atcmprogressbarplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmprogressbarplugin.h \
    variableandvisibilitydialog.h \
    atcmprogressbartaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES = \
    ../atcmplugin.cpp \
    atcmprogressbarplugin.cpp \
    variableandvisibilitydialog.cpp \
    atcmprogressbartaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmprogressbar.pri)
