
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
 *                          assistant.h  -  description
 *                          ---------------------------
 *   begin                : Mon May 1 2002
 *   copyright            : (C) 2000-2008 by Damien Sandras
 *                          (C) 2008 by Steve Frécinaux
 *   description          : This file contains all the functions needed to
 *                          build the assistant.
 */


#ifndef __EKIGA_ASSISTANT_H__
#define __EKIGA_ASSISTANT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

enum {
  NET_PSTN,
  NET_ISDN,
  NET_DSL128,
  NET_DSL512,
  NET_LAN,
  NET_CUSTOM
};

#define EKIGA_TYPE_ASSISTANT               (ekiga_assistant_get_type ())
#define EKIGA_ASSISTANT(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), EKIGA_TYPE_ASSISTANT, EkigaAssistant))
#define EKIGA_ASSISTANT_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), EKIGA_TYPE_ASSISTANT, EkigaAssistantClass))
#define EKIGA_IS_ASSISTANT(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EKIGA_TYPE_ASSISTANT))
#define EKIGA_IS_ASSISTANT_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), EKIGA_TYPE_ASSISTANT))

typedef struct _EkigaAssistantPrivate       EkigaAssistantPrivate;
typedef struct _EkigaAssistant              EkigaAssistant;

struct _EkigaAssistant {
  GtkAssistant           parent;
  EkigaAssistantPrivate *priv;
};

typedef GtkAssistantClass EkigaAssistantClass;

GType        ekiga_assistant_get_type   ();
GtkWidget   *ekiga_assistant_new        (Ekiga::ServiceCore *core);

G_END_DECLS

#endif /* __EKIGA_ASSISTANT_H__ */
