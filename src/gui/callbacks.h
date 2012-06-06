
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
 *                         callbacks.h  -  description
 *                         ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include "common.h"


/* DESCRIPTION  :  Simple wrapper that will call gnomemeeting_hide_window.
 * BEHAVIOR     :  Calls gnomemeeting_window_hide.
 * PRE          :  /
 */
gboolean delete_window_cb (GtkWidget *widget,
                           GdkEvent *event,
                           gpointer data);


/* DESCRIPTION  :  Simple wrapper that will call gnomemeeting_show_window.
 * BEHAVIOR     :  Calls gnomemeeting_window_show or present the window if
 * 		   it was already shown.
 * PRE          :  The gpointer is a valid pointer to the GtkWindow that needs
 *                 to be shown with the correct size and position.
 */
void show_window_cb (GtkWidget *widget,
		     gpointer data);

void show_assistant_window_cb (GtkWidget *widget,
                               gpointer   data);

void show_prefs_window_cb (GtkWidget *widget,
			   gpointer   data);

/* DESCRIPTION  :  Simple wrapper that will call gnomemeeting_window_hide.
 * BEHAVIOR     :  Calls gnomemeeting_window_hide.
 * PRE          :  The gpointer is a valid pointer to the GtkWindow that needs
 *                 to be shown with the correct size and position.
 */
void hide_window_cb (GtkWidget *widget,
		     gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to open
 *                 the about window.
 * BEHAVIOR     :  Open the about window.
 * PRE          :  /
 */
void about_callback (GtkWidget *widget,
		     gpointer parent_window);


/* DESCRIPTION  :  This callback is called when the user chooses to open
 *                 the help window.
 * BEHAVIOR     :  Open the help window.
 * PRE          :  /
 */
void help_cb (GtkWidget *widget,
              gpointer data);


/* DESCRIPTION  :  This callback is called when the user chooses to quit.
 * BEHAVIOR     :  Quit.
 * PRE          :  /
 */
void quit_callback (GtkWidget *widget,
		    gpointer data);


/* DESCRIPTION  :  This callback is called when the user toggles a
 *                 connect button.
 * BEHAVIOR     :  Connect if there is a connect URL in the URL bar and if the
 * 		   button is toggled, the button is untoggled if there is no 
 * 		   url, disconnect if the button is untoggled. Do nothing
 * 		   if the endpoint calling state is not Standby.
 * PRE          :  data is a valid pointer to the URL GtkEntry.
 */
void connect_button_clicked_cb (GtkToggleButton *widget,
				gpointer data);


#endif
