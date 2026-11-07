; Installer for the Windows version of QCADesigner-E. build-windows.sh runs this with
; VERSION, APPDIR (the built application folder) and OUTFILE defined.

Unicode true
!include "MUI2.nsh"
!include "x64.nsh"

!define APP_NAME "QCADesignerE"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"

Name "QCADesigner-E ${VERSION}"
OutFile "${OUTFILE}"
InstallDir "$PROGRAMFILES64\${APP_NAME}"
InstallDirRegKey HKLM "${UNINSTALL_KEY}" "InstallLocation"
RequestExecutionLevel admin

!define MUI_ICON "QCADesignerE.ico"
!define MUI_UNICON "QCADesignerE.ico"
!define MUI_FINISHPAGE_RUN "$INSTDIR\bin\QCADesignerE.exe"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${APPDIR}\share\doc\QCADesigner-${VERSION}\COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Function .onInit
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "QCADesigner-E needs 64-bit Windows."
    Abort
  ${EndIf}
  SetRegView 64
  ; Shortcuts for every user, since the installer runs as administrator
  SetShellVarContext all
FunctionEnd

Function un.onInit
  SetRegView 64
  SetShellVarContext all
FunctionEnd

Section "QCADesigner-E" SecApp
  SectionIn RO
  SetOutPath "$INSTDIR"
  File /r "${APPDIR}\*.*"

  CreateDirectory "$SMPROGRAMS\QCADesigner-E"
  CreateShortcut "$SMPROGRAMS\QCADesigner-E\QCADesigner-E.lnk" "$INSTDIR\bin\QCADesignerE.exe"
  CreateShortcut "$SMPROGRAMS\QCADesigner-E\Uninstall QCADesigner-E.lnk" "$INSTDIR\uninstall.exe"

  ; Open .qca files with QCADesigner-E
  WriteRegStr HKLM "Software\Classes\.qca" "" "${APP_NAME}.File"
  WriteRegStr HKLM "Software\Classes\${APP_NAME}.File" "" "QCADesigner-E design"
  WriteRegStr HKLM "Software\Classes\${APP_NAME}.File\DefaultIcon" "" '"$INSTDIR\bin\QCADesignerE.exe",0'
  WriteRegStr HKLM "Software\Classes\${APP_NAME}.File\shell\open\command" "" '"$INSTDIR\bin\QCADesignerE.exe" "%1"'
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'

  WriteUninstaller "$INSTDIR\uninstall.exe"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayName" "QCADesigner-E"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "Publisher" "QCADesigner-E"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "DisplayIcon" '"$INSTDIR\bin\QCADesignerE.exe",0'
  WriteRegStr HKLM "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD HKLM "${UNINSTALL_KEY}" "NoRepair" 1
SectionEnd

Section /o "Desktop shortcut" SecDesktop
  CreateShortcut "$DESKTOP\QCADesigner-E.lnk" "$INSTDIR\bin\QCADesignerE.exe"
SectionEnd

Section "Uninstall"
  Delete "$DESKTOP\QCADesigner-E.lnk"
  RMDir /r "$SMPROGRAMS\QCADesigner-E"

  ReadRegStr $0 HKLM "Software\Classes\.qca" ""
  ${If} $0 == "${APP_NAME}.File"
    DeleteRegKey HKLM "Software\Classes\.qca"
  ${EndIf}
  DeleteRegKey HKLM "Software\Classes\${APP_NAME}.File"
  System::Call 'shell32::SHChangeNotify(i 0x08000000, i 0, p 0, p 0)'

  DeleteRegKey HKLM "${UNINSTALL_KEY}"
  RMDir /r "$INSTDIR"
SectionEnd
