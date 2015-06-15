#-------------------------------------------------
#
# Project for ATCMcommunication library 
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG += staticlib

include(../qt_designer_environment.pri)

INCLUDEPATH += .\
			   /imx_mect/trunk/imx28/ltib/rootfs_full_qt/usr/src/linux/include \
			   /imx_mect/trunk/imx28/ltib/rootfs_full_qt/usr/include \
                           ../ATCMutility \
                           ../ATCMcommunication \
                           ../ATCMinputdialog

QMAKE_LIBDIR += \
		   /imx_mect/trunk/imx28/ltib/rootfs_full_qt/usr/lib \
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
