!include "MUI.nsh"

SetCompressor /solid LZMA 
SetCompressorDictSize 16
RequestExecutionLevel highest

!searchparse /file main_wnd.cpp '#define VERSTRING "' VER_MAJOR '.' VER_MINOR '"'

!define ARCH_LABEL ""
!define ARCH_STR ""

  ;Name and file
  Name "SnapEase v${VER_MAJOR}.${VER_MINOR}${ARCH_LABEL}"
  OutFile "snapease${VER_MAJOR}${VER_MINOR}${ARCH_STR}-install.exe"

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SnapEase"
  
  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SnapEase" ""

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "SnapEase" SecSnapEase

  SetOutPath "$INSTDIR"
  
  File release\SnapEase.exe
  File generic_upload.php

  WriteRegStr HKCU "Software\SnapEase" "" $INSTDIR
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

Section "Start Menu Shortcuts" SecStart
  SetOutPath $SMPROGRAMS\SnapEase
  CreateShortcut "$OUTDIR\SnapEase.lnk" "$INSTDIR\SnapEase.exe"
  CreateShortcut "$OUTDIR\Uninstall SnapEase.lnk" "$INSTDIR\uninstall.exe"
  
SectionEnd

Section "Desktop Shortcut" SecDesk
  SetOutPath $DESKTOP
  CreateShortcut "$OUTDIR\SnapEase.lnk" "$INSTDIR\SnapEase.exe"
  
SectionEnd

Function .onInstSuccess
  MessageBox MB_OK "SnapEase installed. "
FunctionEnd


;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecSnapEase ${LANG_ENGLISH} "Required SnapEase files."
  LangString DESC_SecStart ${LANG_ENGLISH} "Put links to SnapEase in the start menu."
  LangString DESC_SecDesk ${LANG_ENGLISH} "Put link to SnapEase on the desktop."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSnapEase} $(DESC_SecSnapEase)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesk} $(DESC_SecDesk)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStart} $(DESC_SecStart)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  DeletE "$DESKTOP\SnapEase.lnk"
  Delete "$SMPROGRAMS\SnapEase\SnapEase.lnk"
  Delete "$SMPROGRAMS\SnapEase\Uninstall SnapEase.lnk"
  RMDir "$SMPROGRAMS\SnapEase"

  Delete "$INSTDIR\snapease.exe"
  Delete "$INSTDIR\generic_upload.php"

  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey HKCU "Software\SnapEase"

SectionEnd
