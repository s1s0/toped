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
OutFile "toped_install-086RC.exe"

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
  File "release\toped.exe"
  File "glu32.dll"
  File "toped_example.bat"
  File "news.txt"
  File "readme.txt"

  SetOutPath $INSTDIR\tll
  File "tll\seed.tll"
  File "tll\laylogic.tll"
  File "tll\tcase.tll"
  File "tll\structures.tll"
  File "tll\checklists.tll"

  ;icons
  SetOutPath $INSTDIR\ui
  File "ui\box16x16.png"
  File "ui\copy16x16.png"
  File "ui\cut_with_poly16x16.png"
  File "ui\delete16x16.png"
  File "ui\edit_pop16x16.png"
  File "ui\edit_push16x16.png"
  File "ui\flipx16x16.png"
  File "ui\flipy16x16.png"
  File "ui\move16x16.png"
  File "ui\box16x16.png"
  File "ui\new16x16.png"
  File "ui\open16x16.png"
  File "ui\poly16x16.png"  
  File "ui\redo16x16.png"
  File "ui\rotate_left16x16.png"
  File "ui\rotate_right16x16.png"
  File "ui\ruler16x16.png"
  File "ui\save16x16.png"
  File "ui\text16x16.png"
  File "ui\undo16x16.png"
  File "ui\wire16x16.png"
  File "ui\zoom_all16x16.png"
  File "ui\zoom_in16x16.png"
  File "ui\zoom_out16x16.png"


  ;Read current user directory
  ;ReadRegStr $R0 HKCU "Environment" "HOME"
  ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Personal"
  StrCpy $LocalDir "$R0\toped"
  CreateDirectory "$LocalDir\log"

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
  SetRebootFlag true
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\toped"
  CreateShortCut "$SMPROGRAMS\toped\toped.lnk" "$INSTDIR\toped.exe" "" "$INSTDIR\toped.exe" 0
  CreateShortCut "$SMPROGRAMS\toped\toped_example.lnk" "$INSTDIR\toped_example.bat" "" "$INSTDIR\toped_example.bat" 0
  CreateShortCut "$SMPROGRAMS\toped\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

  
SectionEnd


;MessageBox MB_YESNO|MB_ICONQUESTION "Do you wish to reboot the system right now?" IDNO +2
;  Reboot

Function .onInstSuccess
  MessageBox MB_YESNO|MB_ICONQUESTION "Do you wish to reboot the system right now?" IDNO +2
  Reboot
FunctionEnd

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
  Delete $INSTDIR\toped_example.bat
  Delete $INSTDIR\news.txt
  Delete $INSTDIR\readme.txt

  Delete $INSTDIR\tll\seed.tll
  Delete $INSTDIR\tll\laylogic.tll
  Delete $INSTDIR\tll\tcase.tll
  Delete $INSTDIR\tll\structures.tll
  Delete $INSTDIR\tll\checklists.tll

  Delete $INSTDIR\fonts\arial1.glf
  Delete $INSTDIR\fonts\courier1.glf
  Delete $INSTDIR\fonts\crystal1.glf
  Delete $INSTDIR\fonts\techno0.glf
  Delete $INSTDIR\fonts\techno1.glf
  Delete $INSTDIR\fonts\times_new1.glf

  Delete $INSTDIR\ui\box16x16.png
  Delete $INSTDIR\ui\copy16x16.png
  Delete $INSTDIR\ui\cut_with_poly16x16.png
  Delete $INSTDIR\ui\delete16x16.png
  Delete $INSTDIR\ui\edit_pop16x16.png
  Delete $INSTDIR\ui\edit_push16x16.png
  Delete $INSTDIR\ui\flipx16x16.png
  Delete $INSTDIR\ui\flipy16x16.png
  Delete $INSTDIR\ui\move16x16.png
  Delete $INSTDIR\ui\box16x16.png
  Delete $INSTDIR\ui\new16x16.png
  Delete $INSTDIR\ui\open16x16.png
  Delete $INSTDIR\ui\poly16x16.png  
  Delete $INSTDIR\ui\redo16x16.png
  Delete $INSTDIR\ui\rotate_left16x16.png
  Delete $INSTDIR\ui\rotate_right16x16.png
  Delete $INSTDIR\ui\ruler16x16.png
  Delete $INSTDIR\ui\save16x16.png
  Delete $INSTDIR\ui\text16x16.png
  Delete $INSTDIR\ui\undo16x16.png
  Delete $INSTDIR\ui\wire16x16.png
  Delete $INSTDIR\ui\zoom_all16x16.png
  Delete $INSTDIR\ui\zoom_in16x16.png
  Delete $INSTDIR\ui\zoom_out16x16.png

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
  RMDir $INSTDIR\ui
  RMDir $INSTDIR\tll
  RMDir $INSTDIR\fonts
  Delete $INSTDIR\uninstall.exe


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\toped\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\toped"
  RMDir "$INSTDIR"

SectionEnd
