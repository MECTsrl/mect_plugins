TARGET      = $$qtLibraryTarget(atcmgraphplugin)

include (../qt_plugins.pri)

LIBS += -lqwt

CONFIG += QwtDll

INCLUDEPATH += $${ATCM_QWT_INCL}
QMAKE_LIBDIR += $${ATCM_QWT_LIB}

INCLUDEPATH += 	../ATCMplugineditor

HEADERS     = atcmgraphplugin.h \
    atcmgraphdialog.h \
    atcmgraphtaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES     = atcmgraphplugin.cpp \
    atcmgraphdialog.cpp \
    atcmgraphtaskmenu.cpp \
   ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS    += target

include(atcmgraph.pri)
