#-------------------------------------------------
#
# Project for ATCMstore library 
#
#-------------------------------------------------

TARGET = ATCMstore

include (../qt_library.pri)

INCLUDEPATH += \
../ATCMlogger

QMAKE_LIBDIR += \
../ATCMlogger

LIBS += \
-lATCMlogger

SOURCES += \
store.cpp \
store_filter.cpp

HEADERS += \
store.h \
store_filter.h

FORMS += \
store.ui \
store_filter.ui

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
