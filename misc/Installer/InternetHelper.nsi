
  ; header files
  ;

  !include "MUI2.nsh"
  !include "LogicLib.nsh"

  ; define vendor files
  ;

  !define VENDOR "REAL"
  !define VENDORL "РЕАЛ"

  ; general defines and instructions
  ;

  SetCompressor lzma
  XPStyle on

;  Var Dialog
;  Var LoginLabel
;  Var LoginText

;  Page custom nsDialogsPage

;  Function nsDialogsPage

;    nsDialogs::Create 1018
;    Pop $Dialog

;    ${If} $Dialog == error
;      Abort
;    ${EndIf}

;    ${NSD_CreateLabel} 0 0 100% 12u "Введите Ваш логин:"
;    Pop $LoginLabel

;    ${NSD_CreateText} 0 13u 100% -13u "<login>"
;    Pop $LoginText

;    nsDialogs::Show

;  FunctionEnd

  Name "Интернет-Помощник ${VENDORL}"
  OutFile "InternetHelperSetup.exe"

  InstallDir "$PROGRAMFILES\Internet Helper ${VENDOR}"

  ; get install dir from registry (for auto-update process)
  ;
  
  InstallDirRegKey HKLM "Software\Internet Helper ${VENDOR}" ""

  ; request admin privileges on Vista+
  ;

  RequestExecutionLevel admin

  Var StartMenuFolder

  !define MUI_ABORTWARNING

  ; pages
  ;

  !insertmacro MUI_PAGE_LICENSE "license.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Internet Helper ${VENDOR}" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  ; supported languages
  ;
  
  !insertmacro MUI_LANGUAGE "Russian"

  ; main section (non-deselectable)
  ;

Section "!Файлы Помощника" SecHelperFiles

  ; make sure that Internet Helper isn't already running
  ;

  ${Do}

    System::Call 'kernel32::CreateMutexA(i 0, i 0, t "Global\InternetHelper") i .r1 ?e'
    Pop $R0

    ${If} $R0 == 183
      MessageBox MB_OK "Интернет-Помощник ${VENDORL} уже запущен! Для продолжения установки, закройте его." /SD IDOK
    ${EndIf}

    System::Call 'kernel32::CloseHandle(i r1.) i .r2'

    Sleep 666

  ${LoopUntil} $R0 == 0

  ; install files
  ;

  SetOutPath "$INSTDIR"
  
  File Release\InternetHelper.exe
  
  ; store installation folder for later use of auto-update process
  ;

  WriteRegStr HKLM "Software\Internet Helper ${VENDOR}" "" $INSTDIR

  ; add entry to Add/Remove Control Panel applet
  ;

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "DisplayName" "Интернет-Помощник ${VENDORL}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\""

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "InstallLocation" "$\"$INSTDIR$\""

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "HelpTelephone" "+7 8512 480000"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "DisplayVersion" "1.0"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "VersionMajor" 0x1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "VersionMinor" 0x0

  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "NoModify" 0x1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "NoRepair" 0x1

  !if ${VENDOR} == "REAL"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "Publisher" "ООО НТС $\"РЕАЛ$\""

    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "HelpLink" "http://real.astrakhan.ru/internet-helper/"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "URLUpdateInfo" "http://real.astrakhan.ru/internet-helper/"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "URLInfoAbout" "http://real.astrakhan.ru/internet-helper/"
  !else if ${VENDOR} == "ADTV"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "Publisher" "ЗАО $\"АЦТ$\""

    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "HelpLink" "http://www.telplus.ru/internet-helper/"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "URLUpdateInfo" "http://www.telplus.ru/internet-helper/"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}" "URLInfoAbout" "http://www.telplus.ru/internet-helper/"
  !endif
  
  ; create uninstaller
  ;

  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; create Start Menu entries
  ;
  
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Интернет-Помощник ${VENDORL}.lnk" "$INSTDIR\InternetHelper.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

  ; start Internet Helper after update and optionally delete installer file when running auto-update
  ;

  IfSilent +1 +5
    Exec '"$INSTDIR\InternetHelper.exe"'
    System::Call 'kernel32::GetModuleFileNameA(i 0, t .r0, i 1024) i r1'
    Delete /REBOOTOK $R0
    SetRebootFlag false

  Nop

SectionEnd

  ; desktop shortcut section
  ;

Section /o "Ярлык на Рабочем Столе" SecDesktopIcon

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    CreateShortCut "$DESKTOP\Интернет-Помощник ${VENDORL}.lnk" "$INSTDIR\InternetHelper.exe"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

  ; Windows startup section
  ;

Section /o "Запускать при старте Windows" SecStartup

  WriteRegDWORD HKCU "Software\REAL\Internet Helper\Startup" "AutoStart" 0x1

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    CreateShortCut "$SMSTARTUP\Интернет-Помощник ${VENDORL}.lnk" "$INSTDIR\InternetHelper.exe" "--minimize"
  
  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd


  ; descriptions
  ;

  LangString DESC_SecHelperFiles ${LANG_RUSSIAN} "Файлы Интернет-Помощника."
  LangString DESC_SecDesktopIcon ${LANG_RUSSIAN} "Создаёт ярлык на Рабочем Столе."
  LangString DESC_SecStartup ${LANG_RUSSIAN} "Включает автозапуск при старте Windows."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecHelperFiles} $(DESC_SecHelperFiles)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopIcon} $(DESC_SecDesktopIcon)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartup} $(DESC_SecStartup)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
  ; uninstaller
  ;

Section "Uninstall"

  ; delete files and directories
  ;

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\InternetHelper.exe"

  RMDir "$INSTDIR"

  ; delete Start Menu shortcuts
  ;
  
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
  Delete "$SMPROGRAMS\$StartMenuFolder\Интернет-Помощник ${VENDORL}.lnk"
  Delete "$SMSTARTUP\Интернет-Помощник ${VENDORL}.lnk"
  Delete "$DESKTOP\Интернет-Помощник ${VENDORL}.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  ; delete registry information
  ;

  DeleteRegKey HKCU "Software\REAL\Internet Helper"
  DeleteRegKey /IFEMPTY HKCU "Software\REAL"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InternetHelper${VENDOR}"
  
  DeleteRegKey HKLM "Software\Internet Helper ${VENDOR}"

SectionEnd

  ; set main section mandatory
  ;

Function .onInit

  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  SectionSetFlags ${SecHelperFiles} $0

FunctionEnd

