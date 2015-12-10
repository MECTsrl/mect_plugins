#include (qt_environment.pri)
win32 {
	ATCM_QWT_LIB = C:/Qt485/desktop/lib
	ATCM_QWT_INCL = C:/Qt485/desktop/include/Qwt
        QT_CREATOR = C:/Qt485/desktop
        ATCM_DIR_COPY = "cmd /c xcopy /Q /Y /E /S /I"
        ATCM_INSTALL_WIZARD = C:/Qt485/desktop/share/qtcreator/templates/wizards
}
unix:!macx {
	ATCM_QWT_LIB = $$QT_ROOTFS/usr/lib
	ATCM_QWT_INCL = $$QT_ROOTFS/usr/include
        QT_CREATOR = C:/Qt485/desktop
        ATCM_DIR_COPY = "cp -rf"
        ATCM_INSTALL_WIZARD = ""
}
