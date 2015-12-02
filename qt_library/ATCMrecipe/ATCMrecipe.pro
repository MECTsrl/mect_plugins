#-------------------------------------------------
#
# Project for ATCMrecipe library 
#
#-------------------------------------------------

TARGET = ATCMrecipe

include (../qt_library.pri)

INCLUDEPATH += .\
../ATCMlogger

QMAKE_LIBDIR += \
../ATCMlogger

LIBS += \
-lATCMlogger

SOURCES += \
recipe.cpp \
recipe_select.cpp

HEADERS += \
recipe.h \
recipe_select.h

FORMS += \
recipe.ui \
recipe_select.ui

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
