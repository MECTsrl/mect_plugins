#-------------------------------------------------
#
# Project for ATCMcommon library 
#
#-------------------------------------------------

TARGET = ATCMcommon

include (../qt_library.pri)

INCLUDEPATH += .\
.. \
../ATCMlogger \
../ATCMsystem \
../ATCMalarms \
../ATCMstore \
../ATCMtrend \
../ATCMrecipe

LIBS += \
-lATCMutility

SOURCES += \
ntpclient.cpp \
calendar.cpp \
timepopup.cpp \
global_var.cpp \
pagebrowser.cpp \
screensaver.cpp \
    global_functions.cpp \
    setstyle.cpp


HEADERS += \
ntpclient.h \
calendar.h \
timepopup.h \
global_var.h \
pagebrowser.h \
screensaver.h \
    defines.h \
    global_functions.h \
    setstyle.h


FORMS += \


# install
target.path = $${ATCM_ARM_LIBRARY_INSTALL_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INSTALL_INCPATH}
INSTALLS += target include
