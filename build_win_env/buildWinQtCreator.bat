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
SET SRC_DIR=%TEMP_DIR%qt-everywhere-opensource-src-4.8.7\
Rem ---- File Download program
SET TRANSFER_CMD=%CD%\getFileFromArchive.bat
SET EXTRACT_CMD="c:\Program Files\7-Zip\7z.exe" x -y -r 
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
echo Creating Qt Creator Version %QT_CREATOR_VERSION% for %QT_VERSION%
echo ----------------------------------------
echo. 

:downloadCreator
call :screenAndLog "Downloading Sources %CREATOR_SOURCES% for %QT_CREATOR_VERSION%"
call  %TRANSFER_CMD% %CREATOR_SOURCES% %TEMP_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Downloading %CREATOR_SOURCES%"
	goto AbortProcess
)  else  (
	call :screenAndLog "Downloaded %CREATOR_SOURCES%"
)
call :screenAndLog "Expanding Sources %CREATOR_SOURCES%"
%EXTRACT_CMD%  %TEMP_DIR%%CREATOR_SOURCES% -o%ROOT_DIR%
if errorlevel 1 (
	call :screenAndLog "Error Expanding:  %CREATOR_SOURCES% to %ROOT_DIR%"
	goto AbortProcess
)
call :screenAndLog "Extraction completed for Qt %CREATOR_SOURCES%"
IF [%USER_MODE%] EQU [download] goto JobDone


:configureCreator
call :screenAndLog "Configuring Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"


:buildCreator
call :screenAndLog "Building Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"


:installCreator
call :screenAndLog "Installing Qt Creator %QT_CREATOR_VERSION% for Qt %QT_VERSION%"


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

goto JobDone
