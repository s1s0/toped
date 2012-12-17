; toped.nsi
;
; This script is for NSIS install system 
;To use this script you must collect all files in one directory 
;(for list of all files see section "toped" commands "File")
; and run MakeNSISW 
; 

;--------------------------------

!include MUI2.nsh

;Request application privileges for Windows Vista


; The name of the installer
Name "toped"

; The file to write
OutFile "toped_admin.exe"

; The default installation directory
InstallDir $PROGRAMFILES\toped

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\toped" "Install_Dir"

;--------------------------------
;Interface Settings

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "ui\toped_install.bmp" 
  !define MUI_ABORTWARNING

Var LocalDir
;--------------------------------

; Pages
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
;UninstPage uninstConfirm
;UninstPage instfiles

;--------------------------------

; The stuff to install
Section "toped" toped

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "release\toped.exe"
  File "winpack\glu32.dll"
  File "winpack\toped_example.bat"
  File "winpack\authors.txt"
  File "winpack\news.txt"
  File "winpack\readme.txt"
  File "winpack\glew32.dll"
  File "winpack\zlib1.dll"
  File "winpack\virtuoso2tll.exe"

  ;all for PLT scheme
  SetOutPath $INSTDIR\lib
  File "winpack\lib\iconv.dll"
  File "winpack\lib\libracket3m_8aa62o.dll"
  
  ;gds2vrml files
  SetOutPath $INSTDIR\utils\gds2vrml
  File "release\gds2vrml_v0.3.exe"
  File "gds2vrml\authors"
  File "gds2vrml\copying"
  File "gds2vrml\history"
  File "gds2vrml\news"
  File "gds2vrml\readme"
  File "gds2vrml\todo"

  ;gds2vrml samples
  SetOutPath $INSTDIR\utils\gds2vrml\test_toped\aref
  File "gds2vrml\test_toped\aref\*"

  SetOutPath $INSTDIR\utils\gds2vrml\test_toped\sref
  File "gds2vrml\test_toped\sref\*.*"

  ;tll files
  SetOutPath $INSTDIR\tll
  File "tll\.topedrc"
  File "tll\arccheck.tll"
  File "tll\callbackTest.tll"
  File "tll\checklists.tll"
  File "tll\cif_reader.tll"
  File "tll\cif_writer.tll"
  File "tll\gds_types.tll"
  File "tll\laylogic.tll"
  File "tll\prepcheck.tll"
  File "tll\prop.tll"
  File "tll\resize.tll"
  File "tll\seed.tll"
  ;File "tll\structures.tll"
  File "tll\tcase.tll"
  File "tll\wtech.tll"


  ;more tll files
  SetOutPath $INSTDIR\tll\user_functions\config
  File "tll\user_functions\config\*.*"
  SetOutPath $INSTDIR\tll\user_functions\devices
  File "tll\user_functions\devices\*.*"
  SetOutPath $INSTDIR\tll\user_functions\general
  File "tll\user_functions\general\*.*"
  SetOutPath $INSTDIR\tll\user_functions\general\examples
  File "tll\user_functions\general\examples\*.*"
  SetOutPath $INSTDIR\tll\user_functions\technology
  File "tll\user_functions\technology\*.*"
  SetOutPath $INSTDIR\tll\user_functions\technology\mosis_SCMOS_p2m6
  File "tll\user_functions\technology\mosis_SCMOS_p2m6\*.*"
  SetOutPath $INSTDIR\tll\user_functions\test
  File "tll\user_functions\test\*.*"
  SetOutPath $INSTDIR\tll\user_functions\tools\logos
  File "tll\user_functions\tools\logos\*.*"
  SetOutPath $INSTDIR\tll\user_functions\tools\logos\examples
  File "tll\user_functions\tools\logos\examples\*.*"

  ;icons
  SetOutPath $INSTDIR\icons
  File "icons\box16x16.png"
  File "icons\copy16x16.png"
  File "icons\cut_with_poly16x16.png"
  File "icons\delete16x16.png"
  File "icons\edit_pop16x16.png"
  File "icons\edit_push16x16.png"
  File "icons\flipx16x16.png"
  File "icons\flipy16x16.png"
  File "icons\move16x16.png"
  File "icons\new16x16.png"
  File "icons\open16x16.png"
  File "icons\poly16x16.png"  
  File "icons\redo16x16.png"
  File "icons\rotate_left16x16.png"
  File "icons\rotate_right16x16.png"
  File "icons\ruler16x16.png"
  File "icons\save16x16.png"
  File "icons\text16x16.png"
  File "icons\undo16x16.png"
  File "icons\wire16x16.png"
  File "icons\zoom_all16x16.png"
  File "icons\zoom_in16x16.png"
  File "icons\zoom_out16x16.png"

  File "icons\box24x24.png"
  File "icons\copy24x24.png"
  File "icons\cut_with_poly24x24.png"
  File "icons\delete24x24.png"
  File "icons\edit_pop24x24.png"
  File "icons\edit_push24x24.png"
  File "icons\flipx24x24.png"
  File "icons\flipy24x24.png"
  File "icons\move24x24.png"
  File "icons\new24x24.png"
  File "icons\open24x24.png"
  File "icons\poly24x24.png"  
  File "icons\redo24x24.png"
  File "icons\rotate_left24x24.png"
  File "icons\rotate_right24x24.png"
  File "icons\ruler24x24.png"
  File "icons\save24x24.png"
  File "icons\text24x24.png"
  File "icons\undo24x24.png"
  File "icons\wire24x24.png"
  File "icons\zoom_all24x24.png"
  File "icons\zoom_in24x24.png"
  File "icons\zoom_out24x24.png"

  File "icons\box32x32.png"
  File "icons\copy32x32.png"
  File "icons\cut_with_poly32x32.png"
  File "icons\delete32x32.png"
  File "icons\edit_pop32x32.png"
  File "icons\edit_push32x32.png"
  File "icons\flipx32x32.png"
  File "icons\flipy32x32.png"
  File "icons\move32x32.png"
  File "icons\new32x32.png"
  File "icons\open32x32.png"
  File "icons\poly32x32.png"  
  File "icons\redo32x32.png"
  File "icons\rotate_left32x32.png"
  File "icons\rotate_right32x32.png"
  File "icons\ruler32x32.png"
  File "icons\save32x32.png"
  File "icons\text32x32.png"
  File "icons\undo32x32.png"
  File "icons\wire32x32.png"
  File "icons\zoom_all32x32.png"
  File "icons\zoom_in32x32.png"
  File "icons\zoom_out32x32.png"

  File "icons\box48x48.png"
  File "icons\copy48x48.png"
  File "icons\cut_with_poly48x48.png"
  File "icons\delete48x48.png"
  File "icons\edit_pop48x48.png"
  File "icons\edit_push48x48.png"
  File "icons\flipx48x48.png"
  File "icons\flipy48x48.png"
  File "icons\move48x48.png"
  File "icons\new48x48.png"
  File "icons\open48x48.png"
  File "icons\poly48x48.png"  
  File "icons\redo48x48.png"
  File "icons\rotate_left48x48.png"
  File "icons\rotate_right48x48.png"
  File "icons\ruler48x48.png"
  File "icons\save48x48.png"
  File "icons\text48x48.png"
  File "icons\undo48x48.png"
  File "icons\wire48x48.png"
  File "icons\zoom_all48x48.png"
  File "icons\zoom_in48x48.png"
  File "icons\zoom_out48x48.png"


  ;Read current user directory from Command Line
  Call GetParameters
  Pop $R3
  StrCpy $LocalDir "$R3\toped"
  CreateDirectory "$LocalDir\log"
  DetailPrint "LocalDir - $LocalDir"

  ;Change installation directory to $INSTDIR\font
  SetOutPath $INSTDIR\fonts
  File "fonts\arial1.glf"
  File "fonts\courier1.glf"
  File "fonts\crystal1.glf"
  File "fonts\techno0.glf"
  File "fonts\techno1.glf"
  File "fonts\times_new1.glf"
  
  ;Change installation directory to $INSTDIR\examples
  SetOutPath $INSTDIR\examples
  File "winpack\foll.tdt"


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
  WriteRegStr HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" TLL_INCLUDE_PATH "$INSTDIR\tll"
  SetRebootFlag true
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" Start_Menu_Shortcuts

  CreateDirectory "$SMPROGRAMS\toped"
  CreateShortCut "$SMPROGRAMS\toped\toped.lnk" "$INSTDIR\toped.exe" "" "$INSTDIR\toped.exe" 0
  CreateShortCut "$SMPROGRAMS\toped\toped_example.lnk" "$INSTDIR\toped_example.bat" "" "$INSTDIR\toped_example.bat" 0
  CreateShortCut "$SMPROGRAMS\toped\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

  
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut" Desktop_Shortcut
  CreateShortCut "$DESKTOP\toped.lnk" "$INSTDIR\toped.exe" "" "$INSTDIR\toped.exe" 0
SectionEnd

;MessageBox MB_YESNO|MB_ICONQUESTION "Do you wish to reboot the system right now?" IDNO +2
;  Reboot	

Function .onInstSuccess
  MessageBox MB_YESNO|MB_ICONQUESTION "Do you wish to reboot the system right now?" IDNO +2
  Reboot
FunctionEnd

; GetParameters
 ; input, none
 ; output, top of stack (replaces, with e.g. whatever)
 ; modifies no other variables.
Function GetParameters
 
  Push $R0
  Push $R1
  Push $R2
  Push $R3
 
  StrCpy $R2 1
  StrLen $R3 $CMDLINE
 
  ;Check for quote or space
  StrCpy $R0 $CMDLINE $R2
  StrCmp $R0 '"' 0 +3
    StrCpy $R1 '"'
    Goto loop
  StrCpy $R1 " "
 
  loop:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 $R1 get
    StrCmp $R2 $R3 get
    Goto loop
 
  get:
    IntOp $R2 $R2 + 1
    StrCpy $R0 $CMDLINE 1 $R2
    StrCmp $R0 " " get
    StrCpy $R0 $CMDLINE "" $R2
 
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
 
FunctionEnd

;--------------------------------
;Descriptions
!insertmacro MUI_LANGUAGE "English"
  ;Language strings
  LangString DESC_Toped ${LANG_ENGLISH} "Main files of Toped Layout Editor"
  LangString DESC_Start_Menu_Shortcuts ${LANG_ENGLISH} "Update the start menu"
  LangString DESC_Desktop_Shortcut ${LANG_ENGLISH} "Add an icon to the desktop"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${toped} $(DESC_Toped)
    !insertmacro MUI_DESCRIPTION_TEXT ${Start_Menu_Shortcuts} $(DESC_Start_Menu_Shortcuts)
    !insertmacro MUI_DESCRIPTION_TEXT ${Desktop_Shortcut} $(DESC_Desktop_Shortcut)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\toped"
  DeleteRegKey HKLM SOFTWARE\toped

  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TPD_GLOBAL"
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TPD_LOCAL" 
  DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Control\Session Manager\Environment" "TLL_INCLUDE_PATH" 

  ; Remove files and uninstaller
  Delete $INSTDIR\toped.exe
  Delete $INSTDIR\glu32.dll
  Delete $INSTDIR\toped_example.bat
  Delete $INSTDIR\news.txt
  Delete $INSTDIR\readme.txt
  Delete $INSTDIR\authors
  Delete $INSTDIR\glew32.dll
  Delete $INSTDIR\zlib1.dll
  Delete $INSTDIR\virtuoso2tll.exe

  Delete $INSTDIR\lib\iconv.dll
  Delete $INSTDIR\lib\libracket3m_8aa62o.dll

  Delete $INSTDIR\tll\.topedrc
  Delete $INSTDIR\tll\arccheck.tll
  Delete $INSTDIR\tll\callbackTest.tll
  Delete $INSTDIR\tll\checklists.tll
  Delete $INSTDIR\tll\cif_reader.tll
  Delete $INSTDIR\tll\cif_writer.tll
  Delete $INSTDIR\tll\gds_types.tll
  Delete $INSTDIR\tll\laylogic.tll
  Delete $INSTDIR\tll\prepcheck.tll
  Delete $INSTDIR\tll\prop.tll
  Delete $INSTDIR\tll\resize.tll
  Delete $INSTDIR\tll\seed.tll
  ;Delete $INSTDIR\tll\structures.tll
  Delete $INSTDIR\tll\tcase.tll
  Delete $INSTDIR\tll\wtech.tll

  ;more tll files
  Delete $INSTDIR\tll\user_functions\config\*.*
  Delete $INSTDIR\tll\user_functions\devices\*.*
  Delete $INSTDIR\tll\user_functions\general\*.*
  Delete $INSTDIR\tll\user_functions\general\examples\*.*
  Delete $INSTDIR\tll\user_functions\technology\*.*
  Delete $INSTDIR\tll\user_functions\technology\mosis_SCMOS_p2m6\*.*
  Delete $INSTDIR\tll\user_functions\test\*.*
  Delete $INSTDIR\tll\user_functions\tools\logos\*.*
  Delete $INSTDIR\tll\user_functions\tools\logos\examples\*.*



  Delete $INSTDIR\fonts\arial1.glf
  Delete $INSTDIR\fonts\courier1.glf
  Delete $INSTDIR\fonts\crystal1.glf
  Delete $INSTDIR\fonts\techno0.glf
  Delete $INSTDIR\fonts\techno1.glf
  Delete $INSTDIR\fonts\times_new1.glf

  Delete $INSTDIR\icons\box16x16.png
  Delete $INSTDIR\icons\copy16x16.png
  Delete $INSTDIR\icons\cut_with_poly16x16.png
  Delete $INSTDIR\icons\delete16x16.png
  Delete $INSTDIR\icons\edit_pop16x16.png
  Delete $INSTDIR\icons\edit_push16x16.png
  Delete $INSTDIR\icons\flipx16x16.png
  Delete $INSTDIR\icons\flipy16x16.png
  Delete $INSTDIR\icons\move16x16.png
  Delete $INSTDIR\icons\new16x16.png
  Delete $INSTDIR\icons\open16x16.png
  Delete $INSTDIR\icons\poly16x16.png  
  Delete $INSTDIR\icons\redo16x16.png
  Delete $INSTDIR\icons\rotate_left16x16.png
  Delete $INSTDIR\icons\rotate_right16x16.png
  Delete $INSTDIR\icons\ruler16x16.png
  Delete $INSTDIR\icons\save16x16.png
  Delete $INSTDIR\icons\text16x16.png
  Delete $INSTDIR\icons\undo16x16.png
  Delete $INSTDIR\icons\wire16x16.png
  Delete $INSTDIR\icons\zoom_all16x16.png
  Delete $INSTDIR\icons\zoom_in16x16.png
  Delete $INSTDIR\icons\zoom_out16x16.png

  Delete $INSTDIR\icons\box24x24.png
  Delete $INSTDIR\icons\copy24x24.png
  Delete $INSTDIR\icons\cut_with_poly24x24.png
  Delete $INSTDIR\icons\delete24x24.png
  Delete $INSTDIR\icons\edit_pop24x24.png
  Delete $INSTDIR\icons\edit_push24x24.png
  Delete $INSTDIR\icons\flipx24x24.png
  Delete $INSTDIR\icons\flipy24x24.png
  Delete $INSTDIR\icons\move24x24.png
  Delete $INSTDIR\icons\new24x24.png
  Delete $INSTDIR\icons\open24x24.png
  Delete $INSTDIR\icons\poly24x24.png  
  Delete $INSTDIR\icons\redo24x24.png
  Delete $INSTDIR\icons\rotate_left24x24.png
  Delete $INSTDIR\icons\rotate_right24x24.png
  Delete $INSTDIR\icons\ruler24x24.png
  Delete $INSTDIR\icons\save24x24.png
  Delete $INSTDIR\icons\text24x24.png
  Delete $INSTDIR\icons\undo24x24.png
  Delete $INSTDIR\icons\wire24x24.png
  Delete $INSTDIR\icons\zoom_all24x24.png
  Delete $INSTDIR\icons\zoom_in24x24.png
  Delete $INSTDIR\icons\zoom_out24x24.png

  Delete $INSTDIR\icons\box32x32.png
  Delete $INSTDIR\icons\copy32x32.png
  Delete $INSTDIR\icons\cut_with_poly32x32.png
  Delete $INSTDIR\icons\delete32x32.png
  Delete $INSTDIR\icons\edit_pop32x32.png
  Delete $INSTDIR\icons\edit_push32x32.png
  Delete $INSTDIR\icons\flipx32x32.png
  Delete $INSTDIR\icons\flipy32x32.png
  Delete $INSTDIR\icons\move32x32.png
  Delete $INSTDIR\icons\new32x32.png
  Delete $INSTDIR\icons\open32x32.png
  Delete $INSTDIR\icons\poly32x32.png  
  Delete $INSTDIR\icons\redo32x32.png
  Delete $INSTDIR\icons\rotate_left32x32.png
  Delete $INSTDIR\icons\rotate_right32x32.png
  Delete $INSTDIR\icons\ruler32x32.png
  Delete $INSTDIR\icons\save32x32.png
  Delete $INSTDIR\icons\text32x32.png
  Delete $INSTDIR\icons\undo32x32.png
  Delete $INSTDIR\icons\wire32x32.png
  Delete $INSTDIR\icons\zoom_all32x32.png
  Delete $INSTDIR\icons\zoom_in32x32.png
  Delete $INSTDIR\icons\zoom_out32x32.png

  Delete $INSTDIR\icons\box48x48.png
  Delete $INSTDIR\icons\copy48x48.png
  Delete $INSTDIR\icons\cut_with_poly48x48.png
  Delete $INSTDIR\icons\delete48x48.png
  Delete $INSTDIR\icons\edit_pop48x48.png
  Delete $INSTDIR\icons\edit_push48x48.png
  Delete $INSTDIR\icons\flipx48x48.png
  Delete $INSTDIR\icons\flipy48x48.png
  Delete $INSTDIR\icons\move48x48.png
  Delete $INSTDIR\icons\new48x48.png
  Delete $INSTDIR\icons\open48x48.png
  Delete $INSTDIR\icons\poly48x48.png  
  Delete $INSTDIR\icons\redo48x48.png
  Delete $INSTDIR\icons\rotate_left48x48.png
  Delete $INSTDIR\icons\rotate_right48x48.png
  Delete $INSTDIR\icons\ruler48x48.png
  Delete $INSTDIR\icons\save48x48.png
  Delete $INSTDIR\icons\text48x48.png
  Delete $INSTDIR\icons\undo48x48.png
  Delete $INSTDIR\icons\wire48x48.png
  Delete $INSTDIR\icons\zoom_all48x48.png
  Delete $INSTDIR\icons\zoom_in48x48.png
  Delete $INSTDIR\icons\zoom_out48x48.png

  ;gds2vrml files
  Delete $INSTDIR\utils\gds2vrml\gds2vrml_v0.3.exe
  Delete $INSTDIR\utils\gds2vrml\authors
  Delete $INSTDIR\utils\gds2vrml\copying
  Delete $INSTDIR\utils\gds2vrml\history
  Delete $INSTDIR\utils\gds2vrml\news
  Delete $INSTDIR\utils\gds2vrml\readme
  Delete $INSTDIR\utils\gds2vrml\todo

  ;gds2vrml samples
  Delete $INSTDIR\utils\gds2vrml\test_toped\aref\*.*
  Delete $INSTDIR\utils\gds2vrml\test_toped\sref\*.*

  Delete $INSTDIR\examples\foll.tdt

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
  RMDir $INSTDIR\icons
  RMDir $INSTDIR\lib

  RMDir $INSTDIR\tll\user_functions\config
  RMDir $INSTDIR\tll\user_functions\devices
  RMDir $INSTDIR\tll\user_functions\general\examples
  RMDir $INSTDIR\tll\user_functions\general
  RMDir $INSTDIR\tll\user_functions\technology\mosis_SCMOS_p2m6
  RMDir $INSTDIR\tll\user_functions\technology
  RMDir $INSTDIR\tll\user_functions\test
  RMDir $INSTDIR\tll\user_functions\tools\logos\examples
  RMDir $INSTDIR\tll\user_functions\tools\logos
  RMDir $INSTDIR\tll\user_functions\tools
  RMDir $INSTDIR\tll\user_functions
  RMDir $INSTDIR\tll

  RMDir $INSTDIR\utils\gds2vrml\test_toped\aref
  RMDir $INSTDIR\utils\gds2vrml\test_toped\sref
  RMDir $INSTDIR\utils\gds2vrml\test_toped
  RMDir $INSTDIR\utils\gds2vrml
  RMDir $INSTDIR\utils
  RMDir $INSTDIR\fonts
  RMDir $INSTDIR\examples

  Delete $INSTDIR\uninstall.exe


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\toped\*.*"
  Delete "$DESKTOP\toped.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\toped"
  RMDir "$INSTDIR"

SectionEnd
