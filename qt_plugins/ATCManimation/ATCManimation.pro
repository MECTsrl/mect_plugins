TARGET      = $$qtLibraryTarget(atcmanimationplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS =  \
    ../atcmplugin.h \
    atcmanimationplugin.h \
    atcmanimationtaskmenu.h \
    atcmanimationdialog.h \
    ../ATCMplugineditor/crosstableeditor.h \
    ../ATCMplugineditor/resourceeditor.h

SOURCES = \
    atcmanimationplugin.cpp \
    atcmanimationtaskmenu.cpp \
    atcmanimationdialog.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp \
    ../ATCMplugineditor/resourceeditor.cpp \
    ../atcmpluginobject.cpp


RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmanimation.pri)
