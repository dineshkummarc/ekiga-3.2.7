
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         gui-gtk.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the user interface
 *
 */

#include <gtk/gtk.h>

#include "gmstockicons.h"

#include "gtk-core.h"

Gtk::Core::Core ()
{
  // set the basic known icons
  GtkIconFactory *factory = gtk_icon_factory_new ();
  GtkIconSet *icon_set = NULL;

  gnomemeeting_stock_icons_init ();

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_ONLINE);
  gtk_icon_factory_add (factory, "online", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_OFFLINE);
  gtk_icon_factory_add (factory, "offline", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_UNKNOWN);
  gtk_icon_factory_add (factory, "unknown", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_AWAY);
  gtk_icon_factory_add (factory, "away", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_DND);
  gtk_icon_factory_add (factory, "dnd", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_INACALL);
  gtk_icon_factory_add (factory, "inacall", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_STATUS_RINGING);
  gtk_icon_factory_add (factory, "ringing", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_ADD);
  gtk_icon_factory_add (factory, "add", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_NEW);
  gtk_icon_factory_add (factory, "new", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_EDIT);
  gtk_icon_factory_add (factory, "edit", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_REMOVE);
  gtk_icon_factory_add (factory, "remove", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_MESSAGE);
  gtk_icon_factory_add (factory, "message", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_PHONE_PICK_UP_16);
  gtk_icon_factory_add (factory, "call", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_FIND);
  gtk_icon_factory_add (factory, "search", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_REFRESH);
  gtk_icon_factory_add (factory, "refresh", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_PROPERTIES);
  gtk_icon_factory_add (factory, "properties", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GTK_STOCK_CLEAR);
  gtk_icon_factory_add (factory, "clear", icon_set);
  gtk_icon_set_unref (icon_set);

  icon_set = gtk_icon_factory_lookup_default (GM_STOCK_CALL_TRANSFER);
  gtk_icon_factory_add (factory, "transfer", icon_set);
  gtk_icon_set_unref (icon_set);

  gtk_icon_factory_add_default (factory);
  g_object_unref (factory);
}
