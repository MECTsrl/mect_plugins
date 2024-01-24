@echo off
Rem ---- Setting Global Variables
SET USER_MODE=%1
SET STARTDIR=%CD%
SET QT_VERSION=4.8.7
Rem ---- Build Dirs
SET OUT_DIR=C:\Qt487\
SET DESKTOP_DIR=%OUT_DIR%desktop\
SET BIN_DIR=%DESKTOP_DIR%bin
SET SRC_DIR=%OUT_DIR%qt-everywhere-opensource-src-4.8.7
SET DOC_DIR=%DESKTOP_DIR%doc
SET CC_DIR=%DESKTOP_DIR%mingw32
SET CREATOR_DIR=%DESKTOP_DIR%QtCreator
SET WINBUILD_DIR=%OUT_DIR%winbuild
SET TEMP_DIR=C:\Ms35_tmp\
SET OPENSSL_DIR=C:\openssl-1.0.2u
Rem ---- File Download program
SET TRANSFER_CMD=%CD%\getFileFromArchive.bat
SET EXTRACT_CMD="c:\Program Files\7-Zip\7z.exe" x -y -r 
Rem ---- File to be downloaded 
SET DOWNLOAD_LIST=MingW_i686-4.8.2-release-posix-dwarf-rt_v3-rev3.7z qt-everywhere-opensource-src-4.8.7.zip openssl-1.0.2u_winbuild.7z qt-tools.7z qt487_doc.7z
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
call :screenAndLog "Downloading Components for Qt %QT_VERSION%"
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
Rem ---- qt-everywhere-opensource-src-4.8.7.zip
IF EXIST %SRC_DIR% RD /S /Q %SRC_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qt-everywhere-opensource-src-4.8.7.zip -o%OUT_DIR%
if errorlevel 1 (
		call :screenAndLog "Error Expanding: qt-everywhere-opensource-src-4.8.7.zip to %OUT_DIR%"
		goto AbortProcess
)
Rem ---- openssl-1.0.2u_winbuild.7z
IF EXIST %OPENSSL_DIR% RD /S /Q %OPENSSL_DIR%
MKDIR %OPENSSL_DIR%
%EXTRACT_CMD%  %TEMP_DIR%openssl-1.0.2u_winbuild.7z -oC:\
if errorlevel 1 (
		call :screenAndLog "Error Expanding: openssl-1.0.2u_winbuild.7z to C:\"
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
Rem ---- Qt 487 Doc qt487_doc.7z
IF EXIST %DOC_DIR% RD /S /Q %DOC_DIR%
%EXTRACT_CMD%  %TEMP_DIR%qt487_doc.7z -o%DESKTOP_DIR%
if errorlevel 1 (
		call :screenAndLog "Error Expanding: qt487_doc.7z to %DESKTOP_DIR%"
		goto AbortProcess
)
call :screenAndLog "Extraction completed for Qt %QT_VERSION%"
IF [%USER_MODE%] EQU [download] goto JobDone

:configureQt
call :screenAndLog "Configuring  Qt %QT_VERSION% in Folder %WINBUILD_DIR%"
IF EXIST %WINBUILD_DIR% RD /S /Q %WINBUILD_DIR%
Rem ---- To be completed....
call :screenAndLog "Configuration completed for Qt %QT_VERSION% in Folder %WINBUILD_DIR%"
Rem  ---- Exit batch if configure mode
IF [%USER_MODE%] EQU [configure] goto JobDone

:buildQt
call :screenAndLog "Building  Qt %QT_VERSION% in %WINBUILD_DIR%"

Rem  ---- Exit batch if build mode
IF [%USER_MODE%] EQU [build] goto JobDone

:installQt
call :screenAndLog "Installing  Qt %QT_VERSION% from %WINBUILD_DIR%"

Rem  ---- Exit batch if install mode
IF [%USER_MODE%] EQU [install] goto JobDone

:JobDone
	call :screenAndLog "Done."
	pause
	exit /b 

:AbortProcess
	call :screenAndLog "Build Aborted!"
	exit /b 1

:screenAndLog 
	SETLOCAL
	set message=%~1
	echo %message% & echo %TIME%: %message% >> %ErrorLog%
	ENDLOCAL
	exit /b	

:showUsage
echo Usage: %~n0 mode
echo mode = all: 		Do full Build
echo mode = download: 	Download Components and exit
echo mode = configure: 	Configure Qt and exit
echo mode = build: 		Build Qt and exit
echo mode = install: 	Install configured Qt and exit

goto JobDone
