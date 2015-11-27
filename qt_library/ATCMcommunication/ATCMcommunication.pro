#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TARGET = ATCMcommunication

include (../qt_library.pri)

INCLUDEPATH += ../ATCMutility

LIBS += \
-lATCMutility

SOURCES += \
cross_table_utility.c \
io_layer_comm.cpp

HEADERS += \
app_var_list.h \
cross_table_utility.h \
io_layer_comm.h

# install
target.path = $${ATCM_ARM_LIBRARY_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INCPATH}
INSTALLS += target include
