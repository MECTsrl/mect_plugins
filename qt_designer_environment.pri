include (qt_environment.pri)
win32 {
	ATCM_QWT_LIB = C:/Qt485/desktop/lib
	ATCM_QWT_INCL = C:/Qt485/desktop/include/Qwt
}
unix:!macx {
	ATCM_QWT_LIB = /usr/local/qwt-6.1.0/lib
	ATCM_QWT_INCL = /usr/local/qwt-6.1.0/include
}
