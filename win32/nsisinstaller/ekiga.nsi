; NSIS Installer for Ekiga Win32
; Original Authors: Herman Bloggs <hermanator12002@yahoo.com>
; and Daniel Atallah <daniel_atallah@yahoo.com> (GAIM Installler)
; Original version : Installer for Ekiga win32
; Created : 09/01/06

!addPluginDir ${NSISPLUGINDIR}
; ===========================
; Global Variables
var name
var GTK_FOLDER
var STARTUP_RUN_KEY
var ALREADY_INSTALLED
; ===========================
; Configuration

Name $name
SetCompressor /SOLID lzma
!ifdef WITH_GTK
  !if ${DEBUG}
    OutFile "${TARGET_DIR}/ekiga-setup-${EKIGA_VERSION}-debug.exe"
  !else
    OutFile "${TARGET_DIR}/ekiga-setup-${EKIGA_VERSION}.exe"
  !endif
!else
  !if ${DEBUG}
    OutFile "${TARGET_DIR}/ekiga-setup-${EKIGA_VERSION}-nogtk-debug.exe"
  !else
    OutFile "${TARGET_DIR}/ekiga-setup-${EKIGA_VERSION}-nogtk.exe"
  !endif
!endif

; ===========================
; Includes
!include "MUI.nsh"
!include "Sections.nsh"
!include "FileFunc.nsh"
!include "Library.nsh"
!include "WordFunc.nsh"
!include "${NSISSYSTEMDIR}/System.nsh"

!insertmacro GetParameters
!insertmacro GetOptions
!insertmacro GetParent

; ===========================
; Defines

!define EKIGA_REG_KEY			"SOFTWARE\ekiga"
!define EKIGA_UNINST_EXE		"ekiga-uninst.exe"
!define EKIGA_UNINSTALL_KEY		"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Ekiga"
!define HKLM_APP_PATHS_KEY 		"SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\ekiga.exe"
!define EKIGA_REG_LANG		   	"Installer Language"
!define EKIGA_STARTUP_RUN_KEY	"SOFTWARE\Microsoft\Windows\CurrentVersion\Run"
!define GTK_REG_KEY				"SOFTWARE\GTK\2.0"
!define GTK_RUNTIME_INSTALLER	"gtk+-${GTK_VERSION}-setup.exe"
!define GTK_UNINSTALLER_BIN     "unins000.exe"

; ===========================
; Modern UI configuration
!define MUI_ICON                "${EKIGA_DIR}/win32/ico/ekiga.ico"
!define MUI_UNICON              "${EKIGA_DIR}/win32/ico/ekiga-uninstall.ico"

!define MUI_HEADERIMAGE
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_ABORTWARNING

;Finish Page config
!define MUI_FINISHPAGE_RUN			"$INSTDIR\ekiga.exe"
!define MUI_FINISHPAGE_RUN_CHECKED

; ===========================
; Pages

!ifndef WITH_GTK
  !define MUI_PAGE_CUSTOMFUNCTION_PRE preWelcomePage
!endif
!insertmacro MUI_PAGE_WELCOME

; Alter License section
!define MUI_LICENSEPAGE_BUTTON		  $(EKIGA_LICENSE_BUTTON)
!define MUI_LICENSEPAGE_TEXT_BOTTOM	  $(EKIGA_LICENSE_BOTTOM_TEXT)
!insertmacro MUI_PAGE_LICENSE         "${EKIGA_DIR}/LICENSE"

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; ===========================
; Languages

;!define MUI_LANGDLL_ALLLANGUAGES  ; show all languages during install
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Dutch"

!define EKIGA_DEFAULT_LANGFILE "${INSTALLER_DIR}/language_files/english.nsh"

!include "${INSTALLER_DIR}/langmacros.nsh"

!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "ENGLISH"		"${INSTALLER_DIR}/language_files/english.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "HUNGARIAN"	"${INSTALLER_DIR}/language_files/hungarian.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "FRENCH"		"${INSTALLER_DIR}/language_files/french.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "ROMANIAN"	"${INSTALLER_DIR}/language_files/romanian.nsh"
!insertmacro EKIGA_MACRO_INCLUDE_LANGFILE "DUTCH"		"${INSTALLER_DIR}/language_files/dutch.nsh"

; ===========================
; Section SecUninstallOldEkiga
; ===========================
Section -SecUninstallOldEkiga
        ; Check install rights..
        Call CheckUserInstallRights
        Pop $R0

        ;If ekiga is currently set to run on startup,
        ;  save the section of the Registry where the setting is before uninstalling,
        ;  so we can put it back after installing the new version
        ClearErrors
        ReadRegStr $STARTUP_RUN_KEY HKCU "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
        IfErrors +3
        StrCpy $STARTUP_RUN_KEY "HKCU"
        Goto +4
        ClearErrors
        ReadRegStr $STARTUP_RUN_KEY HKLM "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
        IfErrors +2
        StrCpy $STARTUP_RUN_KEY "HKLM"

        StrCmp $R0 "HKLM" ekiga_hklm
        StrCmp $R0 "HKCU" ekiga_hkcu done

        ekiga_hkcu:
                ReadRegStr $R1 HKCU ${EKIGA_REG_KEY} ""
                ReadRegStr $R2 HKCU ${EKIGA_REG_KEY} "Version"
                ReadRegStr $R3 HKCU "${EKIGA_UNINSTALL_KEY}" "UninstallString"
                Goto try_uninstall

        ekiga_hklm:
                ReadRegStr $R1 HKLM ${EKIGA_REG_KEY} ""
                ReadRegStr $R2 HKLM ${EKIGA_REG_KEY} "Version"
                ReadRegStr $R3 HKLM "${EKIGA_UNINSTALL_KEY}" "UninstallString"

        ; If previous version exists .. remove
        try_uninstall:
                StrCmp $R1 "" done
                ; Version key started with 0.60a3. Prior versions can't be
                ; automaticlly uninstalled.
                StrCmp $R2 "" uninstall_problem
                ; Check if we have uninstall string..
                IfFileExists $R3 0 uninstall_problem
                ; Have uninstall string.. go ahead and uninstall.
                ; but before, prevent removal of non-standard
                ;   installation directory of ekiga prior to April 2010
                ; so the lines until nameok1 label could be removed by 2012
                ${GetFileName} $R1 $R5
                StrCmp $R5 ekiga nameok1 0  ; unsensitive comparation
                MessageBox MB_OK "WARNING: Ekiga was installed in $R1, which is not a standard location.  Your old ekiga files will not be removed, please remove manually the directory $R1 after ensuring that you have not added to it useful files for you."
                Goto done
                nameok1:
                SetOverwrite on
                ; Need to copy uninstaller outside of the install dir
                ClearErrors
                CopyFiles /SILENT $R3 "$TEMP\${EKIGA_UNINST_EXE}"
                SetOverwrite off
                IfErrors uninstall_problem
                ; Ready to uninstall..
                ClearErrors
                ExecWait '"$TEMP\${EKIGA_UNINST_EXE}" /S _?=$R1'
                IfErrors exec_error
                Delete "$TEMP\${EKIGA_UNINST_EXE}"
                Goto done

        exec_error:
                Delete "$TEMP\${EKIGA_UNINST_EXE}"
                Goto uninstall_problem

        uninstall_problem:
                ; We can't uninstall.  Either the user must manually uninstall or we ignore and reinstall over it.
                MessageBox MB_OKCANCEL $(EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL) /SD IDOK IDOK done
                Quit

        done:
SectionEnd

; ===========================
; Section SecGtk
; ===========================
!ifdef WITH_GTK
Section $(GTK_SECTION_TITLE) SecGtk
  SectionIn 1 RO

  Call CheckUserInstallRights
  Pop $R1

  SetOutPath $TEMP
  SetOverwrite on
  File ${LIB_DIR}\${GTK_RUNTIME_INSTALLER}
  SetOverwrite off

  ; This keeps track whether we install GTK+ or not..
  StrCpy $R5 "0"

  Call DoWeNeedGtk
  Pop $R0
  Pop $R6

  StrCmp $R0 "0" have_gtk
  StrCmp $R0 "1" upgrade_gtk
  StrCmp $R0 "2" no_gtk no_gtk

  no_gtk:
    StrCmp $R1 "NONE" gtk_no_install_rights
    ClearErrors
    ExecWait "$TEMP\${GTK_RUNTIME_INSTALLER}"
    ; now the GTK path needs to be added to the path of the setup
    ; so that Ekiga could be started from the last page
    ReadEnvStr $R0 "PATH"
    StrCmp $R6 "HKLM" hklm1 hkcu1
    hklm1:
      ReadRegStr $R3 HKLM ${GTK_REG_KEY} "Path"
      Goto hk1
    hkcu1:
      ReadRegStr $R3 HKCU ${GTK_REG_KEY} "Path"
    hk1:
    StrCpy $R0 "$R0;$R3\bin;$R3\lib;$R3"
    System::Call 'Kernel32::SetEnvironmentVariableA(t, t) i("PATH", R0).r0'
    Goto gtk_install_cont

  upgrade_gtk:
    StrCpy $GTK_FOLDER $R6
    MessageBox MB_YESNO $(GTK_UPGRADE_PROMPT) /SD IDYES IDNO done
    ClearErrors
    ExecWait "$TEMP\${GTK_RUNTIME_INSTALLER}"
    Goto gtk_install_cont

  gtk_install_cont:
    IfErrors gtk_install_error
    StrCpy $R5 "1"  ; marker that says we installed...
    Goto done

  gtk_install_error:
    Call DoWeNeedGtk
    Pop $R0
    StrCmp $R0 "0" done exit_on_error

  exit_on_error:
    ;Delete "$TEMP\gtk-runtime.exe"
    MessageBox MB_YESNO $(GTK_INSTALL_ERROR) IDYES docontinue IDNO doexit

  doexit:
    Quit

  docontinue:
    Goto done

  have_gtk:
    StrCpy $GTK_FOLDER $R6
    StrCmp $R1 "NONE" done ; If we have no rights.. can't re-install..
    Goto done

  ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  ; end got_install rights

  gtk_no_install_rights:
    ; Install GTK+ to Ekiga install dir
    StrCpy $GTK_FOLDER $INSTDIR
    ClearErrors
    ExecWait "$TEMP\${GTK_RUNTIME_INSTALLER}"
    IfErrors gtk_install_error
    SetOverwrite on
    ClearErrors
    CopyFiles /FILESONLY "$GTK_FOLDER\bin\*.dll" "$GTK_FOLDER"
    SetOverwrite off
    IfErrors gtk_install_error
    Delete "$GTK_FOLDER\bin\*.dll"
    Goto done
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; end gtk_no_install_rights

  done:
    Delete "$TEMP\${GTK_RUNTIME_INSTALLER}"
SectionEnd ; end of GTK+ section
!endif

; ===========================
; Section SecEkiga
; ===========================
Section $(EKIGA_SECTION_TITLE) SecEkiga
  SectionIn 1 RO

  ; find out a good installation directory, allowing the uninstaller
  ;   to safely remove the whole installation directory
  ; if INSTDIR does not end in [Ee]kiga, then add subdir Ekiga
  ${GetFileName} $INSTDIR $R0
  StrCmp $R0 ekiga nameok 0  ; unsensitive comparation
  StrCpy $INSTDIR "$INSTDIR\Ekiga"

  nameok:
  ; if exists and not empty, then add subdir Ekiga
  IfFileExists $INSTDIR 0 dirok
  ${DirState} $INSTDIR $R0
  IntCmp $R0 0 dirok
  StrCpy $INSTDIR "$INSTDIR\Ekiga"

  ; if exists, abort
  IfFileExists $INSTDIR 0 dirok
  abort "Error: tried $INSTDIR, but it already exists.  Please restart the setup and specify another installation directory"

  dirok:
  ; Check install rights..
  Call CheckUserInstallRights
  Pop $R0

  ; Get GTK+ lib dir if we have it..

  StrCmp $R0 "NONE" ekiga_none
  StrCmp $R0 "HKLM" ekiga_hklm ekiga_hkcu

  ekiga_hklm:
    ReadRegStr $R1 HKLM ${GTK_REG_KEY} "Path"
    WriteRegStr HKLM "${HKLM_APP_PATHS_KEY}" "" "$INSTDIR\ekiga.exe"
    WriteRegStr HKLM "${HKLM_APP_PATHS_KEY}" "Path" "$R1\bin"
    WriteRegStr HKLM ${EKIGA_REG_KEY} "" "$INSTDIR"
    WriteRegStr HKLM ${EKIGA_REG_KEY} "Version" "${EKIGA_VERSION}"
    WriteRegStr HKLM "${EKIGA_UNINSTALL_KEY}" "DisplayName" $(EKIGA_UNINSTALL_DESC)
    WriteRegStr HKLM "${EKIGA_UNINSTALL_KEY}" "UninstallString" "$INSTDIR\${EKIGA_UNINST_EXE}"
    ; Sets scope of the desktop and Start Menu entries for all users.
    SetShellVarContext "all"
    Goto ekiga_install_files

  ekiga_hkcu:
    ReadRegStr $R1 HKCU ${GTK_REG_KEY} "Path"
    StrCmp $R1 "" 0 ekiga_hkcu1
    ReadRegStr $R1 HKLM ${GTK_REG_KEY} "Path"

  ekiga_hkcu1:
    WriteRegStr HKCU ${EKIGA_REG_KEY} "" "$INSTDIR"
    WriteRegStr HKCU ${EKIGA_REG_KEY} "Version" "${EKIGA_VERSION}"
    WriteRegStr HKCU "${EKIGA_UNINSTALL_KEY}" "DisplayName" $(EKIGA_UNINSTALL_DESC)
    WriteRegStr HKCU "${EKIGA_UNINSTALL_KEY}" "UninstallString" "$INSTDIR\${EKIGA_UNINST_EXE}"
    Goto ekiga_install_files

  ekiga_none:
    ReadRegStr $R1 HKLM ${GTK_REG_KEY} "Path"

  ekiga_install_files:
    SetOutPath "$INSTDIR"
    ; Ekiga files
    SetOverwrite on
    File "${TARGET_DIR}\Ekiga\*.exe"
    File "${TARGET_DIR}\Ekiga\*.dll"
    File /r "${TARGET_DIR}\Ekiga\icons"
    File /r "${TARGET_DIR}\Ekiga\ekiga"
    File /r "${TARGET_DIR}\Ekiga\sounds"
    File /r "${TARGET_DIR}\Ekiga\help"
    File /r "${TARGET_DIR}\Ekiga\share\locale"
    File /r "${TARGET_DIR}\Ekiga\plugins"

    IfFileExists "$INSTDIR\ekiga.exe" 0 new_installation
    StrCpy $ALREADY_INSTALLED 1

  new_installation:
    File "${EKIGA_DIR}/win32/ico/ekiga.ico"

    ; If we don't have install rights.. we're done
    StrCmp $R0 "NONE" done
    SetOverwrite off

    ; Write out installer language
    WriteRegStr HKCU "${EKIGA_REG_KEY}" "${EKIGA_REG_LANG}" "$LANGUAGE"

    ; write out uninstaller
    SetOverwrite on
    WriteUninstaller "$INSTDIR\${EKIGA_UNINST_EXE}"
    SetOverwrite off

    ; If we previously had ekiga setup to run on startup, make it do so again
    StrCmp $STARTUP_RUN_KEY "HKCU" +1 +2
    WriteRegStr HKCU "${EKIGA_STARTUP_RUN_KEY}" "Ekiga" "$INSTDIR\ekiga.exe"
    StrCmp $STARTUP_RUN_KEY "HKLM" +1 +2
    WriteRegStr HKLM "${EKIGA_STARTUP_RUN_KEY}" "Ekiga" "$INSTDIR\ekiga.exe"

    SetOutPath "$INSTDIR"

    IfFileExists "$GTK_FOLDER\${GTK_UNINSTALLER_BIN}" 0 done
    ExecWait "$GTK_FOLDER\${GTK_UNINSTALLER_BIN} /gtksetup"
  done:
SectionEnd ; end of default Ekiga section

; ===========================
; Shortcuts
; ===========================
SubSection /e $(EKIGA_SHORTCUTS_SECTION_TITLE) SecShortcuts
  Section $(EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE) SecDesktopShortcut
    SetOutPath "$INSTDIR"
    SetShellVarContext "all"
    SetOverwrite on
    CreateShortCut "$DESKTOP\Ekiga.lnk" "$INSTDIR\ekiga.exe" "" "$INSTDIR\ekiga.ico"
    SetOverwrite off
    SetShellVarContext "current"
  SectionEnd

  Section $(EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE) SecStartMenuShortcut
    SetOutPath "$INSTDIR"
    SetShellVarContext "all"
    SetOverwrite on
    CreateDirectory "$SMPROGRAMS\Ekiga"
    CreateShortCut "$SMPROGRAMS\Ekiga\Ekiga.lnk" "$INSTDIR\ekiga.exe" ""  "$INSTDIR\ekiga.ico"
    CreateShortcut "$SMPROGRAMS\Ekiga\Uninstall Ekiga.lnk" "$INSTDIR\${EKIGA_UNINST_EXE}" "" "" "" "" "" "Uninstall Ekiga"
    SetOverwrite off
    SetShellVarContext "current"
  SectionEnd

  Section $(EKIGA_RUN_AT_STARTUP) SecStartup
     SetOutPath $INSTDIR
     CreateShortCut "$SMSTARTUP\Ekiga.lnk" "$INSTDIR\ekiga.exe" "" "" 0 SW_SHOWNORMAL
  SectionEnd
SubSectionEnd


; ===========================
; Section Uninstall
; ===========================
Section Uninstall
  Call un.CheckUserInstallRights
  Pop $R0
  StrCmp $R0 "NONE" no_rights
  StrCmp $R0 "HKCU" try_hkcu try_hklm

  try_hkcu:
    ReadRegStr $R0 HKCU ${EKIGA_REG_KEY} ""
    StrCmp $R0 $INSTDIR 0 cant_uninstall
    ; HKCU install path matches our INSTDIR.. so uninstall
    DeleteRegKey HKCU ${EKIGA_REG_KEY}
    DeleteRegKey HKCU "${EKIGA_UNINSTALL_KEY}"
    Goto cont_uninstall

  try_hklm:
    ReadRegStr $R0 HKLM ${EKIGA_REG_KEY} ""
    StrCmp $R0 $INSTDIR 0 try_hkcu
    ; HKLM install path matches our INSTDIR.. so uninstall
    DeleteRegKey HKLM ${EKIGA_REG_KEY}
    DeleteRegKey HKLM "${EKIGA_UNINSTALL_KEY}"
    DeleteRegKey HKLM "${HKLM_APP_PATHS_KEY}"
    ; Sets start menu and desktop scope to all users..
    SetShellVarContext "all"

  cont_uninstall:
    ; The WinPrefs plugin may have left this behind..
    DeleteRegValue HKCU "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
    DeleteRegValue HKLM "${EKIGA_STARTUP_RUN_KEY}" "Ekiga"
    ; Remove Language preference info
    DeleteRegKey HKCU ${EKIGA_REG_KEY} ;${MUI_LANGDLL_REGISTRY_ROOT} ${MUI_LANGDLL_REGISTRY_KEY}

    ; this is safe, since Ekiga was installed in an empty directory
    RMDir /r /REBOOTOK "$INSTDIR"

    SetShellVarContext "all"
    Delete /REBOOTOK "$SMPROGRAMS\Ekiga\*.*"
    Delete /REBOOTOK "$SMSTARTUP\Ekiga.lnk"
    RMDir "$SMPROGRAMS\Ekiga"
    Delete "$DESKTOP\Ekiga.lnk"

    SetShellVarContext "current"
    ; Shortcuts..
    RMDir /r "$SMPROGRAMS\Ekiga"
    Delete "$DESKTOP\Ekiga.lnk"

    Goto done

  cant_uninstall:
    MessageBox MB_OK $(un.EKIGA_UNINSTALL_ERROR_1) /SD IDOK
    Quit

  no_rights:
    MessageBox MB_OK $(un.EKIGA_UNINSTALL_ERROR_2) /SD IDOK
    Quit

  done:
SectionEnd ; end of uninstall section

; ===========================
; Function .onInit
; ===========================
Function .onInit
  Push $R0
  SystemLocal::Call 'kernel32::CreateMutexA(i 0, i 0, t "ekiga_installer_running") i .r1 ?e'
  Pop $R0
  StrCmp $R0 0 +3
  MessageBox MB_OK|MB_ICONEXCLAMATION "Another instance of the installer is already running" /SD IDOK
  Abort
  Call RunCheck

  StrCpy $name "Ekiga"

  ${GetParameters} $R0
  ClearErrors
  ; if you wish to start with another language, execute for example:
  ; ekiga-setup.exe /L=1036
  ${GetOptions} $R0 "/L=" $R1
  IfErrors skip_lang

  StrCpy $LANGUAGE $R1
  ;!insertmacro MUI_LANGDLL_DISPLAY  ; display the language selection dialog

  skip_lang:
    ; If install path was set on the command, use it.
    StrCmp $INSTDIR "" 0 instdir_done

    ;  If ekiga is currently intalled, we should default to where it is currently installed
    ClearErrors
    ReadRegStr $INSTDIR HKCU "${EKIGA_REG_KEY}" ""
    IfErrors +2
    StrCmp $INSTDIR "" 0 instdir_done
    ClearErrors
    ReadRegStr $INSTDIR HKLM "${EKIGA_REG_KEY}" ""
    IfErrors +2
    StrCmp $INSTDIR "" 0 instdir_done

    Call CheckUserInstallRights
    Pop $R0

    StrCmp $R0 "HKLM" 0 user_dir
    StrCpy $INSTDIR "$PROGRAMFILES\Ekiga"
    Goto instdir_done

  user_dir:
    Push $SMPROGRAMS
    ${GetParent} $SMPROGRAMS $R2
    ${GetParent} $R2 $R2
    StrCpy $INSTDIR "$R2\Ekiga"

  instdir_done:
    Pop $R0
FunctionEnd

Function un.onInit
  StrCpy $name "Ekiga"
FunctionEnd


; ===========================
; Check if another instance
; of the installer is running
; ===========================
!macro RunCheckMacro UN
Function ${UN}RunCheck
  Push $R0
  Processes::FindProcess "ekiga.exe"
  IntCmp $R0 0 done
  MessageBox MB_YESNO|MB_ICONEXCLAMATION "Ekiga is running. To continue installation I need to shut it down. Shall I proceed?" /SD IDYES IDNO abort_install
  Processes::KillProcess "ekiga.exe"
  Goto done

  abort_install:
    Abort

  done:
    Pop $R0
FunctionEnd
!macroend
!insertmacro RunCheckMacro ""
!insertmacro RunCheckMacro "un."


!macro CheckUserInstallRightsMacro UN
Function ${UN}CheckUserInstallRights
  Push $0
  Push $1
  ClearErrors
  UserInfo::GetName
  IfErrors Win9x
  Pop $0
  UserInfo::GetAccountType
  Pop $1

  StrCmp $1 "Admin" 0 +3
  StrCpy $1 "HKLM"
  Goto done
  StrCmp $1 "Power" 0 +3
  StrCpy $1 "HKLM"
  Goto done
  StrCmp $1 "User" 0 +3
  StrCpy $1 "HKCU"
  Goto done
  StrCmp $1 "Guest" 0 +3
  StrCpy $1 "NONE"
  Goto done
  ; Unknown error
  StrCpy $1 "NONE"
  Goto done

  Win9x:
    StrCpy $1 "HKLM"

  done:
    Exch $1
    Exch
    Pop $0
FunctionEnd
!macroend
!insertmacro CheckUserInstallRightsMacro ""
!insertmacro CheckUserInstallRightsMacro "un."

; ===========================
; Function doWeNeedGtk
; ===========================
; Usage:
; Call DoWeNeedGtk
; First Pop:
;   0 - We have the correct version
;       Second Pop: Key where Version was found
;   1 - We have an old version that needs to be upgraded
;       Second Pop: HKLM or HKCU depending on where GTK was found.
;   2 - We don't have Gtk+ at all
;       Second Pop: "NONE, HKLM or HKCU" depending on our rights..
;
Function DoWeNeedGtk
  ; Logic should be:
  ; - Check what user rights we have (HKLM or HKCU)
  ;   - If HKLM rights..
  ;     - Only check HKLM key for GTK+
  ;       - If installed to HKLM, check it and return.
  ;   - If HKCU rights..
  ;     - First check HKCU key for GTK+
  ;       - if good or bad exists stop and ret.
  ;     - If no hkcu gtk+ install, check HKLM
  ;       - If HKLM ver exists but old, return as if no ver exits.
  ;   - If no rights
  ;     - Check HKLM
  Push $0
  Push $2
  Push $3
  Push $4
  Push $5

  Call CheckUserInstallRights
  Pop $3
  StrCmp $3 "HKLM" check_hklm
  StrCmp $3 "HKCU" check_hkcu check_hklm
  check_hkcu:
    ReadRegStr $0 HKCU ${GTK_REG_KEY} "Version"
    StrCpy $5 "HKCU"
    StrCmp $0 "" check_hklm have_gtk

  check_hklm:
    ReadRegStr $0 HKLM ${GTK_REG_KEY} "Version"
    StrCpy $5 "HKLM"
    StrCmp $0 "" no_gtk have_gtk

  have_gtk:
    ; GTK+ is already installed.. check version.
    ${VersionCompare} ${GTK_VERSION} $0 "$2"
    IntCmp $2 1  bad_version good_version good_version

  bad_version:
    ; Bad version. If hklm ver and we have hkcu or no rights.. return no gtk
    StrCmp $3 "NONE" no_gtk  ; if no rights.. can't upgrade
    StrCmp $3 "HKCU" 0 upgrade_gtk ; if HKLM can upgrade..
    StrCmp $5 "HKLM" no_gtk upgrade_gtk ; have hkcu rights.. if found hklm ver can't upgrade..

  upgrade_gtk:
    StrCpy $2 "1"
    Push $5
    Push $2
    Goto done

  good_version:
    StrCmp $5 "HKLM" have_hklm_gtk have_hkcu_gtk

  have_hkcu_gtk:
    ; Have HKCU version
    ReadRegStr $4 HKCU ${GTK_REG_KEY} "Path"
    Goto good_version_cont

      have_hklm_gtk:
        ReadRegStr $4 HKLM ${GTK_REG_KEY} "Path"
        Goto good_version_cont

    good_version_cont:
      StrCpy $2 "0"
      Push $4  ; The path to existing GTK+
      Push $2
      Goto done

  no_gtk:
    StrCpy $2 "2"
    Push $3 ; our rights
    Push $2
    Goto done

  done:
  ; The top two items on the stack are what we want to return
  Exch 5
  Pop $0
  Exch 5
  Pop $2
  Pop $5
  Pop $4
  Pop $3
FunctionEnd

; ===========================
; Function preWelcomePage
; ===========================
!ifndef WITH_GTK
Function preWelcomePage
  ; If this installer dosn't have GTK, check whether we need it.
  ; We do this here an not in .onInit because language change in
  ; .onInit doesn't take effect until it is finished.
    Push $R0
    Call DoWeNeedGtk
    Pop $R0
    Pop $GTK_FOLDER

    StrCmp $R0 "0" have_gtk need_gtk
    need_gtk:
      MessageBox MB_OK $(GTK_INSTALLER_NEEDED) /SD IDOK
      Quit
    have_gtk:
    Pop $R0
FunctionEnd
!endif

; ===========================
; Descriptions
; ===========================
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecEkiga} $(EKIGA_SECTION_DESCRIPTION)
!ifdef WITH_GTK
  !insertmacro MUI_DESCRIPTION_TEXT ${SecGtk} $(GTK_SECTION_DESCRIPTION)
!endif
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartup} $(EKIGA_STARTUP_SECTION_DESCRIPTION)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} $(EKIGA_SHORTCUTS_SECTION_DESCRIPTION)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktopShortcut} $(EKIGA_DESKTOP_SHORTCUT_DESC)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenuShortcut} $(EKIGA_STARTMENU_SHORTCUT_DESC)
!insertmacro MUI_FUNCTION_DESCRIPTION_END
