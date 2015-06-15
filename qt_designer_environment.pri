include (qt_environment.pri)
win32 {
	ATCM_INSTALL_PLUGINS = c:/Qt485/desktop/plugins/designer
        ATCM_QWT_LIB = C:/Qt485/desktop/lib/qwt-6.0.1/lib
	ATCM_QWT_INCL = 
}
unix:!macx {
	ATCM_INSTALL_PLUGINS = /home/mect/QtSDK/QtCreator/lib/qtcreator/plugins/designer
	ATCM_QWT_LIB = /usr/local/qwt-6.0.1/lib
	ATCM_QWT_INCL = /usr/local/qwt-6.0.1/include
}
