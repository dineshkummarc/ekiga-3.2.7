
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
 *                         toolbar-builder-gtk.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a gtk+ toolbar builder
 *
 */

#ifndef __TOOLBAR_BUILDER_GTK_H__
#define __TOOLBAR_BUILDER_GTK_H__

#include <gtk/gtk.h>

#include "menu-builder.h"

class ToolbarBuilderGtk: public Ekiga::MenuBuilder
{
public:

  ToolbarBuilderGtk (GtkWidget * _toolbar = gtk_toolbar_new ()): toolbar(_toolbar)
  { }

  ~ToolbarBuilderGtk ()
  {/* notice we leak the toolbar if nobody took it */ }

  void add_action (const std::string icon,
		   const std::string label,
		   sigc::slot0<void> callback);

  void add_separator ();

  GtkWidget *toolbar;
};

#endif
