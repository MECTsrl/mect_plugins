QMAKE_CXXFLAGS_RELEASE += -Wno-psabi
QMAKE_CXXFLAGS_DEBUG   += -Wno-psabi

DEFINES += LOG_LEVEL=error_e

CONFIG += store alarms recipe trend

contains(QMAKE_HOST.os,Windows){
        QT_ROOTFS = C:/Qt485/imx28/rootfs
        QT_INSTALL_ROOTFS = C:/Qt485/imx28/rootfs
        QT_CREATOR = C:/Qt485/desktop
        ATCM_DIR_COPY = "cmd /c xcopy /Q /Y /E /S /I"
}
!contains(QMAKE_HOST.os,Windows){
        QT_ROOTFS = $$(MECT_RFSDIR)
        QT_INSTALL_ROOTFS = $$(INSTALL_ROOT)
        QT_CREATOR = C:/Qt485/desktop
        ATCM_DIR_COPY = "cp -rf"
}

ATCM_ARM_LIBRARY_LIBPATH = $$QT_ROOTFS/usr/lib
ATCM_ARM_PLUGINS_LIBPATH = $$QT_ROOTFS/usr/lib
ATCM_ARM_LIBRARY_INCPATH = $$QT_ROOTFS/usr/include
ATCM_ARM_PLUGINS_INCPATH = $$QT_ROOTFS/usr/include

ATCM_ARM_LIBRARY_INSTALL_LIBPATH = $$QT_INSTALL_ROOTFS/usr/lib
ATCM_ARM_PLUGINS_INSTALL_LIBPATH = $$QT_INSTALL_ROOTFS/usr/lib
ATCM_ARM_LIBRARY_INSTALL_INCPATH = $$QT_INSTALL_ROOTFS/usr/include
ATCM_ARM_PLUGINS_INSTALL_INCPATH = $$QT_INSTALL_ROOTFS/usr/include

INCLUDEPATH  += .
INCLUDEPATH  += $$QT_ROOTFS/usr/include
INCLUDEPATH  += $$QT_ROOTFS/usr/src/linux/include
QMAKE_LIBDIR += $$QT_ROOTFS/usr/lib
