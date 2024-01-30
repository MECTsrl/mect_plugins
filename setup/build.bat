@echo off

rem memento cte/cte.pro:ATCM_VERSION (cte/CTE.pluginspec)
SET REV=3.5.0
SET REVISION="%REV%"
SET SETUP_DIR=%~dp0
SET OUT_DIR=%SETUP_DIR%
SET IN_DIR="C:\mect_plugins"
SET ROOT_DIR=C:\
Rem ---- Updating PATH if needed
rem SET PATH=%PATH%;C:\Qt487\desktop\mingw32\bin\
SET QT_DIR=C:\Qt487\
SET DESKTOP_DIR=%QT_DIR%desktop\
SET CREATOR_DIR=Qt487\desktop\QtCreator\
SET TEMPLATE_DIR=%CREATOR_DIR%share\qtcreator\templates\
SET BIN_DIR=%DESKTOP_DIR%bin\
SET CC_DIR=%DESKTOP_DIR%mingw32\
call :addToPath "%CC_DIR%bin"
call :addToPath %CC_DIR%i686-w64-mingw32\bin;
call :addToPath "%BIN_DIR%"

SET TARGET_LIST=AnyTPAC043 AnyTPAC070 TP1043_01_A TP1043_02_A TP1043_02_B TP1070_01_A TP1070_01_B TP1070_01_C TP1070_02_F TPAC1005 TPAC1007_03 TPAC1007_04_AA TPAC1007_04_AB TPAC1007_04_AC TPAC1007_04_AD TPAC1007_04_AE TPAC1008_02_AA TPAC1008_02_AB TPAC1008_02_AD TPAC1008_02_AE TPAC1008_02_AF TPAC1008_03_AC TPAC1008_03_AD TPLC050_01_AA TPLC100_01_AA TPLC100_01_AB TPX1043_03_C TPX1070_03_D TPX1070_03_E 

echo.
echo ----------------------------------------
echo Creating the version %REVISION%
echo ----------------------------------------
echo. 

SET ErrorLog="%OUT_DIR%\Build_%REV%.log"
echo Creating the version %REVISION% > %ErrorLog%

SET ORIGINAL=%CD%

rem -----------------------------------------------------------------------------
echo Clean up %OUT_DIR%\Qt487
IF EXIST %OUT_DIR%\Qt487 RD /S /Q %OUT_DIR%\Qt487

rem -----------------------------------------------------------------------------
echo MectSuite Tutorial help files
mkdir %OUT_DIR%\Qt487\desktop\doc\qch  >> %ErrorLog%
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc.qhp -o %OUT_DIR%\Qt487\desktop\doc\qch\doc.qch  >> %ErrorLog%
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_eng.qhp -o %OUT_DIR%\Qt487\desktop\doc\qch\doc_eng.qch  >> %ErrorLog%	
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_fr.qhp -o %OUT_DIR%\Qt487\desktop\doc\qch\doc_fr.qch  >> %ErrorLog%	
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_es.qhp -o %OUT_DIR%\Qt487\desktop\doc\qch\doc_es.qch  >> %ErrorLog%	
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_de.qhp -o %OUT_DIR%\Qt487\desktop\doc\qch\doc_de.qch  >> %ErrorLog%	

rem -----------------------------------------------------------------------------
echo SSH Keys
mkdir %OUT_DIR%\Qt487\keys
xcopy %OUT_DIR%\keys\*.* %OUT_DIR%\Qt487\keys /Q /Y /I >> %ErrorLog%

rem -----------------------------------------------------------------------------
echo QtCreator designer plugins (distclean + qmake + make + install + distclean)
cd /D %IN_DIR%\qt_plugins
time /t
mingw32-make distclean >> %ErrorLog% 2>&1
"%BIN_DIR%qmake.exe" qt_designer_plugins.pro -r -spec win32-g++ "CONFIG+=release" -config store -config trend -config recipe -config alarms "DEFINES += ATCM_VERSION=\"%REVISION%\"" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
del /q C:\Qt487\desktop\plugins\designer\atcm*
mingw32-make install >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make distclean >> %ErrorLog% 2>&1
time /t

rem CTE Crosstable Editor Plugin for QtCreator ----------------------
echo QtCreator cte plugin (distclean + qmake + make)
cd /D %IN_DIR%\cte
time /t
rem ---- Nuovo path di CTE Plugin
del /q %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\cte.dll
del /q %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\CTE.pluginspec

mingw32-make distclean >> %ErrorLog% 2>&1
"%BIN_DIR%qmake.exe" cte.pro -r -spec win32-g++ "CONFIG+=release"  "DEFINES += ATCM_VERSION=\"%REVISION%\"" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
time /t

rem CTC Crosstable Complier -------------------------------------------------------
echo (QtCreator) ctc.exe (distclean + make + copy + distclean)
cd /D %IN_DIR%\ctc
time /t
mingw32-make distclean >> %ErrorLog% 2>&1
"%BIN_DIR%qmake.exe" ctc.pro -r -spec win32-g++ "CONFIG+=release"  "DEFINES += ATCM_VERSION=\"%REVISION%\"" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
del /q %BIN_DIR%ctc.exe
copy %IN_DIR%\ctc\release\ctc.exe %BIN_DIR%ctc.exe /Y >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make distclean >> %ErrorLog% 2>&1
time /t

rem -----------------------------------------------------------------------------
echo QtCreator template wizards (copy)
cd /D %IN_DIR%\qt_templates
time /t

for /d %%a in ("%ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-*") do rd /s /q "%%~a"

mkdir %OUT_DIR%%TEMPLATE_DIR%wizards
xcopy ATCM-template-project %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project /Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
xcopy ATCM-template-form-class %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class /Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
for /d %%a in (%TARGET_LIST%) do (
	echo "        %%~a"  >> %ErrorLog% 2>&1
	mkdir "%ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a"
	xcopy "ATCM-template-project-%%~a" "%ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a" /Q /Y /E /S /I >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
)
time /t

rem imx28 Configuration skipped!! -----------------------------------------------------------------------------
SET TARGETBUILD=0
IF %TARGETBUILD% == 1 (
	echo Building target libraries distclean + qmake + make + install + distclean
	cd /D "%IN_DIR%"
	time /t
	mingw32-make.exe distclean >> %ErrorLog% 2>&1
	"C:\Qt487\imx28\qt-everywhere-opensource-src-4.8.7\bin\qmake.exe" qt_atcm.pro -r -spec linux-arm-gnueabi-g++ -config release -config store -config trend -config recipe -config alarms >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)	
	"C:\Qt487\imx28\mingw\bin\mingw32-make.exe" >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
	"C:\Qt487\imx28\mingw\bin\mingw32-make.exe" install >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
	"C:\Qt487\imx28\mingw\bin\mingw32-make.exe" distclean  >> %ErrorLog% 2>&1
	time /t
)

rem -----------------------------------------------------------------------------
echo QtProject_487.7z (zip)
time /t
if exist "%OUT_DIR%\QtProject_487.7z" del "%OUT_DIR%\QtProject_487.7z"
rem "c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject\qtcreator\devices.xml" "%APPDATA%\QtProject\qtcreator\profiles.xml" "%APPDATA%\QtProject\qtcreator\qtversion.xml" "%APPDATA%\QtProject\qtcreator\toolchains.xml" "%APPDATA%\QtProject\qtcreator\externaltools\lupdate.xml" >> %ErrorLog%
rem "c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject" -x!QtCreator.db -x!QtCreator.ini -xr!"qtcreator\generic-highlighter" -xr!"qtcreator\json" -xr!qtcreator\macros -x!qtcreator\default.qws -x!qtcreator\helpcollection.qhc >> %ErrorLog%
"c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject_487.7z" "%OUT_DIR%\QtProject" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)

rem -----------------------------------------------------------------------------
echo Qt487_upd_rev%REV%.7z (del + copy {*.dll, wizards, cte, ctc, rootfs, qmake.conf}, zip, clean "Qt487")
time /t

mkdir %OUT_DIR%\Qt487\desktop\plugins\designer
xcopy C:\Qt487\desktop\plugins\designer\atcm*.dll %OUT_DIR%\Qt487\desktop\plugins\designer /Q /Y >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mkdir %OUT_DIR%%TEMPLATE_DIR%wizards
xcopy %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project %OUT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project				/Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
xcopy %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class %OUT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class	/Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
for /d %%a in (%TARGET_LIST%) do (
	xcopy %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a %OUT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a				/Q /Y /E /S /I >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
)
rem CTE Plugin 
mkdir %OUT_DIR%\%CREATOR_DIR%lib\qtcreator\plugins\Mect\
mkdir %OUT_DIR%\Qt487\desktop\bin
copy %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\CTE.dll %OUT_DIR%\%CREATOR_DIR%lib\qtcreator\plugins\Mect\ /Y >> %ErrorLog%
copy %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\CTE.pluginspec %OUT_DIR%\%CREATOR_DIR%lib\qtcreator\plugins\Mect\ /Y >> %ErrorLog%
copy %BIN_DIR%ctc.exe %OUT_DIR%\Qt487\desktop\bin\ /Y >> %ErrorLog%


rem imx28 Configuration skipped!! -----------------------------------------------------------------------------
IF %TARGETBUILD% == 1 (

	mkdir %OUT_DIR%\Qt487\imx28
	xcopy C:\Qt487\imx28\rootfs %OUT_DIR%\Qt487\imx28\rootfs	/Q /Y /E /S /I >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
	mkdir "%OUT_DIR%\Qt487\imx28\qt-everywhere-opensource-src-4.8.7\mkspecs\linux-arm-gnueabi-g++"
	mkdir %OUT_DIR%\Qt487\imx28\qt-everywhere-opensource-src-4.8.7\mkspecs\common
	copy "%ORIGINAL%\mkspecs\linux-arm-gnueabi-g++\qmake.conf.mect" "%OUT_DIR%\Qt487\imx28\qt-everywhere-opensource-src-4.8.7\mkspecs\linux-arm-gnueabi-g++\qmake.conf"	/Y >> %ErrorLog% 2>&1
	copy %ORIGINAL%\mkspecs\common\mect.conf %OUT_DIR%\Qt487\imx28\qt-everywhere-opensource-src-4.8.7\mkspecs\common	/Y >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
)

rem Clean-Up Qt487 Dir
cd /D %OUT_DIR%\Qt487
del /s/q "*.bak"

cd /D %OUT_DIR%
IF EXIST "%OUT_DIR%\Qt487_upd_rev%REV%.7z" del "%OUT_DIR%\Qt487_upd_rev%REV%.7z"
"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt487_upd_rev%REV%.7z %OUT_DIR%\Qt487\desktop >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)

rem imx28 Configuration skipped!! -----------------------------------------------------------------------------
IF %TARGETBUILD% == 1 (
	"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt487_upd_rev%REV%.7z %OUT_DIR%\Qt487\imx28 >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
)

"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt487_upd_rev%REV%.7z %OUT_DIR%\Qt487\keys >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)

RD /S /Q Qt487
time /t

rem -----------------------------------------------------------------------------
echo   Fonts.7z
if exist "%OUT_DIR%\Fonts.7z" del "%OUT_DIR%\Fonts.7z"
"c:\Program Files\7-Zip\7z.exe" u -r -mx9 "%OUT_DIR%\Fonts.7z" "%OUT_DIR%\Fonts" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
time /t


cd %ORIGINAL%

echo OK: build done.
@echo on
pause
exit 0 /b	

:addToPath
	echo ;%PATH%; | find /C /I ";%~1;"1>NUL
	if errorlevel 1 (
		SET PATH=%PATH%;%~1;
		echo Added %~1 to Path Variables
	)  else (
		echo %~1 already in Path
	)
	exit /b	

