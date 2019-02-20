include(C:/qt-creator-2.8.0-rc-src/src/qtcreatorplugin.pri)

HEADERS += \
    cteplugin.h \
    parser.h \
    ctedit.h \
    utils.h \  
    cteerrdef.h \
    cteerrorlist.h \
    mectsettings.h \
    trendeditor.h \
    ctecommon.h \
    config_mpnc.h \
    config_mpne.h \
    cteUtils.h \
    messagelist.h \
    queryportnode.h

SOURCES += \
    cteplugin.cpp \
    parser.c \
    ctedit.cpp \
    utils.cpp \  
    cteerrorlist.cpp \
    mectsettings.cpp \
    trendeditor.cpp \
    config_mpnc.cpp \
    config_mpne.cpp \
    cteUtils.cpp \
    messagelist.cpp \
    queryportnode.cpp

FORMS += \
    ctedit.ui \  
    cteerrorlist.ui \
    mectsettings.ui \
    trendeditor.ui \
    messagelist.ui \
    queryportnode.ui

#DESTDIR = C:/Qt485/desktop//lib/qtcreator/plugins/QtProject
DESTDIR = destdir
ATCM_VERSION = 3.2.0

testcreator.files = destdir/CTE.dll destdir/CTE.pluginspec CTE.qss
testcreator.path = C:/Qt485/desktop/lib/qtcreator/plugins/QtProject/

INSTALLS += testcreator


LIBS += \
    -LC:/Qt485/desktop/lib/qtcreator/plugins/QtProject \
    -LC:/Qt485/desktop/bin \
    -lProjectExplorer

RESOURCES += \
    qtc.qrc

OTHER_FILES +=
