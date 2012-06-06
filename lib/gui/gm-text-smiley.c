
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
 *                        gm-text-smiley.c  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a text decorator for smileys
 *
 */

#include "gm-text-smiley.h"

#include "gm-smileys.h"

#include <string.h>

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
enhancer_helper_check (G_GNUC_UNUSED GmTextBufferEnhancerHelperIFace* self,
		       const gchar* full_text,
		       gint from,
		       gint* start,
		       gint* length)
{
  const gchar **smileys = gm_get_smileys ();
  gint smiley = 0;
  gint best_start = 0;
  gint best_smiley = -1;
  char* found = NULL;
  gint found_start = 0;

  /* all smileys are checked, and the one chosen is:
     - the one which starts the soonest;
     - in case of equality, the one which is the longest.
  */
  for (smiley = 0;
       smileys[smiley] != NULL;
       smiley = smiley + 2) {

    found = strstr (full_text + from, smileys[smiley]);
    if (found != NULL) {

      found_start = found - full_text;
      if ((best_smiley == -1)
          || (found_start < best_start)
	  || ((found_start == best_start)
	      && (strlen (smileys[smiley]) > strlen (smileys[best_smiley])))) {

	best_smiley = smiley;
	best_start = found_start;
      }
    }
  }


  if (best_smiley != -1) {

    *start = best_start;
    *length = strlen (smileys[best_smiley]);
  } else
    *length = 0;
}

static void
enhancer_helper_enhance (G_GNUC_UNUSED GmTextBufferEnhancerHelperIFace* self,
			 GtkTextBuffer* buffer,
			 GtkTextIter* iter,
			 G_GNUC_UNUSED GSList** tags,
			 G_GNUC_UNUSED const gchar* full_text,
			 gint* start,
			 gint length)
{
  const gchar **smileys = gm_get_smileys ();
  gchar* smiley = NULL;
  gint ii = 0;
  const gchar* pixbuf_name = NULL;
  GdkPixbuf* pixbuf = NULL;

  smiley = g_malloc0 (length + 1);

  strncpy (smiley, full_text + *start, length);

  for (ii = 0;
       smileys[ii] != NULL;
       ii = ii + 2) {

    if (strcmp (smiley, smileys[ii]) == 0)
      pixbuf_name = smileys[ii + 1];
  }

  if (pixbuf_name != NULL) {

    pixbuf = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
				       pixbuf_name, 16, 0, NULL);
    gtk_text_buffer_insert_pixbuf (buffer, iter, pixbuf);
    g_object_unref (pixbuf);
    *start = *start + length;
  }

  g_free (smiley);
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
gm_text_smiley_class_init (GmTextSmileyClass* g_class)
{
  parent_class = g_type_class_peek_parent (g_class);
}

GType
gm_text_smiley_get_type ()
{
  static GType result = 0;
  if (!result) {

    static const GTypeInfo my_info = {
      sizeof(GmTextSmileyClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_text_smiley_class_init,
      NULL,
      NULL,
      sizeof(GmTextSmiley),
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
				     "GmTextSmiley",
				     &my_info, 0);
    g_type_add_interface_static (result,
				 GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE,
				 &enhancer_helper_info);
  }
  return result;
}

/* public api */

GmTextBufferEnhancerHelperIFace*
gm_text_smiley_new (void)
{
  return (GmTextBufferEnhancerHelperIFace*)g_object_new(GM_TYPE_TEXT_SMILEY, NULL);
}
