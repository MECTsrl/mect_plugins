CONFIG      += designer plugin debug_and_release
TEMPLATE    = lib

include(../qt_designer_environment.pri)

INCLUDEPATH += \
		../../qt_library/ATCMutility \
		../../qt_library/ATCMcommunication \
		../../qt_library/ATCMinputdialog \
		../ATCMstyle
