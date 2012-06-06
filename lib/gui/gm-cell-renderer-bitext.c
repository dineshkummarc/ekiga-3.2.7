
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
 *                         gm-cell-renderer-bitext.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt, but very directly
 *                          inspired by the code of GossipCellRendererText in
 *                          imendio's gossip instant messaging client
 *   copyright            : (c) 2004 by Imendio AB
 *                          (c) 2006-2007 by Julien Puydt
 *   description          : implementation of a cell renderer showing two texts
 *
 */

#include <string.h>

#include "gm-cell-renderer-bitext.h"

struct _GmCellRendererBitextPrivate
{

  gchar *primary_text;
  gchar *secondary_text;

  gboolean is_valid;
  gboolean is_selected;
};

enum
{

  GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT = 1,
  GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT
};

static GObjectClass *parent_class = NULL;

/* helper function */

static void
gm_cell_renderer_bitext_update_text (GmCellRendererBitext *renderer,
				     GtkWidget *widget,
				     gboolean is_selected)
{
  GtkStyle *style = NULL;
  PangoAttrList *attr_list = NULL;
  GdkColor color;
  PangoAttribute *attr_color = NULL;
  PangoAttribute *attr_style = NULL;
  PangoAttribute *attr_size = NULL;
  gchar *str = NULL;

  if (renderer->priv->is_valid && renderer->priv->is_selected == is_selected)
    return;

  style = gtk_widget_get_style (widget);

  attr_list = pango_attr_list_new ();

  /* secondary text will be in italic */
  attr_style = pango_attr_style_new (PANGO_STYLE_NORMAL);
  attr_style->start_index = strlen (renderer->priv->primary_text) + 1;
  attr_style->end_index = (guint) - 1;
  pango_attr_list_insert (attr_list, attr_style);

  if (!is_selected) {

    color = style->text_aa[GTK_STATE_NORMAL];

    attr_color = pango_attr_foreground_new (color.red,
					    color.green, color.blue);
    attr_color->start_index = attr_style->start_index;
    attr_color->end_index = (guint) - 1;
    pango_attr_list_insert (attr_list, attr_color);
  }

  attr_size = pango_attr_size_new ((int) (pango_font_description_get_size (style->font_desc) * 0.8));	/* we want the secondary text smaller */
  attr_size->start_index = attr_style->start_index;
  attr_size->end_index = (guint) - 1;
  pango_attr_list_insert (attr_list, attr_size);

  if (renderer->priv->secondary_text && strcmp (renderer->priv->secondary_text, ""))
    str = g_strdup_printf ("%s\n%s",
                           renderer->priv->primary_text,
                           renderer->priv->secondary_text);
  else
    str = g_strdup_printf ("%s",
                           renderer->priv->primary_text);

  g_object_set (renderer,
		"visible", TRUE,
		"weight", PANGO_WEIGHT_NORMAL,
		"text", str,
		"attributes", attr_list,
		NULL);
  g_free (str);
  pango_attr_list_unref (attr_list);

  renderer->priv->is_selected = is_selected;
  renderer->priv->is_valid = TRUE;
}

/* overriden inherited functions, so we make sure the text is right before
 * we compute size or draw */

static void
gm_cell_renderer_bitext_get_size (GtkCellRenderer *cell,
				  GtkWidget *widget,
				  GdkRectangle *cell_area,
				  gint *x_offset,
				  gint *y_offset,
				  gint *width,
				  gint *height)
{
  GmCellRendererBitext *renderer = NULL;

  renderer = (GmCellRendererBitext *)cell;

  gm_cell_renderer_bitext_update_text (renderer, widget,
				       renderer->priv->is_selected);

  ((GtkCellRendererClass *)parent_class)->get_size (cell, widget, cell_area,
						    x_offset, y_offset,
						    width, height);
}

static void
gm_cell_renderer_bitext_render (GtkCellRenderer *cell,
				GdkWindow *window,
				GtkWidget *widget,
				GdkRectangle *background_area,
				GdkRectangle *cell_area,
				GdkRectangle *expose_area,
				GtkCellRendererState flags)
{
  GmCellRendererBitext *renderer = NULL;

  renderer = (GmCellRendererBitext *)cell;

  gm_cell_renderer_bitext_update_text (renderer, widget,
				       (flags & GTK_CELL_RENDERER_SELECTED));
  ((GtkCellRendererClass *)parent_class)->render (cell, window, widget,
						  background_area, cell_area,
						  expose_area, flags);
}

/* GObject code */

static void
gm_cell_renderer_bitext_finalize (GObject *obj)
{
  GmCellRendererBitext *self = NULL;

  self = (GmCellRendererBitext *)obj;

  g_free (self->priv->primary_text);
  g_free (self->priv->secondary_text);

  parent_class->finalize (obj);
}

static void
gm_cell_renderer_bitext_get_property (GObject *obj,
				      guint prop_id,
				      GValue *value,
				      GParamSpec *spec)
{
  GmCellRendererBitext *renderer = NULL;

  renderer = (GmCellRendererBitext *)obj;

  switch (prop_id) {

  case GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT:
    g_value_set_string (value, renderer->priv->primary_text);
    break;

  case GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT:
    g_value_set_string (value, renderer->priv->secondary_text);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gm_cell_renderer_bitext_set_property (GObject *obj,
				      guint prop_id,
				      const GValue *value,
				      GParamSpec *spec)
{
  GmCellRendererBitext *renderer = NULL;
  const gchar *str = NULL;

  renderer = (GmCellRendererBitext *)obj;

  switch (prop_id) {

  case GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT:
    g_free (renderer->priv->primary_text);
    str = g_value_get_string (value);
    renderer->priv->primary_text = g_strdup (str ? str : "");
    (void) g_strdelimit (renderer->priv->primary_text, "\n\r\t", ' ');
    renderer->priv->is_valid = FALSE;
    break;

  case GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT:
    g_free (renderer->priv->secondary_text);
    str = g_value_get_string (value);
    renderer->priv->secondary_text = g_strdup (str ? str : "");
    (void) g_strdelimit (renderer->priv->secondary_text, "\n\r\t", ' ');
    renderer->priv->is_valid = FALSE;
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}

static void
gm_cell_renderer_bitext_class_init (gpointer g_class,
				    gpointer class_data)
{
  GObjectClass *gobject_class = NULL;
  GtkCellRendererClass *renderer_class = NULL;
  GParamSpec *spec = NULL;

  (void)class_data; /* -Wextra */

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  g_type_class_add_private (g_class, sizeof (GmCellRendererBitextPrivate));

  renderer_class = (GtkCellRendererClass *) g_class;
  renderer_class->get_size = gm_cell_renderer_bitext_get_size;
  renderer_class->render = gm_cell_renderer_bitext_render;

  gobject_class = (GObjectClass *) g_class;
  gobject_class->finalize = gm_cell_renderer_bitext_finalize;
  gobject_class->get_property = gm_cell_renderer_bitext_get_property;
  gobject_class->set_property = gm_cell_renderer_bitext_set_property;

  spec = g_param_spec_string ("primary-text",
			      "Primary text",
			      "Primary text",
			      NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
				   GM_CELL_RENDERER_BITEXT_PROP_PRIMARY_TEXT,
				   spec);

  spec = g_param_spec_string ("secondary-text",
			      "Secondary text",
			      "Secondary text",
			      NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class,
				   GM_CELL_RENDERER_BITEXT_PROP_SECONDARY_TEXT,
				   spec);
}

static void
gm_cell_renderer_bitext_init (GTypeInstance *instance,
			      gpointer g_class)
{
  GmCellRendererBitext *self = NULL;

  (void)g_class; /* -Wextra */

  self = (GmCellRendererBitext *)instance;

  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self,
					    GM_TYPE_CELL_RENDERER_BITEXT,
					    GmCellRendererBitextPrivate);
  self->priv->primary_text = g_strdup ("");
  self->priv->secondary_text = g_strdup ("");
}

GType
gm_cell_renderer_bitext_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GmCellRendererBitextClass),
      NULL,
      NULL,
      gm_cell_renderer_bitext_class_init,
      NULL,
      NULL,
      sizeof (GmCellRendererBitext),
      0,
      gm_cell_renderer_bitext_init,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_CELL_RENDERER_TEXT,
				     "GmCellRendererBitextType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkCellRenderer *
gm_cell_renderer_bitext_new ()
{
  return (GtkCellRenderer *)g_object_new (GM_TYPE_CELL_RENDERER_BITEXT,
					  NULL);
}
