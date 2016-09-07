CONFIG      += designer plugin resources
TEMPLATE    = lib

include(../qt_designer_environment.pri)

INCLUDEPATH = . \
               ../ \
               ../../qt_library/ATCMcommon \
               ../../qt_library/ATCMutility \
               ../../qt_library/ATCMcommunication \
               ../../qt_library/ATCMinputdialog \
               .././ATCMstyle \
	       $${INCLUDEPATH}

QMAKE_LIBDIR += ../../lib \
               $${ATCM_ARM_LIBRARY_LIBPATH} \
               ../../qt_library/ATCMcommon \
               ../../qt_library/ATCMutility \
               ../../qt_library/ATCMcommunication \
               ../../qt_library/ATCMinputdialog
