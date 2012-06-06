
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
 *                        gm-text-buffer-enhancer-helper-iface.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Declaration of the interface of a text decorator
 *                          (for use with GmTextBufferEnhancer)
 *
 */

#ifndef __GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_H__
#define __GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GmTextBufferEnhancerHelperIFace      GmTextBufferEnhancerHelperIFace;
typedef struct _GmTextBufferEnhancerHelperIFaceClass GmTextBufferEnhancerHelperIFaceClass;

/* public api */

/* This method is used to have all enhancers tell what they think they can do
 * to enhance a text ; then the enhancer will choose the most suitable helper
 * and use the next method.
 * For this reason, the helper receives :
 * - the full text (full_text parameter) ;
 * - where in the full text it should start looking for interesting things (from
 *   parameter) ;
 * and it returns two values (the start and length out parameters) :
 * - where it thinks it can do something (the start parameter) ;
 * - on which length it will do it (the length parameter).
 *
 * The idea is :
 * - return a zero length if nothing interesting can be done (start will then
 *   be ignored by the caller) ;
 * - the length allows the enhancer to prefer the helper which saw "fooz" when
 *   compared to the one which saw "foo".
 */
void gm_text_buffer_enhancer_helper_check (GmTextBufferEnhancerHelperIFace* self,
					   const gchar* full_text,
					   gint from,
					   gint* start,
					   gint* length);


/* This method is called by the enhancer when the helper has been choosen
 * to act ; it receives :
 * - the buffer (buffer parameter) in which the enhanced text has to go ;
 * - the iterator (iter parameter) where to insert the enhanced text ;
 * - the active tags (tags parameter), which is an in&out parameter, hence
 *   the enhancer can change them and use them ;
 * - the full text which the enhancer is trying to insert ;
 * - where in the text the helper said it would work (the start parameter),
 *   as in&out parameter so the helper can swallow it ;
 * - the length (length parameter) of the part the helper said was interested
 *   in.
 *
 * "the helper said" means those values are those the helper answered in its
 * check method.
 */
void gm_text_buffer_enhancer_helper_enhance (GmTextBufferEnhancerHelperIFace* self,
					     GtkTextBuffer* buffer,
					     GtkTextIter* iter,
					     GSList** tags,
					     const gchar* full_text,
					     gint* start,
					     gint length);

/* GObject boilerplate */

struct _GmTextBufferEnhancerHelperIFaceClass {
  GTypeInterface parent;

  void (*do_check) (GmTextBufferEnhancerHelperIFace* self,
		    const gchar* full_text,
		    gint from,
		    gint* start,
		    gint* length);

  void (*do_enhance) (GmTextBufferEnhancerHelperIFace* self,
		      GtkTextBuffer* buffer,
		      GtkTextIter* iter,
		      GSList** tags,
		      const gchar* full_text,
		      gint* start,
		      gint length);
};

#define GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE (gm_text_buffer_enhancer_helper_iface_get_type())
#define GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj),GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE,GmTextBufferEnhancerHelperIFace))
#define GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_CAST((vtable),GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE,GTypeInterface))
#define GM_IS_TEXT_BUFFER_ENHANCER_HELPER_IFACE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE))
#define GM_IS_TEXT_BUFFER_ENHANCER_HELPER_IFACE_CLASS(vtable) (G_TYPE_CHECK_CLASS_TYPE((vtable),GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE))
#define GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_GET_CLASS(inst) (G_TYPE_INSTANCE_GET_INTERFACE((inst),GM_TYPE_TEXT_BUFFER_ENHANCER_HELPER_IFACE,GmTextBufferEnhancerHelperIFaceClass))

GType gm_text_buffer_enhancer_helper_iface_get_type () G_GNUC_CONST;

G_END_DECLS

#endif /* __GM_TEXT_BUFFER_ENHANCER_HELPER_IFACE_H__ */
