; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "CNC Map renderer"
!define VERSION 0.99b
!define APPNAMEANDVERSION "CNC Map renderer ${VERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\CNC Map renderer"
InstallDirRegKey HKLM "Software\${APPNAME}" ""
OutFile "cncmapsrender_setup_${VERSION}.exe"

; Use compression
SetCompressor LZMA

; Modern interface settings
!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_RUN "$INSTDIR\CNC Map Renderer GUI.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

Section "RA2/YR Map renderer" Section1

	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File "build\msvc10\Release Mesa\cncmaprender.exe"
	File "build\msvc10\bin\Release Mesa\CNC Map Renderer GUI.exe"
	CreateShortCut "$DESKTOP\CNC Map renderer.lnk" "$INSTDIR\CNC Map Renderer GUI.exe"
	CreateDirectory "$SMPROGRAMS\CNC Map renderer"
	CreateShortCut "$SMPROGRAMS\CNC Map renderer\CNC Map renderer.lnk" "$INSTDIR\CNC Map Renderer GUI.exe"
	CreateShortCut "$SMPROGRAMS\CNC Map renderer\Uninstall.lnk" "$INSTDIR\uninstall.exe"

SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"

SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${Section1} ""
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall

	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
	DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

	; Delete self
	Delete "$INSTDIR\uninstall.exe"

	; Delete Shortcuts
	Delete "$DESKTOP\CNC Map renderer.lnk"
	Delete "$SMPROGRAMS\CNC Map renderer\RA2/YR Map renderer.lnk"
	Delete "$SMPROGRAMS\CNC Map renderer\Uninstall.lnk"

	; Clean up RA2/YR Map renderer
	Delete "$INSTDIR\cncmaprender.exe"
	Delete "$INSTDIR\CNC Map Renderer GUI.exe"

	; Remove remaining directories
	RMDir "$SMPROGRAMS\CNC Map renderer"
	RMDir "$INSTDIR\"

SectionEnd

BrandingText "by Frank Razenberg"

; eof