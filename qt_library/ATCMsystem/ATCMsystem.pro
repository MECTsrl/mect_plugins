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
buzzer_settings.cpp \
comm_status.cpp \
system_ini.cpp \
data_manager.cpp \
datalog_set.cpp \
display_settings.cpp \
info.cpp \
item_selector.cpp \
menu.cpp \
options.cpp \
sgdd.cpp \
time_set.cpp \
page0.cpp \
main.cpp

HEADERS += \
buzzer_settings.h \
comm_status.h \
system_ini.h \
data_manager.h \
datalog_set.h \
display_settings.h \
info.h \
item_selector.h \
menu.h \
options.h \
sgdd.h \
time_set.h \
page0.h \
main.h

FORMS += \
buzzer_settings.ui \
comm_status.ui \
system_ini.ui \
data_manager.ui \
datalog_set.ui \
display_settings.ui \
info.ui \
item_selector.ui \
menu.ui \
options.ui \
sgdd.ui \
time_set.ui \
page0.ui

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
