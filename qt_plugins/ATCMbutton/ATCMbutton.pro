TARGET      = $$qtLibraryTarget(atcmbuttonplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmbuttonplugin.h \
    variableandvisibilitydialog.h \
    atcmbuttontaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES = \
    ../atcmpluginobject.cpp \
    atcmbuttonplugin.cpp \
    variableandvisibilitydialog.cpp \
    atcmbuttontaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmbutton.pri)
