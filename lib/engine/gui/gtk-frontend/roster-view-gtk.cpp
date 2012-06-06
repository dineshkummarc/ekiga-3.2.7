
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
 *                         roster-view-gtk.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the roster view
 *
 */

#include <algorithm>
#include <iostream>
#include <vector>
#include <glib/gi18n.h>

#include "gm-cell-renderer-bitext.h"
#include "gmcellrendererexpander.h"
#include "gmstockicons.h"
#include "gmconf.h"
#include "menu-builder-tools.h"
#include "roster-view-gtk.h"
#include "menu-builder-gtk.h"
#include "toolbar-builder-gtk.h"
#include "form-dialog-gtk.h"


/* FIXME: this is stupid : I can't seem to be able to use the marshallers
 * defined in lib/ from here!
 */
static void gm_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);


/*
 * The Roster
 */
struct _RosterViewGtkPrivate
{
  _RosterViewGtkPrivate (Ekiga::PresenceCore &_core) : core (_core) { }

  std::vector<sigc::connection> connections;
  GtkTreeStore *store;
  GtkTreeView *tree_view;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GtkWidget* toolbar;
  GSList *folded_groups;
  gboolean show_offline_contacts;
  Ekiga::PresenceCore & core;
};

/* the different type of things which will appear in the view */
enum {

  TYPE_HEAP,
  TYPE_GROUP,
  TYPE_PRESENTITY
};

enum {
  PRESENTITY_SELECTED_SIGNAL,
  HEAP_SELECTED_SIGNAL,
  HEAP_GROUP_SELECTED_SIGNAL,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;


/* This is how things will be stored roughly :
 * - the heaps are at the top ;
 * - under each heap come the groups ;
 * - under each group come the presentities.
 *
 * For the heaps, we show the name.
 *
 * For the groups, we show the name.
 *
 * For the presentities, we show the name, the status and the presence.
 *
 * This means we can share and put the name in a column.
 *
 */
enum {

  COLUMN_TYPE,
  COLUMN_HEAP,
  COLUMN_PRESENTITY,
  COLUMN_NAME,
  COLUMN_STATUS,
  COLUMN_PRESENCE,
  COLUMN_ACTIVE,
  COLUMN_GROUP_SIZE,
  COLUMN_OFFLINE,
  COLUMN_NUMBER
};

/*
 * Helpers
 */

/* DESCRIPTION: Remove a child from a GtkContainer
 *
 */
static void remove_child (GtkWidget* child,
			  GtkWidget* container);

/* DESCRIPTION : Set of functions called when the user clicks in a view
 * BEHAVIOUR   : Folds/unfolds, shows a menu or triggers default action
 */
static void on_clicked_show_heap_menu (Ekiga::HeapPtr heap,
				       GdkEventButton* event);
static void on_clicked_show_heap_group_menu (Ekiga::HeapPtr heap,
					     const std::string name,
					     GdkEventButton* event);
static void on_clicked_show_presentity_menu (Ekiga::HeapPtr heap,
					     Ekiga::PresentityPtr presentity,
					     GdkEventButton* event);

static void on_clicked_fold (RosterViewGtk* self,
			     GtkTreePath* path,
			     const gchar* name);

static void on_clicked_trigger_presentity (Ekiga::PresentityPtr presentity);

/* DESCRIPTION : Called whenever a (online/total) count has to be updated
 * BEHAVIOUR   : Updates things...
 * PRE         : Both arguments have to be correct
 */
static void update_offline_count (RosterViewGtk* self,
				  GtkTreeIter* iter);

/* DESCRIPTION : Called when the user changes the preference for offline
 * BEHAVIOUR   : Updates things...
 * PRE         : The gpointer must be a RosterViewGtk
 */
static void show_offline_contacts_changed_nt (gpointer id,
					      GmConfEntry *entry,
					      gpointer data);

/* DESCRIPTION  : Called when the user selects a presentity
 * BEHAVIOR     : Emit the presentity-selected signal
 * PRE          : The gpointer must point to the RosterViewGtk GObject.
 */
static void on_selection_changed (GtkTreeSelection* selection,
				  gpointer data);

/* DESCRIPTION  : Called when the user right-clicks on a heap, group or
 *                presentity.
 * BEHAVIOR     : Update the menu and displays it as a popup.
 * PRE          : The gpointer must point to the RosterViewGtk GObject.
 */
static gint on_view_event_after (GtkWidget *tree_view,
			         GdkEventButton *event,
			         gpointer data);

/* DESCRIPTION : Helpers for the next function
 */

static gboolean presentity_hide_show_offline (RosterViewGtk* self,
					      GtkTreeModel* model,
					      GtkTreeIter* iter);
static gboolean group_hide_show_offline (RosterViewGtk* self,
					 GtkTreeModel* model,
					 GtkTreeIter* iter);
static gboolean heap_hide_show_offline (RosterViewGtk* self,
					GtkTreeModel* model,
					GtkTreeIter* iter);

/* DESCRIPTION : Called to decide whether to show a line ; used to hide/show
 *               offline contacts on demand.
 * BEHAVIOUR   : Returns TRUE if the line should be shown.
 * PRE         : The gpointer must point to a RosterViewGtk object.
 */
static gboolean tree_model_filter_hide_show_offline (GtkTreeModel *model,
						     GtkTreeIter *iter,
						     gpointer data);

/* DESCRIPTION  : Called for a given renderer in order to show or hide it.
 * BEHAVIOR     : Only show the renderer if current iter points to a line of
 *                type GPOINTER_TO_INT (data).
 * PRE          : The gpointer must be TYPE_HEAP, TYPE_CLUSTER or TYPE_PRESENTITY
 *                once casted using GPOINTER_TO_INT.
 */
static void show_cell_data_func (GtkTreeViewColumn *column,
				 GtkCellRenderer *renderer,
				 GtkTreeModel *model,
				 GtkTreeIter *iter,
				 gpointer data);


/* DESCRIPTION  : Called for a given renderer in order to modify properties.
 * BEHAVIOR     : Expand the expander renderer if required.
 *                Hide the expander renderer for Presentity.
 *                and Heap.
 * PRE          : /
 */
static void expand_cell_data_func (GtkTreeViewColumn *column,
                                   GtkCellRenderer *renderer,
                                   GtkTreeModel *model,
                                   GtkTreeIter *iter,
                                   gpointer data);


/* DESCRIPTION  : Called when a new cluster has been added
 * BEHAVIOR     : Visits the cluster's heaps, and add them to the view
 * PRE          : /
 */
static void on_cluster_added (Ekiga::ClusterPtr cluster,
			      gpointer data);


/* DESCRIPTION  : Called when visiting a new cluster
 * BEHAVIOR     : Adds in the cluster heaps
 * PRE          : /
 */
static bool visit_heaps (Ekiga::HeapPtr heap,
			 Ekiga::ClusterPtr cluster,
			 gpointer data);

/* DESCRIPTION  : Called when the or heap_added signal has been emitted
 * BEHAVIOR     : Add or Update the Heap in the GtkTreeView.
 * PRE          : /
 */
static void on_heap_added (Ekiga::ClusterPtr cluster,
			   Ekiga::HeapPtr heap,
			   gpointer data);

/* DESCRIPTION  : Called when the heap_updated signal has been emitted
 * BEHAVIOR     : Add or Update the Heap in the GtkTreeView.
 * PRE          : /
 */
static void on_heap_updated (Ekiga::ClusterPtr cluster,
			     Ekiga::HeapPtr heap,
			     gpointer data);


/* DESCRIPTION  : Called when the heap_removed signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Removes the Heap from the GtkTreeView. All children,
 *                ie associated Presentity entities are also removed from
 *                the view.
 * PRE          : /
 */
static void on_heap_removed (Ekiga::ClusterPtr cluster,
			     Ekiga::HeapPtr heap,
			     gpointer data);


/* DESCRIPTION  : Called when visiting a new heap
 * BEHAVIOR     : Adds in the heap presentities
 * PRE          : /
 */
static bool visit_presentities (Ekiga::PresentityPtr presentity,
				Ekiga::ClusterPtr cluster,
				Ekiga::HeapPtr heap,
				gpointer data);


/* DESCRIPTION  : Called when the presentity_added signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Add the given Presentity into the Heap on which it was
 *                added.
 * PRE          : A valid Heap.
 */
static void on_presentity_added (Ekiga::ClusterPtr cluster,
				 Ekiga::HeapPtr heap,
				 Ekiga::PresentityPtr presentity,
				 gpointer data);


/* DESCRIPTION  : Called when the presentity_updated signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Update the given Presentity into the Heap.
 * PRE          : A valid Heap.
 */
static void on_presentity_updated (Ekiga::ClusterPtr cluster,
				   Ekiga::HeapPtr heap,
				   Ekiga::PresentityPtr presentity,
				   gpointer data);


/* DESCRIPTION  : Called when the presentity_removed signal has been emitted
 *                by the SignalCentralizer of the BookViewGtk.
 * BEHAVIOR     : Remove the given Presentity from the given Heap.
 * PRE          : A valid Heap.
 */
static void on_presentity_removed (Ekiga::ClusterPtr cluster,
				   Ekiga::HeapPtr heap,
				   Ekiga::PresentityPtr presentity,
				   gpointer data);


/* DESCRIPTION  : Called when the PresenceCore has a form request to handle
 * BEHAVIOR     : Runs the form request in gtk+
 * PRE          : The given pointer is the roster view widget
 */
static bool on_handle_questions (Ekiga::FormRequest *request,
				 gpointer data);


/*
 * Static helper functions
 */

/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given Heap.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_heap (RosterViewGtk *view,
                                                Ekiga::HeapPtr heap,
                                                GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given group name
 *                in the given Heap.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_group (RosterViewGtk *view,
						 Ekiga::HeapPtr heap,
                                                 GtkTreeIter *heap_iter,
                                                 const std::string name,
                                                 GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Update the iter parameter so that it points to
 *                the GtkTreeIter corresponding to the given presentity
 *                in the given group.
 * PRE          : /
 */
static void roster_view_gtk_find_iter_for_presentity (RosterViewGtk *view,
                                                      GtkTreeIter *group_iter,
                                                      Ekiga::PresentityPtr presentity,
                                                      GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Do a clean up in the RosterViewGtk to clean all empty groups
 *                from the view. It also folds or unfolds groups following
 *                the value of the appropriate GMConf key.
 * PRE          : /
 */
static void roster_view_gtk_update_groups (RosterViewGtk *view,
                                           GtkTreeIter *heap_iter);



/* Implementation of the helpers */

static void
remove_child (GtkWidget* child,
	      GtkWidget* container)
{
  gtk_container_remove (GTK_CONTAINER (container), child);
}

static void
on_clicked_show_heap_menu (Ekiga::HeapPtr heap,
			   GdkEventButton* event)
{
  MenuBuilderGtk builder;
  heap->populate_menu (builder);
  if (!builder.empty ()) {

    gtk_widget_show_all (builder.menu);
    gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
		    NULL, NULL, event->button, event->time);
    g_signal_connect (G_OBJECT (builder.menu), "hide",
		      G_CALLBACK (g_object_unref),
		      (gpointer) builder.menu);
  }
  g_object_ref_sink (G_OBJECT (builder.menu));
}

static void
on_clicked_show_heap_group_menu (Ekiga::HeapPtr heap,
				 const std::string name,
				 GdkEventButton* event)
{
  MenuBuilderGtk builder;
  heap->populate_menu_for_group (name, builder);
  if (!builder.empty ()) {

    gtk_widget_show_all (builder.menu);
    gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
		    NULL, NULL, event->button, event->time);
    g_signal_connect (G_OBJECT (builder.menu), "hide",
		      G_CALLBACK (g_object_unref),
		      (gpointer) builder.menu);
  }
  g_object_ref_sink (G_OBJECT (builder.menu));
}

static void
on_clicked_show_presentity_menu (Ekiga::HeapPtr heap,
				 Ekiga::PresentityPtr presentity,
				 GdkEventButton* event)
{
  Ekiga::TemporaryMenuBuilder temp;
  MenuBuilderGtk builder;

  heap->populate_menu (temp);
  presentity->populate_menu (builder);

  if (!temp.empty ()) {

    builder.add_separator ();
    temp.populate_menu (builder);
  }

  if (!builder.empty ()) {
    gtk_widget_show_all (builder.menu);
    gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
		    NULL, NULL, event->button, event->time);
    g_signal_connect (G_OBJECT (builder.menu), "hide",
		      G_CALLBACK (g_object_unref),
		      (gpointer) builder.menu);
  }
  g_object_ref_sink (G_OBJECT (builder.menu));
}

static void
on_clicked_fold (RosterViewGtk* self,
		 GtkTreePath* path,
		 const gchar* name)
{
  gboolean row_expanded = TRUE;
  GSList* existing_group = NULL;

  row_expanded
    = gtk_tree_view_row_expanded (GTK_TREE_VIEW (self->priv->tree_view), path);

  existing_group = g_slist_find_custom (self->priv->folded_groups,
					name,
					(GCompareFunc) g_ascii_strcasecmp);
  if (!row_expanded) {

    if (existing_group == NULL) {
      self->priv->folded_groups = g_slist_append (self->priv->folded_groups,
						  g_strdup (name));
    }
  }
  else {

    if (existing_group != NULL) {

      self->priv->folded_groups
	= g_slist_remove_link (self->priv->folded_groups, existing_group);

      g_free ((gchar *) existing_group->data);
      g_slist_free_1 (existing_group);
    }
  }

  gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/contacts/roster_folded_groups",
			   self->priv->folded_groups);
}

static void
on_clicked_trigger_presentity (Ekiga::PresentityPtr presentity)
{
  Ekiga::TriggerMenuBuilder builder;

  presentity->populate_menu (builder);
}

static void
update_offline_count (RosterViewGtk* self,
		      GtkTreeIter* iter)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter loop_iter;
  gint total = 0;
  gint offline_count = 0;
  gint column_type;
  Ekiga::Presentity* presentity = NULL;
  gchar *size = NULL;

  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &loop_iter, iter, 0)) {

    do {

      gtk_tree_model_get (model, &loop_iter,
			  COLUMN_TYPE, &column_type,
			  COLUMN_PRESENTITY, &presentity,
			  -1);
      if (column_type == TYPE_PRESENTITY
	  && (presentity->get_presence () == "offline"
	      || presentity->get_presence () == "unknown"))
	offline_count++;
    } while (gtk_tree_model_iter_next (model, &loop_iter));
  }

  total = gtk_tree_model_iter_n_children (model, iter);
  size = g_strdup_printf ("(%d/%d)", total - offline_count, total);
  gtk_tree_store_set (GTK_TREE_STORE (model), iter,
		      COLUMN_GROUP_SIZE, size,
		      -1);
  g_free (size);

}

static void
show_offline_contacts_changed_nt (G_GNUC_UNUSED gpointer id,
                                  GmConfEntry *entry,
                                  gpointer data)
{
  RosterViewGtk *self = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter heap_iter;
  GtkTreeIter iter;
  gboolean show_offline_contacts = false;

  g_return_if_fail (data != NULL);

  self = ROSTER_VIEW_GTK (data);

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    show_offline_contacts = gm_conf_entry_get_bool (entry);
    if (show_offline_contacts != self->priv->show_offline_contacts) {

      self->priv->show_offline_contacts = show_offline_contacts;

      /* beware: model is filtered here */
      model = gtk_tree_view_get_model (self->priv->tree_view);
      gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (model));

      /* beware: model is unfiltered here */
      model = GTK_TREE_MODEL (self->priv->store);
      if (gtk_tree_model_get_iter_first (model, &heap_iter)) {

	do {

	  if (gtk_tree_model_iter_nth_child (model, &iter, &heap_iter, 0)) {

	    do {

	      update_offline_count (self, &iter);
	    } while (gtk_tree_model_iter_next (model, &iter));
	  }
	} while (gtk_tree_model_iter_next (model, &heap_iter));
      }
    }
  }
}


static void
on_selection_changed (GtkTreeSelection* selection,
		      gpointer data)
{
  RosterViewGtk* self = NULL;
  GtkTreeModel* model = NULL;
  GtkTreeIter iter;

  self = ROSTER_VIEW_GTK (data);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gint column_type;
    gchar* name = NULL;
    Ekiga::Heap* heap = NULL;
    Ekiga::Presentity *presentity = NULL;
    gtk_tree_model_get (model, &iter,
			COLUMN_NAME, &name,
			COLUMN_TYPE, &column_type,
			COLUMN_HEAP, &heap,
			COLUMN_PRESENTITY, &presentity,
			-1);
    gtk_container_foreach (GTK_CONTAINER (self->priv->toolbar),
			   (GtkCallback)remove_child, self->priv->toolbar);

    switch (column_type) {

    case TYPE_PRESENTITY: {

      //ToolbarBuilderGtk builder(self->priv->toolbar);
      //Ekiga::ShortMenuBuilder shorter(builder);
      //presentity->populate_menu (shorter);
      //gtk_widget_show_all (self->priv->toolbar);
      g_signal_emit (self, signals[PRESENTITY_SELECTED_SIGNAL], 0, presentity);
      g_signal_emit (self, signals[HEAP_SELECTED_SIGNAL], 0, NULL);
      g_signal_emit (self, signals[HEAP_GROUP_SELECTED_SIGNAL], 0, NULL, NULL);
      break;
    }
    case TYPE_HEAP: {

      //ToolbarBuilderGtk builder(self->priv->toolbar);
      //Ekiga::ShortMenuBuilder shorter(builder);
      //heap->populate_menu (shorter);
      //gtk_widget_show_all (self->priv->toolbar);
      g_signal_emit (self, signals[PRESENTITY_SELECTED_SIGNAL], 0, NULL);
      g_signal_emit (self, signals[HEAP_SELECTED_SIGNAL], 0, heap);
      g_signal_emit (self, signals[HEAP_GROUP_SELECTED_SIGNAL], 0, NULL, NULL);
      break;
    }

    case TYPE_GROUP: {

      //ToolbarBuilderGtk builder(self->priv->toolbar);
      //Ekiga::ShortMenuBuilder shorter(builder);
      //heap->populate_menu_for_group (name, shorter);
      //gtk_widget_show_all (self->priv->toolbar);
      g_signal_emit (self, signals[PRESENTITY_SELECTED_SIGNAL], 0, NULL);
      g_signal_emit (self, signals[HEAP_SELECTED_SIGNAL], 0, NULL);
      g_signal_emit (self, signals[HEAP_GROUP_SELECTED_SIGNAL], 0, heap, name);
      break;
    }
    default:
      break;
    }

    g_free (name);
  } else {

    g_signal_emit (self, signals[PRESENTITY_SELECTED_SIGNAL], 0, NULL);
    g_signal_emit (self, signals[HEAP_SELECTED_SIGNAL], 0, NULL);
    g_signal_emit (self, signals[HEAP_GROUP_SELECTED_SIGNAL], 0, NULL, NULL);
  }
}

static gint
on_view_event_after (GtkWidget *tree_view,
		     GdkEventButton *event,
		     gpointer data)
{
  RosterViewGtk *self = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  if (event->type != GDK_BUTTON_PRESS && event->type != GDK_2BUTTON_PRESS)
    return FALSE;

  self = ROSTER_VIEW_GTK (data);
  model = gtk_tree_view_get_model (self->priv->tree_view);

  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
				     (gint) event->x, (gint) event->y,
				     &path, NULL, NULL, NULL)) {

    if (gtk_tree_model_get_iter (model, &iter, path)) {

      gint column_type;
      gchar *name = NULL;
      Ekiga::Heap *heap = NULL;
      Ekiga::Presentity *presentity = NULL;
      gtk_tree_model_get (model, &iter,
			  COLUMN_NAME, &name,
			  COLUMN_TYPE, &column_type,
			  COLUMN_HEAP, &heap,
			  COLUMN_PRESENTITY, &presentity,
			  -1);

      switch (column_type) {

      case TYPE_HEAP:

	if (event->type == GDK_BUTTON_PRESS && event->button == 1 && name)
	  on_clicked_fold (self, path, name);
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	  on_clicked_show_heap_menu (Ekiga::HeapPtr(heap), event);
	break;
      case TYPE_GROUP:

	if (event->type == GDK_BUTTON_PRESS && event->button == 1 && name)
	  on_clicked_fold (self, path, name);
	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	  on_clicked_show_heap_group_menu (Ekiga::HeapPtr(heap),
					   name, event);
	break;
      case TYPE_PRESENTITY:

	if (event->type == GDK_BUTTON_PRESS && event->button == 3)
	  on_clicked_show_presentity_menu (Ekiga::HeapPtr(heap),
					   Ekiga::PresentityPtr(presentity),
					   event);
	if (event->type == GDK_2BUTTON_PRESS)
	  on_clicked_trigger_presentity (Ekiga::PresentityPtr(presentity));
	break;
      default:

	g_assert_not_reached ();
	break; // shouldn't happen
      }
      g_free (name);
    }
    gtk_tree_path_free (path);
  }

  return TRUE;
}

static gboolean
presentity_hide_show_offline (RosterViewGtk* self,
			      GtkTreeModel* model,
			      GtkTreeIter* iter)
{
  gboolean result = FALSE;

  if (self->priv->show_offline_contacts)
    result = TRUE;
  else
    gtk_tree_model_get (model, iter,
			COLUMN_OFFLINE, &result,
			-1);

  return result;
}

static gboolean
group_hide_show_offline (RosterViewGtk* self,
			 GtkTreeModel* model,
			 GtkTreeIter* iter)
{
  gboolean result;
  GtkTreeIter child_iter;

  if (self->priv->show_offline_contacts)
    result = TRUE;
  else {

    if (gtk_tree_model_iter_nth_child (model, &child_iter, iter, 0)) {

      do {

	gtk_tree_model_get (model, &child_iter,
			    COLUMN_OFFLINE, &result,
			    -1);
      } while (!result && gtk_tree_model_iter_next (model, &child_iter));
    }
  }

  return result;
}

static gboolean
heap_hide_show_offline (G_GNUC_UNUSED RosterViewGtk* self,
			G_GNUC_UNUSED GtkTreeModel* model,
			G_GNUC_UNUSED GtkTreeIter* iter)
{
  gboolean result;

  result = TRUE;//FIXME for 548750: gtk_tree_model_iter_has_child (model, iter);

  return result;
}

static gboolean
tree_model_filter_hide_show_offline (GtkTreeModel *model,
				     GtkTreeIter *iter,
				     gpointer data)
{
  gboolean result = FALSE;
  RosterViewGtk *self = NULL;
  gint column_type;

  self = ROSTER_VIEW_GTK (data);

  gtk_tree_model_get (model, iter,
		      COLUMN_TYPE, &column_type,
		      -1);

  switch (column_type) {

  case TYPE_PRESENTITY:

    result = presentity_hide_show_offline (self, model, iter);
    break;

  case TYPE_GROUP:

    result = group_hide_show_offline (self, model, iter);
    break;

  case TYPE_HEAP:

    result = heap_hide_show_offline (self, model, iter);
    break;

  default:
    result = TRUE;
  }

  return result;
}


static void
show_cell_data_func (GtkTreeViewColumn * /*column*/,
		     GtkCellRenderer *renderer,
		     GtkTreeModel *model,
		     GtkTreeIter *iter,
		     gpointer data)
{
  gint column_type;

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == GPOINTER_TO_INT (data))
    g_object_set (renderer, "visible", TRUE, NULL);
  else
    g_object_set (renderer, "visible", FALSE, NULL);

}


static void
expand_cell_data_func (GtkTreeViewColumn *column,
                       GtkCellRenderer *renderer,
                       GtkTreeModel *model,
                       GtkTreeIter *iter,
                       gpointer /*data*/)
{
  GtkTreePath *path = NULL;
  gint column_type;
  gboolean row_expanded = FALSE;

  path = gtk_tree_model_get_path (model, iter);
  row_expanded = gtk_tree_view_row_expanded (GTK_TREE_VIEW (column->tree_view), path);
  gtk_tree_path_free (path);

  gtk_tree_model_get (model, iter, COLUMN_TYPE, &column_type, -1);

  if (column_type == TYPE_PRESENTITY || column_type == TYPE_HEAP)
    g_object_set (renderer, "visible", FALSE, NULL);
  else
    g_object_set (renderer, "visible", TRUE, NULL);

  g_object_set (renderer,
                "expander-style", row_expanded ? GTK_EXPANDER_EXPANDED : GTK_EXPANDER_COLLAPSED,
                NULL);
}

static void
on_cluster_added (Ekiga::ClusterPtr cluster,
		  gpointer data)
{
  cluster->visit_heaps (sigc::bind (sigc::ptr_fun (visit_heaps),
				    cluster, data));
}

static bool
visit_heaps (Ekiga::HeapPtr heap,
	     Ekiga::ClusterPtr cluster,
	     gpointer data)
{
  on_heap_updated (cluster, heap, data);
  heap->visit_presentities (sigc::bind (sigc::ptr_fun (visit_presentities), cluster, heap, data));

  return true;
}

static void
on_heap_added (Ekiga::ClusterPtr cluster,
	       Ekiga::HeapPtr heap,
	       gpointer data)
{
  on_heap_updated (cluster, heap, data);
  heap->visit_presentities (sigc::bind (sigc::ptr_fun (visit_presentities), cluster, heap, data));
}

static void
on_heap_updated (Ekiga::ClusterPtr /*cluster*/,
		 Ekiga::HeapPtr heap,
		 gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &iter);

  gtk_tree_store_set (self->priv->store, &iter,
		      COLUMN_TYPE, TYPE_HEAP,
		      COLUMN_HEAP, heap.get (),
		      COLUMN_NAME, heap->get_name ().c_str (),
		      -1);
  gtk_tree_view_expand_all (self->priv->tree_view);
}


static void
on_heap_removed (Ekiga::ClusterPtr /*cluster*/,
		 Ekiga::HeapPtr heap,
		 gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &iter);

  gtk_tree_store_remove (self->priv->store, &iter);
}


static bool
visit_presentities (Ekiga::PresentityPtr presentity,
		    Ekiga::ClusterPtr cluster,
		    Ekiga::HeapPtr heap,
		    gpointer data)
{
  on_presentity_added (cluster, heap, presentity, data);

  return true;
}

static void
on_presentity_added (Ekiga::ClusterPtr /*cluster*/,
		     Ekiga::HeapPtr heap,
		     Ekiga::PresentityPtr presentity,
		     gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeIter heap_iter;
  std::set<std::string> groups = presentity->get_groups ();
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  bool active = false;
  bool away = false;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  active = presentity->get_presence () != "offline";
  away = presentity->get_presence () == "away";

  for (std::set<std::string>::const_iterator group = groups.begin ();
       group != groups.end ();
       group++) {

    roster_view_gtk_find_iter_for_group (self, heap, &heap_iter,
					 *group, &group_iter);
    roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);

    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_OFFLINE, active,
			COLUMN_HEAP, heap.get (),
			COLUMN_PRESENTITY, presentity.get (),
			COLUMN_NAME, presentity->get_name ().c_str (),
			COLUMN_STATUS, presentity->get_status ().c_str (),
			COLUMN_PRESENCE, presentity->get_presence ().c_str (),
			COLUMN_ACTIVE, (!active || away) ? "gray" : "black",
			-1);
  }

  if (groups.empty ()) {

    roster_view_gtk_find_iter_for_group (self, heap, &heap_iter,
					 _("Unsorted"), &group_iter);
    roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
    gtk_tree_store_set (self->priv->store, &iter,
			COLUMN_TYPE, TYPE_PRESENTITY,
			COLUMN_OFFLINE, active,
			COLUMN_HEAP, heap.get (),
			COLUMN_PRESENTITY, presentity.get (),
			COLUMN_NAME, presentity->get_name ().c_str (),
			COLUMN_STATUS, presentity->get_status ().c_str (),
			COLUMN_PRESENCE, presentity->get_presence ().c_str (),
			COLUMN_ACTIVE, active ? "black" : "gray",
			-1);
  }

  roster_view_gtk_update_groups (self, &heap_iter);
}


static void
on_presentity_updated (Ekiga::ClusterPtr cluster,
		       Ekiga::HeapPtr heap,
		       Ekiga::PresentityPtr presentity,
		       gpointer data)
{
  RosterViewGtk *self = (RosterViewGtk *)data;
  GtkTreeModel *model;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;
  gchar *group_name = NULL;
  std::set<std::string> groups = presentity->get_groups ();

  model = GTK_TREE_MODEL (self->priv->store);

  if (groups.empty ())
    groups.insert (_("Unsorted"));

  // This makes sure we are in all groups where we should
  on_presentity_added (cluster, heap, presentity, data);

  // Now let's remove from all the others
  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);

  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      gtk_tree_model_get (model, &group_iter,
			  COLUMN_NAME, &group_name,
			  -1);
      if (group_name != NULL) {

	if (groups.find (group_name) == groups.end ()) {

	  roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
	  gtk_tree_store_remove (self->priv->store, &iter);
	}
	g_free (group_name);
      }
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  roster_view_gtk_update_groups (self, &heap_iter);
}


static void
on_presentity_removed (Ekiga::ClusterPtr /*cluster*/,
		       Ekiga::HeapPtr heap,
		       Ekiga::PresentityPtr presentity,
		       gpointer data)
{
  RosterViewGtk *self = ROSTER_VIEW_GTK (data);
  GtkTreeModel *model = NULL;
  GtkTreeIter heap_iter;
  GtkTreeIter group_iter;
  GtkTreeIter iter;

  roster_view_gtk_find_iter_for_heap (self, heap, &heap_iter);
  model = GTK_TREE_MODEL (self->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &group_iter, &heap_iter, 0)) {

    do {

      roster_view_gtk_find_iter_for_presentity (self, &group_iter, presentity, &iter);
      gtk_tree_store_remove (self->priv->store, &iter);
    } while (gtk_tree_model_iter_next (model, &group_iter));
  }

  roster_view_gtk_update_groups (self, &heap_iter);
}

static bool
on_handle_questions (Ekiga::FormRequest *request,
		     gpointer data)
{
  GtkWidget *parent = gtk_widget_get_toplevel (GTK_WIDGET (data));
  FormDialog dialog (*request, parent);

  dialog.run ();

  return true;
}


/*
 * Implementation of the static helpers.
 */
static void
roster_view_gtk_find_iter_for_heap (RosterViewGtk *view,
                                    Ekiga::HeapPtr heap,
                                    GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Heap *iter_heap = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_get_iter_first (model, iter)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_HEAP, &iter_heap, -1);
      if (iter_heap == heap.get ())
	found = TRUE;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found)
    gtk_tree_store_append (view->priv->store, iter, NULL);
}


static void
roster_view_gtk_find_iter_for_group (RosterViewGtk *view,
				     Ekiga::HeapPtr heap,
                                     GtkTreeIter *heap_iter,
                                     const std::string name,
                                     GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  gchar *group_name = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, iter, heap_iter, 0)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_NAME, &group_name, -1);
      if (group_name != NULL && name == group_name)
        found = TRUE;
      if (group_name != NULL)
        g_free (group_name);
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found) {

    gtk_tree_store_append (view->priv->store, iter, heap_iter);
    gtk_tree_store_set (view->priv->store, iter,
                        COLUMN_TYPE, TYPE_GROUP,
			COLUMN_HEAP, heap.get (),
                        COLUMN_NAME, name.c_str (),
                        -1);
  }
}


static void
roster_view_gtk_find_iter_for_presentity (RosterViewGtk *view,
                                          GtkTreeIter *group_iter,
                                          Ekiga::PresentityPtr presentity,
                                          GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  Ekiga::Presentity *iter_presentity = NULL;
  gboolean found = FALSE;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, iter, group_iter, 0)) {

    do {

      gtk_tree_model_get (model, iter, COLUMN_PRESENTITY, &iter_presentity, -1);
      if (iter_presentity == presentity.get ())
	found = TRUE;
    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  if (!found)
    gtk_tree_store_append (view->priv->store, iter, group_iter);
}


static void
roster_view_gtk_update_groups (RosterViewGtk *view,
                               GtkTreeIter *heap_iter)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;

  GSList *existing_group = NULL;

  gboolean go_on = FALSE;
  gchar *name = NULL;

  model = GTK_TREE_MODEL (view->priv->store);

  if (gtk_tree_model_iter_nth_child (model, &iter, heap_iter, 0)) {

    do {

      // If this node has children, see if it must be
      // folded or unfolded
      if (gtk_tree_model_iter_has_child (model, &iter)) {


	update_offline_count (view, &iter);
        gtk_tree_model_get (model, &iter,
                            COLUMN_NAME, &name, -1);
        if (name) {

          if (view->priv->folded_groups)
            existing_group = g_slist_find_custom (view->priv->folded_groups,
                                                  name,
                                                  (GCompareFunc) g_ascii_strcasecmp);


          path = gtk_tree_model_get_path (model, heap_iter);
          gtk_tree_view_expand_row (view->priv->tree_view, path, FALSE);
          gtk_tree_path_free (path);

          path = gtk_tree_model_get_path (model, &iter);
          if (path) {

            if (existing_group == NULL)
              gtk_tree_view_expand_row (view->priv->tree_view, path, FALSE);
            else
              gtk_tree_view_collapse_row (view->priv->tree_view, path);

            gtk_tree_path_free (path);
          }

          go_on = gtk_tree_model_iter_next (model, &iter);
        }

        g_free (name);
      }
      // else remove the node (no children)
      else {

        go_on = gtk_tree_store_remove (view->priv->store, &iter);
      }
    } while (go_on);
  }
}


/*
 * GObject stuff
 */
static void
roster_view_gtk_dispose (GObject *obj)
{
  RosterViewGtk *view = NULL;

  view = ROSTER_VIEW_GTK (obj);

  for (std::vector<sigc::connection>::iterator iter
	 = view->priv->connections.begin ();
       iter != view->priv->connections.end ();
       iter++)
    iter->disconnect ();

  if (view->priv->tree_view) {

    g_signal_handlers_disconnect_matched (gtk_tree_view_get_selection (view->priv->tree_view),
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark) 0, /* detail */
					  NULL,	/* closure */
					  NULL,	/* func */
					  view); /* data */
    //   gtk_tree_store_clear (view->priv->store);

    g_slist_foreach (view->priv->folded_groups, (GFunc) g_free, NULL);
    g_slist_free (view->priv->folded_groups);
    view->priv->folded_groups = NULL;

    view->priv->store = NULL;
    view->priv->tree_view = NULL;
  }

  parent_class->dispose (obj);
}


static void
roster_view_gtk_finalize (GObject *obj)
{
  RosterViewGtk *view = NULL;

  view = (RosterViewGtk *)obj;

  delete view->priv;

  parent_class->finalize (obj);
}


static void
roster_view_gtk_class_init (gpointer g_class,
			    gpointer /*class_data*/)
{
  RosterViewGtkClass* roster_view_gtk_class = NULL;
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = roster_view_gtk_dispose;
  gobject_class->finalize = roster_view_gtk_finalize;

  signals[PRESENTITY_SELECTED_SIGNAL] =
    g_signal_new ("presentity-selected",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (RosterViewGtkClass, presentity_selected),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1,
		  G_TYPE_POINTER);

  signals[HEAP_SELECTED_SIGNAL] =
    g_signal_new ("heap-selected",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (RosterViewGtkClass, heap_selected),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1,
		  G_TYPE_POINTER);

  signals[HEAP_GROUP_SELECTED_SIGNAL] =
    g_signal_new ("heap-group-selected",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (RosterViewGtkClass, heap_group_selected),
		  NULL, NULL,
		  gm_marshal_VOID__POINTER_STRING,
		  G_TYPE_NONE, 2,
		  G_TYPE_POINTER,
		  G_TYPE_STRING);

  /* FIXME: is it useful? */
  roster_view_gtk_class = (RosterViewGtkClass*)g_class;
  roster_view_gtk_class->presentity_selected = NULL;
  roster_view_gtk_class->heap_selected = NULL;
  roster_view_gtk_class->heap_group_selected = NULL;
}


GType
roster_view_gtk_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (RosterViewGtkClass),
      NULL,
      NULL,
      roster_view_gtk_class_init,
      NULL,
      NULL,
      sizeof (RosterViewGtk),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_FRAME,
				     "RosterViewGtk",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/*
 * Public API
 */
GtkWidget *
roster_view_gtk_new (Ekiga::PresenceCore &core)
{
  RosterViewGtk *self = NULL;

  sigc::connection conn;

  GtkTreeModel *filtered = NULL;
  GtkTreeSelection *selection = NULL;
  GtkTreeViewColumn *col = NULL;
  GtkCellRenderer *renderer = NULL;

  self = (RosterViewGtk *) g_object_new (ROSTER_VIEW_GTK_TYPE, NULL);

  self->priv = new _RosterViewGtkPrivate (core);

  self->priv->folded_groups = gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/contacts/roster_folded_groups");
  self->priv->show_offline_contacts = gm_conf_get_bool ("/apps/" PACKAGE_NAME "/contacts/show_offline_contacts");
  self->priv->vbox = gtk_vbox_new (FALSE, 0);
  self->priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  self->priv->toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (self->priv->toolbar), GTK_TOOLBAR_BOTH);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->vbox), 0);
  gtk_container_set_border_width (GTK_CONTAINER (self->priv->scrolled_window), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (self), GTK_SHADOW_NONE);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (self->priv->scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  self->priv->store = gtk_tree_store_new (COLUMN_NUMBER,
                                          G_TYPE_INT,         // type
                                          G_TYPE_POINTER,     // heap
                                          G_TYPE_POINTER,     // presentity
                                          G_TYPE_STRING,      // name
                                          G_TYPE_STRING,      // status
                                          G_TYPE_STRING,      // presence
                                          G_TYPE_STRING,      // color if active
					  G_TYPE_STRING,      // group size
					  G_TYPE_BOOLEAN);    // offline

  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (self->priv->store),
                                        COLUMN_NAME, GTK_SORT_ASCENDING);
  filtered = gtk_tree_model_filter_new (GTK_TREE_MODEL (self->priv->store),
					NULL);
  g_object_unref (self->priv->store);
  self->priv->tree_view =
    GTK_TREE_VIEW (gtk_tree_view_new_with_model (filtered));
  g_object_unref (filtered);
  gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filtered),
					  tree_model_filter_hide_show_offline,
					  self, NULL);

  gtk_tree_view_set_headers_visible (self->priv->tree_view, FALSE);

  gtk_container_add (GTK_CONTAINER (self), GTK_WIDGET (self->priv->vbox));
  gtk_box_pack_start (GTK_BOX (self->priv->vbox),
		      GTK_WIDGET (self->priv->scrolled_window), TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (self->priv->vbox),
		      self->priv->toolbar, FALSE, TRUE, 2);
  gtk_container_add (GTK_CONTAINER (self->priv->scrolled_window),
		     GTK_WIDGET (self->priv->tree_view));

  /* Build the GtkTreeView */
  // We hide the normal GTK+ expanders and use our own
  col = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  g_object_set (col, "visible", FALSE, NULL);
  gtk_tree_view_append_column (self->priv->tree_view, col);
  gtk_tree_view_set_expander_column (self->priv->tree_view, col);

  col = gtk_tree_view_column_new ();
  renderer = gm_cell_renderer_expander_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  g_object_set (renderer,
                "xalign", 0,
                "xpad", 0,
                "ypad", 0,
                "visible", TRUE,
                "expander-style", GTK_EXPANDER_COLLAPSED,
                NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer, expand_cell_data_func, NULL, NULL);
  gtk_tree_view_append_column (self->priv->tree_view, col);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_NAME);
  gtk_tree_view_column_set_alignment (col, 0.0);
  g_object_set (renderer, "xalign", 0.5, "ypad", 0, NULL);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_HEAP), NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "text", COLUMN_NAME);
  g_object_set (renderer, "weight", PANGO_WEIGHT_BOLD, NULL);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_GROUP), NULL);

  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (renderer, "yalign", 0.5, "xpad", 5, NULL);
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer,
				      "stock-id",
				      COLUMN_PRESENCE);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  renderer = gm_cell_renderer_bitext_new ();
  gtk_tree_view_column_pack_start (col, renderer, FALSE);
  gtk_tree_view_column_add_attribute (col, renderer, "primary-text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (col, renderer, "secondary-text", COLUMN_STATUS);
  gtk_tree_view_column_add_attribute (col, renderer, "foreground", COLUMN_ACTIVE);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
                                           show_cell_data_func, GINT_TO_POINTER (TYPE_PRESENTITY), NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_set_spacing (col, 0);
  gtk_tree_view_column_pack_start (col, renderer, TRUE);
  gtk_tree_view_column_add_attribute (col, renderer, "text", COLUMN_GROUP_SIZE);
  gtk_tree_view_column_set_alignment (col, 0.0);
  gtk_tree_view_column_set_cell_data_func (col, renderer,
					   show_cell_data_func,
					   GINT_TO_POINTER (TYPE_GROUP), NULL);

  /* Callback when the selection has been changed */
  selection = gtk_tree_view_get_selection (self->priv->tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (selection), "changed",
		    G_CALLBACK (on_selection_changed), self);
  g_signal_connect (G_OBJECT (self->priv->tree_view), "event-after",
		    G_CALLBACK (on_view_event_after), self);


  /* Relay signals */
  conn = core.cluster_added.connect (sigc::bind (sigc::ptr_fun (on_cluster_added),
 						 (gpointer) self));
  self->priv->connections.push_back (conn);
  conn = core.heap_added.connect (sigc::bind (sigc::ptr_fun (on_heap_added),
 					      (gpointer) self));
  self->priv->connections.push_back (conn);
  conn = core.heap_updated.connect (sigc::bind (sigc::ptr_fun (on_heap_updated),
 						(gpointer) self));
  self->priv->connections.push_back (conn);
  conn = core.heap_removed.connect (sigc::bind (sigc::ptr_fun (on_heap_removed),
 						(gpointer) self));

  self->priv->connections.push_back (conn);
  conn = core.presentity_added.connect (sigc::bind (sigc::ptr_fun (on_presentity_added),
 						    (gpointer) self));
  self->priv->connections.push_back (conn);
  conn = core.presentity_updated.connect (sigc::bind (sigc::ptr_fun (on_presentity_updated),
 						      self));
  self->priv->connections.push_back (conn);
  conn = core.presentity_removed.connect (sigc::bind (sigc::ptr_fun (on_presentity_removed),
 						      (gpointer) self));
  self->priv->connections.push_back (conn);
  conn = core.questions.add_handler (sigc::bind (sigc::ptr_fun (on_handle_questions), (gpointer) self));
  self->priv->connections.push_back (conn);

  core.visit_clusters (sigc::bind_return (sigc::bind (sigc::ptr_fun (on_cluster_added), (gpointer) self), true));

  /* Notifiers */
  gm_conf_notifier_add ("/apps/" PACKAGE_NAME "/contacts/show_offline_contacts",
			show_offline_contacts_changed_nt, self);

  return (GtkWidget *) self;
}



/* FIXME: stupid, stupid! */
static void
gm_marshal_VOID__POINTER_STRING (GClosure     *closure,
                                 GValue       *return_value G_GNUC_UNUSED,
                                 guint         n_param_values,
                                 const GValue *param_values,
                                 gpointer      invocation_hint G_GNUC_UNUSED,
                                 gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__POINTER_STRING) (gpointer     data1,
                                                     gpointer     arg_1,
                                                     gpointer     arg_2,
                                                     gpointer     data2);
  GMarshalFunc_VOID__POINTER_STRING callback;
  GCClosure *cc = (GCClosure*) closure;
  gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__POINTER_STRING) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            (param_values + 1)->data[0].v_pointer,
            (gchar*)(param_values + 2)->data[0].v_pointer,
            data2);
}
