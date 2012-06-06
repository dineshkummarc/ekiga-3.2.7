
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
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

/*
 *                         dialog.c  -  description
 *                         ------------------------
 *   begin                : Mon Jun 17 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          to create dialogs for GnomeMeeting.
 */


#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "gmdialog.h"

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

#include "config.h"


/* GUI functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Shows a dialog of the given type. The first argument
 * 		   is the parent window. The rest follows an API similar
 * 		   to the one of printf.
 * PRE          :  /
 */
static GtkWidget *gnomemeeting_dialog (GtkWindow *,
				       const char *,
				       const char *, 
				       va_list,
				       GtkMessageType);


/* Callbacks */

#ifdef WIN32
/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the window given as a pointer in the
 *  		   idle loop.
 * PRE          :  data = GtkWindow to show.
 */
static gboolean thread_safe_window_show (gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the window given as a pointer in the
 *  		   idle loop (using gtk_widget_show_all).
 * PRE          :  data = GtkWindow to show.
 */
static gboolean thread_safe_window_show_all (gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Hides the window given as a pointer in the
 *  		   idle loop.
 * PRE          :  data = GtkWindow to hide.
 */
static gboolean thread_safe_window_hide (gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Destroys the widget given as a pointer in the
 *  		   idle loop.
 * PRE          :  data = GtkWidget to destroy.
 */
static gboolean thread_safe_widget_destroy (gpointer);
#endif


/* DESCRIPTION  :  Callback called when the progress dialog receives
 * 	           a delete_event.
 * BEHAVIOR     :  Prevents the dialog to be destroyed through user 
 *                 interaction.
 * PRE          :  /
 */
static gboolean progress_dialog_delete_event_cb (GtkWidget *widget,
                                                 GdkEvent *event,
                                                 gpointer data);

/* DESCRIPTION  :  Callback called when the progress dialog is 
 * 		   destroyed.
 * BEHAVIOR     :  Stops the timer refreshing the progress bar.
 * PRE          :  data = integer representing the timeout id.
 */
static void progress_dialog_destroyed_cb (GtkWidget *, 
					  gpointer);


/* DESCRIPTION  :  Callback called in a timer.
 * BEHAVIOR     :  Refreshes the progress bar.
 * PRE          :  data = the progress bar.
 */
static gboolean progress_dialog_pulse_cb (gpointer);


/* DESCRIPTION  :  Callback called when the warning dialog receives
 * 		   a response.
 * BEHAVIOR     :  Updates the internal state storing the state of the toggle
 * 		   button.
 * PRE          :  data = the internal state name.
 */
static void warning_dialog_destroyed_cb (GtkWidget *,
					 gint,
					 gpointer);


/* Workaround for windows and threads problems */
#ifdef WIN32
static gboolean
thread_safe_window_show (gpointer data)
{
  g_return_val_if_fail (data != NULL, FALSE);

  gtk_window_present (GTK_WINDOW (data));
  gtk_widget_show (GTK_WIDGET (data));

  return FALSE;
}


static gboolean
thread_safe_window_show_all (gpointer data)
{
  g_return_val_if_fail (data != NULL, FALSE);

  gtk_window_present (GTK_WINDOW (data));
  gtk_widget_show_all (GTK_WIDGET (data));

  return FALSE;
}


static gboolean
thread_safe_window_hide (gpointer data)
{
  g_return_val_if_fail (data != NULL, FALSE);

  gtk_widget_hide (GTK_WIDGET (data));

  return FALSE;
}


static gboolean
thread_safe_widget_destroy (gpointer data)
{
  g_return_val_if_fail (data != NULL, FALSE);

  gtk_widget_destroy (GTK_WIDGET (data));

  return FALSE;
}
#endif


static void
progress_dialog_destroyed_cb (G_GNUC_UNUSED GtkWidget *w, 
			      gpointer data)
{
  g_return_if_fail (data != NULL);
  
  g_source_remove (GPOINTER_TO_INT (data));
}


static gboolean
progress_dialog_delete_event_cb (G_GNUC_UNUSED GtkWidget *widget,
                                 G_GNUC_UNUSED GdkEvent *event,
                                 G_GNUC_UNUSED gpointer data)
{
  return TRUE;
}


static gboolean
progress_dialog_pulse_cb (gpointer data)
{
  g_return_val_if_fail (data != NULL, FALSE);

  gtk_progress_bar_pulse (GTK_PROGRESS_BAR (data));
  
  return TRUE;
}


static void 
warning_dialog_destroyed_cb (GtkWidget *w,
			     G_GNUC_UNUSED gint i,
			     gpointer data)
{
  GList *children = NULL;
  
  children = gtk_container_get_children (GTK_CONTAINER (GTK_DIALOG (w)->vbox));

  g_return_if_fail (data != NULL);

  while (children) {
    
    if (GTK_IS_TOGGLE_BUTTON (children->data)) 
      g_object_set_data (G_OBJECT (gtk_window_get_transient_for (GTK_WINDOW (w))), (const char *) data, GINT_TO_POINTER ((int) (GTK_TOGGLE_BUTTON (children->data)->active)));
  
    children = g_list_next (children);
  }

  gtk_widget_destroy (GTK_WIDGET (w));
}


/* Implementation of public functions */
void
gnomemeeting_threads_dialog_show (GtkWidget *dialog)
{
  g_return_if_fail (dialog != NULL);

#ifndef WIN32
  gtk_window_present (GTK_WINDOW (dialog));
  gtk_widget_show (dialog);
#else
  g_idle_add (thread_safe_window_show, dialog);
#endif
}


void
gnomemeeting_threads_dialog_show_all (GtkWidget *dialog)
{
  g_return_if_fail (dialog != NULL);

#ifndef WIN32
  gtk_window_present (GTK_WINDOW (dialog));
  gtk_widget_show_all (dialog);
#else
  g_idle_add (thread_safe_window_show_all, dialog);
#endif
}

void
gnomemeeting_threads_dialog_hide (GtkWidget *dialog)
{
  g_return_if_fail (dialog != NULL);

#ifndef WIN32
  gtk_widget_hide (dialog);
#else
  g_idle_add (thread_safe_window_hide, dialog);
#endif
}

void
gnomemeeting_threads_widget_destroy (GtkWidget *widget)
{
  g_return_if_fail (widget != NULL);

#ifndef WIN32
  gtk_widget_destroy (widget);
#else
  g_idle_add (thread_safe_widget_destroy, widget);
#endif
}


GtkWidget *
gnomemeeting_error_dialog (GtkWindow *parent,
			   const char *primary_text,
			   const char *format,
			   ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);

  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args,
			 GTK_MESSAGE_ERROR);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_warning_dialog (GtkWindow *parent,
			     const char *primary_text,
			     const char *format,
			     ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);

  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args,
			 GTK_MESSAGE_WARNING);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_message_dialog (GtkWindow *parent,
			     const char *primary_text,
			     const char *format,
			     ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);
  
  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args, GTK_MESSAGE_INFO);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_progress_dialog (GtkWindow *parent,
			      const char *prim_text,
			      const char *format,
			      ...)
{
  GtkWidget *dialog = NULL;
  GtkWidget *label = NULL;
  GtkWidget *progressbar = NULL;
  
  gchar *primary_text = NULL;
  gchar *dialog_text = NULL;
  char buffer [1025];
  guint id = 0;

  va_list args;
  
  va_start (args, format);
  
  primary_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     prim_text);
  if (format == NULL)
    buffer[0] = 0;
  else 
    vsnprintf (buffer, 1024, format, args);

  dialog_text =
    g_strdup_printf ("%s\n\n%s", primary_text, buffer);
  
  dialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog), "");
  if (parent) {
    
    gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
    gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
  }
  
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), dialog_text);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, 
		      FALSE, FALSE, 0);
  
  progressbar = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), progressbar, 
		      FALSE, FALSE, 0);

  id = g_timeout_add (100, progress_dialog_pulse_cb, progressbar);
  g_signal_connect (GTK_OBJECT (dialog), "destroy",
		    G_CALLBACK (progress_dialog_destroyed_cb),
		    GINT_TO_POINTER (id));
  g_signal_connect (GTK_OBJECT (dialog), "delete-event",
		    G_CALLBACK (progress_dialog_delete_event_cb),
		    NULL);
  g_free (dialog_text);
  g_free (primary_text);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_warning_dialog_on_widget (GtkWindow *parent, 
                                       const char *key,
				       const char *primary_text,
                                       const char *format,
				       ...)
{
  va_list args;
  
  GtkWidget *button = NULL;
  GtkWidget *dialog = NULL;

  char buffer[1025];

  gchar *prim_text = NULL;
  gchar *dialog_text = NULL;

  gboolean do_not_show = FALSE;
  
  g_return_val_if_fail (parent != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);

     
  /* if not set, do_not_show will get the value of 0 */
  do_not_show = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (parent), key));
  
  if (do_not_show)
    /* doesn't show warning dialog as state is 'hide' */
    return NULL;

  va_start (args, format);
 
  button = 
    gtk_check_button_new_with_label (_("Do not show this dialog again"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), do_not_show);

  
  if (format == NULL)
    buffer[0] = 0;
  else 
    vsnprintf (buffer, 1024, format, args);

  prim_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     primary_text);
  
  dialog_text =
    g_strdup_printf ("%s\n\n%s", prim_text, buffer);

  dialog = gtk_message_dialog_new (parent, 
                                   0,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK,
				   NULL);
  gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
  
  gtk_window_set_title (GTK_WINDOW (dialog), "");
  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			dialog_text);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 
                     button);
  
  /* Can be called from threads */
  gnomemeeting_threads_dialog_show_all (dialog);

  g_signal_connect_data (GTK_OBJECT (dialog), "response",
			 G_CALLBACK (warning_dialog_destroyed_cb),
			 (gpointer) g_strdup (key),
			 (GClosureNotify) g_free,
			 (GConnectFlags) 0);
  
  va_end (args);

  g_free (prim_text);
  g_free (dialog_text);

  return dialog;
}


/**
 * gnomemeeting_dialog
 *
 * @parent: The parent window of the dialog.
 * @format: a char * including printf formats
 * @args  : va_list that the @format char * uses.
 * @type  : specifies the kind of GtkMessageType dialogs to use. 
 *
 * Creates and runs a dialog and destroys it afterward. 
 **/
static GtkWidget *
gnomemeeting_dialog (GtkWindow *parent,
		     const char *prim_text,
                     const char *format, 
                     va_list args, 
                     GtkMessageType type)
{
  GtkWidget *dialog;
  gchar *primary_text = NULL;
  gchar *dialog_text = NULL;
  char buffer [1025];

  primary_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     prim_text);
  
  if (format == NULL)
    buffer[0] = 0;
  else 
    vsnprintf (buffer, 1024, format, args);

  dialog_text =
    g_strdup_printf ("%s\n\n%s", primary_text, buffer);
  
  dialog =
    gtk_message_dialog_new (parent, 
                            GTK_DIALOG_MODAL, 
                            type,
			    GTK_BUTTONS_OK, NULL);

  gtk_window_set_title (GTK_WINDOW (dialog), "");
  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			dialog_text);
  
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog));
  
  /* Can be called from threads */
  gnomemeeting_threads_dialog_show_all (dialog);

  g_free (dialog_text);
  g_free (primary_text);

  return dialog;
}
