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
global_var.cpp \
pagebrowser.cpp \
screensaver.cpp \
    global_functions.cpp \
    setstyle.cpp


HEADERS += \
global_var.h \
pagebrowser.h \
screensaver.h \
    defines.h \
    global_functions.h \
    setstyle.h


FORMS += \


# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
