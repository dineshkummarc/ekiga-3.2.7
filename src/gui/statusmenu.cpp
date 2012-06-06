
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
 *                         statusmenu.h  -  description
 *                         -------------------------------
 *   begin                : Mon Jan 28 2008
 *   copyright            : (C) 2000-2008 by Damien Sandras 
 *   description          : Contains a StatusMenu
 *
 */



#include "statusmenu.h"

#include "common.h"
#include "personal-details.h"

#include "gmconf.h"
#include "gmstockicons.h"

#include <glib/gi18n.h>
#include <vector>

struct _StatusMenuPrivate
{
  gmref_ptr<Ekiga::PersonalDetails> personal_details;
  std::vector<sigc::connection> connections;

  GtkListStore *list_store; // List store storing the menu
  GtkWindow    *parent;     // Parent window
};

enum Columns
{
  COL_ICON,          // The status icon
  COL_MESSAGE,       // The status message (if any)
  COL_MESSAGE_TYPE,  // The status message type
  COL_SEPARATOR,     // A separator
  NUM_COLUMNS
};

enum MessageType
{
  TYPE_ONLINE,             // Generic online message
  TYPE_AWAY,               // Generic away message
  TYPE_DND,                // Generic Do Not Disturb message
  NUM_STATUS_TYPES,
  TYPE_CUSTOM_ONLINE,      // Custom online message
  TYPE_CUSTOM_AWAY,        // Custom away message
  TYPE_CUSTOM_DND,         // Custom DND message
  NUM_STATUS_CUSTOM_TYPES,
  TYPE_CUSTOM_ONLINE_NEW,  // Add new custom online message
  TYPE_CUSTOM_AWAY_NEW,    // Add new custom away message
  TYPE_CUSTOM_DND_NEW,     // Add new custom dnd message
  TYPE_CLEAR               // Clear custom message(s)
};

const gchar *statuses [] = 
{ 
  N_("Online"), 
  N_("Away"), 
  N_("Do Not Disturb") 
};

const char* status_types_names[] = 
{ 
  "online",
  "away",
  "dnd"
};

const char* status_types_keys[] = 
{ 
  PERSONAL_DATA_KEY "online_custom_status",
  PERSONAL_DATA_KEY "away_custom_status",
  PERSONAL_DATA_KEY "dnd_custom_status"
};

const char* stock_status[] = 
{ 
  GM_STOCK_STATUS_ONLINE,
  GM_STOCK_STATUS_AWAY,
  GM_STOCK_STATUS_DND
};


/**
 * Callbacks
 */

/** Return true if the row is a separator.
 *
 * @param model is a pointer to the GtkTreeModel
 * @param iter is a pointer to the current row in the GtkTreeModel
 * @return true if the current row is a separator, false otherwise
 */
static gboolean
status_menu_row_is_separator (GtkTreeModel *model,
                              GtkTreeIter *iter,
                              gpointer /*data*/);


/** Trigger the appropriate action when a choice is made in the StatusMenu.
 *
 * It will update the GmConf key with the chosen value or display a popup 
 * allowing to add or remove status messages.
 *
 * @param box is a pointer to the GtkComboBox
 * @param data is a pointer to the StatusMenu
 */
static void 
status_menu_option_changed (GtkComboBox *box,
                            gpointer data);


/**
 * GmConf notifiers
 */

/** This notifier is triggered when one of the custom messages list is updated. 
 *
 * It updates the StatusMenu content with the new values.
 *
 * @param id is the GmConf notifier id
 * @param entry is the GmConfEntry for which the notification was triggered
 * @param data is a pointer to the StatusMenu
 */
static void
status_menu_custom_messages_changed (gpointer id,
                                     GmConfEntry *entry,
                                     gpointer data);


/** This callback is triggered when the user details are modified.
 *
 * It updates the StatusMenu content with the new value as current choice.
 *
 * @param self is a pointer to the StatusMenu
 */
static void
on_details_updated (StatusMenu *self);


/**
 * Static methods
 */

/** This function populates the StatusMenu with its initial content. Content is 
 * taken from the GmConf keys used by the StatusMenu.
 *
 * @param self is the StatusMenu
 * @param custom_status_array is the list of custom messages
 */
static void
status_menu_populate (StatusMenu *self,
                      GSList *custom_status_array [NUM_STATUS_TYPES]);


/** This function updates the default active status in the StatusMenu.
 *
 * @param self is the StatusMenu
 * @param presence is the short status of the current status
 * @param status is the long status description of the current status
 */
static void
status_menu_set_option (StatusMenu *self,
                        const std::string & presence,
                        const std::string & status);


/** This function presents a popup allowing to remove some of the user defined
 * status messages.
 *
 * @param self is the StatusMenu
 */
static void
status_menu_clear_status_message_dialog_run (StatusMenu *self);


/** This function runs a dialog allowing the user to define custom long status
 * messages that he will be able to publish.
 *
 * @param self is the StatusMenu
 * @param option is the defined message type (TYPE_CUSTOM_ONLINE, 
 * TYPE_CUSTOM_AWAY, TYPE_CUSTOM_DND)
 */
static void
status_menu_new_status_message_dialog_run (StatusMenu *self,
                                           int option);


/* 
 * GObject stuff 
 */
static GObjectClass *parent_class = NULL;

static void status_menu_class_init (gpointer g_class,
                                    gpointer class_data);

static void status_menu_dispose (GObject *obj);

static void status_menu_finalize (GObject *obj);


/* 
 * Callbacks
 */
static gboolean
status_menu_row_is_separator (GtkTreeModel *model,
                              GtkTreeIter *iter,
                              gpointer /*data*/)
{
  gboolean is_separator;

  gtk_tree_model_get (model, iter, COL_SEPARATOR, &is_separator, -1);

  return is_separator;
}


static void 
status_menu_option_changed (GtkComboBox *box,
                            gpointer data)
{
  GtkTreeIter iter;

  int i = 0;
  gchar* status = NULL;

  GtkTreeModel* model = NULL;
  StatusMenu* self = STATUS_MENU (data);

  g_return_if_fail (self != NULL);

  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (box), &iter)) {

    model = gtk_combo_box_get_model (GTK_COMBO_BOX (box));
    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, COL_MESSAGE_TYPE, &i, 
                        COL_MESSAGE, &status, -1);

    switch (i)
      {
      case TYPE_ONLINE:
        self->priv->personal_details->set_presence_info ("online", "");
        break;

      case TYPE_AWAY:
        self->priv->personal_details->set_presence_info ("away", "");
        break;

      case TYPE_DND:
        self->priv->personal_details->set_presence_info ("dnd", "");
        break;

      case TYPE_CUSTOM_ONLINE:
        self->priv->personal_details->set_presence_info ("online", status);
        break;

      case TYPE_CUSTOM_AWAY:
        self->priv->personal_details->set_presence_info ("away", status);
        break;

      case TYPE_CUSTOM_DND:
        self->priv->personal_details->set_presence_info ("dnd", status);
        break;

      case TYPE_CUSTOM_ONLINE_NEW:
        status_menu_new_status_message_dialog_run (self, TYPE_CUSTOM_ONLINE);
        break;

      case TYPE_CUSTOM_AWAY_NEW:
        status_menu_new_status_message_dialog_run (self, TYPE_CUSTOM_AWAY);
        break;

      case TYPE_CUSTOM_DND_NEW:
        status_menu_new_status_message_dialog_run (self, TYPE_CUSTOM_DND);
        break;

      case TYPE_CLEAR:
        status_menu_clear_status_message_dialog_run (self);
        break;

      default:
        break;
      }

    g_free (status);
  }
}


static void
status_menu_custom_messages_changed (gpointer /*id*/,
                                     GmConfEntry *entry,
                                     gpointer self)
{
  std::string key = gm_conf_entry_get_key (entry);
  GSList *current = gm_conf_entry_get_list (entry);
  GSList *custom_status_array [NUM_STATUS_TYPES];

  for (int i = 0 ; i < NUM_STATUS_TYPES ; i++) {
    if (key == status_types_keys [i])
      custom_status_array [i] = current;
    else
      custom_status_array [i] = gm_conf_get_string_list (status_types_keys [i]);
  }

  status_menu_populate (STATUS_MENU (self), custom_status_array);

  for (int i = 0 ; i < NUM_STATUS_TYPES ; i++) {
    g_slist_foreach (custom_status_array [i], (GFunc) g_free, NULL);
    g_slist_free (custom_status_array [i]);
  }
}


static void
on_details_updated (StatusMenu *self)
{
  status_menu_set_option (self, self->priv->personal_details->get_presence (), self->priv->personal_details->get_status ());
}


/*
 * Static methods
 */
static void
status_menu_populate (StatusMenu *self,
                      GSList *custom_status_array [NUM_STATUS_TYPES])
{
  gboolean has_custom_messages = false;
  GSList *custom_status = NULL;
  GSList *liter = NULL;
  GtkTreeIter iter;
  GdkPixbuf* icon = NULL;

  gtk_list_store_clear (GTK_LIST_STORE (self->priv->list_store));

  for (int i = 0 ; i < NUM_STATUS_TYPES ; i++) {

    statuses [i] = gettext (statuses [i]);
    custom_status = custom_status_array [i];
    liter = custom_status;

    icon = gtk_widget_render_icon (GTK_WIDGET (self),
                                   stock_status [i],
                                   GTK_ICON_SIZE_MENU, NULL);

    gtk_list_store_append (GTK_LIST_STORE (self->priv->list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (self->priv->list_store), &iter,
                        COL_ICON, icon, 
                        COL_MESSAGE, statuses[i], 
                        COL_MESSAGE_TYPE, i,
                        COL_SEPARATOR, false, 
                        -1); 

    gtk_list_store_append (GTK_LIST_STORE (self->priv->list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (self->priv->list_store), &iter,
                        COL_ICON, icon, 
                        COL_MESSAGE, _("Custom message..."), 
                        COL_MESSAGE_TYPE, NUM_STATUS_CUSTOM_TYPES + (i + 1),
                        COL_SEPARATOR, false, 
                        -1); 

    while (liter) {

      gtk_list_store_append (GTK_LIST_STORE (self->priv->list_store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (self->priv->list_store), &iter,
                          COL_ICON, icon, 
                          COL_MESSAGE, (char*) (liter->data), 
                          COL_MESSAGE_TYPE, NUM_STATUS_TYPES + (i + 1),
                          COL_SEPARATOR, false, 
                          -1); 

      liter = g_slist_next (liter);
      has_custom_messages = true;
    }

    if (i < NUM_STATUS_TYPES - 1) {
      gtk_list_store_append (GTK_LIST_STORE (self->priv->list_store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (self->priv->list_store), &iter,
                          COL_SEPARATOR, true, 
                          -1); 
    }

    g_object_unref (icon);
  }

  /* Clear message */
  if (has_custom_messages) {

    gtk_list_store_append (GTK_LIST_STORE (self->priv->list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (self->priv->list_store), &iter,
                        COL_SEPARATOR, true, 
                        -1); 

    icon = gtk_widget_render_icon (GTK_WIDGET (self),
                                   GTK_STOCK_CLEAR,
                                   GTK_ICON_SIZE_MENU, NULL);
    gtk_list_store_append (GTK_LIST_STORE (self->priv->list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (self->priv->list_store), &iter,
                        COL_ICON, icon,
                        COL_MESSAGE, _("Clear"), 
                        COL_MESSAGE_TYPE, TYPE_CLEAR,
                        COL_SEPARATOR, false, 
                        -1); 
    g_object_unref (icon);
  }

  status_menu_set_option (self, self->priv->personal_details->get_presence (), self->priv->personal_details->get_status ());
}


static void
status_menu_set_option (StatusMenu *self,
                        const std::string & presence,
                        const std::string & status)
{
  GtkTreeIter iter;

  bool valid = false;
  gchar *sstatus = NULL;
  int i = 0;
  int cpt = 0;

  if (presence.empty ())
    return;

  valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->priv->list_store), &iter);
  while (valid) {

    gtk_tree_model_get (GTK_TREE_MODEL (self->priv->list_store), &iter, 
                        COL_MESSAGE_TYPE, &i,
                        COL_MESSAGE, &sstatus, -1); 

    // Check if it is a custom status message and if it is in the list
    if (i == TYPE_CUSTOM_ONLINE || i == TYPE_CUSTOM_AWAY || i == TYPE_CUSTOM_DND) {
      if (presence == status_types_names[i - NUM_STATUS_TYPES - 1] && status == sstatus)
        break;
    }

    // Long status empty, the user did not set a custom message
    if (i == TYPE_ONLINE || i == TYPE_AWAY || i == TYPE_DND) {
      if (status.empty () && presence == status_types_names[i]) 
        break;
    }

    g_free (sstatus);

    cpt++;
    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (self->priv->list_store), &iter);
  }

  if (valid) {
    gtk_combo_box_set_active (GTK_COMBO_BOX (self), cpt);

    if (sstatus)
      g_free (sstatus);
  }
}


static void
status_menu_clear_status_message_dialog_run (StatusMenu *self)
{
  GtkTreeIter iter, liter;

  GSList *conf_list [3] = { NULL, NULL, NULL };
  GtkWidget *dialog = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *tree_view = NULL;

  GdkPixbuf *pixbuf = NULL;
  GtkTreeSelection *selection = NULL;
  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkWidget *label = NULL;

  bool found = false;
  bool close = false;
  int response = 0;
  int i = 0;
  gchar *message = NULL;
  gchar *presence = NULL;
  gchar *status = NULL;

  // Current status
  presence = gm_conf_get_string (PERSONAL_DATA_KEY "short_status");
  status = gm_conf_get_string (PERSONAL_DATA_KEY "long_status");

  // Build the dialog
  dialog = gtk_dialog_new_with_buttons (_("Custom Message"),
                                        self->priv->parent,
                                        (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                        GTK_STOCK_DELETE,
                                        GTK_RESPONSE_APPLY,
                                        GTK_STOCK_CLOSE,
                                        GTK_RESPONSE_CLOSE,
                                        NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CLOSE);
  gtk_window_set_icon_name (GTK_WINDOW (dialog), GTK_STOCK_DELETE);

  vbox = gtk_vbox_new (false, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox, false, false, 2);


  label = gtk_label_new (_("Delete custom messages:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, false, false, 2);

  list_store = gtk_list_store_new (3,
                                   GDK_TYPE_PIXBUF,
                                   G_TYPE_STRING,
                                   G_TYPE_INT);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  g_object_unref (list_store);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), false);

  column = gtk_tree_view_column_new ();
  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
                                       "pixbuf", 0,
                                       NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_column_pack_start (column, renderer, FALSE);
  gtk_tree_view_column_set_attributes (column, renderer,
                                       "text", 1,
                                       NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  frame = gtk_frame_new (NULL);
  gtk_container_add (GTK_CONTAINER (frame), tree_view);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 2);

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (self->priv->list_store), &iter)) {

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (self->priv->list_store), &iter,
                          COL_MESSAGE_TYPE, &i, -1);

      if (i == TYPE_CUSTOM_ONLINE || i == TYPE_CUSTOM_AWAY || i == TYPE_CUSTOM_DND) {

        gtk_tree_model_get (GTK_TREE_MODEL (self->priv->list_store), &iter,
                            COL_ICON, &pixbuf, 
                            COL_MESSAGE, &message,
                            -1);
        gtk_list_store_append (GTK_LIST_STORE (list_store), &liter);
        gtk_list_store_set (GTK_LIST_STORE (list_store), &liter, 
                            COL_ICON, pixbuf, 
                            COL_MESSAGE, message,
                            COL_MESSAGE_TYPE, i,
                            -1);
        g_free (message);
        g_object_unref (pixbuf);
      }

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (self->priv->list_store), &iter));
  }

  // Select the first iter
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) 
    gtk_tree_selection_select_iter (selection, &iter);

  gtk_widget_show_all (dialog);
  while (!close) {
    response = gtk_dialog_run (GTK_DIALOG (dialog));

    switch (response)
      {
      case GTK_RESPONSE_APPLY:
        if (gtk_tree_selection_get_selected (selection, NULL, &iter)) 
          gtk_list_store_remove (GTK_LIST_STORE (list_store), &iter);
        if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) 
          gtk_tree_selection_select_iter (selection, &iter);
        else
          close = true;
        break;

      case GTK_RESPONSE_CLOSE:
      default:
        close = true;
      }
  }

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter)) {

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (list_store), &iter,
                          1, &message,
                          2, &i, -1);
      if (status && message && !strcmp (status, message))
        found = true;

      conf_list[i - NUM_STATUS_TYPES - 1] = g_slist_append (conf_list[i - NUM_STATUS_TYPES - 1], g_strdup (message));
      g_free (message);
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter));
  }

  for (int j = 0 ; j < 3 ; j++) {
    gm_conf_set_string_list (status_types_keys[j], conf_list[j]);
    g_slist_foreach (conf_list[j], (GFunc) g_free, NULL);
    g_slist_free (conf_list[j]);
  }

  if (!found) {
    // Reset current config
    self->priv->personal_details->set_presence_info ("online", "");
  }

  gtk_widget_destroy (dialog);
}


static void
status_menu_new_status_message_dialog_run (StatusMenu *self,
                                           int option)
{
  gchar *presence = NULL;
  gchar *status = NULL;

  GSList *clist = NULL;
  GtkWidget *dialog = NULL;
  GtkWidget *label = NULL;
  GtkWidget *entry = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *image = NULL;

  GdkPixbuf* icon = NULL;

  const char *message = NULL;

  presence = gm_conf_get_string (PERSONAL_DATA_KEY "short_status");
  status = gm_conf_get_string (PERSONAL_DATA_KEY "long_status");

  dialog = gtk_dialog_new_with_buttons (_("Custom Message"),
                                        self->priv->parent,
                                        (GtkDialogFlags) (GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
                                        GTK_STOCK_OK,
                                        GTK_RESPONSE_ACCEPT,
                                        NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_ACCEPT);

  vbox = gtk_vbox_new (false, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), vbox, false, false, 2);

  hbox = gtk_hbox_new (false, 2);
  icon = gtk_widget_render_icon (GTK_WIDGET (self),
                                 stock_status [option - NUM_STATUS_TYPES - 1],
                                 GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (dialog), icon);
  image = gtk_image_new_from_pixbuf (icon);
  g_object_unref (icon);
  gtk_box_pack_start (GTK_BOX (hbox), image, false, false, 2);

  label = gtk_label_new (_("Define a custom message:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, false, false, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, false, false, 2);

  entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (entry), true);
  gtk_box_pack_start (GTK_BOX (vbox), entry, false, false, 2);

  gtk_widget_show_all (dialog);
  switch (gtk_dialog_run (GTK_DIALOG (dialog))) {

  case GTK_RESPONSE_ACCEPT:
    message = gtk_entry_get_text (GTK_ENTRY (entry));
    clist = gm_conf_get_string_list (status_types_keys[option - NUM_STATUS_TYPES - 1]);
    if (message && strcmp (message, "")) { 
      clist = g_slist_append (clist, g_strdup (message));
      gm_conf_set_string_list (status_types_keys[option - NUM_STATUS_TYPES - 1], clist);
      self->priv->personal_details->set_presence_info (status_types_names[option - NUM_STATUS_TYPES - 1], message);
    }
    else {
      status_menu_set_option (self, presence ? presence : "", status ? status : "");
    }
    g_slist_foreach (clist, (GFunc) g_free, NULL);
    g_slist_free (clist);
    break;

  default:
    status_menu_set_option (self, presence ? presence : "", status ? status : "");
    break;
  }

  gtk_widget_destroy (dialog);

  g_free (presence);
  g_free (status);
}


/* 
 * GObject stuff
 */
static void
status_menu_class_init (gpointer g_class,
                        gpointer /*class_data*/)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = status_menu_dispose;
  gobject_class->finalize = status_menu_finalize;
}


static void
status_menu_dispose (GObject *obj)
{
  StatusMenu *self = NULL;

  self = STATUS_MENU (obj);
  delete self->priv;

  self->priv = NULL;

  // NULLify everything
  parent_class->dispose (obj);
}


static void
status_menu_finalize (GObject *obj)
{
  parent_class->finalize (obj);
}


GType
status_menu_get_type (void)
{
  static GType status_menu_type = 0;

  if (status_menu_type == 0) {

    static const GTypeInfo status_menu_info =
      {
        sizeof (StatusMenuClass),
        NULL,
        NULL,
        (GClassInitFunc) status_menu_class_init,
        NULL,
        NULL,
        sizeof (StatusMenu),
        0,
        NULL,
        NULL
      };

    status_menu_type = g_type_register_static (GTK_TYPE_COMBO_BOX,
                                               "StatusMenu",
                                               &status_menu_info,
                                               (GTypeFlags) 0);
  }

  return status_menu_type;
}


GtkWidget *
status_menu_new (Ekiga::ServiceCore & core)
{
  StatusMenu *self = NULL;

  sigc::connection conn;
  GtkCellRenderer *renderer = NULL;
  GSList *custom_status_array [NUM_STATUS_TYPES];

  self = (StatusMenu *) g_object_new (STATUS_MENU_TYPE, NULL);
  self->priv = new StatusMenuPrivate ();

  self->priv->personal_details = core.get ("personal-details");
  self->priv->parent = NULL;
  self->priv->list_store = gtk_list_store_new (NUM_COLUMNS,
                                               GDK_TYPE_PIXBUF,
                                               G_TYPE_STRING,
                                               G_TYPE_INT,
                                               G_TYPE_BOOLEAN);

  gtk_combo_box_set_model (GTK_COMBO_BOX (self),
                           GTK_TREE_MODEL (self->priv->list_store));
  g_object_unref (self->priv->list_store);

  renderer = gtk_cell_renderer_pixbuf_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self), renderer,
                                  "pixbuf", COL_ICON, NULL);

  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (self), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (self), renderer, "text", COL_MESSAGE, NULL);
  g_object_set (renderer, "width", 130, 
                "ellipsize-set", true, 
                "ellipsize", PANGO_ELLIPSIZE_END, NULL);

  for (int i = 0 ; i < NUM_STATUS_TYPES ; i++)
    custom_status_array [i] = gm_conf_get_string_list (status_types_keys [i]);

  status_menu_populate (self, custom_status_array);

  for (int i = 0 ; i < NUM_STATUS_TYPES ; i++) {
    g_slist_foreach (custom_status_array [i], (GFunc) g_free, 0);
    g_slist_free (custom_status_array [i]);
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (self), 0);

  gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (self),
                                        (GtkTreeViewRowSeparatorFunc) status_menu_row_is_separator,
                                        NULL, NULL);
  gtk_container_set_border_width (GTK_CONTAINER (self), 0);
  status_menu_set_option (self, self->priv->personal_details->get_presence (), self->priv->personal_details->get_status ());

  g_signal_connect (G_OBJECT (self), "changed",
                    G_CALLBACK (status_menu_option_changed), self);

  gm_conf_notifier_add (PERSONAL_DATA_KEY "online_custom_status", 
                        status_menu_custom_messages_changed, self);
  gm_conf_notifier_add (PERSONAL_DATA_KEY "away_custom_status", 
                        status_menu_custom_messages_changed, self);
  gm_conf_notifier_add (PERSONAL_DATA_KEY "dnd_custom_status", 
                        status_menu_custom_messages_changed, self);

  conn = self->priv->personal_details->updated.connect (sigc::bind (sigc::ptr_fun (on_details_updated), self)); 
  self->priv->connections.push_back (conn);

  return GTK_WIDGET (self);
}


void
status_menu_set_parent_window (StatusMenu *self,
                               GtkWindow *parent)
{
  self->priv->parent = parent;
}
