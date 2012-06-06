;;  Dutch language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;  Author: Pieter van der Wolk <pwolk@dds.nl>

; Startup Checks
!define INSTALLER_IS_RUNNING			"Het installatieprogramma draait al."
!define EKIGA_IS_RUNNING				"Een versie van Ekiga draait al op dit moment. Stop Ekiga en probeer opnieuw."
!define GTK_INSTALLER_NEEDED			"The GTK+ runtime environment is either missing or needs to be upgraded.$\rPlease install v${GTK_VERSION} or higher of the GTK+ runtime"

; License Page
!define EKIGA_LICENSE_BUTTON			"Next >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) is released under the GNU General Public License (GPL). The license is provided here for information purposes only. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga videophone (required)"
!define GTK_SECTION_TITLE			"GTK+ Runtime Environment (required)"
!define GTK_THEMES_SECTION_TITLE			"GTK+ Themes"
!define GTK_NOTHEME_SECTION_TITLE		"Standaard vormgeving"
!define GTK_WIMP_SECTION_TITLE			"Wimp vormgeving"
!define GTK_BLUECURVE_SECTION_TITLE		"Bluecurve vormgeving"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Light House Blue vormgeving"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Snelkoppelingen"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Bureaublad"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Menu"
!define EKIGA_SECTION_DESCRIPTION			"Ekiga kernbestanden en bibliotheken"
!define GTK_SECTION_DESCRIPTION			"Een multi-platform schermvormgeving gereedschapskist, gebruikt door Ekiga"
!define GTK_THEMES_SECTION_DESCRIPTION		"GTK+ Vormgevingen kunnen uiterlijk en gebruik van GTK+ toepassingen aanpassen."
!define GTK_NO_THEME_DESC			"Install geen GTK+ vormgeving"
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (Windows alshetware) is een GTK vormgeving die goed voegt in de Windows omgeving."
!define GTK_BLUECURVE_THEME_DESC			"De Bluecurve vormgeving."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"The Lighthouseblue vormgeving."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Start Ekiga als Windows opstart"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Snelkoppelingen om Ekiga te starten"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Maak een Snelkoppeling naar Ekiga op het bureaublad"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Maak een Start Menu icoon voor Ekiga"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"Een oude versie van de GTK+ runtime is gevonden. Wilt u deze bijwerken?$\rLet op: Ekiga werkt misschien niet met de oude versie."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Bezoek de Windows Ekiga Webpagina"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (alleen verwijderen)"
!define EKIGA_RUN_AT_STARTUP			"Start Ekiga bij starten Windows"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"De al geinstalleerde versie van Ekiga kan niet worden verwijderd. De nieuwe versie wordt geinstalleerd zonder de oude te verwijderen."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Een fout is opgetreden bij het installeren van de GTK+ runtime. Wilt u toch verdergaan?"
!define GTK_BAD_INSTALL_PATH			"Het pad of bestand dat u heeft opgegeven kan niet worden gelezen/benaderd."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"U heeft geen autorisatie om een andere GTK+ vormgeving te installeren."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Het de-installatieprogramma heeft geen Ekiga gegevens in het register gevonden.$\rWaarschijnlijk installeerde een andere gebruiker Ekiga."
!define un.EKIGA_UNINSTALL_ERROR_2		"U kunt dit programma niet de-installeren."
