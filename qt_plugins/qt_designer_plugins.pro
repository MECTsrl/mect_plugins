# /usr/local/Trolltech/Qt-4.6.0/bin/qmake -set QT_INSTALL_PLUGINS /home/mect/QtSDK/QtCreator/lib/qtcreator/plugins -o Makefile

include(../qt_designer_environment.pri)

TEMPLATE      = subdirs

SUBDIRS       =	\
ATCManimation \
ATCMcombobox \
ATCMbutton \
ATCMlabel \
ATCMled \
ATCMprogressbar \
ATCMslider \
ATCMspinbox \
ATCMgraph \
ATCMdate \
ATCMtime \
#ATCMlcdnumber \

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets
#sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS widgets.pro README
#sources.path = $$[QT_INSTALL_EXAMPLES]/widgets
#INSTALLS += target sources

# install
#target.path = $$[QT_INSTALL_PLUGINS]/designer
#sources.files = $$SOURCES $$HEADERS *.pro
#sources.path = $$[QT_INSTALL_EXAMPLES]/designer/taskmenuextension
#INSTALLS += target sources

SRCDIR="../qt_templates/ATCM-template-project"
DSTDIR="$${ATCM_INSTALL_WIZARD}/ATCM-template-project"
SRCDIR ~=s,/,\\,g
DSTDIR ~=s,/,\\,g
copytemplateprj.commands = $$quote( $${ATCM_DIR_COPY} $${SRCDIR} $${DSTDIR})

SRCDIR="../qt_templates/ATCM-template-form-class"
DSTDIR="$${ATCM_INSTALL_WIZARD}/ATCM-template-form-class"
SRCDIR ~=s,/,\\,g
DSTDIR ~=s,/,\\,g
copytemplateformclass.commands = $$quote( $${ATCM_DIR_COPY} $${SRCDIR} $${DSTDIR})

first.depends = $(first) copytemplateprj copytemplateformclass
export(first.depends)
export(copytemplateprj.commands)
export(copytemplateformclass.commands)
QMAKE_EXTRA_TARGETS += first copytemplateprj copytemplateformclass
