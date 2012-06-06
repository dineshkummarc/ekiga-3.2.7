
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
 *                         gmwindow.h -  description
 *                         -------------------------
 *   begin                : 16 August 2007 
 *   copyright            : (c) 2007 by Damien Sandras 
 *   description          : Implementation of a GtkWindow able to restore
 *                          its position and size in a GmConf key.
 *
 */


#ifndef __GM_WINDOW_H__
#define __GM_WINDOW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GmWindow GmWindow;
typedef struct _GmWindowPrivate GmWindowPrivate;
typedef struct _GmWindowClass GmWindowClass;


/* GObject thingies */
struct _GmWindow
{
  GtkWindow parent;
  GmWindowPrivate *priv;
};

struct _GmWindowClass
{
  GtkWindowClass parent;
};

#define GM_WINDOW_TYPE (gm_window_get_type ())

#define GM_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_WINDOW_TYPE, GmWindow))

#define IS_GM_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_WINDOW_TYPE))

#define GM_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GM_WINDOW_TYPE, GmWindowClass))

#define IS_GM_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GM_WINDOW_TYPE))

#define GM_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_WINDOW_TYPE, GmWindowClass))

GType gm_window_get_type ();



/* Public API */

/** Create a new GmWindow.
 * It will be necessary to call gm_window_set_key
 * in order to allow saving and restoring the position
 * and size of the GmWindow.
 * @return A GmWindow 
 */
GtkWidget *gm_window_new ();


/** Create a new GmWindow.
 * @param The key where the position and size of the
 * window will be saved.
 * @return A GmWindow 
 */
GtkWidget *gm_window_new_with_key (const char *key);


/** Set a new key for a given GmWindow.
 * @param window is the GmWindow and key
 * is the key where the position and size of the
 * window will be saved.
 */
void gm_window_set_key (GmWindow *window,
                        const char *key);


/** Return the size of the GmWindow.
 * @param window is the GmWindow
 * @param x is the width
 * @param y is the height
 */
void gm_window_get_size (GmWindow *window,
                         int *x,
                         int *y);

/** Set if the GmWindow should hide on a delete-event or just relay the event
 * @param window is the GmWindow
 * @param hode_on_delete TRUE to just hide, FALSE to relay
 */
void gm_window_set_hide_on_delete (GmWindow *window,
				   gboolean hide_on_delete);

/** Check if the GmWindow will hide or event-relay on a delete-event
 * @param window is the GmWindow
 */
gboolean gm_window_get_hide_on_delete (GmWindow *window);		    

G_END_DECLS

#endif
