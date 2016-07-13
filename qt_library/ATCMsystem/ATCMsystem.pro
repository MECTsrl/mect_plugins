#-------------------------------------------------
#
# Project for ATCMsystem library 
#
#-------------------------------------------------

TARGET = ATCMsystem

DEFINES+="TARGET_ARM"


include (../qt_library.pri)

INCLUDEPATH += .\
../ATCMlogger \
../ATCMalarms \
../ATCMstore \
../ATCMtrend \
../ATCMrecipe \
../../qt_plugins \
../../qt_plugins/ATCMlabel \
../../qt_plugins/ATCMcombobox \
../../qt_plugins/ATCMspinbox \
../../qt_plugins/ATCMbutton \
../../qt_plugins/ATCMslider \
../../qt_plugins/ATCMprogressbar \
../../qt_plugins/ATCMled \
../../qt_plugins/ATCManimation \
../../qt_plugins/ATCMgraph \
../../qt_plugins/ATCMdate \
../../qt_plugins/ATCMtime


QMAKE_LIBDIR += \
../ATCMlogger

LIBS += \
-lATCMlogger

SOURCES += \
system_ini.cpp \
data_manager.cpp \
display_settings.cpp \
display_test.cpp \
info.cpp \
item_selector.cpp \
menu.cpp \
options.cpp \
time_set.cpp \
net_conf.cpp \
page0.cpp \
main.cpp

HEADERS += \
system_ini.h \
data_manager.h \
display_settings.h \
display_test.h \
info.h \
item_selector.h \
menu.h \
options.h \
time_set.h \
net_conf.h \
page0.h \
main.h

FORMS += \
system_ini.ui \
data_manager.ui \
display_settings.ui \
display_test.ui \
info.ui \
item_selector.ui \
menu.ui \
options.ui \
time_set.ui \
net_conf.ui \
page0.ui

# install
target.path = $${ATCM_ARM_LIBRARY_INSTALL_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INSTALL_INCPATH}
INSTALLS += target include

RESOURCES += \
    libicons.qrc
