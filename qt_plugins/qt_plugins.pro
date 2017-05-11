#-------------------------------------------------
#
# Project file for plugins library
#
#-------------------------------------------------

include(../qt_environment.pri)

TARGET = ATCMplugin

TEMPLATE    = lib
DEFINES+="TARGET_ARM"

INCLUDEPATH = . \
               ../qt_library/ATCMcommon \
               ../qt_library/ATCMutility \
               ../qt_library/ATCMcommunication \
               ../qt_library/ATCMinputdialog \
               ./ATCMstyle \
		$${INCLUDEPATH}

QMAKE_LIBDIR += ../../lib \
               ../qt_library/ATCMcommon \
               ../qt_library/ATCMutility \
               ../qt_library/ATCMcommunication \
               ../qt_library/ATCMinputdialog \
               $${ATCM_ARM_LIBRARY_LIBPATH}

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
target.path = $${ATCM_ARM_PLUGINS_INSTALL_LIBPATH}
include.files = $$HEADERS atcmplugin.h
include.path = $${ATCM_ARM_PLUGINS_INSTALL_INCPATH}
INSTALLS += target include

OTHER_FILES += \
    ATCManimation/ATCManimation.pro \
    ATCManimation/atcmanimation.pri \
    ATCManimation/atcmanimation.png \
    ATCMbutton/ATCMbutton.pro \
    ATCMbutton/atcmbutton.pri \
    ATCMbutton/atcmbutton.png \
    ATCMcombobox/down.png \
    ATCMcombobox/ATCMcombobox.pro \
    ATCMcombobox/atcmcombobox.pri \
    ATCMcombobox/atcmcombobox.png \
    ATCMdate/ATCMdate.pro \
    ATCMdate/atcmdate.pri \
    ATCMdate/atcmdate.png \
    ATCMgraph/ATCMgraph.pro \
    ATCMgraph/atcmgraph.pri \
    ATCMgraph/atcmgraph.png \
    ATCMlabel/ATCMlabel.pro \
    ATCMlabel/atcmlabel.pri \
    ATCMlabel/atcmlabel.png \
    ATCMlcdnumber/ATCMlcdnumber.pro \
    ATCMlcdnumber/atcmlcdnumber.pri \
    ATCMlcdnumber/atcmlcdnumber.png \
    ATCMled/unk.png \
    ATCMled/on.png \
    ATCMled/off.png \
    ATCMled/ATCMled.pro \
    ATCMled/atcmled.pri \
    ATCMled/atcmled.png \
    ATCMprogressbar/ATCMprogressbar.pro \
    ATCMprogressbar/atcmprogressbar.pri \
    ATCMprogressbar/atcmprogressbar.png \
    ATCMslider/marker.png \
    ATCMslider/ATCMslider.pro \
    ATCMslider/atcmslider.pri \
    ATCMslider/atcmslider.png \
    ATCMspinbox/right.png \
    ATCMspinbox/left.png \
    ATCMspinbox/ATCMspinbox.pro \
    ATCMspinbox/atcmspinbox.pri \
    ATCMspinbox/atcmspinbox.png \
    ATCMtime/ATCMtime.pro \
    ATCMtime/atcmtime.pri \
    ATCMtime/atcmtime.png \
    ATCMplugineditor/ATCMplugineditor.pro
