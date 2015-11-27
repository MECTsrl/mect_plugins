#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TEMPLATE = lib
#CONFIG += staticlib

include(../qt_designer_environment.pri)

INCLUDEPATH += .\
                   ../ATCMutility \
                   ../ATCMcommunication \
                   ../ATCMcommon \
                   ../ATCMinputdialog

QMAKE_LIBDIR += \
		   $${ATCM_SRC_LIBRARY_INCPATH}ATCMutility \
		   $${ATCM_SRC_LIBRARY_INCPATH}ATCMcommunication \
		   $${ATCM_SRC_LIBRARY_INCPATH}ATCMinputdialog

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
