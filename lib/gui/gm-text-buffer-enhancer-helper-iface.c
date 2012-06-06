
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
 *                        gm-text-buffer-enhancer-helper-iface.c  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of the interface of a text decorator
 *                          (for use with GmTextBufferEnhancer)
 *
 */

#include "gm-text-buffer-enhancer-helper-iface.h"

/* GObject code */

static void
gm_text_buffer_enhancer_helper_iface_base_init (G_GNUC_UNUSED gpointer g_class)
{
  static gboolean initialized = FALSE;
  if (!initialized) {
    /* nothing */
    initialized = TRUE;
  }
}
GType
gm_text_buffer_enhancer_helper_iface_get_type (void)
{
  static GType result = 0;
  if (!result) {

    static const GTypeInfo my_info = {
      sizeof(GmTextBufferEnhancerHelperIFaceClass),
      gm_text_buffer_enhancer_helper_iface_base_init,
      NULL,
      NULL,
      NULL,
      NULL,
      0,
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (G_TYPE_INTERFACE,
				     "GmTextBufferEnhancerHelperIFace",
				     &my_info, 0);

    g_type_interface_add_prerequisite (result, G_TYPE_OBJECT);
  }
  return result;
}

/* public api */

void
gm_text_buffer_enhancer_helper_check (GmTextBufferEnhancerHelperIFace* self,
				      const gchar* full_text,
				      gint from,
				      gint* start,
				      gint* length)
{
  GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_GET_CLASS (self)->do_check (self, full_text, from, start, length);
}

void
gm_text_buffer_enhancer_helper_enhance (GmTextBufferEnhancerHelperIFace* self,
					GtkTextBuffer* buffer,
					GtkTextIter* iter,
					GSList** tags,
					const gchar* full_text,
					gint* start,
					gint length)
{
  GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_GET_CLASS (self)->do_enhance (self, buffer, iter, tags, full_text, start, length);
}
