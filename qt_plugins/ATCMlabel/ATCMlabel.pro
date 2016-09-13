TARGET      = $$qtLibraryTarget(atcmlabelplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS     = atcmlabelplugin.h \
    variableandvisibilitydialog.h \
    atcmlabeltaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES     = atcmlabelplugin.cpp \
    variableandvisibilitydialog.cpp \
    atcmlabeltaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmlabel.pri)
