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

