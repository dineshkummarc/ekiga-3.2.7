
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
 *                         book-view-gtk.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the widget representing a book
 *
 */

#ifndef __BOOK_VIEW_GTK_H__
#define __BOOK_VIEW_GTK_H__

#include <gtk/gtk.h>
#include "book.h"

typedef struct _BookViewGtk BookViewGtk;
typedef struct _BookViewGtkPrivate BookViewGtkPrivate;
typedef struct _BookViewGtkClass BookViewGtkClass;

/* public api */

GtkWidget *book_view_gtk_new (Ekiga::BookPtr book);

void book_view_gtk_populate_menu (BookViewGtk *, GtkWidget *);

/* GObject thingies */

struct _BookViewGtk
{
  GtkFrame parent;

  BookViewGtkPrivate *priv;
};

struct _BookViewGtkClass
{
  GtkFrameClass parent;
};

#define BOOK_VIEW_GTK_TYPE (book_view_gtk_get_type ())

#define BOOK_VIEW_GTK(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), BOOK_VIEW_GTK_TYPE, BookViewGtk))

#define IS_BOOK_VIEW_GTK(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BOOK_VIEW_GTK_TYPE))

#define BOOK_VIEW_GTK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BOOK_VIEW_GTK_TYPE, BookViewGtkClass))

#define IS_BOOK_VIEW_GTK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BOOK_VIEW_GTK_TYPE))

#define BOOK_VIEW_GTK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), BOOK_VIEW_GTK_TYPE, BookViewGtkClass))

GType book_view_gtk_get_type ();

#endif
