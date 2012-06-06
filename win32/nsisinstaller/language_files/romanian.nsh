;;  Romanian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Eugen Dedu <Eugen.Dedu@pu-pm.univ-fcomte.fr>, May 2010

; Startup Checks
!define INSTALLER_IS_RUNNING			"Programul de instalare este deja în execuție."
!define EKIGA_IS_RUNNING				"O instanță a Ekiga este deja în execuție.  Ieșiți din Ekiga și încercați din nou."
!define GTK_INSTALLER_NEEDED			"Mediul de execuție GTK+ este fie absent, fie trebuie să fie upgradat.$\rInstalați mediul GTK+ v${GTK_VERSION} sau superior"

; License Page
!define EKIGA_LICENSE_BUTTON			"Următorul >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) este distribuit sub licență GNU General Public License (GPL). Licența este dată aici numai ca informație. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Videofon Ekiga (obligatoriu)"
!define GTK_SECTION_TITLE			"Mediu de execuție GTK+ (obligatoriu)"
!define GTK_THEMES_SECTION_TITLE			"Teme GTK+"
!define GTK_NOTHEME_SECTION_TITLE		"Nicio temă"
!define GTK_WIMP_SECTION_TITLE			"Tema Wimp"
!define GTK_BLUECURVE_SECTION_TITLE		"Tema Bluecurve"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Tema Light House Blue"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Scurtături"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Desktop"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Meniul Start"
!define EKIGA_SECTION_DESCRIPTION			"Fișiere Ekiga de bază și dll-uri"
!define GTK_SECTION_DESCRIPTION			"Un toolkit grafic multi-platformă, utilizat de Ekiga"
!define GTK_THEMES_SECTION_DESCRIPTION		"GTK+ Themes can change the look and feel of GTK+ applications."
!define GTK_NO_THEME_DESC			"Don't install a GTK+ theme"
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (Windows impersonator) is a GTK theme that blends well into the Windows desktop environment."
!define GTK_BLUECURVE_THEME_DESC			"The Bluecurve theme."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"The Lighthouseblue theme."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Executa Ekiga la demararea Windows-ului"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Scurtături pentru lansarea Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Creează o scurtătură Ekiga pe Desktop"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Creează o intrare Ekiga în meniul Start"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"An old version of the GTK+ runtime was found. Do you wish to upgrade?$\rNote: Ekiga may not work unless you do."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Visitați pagina Web a Ekiga Windows"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (remove only)"
!define EKIGA_RUN_AT_STARTUP			"Lansează Ekiga la demararea Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Unable to uninstall the currently installed version of Ekiga. The new version will be installed without removing the currently installed version."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Error installing GTK+ runtime. Would you like to continue anyway?"
!define GTK_BAD_INSTALL_PATH			"The path you entered can not be accessed or created."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"You do not have permission to install a GTK+ theme."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"The uninstaller could not find registry entries for Ekiga.$\rIt is likely that another user installed this application."
!define un.EKIGA_UNINSTALL_ERROR_2		"You do not have permission to uninstall this application."
