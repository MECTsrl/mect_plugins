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

