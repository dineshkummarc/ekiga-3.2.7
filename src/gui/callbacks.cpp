
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         callbacks.cpp  -  description
 *                         -----------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#include "config.h"

#include "callbacks.h"
#include "main.h"
#include "misc.h"

#include "ekiga.h"

#include "gmentrydialog.h"
#include "gmconf.h"
#include "gmdialog.h"
#include "gmconnectbutton.h"
#include "gmmenuaddon.h"
#include "gmstockicons.h"

#ifdef WIN32
#include "platform/winpaths.h"
#include <shellapi.h>
#define WIN32_HELP_DIR "help"
#define WIN32_HELP_FILE "index.html"
#endif

#include <glib/gi18n.h>

#undef GTK_DISABLE_DEPRECATED
#ifdef HAVE_GNOME
#include <libgnome/gnome-help.h>
#endif


/* The callbacks */
gboolean
delete_window_cb (GtkWidget *widget,
                  G_GNUC_UNUSED GdkEvent *event,
                  G_GNUC_UNUSED gpointer data)
{
  gnomemeeting_window_hide (GTK_WIDGET (widget));

  return TRUE;
}

static void
show_window (GtkWidget *window)
{
  if (!gnomemeeting_window_is_visible (window))
    gnomemeeting_window_show (window);
  else
    gtk_window_present (GTK_WINDOW (window));
}

void
show_window_cb (G_GNUC_UNUSED GtkWidget *widget,
		gpointer data)
{
  show_window (GTK_WIDGET (data));
}


void
show_assistant_window_cb (G_GNUC_UNUSED GtkWidget *widget,
                          G_GNUC_UNUSED gpointer data)
{
  GtkWidget *window = GnomeMeeting::Process ()->GetAssistantWindow ();
  show_window (window);
}

void
show_prefs_window_cb (G_GNUC_UNUSED GtkWidget *widget,
                      G_GNUC_UNUSED gpointer data)
{
  GtkWidget *window = GnomeMeeting::Process ()->GetPrefsWindow ();
  show_window (window);
}


void
hide_window_cb (G_GNUC_UNUSED GtkWidget *widget,
		gpointer data)
{
  if (gnomemeeting_window_is_visible (GTK_WIDGET (data)))
    gnomemeeting_window_hide (GTK_WIDGET (data));
}


void
about_callback (G_GNUC_UNUSED GtkWidget *widget, 
		gpointer parent_window)
{
  const gchar *authors [] = {
      "Damien Sandras <dsandras@seconix.com>",
      "",
      N_("Contributors:"),
      "Howard Chu <hyc@symas.com>",
      "Yannick Defais <sevmek@free.fr>",
      "Steve Fr\303\251cinaux <code@istique.net>",
      "Kilian Krause <kk@verfaction.de>", 
      "Vincent Luba <luba@novacom.be>",
      "Julien Puydt <julien.puydt@laposte.net>",
      "Luc Saillard <luc@saillard.org>",
      "Jan Schampera <jan.schampera@web.de>",
      "Matthias Schneider <ma30002000@yahoo.de>",
      "Craig Southeren <craigs@postincrement.com>",
      "",
      N_("Artwork:"),
      "Fabian Deutsch <fabian.deutsch@gmx.de>",
      "Vinicius Depizzol <vdepizzol@gmail.com>",
      "Andreas Kwiatkowski <post@kwiat.org>",
      "Andreas Nilsson <nisses.mail@home.se>",
      "Carlos Pardo <me@m4de.com>",
      "Jakub Steiner <jimmac@ximian.com>",
      "",
      N_("See AUTHORS file for full credits"),
      NULL
  };
	
  authors [2] = gettext (authors [2]);
  authors [12] = gettext (authors [12]);
  authors [20] = gettext (authors [20]);
  
  const gchar *documenters [] = {
    "Damien Sandras <dsandras@seconix.com>",
    "Christopher Warner <zanee@kernelcode.com>",
    "Matthias Redlich <m-redlich@t-online.de>",
    NULL
  };

  const gchar *license[] = {
N_("This program is free software; you can redistribute it and/or modify \
it under the terms of the GNU General Public License as published by \
the Free Software Foundation; either version 2 of the License, or \
(at your option) any later version. "),
N_("This program is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY; without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the \
GNU General Public License for more details. \
You should have received a copy of the GNU General Public License \
along with this program; if not, write to the Free Software Foundation, \
Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA."),
N_("Ekiga is licensed under the GPL license and as a special exception, \
you have permission to link or otherwise combine this program with the \
programs OPAL, OpenH323 and PWLIB, and distribute the combination, \
without applying the requirements of the GNU GPL to the OPAL, OpenH323 \
and PWLIB programs, as long as you do follow the requirements of the \
GNU GPL for all the rest of the software thus combined.")
  };

  gchar *license_trans;

  /* Translators: Please write translator credits here, and
   * separate names with \n */
  const gchar *translator_credits = _("translator-credits");
  if (strcmp (translator_credits, "translator-credits") == 0)
    translator_credits = "No translators, English by\n"
        "Damien Sandras <dsandras@seconix.com>";
 
  const gchar *comments =  _("Ekiga is full-featured SIP and H.323 compatible VoIP, IP-Telephony and Videoconferencing application that allows you to make audio and video calls to remote users with SIP and H.323 hardware or software.");
 
  license_trans = g_strconcat (_(license[0]), "\n\n", _(license[1]), "\n\n",
                               _(license[2]), "\n\n", NULL);

  gtk_show_about_dialog (GTK_WINDOW (parent_window),
		"name", "Ekiga",
		"version", VERSION,
                "copyright", "Copyright © 2000-2009 Damien Sandras",
		"authors", authors,
		"documenters", documenters,
		"translator-credits", translator_credits,
		"comments", comments,
		"logo-icon-name", GM_ICON_LOGO,
		"license", license_trans,
		"wrap-license", TRUE,
		"website", "http://www.ekiga.org",
		NULL);

  g_free (license_trans);
}


void
help_cb (G_GNUC_UNUSED GtkWidget *widget,
	 G_GNUC_UNUSED gpointer data)
{
#ifdef WIN32
  gchar *locale, *loc_ , *index_path;
  int hinst = 0;

  locale = g_win32_getlocale ();
  if (strlen (locale) > 0) {

    /* try returned locale first, it may be fully qualified e.g. zh_CN */
    index_path = g_build_filename (WIN32_HELP_DIR, locale,
				   WIN32_HELP_FILE, NULL);
    hinst = (int) ShellExecute (NULL, "open", index_path, NULL,
			  	DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }

  if (hinst <= 32 && (loc_ = g_strrstr (locale, "_"))) {
    /* on error, try short locale */
    *loc_ = 0;
    index_path = g_build_filename (WIN32_HELP_DIR, locale,
				   WIN32_HELP_FILE, NULL);
    hinst = (int) ShellExecute (NULL, "open", index_path, NULL,
				DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }

  g_free (locale);

  if (hinst <= 32) {

    /* on error or missing locale, try default locale */
    index_path = g_build_filename (WIN32_HELP_DIR, "C", WIN32_HELP_FILE, NULL);
    (void)ShellExecute (NULL, "open", index_path, NULL,
			DATA_DIR, SW_SHOWNORMAL);
    g_free (index_path);
  }
#else /* !WIN32 */
  GError *err = NULL;
  gboolean success = FALSE;

#ifdef HAVE_GNOME
  success = gnome_help_display (PACKAGE_NAME ".xml", NULL, &err);
#elif GTK_CHECK_VERSION(2, 13, 1)
  success = gtk_show_uri (NULL, "ghelp:" PACKAGE_NAME, GDK_CURRENT_TIME, &err);
#else
  success = FALSE;
  err = g_error_new_literal (g_quark_from_static_string ("ekiga"),
                             0,
			     _("Help display is not supported by your GTK+ version"));
#endif

  if (!success) {
    GtkWidget *d;
    d = gtk_message_dialog_new (GTK_WINDOW (GnomeMeeting::Process ()->GetMainWindow ()), 
                                (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, 
                                "%s", _("Unable to open help file."));
    gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (d),
                                              "%s", err->message);
    g_signal_connect (d, "response", G_CALLBACK (gtk_widget_destroy), NULL);
    gtk_window_present (GTK_WINDOW (d));
    g_error_free (err);
  }
#endif
}


void
quit_callback (G_GNUC_UNUSED GtkWidget *widget, 
	       G_GNUC_UNUSED gpointer data)
{
  GtkWidget *main_window = NULL;
  GtkWidget *assistant_window = NULL;
  GtkWidget *prefs_window = NULL;
  GtkWidget *accounts_window = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  assistant_window = GnomeMeeting::Process ()->GetAssistantWindow (false);
  prefs_window = GnomeMeeting::Process ()->GetPrefsWindow (false);
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();
  
  gtk_widget_hide (main_window);
  if (assistant_window)
    gnomemeeting_window_hide (assistant_window);
  if (prefs_window)
    gnomemeeting_window_hide (prefs_window);
  gnomemeeting_window_hide (accounts_window);

  while (gtk_events_pending ())
    gtk_main_iteration ();

  gtk_main_quit ();
}  


