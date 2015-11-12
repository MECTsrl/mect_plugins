#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TARGET = ATCMlogger

include (../qt_library.pri)

INCLUDEPATH += ../ATCMcommon

LIBS += \
ATCMutility

SOURCES += \
hmi_logger.cpp

HEADERS += \
hmi_logger.h

FORMS += \

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
