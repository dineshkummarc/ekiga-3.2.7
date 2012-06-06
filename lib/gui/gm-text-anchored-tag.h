
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
 *                        gm-text-anchored-tag.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of an anchor-based text decorator
 *
 */

#ifndef __GM_TEXT_ANCHORED_TAG_H__
#define __GM_TEXT_ANCHORED_TAG_H__

#include "gm-text-buffer-enhancer-helper-iface.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* public api */

/* The anchor parameter is the text this decorator should look for,
 * the tag parameter is the tag which this decorator should apply/unapply,
 * and the opening parameter tells if it must apply (TRUE) or unapply (FALSE)
 *
 * Here is sample code showing how to use it :
 *  tag = gtk_text_buffer_create_tag (buffer, "bold",
 *				      "weight", PANGO_WEIGHT_BOLD,
 *				      NULL);
 *  helper = gm_text_anchored_tag_new ("<b>", tag, TRUE);
 *  gm_text_buffer_enhancer_add_helper (enhancer, helper);
 *  g_object_unref (helper);
 *  helper = gm_text_anchored_tag_new ("</b>", tag, FALSE);
 *  gm_text_buffer_enhancer_add_helper (enhancer, helper);
 *  g_object_unref (helper);
 *
 */
GmTextBufferEnhancerHelperIFace* gm_text_anchored_tag_new (const gchar* anchor,
							   GtkTextTag* tag,
							   gboolean opening);

/* GObject boilerplate */

typedef struct _GmTextAnchoredTag      GmTextAnchoredTag;
typedef struct _GmTextAnchoredTagClass GmTextAnchoredTagClass;

struct _GmTextAnchoredTag {
  GObject parent;
};

struct _GmTextAnchoredTagClass {
  GObjectClass parent_class;
};

GType gm_text_anchored_tag_get_type () G_GNUC_CONST;

#define GM_TYPE_TEXT_ANCHORED_TAG (gm_text_anchored_tag_get_type())
#define GM_TEXT_ANCHORED_TAG(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GM_TYPE_TEXT_ANCHORED_TAG,GmTextAnchoredTag))
#define GM_TEXT_ANCHORED_TAG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass),GM_TYPE_TEXT_ANCHORED_TAG,GObject))
#define GM_IS_TEXT_ANCHORED_TAG(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GM_TYPE_TEXT_ANCHORED_TAG))
#define GM_IS_TEXT_ANCHORED_TAG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GM_TYPE_TEXT_ANCHORED_TAG))
#define GM_TEXT_ANCHORED_TAG_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj),GM_TYPE_TEXT_ANCHORED_TAG,GmTextAnchoredTagClass))

G_END_DECLS

#endif /* __GM_TEXT_ANCHORED_TAG_H__ */
