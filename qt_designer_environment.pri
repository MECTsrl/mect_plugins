win32 {
	ATCM_QWT_LIB = C:/Qt487/desktop/lib
	ATCM_QWT_INCL = C:/Qt487/desktop/include/Qwt
        QT_CREATOR = C:/Qt487/desktop
        ATCM_DIR_COPY = "cmd /c xcopy /Q /Y /E /S /I"
        ATCM_INSTALL_WIZARD = C:/Qt487/desktop/share/qtcreator/templates/wizards
        MECT_INSTALL_PLUGINS = $$[QT_INSTALL_PLUGINS]
}
unix:!macx {
	ATCM_QWT_LIB = $$QT_ROOTFS/usr/lib
	ATCM_QWT_INCL = $$QT_ROOTFS/usr/include
        QT_CREATOR = $$HOME/qtcreator-2.8.1/bin/qtcreator
        ATCM_DIR_COPY = "cp -rf"
        ATCM_INSTALL_WIZARD = "$$(HOME)/qtcreator-2.8.1/share/qtcreator/templates/wizards/"
        MECT_INSTALL_PLUGINS = "$$(HOME)/qtcreator-2.8.1/bin/plugins"
}

