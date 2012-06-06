
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
 *                         codecsbox.c  -  description
 *                         -------------------------------
 *   begin                : Sat Sep 2 2006
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a codecs box widget
 *
 */

#include "config.h"
#include "codecsbox.h"

#include "gmconf.h"
#include "codec-description.h"
#include <cstdlib>
#include <cstring>
#include <sstream>

/* Columns for the codecs page */
enum {

  COLUMN_CODEC_ACTIVE,
  COLUMN_CODEC_NAME, 
  COLUMN_CODEC_CLOCKRATE,
  COLUMN_CODEC_PROTOCOLS,
  COLUMN_CODEC_CONFIG_NAME,
  COLUMN_CODEC_SELECTABLE,
  COLUMN_CODEC_AUDIO,
  COLUMN_CODEC_NUMBER
};

struct _CodecsBoxPrivate
{
  Ekiga::Call::StreamType type;
  GtkWidget *codecs_list;
};

enum { TYPE = 1 };

static GObjectClass *parent_class = NULL;


/* Static functions */
static void codecs_box_set_codecs (CodecsBox *self,
                                   GSList *list);


/* GTK+ Callbacks */
static void codec_toggled_cb (GtkCellRendererToggle *cell,
                              gchar *path_str,
                              gpointer data);

static void codec_moved_cb (GtkWidget *widget, 
                            gpointer data);

static GSList *codecs_box_to_gm_conf_list (CodecsBox *self);


/* Static functions and declarations */
static void codecs_box_class_init (gpointer g_class,
                                   gpointer class_data);

static void codecs_box_init (CodecsBox *);

static void codecs_box_dispose (GObject *obj);

static void codecs_box_finalize (GObject *obj);

static void codecs_box_get_property (GObject *obj,
                                     guint prop_id,
                                     GValue *value,
                                     GParamSpec *spec);

static void codecs_box_set_property (GObject *obj,
                                     guint prop_id,
                                     const GValue *value,
                                     GParamSpec *spec);



static void 
codecs_box_set_codecs (CodecsBox *self,
                       GSList *list)
{
  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  GSList *codecs_data = NULL;
  GSList *codecs_data_iter = NULL;

  gchar *selected_codec = NULL;
  unsigned select_rate = 0;
  bool selected = false;

  g_return_if_fail (self != NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->codecs_list));
  codecs_data = list;

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->codecs_list));

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) 
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                        COLUMN_CODEC_NAME, &selected_codec, 
                        COLUMN_CODEC_CLOCKRATE, &select_rate, -1);
  gtk_list_store_clear (GTK_LIST_STORE (model));

  codecs_data_iter = codecs_data;
  while (codecs_data_iter) {

    Ekiga::CodecDescription desc = Ekiga::CodecDescription ((char *) codecs_data_iter->data);

    if ((self->priv->type == Ekiga::Call::Audio && desc.audio)
        || (self->priv->type == Ekiga::Call::Video && !desc.audio)) {
      std::stringstream rate;
      std::stringstream protocols;

      for (std::list<std::string>::iterator it = desc.protocols.begin ();
           it != desc.protocols.end ();
           it++) {
        if (it != desc.protocols.begin ())
          protocols << ", ";

        protocols << *it;
      }
      rate << desc.rate / 1000 << " kHz";

      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          COLUMN_CODEC_ACTIVE, desc.active,
                          COLUMN_CODEC_NAME, desc.name.c_str (),
                          COLUMN_CODEC_CLOCKRATE, rate.str ().c_str (),
                          COLUMN_CODEC_PROTOCOLS, protocols.str ().c_str (),
                          COLUMN_CODEC_CONFIG_NAME, desc.str ().c_str (),
                          COLUMN_CODEC_AUDIO, desc.audio,
                          COLUMN_CODEC_SELECTABLE, "true",
                          -1);

      if (selected_codec && desc.name == selected_codec
          && select_rate && desc.rate == select_rate) {

        selected = true;
        gtk_tree_selection_select_iter (selection, &iter);
      }
    }

    codecs_data_iter = g_slist_next (codecs_data_iter);
  }

  if (!selected && gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    gtk_tree_selection_select_iter (selection, &iter);
}


static void
codec_toggled_cb (G_GNUC_UNUSED GtkCellRendererToggle *cell,
                  gchar *path_str,
                  gpointer data)
{
  CodecsBox *self = NULL;

  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GSList *codecs_data = NULL;

  gboolean fixed = FALSE;

  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_CODECS_BOX (data));

  self = CODECS_BOX (data);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->codecs_list));
  path = gtk_tree_path_new_from_string (path_str);

  /* Update the tree model */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter, COLUMN_CODEC_ACTIVE, &fixed, -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      COLUMN_CODEC_ACTIVE, fixed^1, -1);
  gtk_tree_path_free (path);

  /* Update the gmconf key */
  codecs_data = codecs_box_to_gm_conf_list (self);
  if (self->priv->type == Ekiga::Call::Audio)
    gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/codecs/audio/media_list", codecs_data);
  else if (self->priv->type == Ekiga::Call::Video)
    gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/codecs/video/media_list", codecs_data);
  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}


static void
codec_moved_cb (GtkWidget *widget, 
                gpointer data)
{ 	
  CodecsBox *self = NULL;

  GtkTreeIter iter;
  GtkTreeIter *iter2 = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreePath *tree_path = NULL;

  GSList *codecs_data = NULL;

  gchar *path_str = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (IS_CODECS_BOX (data));

  self = CODECS_BOX (data);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->codecs_list));
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (self->priv->codecs_list));
  if (!gtk_tree_selection_get_selected (GTK_TREE_SELECTION (selection), 
                                        NULL, &iter))
    return;

  /* Update the tree view */
  iter2 = gtk_tree_iter_copy (&iter);
  path_str = gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (model), 
                                                  &iter);
  tree_path = gtk_tree_path_new_from_string (path_str);
  if (!strcmp ((gchar *) g_object_get_data (G_OBJECT (widget), "operation"), 
               "up"))
    gtk_tree_path_prev (tree_path);
  else
    gtk_tree_path_next (tree_path);

  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, tree_path);
  if (gtk_list_store_iter_is_valid (GTK_LIST_STORE (model), &iter)
      && gtk_list_store_iter_is_valid (GTK_LIST_STORE (model), iter2))
    gtk_list_store_swap (GTK_LIST_STORE (model), &iter, iter2);

  /* Scroll to the new position */
  gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (self->priv->codecs_list), 
                                tree_path, NULL, FALSE, 0, 0);

  gtk_tree_path_free (tree_path);
  gtk_tree_iter_free (iter2);
  g_free (path_str);

  /* Update the gmconf key */
  codecs_data = codecs_box_to_gm_conf_list (self);
  if (self->priv->type == Ekiga::Call::Audio)
    gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/codecs/audio/media_list", codecs_data);
  else if (self->priv->type == Ekiga::Call::Video)
    gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/codecs/video/media_list", codecs_data);
  g_slist_foreach (codecs_data, (GFunc) g_free, NULL);
  g_slist_free (codecs_data);
}


static GSList *
codecs_box_to_gm_conf_list (CodecsBox *self)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *name = NULL;
  gchar *rate = NULL;
  gchar *protocols = NULL;
  gboolean active = false;
  gboolean audio = false;

  GSList *codecs_data = NULL;

  g_return_val_if_fail (self != NULL, NULL);
  g_return_val_if_fail (IS_CODECS_BOX (self), NULL);

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (self->priv->codecs_list));
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      gtk_tree_model_get (model, &iter, 
                          COLUMN_CODEC_ACTIVE, &active,
                          COLUMN_CODEC_NAME, &name,
                          COLUMN_CODEC_CLOCKRATE, &rate,
                          COLUMN_CODEC_PROTOCOLS, &protocols,
                          COLUMN_CODEC_AUDIO, &audio,
                          -1);

      Ekiga::CodecDescription desc;
      desc = Ekiga::CodecDescription (name, atoi (rate) * 1000, audio, protocols, active);
      codecs_data = g_slist_append (codecs_data, g_strdup (desc.str ().c_str ()));

      g_free (name);
      g_free (protocols);
      g_free (rate);

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  return codecs_data;
}


static void 
codecs_list_changed_nt (G_GNUC_UNUSED gpointer id,
                        GmConfEntry *entry,
                        gpointer data)
{
  CodecsBox *self = CODECS_BOX (data);

  GSList *list = gm_conf_entry_get_list (entry);
  GSList *current_list = codecs_box_to_gm_conf_list (self);
  Ekiga::CodecList clist (list);
  Ekiga::CodecList curlist (current_list);

  if (clist != curlist)
    codecs_box_set_codecs (self, list);

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  g_slist_foreach (current_list, (GFunc) g_free, NULL);
  g_slist_free (current_list);
}


static void
codecs_box_class_init (gpointer g_class,
                       gpointer /*class_data*/)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *spec = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = codecs_box_dispose;
  gobject_class->finalize = codecs_box_finalize;
  gobject_class->get_property = codecs_box_get_property;
  gobject_class->set_property = codecs_box_set_property;

  spec = g_param_spec_int ("type", "Type", "Type",
                           0, 1, 0, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, TYPE, spec);
}


static void
codecs_box_init (CodecsBox *self)
{
  GtkWidget *image = NULL;
  GtkWidget *scroll_window = NULL;
  GtkWidget *button = NULL;

  GtkWidget *buttons_vbox = NULL;
  GtkWidget *alignment = NULL;

  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;

  g_return_if_fail (self != NULL);
  g_return_if_fail (IS_CODECS_BOX (self));

  self->priv = new CodecsBoxPrivate;
  self->priv->type = Ekiga::Call::Audio;

  gtk_box_set_spacing (GTK_BOX (self), 6);
  gtk_box_set_homogeneous (GTK_BOX (self), FALSE);

  self->priv->codecs_list = gtk_tree_view_new ();

  list_store = gtk_list_store_new (COLUMN_CODEC_NUMBER,
                                   G_TYPE_BOOLEAN,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING,
                                   G_TYPE_STRING,
                                   G_TYPE_BOOLEAN,
                                   G_TYPE_BOOLEAN);

  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (self->priv->codecs_list), TRUE);
  gtk_tree_view_set_reorderable (GTK_TREE_VIEW (self->priv->codecs_list), TRUE);
  gtk_tree_view_set_search_column (GTK_TREE_VIEW (self->priv->codecs_list),0);
  gtk_tree_view_set_model (GTK_TREE_VIEW (self->priv->codecs_list), 
                           GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (self->priv->codecs_list), FALSE);

  /* Set all Colums */
  renderer = gtk_cell_renderer_toggle_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "active", 
                                                     COLUMN_CODEC_ACTIVE,
                                                     NULL);
  gtk_tree_view_column_add_attribute (column, renderer, 
                                      "activatable", COLUMN_CODEC_SELECTABLE);
  gtk_tree_view_column_set_fixed_width (GTK_TREE_VIEW_COLUMN (column), 25);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->codecs_list), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (codec_toggled_cb),
                    (gpointer) self);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_CODEC_NAME,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->codecs_list), column);
  g_object_set (G_OBJECT (renderer), "weight", PANGO_WEIGHT_BOLD, NULL);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_CODEC_CLOCKRATE,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->codecs_list), column);

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes (NULL,
                                                     renderer,
                                                     "text", 
                                                     COLUMN_CODEC_PROTOCOLS,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (self->priv->codecs_list), column);

  scroll_window = gtk_scrolled_window_new (FALSE, FALSE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll_window), 
                                  GTK_POLICY_NEVER, 
                                  GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scroll_window),
                                       GTK_SHADOW_IN);
  gtk_widget_set_size_request (scroll_window, -1, 120);
  gtk_container_add (GTK_CONTAINER (scroll_window), 
                     GTK_WIDGET (self->priv->codecs_list));
  gtk_box_pack_start (GTK_BOX (self), scroll_window, TRUE, TRUE, 0);


  /* The buttons */
  alignment = gtk_alignment_new (1, 0.5, 0, 0);
  buttons_vbox = gtk_vbox_new (TRUE, 6);

  gtk_container_add (GTK_CONTAINER (alignment), buttons_vbox);

  image = gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU);
  button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_set_tooltip_text (button, _("Move selected codec priority upwards"));
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, FALSE, FALSE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "up");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (codec_moved_cb), 
                    (gpointer) self);

  image = gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU);
  button = gtk_button_new ();
  gtk_container_add (GTK_CONTAINER (button), image);
  gtk_widget_set_tooltip_text (button, _("Move selected codec priority downwards"));
  gtk_box_pack_start (GTK_BOX (buttons_vbox), button, FALSE, FALSE, 0);
  g_object_set_data (G_OBJECT (button), "operation", (gpointer) "down");
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (codec_moved_cb), 
                    (gpointer) self);

  gtk_box_pack_start (GTK_BOX (self), alignment, FALSE, FALSE, 0);

  gtk_widget_show_all (GTK_WIDGET (self));
}


static void
codecs_box_dispose (GObject *obj)
{
  CodecsBox *self = NULL;

  self = CODECS_BOX (obj);

  self->priv->codecs_list = NULL;

  parent_class->dispose (obj);
}


static void
codecs_box_finalize (GObject *obj)
{
  parent_class->finalize (obj);
}


static void
codecs_box_get_property (GObject *obj,
                         guint prop_id,
                         GValue *value,
                         GParamSpec *spec)
{
  CodecsBox *self = NULL;

  self = CODECS_BOX (obj);

  switch (prop_id) {

  case TYPE:
    g_value_set_int (value, self->priv->type);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
codecs_box_set_property (GObject *obj,
                         guint prop_id,
                         const GValue *value,
                         GParamSpec *spec)
{
  CodecsBox *self = NULL;
  GSList *list = NULL;

  self = CODECS_BOX (obj);

  switch (prop_id) {

  case TYPE:
    self->priv->type = (Ekiga::Call::StreamType) g_value_get_int (value); 
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }

  if (self->priv->type == Ekiga::Call::Audio)
    list = gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/codecs/audio/media_list");
  else if (self->priv->type == Ekiga::Call::Video)
    list = gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/codecs/video/media_list");

  codecs_box_set_codecs (self, list);

  g_slist_foreach (list, (GFunc) g_free, NULL);
  g_slist_free (list);

  if (self->priv->type == Ekiga::Call::Audio)
    gm_conf_notifier_add ("/apps/" PACKAGE_NAME "/codecs/audio/media_list", codecs_list_changed_nt, GTK_WIDGET (self));
  else
    gm_conf_notifier_add ("/apps/" PACKAGE_NAME "/codecs/video/media_list", codecs_list_changed_nt, GTK_WIDGET (self));
}


/* Global functions */
GType
codecs_box_get_type (void)
{
  static GType codecs_box_type = 0;

  if (codecs_box_type == 0)
    {
      static const GTypeInfo codecs_box_info =
        {
          sizeof (CodecsBoxClass),
          NULL,
          NULL,
          (GClassInitFunc) codecs_box_class_init,
          NULL,
          NULL,
          sizeof (CodecsBox),
          0,
          (GInstanceInitFunc) codecs_box_init,
          NULL
        };

      codecs_box_type =
        g_type_register_static (GTK_TYPE_HBOX,
                                "CodecsBox",
                                &codecs_box_info,
                                (GTypeFlags) 0);
    }

  return codecs_box_type;
}


GtkWidget *
codecs_box_new ()
{
  return GTK_WIDGET (CODECS_BOX (g_object_new (CODECS_BOX_TYPE, NULL)));
}


GtkWidget *
codecs_box_new_with_type (Ekiga::Call::StreamType type)
{
  GtkWidget *codecs_box = NULL;

  codecs_box = codecs_box_new ();
  g_object_set (G_OBJECT (codecs_box), "type", type, NULL);

  return codecs_box;
}
