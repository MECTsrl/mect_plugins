#-------------------------------------------------
#
# Project for ATCMalarms library 
#
#-------------------------------------------------

TARGET = ATCMalarms

include (../qt_library.pri)

INCLUDEPATH += .\
../ATCMlogger \
../ATCMsystem

QMAKE_LIBDIR += \
../ATCMlogger

LIBS += \
-lATCMlogger \
-lATCMcommon

SOURCES += \
alarms.cpp \
alarms_history.cpp

HEADERS += \
alarms.h \
alarms_history.h

FORMS += \
alarms.ui \
alarms_history.ui

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
