; toped.nsi
;
; This script is for NSIS install system 
;To use this script you must collect all files in one directory 
;(for list of all files see section "toped" commands "File")
; and run MakeNSISW 
; 

;--------------------------------


;Request application privileges for Windows Vista
RequestExecutionLevel user


; The name of the installer
Name "toped"

; The file to write
OutFile "toped_install-0.98.r2133.exe"

; The default installation directory
;InstallDir $PROGRAMFILES\toped
InstallDir "D:\"

Var LocalDir
;--------------------------------

;--------------------------------

; The stuff to install
Section "toped" toped

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath "$TEMP"
  
  ; Put file there
  File "toped_admin.exe"

   ;Read current user directory
  ;ReadRegStr $R0 HKCU "Environment" "HOME"
  ReadRegStr $R0 HKCU "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Personal"
  StrCpy $LocalDir "$R0\toped"

  ExecShell "open" '"$TEMP\toped_admin.exe"' $R0
  ;Exec "$TEMP\toped_admin.exe" 
DetailPrint "exit - $R0"	 
  ;Delete $TEMP\toped_admin.exe
  ;Return installation directory
  SetOutPath $INSTDIR
    

SectionEnd
