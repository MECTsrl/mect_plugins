@echo off
Rem ---- Setting Variable Names
SET STARTDIR=%CD%
SET OUT_DIR=C:\Qt487\
SET DESKTOP_DIR=%OUT_DIR%desktop\
SET BIN_DIR=%DESKTOP_DIR%bin
SET SRC_DIR=%DESKTOP_DIR%src
SET DOC_DIR=%DESKTOP_DIR%doc
SET CC_DIR=%DESKTOP_DIR%mingw32
SET CREATOR_DIR=%DESKTOP_DIR%QtCreator
SET WINBUILD_DIR=%OUT_DIR%winbuild
SET QT_VERSION=4.8.7
SET TEMP_DIR=C:\Ms35_tmp\
echo.
echo ----------------------------------------
echo Creating Windows Building Environment for %QT_VERSION%
echo ----------------------------------------
echo. 
SET ErrorLog="%STARTDIR%\%~n0.log"
echo %DATE% - %TIME%: Creating the Desktop Environment for Qt %QT_VERSION% in %DESKTOP_DIR% > %ErrorLog%

IF EXIST %TEMP_DIR% RD /S /Q %TEMP_DIR%
MKDIR %TEMP_DIR%
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
