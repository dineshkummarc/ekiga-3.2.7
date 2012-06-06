
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
 *                        gm-text-extlink.c  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a text decorator for external links
 *
 */

#include "gm-text-extlink.h"

#include <string.h>
#include <sys/types.h>
#include <regex.h>

static GObjectClass* parent_class = NULL;

typedef struct _GmTextExtlinkPrivate GmTextExtlinkPrivate;

struct _GmTextExtlinkPrivate {
  regex_t* regex;
  GtkTextTag* tag;
};

#define GM_TEXT_EXTLINK_GET_PRIVATE(o) (G_TYPE_INSTANCE_GET_PRIVATE((o), GM_TYPE_TEXT_EXTLINK, GmTextExtlinkPrivate))

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
  GmTextExtlinkPrivate* priv = GM_TEXT_EXTLINK_GET_PRIVATE (self);
  gint match;
  regmatch_t regmatch;

  match = regexec (priv->regex, full_text + from, 1, &regmatch, 0);
  if (!match) {

    *start = from + regmatch.rm_so;
    *length = regmatch.rm_eo - regmatch.rm_so;
  } else {

    *length = 0;
  }
}

static void
enhancer_helper_enhance (GmTextBufferEnhancerHelperIFace* self,
			 GtkTextBuffer* buffer,
			 GtkTextIter* iter,
			 G_GNUC_UNUSED GSList** tags,
			 const gchar* full_text,
			 gint* start,
			 gint length)
{
  GmTextExtlinkPrivate* priv = GM_TEXT_EXTLINK_GET_PRIVATE (self);
  gchar* link = NULL;

  link = g_malloc0 (length + 1);

  strncpy (link, full_text + *start, length);

  gtk_text_buffer_insert_with_tags (buffer, iter, link, length,
				    priv->tag, NULL);
  g_free (link);

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
gm_text_extlink_dispose (GObject* obj)
{
  GmTextExtlinkPrivate* priv = GM_TEXT_EXTLINK_GET_PRIVATE(obj);

  if (priv->tag != NULL) {

    g_object_unref (priv->tag);
    priv->tag = NULL;
  }

  parent_class->dispose (obj);
}

static void
gm_text_extlink_finalize (GObject* obj)
{
  GmTextExtlinkPrivate* priv = GM_TEXT_EXTLINK_GET_PRIVATE(obj);

  if (priv->regex != NULL) {

    regfree (priv->regex);
    priv->regex = NULL;
  }

  parent_class->finalize (obj);
}

static void
gm_text_extlink_class_init (GmTextExtlinkClass* g_class)
{
  GObjectClass* gobject_class = NULL;

  parent_class = g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = gm_text_extlink_dispose;
  gobject_class->finalize = gm_text_extlink_finalize;

  g_type_class_add_private (gobject_class, sizeof (GmTextExtlinkPrivate));
}

GType
gm_text_extlink_get_type ()
{
  static GType result = 0;
  if (!result) {

    static const GTypeInfo my_info = {
      sizeof(GmTextExtlinkClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_text_extlink_class_init,
      NULL,
      NULL,
      sizeof(GmTextExtlink),
      0,
      NULL,
      NULL
    };

    static const GInterfaceInfo enhancer_helper_info = {
      enhancer_helper_iface_init,
      NULL,
      NULL
    };

    result = g_type_register_static (G_TYPE_OBJECT,
				     "GmTextExtlink",
				     &my_info, 0);
    g_type_add_interface_static (result,
				 GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE,
				 &enhancer_helper_info);
  }
  return result;
}

/* public api */

GmTextBufferEnhancerHelperIFace*
gm_text_extlink_new (const gchar* regex,
		     GtkTextTag* tag)
{
  GmTextBufferEnhancerHelperIFace* result = NULL;
  GmTextExtlinkPrivate* priv = NULL;

  g_return_val_if_fail (regex != NULL, NULL);

  result = (GmTextBufferEnhancerHelperIFace*)g_object_new(GM_TYPE_TEXT_EXTLINK, NULL);
  priv = GM_TEXT_EXTLINK_GET_PRIVATE (result);

  g_object_ref (tag);
  priv->tag = tag;

  priv->regex = (regex_t*)g_malloc0 (sizeof(regex_t));
  if (regcomp (priv->regex, regex, REG_EXTENDED) != 0) {

    regfree (priv->regex);
    priv->regex = NULL;
    g_object_unref (result);
    result = NULL;
  }

  return result;
}
