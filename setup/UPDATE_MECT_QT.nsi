; MECT_QT.nsi
;
; This script generates  MECT QT installer.

;--------------------------------

;  REV 1.0.0	PRIMA EMISSIONE 13/05/2014
; 
;	

!define TEMP1 $R3 ;Temp variable

!ifdef HAVE_UPX
!packhdr tmp.dat "upx\upx -9 tmp.dat"
!endif

!ifdef NOCOMPRESS
SetCompress off
!endif
;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------

!define NOINSTTYPES
!define MUI_ICON mect.ico 
!define MUI_COMPONENTSPAGE_NODESC

!include "FileFunc.nsh"
!insertmacro GetSize
!insertmacro DriveSpace
;--------------------------------

Name "MECT"
Caption "MECT QT rev. ${REVISION}"

OutFile "MectSuite_${REVISION}_Update.exe"

BrandingText /TRIMCENTER "MECT QT ${REVISION}"
SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
BGGradient 000000  000060 FFFFFF
InstallColors FF8080 000030
XPStyle on

InstallDir "C:\Qt485\"

;CheckBitmap classic-cross.bmp

;LicenseText "License"
;LicenseData "License.dat"

;--------------------------------
  !insertmacro MUI_PAGE_LICENSE $(MUILicense)
  !insertmacro MUI_PAGE_COMPONENTS
  ;!insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  ;!insertmacro MUI_PAGE_FINISH
;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English" # first language is the default language

  !insertmacro MUI_RESERVEFILE_LANGDLL


;--------------------------------

;License Language String

  LicenseLangString MUILicense ${LANG_ENGLISH} "License.txt"

;--------------------------------

!ifndef NOINSTTYPES ; only if not defined

  InstType "Typical"
;  InstType "null"

!endif

AutoCloseWindow false
ShowInstDetails show

;--------------------------------

Section "" ; empty string makes it hidden, so would starting with -

   	SetOutPath $INSTDIR	
	DetailPrint "TMP1 = $TEMP1  "
SectionEnd

Section "Mect plugins ${REVISION}"
	SectionIn 1
	
	SetOutPath "C:\Qt485"
	RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project"
	RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class"
	RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-TP1057HR-template-form-class"
	RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-TP1057-template-form-class"
	RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-TPAC1007-template-form-class"
	RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-TPAC1008-template-form-class"
	RMDir /r "C:\Qt485\imx28\rootfs"
	SetOverwrite on
	SetCompress off
	File "Qt485_upd_rev${REVISION}.7z"
	Nsis7z::ExtractWithDetails "Qt485_upd_rev${REVISION}.7z" "Installing update %..."
	Delete "Qt485_upd_rev${REVISION}.7z"
	
	SetOutPath "$APPDATA"
	SetOverwrite on
	SetCompress off
	File "QtProject.7z"
	Nsis7z::ExtractWithDetails "QtProject.7z"  "Installing QtProject files %..."
	Delete "QtProject.7z"

	SetOutPath "C:\Qt485"
	SetOverwrite on

	nsExec::ExecToLog 'echo "%PATH%"|findstr /i /c:"C:\Perl\bin">nul || C:\Qt485\Desktop\bin\pathman /au "C:\Perl\bin"'
	nsExec::ExecToLog 'echo "%PATH%"|findstr /i /c:"C:\Qt485\imx28\mingw\bin">nul || C:\Qt485\Desktop\bin\pathman /au "C:\Qt485\imx28\mingw\bin"'

	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite" "DisplayVersion" '${REVISION}'

SectionEnd

Section "Mect Configurator ${MECT_CONFIGURATOR_REVISION}"
	SectionIn 1
	SetOutPath "$TEMP"
	SetOverwrite on
	SetCompress off
	RMDir /r  "$TEMP\MectConfigurator"

	File "MectConfigurator.7z"
	Nsis7z::ExtractWithDetails "MectConfigurator.7z" "Installing MectConfigurator ${MECT_CONFIGURATOR_REVISION} %..."
	Delete "MectConfigurator.7z"

	ExecWait "MectConfigurator\MectConfiguratorInstaller\Volume\setup.exe /q /acceptlicenses yes /r:n"
	RMDir /r  "$TEMP\MectConfigurator"

	nsExec::ExecToLog 'IF NOT EXIST "c:\Program Files\MectConfiguratorInstaller" mklink /d   "c:\Program Files\MectConfiguratorInstaller" "c:\Program Files (x86)/MectConfiguratorInstaller"'
SectionEnd
