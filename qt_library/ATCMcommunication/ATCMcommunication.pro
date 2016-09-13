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
io_layer_comm.cpp \
    cross_table_utility.cpp

HEADERS += \
app_var_list.h \
cross_table_utility.h \
io_layer_comm.h \
    protocol.h

# install
target.path = $${ATCM_ARM_LIBRARY_INSTALL_LIBPATH}
include.files = $$HEADERS
include.path = $${ATCM_ARM_LIBRARY_INSTALL_INCPATH}
INSTALLS += target include
