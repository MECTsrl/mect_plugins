@echo off

SET REVISION="2.0.9rc3"
SET SETUP_DIR=%~dp0
SET OUT_DIR=%SETUP_DIR%
SET IN_DIR="C:\Users\UserName\Documents\GitHub\mect_plugins"

SET TARGET_LIST=TP1043_01_A TP1043_01_B TP1043_01_C TP1057_01_A TP1057_01_B TP1070_01_A TP1070_01_B TP1070_01_C TP1070_01_D TPAC1006 TPAC1007_03 TPAC1007_04_AA TPAC1007_04_AB TPAC1007_04_AC TPAC1008_01 TPAC1008_02_AA TPAC1008_02_AB TPAC1008_02_AC TPAC1008_02_AD TPAC1008_02_AE TPAC1008_02_AF

rem extract MECT_CONFIGURATOR_REVISION
FOR /f "eol=#tokens=2delims==" %%a IN ('findstr DistributionVersion %OUT_DIR%\MectConfigurator\MectConfiguratorInstaller\Volume\nidist.id') DO SET MECT_CONFIGURATOR_REVISION="%%a"

SET QTPROJECT=1
SET BUILD=1
SET INSTALL=0
SET UPDATE=1

IF %UPDATE% == 1 (
	SET PREPARE_UPDATE=1
) ELSE (
	SET PREPARE_UPDATE=0
)

echo.
echo Mect plugin Revision: %REVISION%
echo MectConfigurator Revision: %MECT_CONFIGURATOR_REVISION%
echo.
echo Building: %BUILD%
echo Install: %INSTALL%
echo.

echo.
echo ----------------------------------------
echo Creating the installer verion %REVISION%
echo ----------------------------------------
echo. 

SET ORIGINAL=%CD%

IF EXIST %OUT_DIR%\Qt485 RD /S /Q %OUT_DIR%\Qt485

rem ##############################################
rem BUILDING VERSION
rem ##############################################
IF %BUILD% == 1 (
	SET DLLBUILD=1
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
	"C:\Qt485\desktop\bin\qmake.exe" qt_designer_plugins.pro -r -spec win32-g++ "CONFIG+=release" -config store -config trend -config recipe -config alarms "DEFINES += ATCM_VERSION=\"%REVISION%\"" > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during Building dll libraries
		pause
		cd %ORIGINAL%
		exit
	)

	"C:\Qt485\desktop\mingw32\bin\mingw32-make.exe" > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during Building dll libraries
		pause
		cd %ORIGINAL%
		exit
	)

	rem remove the old files
 	del /q C:\Qt485\desktop\plugins\designer\atcm*
	"C:\Qt485\desktop\mingw32\bin\mingw32-make.exe" install > %OUT_DIR%\error.log 2>&1
 	IF ERRORLEVEL 1 (
		echo problem during installation dll libraries
		pause
		cd %ORIGINAL%
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
	
	xcopy %IN_DIR%\qt_templates\ATCM-template-project C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project /Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		cd %ORIGINAL%
		exit
	)

	xcopy %IN_DIR%\qt_templates\ATCM-template-form-class C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class /Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		cd %ORIGINAL%
		exit
	)

	for /d %%a in (%TARGET_LIST%) do (
		echo 	Installing %%~a...
		xcopy "%IN_DIR%\qt_templates\ATCM-template-project-%%~a" "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a" /Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
		IF ERRORLEVEL 1 (
			echo problem during template.
			pause
			cd %ORIGINAL%
			exit
		)
	)

	time /t
)

IF %TARGETBUILD% == 1 (
	echo Building target libraries...
	cd /D "%IN_DIR%"
	time /t
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" distclean>nul 2>&1
	"C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\bin\qmake.exe" qt_atcm.pro -r -spec linux-arm-gnueabi-g++ -config release -config store -config trend -config recipe -config alarms > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during crating make file for target libraries
		pause
		cd %ORIGINAL%
		exit
	)
	
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during building target libraries
		pause
		cd %ORIGINAL%
		exit
	)

	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" install > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during install target libraries
		pause
		cd %ORIGINAL%
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
	del /q "%OUT_DIR%\QtProject.7z"
	"c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject" -x!QtCreator.db -x!QtCreator.ini -xr!"qtcreator\generic-highlighter" -xr!"qtcreator\json" -xr!qtcreator\macros -x!qtcreator\default.qws -x!qtcreator\helpcollection.qhc > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
		cd %ORIGINAL%
	 	exit
	)
)

rem ##############################################
rem UPDATE
rem ##############################################
IF %INSTALL% == 1 (
	SET PREPARE_UPDATE=1
)

IF %PREPARE_UPDATE% == 1 (
	echo Creating the update
	time /t
	echo Copying dll...
	mkdir %OUT_DIR%\Qt485\desktop\plugins\designer
	xcopy C:\Qt485\desktop\plugins\designer\*.dll %OUT_DIR%\Qt485\desktop\plugins\designer /Q /Y > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during .dll copy.
		pause
		cd %ORIGINAL%
		exit
	)

	echo Copying template...
	mkdir %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards
	xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project				/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		cd %ORIGINAL%
		exit
	)

	xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		cd %ORIGINAL%
		exit
	)

	for /d %%a in (%TARGET_LIST%) do (
		echo Installing %%~a...
		xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a				/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
		IF ERRORLEVEL 1 (
			echo problem during template.
			pause
			cd %ORIGINAL%
			exit
		)
	)

	echo Copying rootfs...
	mkdir %OUT_DIR%\Qt485\imx28
	xcopy C:\Qt485\imx28\rootfs %OUT_DIR%\Qt485\imx28\rootfs	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		cd %ORIGINAL%
		exit
	)

	echo Copying conf files...
	mkdir %OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++
	mkdir %OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\common
	xcopy C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf %OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	xcopy C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\common\mect.conf %OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\common	/Q /Y /E /S /I > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during template.
		pause
		cd %ORIGINAL%
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
	IF EXIST Qt485_upd_rev%REVISION%.7z del /q Qt485_upd_rev%REVISION%.7z

	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt485_upd_rev%REVISION%.7z %OUT_DIR%\Qt485\imx28 > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of 7z update archive.
		pause
		cd %ORIGINAL%
		exit
	)

	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt485_upd_rev%REVISION%.7z %OUT_DIR%\Qt485\Desktop > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of 7z update archive.
		pause
		cd %ORIGINAL%
		exit
	)

	RD /S /Q Qt485
	time /t

	echo   Configurator files...
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\MectConfigurator.7z" "%OUT_DIR%\MectConfigurator" > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
		cd %ORIGINAL%
	  	exit
	)
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
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Qt485.7z" "C:\Qt485\imx28" -xr!rootfs -x!qt-everywhere-opensource-src-4.8.5\mkspecs\common\mect.conf -x!qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf  > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
		cd %ORIGINAL%
	  	exit
	)
	echo   PC files...
	copy "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf" "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf.mect"
	copy "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf.ori" "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf"
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Qt485.7z" "C:\Qt485\Desktop"  -xr!atcm*.dll -xr!ATCM-template-* > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
		copy "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf.mect" "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf"
		echo problem during creation 7z file
		pause
		cd %ORIGINAL%
		exit
	)
	copy "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf.mect" "C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf"

	echo   Fonts files...
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Fonts.7z" "%OUT_DIR%\Fonts" > %OUT_DIR%\error.log
	IF ERRORLEVEL 1 (
	 	echo problem during creation 7z file
	 	pause
		cd %ORIGINAL%
	  	exit
	)
	time /t

	echo Preparing setup...
	time /t
	cd /D "%SETUP_DIR%"
	"c:\Program Files\NSIS\makensis.exe" /DREVISION=%REVISION% /DMECT_CONFIGURATOR_REVISION=%MECT_CONFIGURATOR_REVISION% SETUP_MECT_QT.nsi > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of setup
		pause
		cd %ORIGINAL%
		exit
	)
	time /t
)

IF %UPDATE% == 1 (
	echo Preparing update...
	time /t
	cd /D "%SETUP_DIR%"
	"c:\Program Files\NSIS\makensis.exe" /DREVISION=%REVISION% /DMECT_CONFIGURATOR_REVISION=%MECT_CONFIGURATOR_REVISION% UPDATE_MECT_QT.nsi > %OUT_DIR%\error.log 2>&1
	IF ERRORLEVEL 1 (
		echo problem during creation of setup
		pause
		cd %ORIGINAL%
		exit
	)
	time /t
)

rem IF EXIST %OUT_DIR%\Qt485.7z del /q %OUT_DIR%\Qt485.7z

IF EXIST %OUT_DIR%\MectConfigurator.7z del /q %OUT_DIR%\MectConfigurator.7z

IF EXIST %OUT_DIR%\error.log del /q %OUT_DIR%\error.log

IF EXIST %OUT_DIR%\Qt485 RD /S /Q %OUT_DIR%\Qt485

IF EXIST %OUT_DIR%\Qt485_upd_rev%REVISION%.7z del /q %OUT_DIR%\Qt485_upd_rev%REVISION%.7z

cd %ORIGINAL%

echo Setup done.
@echo on

pause
