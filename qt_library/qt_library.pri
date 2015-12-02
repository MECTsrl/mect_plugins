#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TEMPLATE = lib
#CONFIG += staticlib

include(../qt_designer_environment.pri)

INCLUDEPATH += .\
		   $${ATCM_QWT_INCL} \
                   ../ATCMutility \
                   ../ATCMcommunication \
                   ../ATCMcommon \
                   ../ATCMinputdialog

QMAKE_LIBDIR += \
                   ../ATCMutility \
                   ../ATCMcommunication \
                   ../ATCMcommon \
                   ../ATCMinputdialog

store {
	DEFINES+=ENABLE_STORE
}

alarms {
	DEFINES+=ENABLE_ALARMS
}

trend {
	DEFINES+=ENABLE_TREND
}

recipe {
	DEFINES+=ENABLE_RECIPE
}
