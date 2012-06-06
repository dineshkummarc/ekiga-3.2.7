
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
 *                         statusmenu.h  -  description
 *                         -------------------------------
 *   begin                : Mon Jan 28 2008
 *   copyright            : (C) 2000-2008 by Damien Sandras 
 *   description          : Contains a StatusMenu
 *
 */



#ifndef __STATUS_MENU_H
#define __STATUS_MENU_H

#include <gtk/gtk.h>

#include "call.h"
#include "services.h"

/** This implements a StatusMenu.
 */

G_BEGIN_DECLS

#define STATUS_MENU_TYPE (status_menu_get_type ())
#define STATUS_MENU(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), STATUS_MENU_TYPE, StatusMenu))
#define STATUS_MENU_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), STATUS_MENU_TYPE, StatusMenuClass))
#define IS_STATUS_MENU(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), STATUS_MENU_TYPE))
#define IS_STATUS_MENU_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), STATUS_MENU_TYPE))
#define STATUS_MENU_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), STATUS_MENU_TYPE, StatusMenuClass))

typedef struct _StatusMenu StatusMenu;
typedef struct _StatusMenuPrivate StatusMenuPrivate;
typedef struct _StatusMenuClass StatusMenuClass;

struct _StatusMenu
{
  GtkComboBox parent;

  StatusMenuPrivate *priv;
};


struct _StatusMenuClass
{
  GtkButtonClass parent_class;
};

GType status_menu_get_type (void);


/** Create a new StatusMenu 
 */
GtkWidget *status_menu_new (Ekiga::ServiceCore & core);


/** Set parent window for a StatusMenu 
 *
 * @param self   The StatusMenu.
 * @param parent The parent window.
 */
void status_menu_set_parent_window (StatusMenu *self,
                                    GtkWindow *parent);


G_END_DECLS

#endif /* __STATUS_MENU_H */
