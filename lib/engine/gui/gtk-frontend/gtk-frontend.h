
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
 *                         gtk-frontend.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to hook a gtk+ user interface to
 *                          the main program
 *
 */

#ifndef __GTK_FRONTEND_H__
#define __GTK_FRONTEND_H__

#include <gtk/gtk.h>
#include <vector>

#include "services.h"
#include "contact-core.h"
#include "presence-core.h"


class GtkFrontend: public Ekiga::Service
{
public:

  GtkFrontend (Ekiga::ServiceCore & _core);

  ~GtkFrontend ();

  const std::string get_name () const;

  const std::string get_description () const;

  const GtkWidget *get_addressbook_window () const;

  const GtkWidget *get_chat_window () const;

private :

  GtkWidget *addressbook_window;
  GtkWidget *chat_window;
};


bool gtk_frontend_init (Ekiga::ServiceCore &core,
			int *argc,
			char **argv[]);

#endif
