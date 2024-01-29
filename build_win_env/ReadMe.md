# Building the Qt 4.8.7 and Mect Suite 3.5 compilation environment for Windows
***
## This folder contains instructions to rebuild the C:\Qt487\desktop folder and compile Qt 4.8.7 from the Sources and the necessary tools (Windows Plugins) for the operation of Mect Suite 3.5 
## The version of Qt 4.8.7 for Windows Desktop obtained in this way can be used to compile Qt Creator 2.8.1 from source and install it in the C:\Qt487\desktop\QtCreator folder  
***
Supported operating system: 
- Windows 7 32/64 bit
- Windows 10 64 bit

Prerequisites:
- Please read https://wiki.qt.io/Building_Qt_Desktop_for_Windows_with_MinGW

Requested components (must be installed separately before starting buildWinEnv.bat):
- Git for Windows *(https://git-scm.com/download/win)*
- Curl for Windows *(https://curl.se/download.html)*, (native for Windows 10)
- 7zip for Windows *(https://www.7-zip.org/download.html)*
- Active Perl for Windows. Use getPerl.bat to download it from https://www.mect.it/archive/
- MySql mysql-connector-c-6.1.11-win32 installed in C:\MySQLConnector if you want to configure Windows plugin for MySQL (use Use getMySql.bat to download it from https://www.mect.it/archive/)

Building Qt 4.8.7 Windows Desktop folder (C:\Qt487\desktop)





