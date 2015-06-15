# /usr/local/Trolltech/Qt-4.6.0/bin/qmake -set QT_INSTALL_PLUGINS /home/mect/QtSDK/QtCreator/lib/qtcreator/plugins -o Makefile

include(../qt_environment.pri)

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

SRCDIR="../qt_templates/ATCM-TP1057HR-template-form-class"
DSTDIR="$${ATCM_INSTALL_WIZARD}/ATCM-TP1057HR-template-form-class"
SRCDIR ~=s,/,\\,g
DSTDIR ~=s,/,\\,g
copytp1057hr.commands = $$quote( $${ATCM_DIR_COPY} $${SRCDIR} $${DSTDIR})

SRCDIR="../qt_templates/ATCM-TP1057-template-form-class"
DSTDIR="$${ATCM_INSTALL_WIZARD}/ATCM-TP1057-template-form-class"
SRCDIR ~=s,/,\\,g
DSTDIR ~=s,/,\\,g
copytp1057.commands = $$quote( $${ATCM_DIR_COPY} $${SRCDIR} $${DSTDIR})

SRCDIR="../qt_templates/ATCM-TPAC1007-template-form-class"
DSTDIR="$${ATCM_INSTALL_WIZARD}/ATCM-TPAC1007-template-form-class"
SRCDIR ~=s,/,\\,g
DSTDIR ~=s,/,\\,g
copytp1007.commands = $$quote( $${ATCM_DIR_COPY} $${SRCDIR} $${DSTDIR})

SRCDIR="../qt_templates/ATCM-TPAC1008-template-form-class"
DSTDIR="$${ATCM_INSTALL_WIZARD}/ATCM-TPAC1008-template-form-class"
SRCDIR ~=s,/,\\,g
DSTDIR ~=s,/,\\,g
copytp1008.commands = $$quote( $${ATCM_DIR_COPY} $${SRCDIR} $${DSTDIR})

first.depends = $(first) copytemplateprj copytp1057hr copytp1057 copytp1007 copytp1008
export(first.depends)
export(copytemplateprj.commands)
export(copytp1057hr.commands)
export(copytp1057.commands)
export(copytp1007.commands)
export(copytp1008.commands)
QMAKE_EXTRA_TARGETS += first copytemplateprj copytp1057hr copytp1057 copytp1007 copytp1008
