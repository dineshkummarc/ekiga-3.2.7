
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
 *                         dbus_component.h  -  description
 *                         -----------------------------
 *   begin                : Tue Nov 1  2005
 *   copyright            : (c) 2005 by Julien Puydt
 *                          (c) 2007 by Damien Sandras
 *   description          : This files contains the interface to the DBUS
 *                          interface of gnomemeeting.
 *
 */

#ifndef __DBUS_COMPONENT_H
#define __DBUS_COMPONENT_H

#include <glib-object.h>
#include "framework/services.h"

G_BEGIN_DECLS

#define EKIGA_TYPE_DBUS_COMPONENT               (ekiga_dbus_component_get_type ())
#define EKIGA_DBUS_COMPONENT(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), EKIGA_TYPE_DBUS_COMPONENT, EkigaDBusComponent))
#define EKIGA_DBUS_COMPONENT_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), EKIGA_TYPE_DBUS_COMPONENT, EkigaDBusComponentClass))
#define EKIGA_IS_DBUS_COMPONENT(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EKIGA_TYPE_DBUS_COMPONENT))
#define EKIGA_IS_DBUS_COMPONENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), EKIGA_TYPE_DBUS_COMPONENT))

typedef struct _EkigaDBusComponentPrivate       EkigaDBusComponentPrivate;
typedef struct _EkigaDBusComponent              EkigaDBusComponent;

struct _EkigaDBusComponent {
  GObject                    parent;
  EkigaDBusComponentPrivate *priv;
};

typedef GObjectClass EkigaDBusComponentClass;

GType                ekiga_dbus_component_get_type ();
EkigaDBusComponent  *ekiga_dbus_component_new (Ekiga::ServiceCore *core);

gboolean             ekiga_dbus_claim_ownership ();

void                 ekiga_dbus_client_show ();
void                 ekiga_dbus_client_connect (const gchar *uri);

G_END_DECLS

#endif /* __DBUS_COMPONENT_H */
/* ex:set ts=2 sw=2 et: */
