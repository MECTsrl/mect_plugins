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
SET TEMP_DIR=%ROOT_DIR%Ms35_tmp\
SET SRC_DIR=%TEMP_DIR%qt-everywhere-opensource-src-4.8.7\
SET OPENSSL_DIR=%ROOT_DIR%openssl-1.0.2u\
SET QWT_DIR=%ROOT_DIR%qwt-6.1-multiaxes_r2275_win\
SET QTSERIALPORT=qtserialport
SET QTSERIAL_DIR=%ROOT_DIR%%QTSERIALPORT%\
Rem ---- File Download program
SET TRANSFER_CMD=%CD%\getFileFromArchive.bat
SET EXTRACT_CMD="%ProgramFiles%\7-Zip\7z.exe" x -y -r 
SET TEE=%ProgramFiles%\Git\usr\bin\tee
Rem ---- File to be downloaded 
SET DOWNLOAD_LIST=MingW_i686-4.8.2-release-posix-dwarf-rt_v3-rev3.7z qt-everywhere-opensource-src-4.8.7.zip openssl-1.0.2u_winbuild.7z qt-tools.7z qt487_doc.7z qwt-6.1-multiaxes_r2275_win.7z qtserialport_487.7z
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

Rem ---- qwt
IF /I [%USER_MODE%] EQU [qwt] (
	echo Add qwt to Qt %QT_VERSION% Installation in %DESKTOP_DIR%
	goto buildQWT
)

Rem ---- qtserial
IF /I [%USER_MODE%] EQU [qtserial] (
	echo Add QtSerial to Qt %QT_VERSION% Installation in %DESKTOP_DIR%
	goto buildQtSerial
)

Rem ---- Checking all
IF /I NOT [%USER_MODE%] EQU [all]  (
	goto showUsage
)


echo.
echo ----------------------------------------
echo Creating Windows Building Environment for %QT_VERSION%
echo ----------------------------------------
echo. 
call :screenAndLog "Creating the Desktop Environment for Qt %QT_VERSION% in %DESKTOP_DIR%"

IF NOT EXIST %OUT_DIR%  (
	call :screenAndLog "Creating Qt Root Dir: %OUT_DIR%"
	MKDIR %OUT_DIR% 
)
IF EXIST %DESKTOP_DIR% (
	call :screenAndLog "Destination Folder %DESKTOP_DIR% already exist: Clean and continue"
	goto AbortProcess
) else (
	call :screenAndLog "Creating Qt Dir %DESKTOP_DIR%"
	MKDIR %DESKTOP_DIR%
)

:downloadComponents
call :screenAndLog "Downloading components for Qt %QT_VERSION%"
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
call :screenAndLog "Download completed for Qt %QT_VERSION%"
:expandZip
call :screenAndLog "Expanding downloaded components for Qt %QT_VERSION%"
Rem ---- MingW_i686-4.8.2-release-posix-dwarf-rt_v3-rev3.7z
%EXTRACT_CMD%  %TEMP_DIR%MingW_i686-4.8.2-release-posix-dwarf-rt_v3-rev3.7z -o%DESKTOP_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: MingW_i686-4.8.2-release-posix-dwarf-rt_v3-rev3.7z to %DESKTOP_DIR%"
	goto AbortProcess
)
Rem ---- Qt Tools qt-tools.7z
IF EXIST %BIN_DIR% RD /S /Q %BIN_DIR%
MKDIR %BIN_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qt-tools.7z -o%BIN_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: qt-tools.7z to %BIN_DIR%"
	goto AbortProcess
)
Rem ---- qt-everywhere-opensource-src-4.8.7.zip (Extracting in TEMP, Merge with Qt487\desktop\)
IF EXIST %SRC_DIR% RD /S /Q %SRC_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qt-everywhere-opensource-src-4.8.7.zip -o%TEMP_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: qt-everywhere-opensource-src-4.8.7.zip to %TEMP_DIR%"
	goto AbortProcess
)
call :screenAndLog "Moving Qt %QT_VERSION% sources to %DESKTOP_DIR%"
xcopy %SRC_DIR%*.* %DESKTOP_DIR%*.* /s /y /e /v /q
call :screenAndLog "Done"
RD /S /Q %SRC_DIR%
Rem ---- openssl-1.0.2u_winbuild.7z
IF EXIST %OPENSSL_DIR% RD /S /Q %OPENSSL_DIR%
MKDIR %OPENSSL_DIR%
%EXTRACT_CMD%  %TEMP_DIR%openssl-1.0.2u_winbuild.7z -o%ROOT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: openssl-1.0.2u_winbuild.7z to %ROOT_DIR%"
	goto AbortProcess
)
Rem ---- Qt 487 Doc qt487_doc.7z
IF EXIST %DOC_DIR% RD /S /Q %DOC_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qt487_doc.7z -o%DESKTOP_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: qt487_doc.7z to %DESKTOP_DIR%"
	goto AbortProcess
)
Rem ---- qwt-6.1-multiaxes_r2275_win.7z
IF EXIST %QWT_DIR% RD /S /Q %QWT_DIR%
MKDIR %QWT_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qwt-6.1-multiaxes_r2275_win.7z -o%ROOT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: qwt-6.1-multiaxes_r2275_win.7z to %ROOT_DIR%"
	goto AbortProcess
)
Rem ---- qtserialport_487.7z
IF EXIST %QTSERIAL_DIR%  RD /S /Q %QTSERIAL_DIR%
MKDIR %QTSERIAL_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qtserialport_487.7z -o%ROOT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding: qtserialport_487.7z to %ROOT_DIR%"
	goto AbortProcess
)

call :screenAndLog "Extraction completed for Qt %QT_VERSION%"
Set DOWNLOAD_LIST=
IF /I [%USER_MODE%] EQU [download] goto JobDone

:configureQt
call :screenAndLog "Configuring  Qt %QT_VERSION% in Folder %WINBUILD_DIR%"
IF EXIST %WINBUILD_DIR% RD /S /Q %WINBUILD_DIR%
MKDIR %WINBUILD_DIR%
cd %WINBUILD_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %WINBUILD_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin
call :addToPath "%BIN_DIR%"
rem Set PATH=%CC_DIR%bin;%CC_DIR%i686-w64-mingw32\bin;%BIN_DIR%;%PATH%
%DESKTOP_DIR%configure  -prefix C:/Qt487/desktop -fast -opensource -platform win32-g++ -debug-and-release -confirm-license -no-vcproj -no-s60 -no-webkit -no-cetest -no-dsp -no-phonon -no-phonon-backend -no-qt3support -nomake examples -nomake demos -qt-zlib -qt-sql-odbc -qt-sql-sqlite -plugin-sql-sqlite -plugin-sql-odbc -plugin-sql-mysql -I C:/MySQLConnector/include -L C:/MySQLConnector/lib -openssl -I %OPENSSL_DIR%include 2>&1 | "%TEE%" %STARTDIR%\Qt487-desktop-config.log
if errorlevel 1 (
	call :screenAndLog "Error Configuring Qt in: %WINBUILD_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configuration completed for Qt %QT_VERSION% in Folder %WINBUILD_DIR%"
)
cd %STARTDIR%
Rem  ---- Exit batch if configure mode
IF /I [%USER_MODE%] EQU [configure] goto JobDone

:buildQt
call :screenAndLog "Building  Qt %QT_VERSION% in %WINBUILD_DIR%"
cd %WINBUILD_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %WINBUILD_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin
call :addToPath "%BIN_DIR%"
set QMAKESPEC=%WINBUILD_DIR%mkspecs\win32-g++
Rem Creating ./bin/qt.conf with right qmake prefix path
Set QPREFIX=%WINBUILD_DIR:\=/%
Echo [Paths] > bin\qt.conf
ECHO Prefix=%QPREFIX%>> bin\qt.conf
mingw32-make  2>&1 | "%TEE%" %STARTDIR%\Qt487-desktop-make.log
if errorlevel 1 (
	call :screenAndLog "Error Building Qt in: %WINBUILD_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Build completed for Qt %QT_VERSION% in Folder %WINBUILD_DIR%"
)
cd %STARTDIR%

Rem  ---- Exit batch if build mode
IF /I [%USER_MODE%] EQU [build] goto JobDone

:installQt
call :screenAndLog "Installing  Qt %QT_VERSION% from %WINBUILD_DIR%"
cd %WINBUILD_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %WINBUILD_DIR%"
	goto AbortProcess
)
Rem ---- Extracting BUILD PATH and INSTALLATION PATH
SET WINBUILD_PATH=%WINBUILD_DIR:C:=%
SET DESKTOP_PATH=%DESKTOP_DIR:C:=%
Rem ---- Updating all Makefile
fart -c -i -r Makefile*  c:$(INSTALL_ROOT)%WINBUILD_PATH% c:$(INSTALL_ROOT)%DESKTOP_PATH%
set INSTALL_ROOT=
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin
call :addToPath "%BIN_DIR%"
set QMAKESPEC=C:\Qt487\winbuild\mkspecs\win32-g++
Rem ---- Start Qt Installation
mingw32-make install 2>&1 | "%TEE%" %STARTDIR%\Qt487-Install.log
if errorlevel 1 (
	call :screenAndLog "Error Installing Qt from %WINBUILD_DIR% to: %DESKTOP_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Installed Qt from %WINBUILD_DIR% to: %DESKTOP_DIR%"
)
cd %STARTDIR%
Rem  ---- Exit batch if install mode
IF /I [%USER_MODE%] EQU [install] goto JobDone

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
call :addToPath %CC_DIR%i686-w64-mingw32\bin
call :addToPath "%BIN_DIR%"
Rem ---- Configuring Qwt
call :screenAndLog "Configuring Qwt"
qmake qwt.pro 2>&1 | "%TEE%" %STARTDIR%\Qwt-Configure.log
if errorlevel 1 (
	call :screenAndLog "Error Configuring Qwt 6.1 Multiaxes in %QWT_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configured Qwt 6.1 Multiaxes in %QWT_DIR%"
)
Rem ---- Building Qwt
call :screenAndLog "Building Qwt"
mingw32-make  2>&1 | "%TEE%" %STARTDIR%\Qwt-Make.log
if errorlevel 1 (
	call :screenAndLog "Error Building Qwt 6.1 Multiaxes in %QWT_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Builded Qwt 6.1 Multiaxes in %QWT_DIR%"
)
Rem ---- Start Qwt Installation
call :screenAndLog "Installing Qwt in %DESKTOP_DIR%"
mingw32-make install 2>&1 | "%TEE%" %STARTDIR%\Qwt-Install.log
if errorlevel 1 (
	call :screenAndLog "Error Installing Qwt 6.1 Multiaxes from %QWT_DIR% to %DESKTOP_DIR%"
	goto AbortProcess
)  else  (
	xcopy %QWT_DIR%lib\qwt.dll %BIN_DIR% /Y /I
	call :screenAndLog "Installed Qwt 6.1 Multiaxes from %QWT_DIR% to %DESKTOP_DIR%"
)
Rem  ---- Exit batch if QWT
cd %STARTDIR%
IF /I [%USER_MODE%] EQU [qwt] goto JobDone

:buildQtSerial
call :screenAndLog "Configuring QtSerialPort Project"
Rem ---- Checking Sources
if NOT EXIST %QTSERIAL_DIR% (
	call :screenAndLog "QtSerialPort source dir [%QTSERIAL_DIR%] not found, please download Qt components"
	goto AbortProcess
)
if not EXIST %WINSPEC_DIR% (
	call :screenAndLog "You must configure and install Qt first"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin
call :addToPath "%BIN_DIR%"
Rem ---- Clear QtSerial Debug Mode Build Dir
cd %QTSERIAL_DIR%
IF EXIST %QTSERIAL_DIR%build_debug RD /S /Q %QTSERIAL_DIR%build_debug
MKDIR %QTSERIAL_DIR%build_debug
Rem ---- Configuring QtSerialPort in Debug Mode
call :screenAndLog "Configuring QtSerialPort in debug mode"
cd %QTSERIAL_DIR%build_debug
qmake ..\%QTSERIALPORT%.pro  CONFIG+=debug  2>&1 | "%TEE%"  %STARTDIR%\%QTSERIALPORT%_Configure.log
if errorlevel 1 (
	call :screenAndLog "Error Configuring %QTSERIALPORT% in %QTSERIAL_DIR%build_debug in Debug Mode"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configured %QTSERIALPORT% in %QTSERIAL_DIR%build_debug in Debug Mode"
)
Rem ---- Building QtSerialPort in Debug Mode
call :screenAndLog "Building %QTSERIALPORT% in Debug Mode"
mingw32-make  2>&1 | "%TEE%" %STARTDIR%\%QTSERIALPORT%_Build.log
if errorlevel 1 (
	call :screenAndLog "Error Building %QTSERIALPORT% in %QTSERIAL_DIR% in Debug Mode"
	goto AbortProcess
)  else  (
	call :screenAndLog "Builded %QTSERIALPORT% in %QTSERIAL_DIR%build_debug in Debug Mode"
)
Rem ---- Start QtSerialPort Installation in Debug Mode
call :screenAndLog "Installing %QTSERIALPORT% in %DESKTOP_DIR%"
mingw32-make install 2>&1 | "%TEE%" %STARTDIR%\%QTSERIALPORT%_Install.log
if errorlevel 1 (
	call :screenAndLog "Error Installing %QTSERIALPORT% from %QTSERIAL_DIR%build_debug to %DESKTOP_DIR% in Debug Mode"
	goto AbortProcess
)  else  (
	call :screenAndLog "Installed %QTSERIALPORT% from %QTSERIAL_DIR%build_debug to %DESKTOP_DIR% in Debug Mode"
)

Rem ---- Clear QtSerial Release Mode Build Dir
cd %QTSERIAL_DIR%
IF EXIST %QTSERIAL_DIR%build_release RD /S /Q %QTSERIAL_DIR%build_release
MKDIR %QTSERIAL_DIR%build_release
Rem ---- Configuring QtSerialPort in Release Mode
call :screenAndLog "Configuring QtSerialPort in release mode"
cd %QTSERIAL_DIR%build_release
qmake ..\%QTSERIALPORT%.pro  CONFIG+=release  2>&1 | "%TEE%" -a %STARTDIR%\%QTSERIALPORT%_Configure.log
if errorlevel 1 (
	call :screenAndLog "Error Configuring %QTSERIALPORT% in %QTSERIAL_DIR%build_release in Release Mode"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configured %QTSERIALPORT% in %QTSERIAL_DIR%build_release in Release Mode"
)
Rem ---- Building QtSerialPort in Release Mode
call :screenAndLog "Building %QTSERIALPORT% in Release Mode"
mingw32-make  2>&1 | "%TEE%" -a %STARTDIR%\%QTSERIALPORT%_Build.log
if errorlevel 1 (
	call :screenAndLog "Error Building %QTSERIALPORT% in %QTSERIAL_DIR%build_release in Release Mode"
	goto AbortProcess
)  else  (
	call :screenAndLog "Builded %QTSERIALPORT% in %QTSERIAL_DIR%build_release in Release Mode"
)
Rem ---- Start QtSerialPort Installation in Release Mode
call :screenAndLog "Installing %QTSERIALPORT% in %DESKTOP_DIR%"
mingw32-make install 2>&1 | "%TEE%" -a %STARTDIR%\%QTSERIALPORT%_Install.log
if errorlevel 1 (
	call :screenAndLog "Error Installing %QTSERIALPORT% from %QTSERIAL_DIR%build_release to %DESKTOP_DIR% in Release Mode"
	goto AbortProcess
)  else  (
	call :screenAndLog "Installed %QTSERIALPORT% from %QTSERIAL_DIR%build_release to %DESKTOP_DIR% in Release Mode"
)
Rem  ---- Exit batch if QtSerial
cd %STARTDIR%
IF /I [%USER_MODE%] EQU [qtserial] goto JobDone
Rem  ---- All Done
IF /I [%USER_MODE%] EQU [all] goto JobDone

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
echo mode = all: 		Do full Build and Install
echo mode = download: 	Download Components and exit
echo mode = configure: 	Configure Qt and exit
echo mode = build: 		Build Qt and exit
echo mode = install: 	Install configured Qt and exit
echo mode = qwt:		Configure, compile, install QWT Multiaxes	
echo mode = qtserial:	Configure, compile, install QtSerialPort

goto JobDone
