
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
 *                         statusicon.h  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : High level tray api interface
 */


#ifndef _STATUSICON_H_
#define _STATUSICON_H_

#include "services.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _StatusIcon StatusIcon;
typedef struct _StatusIconPrivate StatusIconPrivate;
typedef struct _StatusIconClass StatusIconClass;


/* GObject thingies */
struct _StatusIcon
{
  GtkStatusIcon parent;
  StatusIconPrivate *priv;
};

struct _StatusIconClass
{
  GtkStatusIconClass parent;
};

#define STATUSICON_TYPE (statusicon_get_type ())

#define STATUSICON(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), STATUSICON_TYPE, StatusIcon))

#define IS_STATUSICON(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), STATUSICON_TYPE))

#define STATUSICON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), STATUSICON_TYPE, StatusIconClass))

#define IS_STATUSICON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), STATUSICON_TYPE))

#define STATUSICON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), STATUSICON_TYPE, StatusIconClass))

GType statusicon_get_type ();



/* Public API */

/* DESCRIPTION  : /
 * BEHAVIOR     : Returns a new statusicon, with the default icon and menu
 */
StatusIcon *statusicon_new (Ekiga::ServiceCore & core);

G_END_DECLS

#endif
