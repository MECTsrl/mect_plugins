QT += network

TEMPLATE      = subdirs
SUBDIRS       =	\
qt_library \
qt_plugins \
qt_templates

#example

include(./qt_environment.pri)

# install
sources.files = atcmplugin.h
sources.path = $${ATCM_ARM_PLUGINS_INCPATH}
INSTALLS += sources

HEADERS += \
    qt_plugins/ATCManimation/atcmanimationplugin.h \
    qt_plugins/ATCManimation/atcmanimationdialog.h \
    qt_plugins/ATCManimation/atcmanimationtaskmenu.h \
    qt_plugins/ATCMbutton/variableandvisibilitydialog.h \
    qt_plugins/ATCMbutton/atcmbuttontaskmenu.h \
    qt_plugins/ATCMbutton/atcmbuttonplugin.h \
    qt_plugins/ATCMcombobox/atcmcomboboxtaskmenu.h \
    qt_plugins/ATCMcombobox/atcmcomboboxpropertysheet.h \
    qt_plugins/ATCMcombobox/atcmcomboboxplugin.h \
    qt_plugins/ATCMcombobox/atcmcomboboxdialog.h \
    qt_plugins/ATCMdate/atcmdateplugin.h \
    qt_plugins/ATCMgraph/atcmgraphtaskmenu.h \
    qt_plugins/ATCMgraph/atcmgraphplugin.h \
    qt_plugins/ATCMgraph/atcmgraphdialog.h \
    qt_plugins/ATCMlabel/variableandvisibilitydialog.h \
    qt_plugins/ATCMlabel/atcmlabeltaskmenu.h \
    qt_plugins/ATCMlabel/atcmlabelplugin.h \
    qt_plugins/ATCMlcdnumber/atcmlcdnumberplugin.h \
    qt_plugins/ATCMled/variableandvisibilitydialog.h \
    qt_plugins/ATCMled/atcmledtaskmenu.h \
    qt_plugins/ATCMled/atcmledplugin.h \
    qt_plugins/ATCMprogressbar/variableandvisibilitydialog.h \
    qt_plugins/ATCMprogressbar/atcmprogressbartaskmenu.h \
    qt_plugins/ATCMprogressbar/atcmprogressbarplugin.h \
    qt_plugins/ATCMslider/variableandvisibilitydialog.h \
    qt_plugins/ATCMslider/atcmslidertaskmenu.h \
    qt_plugins/ATCMslider/atcmsliderplugin.h \
    qt_plugins/ATCMspinbox/variableandvisibilitydialog.h \
    qt_plugins/ATCMspinbox/atcmspinboxtaskmenu.h \
    qt_plugins/ATCMspinbox/atcmspinboxplugin.h \
    qt_plugins/ATCMtime/atcmtimeplugin.h \
    qt_plugins/ATCMplugineditor/resourceeditor.h \
    qt_plugins/ATCMplugineditor/crosstablemanager.h \
    qt_plugins/ATCMplugineditor/crosstableeditor.h \
    qt_plugins/ATCMplugineditor/crosstableeditfactory.h

SOURCES += \
    qt_plugins/ATCManimation/atcmanimationtaskmenu.cpp \
    qt_plugins/ATCManimation/atcmanimationplugin.cpp \
    qt_plugins/ATCManimation/atcmanimationdialog.cpp \
    qt_plugins/ATCMbutton/variableandvisibilitydialog.cpp \
    qt_plugins/ATCMbutton/atcmbuttontaskmenu.cpp \
    qt_plugins/ATCMbutton/atcmbuttonplugin.cpp \
    qt_plugins/ATCMcombobox/atcmcomboboxtaskmenu.cpp \
    qt_plugins/ATCMcombobox/atcmcomboboxpropertysheet.cpp \
    qt_plugins/ATCMcombobox/atcmcomboboxplugin.cpp \
    qt_plugins/ATCMcombobox/atcmcomboboxdialog.cpp \
    qt_plugins/ATCMdate/atcmdateplugin.cpp \
    qt_plugins/ATCMgraph/atcmgraphtaskmenu.cpp \
    qt_plugins/ATCMgraph/atcmgraphplugin.cpp \
    qt_plugins/ATCMgraph/atcmgraphdialog.cpp \
    qt_plugins/ATCMlabel/variableandvisibilitydialog.cpp \
    qt_plugins/ATCMlabel/atcmlabeltaskmenu.cpp \
    qt_plugins/ATCMlabel/atcmlabelplugin.cpp \
    qt_plugins/ATCMlcdnumber/atcmlcdnumberplugin.cpp \
    qt_plugins/ATCMled/variableandvisibilitydialog.cpp \
    qt_plugins/ATCMled/atcmledtaskmenu.cpp \
    qt_plugins/ATCMled/atcmledplugin.cpp \
    qt_plugins/ATCMprogressbar/variableandvisibilitydialog.cpp \
    qt_plugins/ATCMprogressbar/atcmprogressbartaskmenu.cpp \
    qt_plugins/ATCMprogressbar/atcmprogressbarplugin.cpp \
    qt_plugins/ATCMslider/variableandvisibilitydialog.cpp \
    qt_plugins/ATCMslider/atcmslidertaskmenu.cpp \
    qt_plugins/ATCMslider/atcmsliderplugin.cpp \
    qt_plugins/ATCMspinbox/variableandvisibilitydialog.cpp \
    qt_plugins/ATCMspinbox/atcmspinboxtaskmenu.cpp \
    qt_plugins/ATCMspinbox/atcmspinboxplugin.cpp \
    qt_plugins/ATCMtime/atcmtimeplugin.cpp \
    qt_plugins/ATCMplugineditor/resourceeditor.cpp \
    qt_plugins/ATCMplugineditor/crosstablemanager.cpp \
    qt_plugins/ATCMplugineditor/crosstableeditor.cpp \
    qt_plugins/ATCMplugineditor/crosstableeditfactory.cpp

RESOURCES += \
    qt_plugins/ATCManimation/icons.qrc \
    qt_plugins/ATCMbutton/icons.qrc \
    qt_plugins/ATCMcombobox/icons.qrc \
    qt_plugins/ATCMdate/icons.qrc \
    qt_plugins/ATCMgraph/icons.qrc \
    qt_plugins/ATCMlabel/icons.qrc \
    qt_plugins/ATCMlcdnumber/icons.qrc \
    qt_plugins/ATCMled/icons.qrc \
    qt_plugins/ATCMprogressbar/icons.qrc \
    qt_plugins/ATCMslider/icons.qrc \
    qt_plugins/ATCMspinbox/icons.qrc \
    qt_plugins/ATCMtime/icons.qrc
