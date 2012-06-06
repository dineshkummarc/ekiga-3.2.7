
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
 *                         toolbar-builder-gtk.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a gtk+ toolbar builder
 *
 */

#include <iostream>

#include "toolbar-builder-gtk.h"

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
on_item_clicked (GtkToolButton *button,
		 gpointer /*data*/)
{
  Action *action = NULL;

  action = (Action *)g_object_get_data (G_OBJECT (button),
					"toolbar-builder-gtk-action");

  if (action != NULL) {

    action->callback ();
  }
}

void
ToolbarBuilderGtk::add_action (const std::string icon,
			       const std::string label,
			       sigc::slot0<void> callback)
{
  Action *action = new Action (callback);
  GtkToolItem *item = NULL;

  item = gtk_tool_button_new_from_stock (icon.c_str ());
  gtk_tool_button_set_label (GTK_TOOL_BUTTON (item), label.c_str ());
  g_object_set_data_full (G_OBJECT (item),
			  "toolbar-builder-gtk-action",
			  (gpointer)action, delete_action_with_item);
  g_signal_connect (item, "clicked",
		    G_CALLBACK (on_item_clicked), NULL);
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
}

void
ToolbarBuilderGtk::add_separator ()
{
  GtkToolItem *item = NULL;

  item = gtk_separator_tool_item_new ();
  gtk_toolbar_insert (GTK_TOOLBAR (toolbar), item, -1);
}
