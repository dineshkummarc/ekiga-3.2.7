
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
 *                         chat-window.h  -  description
 *                         -----------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a window to display chats
 *
 */

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#include "chat-window.h"
#include "simple-chat-page.h"
#include "multiple-chat-page.h"

struct _ChatWindowPrivate
{
  _ChatWindowPrivate (Ekiga::ChatCore& core_): core(core_)
  {}

  Ekiga::ChatCore& core;
  std::list<sigc::connection> connections;

  GtkWidget* notebook;
};

enum {
  UNREAD_COUNT,
  UNREAD_ALERT,
  LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

static GObjectClass* parent_class = NULL;

/* helper (declaration) */

static void update_unread (ChatWindow* self);

/* signal callbacks (declarations) */

static void on_close_button_clicked (GtkButton* button,
				     gpointer data);

static void on_escaped (GtkWidget *widget,
                        gpointer data);

static void on_switch_page (GtkNotebook* notebook,
			    GtkNotebookPage* page,
			    guint num,
			    gpointer data);

static gboolean on_focus_in_event (GtkWidget* widget,
				   GdkEventFocus* event,
				   gpointer data);

static void on_message_notice_event (GtkWidget* page,
				     gpointer data);

static bool on_dialect_added (ChatWindow* self,
			      Ekiga::DialectPtr dialect);
static bool on_simple_chat_added (ChatWindow* self,
				  Ekiga::SimpleChatPtr chat);
static bool on_multiple_chat_added (ChatWindow* self,
				    Ekiga::MultipleChatPtr chat);
static void on_some_chat_user_requested (ChatWindow* self,
					 GtkWidget* page);

/* helper (implementation) */

static void
update_unread (ChatWindow* self)
{
  guint unread_count = 0;
  GtkWidget* page = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* label = NULL;

  for (gint ii = 0;
       ii < gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) ;
       ii++) {

    page
      = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), ii);
    hbox = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
				       page);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (hbox), "label-widget");
    unread_count
      = unread_count
      + GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (label), "unread-count"));

  }

  g_signal_emit (self, signals[UNREAD_COUNT], 0, unread_count);
}

/* signal callbacks (implementations) */

static void
on_close_button_clicked (GtkButton* button,
			 gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  GtkWidget* page = NULL;
  gint num = 0;

  page = (GtkWidget*)g_object_get_data (G_OBJECT (button), "page-widget");
  num = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv->notebook), page);

  gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), num);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) == 0)
    gtk_widget_hide (GTK_WIDGET (self));
}

static void
on_escaped (GtkWidget */*widget*/,
            gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  gint num = 0;

  num = gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook));
  gtk_notebook_remove_page (GTK_NOTEBOOK (self->priv->notebook), num);

  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) == 0)
    gtk_widget_hide (GTK_WIDGET (self));
}

static void
on_switch_page (G_GNUC_UNUSED GtkNotebook* notebook,
		G_GNUC_UNUSED GtkNotebookPage* page_,
		guint num,
		gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  GtkWidget* page = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* label = NULL;

  page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), num);
  hbox = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
				     page);
  label = (GtkWidget*)g_object_get_data (G_OBJECT (hbox), "label-widget");
  gtk_label_set_text (GTK_LABEL (label),
		      (const gchar*)g_object_get_data (G_OBJECT (label),
						       "base-title"));
  g_object_set_data (G_OBJECT (label), "unread-count",
		     GUINT_TO_POINTER (0));

  update_unread (self);

  gtk_widget_grab_focus (page);
}

static gboolean
on_focus_in_event (G_GNUC_UNUSED GtkWidget* widget,
		   G_GNUC_UNUSED GdkEventFocus* event,
		   gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  gint num;
  GtkWidget* page = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* label = NULL;

  num = gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook));
  if (num != -1) { /* the notebook may be empty */

    page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook), num);
    hbox = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
				       page);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (hbox), "label-widget");
    gtk_label_set_text (GTK_LABEL (label),
			(const gchar*)g_object_get_data (G_OBJECT (label),
						       "base-title"));
    g_object_set_data (G_OBJECT (label), "unread-count",
		       GUINT_TO_POINTER (0));

    update_unread (self);
  }

  return FALSE;
}

static void
on_message_notice_event (GtkWidget* page,
			 gpointer data)
{
  ChatWindow* self = (ChatWindow*)data;
  gint num = -1;

  for (gint ii = 0;
       ii < gtk_notebook_get_n_pages (GTK_NOTEBOOK (self->priv->notebook)) ;
       ii++) {

    if (page == gtk_notebook_get_nth_page (GTK_NOTEBOOK (self->priv->notebook),
					   ii)) {

      num = ii;
      break;
    }
  }

  if (num
      != gtk_notebook_get_current_page (GTK_NOTEBOOK (self->priv->notebook))
      || !gtk_window_is_active (GTK_WINDOW (self))) {

    GtkWidget* hbox = NULL;
    GtkWidget* label = NULL;
    guint unread_count = 0;
    const gchar* base_title = NULL;
    gchar* txt = NULL;

    hbox = gtk_notebook_get_tab_label (GTK_NOTEBOOK (self->priv->notebook),
				       page);
    label = (GtkWidget*)g_object_get_data (G_OBJECT (hbox), "label-widget");
    base_title = (const gchar*)g_object_get_data (G_OBJECT (label),
						  "base-title");
    unread_count = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (label),
							"unread-count"));
    unread_count = unread_count + 1;
    g_object_set_data (G_OBJECT (label), "unread-count",
		       GUINT_TO_POINTER (unread_count));

    txt = g_strdup_printf ("[%d] %s", unread_count, base_title);
    gtk_label_set_text (GTK_LABEL (label), txt);
    g_free (txt);

    g_signal_emit (self, signals[UNREAD_ALERT], 0, NULL);
  }

  update_unread (self);
}

static bool
on_dialect_added (ChatWindow* self,
		  Ekiga::DialectPtr dialect)
{
  self->priv->connections.push_front (dialect->simple_chat_added.connect (sigc::hide_return (sigc::bind<0> (sigc::ptr_fun (on_simple_chat_added), self))));
  self->priv->connections.push_front (dialect->multiple_chat_added.connect (sigc::hide_return (sigc::bind<0> (sigc::ptr_fun (on_multiple_chat_added), self))));

  dialect->visit_simple_chats (sigc::bind<0> (sigc::ptr_fun (on_simple_chat_added), self));
  dialect->visit_multiple_chats (sigc::bind<0> (sigc::ptr_fun (on_multiple_chat_added), self));

  return true;
}

static bool
on_simple_chat_added (ChatWindow* self,
		      Ekiga::SimpleChatPtr chat)
{
  GtkWidget* page = NULL;
  GtkWidget* hbox = NULL;
  GtkWidget* label = NULL;
  GtkWidget* close_button = NULL;
  GtkWidget* close_image = NULL;
  gint num;

  page = simple_chat_page_new (chat);
  hbox = gtk_hbox_new (FALSE, 2);

  label = gtk_label_new (chat->get_title ().c_str ());
  g_object_set_data_full (G_OBJECT (label), "base-title",
			  g_strdup (chat->get_title ().c_str ()),
			  g_free);
  
  close_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
  close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU);
  gtk_widget_set_size_request (GTK_WIDGET (close_image), 12, 12);
  gtk_widget_set_size_request (GTK_WIDGET (close_button), 16, 16);
  gtk_container_add (GTK_CONTAINER (close_button), close_image);
  gtk_container_set_border_width (GTK_CONTAINER (close_button), 0);
  g_object_set_data (G_OBJECT (close_button), "page-widget", page);
  g_signal_connect (close_button, "clicked",
		    G_CALLBACK (on_close_button_clicked), self);

  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
  g_object_set_data (G_OBJECT (hbox), "label-widget", label);
  gtk_box_pack_end (GTK_BOX (hbox), close_button, FALSE, FALSE, 2);
  g_object_set_data (G_OBJECT (hbox), "close-button-widget", close_button);
  gtk_widget_show_all (hbox);

  num = gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
				  page, hbox);
  gtk_widget_show (page);
  g_signal_connect (page, "message-notice-event",
		    G_CALLBACK (on_message_notice_event), self);

  self->priv->connections.push_front (chat->user_requested.connect (sigc::bind (sigc::ptr_fun (on_some_chat_user_requested), self, page)));

  return true;
}

static bool
on_multiple_chat_added (ChatWindow* self,
			Ekiga::MultipleChatPtr chat)
{
  GtkWidget* page = NULL;
  GtkWidget* label = NULL;
  gint num;

  page = multiple_chat_page_new (chat);
  label = gtk_label_new (chat->get_title ().c_str ());

  num = gtk_notebook_append_page (GTK_NOTEBOOK (self->priv->notebook),
				  page, label);
  gtk_widget_show_all (page);

  self->priv->connections.push_front (chat->user_requested.connect (sigc::bind (sigc::ptr_fun (on_some_chat_user_requested), self, page)));

  return true;
}

static void
on_some_chat_user_requested (ChatWindow* self,
			     GtkWidget* page)
{
  gint num;

  num = gtk_notebook_page_num (GTK_NOTEBOOK (self->priv->notebook), page);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (self->priv->notebook), num);
  gtk_widget_show (GTK_WIDGET (self));
  gtk_window_present (GTK_WINDOW (self));
}

/* GObject code */

static void
chat_window_dispose (GObject* obj)
{
  ChatWindow* self = NULL;

  self = CHAT_WINDOW (obj);

  parent_class->dispose (obj);
}

static void
chat_window_finalize (GObject* obj)
{
  ChatWindow* self = NULL;

  self = CHAT_WINDOW (obj);

  for (std::list<sigc::connection>::iterator iter
	 = self->priv->connections.begin ();
       iter != self->priv->connections.end ();
       ++iter)
    iter->disconnect ();

  delete self->priv;
  self->priv = NULL;

  parent_class->finalize (obj);
}

static void
chat_window_class_init (gpointer g_class,
			G_GNUC_UNUSED gpointer class_data)
{
  ChatWindowClass* chat_window_class = NULL;
  GObjectClass* gobject_class = NULL;

  parent_class = (GObjectClass*) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*) g_class;
  gobject_class->dispose = chat_window_dispose;
  gobject_class->finalize = chat_window_finalize;

  signals[UNREAD_COUNT] =
    g_signal_new ("unread-count",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatWindowClass, unread_count),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__UINT,
		  G_TYPE_NONE, 1,
		  G_TYPE_UINT);

  signals[UNREAD_ALERT] =
    g_signal_new ("unread-alert",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (ChatWindowClass, unread_alert),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

  /* FIXME: is it useful? */
  chat_window_class = (ChatWindowClass*)g_class;
  chat_window_class->unread_count = NULL;
}

static void
chat_window_init (GTypeInstance* instance,
		  G_GNUC_UNUSED gpointer g_class)
{
  /* we can't do much here since we get the Chat as reference... */
  gtk_window_set_title (GTK_WINDOW (instance), _("Chat Window"));
}

GType
chat_window_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (ChatWindowClass),
      NULL,
      NULL,
      chat_window_class_init,
      NULL,
      NULL,
      sizeof (ChatWindow),
      0,
      chat_window_init,
      NULL
    };

    result = g_type_register_static (GM_WINDOW_TYPE,
				     "ChatWindow",
				     &info, (GTypeFlags) 0);
  }

  return result;
}

/* public api */

GtkWidget*
chat_window_new (Ekiga::ChatCore& core,
		 const std::string key)
{
  ChatWindow* result = NULL;
  GtkAccelGroup *accel = NULL;

  result = (ChatWindow*)g_object_new (CHAT_WINDOW_TYPE,
                                      "key", key.c_str (), 
                                      "hide_on_esc", FALSE, 
                                      NULL);

  result->priv = new ChatWindowPrivate (core);

  result->priv->notebook = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (result), result->priv->notebook);
  gtk_widget_show (result->priv->notebook);

  accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (result), accel);
  gtk_accel_group_connect (accel, GDK_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (on_escaped), (gpointer) result, NULL));
  g_object_unref (accel);

  g_signal_connect (result, "focus-in-event",
		    G_CALLBACK (on_focus_in_event), result);
  g_signal_connect (result->priv->notebook, "switch-page",
		    G_CALLBACK (on_switch_page), result);

  result->priv->connections.push_front (core.dialect_added.connect (sigc::hide_return(sigc::bind<0>(sigc::ptr_fun (on_dialect_added), result))));
  core.visit_dialects (sigc::bind<0>(sigc::ptr_fun (on_dialect_added), result));

  return (GtkWidget*)result;
}
