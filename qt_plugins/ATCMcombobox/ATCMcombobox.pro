TARGET      = $$qtLibraryTarget(atcmcomboboxplugin)

include (../qt_plugins.pri)

INCLUDEPATH += ../ATCMplugineditor

HEADERS     = atcmcomboboxplugin.h \
				atcmcomboboxtaskmenu.h \
				atcmcomboboxdialog.h \
    ../ATCMplugineditor/crosstableeditor.h \
    #atcmcomboboxpropertysheet.h

SOURCES     = atcmcomboboxplugin.cpp \
				atcmcomboboxtaskmenu.cpp \
				atcmcomboboxdialog.cpp \
    ../ATCMplugineditor/crosstableeditor.cpp \
    #atcmcomboboxpropertysheet.cpp

RESOURCES   = icons.qrc

target.path = $${MECT_INSTALL_PLUGINS}/designer
INSTALLS    += target

include(atcmcombobox.pri)
