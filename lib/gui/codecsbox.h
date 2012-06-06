
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
 *                         codecsbox.h  -  description
 *                         -------------------------------
 *   begin                : Sat Sep 02 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a codecs box
 *
 */



#ifndef __CODECS_BOX_H
#define __CODECS_BOX_H

#include <glib/gi18n.h>
#include <glib-object.h>
#include <gtk/gtk.h>

#include "call.h"


/** This implements a CodecsBox for the Ekiga Engine.
 * The codecs list is loaded from the GmConf configuration engine, 
 * using the /apps/ekiga/codecs/audio/list or /apps/ekiga/codecs/video/list
 * following the type of codecs. The key is supposed to contain
 * all codecs supported by the Engine. The key value is formated so that it
 * respects the output of an Ekiga::CodecDescription.
 */

G_BEGIN_DECLS

#define CODECS_BOX_TYPE (codecs_box_get_type ())
#define CODECS_BOX(o) (G_TYPE_CHECK_INSTANCE_CAST ((o), CODECS_BOX_TYPE, CodecsBox))
#define CODECS_BOX_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), CODECS_BOX_TYPE, CodecsBoxClass))
#define IS_CODECS_BOX(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), CODECS_BOX_TYPE))
#define IS_CODECS_BOX_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), CODECS_BOX_TYPE))
#define CODECS_BOX_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), CODECS_BOX_TYPE, CodecsBoxClass))

typedef struct _CodecsBox CodecsBox;
typedef struct _CodecsBoxPrivate CodecsBoxPrivate;
typedef struct _CodecsBoxClass CodecsBoxClass;

struct _CodecsBox
{
  GtkHBox parent;

  CodecsBoxPrivate *priv;
};


struct _CodecsBoxClass
{
  GtkHBoxClass parent_class;
};

GType codecs_box_get_type (void);


/** Create a new CodecsBox for Ekiga::Call::Audio codecs.
 */
GtkWidget *codecs_box_new ();


/** Create a new CodecsBox.
 * @param type is a valid Ekiga::Call::StreamType (Audio or Video supported).
 */
GtkWidget *codecs_box_new_with_type (Ekiga::Call::StreamType type);

G_END_DECLS

#endif /* __CODECS_BOX_H */
