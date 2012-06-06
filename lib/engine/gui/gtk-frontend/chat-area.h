
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
 *                        chat-area.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a Chat area (both view and control)
 *
 */

#ifndef __CHAT_AREA_H__
#define __CHAT_AREA_H__

#include <gtk/gtk.h>

#include "chat.h"

G_BEGIN_DECLS

typedef struct _ChatArea ChatArea;
typedef struct _ChatAreaClass ChatAreaClass;
typedef struct _ChatAreaPrivate ChatAreaPrivate;

struct _ChatArea
{
  GtkVPaned parent;

  ChatAreaPrivate* priv;
};

struct _ChatAreaClass
{
  GtkVPanedClass parent;

  /* this is the "message-notice-event" signal */
  void (*message_notice_event) (ChatArea* self);
};

/* public api */

GtkWidget *chat_area_new (gmref_ptr<Ekiga::Chat> chat);

const std::string chat_area_get_title (ChatArea* chat);

/* GObject thingies */

#define TYPE_CHAT_AREA (chat_area_get_type ())
#define CHAT_AREA(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_CHAT_AREA, ChatArea))
#define CHAT_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_CHAT_AREA, ChatAreaClass))
#define GTK_IS_CHAT_AREA(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_CHAT_AREA))
#define GTK_IS_CHAT_AREA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_CHAT_AREA))
#define CHAT_AREA_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), TYPE_CHAT_AREA, ChatAreaClass))

GType chat_area_get_type () G_GNUC_CONST;


G_END_DECLS
#endif
