#-------------------------------------------------
#
# Project for ATCMtrend library 
#
#-------------------------------------------------

TARGET = ATCMtrend

include (../qt_library.pri)

INCLUDEPATH += .\
../ATCMlogger \
../ATCMsystem

QMAKE_LIBDIR += \
../ATCMlogger

LIBS += \
-lATCMlogger

SOURCES += \
trend.cpp \
trend_option.cpp \
trend_other.cpp \
trend_range.cpp

HEADERS += \
trend.h \
trend_option.h \
trend_other.h \
trend_range.h

FORMS += \
trend.ui \
trend_option.ui \
trend_other.ui \
trend_range.ui

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
