@echo off
Rem ---- Setting Global Variables
SET USER_MODE=%1
SET STARTDIR=%CD%
SET QT_VERSION=4.8.7
Rem ---- Build Dirs
SET ROOT_DIR=C:\
SET OUT_DIR=%ROOT_DIR%Qt487\
SET DESKTOP_DIR=%OUT_DIR%desktop\
SET BIN_DIR=%DESKTOP_DIR%bin\
SET DOC_DIR=%DESKTOP_DIR%doc
SET CC_DIR=%DESKTOP_DIR%mingw32\
SET WINSPEC_DIR=%DESKTOP_DIR%mkspecs\
SET WINBUILD_DIR=%OUT_DIR%winbuild\
SET IMX28BUILD_DIR=%OUT_DIR%imx28_build\
SET TEMP_DIR=%ROOT_DIR%Ms35_tmp\
SET SRC_DIR=%TEMP_DIR%qt-everywhere-opensource-src-4.8.7\
SET OPENSSL_DIR=%ROOT_DIR%openssl-1.0.2u\
SET QWT_DIR=%ROOT_DIR%qwt-6.1-multiaxes_r2275_win\
Rem ---- Mect Settings
SET MECT_PREFIX==%OUT_DIR%imx28\
SET MECT_QT_EMBEDDED=arm
SET MECT_QT_ARCH=arm
Rem SET MECT_QT_XPLATFORM=qws/linux-g++-mx  
SET MECT_QT_XPLATFORM=linux-arm-gnueabi-g++
Rem ---- File Download program
SET TRANSFER_CMD=%CD%\getFileFromArchive.bat
SET EXTRACT_CMD="%ProgramFiles%\7-Zip\7z.exe" x -y -r 
Rem ---- File to be downloaded 
SET SOURCERY_GCC=sourcery-g++-lite
SET DOWNLOAD_LIST=%SOURCERY_GCC%.7z
SET ErrorLog=%STARTDIR%\%~n0.log

Rem ---- Checking Params
IF [%USER_MODE%] EQU [] goto showUsage
echo %DATE% - %TIME%: Starting: %~n0 Param: [%USER_MODE%]  > %ErrorLog%

Rem ---- Download 
IF [%USER_MODE%] EQU [download] (
	echo Downloading Components for %QT_VERSION%
	goto downloadComponents
)
Rem ---- Configure 
IF [%USER_MODE%] EQU [configure] (
	echo Configuring Qt %QT_VERSION%
	goto configureQt
)
Rem ---- Build Qt 
IF [%USER_MODE%] EQU [build] (
	echo Building Qt %QT_VERSION%
	goto buildQt
)

Rem ---- Install Qt 
IF [%USER_MODE%] EQU [install] (
	echo Installing Qt %QT_VERSION%
	goto installQt
)


echo.
echo ----------------------------------------
echo Creating Windows Toolchain for %QT_VERSION% for ARM
echo ----------------------------------------
echo. 
call :screenAndLog "Creating the Windows  Environment for ARM Qt %QT_VERSION% in %IMX28BUILD_DIR%"

IF NOT EXIST %IMX28BUILD_DIR%  (
	call :screenAndLog "Creating Qt Root Dir: %IMX28BUILD_DIR%"
	MKDIR %IMX28BUILD_DIR% 
)

:downloadComponents
call :screenAndLog "Downloading components for ARM platform"
Rem ---- Clear Temp Dir
IF EXIST %TEMP_DIR% RD /S /Q %TEMP_DIR%
MKDIR %TEMP_DIR%
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
IF EXIST %OUT_DIR%%SOURCERY_GCC% RD /S /Q %OUT_DIR%%SOURCERY_GCC%
MKDIR %OUT_DIR%%SOURCERY_GCC%
%EXTRACT_CMD%  %TEMP_DIR%%SOURCERY_GCC%.7z -o%OUT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: %SOURCERY_GCC%.7z to %OUT_DIR%"
	goto AbortProcess
)
call :screenAndLog "Extraction completed for I.MX28 platform"
Set DOWNLOAD_LIST=
IF [%USER_MODE%] EQU [download] goto JobDone

:configureQt
call :screenAndLog "Configuring  Qt %QT_VERSION% in Folder %IMX28BUILD_DIR%"
IF EXIST %IMX28BUILD_DIR% RD /S /Q %IMX28BUILD_DIR%
MKDIR %IMX28BUILD_DIR%
cd %IMX28BUILD_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %IMX28BUILD_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
rem Set PATH=%CC_DIR%bin;%CC_DIR%i686-w64-mingw32\bin;%BIN_DIR%;%PATH%
rem configure -embedded arm -xplatform qws/linux-arm-g+
rem %DESKTOP_DIR%configure  -opensource  -confirm-license -release -embedded arm -arch arm -platform win32-g++ -xplatform linux-arm-gnueabi-g++ -fast -no-phonon -no-webkit -no-qt3support -nomake tools -nomake examples -nomake demos  -qt-sql-odbc -qt-sql-sqlite -plugin-sql-sqlite -plugin-sql-odbc -plugin-sql-mysql -I C:/MySQLConnector/include -L C:/MySQLConnector/lib -openssl -I %OPENSSL_DIR%include 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qt487-I_MX28-config.log
%DESKTOP_DIR%configure -prefix %MECT_PREFIX%  -release -opensource -confirm-license -arch %MECT_QT_ARCH% -shared -fast -no-system-proxies -no-exceptions -no-accessibility -no-stl -qt-sql-sqlite -qt-sql-odbc -qt-sql-mysql  -I C:/MySQLConnector/include  -no-qt3support -no-xmlpatterns -no-multimedia -audio-backend -no-phonon -no-phonon-backend -no-webkit -no-script -no-scripttools -no-declarative -no-declarative-debug -no-3dnow -no-mmx -no-sse -no-sse2 -qt-zlib -no-libtiff -qt-libpng -no-libmng -qt-libjpeg -openssl  -I %OPENSSL_DIR%include -nomake examples -nomake demos  -no-nis -no-cups -iconv -xplatform %MECT_QT_XPLATFORM% -little-endian -system-freetype -no-opengl -no-s60 -dbus  2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qt487-I_MX28-config.log


if errorlevel 1 (
	call :screenAndLog "Error Configuring Qt in: %IMX28BUILD_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configuration completed for Qt %QT_VERSION% in Folder %IMX28BUILD_DIR%"
)
cd %STARTDIR%
Rem  ---- Exit batch if configure mode
IF [%USER_MODE%] EQU [configure] goto JobDone

:buildQt
call :screenAndLog "Building  Qt %QT_VERSION% in %IMX28BUILD_DIR%"
cd %IMX28BUILD_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %IMX28BUILD_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
set QMAKESPEC=%IMX28BUILD_DIR%mkspecs\win32-g++
Rem Creating ./bin/qt.conf with right qmake prefix path
Set QPREFIX=%IMX28BUILD_DIR:\=/%
Echo [Paths] > bin\qt.conf
ECHO Prefix=%QPREFIX%>> bin\qt.conf
mingw32-make  2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qt487-desktop-make.log
if errorlevel 1 (
	call :screenAndLog "Error Building Qt in: %IMX28BUILD_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Build completed for Qt %QT_VERSION% in Folder %IMX28BUILD_DIR%"
)
cd %STARTDIR%

Rem  ---- Exit batch if build mode
IF [%USER_MODE%] EQU [build] goto JobDone

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
Rem  ---- Exit batch if install mode
IF [%USER_MODE%] EQU [install] goto JobDone

:buildQWT
call :screenAndLog "Configuring Qwt 6.1 Multiaxes"
cd %QWT_DIR%
if errorlevel 1 (
	call :screenAndLog "Qwt dir %QWT_DIR% not found, please download Qt components"
	goto AbortProcess
)
if not EXIST %WINSPEC_DIR% (
	call :screenAndLog "You must configure and install Qt first"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
Rem ---- Configuring Qwt
call :screenAndLog "Configuring Qwt"
qmake qwt.pro 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" -a %TEMP_DIR%Qwt-Configure.log
if errorlevel 1 (
	call :screenAndLog "Error Configuring Qwt 6.1 Multiaxes in %QWT_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configured Qwt 6.1 Multiaxes in %QWT_DIR%"
)
Rem ---- Building Qwt
call :screenAndLog "Building Qwt"
mingw32-make  2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%Qwt-Make.log
if errorlevel 1 (
	call :screenAndLog "Error Building Qwt 6.1 Multiaxes in %QWT_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Builded Qwt 6.1 Multiaxes in %QWT_DIR%"
)
Rem ---- Start Qwt Installation
call :screenAndLog "Installing Qwt in %DESKTOP_DIR%"
mingw32-make install 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" -a %TEMP_DIR%Qwt-Install.log
if errorlevel 1 (
	call :screenAndLog "Error Installing Qwt 6.1 Multiaxes from %QWT_DIR% to %DESKTOP_DIR%"
	goto AbortProcess
)  else  (
	xcopy %QWT_DIR%lib\qwt.dll %BIN_DIR% /Y /I
	call :screenAndLog "Installed Qwt 6.1 Multiaxes from %QWT_DIR% to %DESKTOP_DIR%"
)
Rem  ---- Exit batch if QWT mode
cd %STARTDIR%
IF [%USER_MODE%] EQU [qwt] goto JobDone

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
