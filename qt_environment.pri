QMAKE_CXXFLAGS_RELEASE += -Wno-psabi
QMAKE_CXXFLAGS_DEBUG   += -Wno-psabi

DEFINES += LOG_LEVEL=error_e
#DEFINES += ATCM_VERSION="rev.6.10"

contains(QMAKE_HOST.os,Windows){
        ATCM_ARM_LIBRARY_LIBPATH = C:/Qt485/imx28/ATCM/library/lib
	ATCM_ARM_PLUGINS_LIBPATH = C:/Qt485/imx28/ATCM/plugins/lib
	ATCM_ARM_LIBRARY_INCPATH = C:/Qt485/imx28/ATCM/library/include
	ATCM_ARM_PLUGINS_INCPATH = C:/Qt485/imx28/ATCM/plugins/include
        ATCM_INSTALL_PLUGINS = C:/Qt485/desktop/plugins/designer
        ATCM_INSTALL_WIZARD = C:/Qt485/desktop/share/qtcreator/templates/wizards
        ATCM_DIR_COPY = "cmd /c xcopy /Q /Y /E /S /I"
}
!contains(QMAKE_HOST.os,Windows){
        ATCM_ARM_LIBRARY_LIBPATH = $$(HOME)/install/library/lib
	ATCM_ARM_PLUGINS_LIBPATH = $$(HOME)/install/plugins/lib
	ATCM_ARM_LIBRARY_INCPATH = $$(HOME)/install/library/include
	ATCM_ARM_PLUGINS_INCPATH = $$(HOME)/install/plugins/include
	ATCM_INSTALL_PLUGINS = $$(HOME)/QtCreator/lib/qtcreator/plugins/designer
        ATCM_INSTALL_WIZARD = $$(HOME)/QtCreator/share/qtcreator/templates/wizards
        ATCM_DIR_COPY = "cp -rf"
}

