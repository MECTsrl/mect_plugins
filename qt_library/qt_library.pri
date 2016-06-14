#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TEMPLATE = lib
#CONFIG += staticlib

include(../qt_environment.pri)

INCLUDEPATH = .\
		   $${ATCM_QWT_INCL} \
                   ../ATCMutility \
                   ../ATCMcommunication \
                   ../ATCMcommon \
                   ../ATCMinputdialog \
                   ../ATCMalarms \
                   ../ATCMsystem \
		   $${INCLUDEPATH}

QMAKE_LIBDIR += \
                   ../ATCMutility \
                   ../ATCMcommunication \
                   ../ATCMcommon \
                   ../ATCMinputdialog
