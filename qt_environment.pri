QMAKE_CXXFLAGS_RELEASE += -Wno-psabi
QMAKE_CXXFLAGS_DEBUG   += -Wno-psabi

DEFINES += LOG_LEVEL=error_e
#DEFINES += ATCM_VERSION=7.0rc0
CONFIG += store alarms recipe trend
contains(QMAKE_HOST.os,Windows){
        QT_ROOTFS = C:/Qt485/imx28/rootfs
        QT_CREATOR = C:/Qt485/desktop
        ATCM_DIR_COPY = "cmd /c xcopy /Q /Y /E /S /I"
}
!contains(QMAKE_HOST.os,Windows){
        QT_ROOTFS = $$(MECT_RFSDIR)
        QT_CREATOR = C:/Qt485/desktop
        ATCM_DIR_COPY = "cp -rf"
}

ATCM_ARM_LIBRARY_LIBPATH = $$QT_ROOTFS/usr/lib
ATCM_ARM_PLUGINS_LIBPATH = $$QT_ROOTFS/usr/lib
ATCM_ARM_LIBRARY_INCPATH = $$QT_ROOTFS/usr/include
ATCM_ARM_PLUGINS_INCPATH = $$QT_ROOTFS/usr/include

INCLUDEPATH += $$QT_ROOTFS/usr/src/include
INCLUDEPATH += $$QT_ROOTFS/usr/src/linux/include
QMAKE_LIBDIR += $$QT_ROOTFS/usr/lib
