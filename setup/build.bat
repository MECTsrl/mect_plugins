@echo off

SET REV=3.0.0rc12
SET REVISION="%REV%"
SET SETUP_DIR=%~dp0
SET OUT_DIR=%SETUP_DIR%
SET IN_DIR="C:\mect_plugins"
SET PATH=%PATH%;C:\Qt485\desktop\mingw32\bin\

rem SET TARGET_LIST=TP1043_01_A TP1043_01_B TP1043_01_C TP1057_01_A TP1057_01_B TP1070_01_A TP1070_01_B TP1070_01_C TP1070_01_D TPAC1006 TPAC1007_03 TPAC1007_04_AA TPAC1007_04_AB TPAC1007_04_AC TPAC1008_01 TPAC1008_02_AA TPAC1008_02_AB TPAC1008_02_AC TPAC1008_02_AD TPAC1008_02_AE TPAC1008_02_AF
SET TARGET_LIST=TP1043_01_A TP1043_01_B TP1057_01_A TP1057_01_B TP1070_01_A TP1070_01_B TP1070_01_C TPAC1006 TPAC1007_03 TPAC1007_04_AA TPAC1007_04_AB TPAC1007_04_AC TPAC1008_01 TPAC1008_02_AA TPAC1008_02_AB TPAC1008_02_AD TPAC1008_02_AE TPAC1008_02_AF TPLC100_01_AA TPLC100_01_AB

echo.
echo ----------------------------------------
echo Creating the version %REVISION%
echo ----------------------------------------
echo. 

SET ErrorLog="%OUT_DIR%\Build_%REV%.log"
echo Creating the version %REVISION% > %ErrorLog%

SET ORIGINAL=%CD%

rem -----------------------------------------------------------------------------
echo Clean up %OUT_DIR%\Qt485
IF EXIST %OUT_DIR%\Qt485 RD /S /Q %OUT_DIR%\Qt485

rem -----------------------------------------------------------------------------
echo MectSuite Tutorial help files
mkdir %OUT_DIR%\Qt485\desktop\doc\qch  >> %ErrorLog%
c:\Qt485\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc.qhp -o %OUT_DIR%\Qt485\desktop\doc\qch\doc.qch  >> %ErrorLog%
c:\Qt485\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_eng.qhp -o %OUT_DIR%\Qt485\desktop\doc\qch\doc_eng.qch  >> %ErrorLog%	

rem -----------------------------------------------------------------------------
echo QtCreator designer plugins (distclean + qmake + make + install + distclean)
cd /D %IN_DIR%\qt_plugins
time /t
mingw32-make distclean >> %ErrorLog% 2>&1
"C:\Qt485\desktop\bin\qmake.exe" qt_designer_plugins.pro -r -spec win32-g++ "CONFIG+=release" -config store -config trend -config recipe -config alarms "DEFINES += ATCM_VERSION=\"%REVISION%\"" >> %ErrorLog% 2>&1
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
del /q C:\Qt485\desktop\plugins\designer\atcm*
mingw32-make install >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make distclean >> %ErrorLog% 2>&1
time /t

rem -----------------------------------------------------------------------------
echo QtCreator cte plugin (distclean + qmake + make + copy + distclean)
cd /D %IN_DIR%\cte
time /t
mingw32-make distclean >> %ErrorLog% 2>&1
"C:\Qt485\desktop\bin\qmake.exe" cte.pro -r -spec win32-g++ "CONFIG+=release"  "DEFINES += ATCM_VERSION=\"%REVISION%\"" >> %ErrorLog% 2>&1
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
del /q C:\Qt485\desktop\lib\qtcreator\plugins\QtProject\cte.dll
del /q C:\Qt485\desktop\lib\qtcreator\plugins\QtProject\CTE.pluginspec
copy %IN_DIR%\cte\destdir\CTE.dll C:\Qt485\desktop\lib\qtcreator\plugins\QtProject\CTE.dll /Y >> %ErrorLog%
copy %IN_DIR%\cte\CTE.pluginspec  C:\Qt485\desktop\lib\qtcreator\plugins\QtProject\CTE.pluginspec /Y >> %ErrorLog%
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mingw32-make distclean >> %ErrorLog% 2>&1
time /t

rem -----------------------------------------------------------------------------
echo (QtCreator) ctc.exe (distclean + make + copy + distclean)
cd /D %IN_DIR%\ctc
time /t
mingw32-make distclean >> %ErrorLog% 2>&1
"C:\Qt485\desktop\bin\qmake.exe" ctc.pro -r -spec win32-g++ "CONFIG+=release"  "DEFINES += ATCM_VERSION=\"%REVISION%\"" >> %ErrorLog% 2>&1
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
del /q C:\Qt485\desktop\bin\ctc.exe
copy %IN_DIR%\ctc\release\ctc.exe C:\Qt485\desktop\bin\ctc.exe /Y >> %ErrorLog% 2>&1
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

for /d %%a in ("C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-*") do rd /s /q "%%~a"

mkdir %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards
xcopy ATCM-template-project C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project /Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
xcopy ATCM-template-form-class C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class /Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
for /d %%a in (%TARGET_LIST%) do (
	echo "        %%~a"  >> %ErrorLog% 2>&1
	mkdir "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a"
	xcopy "ATCM-template-project-%%~a" "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a" /Q /Y /E /S /I >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
)
time /t

rem -----------------------------------------------------------------------------
SET TARGETBUILD=0
IF %TARGETBUILD% == 1 (
	echo Building target libraries distclean + qmake + make + install + distclean
	cd /D "%IN_DIR%"
	time /t
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" distclean >> %ErrorLog% 2>&1
	"C:\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\bin\qmake.exe" qt_atcm.pro -r -spec linux-arm-gnueabi-g++ -config release -config store -config trend -config recipe -config alarms >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)	
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" install >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
	"C:\Qt485\imx28\mingw\bin\mingw32-make.exe" distclean  >> %ErrorLog% 2>&1
	time /t
)

rem -----------------------------------------------------------------------------
echo QtProject.7z (zip)
time /t
if exist "%OUT_DIR%\QtProject.7z" del "%OUT_DIR%\QtProject.7z"
rem "c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject\qtcreator\devices.xml" "%APPDATA%\QtProject\qtcreator\profiles.xml" "%APPDATA%\QtProject\qtcreator\qtversion.xml" "%APPDATA%\QtProject\qtcreator\toolchains.xml" "%APPDATA%\QtProject\qtcreator\externaltools\lupdate.xml" >> %ErrorLog%
rem "c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject" -x!QtCreator.db -x!QtCreator.ini -xr!"qtcreator\generic-highlighter" -xr!"qtcreator\json" -xr!qtcreator\macros -x!qtcreator\default.qws -x!qtcreator\helpcollection.qhc >> %ErrorLog%
"c:\Program Files\7-Zip\7z.exe" u -r -mx1 "%OUT_DIR%\QtProject.7z" "%OUT_DIR%\QtProject" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)

rem -----------------------------------------------------------------------------
echo Qt485_upd_rev%REV%.7z (del + copy {*.dll, wizards, cte, ctc, rootfs, qmake.conf}, zip, clean "Qt485")
time /t

mkdir %OUT_DIR%\Qt485\desktop\plugins\designer
xcopy C:\Qt485\desktop\plugins\designer\atcm*.dll %OUT_DIR%\Qt485\desktop\plugins\designer /Q /Y >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mkdir %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards
xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project				/Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class	/Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
for /d %%a in (%TARGET_LIST%) do (
	xcopy C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a %OUT_DIR%\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-%%~a				/Q /Y /E /S /I >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		echo ERROR
		pause
		cd %ORIGINAL%
		exit
	)
)
mkdir %OUT_DIR%\Qt485\desktop\lib\qtcreator\plugins\QtProject
mkdir %OUT_DIR%\Qt485\desktop\bin
copy C:\Qt485\desktop\lib\qtcreator\plugins\QtProject\CTE.dll %OUT_DIR%\Qt485\desktop\lib\qtcreator\plugins\QtProject\ /Y >> %ErrorLog%
copy C:\Qt485\desktop\lib\qtcreator\plugins\QtProject\CTE.pluginspec %OUT_DIR%\Qt485\desktop\lib\qtcreator\plugins\QtProject\ /Y >> %ErrorLog%
copy C:\Qt485\desktop\bin\ctc.exe %OUT_DIR%\Qt485\desktop\bin\ /Y >> %ErrorLog%
mkdir %OUT_DIR%\Qt485\imx28
xcopy C:\Qt485\imx28\rootfs %OUT_DIR%\Qt485\imx28\rootfs	/Q /Y /E /S /I >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
mkdir "%OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++"
mkdir %OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\common
copy "%ORIGINAL%\mkspecs\linux-arm-gnueabi-g++\qmake.conf.mect" "%OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\linux-arm-gnueabi-g++\qmake.conf"	/Y >> %ErrorLog% 2>&1
copy %ORIGINAL%\mkspecs\common\mect.conf %OUT_DIR%\Qt485\imx28\qt-everywhere-opensource-src-4.8.5\mkspecs\common	/Y >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)

cd /D %OUT_DIR%\Qt485
del /s/q "*.bak"

cd /D %OUT_DIR%
IF EXIST "%OUT_DIR%\Qt485_upd_rev%REV%.7z" del "%OUT_DIR%\Qt485_upd_rev%REV%.7z"
"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt485_upd_rev%REV%.7z %OUT_DIR%\Qt485\desktop >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)
"c:\Program Files\7-Zip\7z.exe" u -r -mx9 %OUT_DIR%\Qt485_upd_rev%REV%.7z %OUT_DIR%\Qt485\imx28 >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	echo ERROR
	pause
	cd %ORIGINAL%
	exit
)

RD /S /Q Qt485
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
