@echo off
Rem ---- Checking Params
IF [%1]==[] goto showUsage

SETLOCAL 

Rem ---- Destination Directory
IF [%2]==[] (Set DEST_DIR=%CD%) else (Set DEST_DIR=%2)
SET STARTDIR=%CD%
Rem ---- Switching to Destination Dir (needed for md5 calculation)
IF /I  %STARTDIR% NEQ %DEST_DIR% CD %DEST_DIR%

Rem ---- Setting Source and destination paths and file names
Set ARCHIVE_SRC=http://www.mect.it/archive/build_win_env/
Set SOURCE_FILE=%~nx1
Set DEST_FILE=%DEST_DIR%\%SOURCE_FILE%

echo.
echo ----------------------------------------
echo Getting file %SOURCE_FILE% from %ARCHIVE_SRC% to %DEST_FILE%
echo ----------------------------------------
echo. 

rem ---- Getting requested file
curl -L --output %DEST_FILE% %ARCHIVE_SRC%%SOURCE_FILE%
if errorlevel 1 goto errCurl

rem ---- Getting associated md5 file
curl -L --output %DEST_FILE%.md5 %ARCHIVE_SRC%%SOURCE_FILE%.md5
if errorlevel 1 goto errCurlMD5

rem ---- Calculating MD5 for received file
echo Calculating MD5 for %DEST_FILE%
"%ProgramFiles%\Git\usr\bin\md5sum" -b  %SOURCE_FILE% > %SOURCE_FILE%.md5.new
if errorlevel 1 goto errCalcMD5

rem ---- Comparing MD5 for received file
fc %DEST_FILE%.md5 %DEST_FILE%.md5.new >NUL
if errorlevel 1 goto errMD5
echo Successfully downloaded File:%SOURCE_FILE% from %ARCHIVE_SRC%
IF /I  %STARTDIR% NEQ %DEST_DIR% CD %STARTDIR%
exit /b 0

:errCurl
:errCurlMD5
echo Error Downloading file:%SOURCE_FILE%
IF /I  %STARTDIR% NEQ %DEST_DIR% CD %STARTDIR%
exit /b 1

:errCalcMD5
echo Error Calculating MD5 for file:%DEST_FILE%
IF /I  %STARTDIR% NEQ %DEST_DIR% CD %STARTDIR%
exit /b 2

:errMD5
echo Error Comparing %DEST_FILE%.md5 with %DEST_FILE%.md5.new
IF /I  %STARTDIR% NEQ %DEST_DIR% CD %STARTDIR%
exit /b 3

:showUsage
echo Usage: %~n0 "Archive Source File" "Destination Directory" 
IF /I  %STARTDIR% NEQ %DEST_DIR% CD %STARTDIR%
exit /b -1
