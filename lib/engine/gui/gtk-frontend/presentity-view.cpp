
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
 *                        presentity-view.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a Chat area (view and control)
 *
 */

#include "presentity-view.h"

class PresentityViewHelper;

struct _PresentityViewPrivate
{
  Ekiga::PresentityPtr presentity;
  sigc::connection updated_conn;
  sigc::connection removed_conn;

  /* we contain those, so no need to unref them */
  GtkWidget* presence_image;
  GtkWidget* name_status;
  GtkWidget* avatar_image;
};

enum {
  PRESENTITY_VIEW_PROP_PRESENTITY = 1
};

static GObjectClass* parent_class = NULL;

/* declaration of callbacks */

static void on_presentity_updated (PresentityView* self);

static void on_presentity_removed (PresentityView* self);

/* declaration of internal api */

static void presentity_view_set_presentity (PresentityView* self,
					    Ekiga::PresentityPtr presentity);

static void presentity_view_unset_presentity (PresentityView* self);

/* implementation of callbacks */

static void
on_presentity_updated (PresentityView* self)
{
  gchar *txt = NULL;

  gtk_image_set_from_stock (GTK_IMAGE (self->priv->presence_image),
			    self->priv->presentity->get_presence ().c_str (),
			    GTK_ICON_SIZE_MENU);
  if (!self->priv->presentity->get_status ().empty ())
    txt = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>\n<span size=\"small\">%s</span>",
                                   self->priv->presentity->get_name ().c_str (),
                                   self->priv->presentity->get_status ().c_str ());
  else
    txt = g_markup_printf_escaped ("<span weight=\"bold\">%s</span>",
                                   self->priv->presentity->get_name ().c_str ());

  gtk_label_set_markup (GTK_LABEL (self->priv->name_status), txt);
  g_free (txt);
  gtk_image_set_from_stock (GTK_IMAGE (self->priv->avatar_image),
			    self->priv->presentity->get_avatar ().c_str (),
			    GTK_ICON_SIZE_MENU);
}

static void
on_presentity_removed (PresentityView* self)
{
  presentity_view_unset_presentity (self);
}

/* implementation of internal api */

static void
presentity_view_set_presentity (PresentityView* self,
				Ekiga::PresentityPtr presentity)
{
  g_return_if_fail ( !self->priv->presentity);

  self->priv->presentity = presentity;
  self->priv->updated_conn = self->priv->presentity->updated.connect (sigc::bind (sigc::ptr_fun (on_presentity_updated), self));
  self->priv->removed_conn = self->priv->presentity->removed.connect (sigc::bind (sigc::ptr_fun (on_presentity_removed), self));

  on_presentity_updated (self);
}

static void
presentity_view_unset_presentity (PresentityView* self)
{
  if (self->priv->presentity) {

    self->priv->presentity = Ekiga::PresentityPtr(0);
    self->priv->updated_conn.disconnect ();
    self->priv->removed_conn.disconnect ();
  }
}

/* GObject code */

static void
presentity_view_finalize (GObject* obj)
{
  PresentityView* self = NULL;

  self = (PresentityView*)obj;

  presentity_view_unset_presentity (self);

  delete self->priv;
  self->priv = NULL;

  parent_class->finalize (obj);
}

static void
presentity_view_set_property (GObject* obj,
			      guint prop_id,
			      const GValue* value,
			      GParamSpec* spec)
{
  PresentityView* self = NULL;
  Ekiga::Presentity* presentity = NULL;

  self = (PresentityView* )obj;

  switch (prop_id) {

  case PRESENTITY_VIEW_PROP_PRESENTITY:
    presentity = (Ekiga::Presentity*)g_value_get_pointer (value);
    presentity_view_set_presentity (self, Ekiga::PresentityPtr(presentity));

    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
presentity_view_class_init (gpointer g_class,
			    G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass* gobject_class = NULL;
  GParamSpec* spec = NULL;

  parent_class = (GObjectClass*)g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*)g_class;
  gobject_class->finalize = presentity_view_finalize;
  gobject_class->set_property = presentity_view_set_property;

  spec = g_param_spec_pointer ("presentity",
			       "displayed presentity",
			       "Displayed presentity",
			       (GParamFlags)(G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (gobject_class,
				   PRESENTITY_VIEW_PROP_PRESENTITY,
				   spec);
}

static void
presentity_view_init (GTypeInstance* instance,
		      G_GNUC_UNUSED gpointer g_class)
{
  PresentityView* self = NULL;

  self = (PresentityView*)instance;

  self->priv = new PresentityViewPrivate;

  self->priv->presence_image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (self), self->priv->presence_image,
		      FALSE, FALSE, 2);
  gtk_widget_show (self->priv->presence_image);

  self->priv->name_status = gtk_label_new (NULL);
  gtk_box_pack_start (GTK_BOX (self), self->priv->name_status,
		      FALSE, TRUE, 2);
  gtk_widget_show (self->priv->name_status);

  self->priv->avatar_image = gtk_image_new ();
  gtk_box_pack_start (GTK_BOX (self), self->priv->avatar_image,
		      FALSE, FALSE, 2);
  gtk_widget_show (self->priv->avatar_image);
}


GType
presentity_view_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (PresentityViewClass),
      NULL,
      NULL,
      presentity_view_class_init,
      NULL,
      NULL,
      sizeof (PresentityView),
      0,
      presentity_view_init,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_HBOX,
				     "PresentityView",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkWidget*
presentity_view_new (Ekiga::PresentityPtr presentity)
{
  return (GtkWidget*)g_object_new (TYPE_PRESENTITY_VIEW,
				   "presentity", presentity.get (),
				   NULL);
}
