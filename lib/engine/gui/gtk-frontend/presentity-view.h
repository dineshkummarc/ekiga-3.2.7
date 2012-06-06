
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
 *                        presentity-view.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a widget displaying a Presentity
 *
 */

#ifndef __PRESENTITY_VIEW_H__
#define __PRESENTITY_VIEW_H__

#include <gtk/gtk.h>

#include "presentity.h"

G_BEGIN_DECLS

typedef struct _PresentityView PresentityView;
typedef struct _PresentityViewPrivate PresentityViewPrivate;
typedef struct _PresentityViewClass PresentityViewClass;

struct _PresentityView
{
  GtkHBox parent;

  PresentityViewPrivate* priv;
};

struct _PresentityViewClass
{
  GtkHBoxClass parent;
};

/* public api */

GtkWidget *presentity_view_new (Ekiga::PresentityPtr presentity);

/* GObject thingies */

#define TYPE_PRESENTITY_VIEW (presentity_view_get_type ())
#define PRESENTITY_VIEW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_PRESENTITY_VIEW, PresentityView))
#define PRESENTITY_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_PRESENTITY_VIEW, PresentityViewClass))
#define GTK_IS_PRESENTITY_VIEW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_PRESENTITY_VIEW))
#define GTK_IS_PRESENTITY_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_PRESENTITY_VIEW))
#define PRESENTITY_VIEW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_PRESENTITY_VIEW, PresentityViewClass))

GType presentity_view_get_type () G_GNUC_CONST;


G_END_DECLS
#endif
