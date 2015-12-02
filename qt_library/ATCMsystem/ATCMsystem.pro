#-------------------------------------------------
#
# Project for ATCMsystem library 
#
#-------------------------------------------------

TARGET = ATCMsystem

include (../qt_library.pri)

INCLUDEPATH += .\
../ATCMlogger \
../ATCMalarms \
../ATCMstore \
../ATCMtrend \
../ATCMrecipe

QMAKE_LIBDIR += \
../ATCMlogger

LIBS += \
-lATCMlogger

SOURCES += \
buzzer_settings.cpp \
comm_status.cpp \
commpar_rtu.cpp \
commpar_tcp.cpp \
commpar_tcprtu.cpp \
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
commpar_rtu.h \
commpar_tcp.h \
commpar_tcprtu.h \
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
commpar_rtu.ui \
commpar_tcp.ui \
commpar_tcprtu.ui \
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
