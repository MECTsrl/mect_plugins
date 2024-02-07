@echo off
Rem ---- Setting Global Variables
SET USER_MODE=%1
SET STARTDIR=%CD%
SET QT_VERSION=4.8.7
SET REV=3.5.0
Rem ---- Build Dirs
SET ROOT_DIR=C:\
SET QT_DIR=%ROOT_DIR%Qt487\
SET DESKTOP_DIR=%QT_DIR%desktop\
SET BIN_DIR=%DESKTOP_DIR%bin\
SET DOC_DIR=%DESKTOP_DIR%doc
SET CC_DIR=%DESKTOP_DIR%mingw32\
SET WINSPEC_DIR=%DESKTOP_DIR%mkspecs\
SET IMX28_SRC_DIR=%QT_DIR%imx28\
SET IMX28_BUILD_DIR=%QT_DIR%imx28_build\
SET IMX28_DIR=%QT_DIR%imx28\
SET IMX28_ROOTFS=%IMX28_DIR%rootfs\
SET TEMP_DIR=%ROOT_DIR%Ms35_tmp\
SET SRC_FILE=qt-everywhere-opensource-src-4.8.7_patched
SET SRC_DIR=%TEMP_DIR%%SRC_FILE%\
SET USR_INCLUDE=%IMX28_ROOTFS%\usr\include
SET LINUX_INCLUDE=%IMX28_ROOTFS%\src\linux\include
Rem SET OPENSSL_DIR=%ROOT_DIR%openssl-1.0.2u\
SET QWT_DIR=%ROOT_DIR%qwt-6.1-multiaxes_r2275_win\
Rem ---- Mect Settings
SET MECT_PREFIX=%QT_DIR%imx28
SET MECT_QT_EMBEDDED=arm
SET MECT_QT_ARCH=arm
SET MECT_QT_XPLATFORM=linux-arm-gnueabi-g++
SET MECT_MKSPECS=%IMX28_DIR%mkspecs\
Rem ---- File Download program
SET TRANSFER_CMD=%CD%\getFileFromArchive.bat
SET EXTRACT_CMD="%ProgramFiles%\7-Zip\7z.exe" x -y -r 
Rem ---- File to be downloaded 
SET SOURCERY_GCC=sourcery-g++-lite
SET DOWNLOAD_LIST=%SOURCERY_GCC%.7z %SRC_FILE%.7z  rootfs_dev_%REV%.zip
SET ErrorLog=%STARTDIR%\%~n0.log

Rem ---- Checking Params
IF [%USER_MODE%] EQU [] goto showUsage
echo %DATE% - %TIME%: Starting: %~n0 Param: [%USER_MODE%]  > %ErrorLog%

Rem ---- Download 
IF /I [%USER_MODE%] EQU [download] (
	echo Downloading Components for %QT_VERSION%
	goto downloadComponents
)
Rem ---- Configure 
IF /I [%USER_MODE%] EQU [configure] (
	echo Configuring Qt %QT_VERSION%
	goto configureQt
)
Rem ---- Build Qt 
IF /I [%USER_MODE%] EQU [build] (
	echo Building Qt %QT_VERSION%
	goto buildQt
)

Rem ---- Install Qt 
IF /I [%USER_MODE%] EQU [install] (
	echo Installing Qt %QT_VERSION%
	goto installQt
)

Rem ---- Checking all
IF /I NOT [%USER_MODE%] EQU [all]  (
	goto showUsage
)

echo.
echo ----------------------------------------
echo Creating Windows Toolchain for %QT_VERSION% for ARM
echo ----------------------------------------
echo. 
call :screenAndLog "Creating the Windows  Environment for ARM Qt %QT_VERSION% in %IMX28_DIR%"

:downloadComponents
call :screenAndLog "Downloading components for ARM platform"
Rem ---- Clear Temp Dir
call :screenAndLog "Clearing %TEMP_DIR%"
IF EXIST %TEMP_DIR% RD /S /Q %TEMP_DIR%
MKDIR %TEMP_DIR%
Rem ---- I.MX28 SRC Dir
call :screenAndLog "Clearing %IMX28_SRC_DIR%"
IF EXIST %IMX28_SRC_DIR%  RD /S /Q %IMX28_SRC_DIR%
MKDIR %IMX28_SRC_DIR% 
Rem ---- I.MX28  Dir and rootfs dir
IF /I [%IMX28_SRC_DIR%] NEQ [%IMX28_DIR%]  (
	call :screenAndLog "Clearing %IMX28_DIR%"
	IF EXIST %IMX28_DIR%  RD /S /Q %IMX28_DIR%
	MKDIR %IMX28_DIR% 
)

Rem ---- Download File List
For %%a in (%DOWNLOAD_LIST%) do (
	call :screenAndLog "Downloading %%~a to %TEMP_DIR%"
	call  %TRANSFER_CMD% %%a %TEMP_DIR%
	if errorlevel 1 (
		call :screenAndLog "Error Downloading %%~a"
		goto AbortProcess
	)  else  (
		call :screenAndLog "Downloaded %%~a"
	)
)
call :screenAndLog "Download completed for ARM platform"
:expandZip
call :screenAndLog "Expanding downloaded components for ARM platform"
Rem ---- sourcery-g++-lite.7z
IF EXIST %QT_DIR%%SOURCERY_GCC% RD /S /Q %QT_DIR%%SOURCERY_GCC%
MKDIR %QT_DIR%%SOURCERY_GCC%
%EXTRACT_CMD%  %TEMP_DIR%%SOURCERY_GCC%.7z -o%QT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: %SOURCERY_GCC%.7z to %QT_DIR%"
	goto AbortProcess
)
Rem ---- Qt 4.8.7 Patched Sources
call :screenAndLog "Expanding Qt Patched Sources"
%EXTRACT_CMD%  %TEMP_DIR%%SRC_FILE%.7z -o%TEMP_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: %SRC_FILE%.7z to %TEMP_DIR%"
	goto AbortProcess
)
call :screenAndLog "Moving Qt %QT_VERSION% sources to %IMX28_SRC_DIR%"
xcopy %SRC_DIR%*.* %IMX28_SRC_DIR%*.* /s /y /e /v /q
call :screenAndLog "Cleaning  %SRC_DIR%"
RD /S /Q %SRC_DIR%
Rem --- Update MECT_QT_XPLATFORM MKSPECS for build
call :screenAndLog "Updating  %MECT_QT_XPLATFORM%"
xcopy %STARTDIR%\%MECT_QT_XPLATFORM% %IMX28_SRC_DIR%mkspecs\%MECT_QT_XPLATFORM% /i /s /y /v
Rem ---  rootfs_dev_X.Y.Z.zip
call :screenAndLog "Expanding I.MX28 root_fs"
%EXTRACT_CMD%  %TEMP_DIR%rootfs_dev_%REV%.zip -o%IMX28_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: rootfs_dev%REV%.zip to %IMX28_DIR%"
	goto AbortProcess
)
call :screenAndLog "Extraction completed for I.MX28 platform"

Set DOWNLOAD_LIST=
IF /I [%USER_MODE%] EQU [download] goto JobDone

:configureQt
call :screenAndLog "Configuring  Qt %QT_VERSION% in Folder %IMX28_DIR%"
cd %IMX28_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %IMX28_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
rem Set PATH=%CC_DIR%bin;%CC_DIR%i686-w64-mingw32\bin;%BIN_DIR%;%PATH%
rem configure -embedded arm -xplatform qws/linux-arm-g+
rem %DESKTOP_DIR%configure  -opensource  -confirm-license -release -embedded arm -arch arm -platform win32-g++ -xplatform linux-arm-gnueabi-g++ -fast -no-phonon -no-webkit -no-qt3support -nomake tools -nomake examples -nomake demos  -qt-sql-odbc -qt-sql-sqlite -plugin-sql-sqlite -plugin-sql-odbc -plugin-sql-mysql -I C:/MySQLConnector/include -L C:/MySQLConnector/lib -openssl -I %OPENSSL_DIR%include 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qt487-I_MX28-config.log  -prefix %MECT_PREFIX%  -openssl  -I %USR_INCLUDE%  -I %OPENSSL_DIR%include  
%IMX28_SRC_DIR%configure   -prefix %MECT_PREFIX%  -platform win32-g++  -release -opensource -embedded -confirm-license -arch %MECT_QT_ARCH% -shared -fast -no-system-proxies -no-exceptions -no-accessibility -no-stl -qt-sql-sqlite -qt-sql-odbc -no-qt3support -no-xmlpatterns -no-multimedia -audio-backend -no-phonon -no-phonon-backend -no-webkit -no-script -no-scripttools -no-declarative -no-declarative-debug -no-3dnow -no-mmx -no-sse -no-sse2 -qt-zlib -no-libtiff -qt-libpng -no-libmng -qt-libjpeg -openssl -nomake examples -nomake demos  -no-nis -no-cups -iconv -xplatform %MECT_QT_XPLATFORM%  -little-endian -system-freetype -no-opengl -no-s60 -dbus  2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qt487-I_MX28-config.log

if errorlevel 1 (
	call :screenAndLog "Error Configuring Qt in: %IMX28_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configuration completed for Qt %QT_VERSION% in Folder %IMX28_DIR%"
)
Rem --- Update MECT_QT_XPLATFORM MKSPECS for build
Rem call :screenAndLog "Updating  %MECT_QT_XPLATFORM%"
Rem xcopy %STARTDIR%\%MECT_QT_XPLATFORM% %MECT_MKSPECS%\%MECT_QT_XPLATFORM% /i /s /y /v
cd %STARTDIR%
Rem  ---- Exit batch if configure mode
IF /I [%USER_MODE%] EQU [configure] goto JobDone

:buildQt
call :screenAndLog "Building  Qt %QT_VERSION% in %IMX28_DIR%"
cd %IMX28_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %IMX28_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
set QMAKESPEC=%IMX28_DIR%mkspecs\win32-g++
Rem Creating ./bin/qt.conf with right qmake prefix path
Rem Set QPREFIX=%IMX28_DIR:\=/%
Rem Echo [Paths] > bin\qt.conf
Rem ECHO Prefix=%QPREFIX%>> bin\qt.conf
mingw32-make  2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qt487-desktop-make.log
if errorlevel 1 (
	call :screenAndLog "Error Building Qt in: %IMX28_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Build completed for Qt %QT_VERSION% in Folder %IMX28_DIR%"
)
cd %STARTDIR%

Rem  ---- Exit batch if build mode
IF /I [%USER_MODE%] EQU [build] goto JobDone

:installQt
call :screenAndLog "Installing  Qt %QT_VERSION% from %IMX28BUILD_DIR%"
cd %IMX28BUILD_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %IMX28BUILD_DIR%"
	goto AbortProcess
)
Rem ---- Extracting BUILD PATH and INSTALLATION PATH
SET WINBUILD_PATH=%IMX28BUILD_DIR:C:=%
SET DESKTOP_PATH=%DESKTOP_DIR:C:=%
Rem ---- Updating all Makefile
fart -c -i -r Makefile*  c:$(INSTALL_ROOT)%WINBUILD_PATH% c:$(INSTALL_ROOT)%DESKTOP_PATH%
set INSTALL_ROOT=
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
set QMAKESPEC=C:\Qt487\winbuild\mkspecs\win32-g++
Rem ---- Start Qt Installation
mingw32-make install 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" -a %TEMP_DIR%Qt487-Install.log
if errorlevel 1 (
	call :screenAndLog "Error Installing Qt from %IMX28BUILD_DIR% to: %DESKTOP_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Installed Qt from %IMX28BUILD_DIR% to: %DESKTOP_DIR%"
)
cd %STARTDIR%
Rem  ---- Exit batch if install or all mode
IF /I [%USER_MODE%] EQU [install] goto JobDone
IF /I [%USER_MODE%] EQU [all]     goto JobDone


:JobDone
	call :screenAndLog "Done."
	pause
	exit /b 

:AbortProcess
	call :screenAndLog "Build Aborted!"
	cd %STARTDIR%
	exit /b 1

:screenAndLog 
	SETLOCAL
	set message=%~1
	echo %message% & echo %TIME%: %message% >> %ErrorLog%
	ENDLOCAL
	exit /b	

:addToPath
	echo ;%PATH%; | find /C /I ";%~1;"1>NUL
	if errorlevel 1 (
		SET PATH=%PATH%;%~1;
		echo Added %~1 to Path Variables
	)  else (
		echo %~1 already in Path
	)
	exit /b	

:showUsage
echo Usage: %~n0 mode
echo mode = all: 		Do full ARM Build and Install
echo mode = download: 	Download ARM Components and exit
echo mode = configure: 	Configure Qt for ARM and exit
echo mode = build: 		Build Qt for ARM and exit
echo mode = install: 	Install configured Qt and exit

goto JobDone
