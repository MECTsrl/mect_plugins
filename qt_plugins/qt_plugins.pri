#include(../qt_environment.pri)

CONFIG      += designer plugin debug_and_release

INCLUDEPATH += . \
        	$${QT_ROOTFS}/usr/include \
               ../qt_library/ATCMcommon \
               ../qt_library/ATCMutility \
               ../qt_library/ATCMcommunication \
               ../qt_library/ATCMinputdialog \
               ./ATCMstyle

QMAKE_LIBDIR += ../../lib \
               $${ATCM_ARM_LIBRARY_LIBPATH} \
               ../qt_library/ATCMcommon \
               ../qt_library/ATCMutility \
               ../qt_library/ATCMcommunication \
               ../qt_library/ATCMinputdialog

