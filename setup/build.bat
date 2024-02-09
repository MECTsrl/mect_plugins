@echo off

rem memento cte/cte.pro:ATCM_VERSION (cte/CTE.pluginspec)
SET REV=3.5.0
SET REVISION="%REV%"
SET SETUP_DIR=%~dp0
SET OUT_DIR=%SETUP_DIR%
SET ErrorLog="%OUT_DIR%Build_%REV%.log"
SET IN_DIR="C:\mect_plugins"
SET ROOT_DIR=C:\
SET ORIGINAL=%CD%
SET TEE=%ProgramFiles%\Git\usr\bin\tee
SET ZIP_CMD="%ProgramFiles%\7-Zip\7z.exe"
Rem ---- Updating PATH if needed
rem SET PATH=%PATH%;C:\Qt487\desktop\mingw32\bin\
SET QT_DIR=C:\Qt487\
SET IMX28_DIR=imx28\
SET DESKTOP_DIR=%QT_DIR%desktop\
SET CREATOR_DIR=Qt487\desktop\QtCreator\
SET TEMPLATE_DIR=%CREATOR_DIR%share\qtcreator\templates\
SET BIN_DIR=%DESKTOP_DIR%bin\
SET CC_DIR=%DESKTOP_DIR%mingw32\
call :addToPath %CC_DIR%bin
call :addToPath %CC_DIR%i686-w64-mingw32\bin
call :addToPath %BIN_DIR%

SET TARGET_LIST=AnyTPAC043 AnyTPAC070 TP1043_01_A TP1043_02_A TP1043_02_B TP1070_01_A TP1070_01_B TP1070_01_C TP1070_02_F TPAC1005 TPAC1007_03 TPAC1007_04_AA TPAC1007_04_AB TPAC1007_04_AC TPAC1007_04_AD TPAC1007_04_AE TPAC1008_02_AA TPAC1008_02_AB TPAC1008_02_AD TPAC1008_02_AE TPAC1008_02_AF TPAC1008_03_AC TPAC1008_03_AD TPLC050_01_AA TPLC100_01_AA TPLC100_01_AB TPX1043_03_C TPX1070_03_D TPX1070_03_E 

IF EXIST %ErrorLog% del %ErrorLog%
call :screenAndLog "----------------------------------------"
call :screenAndLog "Creating the version %REVISION% in [%OUT_DIR%]"
call :screenAndLog "----------------------------------------"



rem -----------------------------------------------------------------------------
call :screenAndLog "Clean up %OUT_DIR%Qt487"
IF EXIST %OUT_DIR%Qt487 RD /S /Q %OUT_DIR%Qt487

rem -----------------------------------------------------------------------------
call :screenAndLog "MectSuite Tutorial help files"
mkdir %OUT_DIR%Qt487\desktop\doc\qch  
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc.qhp -o %OUT_DIR%Qt487\desktop\doc\qch\doc.qch   >> %ErrorLog% 2>&1
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_eng.qhp -o %OUT_DIR%Qt487\desktop\doc\qch\doc_eng.qch   >> %ErrorLog% 2>&1
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_fr.qhp -o %OUT_DIR%Qt487\desktop\doc\qch\doc_fr.qch   >> %ErrorLog% 2>&1
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_es.qhp -o %OUT_DIR%Qt487\desktop\doc\qch\doc_es.qch   >> %ErrorLog% 2>&1
c:\Qt487\desktop\bin\qhelpgenerator.exe ..\qt_help\tutorial\doc_de.qhp -o %OUT_DIR%Qt487\desktop\doc\qch\doc_de.qch   >> %ErrorLog% 2>&1

rem -----------------------------------------------------------------------------
call :screenAndLog "SSH Keys"
mkdir %OUT_DIR%Qt487\keys
xcopy %OUT_DIR%keys\*.* %OUT_DIR%Qt487\keys /Q /Y /I >> %ErrorLog% 2>&1

rem -----------------------------------------------------------------------------
call :screenAndLog "QtCreator designer plugins (distclean + qmake + make + install + distclean)"
cd /D %IN_DIR%\qt_plugins
time /t
mingw32-make distclean >> %ErrorLog% 2>&1
"%BIN_DIR%qmake.exe" qt_designer_plugins.pro -r -spec win32-g++ "CONFIG+=release" -config store -config trend -config recipe -config alarms "DEFINES += ATCM_VERSION=\"%REVISION%\""  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
mingw32-make  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
del /q C:\Qt487\desktop\plugins\designer\atcm*
mingw32-make install  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
mingw32-make distclean  >> %ErrorLog% 2>&1
time /t

rem CTE Crosstable Editor Plugin for QtCreator ----------------------
call :screenAndLog "QtCreator cte plugin (distclean + qmake + make)"
cd /D %IN_DIR%\cte
time /t
rem ---- Nuovo path di CTE Plugin
del /q %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\cte.dll
del /q %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\CTE.pluginspec

mingw32-make distclean  >> %ErrorLog% 2>&1
"%BIN_DIR%qmake.exe" cte.pro -r -spec win32-g++ "CONFIG+=release"  "DEFINES += ATCM_VERSION=\"%REVISION%\""  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
mingw32-make  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
time /t

rem CTC Crosstable Complier -------------------------------------------------------
call :screenAndLog "(QtCreator) ctc.exe (distclean + make + copy + distclean)"
cd /D %IN_DIR%\ctc
time /t
mingw32-make distclean  >> %ErrorLog% 2>&1
"%BIN_DIR%qmake.exe" ctc.pro -r -spec win32-g++ "CONFIG+=release"  "DEFINES += ATCM_VERSION=\"%REVISION%\""  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
mingw32-make >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
Rem del /q %BIN_DIR%ctc.exe  >> %ErrorLog% 2>&1 :Moved from C:\Qt487\desktop\bin to C:\Qt487\desktop\QtCreator\bin
del /q %ROOT_DIR%%CREATOR_DIR%bin\ctc.exe  >> %ErrorLog% 2>&1
copy %IN_DIR%\ctc\release\ctc.exe %ROOT_DIR%%CREATOR_DIR%bin\ctc.exe /Y  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
mingw32-make distclean  >> %ErrorLog% 2>&1
time /t

rem -----------------------------------------------------------------------------
call :screenAndLog "QtCreator template wizards (copy)"
cd /D %IN_DIR%\qt_templates
time /t

for /d %%a in ("%ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-*") do rd /s /q "%%~a"

mkdir %OUT_DIR%%TEMPLATE_DIR%wizards
xcopy ATCM-template-project %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project /Q /Y /E /S /I  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
xcopy ATCM-template-form-class %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class /Q /Y /E /S /I  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
for /d %%a in (%TARGET_LIST%) do (
	call :screenAndLog "Model:        %%~a"
	mkdir "%ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a"
	xcopy "ATCM-template-project-%%~a" "%ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a" /Q /Y /E /S /I  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)
)
time /t

rem Copy current imx28 Configuration (root-fs and qt Configuration)
cd /D "%OUT_DIR%"
IF EXIST %QT_DIR%%IMX28_DIR%  (
	call :screenAndLog "Dumping [%QT_DIR%%IMX28_DIR%] to [%OUT_DIR%Qt487\%IMX28_DIR%]"
	time /t
	IF EXIST %OUT_DIR%Qt487\%IMX28_DIR% ( 
		call :screenAndLog "%OUT_DIR%Qt487\%IMX28_DIR%"
		RD /S /Q %OUT_DIR%Qt487\%IMX28_DIR%
	)
	mkdir    %OUT_DIR%Qt487\%IMX28_DIR%
	call :screenAndLog "Copy I.MX28 Configuration from [%QT_DIR%%IMX28_DIR%]  to [%OUT_DIR%Qt487\%IMX28_DIR%]"
	xcopy %QT_DIR%%IMX28_DIR% %OUT_DIR%Qt487\%IMX28_DIR% /Q /Y /E /S /I  >> %ErrorLog% 2>&1
	time /t
	rem Removing extra files
	call :screenAndLog "Removing %OUT_DIR%Qt487\imx28\demos" 
	RD /S /Q %OUT_DIR%Qt487\imx28\demos 
	call :screenAndLog "Removing %OUT_DIR%Qt487\imx28\doc"
	RD /S /Q %OUT_DIR%Qt487\imx28\doc 
	call :screenAndLog "Removing %OUT_DIR%Qt487\imx28\examples" 
	RD /S /Q %OUT_DIR%Qt487\imx28\examples 
	time /t
)

rem imx28 Configuration skipped!! -----------------------------------------------------------------------------
SET TARGETBUILD=0
IF %TARGETBUILD% == 1 (
	call :screenAndLog "Building target libraries distclean + qmake + make + install + distclean"
	cd /D "%IN_DIR%"
	time /t
	mingw32-make.exe distclean  >> %ErrorLog% 2>&1
	"C:\Qt487\imx28\qt-everywhere-opensource-src-4.8.7\bin\qmake.exe" qt_atcm.pro -r -spec linux-arm-gnueabi-g++ -config release -config store -config trend -config recipe -config alarms  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)	
	mingw32-make.exe  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)
	mingw32-make.exe install  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)
	mingw32-make.exe distclean   >> %ErrorLog% 2>&1
	time /t
)

rem -----------------------------------------------------------------------------
call :screenAndLog "QtProject_487.7z (zip)"
time /t
if exist "%OUT_DIR%QtProject_487.7z" (
	call :screenAndLog "removing previous %OUT_DIR%QtProject_487.7z"
	del "%OUT_DIR%QtProject_487.7z"
)
rem %ZIP_CMD% u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject\qtcreator\devices.xml" "%APPDATA%\QtProject\qtcreator\profiles.xml" "%APPDATA%\QtProject\qtcreator\qtversion.xml" "%APPDATA%\QtProject\qtcreator\toolchains.xml" "%APPDATA%\QtProject\qtcreator\externaltools\lupdate.xml" >> %ErrorLog%
rem %ZIP_CMD% u -r -mx1 "%OUT_DIR%\QtProject.7z" "%APPDATA%\QtProject" -x!QtCreator.db -x!QtCreator.ini -xr!"qtcreator\generic-highlighter" -xr!"qtcreator\json" -xr!qtcreator\macros -x!qtcreator\default.qws -x!qtcreator\helpcollection.qhc >> %ErrorLog%
call :screenAndLog "Creating %OUT_DIR%QtProject_487.7z"
%ZIP_CMD% a -r -mx1 "%OUT_DIR%QtProject_487.7z" "%OUT_DIR%QtProject" >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)

rem Qt487_upd_X.Y.Z -----------------------------------------------------------------------------
IF EXIST "%OUT_DIR%Qt487_upd_rev%REV%.7z" (
	call :screenAndLog "removing previous %OUT_DIR%Qt487_upd_rev%REV%.7z" 
	del "%OUT_DIR%Qt487_upd_rev%REV%.7z"
)
call :screenAndLog "Qt487_upd_rev%REV%.7z (del + copy {*.dll, wizards, cte, ctc, rootfs, qmake.conf}, zip, clean)"
time /t

call :screenAndLog "Dumping Designer Plugins"
mkdir %OUT_DIR%Qt487\desktop\plugins\designer
xcopy C:\Qt487\desktop\plugins\designer\atcm*.dll %OUT_DIR%Qt487\desktop\plugins\designer /Q /Y  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)

call :screenAndLog "Dumping %OUT_DIR%%TEMPLATE_DIR%wizards"  
IF NOT EXIST  %OUT_DIR%%TEMPLATE_DIR%wizards mkdir %OUT_DIR%%TEMPLATE_DIR%wizards
xcopy %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project %OUT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project				/Q /Y /E /S /I  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
call :screenAndLog "Dumping %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class"
xcopy %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class %OUT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-form-class	/Q /Y /E /S /I  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
for /d %%a in (%TARGET_LIST%) do (
	xcopy %ROOT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a %OUT_DIR%%TEMPLATE_DIR%wizards\ATCM-template-project-%%~a				/Q /Y /E /S /I  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)
)

call :screenAndLog "Dumping CTE Plugin from %OUT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\"
mkdir %OUT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\
mkdir %OUT_DIR%Qt487\desktop\bin
copy %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\CTE.dll %OUT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\ /Y  >> %ErrorLog% 2>&1
copy %ROOT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\CTE.pluginspec %OUT_DIR%%CREATOR_DIR%lib\qtcreator\plugins\Mect\ /Y  >> %ErrorLog% 2>&1
call :screenAndLog "Dumping CTC %ROOT_DIR%%CREATOR_DIR%bin\ctc.exe to %OUT_DIR%%CREATOR_DIR%bin\"
xcopy %ROOT_DIR%%CREATOR_DIR%bin\ctc.exe %OUT_DIR%%CREATOR_DIR%bin\ /I /Y  >> %ErrorLog% 2>&1


rem imx28 Reconfiguration skipped!! -----------------------------------------------------------------------------
Rem Chek it !!
IF %TARGETBUILD% == 1 (

	mkdir %OUT_DIR%Qt487\imx28
	xcopy C:\Qt487\imx28\rootfs %OUT_DIR%Qt487\imx28\rootfs	/Q /Y /E /S /I  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)
	mkdir "%OUT_DIR%Qt487\imx28\mkspecs\linux-arm-gnueabi-g++"
	mkdir %OUT_DIR%Qt487\imx28\mkspecs\common
	copy "%ORIGINAL%\mkspecs\linux-arm-gnueabi-g++\qmake.conf.mect" "%OUT_DIR%Qt487\imx28\mkspecs\linux-arm-gnueabi-g++\qmake.conf"	/Y 2>&1  >> %ErrorLog% 2>&1
	copy %ORIGINAL%\mkspecs\common\mect.conf %OUT_DIR%Qt487\imx28\qt-everywhere-opensource-src-4.8.7\mkspecs\common	/Y  >> %ErrorLog% 2>&1
	IF ERRORLEVEL 1 (
		goto AbortProcess
	)
)

rem Clean-Up Qt487 Dir
cd /D %OUT_DIR%Qt487
del /s/q "*.bak"

time /t
cd /D %OUT_DIR%
call :screenAndLog "Creating [%OUT_DIR%Qt487_upd_rev%REV%.7z]"
call :screenAndLog "Compressing %OUT_DIR%Qt487\desktop"
%ZIP_CMD% a -r -mx9 %OUT_DIR%Qt487_upd_rev%REV%.7z %OUT_DIR%Qt487\desktop  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
time /t

Rem Dumping full imx28 Configuration -----------------------------------------------------------------------------
call :screenAndLog "Compressing %OUT_DIR%Qt487\imx28"
%ZIP_CMD% u -r -mx9 %OUT_DIR%Qt487_upd_rev%REV%.7z %OUT_DIR%Qt487\imx28  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
time /t

call :screenAndLog "Compressing %OUT_DIR%Qt487\keys"
%ZIP_CMD% u -r -mx9 %OUT_DIR%Qt487_upd_rev%REV%.7z %OUT_DIR%Qt487\keys >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
time /t

call :screenAndLog "Clean-Up [%OUT_DIR%Qt487] Dir"

RD /S /Q Qt487
time /t

rem -----------------------------------------------------------------------------
call :screenAndLog "Fonts.7z"
if exist "%OUT_DIR%Fonts.7z" (
	call :screenAndLog "Removing previous %OUT_DIR%Fonts.7z"
	del "%OUT_DIR%Fonts.7z"
)

call :screenAndLog "Creating %OUT_DIR%Fonts.7z"
%ZIP_CMD% a -r -mx9 "%OUT_DIR%Fonts.7z" "%OUT_DIR%Fonts"  >> %ErrorLog% 2>&1
IF ERRORLEVEL 1 (
	goto AbortProcess
)
time /t


cd %ORIGINAL%

call :screenAndLog "build done."
pause
@echo on
exit /B	0

:addToPath
	echo ;%PATH%; | find /C /I ";%~1;"1>NUL
	if errorlevel 1 (
		SET PATH=%PATH%;%~1;
		echo Added %~1 to Path Variables
	)  else (
		echo %~1 already in Path
	)
	exit /B

:screenAndLog 
	SETLOCAL
	set message=%~1
	echo %message% & echo %TIME%: %message% >> %ErrorLog%
	ENDLOCAL
	exit /b	

:AbortProcess
	call :screenAndLog "Build Aborted!"
	cd %ORIGINAL%
	Pause
	@echo on
	exit /B 1
