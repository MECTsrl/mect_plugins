@echo off
SET TRANSFERCMD=%CD%\getFileFromArchive.bat
SET TEMP_DIR=C:\Ms35_tmp
IF NOT EXIST %TEMP_DIR% MKDIR %TEMP_DIR%
call  %TRANSFERCMD% "ActivePerl-5.16.3.1603-MSWin32-x86-296746.msi" %TEMP_DIR%
if errorlevel 1 goto errorDownload
echo File Downloaded in %TEMP_DIR%
Pause
exit /b

:errorDownload
echo Error Downloading file: ActivePerl-5.16.3.1603-MSWin32-x86-296746.msi
Pause
exit /b
