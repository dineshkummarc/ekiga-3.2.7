
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
 *                         gmstatusbar.h  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a statusbar widget 
 *
 */



#ifndef __GM_STATUSBAR_H
#define __GM_STATUSBAR_H

#include <glib-object.h>
#include <gtk/gtk.h>


G_BEGIN_DECLS

#define GM_STATUSBAR_TYPE (gm_statusbar_get_type ())
#define GM_STATUSBAR(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_STATUSBAR_TYPE, GmStatusbar))
#define GM_STATUSBAR_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), GM_STATUSBAR_TYPE, GmStatusbarClass))
#define GM_IS_STATUSBAR(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_STATUSBAR_TYPE))
#define GM_IS_STATUSBAR_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), GM_STATUSBAR_TYPE))
#define GM_STATUSBAR_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_STATUSBAR_TYPE, GmStatusbarClass))


typedef struct GmStatusbarPrivate GmStatusbarPrivate;


typedef struct
{
  GtkStatusbar parent;
  
} GmStatusbar;


typedef struct
{
  GtkStatusbarClass parent_class;
  
} GmStatusbarClass;


/* The functions */

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the GType for the GmStatusbar.
 * PRE          :  /
 */
GType gm_statusbar_get_type (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new GmStatusbar.
 * PRE          :  /
 */
GtkWidget *gm_statusbar_new ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Flash a message with the printf syntax in the status bar.
 * 	           This message will stay displayed for 4 seconds.
 * PRE          :  /
 */
void gm_statusbar_flash_message (GmStatusbar *, 
				 const char *msg, 
				 ...) G_GNUC_PRINTF(2,3);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Display a message with the printf syntax in the status bar.
 * 		   That message will stay displayed until the GmStatusbar is 
 * 		   explicitely cleared or another message is displayed.
 * PRE          :  /
 */
void gm_statusbar_push_message (GmStatusbar *, 
				const char *msg, 
				...) G_GNUC_PRINTF(2,3);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Display an info message with the printf syntax 
 * 		   in the status bar. An info message will stay displayed 
 * 		   forever until the GmStatusbar is explicitely cleared.
 * PRE          :  /
 */
void gm_statusbar_push_info_message (GmStatusbar *, 
				     const char *msg, 
				     ...) G_GNUC_PRINTF(2,3);

G_END_DECLS

#endif /* __GM_statusbar_H */
