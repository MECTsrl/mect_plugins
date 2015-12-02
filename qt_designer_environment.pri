include (qt_environment.pri)
win32 {
	ATCM_QWT_LIB = C:/Qt485/desktop/lib
	ATCM_QWT_INCL = C:/Qt485/desktop/include/Qwt
}
unix:!macx {
	ATCM_QWT_LIB = $$QT_ROOTFS/usr/lib
	ATCM_QWT_INCL = $$QT_ROOTFS/usr/include
}
