#include(../qt_designer_environment.pri)
#include(../qt_templates/ATCM-template-project/qt_environment.pri)
include(../qt_environment.pri)

TEMPLATE      = subdirs
SUBDIRS       =	\
                                ATCMcommon \
                                ATCMutility \
				ATCMcommunication \
                                ATCMinputdialog \
                                ATCMlogger
