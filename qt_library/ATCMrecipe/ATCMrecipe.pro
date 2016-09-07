#-------------------------------------------------
#
# Project for ATCMrecipe library 
#
#-------------------------------------------------

TARGET = ATCMrecipe

include (../qt_library.pri)

INCLUDEPATH += .\
../ATCMlogger \
../ATCMcommon

QMAKE_LIBDIR += \
../ATCMlogger \
../ATCMcommon

LIBS += \
-lATCMlogger \
-lATCMcommon

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
target.path = $${ATCM_ARM_LIBRARY_INSTALL_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INSTALL_INCPATH}
INSTALLS += target include
