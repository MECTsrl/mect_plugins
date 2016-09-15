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

OutFile "MectSuite_${REVISION}_Setup.exe"

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

SectionGroup "Toolchain"
	
	Section "CodeSourcery"		
		SectionIn 1
		SetOutPath "$TEMP"		
		File "arm-2011.03-41-arm-none-linux-gnueabi.exe"
		;ExecWait "arm-2011.03-41-arm-none-linux-gnueabi.exe -i silent"
		;ExecWait "arm-2011.03-41-arm-none-linux-gnueabi.exe -i GUI silent"
		ExecWait "arm-2011.03-41-arm-none-linux-gnueabi.exe -i GUI"
		Delete "arm-2011.03-41-arm-none-linux-gnueabi.exe"		
	SectionEnd

SectionGroupEnd

SectionGroup "Perl"

	Section "Perl"
		SectionIn 1
		SetOutPath "$TEMP"
		File "ActivePerl-5.16.3.1603-MSWin32-x86-296746.msi"		
		ExecWait "msiexec /qb /i ActivePerl-5.16.3.1603-MSWin32-x86-296746.msi PERL_PATH=Yes PERL_EXT=Yes"
		Delete "ActivePerl-5.16.3.1603-MSWin32-x86-296746.msi"	
	SectionEnd
	
SectionGroupEnd

SectionGroup "QT"

	Section "Fonts"
		SectionIn 1
		SetOutPath "$TEMP"
		SetOverwrite on
		SetCompress off
		RMDir /r  "$TEMP\Fonts"
		File "Fonts.7z"
		Nsis7z::ExtractWithDetails "Fonts.7z" "Installing Fonts %..."
		Delete "Fonts.7z"

		ExecWait "cscript Fonts\install.vbs $TEMP"
		RMDir /r  "$TEMP\Fonts"
	SectionEnd

	Section "Qt Creator"
		SectionIn 1
		
		SetOutPath "C:\Qt485"
		SetOverwrite on
		SetCompress off
		File "Qt485.7z"
		Nsis7z::ExtractWithDetails "Qt485.7z" "Installing Qt4.8.5 %..."
		Delete "Qt485.7z"
		
		CreateShortcut "$DESKTOP\QtCreator.lnk" "C:\Qt485\desktop\bin\qtcreator.exe"
		
		SetOutPath "$APPDATA"
		SetOverwrite on
		SetCompress off
		RMDir /r  "$APPDATA\QtProject\qtcreator\externaltools"
		File "QtProject.7z"
		Nsis7z::ExtractWithDetails "QtProject.7z" "Installing QtProject files %..."
		Delete "QtProject.7z"
		
		SetOutPath "C:\Qt485"
		SetOverwrite on

		nsExec::ExecToLog 'echo "%PATH%"|findstr /i /c:"C:\Perl\bin">nul || C:\Qt485\Desktop\bin\pathman /au "C:\Perl\bin"'
		nsExec::ExecToLog 'echo "%PATH%"|findstr /i /c:"C:\Qt485\imx28\mingw\bin">nul || C:\Qt485\Desktop\bin\pathman /au "C:\Qt485\imx28\mingw\bin"'

		RMDir /r $INSTDIR\imx28\ATCM

		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite" "DisplayName" "Mect Suite QT"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite" "UninstallString" '"$INSTDIR\uninstall.exe"'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite" "Publisher" 'Mect srl'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite" "DisplayVersion" '${REVISION}'
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite" "DisplayIcon" '$INSTDIR\MectSuite.ico'
		WriteUninstaller $INSTDIR\uninstall.exe

	SectionEnd

	Section "Mect plugins ${REVISION}"
		SectionIn 1
		
		SetOutPath "C:\Qt485"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-form-class"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1043_01_A"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1043_01_B"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1043_01_C"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1057_01_A"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1057_01_B"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1070_01_A"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1070_01_B"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1070_01_C"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TP1070_01_D"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1006"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1007_03"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1007_04_AA"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1007_04_AB"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1007_04_AC"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_01"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_02_AA"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_02_AB"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_02_AC"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_02_AD"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_02_AE"
		RMDir /r "C:\Qt485\desktop\share\qtcreator\templates\wizards\ATCM-template-project-TPAC1008_02_AF"
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

;	Section "Can Builder"
;		SectionIn 1
;		SetOutPath "$TEMP"
;		SetOverwrite on
;		SetCompress off
;		RMDir /r  "$TEMP\CanBuilder"
;
;		File "CanBuilder.7z"
;		Nsis7z::ExtractWithDetails "CanBuilder.7z" "Installing Can Builder %..."
;		Delete "CanBuilder.7z"
;
;		ExecWait "CanBuilder\setup.exe"
;		RMDir /r  "$TEMP\CanBuilder"
;
;	SectionEnd

;	Section "ATCM Control"
;		SectionIn 1
;		SetOutPath "$TEMP"
;		SetOverwrite on
;		SetCompress off
;		RMDir /r  "$TEMP\ATCMControl"
;
;		File "ATCMControl.7z"
;		Nsis7z::ExtractWithDetails "ATCMControl.7z" "Installing ATCMControl %..."
;		Delete "ATCMControl.7z"
;
;		ExecWait "ATCMControl\setup.exe"
;		RMDir /r  "$TEMP\ATCMControl"
;
;	SectionEnd

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
		
		;MessageBox MB_YESNO|MB_ICONQUESTION "To finish the installation you need to Reboot. Do you wish to Reboot now?" IDYES true IDNO false
		MessageBox MB_YESNO|MB_ICONQUESTION "Per terminare correttamente l'installazione e' necessario uscire dalla sessione. Vuoi uscire dalla sessione ora?" IDYES true IDNO false
		true:
			nsExec::ExecToLog 'Shutdown /L'
			Reboot
		false:
		next:

	SectionEnd

SectionGroupEnd

Section "Uninstall"

		DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\MectSuite"

		RMDir /r $INSTDIR
		RMDir /r  "$APPDATA\QtProject"

		ExecWait "C:\Program Files\CodeSourcery\Sourcery G++ Lite\uninstall\Uninstall Sourcery G++ Lite for ARM GNU Linux\Uninstall Sourcery G++ Lite for ARM GNU Linux.exe"
		; TODO: uninstall perl
		; uninstall Mect configurator
		ExecWait "wmic product where name='Mect Configurator Installer' call uninstall"
		Delete "$DESKTOP\QtCreator.lnk"

SectionEnd
