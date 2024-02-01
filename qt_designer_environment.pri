win32 {
	ATCM_QWT_LIB = C:/Qt487/desktop/lib
	ATCM_QWT_INCL = C:/Qt487/desktop/include/Qwt
        MECT_INSTALL_PLUGINS = $$[QT_INSTALL_PLUGINS]
}
unix:!macx {
	ATCM_QWT_LIB = /opt/Qt487/desktop/lib
	ATCM_QWT_INCL = /opt/Qt487/desktop/include/Qwt
        MECT_INSTALL_PLUGINS = $$[QT_INSTALL_PLUGINS]
}

