
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
 *                         statusicon.cpp  -  description
 *                         --------------------------
 *   begin                : Thu Jan 12 2006
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : High level tray api implementation
 */


#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#include "statusicon.h"

#include "gmstockicons.h"
#include "gmmenuaddon.h"

#include "callbacks.h" // FIXME SHOULD GET RID OF THIS
#include "misc.h" // FIXME same here
#include "ekiga.h"

#include "gtk-frontend.h"
#include "call-core.h"

#include <sigc++/sigc++.h>
#include <vector>

#include "services.h"
#include "gtk-frontend.h"
#include "personal-details.h"


/*
 * The StatusIcon
 */
struct _StatusIconPrivate
{
  _StatusIconPrivate (Ekiga::ServiceCore & _core) : core (_core) { }

  GtkWidget *popup_menu;
  gboolean has_message;

  std::vector<sigc::connection> connections;

  int blink_id;
  std::string status;
  bool unread_messages;
  bool blinking;

  gchar *blink_image;

  Ekiga::ServiceCore & core;
};

static GObjectClass *parent_class = NULL;


/*
 * Declaration of Callbacks
 */
static void
show_popup_menu_cb (GtkStatusIcon *icon,
                    guint button,
                    guint activate_time,
                    gpointer data);

static void
statusicon_activated_cb (GtkStatusIcon *icon,
                         gpointer data);

static void
unread_count_cb (GtkWidget *widget,
		 guint messages,
		 gpointer data);

static gboolean
statusicon_blink_cb (gpointer data);


/*
 * Declaration of local functions
 */
static GtkWidget *
statusicon_build_menu ();

static void
statusicon_start_blinking (StatusIcon *icon,
                           const char *stock_id);

static void
statusicon_stop_blinking (StatusIcon *icon);

static void
statusicon_set_status (StatusIcon *widget,
                       const std::string & presence);

static void
statusicon_set_inacall (StatusIcon *widget,
                        bool inacall);

static void 
established_call_cb (gmref_ptr<Ekiga::CallManager>  manager,
                     gmref_ptr<Ekiga::Call>  call,
                     gpointer self);

static void 
cleared_call_cb (gmref_ptr<Ekiga::CallManager>  manager,
                 gmref_ptr<Ekiga::Call>  call,
                 std::string reason,
                 gpointer self);

/*
 * GObject stuff
 */
static void
statusicon_dispose (GObject *obj)
{
  StatusIcon *icon = NULL;

  icon = STATUSICON (obj);

  if (icon->priv->popup_menu) {

    g_object_unref (icon->priv->popup_menu);
    icon->priv->popup_menu = NULL;
  }

  if (icon->priv->blink_image) {

    g_free (icon->priv->blink_image);
    icon->priv->blink_image = NULL;
  }
  
  parent_class->dispose (obj);
}


static void
statusicon_finalize (GObject *obj)
{
  StatusIcon *self = NULL;

  self = STATUSICON (obj);

  if (self->priv->blink_image)
    g_free (self->priv->blink_image);

  for (std::vector<sigc::connection>::iterator iter = self->priv->connections.begin () ;
       iter != self->priv->connections.end ();
       iter++)
    iter->disconnect ();

  delete self->priv;

  parent_class->finalize (obj);
}


static void
statusicon_class_init (gpointer g_class,
                       G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = statusicon_dispose;
  gobject_class->finalize = statusicon_finalize;
}


GType
statusicon_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (StatusIconClass),
      NULL,
      NULL,
      statusicon_class_init,
      NULL,
      NULL,
      sizeof (StatusIcon),
      0,
      NULL,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_STATUS_ICON,
				     "StatusIcon",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/*
 * Our own stuff
 */

/*
 * Callbacks
 */
static void
show_popup_menu_cb (GtkStatusIcon *icon,
                    guint button,
                    guint activate_time,
                    gpointer data)
{
  GtkWidget *popup = NULL;

  popup = GTK_WIDGET (data);

  gtk_menu_popup (GTK_MENU (popup),
                  NULL, NULL, 
                  (GtkMenuPositionFunc)gtk_status_icon_position_menu, icon,
                  button, activate_time);
}


static void
statusicon_activated_cb (G_GNUC_UNUSED GtkStatusIcon *icon,
                         gpointer data)
{
  StatusIcon *self = STATUSICON (data);

  GtkWidget *window = NULL;

  if (!self->priv->unread_messages) {

    window = GnomeMeeting::Process ()->GetMainWindow (); //FIXME

    // FIXME when the main window becomes a gobject
#if GTK_CHECK_VERSION(2,18,0)
    if (!(gtk_widget_get_visible (GTK_WIDGET (window))
#else
    if (!(GTK_WIDGET_VISIBLE (window)
#endif
          && !(gdk_window_get_state (GDK_WINDOW (window->window)) & GDK_WINDOW_STATE_ICONIFIED))) {
      gtk_widget_show (window);
    }
    else {

      if (gtk_window_has_toplevel_focus (GTK_WINDOW (window)))
        gtk_widget_hide (window);
      else
        gtk_window_present (GTK_WINDOW (window));
    }
  }
  else {

    gmref_ptr<GtkFrontend> frontend = self->priv->core.get ("gtk-frontend");
    GtkWidget *w = GTK_WIDGET (frontend->get_chat_window ());

    gtk_widget_show (w);
    gtk_window_present (GTK_WINDOW (w));
  }
}


static void
unread_count_cb (G_GNUC_UNUSED GtkWidget *widget,
		 guint messages,
		 gpointer data)
{
  StatusIcon *self = STATUSICON (data);

  gchar *message = NULL;

  if (messages > 0)
    statusicon_start_blinking (self, GM_STOCK_MESSAGE);
  else
    statusicon_stop_blinking (self);

  if (messages > 0) {

    message = g_strdup_printf (ngettext ("You have %d message",
					 "You have %d messages",
					 messages), messages);

#if GTK_CHECK_VERSION(2,16,0)
    gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (self), message);
#else
    gtk_status_icon_set_tooltip (GTK_STATUS_ICON (self), message);
#endif

    g_free (message);
  }
  else {
#if GTK_CHECK_VERSION(2,16,0)
    gtk_status_icon_set_tooltip_text (GTK_STATUS_ICON (self), NULL);
#else
    gtk_status_icon_set_tooltip (GTK_STATUS_ICON (self), NULL);
#endif
  }

  self->priv->unread_messages = (messages > 0);
}


static gboolean
statusicon_blink_cb (gpointer data)
{
  StatusIcon *statusicon = STATUSICON (data);

  GtkWidget *chat_window = NULL;
  GdkPixbuf *pixbuf = NULL;

  g_return_val_if_fail (data != NULL, false);

  gmref_ptr<GtkFrontend> frontend = statusicon->priv->core.get ("gtk-frontend");
  // FIXME use main_window here
  chat_window = GTK_WIDGET (frontend->get_chat_window ());

  pixbuf = gtk_widget_render_icon (chat_window, STATUSICON (data)->priv->blink_image,
                                   GTK_ICON_SIZE_MENU, NULL); 

  if (statusicon->priv->blinking)
    gtk_status_icon_set_from_pixbuf (GTK_STATUS_ICON (statusicon), pixbuf);
  else
    statusicon_set_status (statusicon, statusicon->priv->status);

  statusicon->priv->blinking = !statusicon->priv->blinking;

  return true;
}


static void
personal_details_updated_cb (StatusIcon* self,
			     gmref_ptr<Ekiga::PersonalDetails> details)
{
  statusicon_set_status (self, details->get_presence ());
}


static void 
established_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                     gmref_ptr<Ekiga::Call>  /*call*/,
                     gpointer self)
{
  statusicon_set_inacall (STATUSICON (self), true);
}


static void 
cleared_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                 gmref_ptr<Ekiga::Call>  /*call*/,
                 std::string /*reason*/,
                 gpointer self)
{
  statusicon_set_inacall (STATUSICON (self), false);
}


/*
 * Local functions
 */
static GtkWidget *
statusicon_build_menu ()
{
  GtkWidget *main_window = NULL;

  Ekiga::ServiceCore *services = NULL;

  services = GnomeMeeting::Process ()->GetServiceCore ();
  gmref_ptr<GtkFrontend> gtk_frontend = services->get ("gtk-frontend");
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  static MenuEntry menu [] =
    {
      GTK_MENU_ENTRY("help", NULL,
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1,
                     G_CALLBACK (help_cb), NULL, TRUE),

      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
		     GTK_STOCK_ABOUT, 0,
		     G_CALLBACK (about_callback), (gpointer) main_window,
		     TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("quit", NULL, _("Quit"),
		     GTK_STOCK_QUIT, 'Q',
		     G_CALLBACK (quit_callback),
		     main_window, TRUE),

      GTK_MENU_END
    };

  return GTK_WIDGET (gtk_build_popup_menu (NULL, menu, NULL));
}


static void
statusicon_start_blinking (StatusIcon *icon,
                           const char *stock_id)
{
  g_return_if_fail (icon != NULL);

  icon->priv->blink_image = g_strdup (stock_id);
#if GLIB_CHECK_VERSION (2, 14, 0)
  if (icon->priv->blink_id == -1)
    icon->priv->blink_id = g_timeout_add_seconds (1, statusicon_blink_cb, icon);
#else
  if (icon->priv->blink_id == -1)
    icon->priv->blink_id = g_timeout_add (1000, statusicon_blink_cb, icon);
#endif
}


static void
statusicon_stop_blinking (StatusIcon *self)
{
  if (self->priv->blink_image) {

    g_free (self->priv->blink_image);
    self->priv->blink_image = NULL;
  }

  if (self->priv->blink_id != -1) {

    g_source_remove (self->priv->blink_id);
    self->priv->blink_id = -1;
    self->priv->blinking = false;
  }

  statusicon_set_status (STATUSICON (self), self->priv->status);
}


static void
statusicon_set_status (StatusIcon *statusicon,
                       const std::string & presence)
{
  GtkWidget *chat_window = NULL;
  GdkPixbuf *pixbuf = NULL;

  g_return_if_fail (statusicon != NULL);

  gmref_ptr<GtkFrontend> frontend = statusicon->priv->core.get ("gtk-frontend");
  // FIXME use main_window here
  chat_window = GTK_WIDGET (frontend->get_chat_window ());

  /* Update the status icon */
  if (presence == "away")
    pixbuf = gtk_widget_render_icon (chat_window, GM_STOCK_STATUS_AWAY, 
                                     GTK_ICON_SIZE_MENU, NULL); 

  else if (presence == "dnd")
    pixbuf = gtk_widget_render_icon (chat_window, GM_STOCK_STATUS_DND, 
                                     GTK_ICON_SIZE_MENU, NULL); 
  else
    pixbuf = gtk_widget_render_icon (chat_window, GM_STOCK_STATUS_ONLINE, 
                                     GTK_ICON_SIZE_MENU, NULL); 

  gtk_status_icon_set_from_pixbuf (GTK_STATUS_ICON (statusicon), pixbuf);
  g_object_unref (pixbuf);

  statusicon->priv->status = presence;
}


static void
statusicon_set_inacall (StatusIcon *statusicon,
                        bool inacall)
{
  GtkWidget *chat_window = NULL;
  GdkPixbuf *pixbuf = NULL;

  g_return_if_fail (statusicon != NULL);

  gmref_ptr<GtkFrontend> frontend = statusicon->priv->core.get ("gtk-frontend");
  // FIXME use main_window here
  chat_window = GTK_WIDGET (frontend->get_chat_window ());

  /* Update the status icon */ 
  if (inacall) {

    pixbuf = gtk_widget_render_icon (chat_window, 
                                     GM_STOCK_STATUS_INACALL, 
                                     GTK_ICON_SIZE_MENU, 
                                     NULL); 
    gtk_status_icon_set_from_pixbuf (GTK_STATUS_ICON (statusicon), pixbuf);
    g_object_unref (pixbuf);
  }
  else {
    
    statusicon_set_status (statusicon, statusicon->priv->status);
  }
}


/*
 * Public API
 */
StatusIcon *
statusicon_new (Ekiga::ServiceCore & core)
{
  StatusIcon *self = NULL;

  sigc::connection conn;

  self = STATUSICON (g_object_new (STATUSICON_TYPE, NULL));
  self->priv = new StatusIconPrivate (core);

  self->priv->popup_menu = statusicon_build_menu ();
  g_object_ref_sink (self->priv->popup_menu);
  self->priv->has_message = FALSE;
  self->priv->blink_id = -1;
  self->priv->blinking = false;
  self->priv->blink_image = NULL;
  self->priv->unread_messages = false;

  gmref_ptr<GtkFrontend> frontend = core.get ("gtk-frontend");
  gmref_ptr<Ekiga::PersonalDetails> details = core.get ("personal-details");
  gmref_ptr<Ekiga::CallCore> call_core = core.get ("call-core");
  GtkWidget *chat_window = GTK_WIDGET (frontend->get_chat_window ());

  statusicon_set_status (self, details->get_presence ());
  conn = details->updated.connect (sigc::bind (sigc::ptr_fun (personal_details_updated_cb), self, details));
  self->priv->connections.push_back (conn);

  conn = call_core->established_call.connect (sigc::bind (sigc::ptr_fun (established_call_cb), 
                                                          (gpointer) self));
  self->priv->connections.push_back (conn);

  conn = call_core->cleared_call.connect (sigc::bind (sigc::ptr_fun (cleared_call_cb), 
                                                      (gpointer) self));
  self->priv->connections.push_back (conn);

  g_signal_connect (self, "popup-menu",
                    G_CALLBACK (show_popup_menu_cb), self->priv->popup_menu);

  g_signal_connect (self, "activate",
                    G_CALLBACK (statusicon_activated_cb), self);

  g_signal_connect (chat_window, "unread-count",
                    G_CALLBACK (unread_count_cb), self);

  return self;
}
