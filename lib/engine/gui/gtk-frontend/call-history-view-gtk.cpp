
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
 *                         call-history-view-gtk.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a call history view widget
 *
 */

#include <sstream>
#include <iostream>
#include <iomanip>
#include <glib/gi18n.h>

#include "call-history-view-gtk.h"

#include "menu-builder-tools.h"
#include "menu-builder-gtk.h"
#include "gm-cell-renderer-bitext.h"
#include "gmstockicons.h"

enum {
  COLUMN_CONTACT,
  COLUMN_PIXBUF,
  COLUMN_NAME,
  COLUMN_INFO,
  COLUMN_NUMBER
};


/* make sure we stop watching signals when the widget gets destroyed */
static void
destroy_connections (gpointer data,
		     GObject */*unused*/)
{
  std::list<sigc::connection> *conns
    = (std::list<sigc::connection> *)data;

  delete conns;
}

/* react to a new call being inserted in history */
static void
on_contact_added (Ekiga::ContactPtr contact,
		  GtkListStore *store)
{
  time_t t;
  struct tm *timeinfo = NULL;
  char buffer [80];
  std::stringstream info;
  const gchar *id = NULL;

  gmref_ptr<History::Contact> hcontact = contact;
  GtkTreeIter iter;

  if (hcontact) {

    switch (hcontact->get_type ()) {

    case History::RECEIVED:

      id = GM_STOCK_CALL_RECEIVED;
      break;

    case History::PLACED:

      id = GM_STOCK_CALL_PLACED;
      break;

    case History::MISSED:

      id = GM_STOCK_CALL_MISSED;
      break;

    default:
      id = GTK_STOCK_MISSING_IMAGE;
    }
  }

  t = hcontact->get_call_start ();
  timeinfo = localtime (&t);
  if (timeinfo != NULL) {
    strftime (buffer, 80, "%x %X", timeinfo);
    info << buffer;
    if (!hcontact->get_call_duration ().empty ())
      info << " (" << hcontact->get_call_duration () << ")";
  }
  else
    info << hcontact->get_call_duration ();

  gtk_list_store_prepend (store, &iter);
  gtk_list_store_set (store, &iter,
		      COLUMN_CONTACT, contact.get (),
		      COLUMN_PIXBUF, id,
		      COLUMN_NAME, contact->get_name ().c_str (),
		      COLUMN_INFO, info.str ().c_str (),
		      -1);
}

/* react to user clicks */
static gint
on_clicked (GtkWidget *tree,
	    GdkEventButton *event,
	    gpointer data)
{
  History::Book *book = NULL;
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;
  GtkTreeIter iter;
  Ekiga::Contact *contact = NULL;

  book = (History::Book*)data;
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree));


  if (gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (tree),
				     (gint) event->x, (gint) event->y,
				     &path, NULL, NULL, NULL)) {

    if (gtk_tree_model_get_iter (model, &iter, path)) {

      gtk_tree_model_get (model, &iter,
			  COLUMN_CONTACT, &contact,
			  -1);


      if (event->type == GDK_BUTTON_PRESS && event->button == 3) {

	MenuBuilderGtk builder;
	if (contact != NULL)
	  contact->populate_menu (builder);
	if (!builder.empty())
	  builder.add_separator ();
	builder.add_action ("clear", _("Clear List"),
			    sigc::mem_fun (book, &History::Book::clear));
	gtk_widget_show_all (builder.menu);
	gtk_menu_popup (GTK_MENU (builder.menu), NULL, NULL,
			NULL, NULL, event->button, event->time);
	g_signal_connect (G_OBJECT (builder.menu), "hide",
			  G_CALLBACK (g_object_unref),
			  (gpointer) builder.menu);
	g_object_ref_sink (G_OBJECT (builder.menu));
      }
      if (event->type == GDK_2BUTTON_PRESS) {

	if (contact != NULL) {

	  Ekiga::TriggerMenuBuilder builder;

	  contact->populate_menu (builder);
	}
      }

    }
    gtk_tree_path_free (path);
  }

  return TRUE;
}

/* public api */

GtkWidget *
call_history_view_gtk_new (gmref_ptr<History::Book> book)
{
  GtkWidget *result = NULL;
  std::list<sigc::connection> *conns = NULL;
  GtkListStore *store = NULL;
  GtkWidget *tree = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeSelection *selection = NULL;
  sigc::connection connection;

  result = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (result),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  /* we don't leak conns : it will be freed when we die... and it will
   * prevent the signals to come to a now-dead GObject
   */
  conns = new std::list<sigc::connection>;
  g_object_weak_ref (G_OBJECT (result), destroy_connections, (gpointer)conns);

  /* build the store then the tree */
  store = gtk_list_store_new (COLUMN_NUMBER,
			      G_TYPE_POINTER,
			      G_TYPE_STRING,
			      G_TYPE_STRING,
			      G_TYPE_STRING);

  tree = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree), FALSE);
  gtk_container_add (GTK_CONTAINER (result), tree);

  /* one column should be enough for everyone */
  column = gtk_tree_view_column_new ();

  /* show icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "stock-id", COLUMN_PIXBUF);

  /* show name and text */
  renderer = gm_cell_renderer_bitext_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "primary-text", COLUMN_NAME);
  gtk_tree_view_column_add_attribute (column, renderer,
				      "secondary-text", COLUMN_INFO);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  /* react to user clicks */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT (tree), "event-after",
		    G_CALLBACK (on_clicked), &(*book));

  /* connect to the signals */
  connection = book->cleared.connect (sigc::bind (sigc::ptr_fun (gtk_list_store_clear), store));
  conns->push_front (connection);
  connection = book->contact_added.connect (sigc::bind (sigc::ptr_fun (on_contact_added), store));
  conns->push_front (connection);

  /* populate */
  book->visit_contacts (sigc::bind_return(sigc::bind (sigc::ptr_fun (on_contact_added), store), true));

  return result;
}
