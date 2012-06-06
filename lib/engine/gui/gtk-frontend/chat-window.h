
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
 *                         chat-window.h  -  description
 *                         -----------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a window to display chats
 *
 */


#ifndef __CHAT_WINDOW_H__
#define __CHAT_WINDOW_H__

#include "gmwindow.h"
#include "chat-core.h"

G_BEGIN_DECLS

typedef struct _ChatWindow ChatWindow;
typedef struct _ChatWindowPrivate ChatWindowPrivate;
typedef struct _ChatWindowClass ChatWindowClass;

/* GObject thingies */
struct _ChatWindow
{
  GmWindow parent;
  ChatWindowPrivate *priv;
};

struct _ChatWindowClass
{
  GmWindowClass parent;

  /* this is the "unread-count" signal */
  void (*unread_count) (ChatWindow* self,
			guint unread_count);

  /* this is the "unread-alert" signal */
  void (*unread_alert) (ChatWindow* self);
};

#define CHAT_WINDOW_TYPE (chat_window_get_type ())

#define CHAT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), CHAT_WINDOW_TYPE, ChatWindow))

#define IS_CHAT_WINDOW(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), CHAT_WINDOW_TYPE))

#define CHAT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), CHAT_WINDOW_TYPE, ChatWindowClass))

#define IS_CHAT_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), CHAT_WINDOW_TYPE))

#define CHAT_WINDOW_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), CHAT_WINDOW_TYPE, ChatWindowClass))

GType chat_window_get_type ();


/* Public API */

GtkWidget* chat_window_new (Ekiga::ChatCore& core,
			    const std::string key);

G_END_DECLS

#endif
