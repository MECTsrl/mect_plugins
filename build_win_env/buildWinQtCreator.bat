@echo off
Rem ---- Setting Global Variables
SET USER_MODE=%1
SET STARTDIR=%CD%
SET QT_VERSION=4.8.7
SET QT_CREATOR_VERSION=2.8.1
Rem ---- Build Dirs
SET ROOT_DIR=C:\
SET OUT_DIR=C:\Qt487\
SET DESKTOP_DIR=%OUT_DIR%desktop\
SET BIN_DIR=%DESKTOP_DIR%bin\
SET CC_DIR=%DESKTOP_DIR%mingw32\
SET CREATOR_DIR=%DESKTOP_DIR%QtCreator
SET CREATOR_SOURCES=qt-creator-2.8.1-src.zip
SET TEMP_DIR=C:\Ms35_tmp\
SET CREATOR_SRC_DIR=%ROOT_DIR%%CREATOR_SOURCES:.zip=\%
Rem ---- File Download program
SET TRANSFER_CMD=%CD%\getFileFromArchive.bat
SET EXTRACT_CMD="%ProgramFiles%\7-Zip\7z.exe" x -y -r 
SET ErrorLog=%STARTDIR%\%~n0.log
Rem ---- Checking Params
IF [%USER_MODE%] EQU [] goto showUsage
echo %DATE% - %TIME%: Starting: %~n0 Param: [%USER_MODE%]  > %ErrorLog%

Rem ---- Download Qt Creator Sources
IF [%USER_MODE%] EQU [download] (
	echo Downloading Sources for %QT_CREATOR_VERSION%
	goto downloadCreator
)
Rem ---- Configure Qt Creator 
IF [%USER_MODE%] EQU [configure] (
	echo Configuring Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%
	goto configureCreator
)
Rem ---- Build Qt Qt Creator 
IF [%USER_MODE%] EQU [build] (
	echo Building Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%
	goto buildCreator
)

Rem ---- Install Qt 
IF [%USER_MODE%] EQU [install] (
	echo Installing Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%
	goto installCreator
)

echo.
echo ----------------------------------------
echo Creating Qt Creator Version %QT_CREATOR_VERSION% for %QT_VERSION% in %CREATOR_SRC_DIR%
echo ----------------------------------------
echo. 

:downloadCreator
IF EXIST %CREATOR_SRC_DIR% RD /S /Q %CREATOR_SRC_DIR%
call :screenAndLog "Downloading Sources %CREATOR_SOURCES% for %QT_CREATOR_VERSION%"
call  %TRANSFER_CMD% %CREATOR_SOURCES% %TEMP_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Downloading %CREATOR_SOURCES%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Downloaded %CREATOR_SOURCES%"
)
call :screenAndLog "Expanding Sources %CREATOR_SOURCES% to %CREATOR_SRC_DIR%" 
%EXTRACT_CMD%  %TEMP_DIR%%CREATOR_SOURCES% -o%ROOT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding:  %CREATOR_SOURCES% to %ROOT_DIR%"
	goto AbortProcess
)
call :screenAndLog "Extraction completed for Qt %CREATOR_SOURCES%"
IF [%USER_MODE%] EQU [download] goto JobDone

:configureCreator
CD %CREATOR_SRC_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %CREATOR_SRC_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
Rem ---- Configure Qt Creator
call :screenAndLog "Configuring Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"
qmake CONFIG+=release -r 2>&1 | "%ProgramFiles%\Git\usr\bin\tee"  %TEMP_DIR%QtCreator281-Configure_Release.Log
if errorlevel 1 (
	call :screenAndLog "Error Configuring Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Configured  Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"
)
IF [%USER_MODE%] EQU [configure] goto JobDone

:buildCreator
CD %CREATOR_SRC_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %CREATOR_SRC_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
Rem ---- Build  Qt Creator
call :screenAndLog "Building Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"
mingw32-make release 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%QtCreator281-Make_Release.Log
if errorlevel 1 (
	call :screenAndLog "Error Building Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Builded Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"
)
IF [%USER_MODE%] EQU [build] goto JobDone


:installCreator
CD %CREATOR_SRC_DIR%
if errorlevel 1 (
	call :screenAndLog "Error entering  Build Directory: %CREATOR_SRC_DIR%"
	goto AbortProcess
)
Rem ---- Updating PATH if needed
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"
Rem ---- Install Qt Creator
call :screenAndLog "Installing Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION% to %CREATOR_DIR%""
mingw32-make install INSTALL_ROOT=%CREATOR_DIR% 2>&1 | "%ProgramFiles%\Git\usr\bin\tee" %TEMP_DIR%QtCreator281-Install_Release.log
if errorlevel 1 (
	call :screenAndLog "Error Installing Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION% to %CREATOR_DIR%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Installed Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION% to %CREATOR_DIR%"
)
IF [%USER_MODE%] EQU [install] goto JobDone

:JobDone
	call :screenAndLog "Done."
	cd %STARTDIR%
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

goto JobDone
