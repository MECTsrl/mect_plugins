TARGET = ATCMplugin
TEMPLATE = lib
CONFIG += staticlib

DEFINES+="TARGET_ARM"

include(../qt_designer_environment.pri)

INCLUDEPATH += . \
			   /imx_mect/trunk/imx28/ltib/rootfs/usr/src/linux/include \
			   /imx_mect/trunk/imx28/ltib/rootfs/usr/include \
                           ../qt_library/ATCMcommon \
                           ../qt_library/ATCMutility \
                           ../qt_library/ATCMcommunication \
                           ../qt_library/ATCMinputdialog \
                           ./ATCMstyle

QMAKE_LIBDIR += ../../lib \
		   /usr/local/Trolltech/Qt-qvfb-version/lib \
		   /imx_mect/trunk/imx28/ltib/rootfs/usr/lib \
                   $${ATCM_ARM_LIBRARY_LIBPATH} \
                   ../qt_library/ATCMcommon \
                   ../qt_library/ATCMutility \
                   ../qt_library/ATCMcommunication \
                   ../qt_library/ATCMinputdialog

LIBS += \
-lATCMcommon \
-lATCMutility \
-lATCMcommunication \
-lATCMinputdialog

HEADERS += \
		   ./ATCMlabel/atcmlabel.h \
		   ./ATCMcombobox/atcmcombobox.h \
		   ./ATCMspinbox/atcmspinbox.h \
		   ./ATCMlcdnumber/atcmlcdnumber.h \
		   ./ATCMbutton/atcmbutton.h \
		   ./ATCMslider/atcmslider.h \
		   ./ATCMprogressbar/atcmprogressbar.h \
		   ./ATCMled/atcmled.h \
		   ./ATCManimation/atcmanimation.h \
                   ./ATCMstyle/atcmstyle.h \
                   ./ATCMgraph/atcmgraph.h \
                   ./ATCMdate/atcmdate.h \
                   ./ATCMtime/atcmtime.h

SOURCES += \
		   ./ATCMlabel/atcmlabel.cpp \
		   ./ATCMcombobox/atcmcombobox.cpp \
		   ./ATCMspinbox/atcmspinbox.cpp \
		   ./ATCMlcdnumber/atcmlcdnumber.cpp \
		   ./ATCMbutton/atcmbutton.cpp \
		   ./ATCMslider/atcmslider.cpp \
		   ./ATCMprogressbar/atcmprogressbar.cpp \
		   ./ATCMled/atcmled.cpp \
		   ./ATCManimation/atcmanimation.cpp \
                   ./ATCMstyle/atcmstyle.cpp \
                   ./ATCMgraph/atcmgraph.cpp \
                   ./ATCMdate/atcmdate.cpp \
                   ./ATCMtime/atcmtime.cpp

RESOURCES += atcmicons.qrc 

# install
target.path = $${ATCM_ARM_PLUGINS_LIBPATH}
include.files = $$HEADERS atcmplugin.h
include.path = $${ATCM_ARM_PLUGINS_INCPATH}
INSTALLS += target include
