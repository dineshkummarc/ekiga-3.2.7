
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
 *                         addressbook-window.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006-2007 by Julien Puydt
 *   description          : declaration of the user interface for contacts-core
 *
 */

#ifndef __ADDRESSBOOK_WINDOW_H__
#define __ADDRESSBOOK_WINDOW_H__

#include <gtk/gtk.h>
#include "contact-core.h"

#include "gmwindow.h"


typedef struct _AddressBookWindow AddressBookWindow;
typedef struct _AddressBookWindowPrivate AddressBookWindowPrivate;
typedef struct _AddressBookWindowClass AddressBookWindowClass;

/* GObject thingies */
struct _AddressBookWindow
{
  GmWindow parent;

  AddressBookWindowPrivate *priv;
};

struct _AddressBookWindowClass
{
  GmWindowClass parent;
};


#define ADDRESSBOOK_WINDOW_TYPE (addressbook_window_get_type ())

#define ADDRESSBOOK_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), ADDRESSBOOK_WINDOW_TYPE, AddressBookWindow))

#define IS_ADDRESSBOOK_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ADDRESSBOOK_WINDOW_TYPE))

#define ADDRESSBOOK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), ADDRESSBOOK_WINDOW_TYPE, AddressBookWindowClass))

#define IS_ADDRESSBOOK_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), ADDRESSBOOK_WINDOW_TYPE))

#define ADDRESSBOOK_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), ADDRESSBOOK_WINDOW_TYPE, AddressBookWindowClass))

GType addressbook_window_get_type ();


/* public api */
GtkWidget *addressbook_window_new (Ekiga::ContactCore & core);

GtkWidget *addressbook_window_new_with_key (Ekiga::ContactCore & _core,
                                            const std::string _key);

#endif
