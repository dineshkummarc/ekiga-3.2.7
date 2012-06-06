
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
 *                         menu-builder-gtk.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a gtk+ menu builder
 *
 */

#include <iostream>

#include "menu-builder-gtk.h"

struct Action
{
  Action (sigc::slot0<void> _callback): callback (_callback)
  {}

  sigc::slot0<void> callback;
};

static void
delete_action_with_item (gpointer data)
{
  delete (Action *)data;
}


static void
on_item_activate (GtkMenuItem *item,
		  gpointer /*data*/)
{
  Action *action = NULL;

  action = (Action *)g_object_get_data (G_OBJECT (item),
					"menu-builder-gtk-action");

  if (action != NULL) {

    action->callback ();
  }
}


void
MenuBuilderGtk::add_action (const std::string icon,
			    const std::string label,
			    const sigc::slot0<void> callback)
{
  GtkWidget *item = NULL;
  GtkWidget *image = NULL;

  Action *action = new Action (callback);

  nbr_elements++;
  last_was_separator = false;

  item = gtk_image_menu_item_new_with_mnemonic (label.c_str ());
  image = gtk_image_new_from_stock (icon.c_str (), GTK_ICON_SIZE_MENU);
  if (image)
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

  g_object_set_data_full (G_OBJECT (item),
			  "menu-builder-gtk-action",
			  (gpointer)action, delete_action_with_item);
  g_signal_connect (item, "activate",
		    G_CALLBACK (on_item_activate), NULL);

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
}


void
MenuBuilderGtk::add_separator ()
{
  GtkWidget *item = NULL;

  if (!last_was_separator && !empty ()) {

    last_was_separator = true;

    item = gtk_separator_menu_item_new ();

    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  }
}

void
MenuBuilderGtk::add_ghost (const std::string icon,
			   const std::string label)
{
  GtkWidget *item = NULL;
  GtkWidget *image = NULL;

  nbr_elements++;
  last_was_separator = false;

  item = gtk_image_menu_item_new_with_mnemonic (label.c_str ());
  image = gtk_image_new_from_stock (icon.c_str (), GTK_ICON_SIZE_MENU);
  if (image)
    gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item), image);

  gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);

  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
}

int
MenuBuilderGtk::size () const
{
  return nbr_elements;
}
