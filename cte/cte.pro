# CTE: Mect CrossTable Editor plugin

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
    queryportnode.h \
    searchvariable.h

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
    queryportnode.cpp \
    searchvariable.cpp

FORMS += \
    ctedit.ui \  
    cteerrorlist.ui \
    mectsettings.ui \
    trendeditor.ui \
    messagelist.ui \
    queryportnode.ui \
    searchvariable.ui

ATCM_VERSION = 3.2.8

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=C:/qt-creator-2.8.0-rc-src

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=C:/Qt485/desktop/

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
# USE_USER_DESTDIR = yes

PROVIDER = Mect

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

LIBS += -L$$DESTDIR -L$$IDE_APP_PATH -lProjectExplorer

RESOURCES += \
    qtc.qrc

OTHER_FILES += \
    CTE.qss

#other_files.files = $$OTHER_FILES
#other_files.path = $$DESTDIR
#INSTALLS += other_files
