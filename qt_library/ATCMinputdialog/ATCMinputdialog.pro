#-------------------------------------------------
#
# Project for ATCMinputdialog library 
#
#-------------------------------------------------

TARGET = ATCMinputdialog

include (../qt_library.pri)

INCLUDEPATH += ../ATCMutility

LIBS += \
-lATCMutility

SOURCES += \
alphanumpad.cpp \
numpad.cpp

HEADERS += \
alphanumpad.h \
numpad.h

FORMS += \
alphanumpad.ui \
numpad.ui

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
