; toped.nsi
;
; This script is for NSIS install system 
;To use this script you must collect all files in one directory 
;(for list of all files see section "toped" commands "File")
; and run MakeNSISW 
; 

;--------------------------------

; The name of the installer
Name "toped"

; The file to write
OutFile "toped_install.exe"

; The default installation directory
InstallDir $PROGRAMFILES\toped

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\toped" "Install_Dir"

Var LocalDir
;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "toped"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "toped.exe"
  File "glu32.dll"
  File "Microsoft.VC80.DebugCRT.manifest"
  File "msvcm80d.dll"
  File "msvcp80d.dll"
  File "msvcr80d.dll"
  File "opengl32.dll"
  File "toped_example.bat"

  ;Read current user directory
  ReadRegStr $R0 HKCU "Environment" "HOME"
  ;ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Personal"
  StrCpy $LocalDir "$R0\toped"
  CreateDirectory "$LocalDir\log"

  ;Change installation directory to $INSTDIR\font
  SetOutPath $INSTDIR\tll
  File "seed.tll"
  File "laylogic.tll"
  File "tcase.tll"

  ;Change installation directory to $INSTDIR\font
  SetOutPath $INSTDIR\fonts
  File "fonts\arial1.glf"
  File "fonts\courier1.glf"
  File "fonts\crystal1.glf"
  File "fonts\techno0.glf"
  File "fonts\techno1.glf"
  File "fonts\times_new1.glf"
  
  ;Return installation directory
  SetOutPath $INSTDIR
    

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\toped "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\toped" "DisplayName" "Toped"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\toped" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\toped" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\toped" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
  ;Write environment variables
  WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" TPD_GLOBAL "$INSTDIR"
  WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" TPD_LOCAL "$LocalDir"
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\toped"
  CreateShortCut "$SMPROGRAMS\toped\toped.lnk" "$INSTDIR\toped.exe" "" "$INSTDIR\toped.exe" 0
  CreateShortCut "$SMPROGRAMS\toped\toped_example.lnk" "$INSTDIR\toped_example.bat" "" "$INSTDIR\toped_example.bat" 0
  CreateShortCut "$SMPROGRAMS\toped\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

  
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\toped"
  DeleteRegKey HKLM SOFTWARE\toped

  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TPD_GLOBAL"
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TPD_LOCAL" 

  ; Remove files and uninstaller
  Delete $INSTDIR\toped.exe
  Delete $INSTDIR\glu32.dll
  Delete $INSTDIR\Microsoft.VC80.DebugCRT.manifest
  Delete $INSTDIR\msvcm80d.dll
  Delete $INSTDIR\msvcp80d.dll
  Delete $INSTDIR\msvcr80d.dll
  Delete $INSTDIR\opengl32.dll
  Delete $INSTDIR\toped_example.bat
  Delete $INSTDIR\tll\seed.tll
  Delete $INSTDIR\tll\laylogic.tll
  Delete $INSTDIR\tll\tcase.tll
  Delete $INSTDIR\fonts\arial1.glf
  Delete $INSTDIR\fonts\courier1.glf
  Delete $INSTDIR\fonts\crystal1.glf
  Delete $INSTDIR\fonts\techno0.glf
  Delete $INSTDIR\fonts\techno1.glf
  Delete $INSTDIR\fonts\times_new1.glf

  ReadRegStr $R0 HKCU "Environment" "HOME"
  ;ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Personal"
  ;ifFileExists $R0\log\tpd_previous.log 0 +2
  ;	Delete $R0\log\tpd_previous.log
  StrCpy $LocalDir "$R0\toped"
  Delete "$LocalDir\log\*.*"
  RMDir "$LocalDir\log"
  RMDir $LocalDir

  ;Sometimes log creates in installation directory
  ;ifFileExists $INSTDIR\tpd_previous.log 0 +2
  ;	Delete $INSTDIR\tpd_previous.log
  Delete $INSTDIR\*.*
  
  RMDir $INSTDIR\log
  RMDir $INSTDIR\tll
  RMDir $INSTDIR\fonts
  Delete $INSTDIR\uninstall.exe


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\toped\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\toped"
  RMDir "$INSTDIR"

SectionEnd
