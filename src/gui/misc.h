
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
 *                         misc.h  -  description
 *                         ----------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#ifndef _MISC_H_
#define _MISC_H_

#include <gtk/gtk.h>


/* DESCRIPTION  :  /
 * BEHAVIOR     :  /
 * PRE          :  /
 */
const gchar *get_default_audio_device_name (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  /
 * PRE          :  /
 */
const gchar *get_default_video_device_name (const gchar * const *options);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Creates a button with the GtkWidget * as pixmap 
 *                 and the label as label.
 * PRE          :  /
 */
GtkWidget *
gnomemeeting_button_new (const char *label,
			 GtkWidget *pixmap);


void gdk_window_set_always_on_top (GdkWindow *window, 
				   gboolean enable);



/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Returns TRUE if the specified window is present and visible
 *                 on the current workspace, FALSE otherwise.
 * PRE          :  Argument is a GtkWindow *.
 */
gboolean gnomemeeting_window_is_visible (GtkWidget *w);
        

/* DESCRIPTION  :  This callback is called when a window of gnomemeeting
 *                 (addressbook, prefs, ...) has to be shown.
 * BEHAVIOR     :  Restore its size (if applicable) and position from the
 *                 config database. The window is given as gpointer.
 *                 The category can be addressbook, main_window, prefs_window,
 *                 or anything under the
 *                 /apps/gnomemeeting/general/user_interface/ key and is given
 *                 by g_object_get_data (G_OBJECT, "window_name"). The window
 *                 object is pointed by the GtkWidget *.
 * PRE          :  /
 */
void gnomemeeting_window_show (GtkWidget *w);


/* DESCRIPTION  :  This callback is called when a window of gnomemeeting
 *                 (addressbook, prefs, ...) has to be hidden.
 * BEHAVIOR     :  Saves its size (if applicable) and position in the config
 *                 database. The window is given as gpointer.
 *                 The category can be addressbook, main_window, prefs_window,
 *                 or anything under the
 *                 /apps/gnomemeeting/general/user_interface/ key and is given
 *                 by g_object_get_data (G_OBJECT, "window_name"). The window
 *                 object is pointed by the GtkWidget *.
 * PRE          :  /
 */
void gnomemeeting_window_hide (GtkWidget *w);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Gets the size of the GmWindow stored in the configuration.
 *                 The window is given as gpointer.
 *                 The category can be addressbook, main_window, prefs_window,
 *                 or anything under the
 *                 /apps/gnomemeeting/general/user_interface/ key and is given
 *                 by g_object_get_data (G_OBJECT, "window_name"). The window
 *                 object is pointed by the GtkWidget *.
 * PRE          :  /
 */
void gnomemeeting_window_get_size (GtkWidget *w, 
				   int &x, 
				   int &y);
#endif
