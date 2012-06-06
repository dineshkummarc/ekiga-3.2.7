
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
 *                        multiple-chat-page.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a page displaying a MultipleChat
 *
 */

#ifndef __MULTIPLE_CHAT_PAGE_H__
#define __MULTIPLE_CHAT_PAGE_H__

#include <gtk/gtk.h>
#include "chat-multiple.h"

G_BEGIN_DECLS

/* public api */

GtkWidget* multiple_chat_page_new (Ekiga::MultipleChatPtr chat);

/* GObject boilerplate */

typedef struct _MultipleChatPage MultipleChatPage;
typedef struct _MultipleChatPagePrivate MultipleChatPagePrivate;
typedef struct _MultipleChatPageClass MultipleChatPageClass;

struct _MultipleChatPage {
  GtkHBox parent;

  MultipleChatPagePrivate* priv;
};

struct _MultipleChatPageClass {
  GtkHBoxClass parent_class;
};

#define TYPE_MULTIPLE_CHAT_PAGE             (multiple_chat_page_get_type())
#define MULTIPLE_CHAT_PAGE(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj),TYPE_MULTIPLE_CHAT_PAGE,MultipleChatPage))
#define MULTIPLE_CHAT_PAGE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),TYPE_MULTIPLE_CHAT_PAGE,GtkVBox))
#define IS_MULTIPLE_CHAT_PAGE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj),TYPE_MULTIPLE_CHAT_PAGE))
#define IS_MULTIPLE_CHAT_PAGE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),TYPE_MULTIPLE_CHAT_PAGE))
#define MULTIPLE_CHAT_PAGE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),TYPE_MULTIPLE_CHAT_PAGE,MultipleChatPageClass))

GType multiple_chat_page_get_type () G_GNUC_CONST;

G_END_DECLS

#endif /* __MULTIPLE_CHAT_PAGE_H__ */
