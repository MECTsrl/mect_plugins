@echo off
SET TRANSFERCMD=%CD%\getFileFromArchive.bat
SET TEMP_DIR=C:\Ms35_tmp
IF NOT EXIST %TEMP_DIR% MKDIR %TEMP_DIR%
call  %TRANSFERCMD% "mysql-connector-c-6.1.11-win32.msi" %TEMP_DIR%
if errorlevel 1 goto errorDownload
echo File Downloaded in %TEMP_DIR%
exit /b

:errorDownload
echo Error Downloading file: mysql-connector-c-6.1.11-win32.msi
exit /b
