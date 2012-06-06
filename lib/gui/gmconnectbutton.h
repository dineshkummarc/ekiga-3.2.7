
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
 *                         connectbutton.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a connectbutton widget 
 *
 */



#ifndef __GM_connect_button_H
#define __GM_connect_button_H

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GM_CONNECT_BUTTON_TYPE (gm_connect_button_get_type ())
#define GM_CONNECT_BUTTON(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_CONNECT_BUTTON_TYPE, GmConnectButton))
#define GM_CONNECT_BUTTON_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GM_CONNECT_BUTTON_TYPE, GmConnectButtonClass))
#define GM_IS_CONNECT_BUTTON(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_CONNECT_BUTTON_TYPE))
#define GM_IS_CONNECT_BUTTON_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GM_CONNECT_BUTTON_TYPE))
#define GM_CONNECT_BUTTON_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_CONNECT_BUTTON_TYPE, GmConnectButtonClass))


typedef struct GmConnectButtonPrivate GmConnectButtonPrivate;


typedef struct
{
  GtkHBox parent;
  GtkWidget *pickup_button;
  GtkWidget *hangup_button;
  GtkIconSize stock_size;
  
} GmConnectButton;


typedef struct
{
  GtkHBoxClass parent_class;
  guint clicked_signal;
  
} GmConnectButtonClass;


/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the GType for the GmConnectButton.
 * PRE          :  /
 */
GType gm_connect_button_get_type (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new GmConnectButton.
 * PRE          :  The pick up and hang up stock icons.
 */
GtkWidget *gm_connect_button_new (const char *,
				  const char *,
				  GtkIconSize);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the connected state of the button.
 * PRE          :  First parameter must be != NULL.
 */
void gm_connect_button_set_connected (GmConnectButton *,
				      gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the connected state of the button.
 * PRE          :  First parameter must be != NULL.
 */
gboolean gm_connect_button_get_connected (GmConnectButton *);

G_END_DECLS

#endif /* __GM_connect_button_H */
