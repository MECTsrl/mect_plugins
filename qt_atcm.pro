TEMPLATE      = subdirs
SUBDIRS       =	\
qt_library \
qt_plugins \
#example

include(./qt_designer_environment.pri)

# install
sources.files = atcmplugin.h
sources.path = $${ATCM_ARM_PLUGINS_INCPATH}
INSTALLS += sources

