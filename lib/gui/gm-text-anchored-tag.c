
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
 *                        gm-text-anchored-tag.c  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of an anchor-based text decorator
 *
 */

#include "gm-text-anchored-tag.h"

#include <string.h>

typedef struct _GmTextAnchoredTagPrivate GmTextAnchoredTagPrivate;

struct _GmTextAnchoredTagPrivate {
  gchar* anchor;
  GtkTextTag* tag;
  gboolean opening;
};

#define GM_TEXT_ANCHORED_TAG_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GM_TYPE_TEXT_ANCHORED_TAG, GmTextAnchoredTagPrivate))

static GObjectClass* parent_class = NULL;

/* declaration of the GmTextBufferEnhancerHelperIFace code */

static void enhancer_helper_check (GmTextBufferEnhancerHelperIFace* self,
				   const gchar* full_text,
				   gint from,
				   gint* start,
				   gint* length);

static void enhancer_helper_enhance (GmTextBufferEnhancerHelperIFace* self,
				     GtkTextBuffer* buffer,
				     GtkTextIter* iter,
				     GSList** tags,
				     const gchar* full_text,
				     gint* start,
				     gint length);

static void enhancer_helper_iface_init (gpointer g_iface,
					gpointer iface_data);

/* implementation of the GmTextBufferEnhancerHelperIFace code */

static void
enhancer_helper_check (GmTextBufferEnhancerHelperIFace* self,
		       const gchar* full_text,
		       gint from,
		       gint* start,
		       gint* length)
{
  GmTextAnchoredTagPrivate* priv = GM_TEXT_ANCHORED_TAG_GET_PRIVATE (self);
  char* found = NULL;

  found = strstr (full_text + from, priv->anchor);

  if (found != NULL) {

    *start = found - full_text;
    *length = strlen (priv->anchor);
  } else
    *length = 0;
}

static void
enhancer_helper_enhance (GmTextBufferEnhancerHelperIFace* self,
			 G_GNUC_UNUSED GtkTextBuffer* buffer,
			 G_GNUC_UNUSED GtkTextIter* iter,
			 GSList** tags,
			 G_GNUC_UNUSED const gchar* full_text,
			 gint* start,
			 gint length)
{
  GmTextAnchoredTagPrivate* priv = GM_TEXT_ANCHORED_TAG_GET_PRIVATE (self);

  if (priv->opening)
    *tags = g_slist_prepend (*tags, priv->tag);
  else
    *tags = g_slist_remove (*tags, priv->tag);

  *start = *start + length;
}

static void
enhancer_helper_iface_init (gpointer g_iface,
			    G_GNUC_UNUSED gpointer iface_data)
{
  GmTextBufferEnhancerHelperIFaceClass* iface = NULL;

  iface = (GmTextBufferEnhancerHelperIFaceClass*)g_iface;
  iface->do_check = &enhancer_helper_check;
  iface->do_enhance = &enhancer_helper_enhance;
}

/* GObject boilerplate */

static void
gm_text_anchored_tag_dispose (GObject* obj)
{
  GmTextAnchoredTagPrivate* priv = GM_TEXT_ANCHORED_TAG_GET_PRIVATE(obj);

  if (priv->tag != NULL) {

    g_object_unref (priv->tag);
    priv->tag = NULL;
  }

  parent_class->dispose (obj);
}

static void
gm_text_anchored_tag_finalize (GObject* obj)
{
  GmTextAnchoredTagPrivate* priv = GM_TEXT_ANCHORED_TAG_GET_PRIVATE(obj);

  if (priv->anchor != NULL) {

    g_free (priv->anchor);
    priv->anchor = NULL;
  }

  parent_class->finalize (obj);
}

static void
gm_text_anchored_tag_class_init (GmTextAnchoredTagClass* g_class)
{
  GObjectClass* gobject_class = NULL;

  parent_class = g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = gm_text_anchored_tag_dispose;
  gobject_class->finalize = gm_text_anchored_tag_finalize;

  g_type_class_add_private (gobject_class, sizeof (GmTextAnchoredTagPrivate));
}

static void
gm_text_anchored_tag_init (GmTextAnchoredTag* obj)
{
  GmTextAnchoredTagPrivate* priv = GM_TEXT_ANCHORED_TAG_GET_PRIVATE(obj);

  priv->anchor = NULL;
  priv->tag = NULL;
  priv->opening = TRUE;
}

GType
gm_text_anchored_tag_get_type (void)
{
  static GType result = 0;
  if (!result) {

    static const GTypeInfo my_info = {
      sizeof(GmTextAnchoredTagClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_text_anchored_tag_class_init,
      NULL,
      NULL,
      sizeof(GmTextAnchoredTag),
      0,
      (GInstanceInitFunc) gm_text_anchored_tag_init,
      NULL
    };

    static const GInterfaceInfo enhancer_helper_info = {
      enhancer_helper_iface_init,
      NULL,
      NULL
    };

    result = g_type_register_static (G_TYPE_OBJECT,
				     "GmTextAnchoredTag",
				     &my_info, 0);
    g_type_add_interface_static (result,
				 GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE,
				 &enhancer_helper_info);
  }
  return result;
}

/* Implementation of the public api */

GmTextBufferEnhancerHelperIFace*
gm_text_anchored_tag_new (const gchar* anchor,
			  GtkTextTag* tag,
			  gboolean opening)
{
  GmTextAnchoredTag* result = NULL;
  GmTextAnchoredTagPrivate* priv = NULL;

  g_return_val_if_fail (anchor != NULL, NULL);
  g_return_val_if_fail (GTK_IS_TEXT_TAG (tag), NULL);

  result = (GmTextAnchoredTag*)g_object_new (GM_TYPE_TEXT_ANCHORED_TAG, NULL);

  priv = GM_TEXT_ANCHORED_TAG_GET_PRIVATE (result);

  priv->anchor = g_strdup (anchor);

  g_object_ref (tag);
  priv->tag = tag;

  priv->opening = opening;

  return GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE (result);
}
