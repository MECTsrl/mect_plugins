TARGET      = $$qtLibraryTarget(atcmledplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS     = atcmledplugin.h \ 
    variableandvisibilitydialog.h \
    atcmledtaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES     = atcmledplugin.cpp \ 
    variableandvisibilitydialog.cpp \
    atcmledtaskmenu.cpp \
   ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(atcmled.pri)
