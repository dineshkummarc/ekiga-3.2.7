
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
 *                        gm-text-extlink.h  -  description
 *                         --------------------------------
 *   begin                : written in august 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of a text decorator for external links
 *
 */

#ifndef __GM_TEXT_EXTLINK_H__
#define __GM_TEXT_EXTLINK_H__

#include "gm-text-buffer-enhancer-helper-iface.h"

G_BEGIN_DECLS

/* public api */

GmTextBufferEnhancerHelperIFace* gm_text_extlink_new (const gchar* regex,
						      GtkTextTag* tag);

/* GObject boilerplate */

typedef struct _GmTextExtlink      GmTextExtlink;
typedef struct _GmTextExtlinkClass GmTextExtlinkClass;

struct _GmTextExtlink {
  GObject parent;
};

struct _GmTextExtlinkClass {
  GObjectClass parent_class;
};

#define GM_TYPE_TEXT_EXTLINK (gm_text_extlink_get_type())
#define GM_TEXT_EXTLINK(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GM_TYPE_TEXT_EXTLINK,GmTextExtlink))
#define GM_TEXT_EXTLINK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GM_TYPE_TEXT_EXTLINK,GObject))
#define GM_IS_TEXT_EXTLINK(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GM_TYPE_TEXT_EXTLINK))
#define GM_IS_TEXT_EXTLINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GM_TYPE_TEXT_EXTLINK))
#define GM_TEXT_EXTLINK_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj),GM_TYPE_TEXT_EXTLINK,GmTextExtlinkClass))

GType gm_text_extlink_get_type () G_GNUC_CONST;

G_END_DECLS

#endif /* __GM_TEXT_EXTLINK_H__ */
