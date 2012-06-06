
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
 *                         book-view-gtk.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt
 *   copyright            : (c) 2006 by Julien Puydt
 *   description          : implementation of an addressbook view
 *
 */


#include <algorithm>
#include <iostream>
#include <glib/gi18n.h>

#include "book-view-gtk.h"

#include "gmstockicons.h"
#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"

/*
 * The Book View
 */
struct _BookViewGtkPrivate
{
  _BookViewGtkPrivate (Ekiga::BookPtr book_) : book (book_) { }

  GtkTreeView *tree_view;
  GtkWidget *vbox;
  GtkWidget *entry;
  GtkWidget *statusbar;
  GtkWidget *scrolled_window;

  Ekiga::BookPtr book;
  std::list<sigc::connection> connections;
};


enum {

  COLUMN_CONTACT_POINTER,
  COLUMN_PIXBUF,
  COLUMN_NAME,
  COLUMN_NUMBER
};

static GObjectClass *parent_class = NULL;


/*
 * Callbacks
 */


/* DESCRIPTION  : Called when the a contact has been added in a Book.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_contact_added (Ekiga::ContactPtr contact,
			      gpointer data);


/* DESCRIPTION  : Called when the a contact has been updated in a Book.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_contact_updated (Ekiga::ContactPtr contact,
				gpointer data);


/* DESCRIPTION  : Called when the Book status has been updated.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_updated (gpointer data);


/* DESCRIPTION  : Called when the a contact has been removed from a Book.
 * BEHAVIOR     : Update the BookView.
 * PRE          : The gpointer must point to the BookViewGtk GObject.
 */
static void on_contact_removed (Ekiga::ContactPtr contact,
				gpointer data);


/* DESCRIPTION  : Called when the a contact selection has been changed.
 * BEHAVIOR     : Emits the "updated" signal on the GObject passed as
 *                second parameter..
 * PRE          : The gpointer must point to a BookViewGtk GObject.
 */
static void on_selection_changed (GtkTreeSelection * /*selection*/,
                                  gpointer data);


/* DESCRIPTION  : Called when the user activates the filter GtkEntry.
 * BEHAVIOR     : Updates the Book search filter, which triggers
 *                a refresh.
 * PRE          : A valid pointer to the BookViewGtk.
 */
static void on_entry_activated_cb (GtkWidget *entry,
                                   gpointer data);


/* DESCRIPTION  : Called when the user clicks on the Find button.
 * BEHAVIOR     : Activates the GtkEntry.
 * PRE          : A valid pointer to the BookViewGtk containing
 *                the GtkEntry to activate.
 */
static void on_button_clicked_cb (GtkWidget *button,
                                  gpointer data);


/* DESCRIPTION  : Called when the a contact is clicked.
 * BEHAVIOR     : Displays a popup menu.
 * PRE          : The gpointer must point to a BookViewGtk GObject.
 */
static gint on_contact_clicked (GtkWidget *tree_view,
                                GdkEventButton *event,
                                gpointer data);


/* Static functions */

/* DESCRIPTION  : /
 * BEHAVIOR     : Add the contact to the BookViewGtk.
 * PRE          : /
 */
static void
book_view_gtk_add_contact (BookViewGtk *self,
                           Ekiga::ContactPtr contact);


/* DESCRIPTION  : /
 * BEHAVIOR     : Updated the contact in the BookViewGtk.
 * PRE          : /
 */
static void
book_view_gtk_update_contact (BookViewGtk *self,
                              Ekiga::ContactPtr contact,
                              GtkTreeIter *iter);


/* DESCRIPTION  : /
 * BEHAVIOR     : Remove the contact from the BookViewGtk.
 * PRE          : /
 */
static void
book_view_gtk_remove_contact (BookViewGtk *self,
                              Ekiga::ContactPtr contact);


/* DESCRIPTION  : /
 * BEHAVIOR     : Return TRUE and update the GtkTreeIter if we found
 *                the iter corresponding to the Contact in the BookViewGtk.
 * PRE          : /
 */
static gboolean
book_view_gtk_find_iter_for_contact (BookViewGtk *view,
                                     Ekiga::ContactPtr contact,
                                     GtkTreeIter *iter);



/* Implementation of the callbacks */
static void
on_contact_added (Ekiga::ContactPtr contact,
		  gpointer data)
{
  book_view_gtk_add_contact (BOOK_VIEW_GTK (data), contact);
}


static void
on_contact_updated (Ekiga::ContactPtr contact,
		    gpointer data)
{
  BookViewGtk *view = NULL;
  GtkTreeIter iter;

  view = BOOK_VIEW_GTK (data);

  if (book_view_gtk_find_iter_for_contact (view, contact, &iter)) {
    book_view_gtk_update_contact (view, contact, &iter);
  }
}


static void
on_updated (gpointer data)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (data);

  std::string status = view->priv->book->get_status ();

  gtk_statusbar_pop (GTK_STATUSBAR (view->priv->statusbar), 0);
  gtk_statusbar_push (GTK_STATUSBAR (view->priv->statusbar), 0, status.c_str ());

  if (GDK_IS_WINDOW (GTK_WIDGET (view)->window))
    gdk_window_set_cursor (GTK_WIDGET (view)->window, NULL);
}



static void
on_contact_removed (Ekiga::ContactPtr contact,
		    gpointer data)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (data);
  book_view_gtk_remove_contact (view, contact);
}


static void
on_selection_changed (GtkTreeSelection * /*selection*/,
		      gpointer data)
{
  g_signal_emit_by_name (data, "updated", NULL);
}


static void
on_entry_activated_cb (GtkWidget *entry,
                       gpointer data)
{
  GdkCursor *cursor = NULL;
  const char *entry_text = gtk_entry_get_text (GTK_ENTRY (entry));

  cursor = gdk_cursor_new (GDK_WATCH);
  gdk_window_set_cursor (GTK_WIDGET (data)->window, cursor);
  gdk_cursor_unref (cursor);

  BOOK_VIEW_GTK (data)->priv->book->set_search_filter (entry_text);
}


static void
on_button_clicked_cb (G_GNUC_UNUSED GtkWidget *button,
                      gpointer data)
{
  g_return_if_fail (data != NULL);

  gtk_widget_activate (GTK_WIDGET (BOOK_VIEW_GTK (data)->priv->entry));
}


static gint
on_contact_clicked (GtkWidget *tree_view,
		    GdkEventButton *event,
		    gpointer data)
{
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;

  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree_view),
				     (gint) event->x, (gint) event->y,
				     &path, NULL, NULL, NULL)) {

    model
      = gtk_tree_view_get_model (BOOK_VIEW_GTK (data)->priv->tree_view);

    if (gtk_tree_model_get_iter (model, &iter, path)) {

      Ekiga::Contact *contact = NULL;
      gtk_tree_model_get (model, &iter,
			  COLUMN_CONTACT_POINTER, &contact,
			  -1);

      if (contact != NULL) {

	if (event->type == GDK_BUTTON_PRESS && event->button == 3) {

	  Ekiga::TemporaryMenuBuilder temp;
	  MenuBuilderGtk builder;

	  BOOK_VIEW_GTK (data)->priv->book->populate_menu (temp);
	  contact->populate_menu (builder);

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
	} else if (event->type == GDK_2BUTTON_PRESS) {

	  Ekiga::TriggerMenuBuilder builder;

	  contact->populate_menu (builder);
	}
      }
    }
    gtk_tree_path_free (path);
  }

  return TRUE;
}


/* Implementation of the static functions */
static void
book_view_gtk_add_contact (BookViewGtk *self,
                           Ekiga::ContactPtr contact)
{
  GtkTreeModel *model = NULL;
  GtkListStore *store = NULL;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (self->priv->tree_view);
  store = GTK_LIST_STORE (model);

  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter, COLUMN_CONTACT_POINTER, contact.get (), -1);
  book_view_gtk_update_contact (self, contact, &iter);
}


static void
book_view_gtk_update_contact (BookViewGtk *self,
			      Ekiga::ContactPtr contact,
			      GtkTreeIter *iter)
{
  GdkPixbuf *icon = NULL;
  GtkListStore *store = NULL;

  store = GTK_LIST_STORE (gtk_tree_view_get_model (self->priv->tree_view));
  icon = gtk_widget_render_icon (GTK_WIDGET (self->priv->tree_view),
                                 GM_STOCK_STATUS_UNKNOWN, GTK_ICON_SIZE_MENU, NULL);

  gtk_list_store_set (store, iter,
                      COLUMN_PIXBUF, icon,
		      COLUMN_NAME, contact->get_name ().c_str (),
		      -1);
  g_object_unref (icon);

  if (GDK_IS_WINDOW (GTK_WIDGET (self)->window))
    gdk_window_set_cursor (GTK_WIDGET (self)->window, NULL);
}


static void
book_view_gtk_remove_contact (BookViewGtk *self,
                              Ekiga::ContactPtr contact)
{
  GtkTreeModel *model = NULL;
  GtkListStore *store = NULL;
  GtkTreeIter iter;

  model = gtk_tree_view_get_model (self->priv->tree_view);
  store = GTK_LIST_STORE (model);

  while (book_view_gtk_find_iter_for_contact (self, contact, &iter))
    gtk_list_store_remove (store, &iter);

  if (GDK_IS_WINDOW (GTK_WIDGET (self)->window))
    gdk_window_set_cursor (GTK_WIDGET (self)->window, NULL);
}


static gboolean
book_view_gtk_find_iter_for_contact (BookViewGtk *view,
                                     Ekiga::ContactPtr contact,
                                     GtkTreeIter *iter)
{
  GtkTreeModel *model = NULL;
  gboolean found = FALSE;

  model = gtk_tree_view_get_model (view->priv->tree_view);

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), iter)) {

    do {

      Ekiga::Contact *iter_contact = NULL;
      gtk_tree_model_get (model, iter,
                          COLUMN_CONTACT_POINTER, &iter_contact,
                          -1);
      if (iter_contact == contact.get ())
        found = TRUE;

    } while (!found && gtk_tree_model_iter_next (model, iter));
  }

  return found;
}


/* GObject boilerplate code */
static void
book_view_gtk_dispose (GObject *obj)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (obj);

  for (std::list<sigc::connection>::iterator iter
	 = view->priv->connections.begin ();
       iter != view->priv->connections.end ();
       ++iter)
    iter->disconnect ();

  if (view->priv->tree_view) {

    g_signal_handlers_disconnect_matched (gtk_tree_view_get_selection (view->priv->tree_view),
					  (GSignalMatchType) G_SIGNAL_MATCH_DATA,
					  0, /* signal_id */
					  (GQuark) 0, /* detail */
					  NULL,	/* closure */
					  NULL,	/* func */
					  view); /* data */
    gtk_list_store_clear (GTK_LIST_STORE (gtk_tree_view_get_model (view->priv->tree_view)));

    view->priv->tree_view = NULL;
  }

  parent_class->dispose (obj);
}


static void
book_view_gtk_finalize (GObject *obj)
{
  BookViewGtk *view = NULL;

  view = BOOK_VIEW_GTK (obj);

  delete view->priv;

  parent_class->finalize (obj);
}


static void
book_view_gtk_class_init (gpointer g_class,
			  gpointer /*class_data*/)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *)g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *)g_class;
  gobject_class->dispose = book_view_gtk_dispose;
  gobject_class->finalize = book_view_gtk_finalize;

  g_signal_new ("updated",
                G_OBJECT_CLASS_TYPE (g_class),
                G_SIGNAL_RUN_FIRST,
                0, NULL, NULL,
                g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}


GType
book_view_gtk_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (BookViewGtkClass),
      NULL,
      NULL,
      book_view_gtk_class_init,
      NULL,
      NULL,
      sizeof (BookViewGtk),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_FRAME,
				     "BookViewGtkType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/* public methods implementation */
GtkWidget *
book_view_gtk_new (Ekiga::BookPtr book)
{
  BookViewGtk *result = NULL;

  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *button = NULL;

  GtkTreeSelection *selection = NULL;
  GtkListStore *store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;

  result = (BookViewGtk *) g_object_new (BOOK_VIEW_GTK_TYPE, NULL);

  result->priv = new _BookViewGtkPrivate (book);
  result->priv->vbox = gtk_vbox_new (FALSE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (result), GTK_SHADOW_NONE);

  /* The List Store */
  result->priv->scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW
				  (result->priv->scrolled_window),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  result->priv->tree_view = GTK_TREE_VIEW (gtk_tree_view_new ());
  gtk_tree_view_set_headers_visible (result->priv->tree_view, FALSE);
  gtk_container_add (GTK_CONTAINER (result), GTK_WIDGET (result->priv->vbox));
  gtk_box_pack_start (GTK_BOX (result->priv->vbox),
		     GTK_WIDGET (result->priv->scrolled_window), TRUE, TRUE, 0);
  gtk_container_add (GTK_CONTAINER (result->priv->scrolled_window),
		     GTK_WIDGET (result->priv->tree_view));

  selection = gtk_tree_view_get_selection (result->priv->tree_view);
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (selection, "changed",
		    G_CALLBACK (on_selection_changed), result);

  g_signal_connect (G_OBJECT (result->priv->tree_view), "event-after",
		    G_CALLBACK (on_contact_clicked), result);

  store = gtk_list_store_new (COLUMN_NUMBER,
			      G_TYPE_POINTER,
                              GDK_TYPE_PIXBUF,
                              G_TYPE_STRING);

  gtk_tree_view_set_model (result->priv->tree_view, GTK_TREE_MODEL (store));
  g_object_unref (store);

  /* Name */
  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
                                       "pixbuf", COLUMN_PIXBUF,
                                       NULL);


  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
                                       "text", COLUMN_NAME,
                                       NULL);

  gtk_tree_view_column_set_title (column, _("Full Name"));
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
  gtk_tree_view_column_set_sizing (GTK_TREE_VIEW_COLUMN (column),
                                   GTK_TREE_VIEW_COLUMN_AUTOSIZE);
  gtk_tree_view_column_set_resizable (column, true);
  gtk_tree_view_append_column (GTK_TREE_VIEW (result->priv->tree_view), column);

  /* The Search Box */
  hbox = gtk_hbox_new (FALSE, 0);
  result->priv->entry = gtk_entry_new ();
  button = gtk_button_new_from_stock (GTK_STOCK_FIND);
  label = gtk_label_new_with_mnemonic (_("_Search Filter:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), result->priv->entry, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (result->priv->vbox), hbox, FALSE, FALSE, 0);
  g_signal_connect (result->priv->entry, "activate",
                    G_CALLBACK (on_entry_activated_cb), result);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (on_button_clicked_cb), result);


  /* The status bar */
  result->priv->statusbar = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (result->priv->vbox), result->priv->statusbar, FALSE, TRUE, 0);

  /* connect to the signals */
  result->priv->connections.push_back (book->contact_added.connect (sigc::bind (sigc::ptr_fun (on_contact_added), (gpointer)result)));
  result->priv->connections.push_back (book->contact_updated.connect (sigc::bind (sigc::ptr_fun (on_contact_updated), (gpointer)result)));
  result->priv->connections.push_back (book->contact_removed.connect (sigc::bind (sigc::ptr_fun (on_contact_removed), (gpointer)result)));
  result->priv->connections.push_back (book->updated.connect (sigc::bind (sigc::ptr_fun (on_updated), (gpointer)result)));


  /* populate */
  book->visit_contacts (sigc::bind_return (sigc::bind (sigc::ptr_fun (on_contact_added), (gpointer)result),true));

  return (GtkWidget *) result;
}


void
book_view_gtk_populate_menu (BookViewGtk *self,
			     GtkWidget *menu)
{
  g_return_if_fail (IS_BOOK_VIEW_GTK (self));
  g_return_if_fail (GTK_IS_MENU (menu));

  GtkTreeSelection *selection = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  Ekiga::Contact *contact = NULL;
  GtkWidget *item = NULL;
  MenuBuilderGtk builder (menu);

  self->priv->book->populate_menu (builder);

  selection = gtk_tree_view_get_selection (self->priv->tree_view);

  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gtk_tree_model_get (model, &iter, COLUMN_CONTACT_POINTER, &contact, -1);

    if (contact) {

      item = gtk_separator_menu_item_new ();
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
      contact->populate_menu (builder);
    }
  }
}
