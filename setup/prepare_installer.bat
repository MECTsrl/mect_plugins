@echo off

SET REVISION="7.0rc10"
SET MECT_CONFIGURATOR_REVISION="1.8"

SET SETUP_DIR=%~dp0
SET OUT_DIR=%SETUP_DIR%
SET IN_DIR="C:\Users\UserName\Documents\GitHub\mect_plugins"

SET QTPROJECT=0
SET BUILD=0
SET INSTALL=1
SET REPAIR=0
SET UPDATE=0

echo.
echo Revision: %REVISION%
echo.
echo Building: %BUILD%
echo Install: %INSTALL%
echo Repair: %REPAIR%
echo Update: %UPDATE%
echo.

echo.
echo ----------------------------------------
echo Creating the installer verion %REVISION%
echo ----------------------------------------
echo. 

IF EXIST %OUT_DIR%\Qt485 RD /S /Q %OUT_DIR%\Qt485

rem ##############################################
rem BUILDING VERSION
rem ##############################################
IF %BUILD% == 1 (
	SET DLLBUILD=0
	SET TEMPLATE=1
	SET TARGETBUILD=0
) ELSE (
	SET DLLBUILD=0
	SET TEMPLATE=0
	SET TARGETBUILD=0
)

IF %DLLBUILD% == 1 (
	echo Building dll libraries...
	cd /D %IN_DIR%\qt_plugins
	time /t
	"C:\Qt485\desktop\mingw32\bin\mingw32-make.exe" distclean>nul 2>&1
	"C:\Qt485\desktop\bin\qmake.exe" %IN_DIR%\qt_plugins\qt_designer_plugins.pro -r -spec win32-g++ "CONFIG+=release" -config store -config trend -config recipe -config alarms "DEFINES += ATCM_VERSION=\"%REVISION%\"" > %OUT_DIR%\error.log 2>&1
	rem "C:\Qt485\desktop\bin\qmake.exe" %IN_DIR%\qt_plugins\qt_designer_plugins.pro -r -spec win32-g++ -config release -config store -config trend -config recipe -config alarms "DEFINES += ATCM_VERSION=\"%REVISION%\"" >nul 2>&1
	IF ERRORLEVEL 1 (
		echo problem during Building dll libraries
		pause
		exit
	)
	"C:\Qt485\desktop\mingw32\bin\mingw32-make.exe" > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during Building dll libraries
		pause
		exit
	)
 	rem remove the old files
 	del /q C:\Qt485\desktop\plugins\designer\atcm*
	"C:\Qt485\desktop\mingw32\bin\mingw32-make.exe" install > %OUT_DIR%\error.log 2>&1
 	IF ERRORLEVEL 1 (
		echo problem during installation dll libraries
		pause
		exit
	)
	"C:\Qt485\desktop\mingw32\bin\mingw32-make.exe" distclean>nul 2>&1
	time /t
)

IF %TEMPLATE% == 1 (
	echo Installing templates...
	time /t
	for /d %%a in ("C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-*") do rd /s /q "%%~a"
	mkdir %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards
	xcopy %IN_DIR%\qt_templates\ATCM-template-project C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		exit
	)

	xcopy %IN_DIR%\qt_templates\ATCM-template-form-class C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		exit
	)
	time /t
)

IF %TARGETBUILD% == 1 (
	echo Building target libraries...
	cd /D "%IN_DIR%"
	time /t
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" distclean>nul 2>&1
	"C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\bin\qmake.exe" %IN_DIR%\qt_atcm.pro -r -spec linux-arm-gnueabi-g++ -config release -config store -config trend -config recipe -config alarms > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during crating make file for target libraries
		pause
		exit
	)
	
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during building target libraries
		pause
		exit
	)

	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" install > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during install target libraries
		pause
		exit
	)

	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" distclean>nul 2>&1
	time /t
)

rem ##############################################
rem CONFIGURATION QTPROJECT
rem ##############################################
IF %QTPROJECT% == 1 (
	echo Preparing Configuration into QtProject...
	time /t
	rem "c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject\qtcreator\devices.xml" "%APPDATA%\QtProject\qtcreator\profiles.xml" "%APPDATA%\QtProject\qtcreator\qtversion.xml" "%APPDATA%\QtProject\qtcreator\toolchains.xml" "%APPDATA%\QtProject\qtcreator\externaltools\lupdate.xml" > %OUT_DIR%\error.log
	del "%OUT_DIR%\QtProject.7z"
	"c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject" -x!QtCreator.db -x!QtCreator.ini -xr!"qtcreator\generic-highlighter" -xr!"qtcreator\json" -xr!qtcreator\macros -x!qtcreator\default.qws -x!qtcreator\helpcollection.qhc > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
	 	exit
	)
)

rem ##############################################
rem UPDATE
rem ##############################################
IF %INSTALL% == 1 set PREPARE_UPDATE=1
IF %UPDATE% == 1 set PREPARE_UPDATE=1
IF %PREPARE_UPDATE% == 1 (
	echo Creating the update
	time /t
	echo Copying dll...
	mkdir %OUT_DIR%\Qt485\desktop\plugins\designer
	xcopy C:\Qt485\desktop\plugins\designer\*.dll %OUT_DIR%\Qt485\desktop\plugins\designer /Q /Y > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during .dll copy.
		pause
		exit
	)

	echo Copying template...
	mkdir %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards
	xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project				/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		exit
	)

	xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		exit
	)

	echo Copying rootfs...
	xcopy C:\Qt485\imx28\rootfs %OUT_DIR%\Qt485\imx28\rootfs	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		exit
	)

	echo Cleaning svn files...
	cd /D %OUT_DIR%\Qt485
	for /d /r . %%d in (.svn) do @if exist "%%d" rd /s/q "%%d"
	cd ..

	echo Cleaning bak files...
	cd /D %OUT_DIR%\Qt485
	for /d /r . %%d in (*.bak) do @if exist "%%d" rd /s/q "%%d"
	cd ..

	echo Creating archive...
	IF EXIST Qt485_upd_rev%REVISION%.7z del Qt485_upd_rev%REVISION%.7z

	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt485_upd_rev%REVISION%.7z %OUT_DIR%\Qt485\imx28 > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of 7z update archive.
		pause
		exit
	)
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt485_upd_rev%REVISION%.7z %OUT_DIR%\Qt485\Desktop > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of 7z update archive.
		pause
		exit
	)

	IF %UPDATE% == 1 (
		echo Creating setup...
		cd /D "%SETUP_DIR%"
		"c:\Program Files\NSIS\makensis.exe" /DREVISION=%REVISION% /DMECT_CONFIGURATOR_REVISION=%MECT_CONFIGURATOR_REVISION% UPDATE_MECT_QT.nsi > %OUT_DIR%\error.log 2>&1
		IF ERRORLEVEL 1 (
			echo problem during creation of update.
			pause
			exit
		)
	)
	
	RD /S /Q Qt485
	time /t
)

rem ##############################################
rem INSTALLATION
rem ##############################################
IF %INSTALL% == 1 (
	echo Please check if you have rebuild the Target library and the Desktop library
	echo. 

	echo Preparing files...
	time /t
	echo   Target files...
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Qt485.7z" "C:\Qt485\imx28" > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
	  	exit
	)
	echo   PC files...
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Qt485.7z" "C:\Qt485\Desktop" > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	  	echo problem during creation 7z file
	  	pause
	  	exit
	)
	echo   Configurator files...
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\MectConfigurator.7z" "%OUT_DIR%\MectConfigurator" > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
	  	exit
	)
	echo   Fonts files...
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Fonts.7z" "%OUT_DIR%\Fonts" > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
	  	exit
	)
	time /t

	echo Preparing setup...
	time /t
	cd /D "%SETUP_DIR%"
	"c:\Program Files\NSIS\makensis.exe" /DREVISION=%REVISION% SETUP_MECT_QT.nsi > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of setup
		pause
		exit
	)
	time /t
	rem del %OUT_DIR%\Qt485.7z
	del %OUT_DIR%\MectConfigurator.7z
)

rem ##############################################
rem REPAIR
rem ##############################################
IF %REPAIR% == 1 (
	echo Creating the repair...
	time /t
	cd /D "%SETUP_DIR%"
	"c:\Program Files\NSIS\makensis.exe" /DREVISION=%REVISION% SETUP_MECT_QTrepair.nsi > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of repair
		pause
		exit
	)
	time /t
)

IF EXIST %OUT_DIR%\error.log del %OUT_DIR%\error.log

IF EXIST %OUT_DIR%\Qt485 RD /S /Q %OUT_DIR%\Qt485

IF EXIST %OUT_DIR%\Qt485_upd_rev%REVISION%.7z del %OUT_DIR%\Qt485_upd_rev%REVISION%.7z

echo Setup done.
@echo on

pause
