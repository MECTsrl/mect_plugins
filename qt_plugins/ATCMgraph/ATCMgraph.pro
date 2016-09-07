TARGET      = $$qtLibraryTarget(atcmgraphplugin)

include (../qt_plugins.pri)
include (../../qt_designer_environment.pri)

INCLUDEPATH += $${ATCM_QWT_INCL}
QMAKE_LIBDIR += $${ATCM_QWT_LIB}

INCLUDEPATH += 	../ATCMplugineditor

LIBS += -lqwt

CONFIG += QwtDll

HEADERS =  \
    ../atcmplugin.h \
    atcmgraphplugin.h \
    atcmgraphdialog.h \
    atcmgraphtaskmenu.h \
    ../ATCMplugineditor/crosstableeditor.h

SOURCES = \
    ../atcmpluginobject.cpp \
    atcmgraphplugin.cpp \
    atcmgraphdialog.cpp \
    atcmgraphtaskmenu.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmgraph.pri)
