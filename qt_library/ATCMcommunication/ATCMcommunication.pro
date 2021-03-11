#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TARGET = ATCMcommunication

include (../qt_library.pri)

LIBS += \
-lATCMutility

SOURCES += \
hmi_plc.c \
cross_table_utility.c \
io_layer_comm.cpp

HEADERS += \
app_var_list.h \
hmi_plc.h \
cross_table_utility.h \
io_layer_comm.h

# install
target.path = $${ATCM_ARM_LIBRARY_INSTALL_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INSTALL_INCPATH}
INSTALLS += target include
