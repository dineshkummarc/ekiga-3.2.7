;;  Hungarian language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252

; Startup Checks
!define INSTALLER_IS_RUNNING			"A telepítõprogram már fut."
!define EKIGA_IS_RUNNING				"A Ekiga jelenleg fut. Lépjen ki a Ekiga-bõl és próbálja ismét."
!define GTK_INSTALLER_NEEDED			"A GTK+ futtatási környezet vagy hiányzik, vagy újabb verziójára van szükség $\r Kérjük, telepítse a v${GTK_VERSION} vagy a GTK+ futtatási környezet frissebb változatát"

; License Page
!define EKIGA_LICENSE_BUTTON			"Következõ >"
!define EKIGA_LICENSE_BOTTOM_TEXT			"$(^Name) A GNU General Public License (GPL) neve alatt kerül forgalomba hozatalra. Itt a licenc kizárólag információs célokra szolgál. $_CLICK"

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga videótelefon (szükséges)"
!define GTK_SECTION_TITLE			"GTK+ futtatási környezet (szükséges)"
!define GTK_THEMES_SECTION_TITLE			"GTK+ témák"
!define GTK_NOTHEME_SECTION_TITLE		"Nincs téma"
!define GTK_WIMP_SECTION_TITLE			"Wimp téma"
!define GTK_BLUECURVE_SECTION_TITLE		"Bluecurve téma"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE		"Light House Blue téma"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Parancsikonok"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Munkaasztal"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Start Menü"
!define EKIGA_SECTION_DESCRIPTION			"Alapvetõ Ekiga fájlok és dll fájlok"
!define GTK_SECTION_DESCRIPTION			"A NeophoeX által használt multi-platform GUI eszköztár"
!define GTK_THEMES_SECTION_DESCRIPTION		"A GTK+ témák megváltoztatják a GTK+ alkalmazások megjelenését és érzésvilágát."
!define GTK_NO_THEME_DESC			"Ne telepítsen GTK+ témát"
!define GTK_WIMP_THEME_DESC			"A GTK-Wimp (Windows megszemélyesítõ) olyan  GTK táma, amely jól illeszkedik a Windows munkaasztal környezetébe."
!define GTK_BLUECURVE_THEME_DESC			"The Bluecurve téma."
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"The Lighthouseblue téma."
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Will launch Ekiga when Windows starts"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Parancsikonok a Ekiga indításához"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Parancsikont hoz létre a munkaasztalon, amely segítségével a Ekiga elérhetõ"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Start Menü bejegyzést hoz létre a Ekiga számára"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"A rendszer egy régebbi GTK+ futtatás környezetet talált. Kívánja frissíteni? $\r Megjegyzés: Amennyiben nem  végzi el a frissítést, fennáll annak veszélye, hogy a Ekiga  nem fog mûködni."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Látogassas meg a a Windows Ekiga weboldalt"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (csak eltávolítás)"
!define EKIGA_RUN_AT_STARTUP			"A Ekiga futtatása a Windows indításakor"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"A rendszer nem képes a Ekiga jelenleg telepített verziójának eltávolítására. Az új verzió a jelenleg telepített változat eltávolítása nélkül kerül telepítésre."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Hiba a GTK+ futtatás környezet telepítése közben."
!define GTK_BAD_INSTALL_PATH			"A megadott elérési út nem érhetõ el, vagy nem hozható létre."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Nincs engedélye a GTK+ téma telepítéséhez."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Az eltávolító nem talált Ekiga könyvtári bejegyzéseket.$\rIt az alkalmazást valószínûleg másik felhasználó telepítette."
!define un.EKIGA_UNINSTALL_ERROR_2		"Nem jogosult az alkalmazás eltávolítására."
