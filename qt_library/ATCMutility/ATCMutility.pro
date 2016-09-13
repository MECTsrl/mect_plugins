#-------------------------------------------------
#
# Project file for internals library
#
#-------------------------------------------------

QT       -= core gui

TARGET = ATCMutility

include (../qt_library.pri)

SOURCES += \
app_config.c	\
app_logprint.c	\
app_usb.c       \
store_extract.c \
    utility.c

HEADERS += \
common.h		\
app_config.h	\
app_logprint.h	\
app_usb.h       \
store_extract.h \
    utility.h

#target.path = $$[QT_INSTALL_PLUGINS]/designer

# install
target.path = $${ATCM_ARM_LIBRARY_INSTALL_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INSTALL_INCPATH}
INSTALLS += target include
