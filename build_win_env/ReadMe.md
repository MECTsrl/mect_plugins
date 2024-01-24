# Building the Qt 4.8.7 and Mect Suite 3.5 compilation environment for Windows
***
## This folder contains instructions to rebuild the C:\Qt487\desktop folder and compile Qt 4.8.7 from the Sources and the necessary tools (Windows Plugins) for the operation of Mect Suite 3.5
***
Supported operating system: 
- Windows 7 32/64 bit
- Windows 10 64 bit

Prerequisites:
- Please read https://wiki.qt.io/Building_Qt_Desktop_for_Windows_with_MinGW

Requested components (must be installed separately before starting buildWinEnv.bat):
- Git for Windows *(https://git-scm.com/download/win)*
- Curl for Windows *(https://curl.se/download.html)*, native for Windows 10
- 7zip for Windows *(https://www.7-zip.org/download.html)*
- Active Perl for Windows. Use getPerl.bat to download it from https://www.mect.it/archive/
- MySql mysql-connector-c-6.1.11-win32 installed in C:/MySQLConnector if you want to configure Windows plugin for MySQL (use Use getMySql.bat to download it from https://www.mect.it/archive/)





