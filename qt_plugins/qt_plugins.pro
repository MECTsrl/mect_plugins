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
		   ./ATCMtime/atcmtime.h \
    ATCManimation/atcmanimationplugin.h \
    ATCManimation/atcmanimationdialog.h \
    ATCManimation/atcmanimationtaskmenu.h \
    ATCMbutton/variableandvisibilitydialog.h \
    ATCMbutton/atcmbuttontaskmenu.h \
    ATCMbutton/atcmbuttonplugin.h \
    ATCMcombobox/atcmcomboboxtaskmenu.h \
    ATCMcombobox/atcmcomboboxpropertysheet.h \
    ATCMcombobox/atcmcomboboxplugin.h \
    ATCMcombobox/atcmcomboboxdialog.h \
    ATCMdate/atcmdateplugin.h \
    ATCMgraph/atcmgraphtaskmenu.h \
    ATCMgraph/atcmgraphplugin.h \
    ATCMgraph/atcmgraphdialog.h \
    ATCMlabel/variableandvisibilitydialog.h \
    ATCMlabel/atcmlabeltaskmenu.h \
    ATCMlabel/atcmlabelplugin.h \
    ATCMlcdnumber/atcmlcdnumberplugin.h \
    ATCMled/variableandvisibilitydialog.h \
    ATCMled/atcmledtaskmenu.h \
    ATCMled/atcmledplugin.h \
    ATCMprogressbar/variableandvisibilitydialog.h \
    ATCMprogressbar/atcmprogressbartaskmenu.h \
    ATCMprogressbar/atcmprogressbarplugin.h \
    ATCMslider/variableandvisibilitydialog.h \
    ATCMslider/atcmslidertaskmenu.h \
    ATCMslider/atcmsliderplugin.h \
    ATCMspinbox/variableandvisibilitydialog.h \
    ATCMspinbox/atcmspinboxtaskmenu.h \
    ATCMspinbox/atcmspinboxplugin.h \
    ATCMtime/atcmtimeplugin.h

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
		   ./ATCMtime/atcmtime.cpp \
    ATCManimation/atcmanimationtaskmenu.cpp \
    ATCManimation/atcmanimationplugin.cpp \
    ATCManimation/atcmanimationdialog.cpp \
    ATCMbutton/variableandvisibilitydialog.cpp \
    ATCMbutton/atcmbuttontaskmenu.cpp \
    ATCMbutton/atcmbuttonplugin.cpp \
    ATCMcombobox/atcmcomboboxtaskmenu.cpp \
    ATCMcombobox/atcmcomboboxpropertysheet.cpp \
    ATCMcombobox/atcmcomboboxplugin.cpp \
    ATCMcombobox/atcmcomboboxdialog.cpp \
    ATCMdate/atcmdateplugin.cpp \
    ATCMgraph/atcmgraphtaskmenu.cpp \
    ATCMgraph/atcmgraphplugin.cpp \
    ATCMgraph/atcmgraphdialog.cpp \
    ATCMlabel/variableandvisibilitydialog.cpp \
    ATCMlabel/atcmlabeltaskmenu.cpp \
    ATCMlabel/atcmlabelplugin.cpp \
    ATCMlcdnumber/atcmlcdnumberplugin.cpp \
    ATCMled/variableandvisibilitydialog.cpp \
    ATCMled/atcmledtaskmenu.cpp \
    ATCMled/atcmledplugin.cpp \
    ATCMprogressbar/variableandvisibilitydialog.cpp \
    ATCMprogressbar/atcmprogressbartaskmenu.cpp \
    ATCMprogressbar/atcmprogressbarplugin.cpp \
    ATCMslider/variableandvisibilitydialog.cpp \
    ATCMslider/atcmslidertaskmenu.cpp \
    ATCMslider/atcmsliderplugin.cpp \
    ATCMspinbox/variableandvisibilitydialog.cpp \
    ATCMspinbox/atcmspinboxtaskmenu.cpp \
    ATCMspinbox/atcmspinboxplugin.cpp \
    ATCMtime/atcmtimeplugin.cpp

RESOURCES += atcmicons.qrc \ 
    ATCManimation/icons.qrc \
    ATCMbutton/icons.qrc \
    ATCMcombobox/icons.qrc \
    ATCMdate/icons.qrc \
    ATCMgraph/icons.qrc \
    ATCMlabel/icons.qrc \
    ATCMlcdnumber/icons.qrc \
    ATCMled/icons.qrc \
    ATCMprogressbar/icons.qrc \
    ATCMslider/icons.qrc \
    ATCMspinbox/icons.qrc \
    ATCMtime/icons.qrc

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
    ATCMtime/atcmtime.png
