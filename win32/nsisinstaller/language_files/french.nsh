;;  French language strings for the Windows Ekiga NSIS installer.
;;  Windows Code page: 1252
;;
;;  Author: Eric Boumaour <zongo_fr@users.sourceforge.net>, 2003-2005.

; Startup Checks
!define INSTALLER_IS_RUNNING			"Le programme d'installation est déjà en cours d'exécution."
!define EKIGA_IS_RUNNING				"Une instance de Ekiga est en cours d'exécution. Veuillez quitter Ekiga et réessayer."
!define GTK_INSTALLER_NEEDED			"Les bibliothèques de l'environnement GTK+ ne sont pas installées ou ont besoin d'une mise à jour.$\rVeuillez installer la version ${GTK_VERSION} ou plus récente des bibliothèques GTK+."

; License Page
!define EKIGA_LICENSE_BUTTON			"Suivant >"
!define EKIGA_LICENSE_BOTTOM_TEXT		"$(^Name) est disponible sous licence GNU General Public License (GPL). Le texte de licence suivant est fourni uniquement à titre informatif. $_CLICK" 

; Components Page
!define EKIGA_SECTION_TITLE			"Ekiga videophone (obligatoire)"
!define GTK_SECTION_TITLE			"Bibliothèques GTK+ (obligatoire)"
!define GTK_THEMES_SECTION_TITLE		"Thèmes GTK+"
!define GTK_NOTHEME_SECTION_TITLE		"Pas de thème"
!define GTK_WIMP_SECTION_TITLE			"Thème Wimp"
!define GTK_BLUECURVE_SECTION_TITLE		"Thème Bluecurve"
!define GTK_LIGHTHOUSEBLUE_SECTION_TITLE	"Thème Light House Blue"
!define EKIGA_SHORTCUTS_SECTION_TITLE		"Raccourcis"
!define EKIGA_DESKTOP_SHORTCUT_SECTION_TITLE	"Bureau"
!define EKIGA_STARTMENU_SHORTCUT_SECTION_TITLE	"Menu Démarrer"
!define EKIGA_SECTION_DESCRIPTION		"Fichiers et DLLs de base de Ekiga"
!define GTK_SECTION_DESCRIPTION			"Un ensemble d'outils pour interfaces graphiques multi-plateforme, utilisé par Ekiga"
!define GTK_THEMES_SECTION_DESCRIPTION		"Les thèmes GTK+ permettent de changer l'aspect des applications GTK+."
!define GTK_NO_THEME_DESC			"Ne pas installer de thème GTK+"
!define GTK_WIMP_THEME_DESC			"GTK-Wimp (imitateur de Windows) est un thème de GTK+ qui se fond dans l'environnement graphique de Windows."
!define GTK_BLUECURVE_THEME_DESC		"Thème Bluecurve"
!define GTK_LIGHTHOUSEBLUE_THEME_DESC		"Thème Lighthouseblue"
!define EKIGA_STARTUP_SECTION_DESCRIPTION	"Exécuter Ekiga quand Windows démarre"
!define EKIGA_SHORTCUTS_SECTION_DESCRIPTION	"Raccourcis pour lancer Ekiga"
!define EKIGA_DESKTOP_SHORTCUT_DESC		"Créer un raccourci pour Ekiga sur le bureau"
!define EKIGA_STARTMENU_SHORTCUT_DESC		"Créer un raccourci pour Ekiga dans le menu Démarrer"
!define EKIGA_RUN_AT_STARTUP			"Lancer Ekiga au démarrage de Windows"

; GTK+ Directory Page
!define GTK_UPGRADE_PROMPT			"Une ancienne version des bibliothèques GTK+ a été trouvée. Voulez-vous la mettre à jour ?$\rNote : Ekiga peut ne pas fonctionner si vous ne le faites pas."

; Installer Finish Page
!define EKIGA_FINISH_VISIT_WEB_SITE		"Visitez la page web de Ekiga Windows"

; Ekiga Section Prompts and Texts
!define EKIGA_UNINSTALL_DESC			"Ekiga (supprimer uniquement)"
!define EKIGA_PROMPT_CONTINUE_WITHOUT_UNINSTALL	"Impossible de supprimer la version courante d'Ekiga. La nouvelle version écrasera la précédente."

; GTK+ Section Prompts
!define GTK_INSTALL_ERROR			"Erreur lors de l'installation des bibliothèques GTK+. Voulez-vous tout de même continuer l'installation d'Ekiga?"
!define GTK_BAD_INSTALL_PATH			"Le dossier d'installation ne peut pas être créé ou n'est pas accessible."

; GTK+ Themes section
!define GTK_NO_THEME_INSTALL_RIGHTS		"Vous n'avez pas les permissions pour installer un thème GTK+."

; Uninstall Section Prompts
!define un.EKIGA_UNINSTALL_ERROR_1		"Le programme de désinstallation n'a pas retrouvé les entrées de Ekiga dans la base de registres.$\rL'application a peut-être été installée par un utilisateur différent."
!define un.EKIGA_UNINSTALL_ERROR_2		"Vous n'avez pas les permissions pour supprimer cette application."
