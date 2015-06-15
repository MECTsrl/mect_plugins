#-------------------------------------------------
#
# Project file for internals library
#
#-------------------------------------------------

QT       -= core gui

TARGET = ATCMutility

include (../qt_library.pri)

SOURCES += \
app_cfg_file.c	\
app_config.c	\
app_logprint.c	\
app_usb.c       \
store_extract.c \
    utility.c

HEADERS += \
common.h		\
app_cfg_file.h	\
app_config.h	\
app_logprint.h	\
app_usb.h       \
store_extract.h \
    utility.h

#target.path = $$[QT_INSTALL_PLUGINS]/designer
#INSTALLS    += target

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
