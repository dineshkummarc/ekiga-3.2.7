
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
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez
 *
 */


#include "config.h"

#include "misc.h"
#include "ekiga.h"
#include "callbacks.h"

#include "gmdialog.h"
#include "gmconf.h"

#ifndef WIN32
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#endif

#include <glib/gi18n.h>


/* return the default audio device name */
const gchar *get_default_audio_device_name (void)
{
#ifdef WIN32
  return "Default (PTLIB/WindowsMultimedia)";
#else
  return "Default (PTLIB/ALSA)";
#endif
}

/* return the default video name from the list of existing devices */
const gchar *get_default_video_device_name (const gchar * const *options)
{
#ifdef WIN32
  /* look for the entry containing "PTLIB/DirectShow" or "PTLIB/VideoForWindows" */
  for (int i = 0; options[i]; i++)
    if (g_strrstr (options[i], "PTLIB/DirectShow")
        || g_strrstr (options[i], "PTLIB/VideoForWindows"))
      return options[i];
#else
  /* look for the entry containing "PTLIB/V4L2", otherwise "PTLIB/V4L" */
  for (int i = 0; options[i]; i++)
    if (g_strrstr (options[i], "PTLIB/V4L2"))
      return options[i];
  for (int i = 0; options[i]; i++)
    if (g_strrstr (options[i], "PTLIB/V4L"))
      return options[i];
#endif
  return NULL;  // not found
}


/* The functions */
GtkWidget *
gnomemeeting_button_new (const char *lbl, 
			 GtkWidget *pixmap)
{
  GtkWidget *button = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *label = NULL;

  button = gtk_button_new ();
  label = gtk_label_new_with_mnemonic (lbl);
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, 0);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, 0);

  gtk_container_add (GTK_CONTAINER (button), hbox2);

  return button;
}


/* Stolen from GDK */
#ifndef WIN32
static void
gdk_wmspec_change_state (gboolean add,
			 GdkWindow *window,
			 GdkAtom state1,
			 GdkAtom state2)
{
  GdkDisplay *display = 
    gdk_screen_get_display (gdk_drawable_get_screen (GDK_DRAWABLE (window)));
  XEvent xev;
  
#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */  
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.window = GDK_WINDOW_XID (window);
  xev.xclient.message_type = 
    gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_STATE");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
  xev.xclient.data.l[1] = gdk_x11_atom_to_xatom_for_display (display, state1);
  xev.xclient.data.l[2] = gdk_x11_atom_to_xatom_for_display (display, state2);
  
  XSendEvent (GDK_WINDOW_XDISPLAY (window),
	      GDK_WINDOW_XWINDOW (gdk_screen_get_root_window (gdk_drawable_get_screen (GDK_DRAWABLE (window)))),
	      False, SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}
#endif


void
gdk_window_set_always_on_top (GdkWindow *window, 
			      gboolean enable)
{
#ifndef WIN32
  gdk_wmspec_change_state (enable, window, 
			   gdk_atom_intern ("_NET_WM_STATE_ABOVE", FALSE), 0);
#endif
}


gboolean 
gnomemeeting_window_is_visible (GtkWidget *w)
{
#if GTK_CHECK_VERSION(2,18,0)
  return (gtk_widget_get_visible (GTK_WIDGET (w))
#else
  return (GTK_WIDGET_VISIBLE (w)
#endif
          && !(gdk_window_get_state (GDK_WINDOW (w->window)) & GDK_WINDOW_STATE_ICONIFIED));
}


void
gnomemeeting_window_show (GtkWidget *w)
{
  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  gchar **couple = NULL;
  
  g_return_if_fail (w != NULL);
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  g_return_if_fail (window_name != NULL);
  
  conf_key_position =
    g_strdup_printf ("%s%s/position", USER_INTERFACE_KEY, window_name);
  conf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);  

  if (!gnomemeeting_window_is_visible (w)) {
    
    position = gm_conf_get_string (conf_key_position);
    if (position)
      couple = g_strsplit (position, ",", 0);

    if (couple && couple [0])
      x = atoi (couple [0]);
    if (couple && couple [1])
      y = atoi (couple [1]);


    if (x != 0 && y != 0)
      gtk_window_move (GTK_WINDOW (w), x, y);

    g_strfreev (couple);
    couple = NULL;
    g_free (position);


    if (gtk_window_get_resizable (GTK_WINDOW (w))) {

      size = gm_conf_get_string (conf_key_size);
      if (size)
	couple = g_strsplit (size, ",", 0);

      if (couple && couple [0])
	x = atoi (couple [0]);
      if (couple && couple [1])
	y = atoi (couple [1]);

      if (x > 0 && y > 0)
	gtk_window_resize (GTK_WINDOW (w), x, y);

      g_strfreev (couple);
      g_free (size);
    }

    gnomemeeting_threads_dialog_show (w);
  }
  
  g_free (conf_key_position);
  g_free (conf_key_size);
}


void
gnomemeeting_window_hide (GtkWidget *w)
{
  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  
  g_return_if_fail (w != NULL);
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  g_return_if_fail (window_name != NULL);
 
  conf_key_position =
    g_strdup_printf ("%s%s/position", USER_INTERFACE_KEY, window_name);
  conf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);

  
  /* If the window is visible, save its position and hide the window */
  if (gnomemeeting_window_is_visible (w)) {
    
    gtk_window_get_position (GTK_WINDOW (w), &x, &y);
    position = g_strdup_printf ("%d,%d", x, y);
    gm_conf_set_string (conf_key_position, position);
    g_free (position);

    if (gtk_window_get_resizable (GTK_WINDOW (w))) {

      gtk_window_get_size (GTK_WINDOW (w), &x, &y);
      size = g_strdup_printf ("%d,%d", x, y);
      gm_conf_set_string (conf_key_size, size);
      g_free (size);
    }

	
    gnomemeeting_threads_dialog_hide (w);
  }
    
  
  g_free (conf_key_position);
  g_free (conf_key_size);
}


void
gnomemeeting_window_get_size (GtkWidget *w,
			      int & x,
			      int & y)
{
  gchar *window_name = NULL;
  gchar *conf_key_size = NULL;
  gchar *size = NULL;

  gchar **couple = NULL;
  
  g_return_if_fail (w != NULL);
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  g_return_if_fail (window_name != NULL);
  
  conf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);  

  
  size = gm_conf_get_string (conf_key_size);
  if (size)
    couple = g_strsplit (size, ",", 0);

  if (couple && couple [0])
    x = atoi (couple [0]);
  if (couple && couple [1])
    y = atoi (couple [1]);

  g_strfreev (couple);
  g_free (size);
  g_free (conf_key_size);
}
