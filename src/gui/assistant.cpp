
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
 *                          assistant.cpp  -  description
 *                          -----------------------------
 *   begin                : Mon May 1 2002
 *   copyright            : (C) 2000-2008 by Damien Sandras
 *                          (C) 2008 by Steve Frécinaux
 *   description          : This file contains all the functions needed to
 *                          build the druid.
 */

#include <glib/gi18n.h>

#include "framework/services.h"
#include "account-core.h"
#include "account.h"

#include "ekiga.h"
#include "gmconf.h"
#include "misc.h"
#include "toolbox/toolbox.h"
#include "assistant.h"

#include "opal-bank.h"
#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"
#include <gdk/gdkkeysyms.h>

G_DEFINE_TYPE (EkigaAssistant, ekiga_assistant, GTK_TYPE_ASSISTANT);

struct _EkigaAssistantPrivate
{
  Ekiga::ServiceCore *core;
  GdkPixbuf *icon;

  GtkWidget *welcome_page;
  GtkWidget *personal_data_page;
  GtkWidget *ekiga_net_page;
  GtkWidget *ekiga_out_page;
  GtkWidget *connection_type_page;
  GtkWidget *audio_devices_page;
  GtkWidget *video_devices_page;
  GtkWidget *summary_page;

  GtkWidget *name;

  GtkWidget *username;
  GtkWidget *password;
  GtkWidget *skip_ekiga_net;

  GtkWidget *dusername;
  GtkWidget *dpassword;
  GtkWidget *skip_ekiga_out;

  GtkWidget *connection_type;

  GtkWidget *audio_ringer;
  GtkWidget *audio_player;
  GtkWidget *audio_recorder;

  GtkWidget *video_device;

  gint last_active_page;

  GtkListStore *summary_model;
  std::vector<sigc::connection> connections;
};

/* presenting the network connectoin type to the user */
enum {
  CNX_LABEL_COLUMN,
  CNX_CODE_COLUMN
};

/**/
enum {
  SUMMARY_KEY_COLUMN,
  SUMMARY_VALUE_COLUMN
};


static GtkWidget *
create_page (EkigaAssistant       *assistant,
             const gchar          *title,
             GtkAssistantPageType  page_type)
{
  GtkWidget *vbox;

  vbox = gtk_vbox_new (FALSE, 2);

  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);

  gtk_assistant_append_page (GTK_ASSISTANT (assistant), vbox);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), vbox, title);
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), vbox, page_type);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), vbox, assistant->priv->icon);

  if (page_type != GTK_ASSISTANT_PAGE_CONTENT)
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);

  return vbox;
}

static void
set_current_page_complete (GtkAssistant *assistant,
                           gboolean      complete)
{
  gint page_number;
  GtkWidget *current_page;

  page_number = gtk_assistant_get_current_page (assistant);
  current_page = gtk_assistant_get_nth_page (assistant, page_number);
  gtk_assistant_set_page_complete (assistant, current_page, complete);
}

void 
get_audiooutput_devices_list (Ekiga::ServiceCore *core,
                              std::vector<std::string> & device_list);
void 
get_audioinput_devices_list (Ekiga::ServiceCore *core,
                             std::vector<std::string> & device_list);
void 
get_videoinput_devices_list (Ekiga::ServiceCore *core,
                             std::vector<std::string> & device_list);

gchar**
convert_string_list (const std::vector<std::string> & list);

static void
update_combo_box (GtkComboBox         *combo_box,
                  const gchar * const *options,
                  const gchar         *default_value)
{
  GtkTreeIter iter;
  GtkTreeModel *model;

  int i;
  int selected;

  g_return_if_fail (options != NULL);

  model = gtk_combo_box_get_model (combo_box);
  gtk_list_store_clear (GTK_LIST_STORE (model));

  selected = 0;
  for (i = 0; options[i]; i++) {
    if (default_value && strcmp (options[i], default_value) == 0)
      selected = i;

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        0, options [i],
                        1, true, 
                        -1);
  }

  gtk_combo_box_set_active(combo_box, selected);
}

static void
add_combo_box (GtkComboBox         *combo_box,
               const gchar         *option)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gboolean found = FALSE;

  if (!option)
    return;

  model = gtk_combo_box_get_model (combo_box);

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {
      gchar *value_string = NULL;
      GValue value = { 0, {{0}, {0}} };
      gtk_tree_model_get_value (GTK_TREE_MODEL (model), &iter, 0, &value);
      value_string = (gchar *) g_value_get_string (&value);
      if (g_ascii_strcasecmp  (value_string, option) == 0) {
        gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                            1, TRUE,
                            -1);
        g_value_unset(&value);
        found = TRUE;
        break;
      }
      g_value_unset(&value);

    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL (model), &iter));
  }

  if (!found) {
    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                        0, option,
                        1, TRUE,
                        -1);
  }
}

static void
remove_combo_box (GtkComboBox         *combo_box,
                  const gchar         *option)
{
  GtkTreeModel *model;
  GtkTreeIter iter;
  int cpt = 0;
  int active;

  g_return_if_fail (option != NULL);
  model = gtk_combo_box_get_model (combo_box);
  active = gtk_combo_box_get_active (GTK_COMBO_BOX (combo_box));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {
      gchar *value_string = NULL;
      GValue value = { 0, {{0}, {0}} };
      gtk_tree_model_get_value (GTK_TREE_MODEL (model), &iter, 0, &value);
      value_string = (gchar *) g_value_get_string (&value);
      if (g_ascii_strcasecmp  (value_string, option) == 0) {

        if (cpt == active) {
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                              1, FALSE,
                              -1);
        }
        else {
          gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
        }
        g_value_unset(&value);
        break;
      }
      g_value_unset(&value);
      cpt++;

    } while (gtk_tree_model_iter_next(GTK_TREE_MODEL (model), &iter));
  }
}

void on_videoinput_device_added_cb (const Ekiga::VideoInputDevice & device, bool, EkigaAssistant *assistant)
{
  std::string device_string = device.GetString();
  add_combo_box (GTK_COMBO_BOX (assistant->priv->video_device), device_string.c_str());
}

void on_videoinput_device_removed_cb (const Ekiga::VideoInputDevice & device, bool,  EkigaAssistant *assistant)
{
  std::string device_string = device.GetString();
  remove_combo_box (GTK_COMBO_BOX (assistant->priv->video_device),  device_string.c_str());
}

void on_audioinput_device_added_cb (const Ekiga::AudioInputDevice & device, bool, EkigaAssistant *assistant)
{
  std::string device_string = device.GetString();
  add_combo_box (GTK_COMBO_BOX (assistant->priv->audio_recorder), device_string.c_str());
}

void on_audioinput_device_removed_cb (const Ekiga::AudioInputDevice & device, bool, EkigaAssistant *assistant)
{
  std::string device_string = device.GetString();
  remove_combo_box (GTK_COMBO_BOX (assistant->priv->audio_recorder),  device_string.c_str());
}

void on_audiooutput_device_added_cb (const Ekiga::AudioOutputDevice & device, bool, EkigaAssistant *assistant)
{
  std::string device_string = device.GetString();
  add_combo_box (GTK_COMBO_BOX (assistant->priv->audio_player), device_string.c_str());
  add_combo_box (GTK_COMBO_BOX (assistant->priv->audio_ringer), device_string.c_str());
}

void on_audiooutput_device_removed_cb (const Ekiga::AudioOutputDevice & device, bool, EkigaAssistant *assistant)
{
  std::string device_string = device.GetString();
  remove_combo_box (GTK_COMBO_BOX (assistant->priv->audio_player),  device_string.c_str());
  remove_combo_box (GTK_COMBO_BOX (assistant->priv->audio_ringer),  device_string.c_str());
}

static void kind_of_net_changed_nt (G_GNUC_UNUSED gpointer id,
                                         GmConfEntry *, 
                                         gpointer)
{
  gm_conf_set_int (GENERAL_KEY "kind_of_net", NET_CUSTOM);
}

static void
create_welcome_page (EkigaAssistant *assistant)
{
  GtkWidget *label;

  label = gtk_label_new (_("This is the Ekiga general configuration assistant. "
                           "The following steps will set up Ekiga by asking "
                           "a few simple questions.\n\nOnce you have completed "
                           "these steps, you can always change them later by "
                           "selecting Preferences in the Edit menu."));
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_widget_show (label);
  gtk_assistant_append_page (GTK_ASSISTANT (assistant), label);
  gtk_assistant_set_page_title (GTK_ASSISTANT (assistant), label, _("Welcome to Ekiga"));
  gtk_assistant_set_page_type (GTK_ASSISTANT (assistant), label, GTK_ASSISTANT_PAGE_INTRO);
  gtk_assistant_set_page_header_image (GTK_ASSISTANT (assistant), label, assistant->priv->icon);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), label, TRUE);

  assistant->priv->welcome_page = label;
}


static void
name_changed_cb (GtkEntry     *entry,
                 GtkAssistant *assistant)
{
  set_current_page_complete (assistant, (gtk_entry_get_text (entry))[0] != '\0');
}


static void
create_personal_data_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  vbox = create_page (assistant, _("Personal Information"), GTK_ASSISTANT_PAGE_CONTENT);

  /* The user fields */
  label = gtk_label_new (_("Please enter your first name and your surname:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->name = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->name), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->name, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("Your first name and surname will be "
                          "used when connecting to other VoIP and "
                          "videoconferencing software."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  g_signal_connect (G_OBJECT (assistant->priv->name), "changed",
                    G_CALLBACK (name_changed_cb), assistant);

  assistant->priv->personal_data_page = vbox;
  gtk_widget_show_all (vbox);
}


static void
prepare_personal_data_page (EkigaAssistant *assistant)
{
  gchar* full_name = gm_conf_get_string (PERSONAL_DATA_KEY "full_name");

  if (full_name == NULL || strlen (full_name) == 0) {

    g_free (full_name);
    full_name = g_strdup (g_get_real_name ());
  }

  gtk_entry_set_text (GTK_ENTRY (assistant->priv->name), full_name);

  g_free (full_name);
}


static void
apply_personal_data_page (EkigaAssistant *assistant)
{
  GtkEntry *entry = GTK_ENTRY (assistant->priv->name);
  const gchar *full_name = gtk_entry_get_text (entry);

  if (full_name)
    gm_conf_set_string (PERSONAL_DATA_KEY "full_name", full_name);
}


static void
ekiga_net_button_clicked_cb (G_GNUC_UNUSED GtkWidget *button,
                             G_GNUC_UNUSED gpointer data)
{
  gm_open_uri ("http://www.ekiga.net");
}


static void
ekiga_out_new_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
                          gpointer data)
{
  EkigaAssistant *assistant = NULL;

  const char *account = NULL;
  const char *password = NULL;

  assistant = EKIGA_ASSISTANT (data);

  account = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername));
  password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword));

  if (account == NULL || password == NULL)
    return; /* no account configured yet */
  
  gm_open_uri ("https://www.diamondcard.us/exec/voip-login?act=sgn&spo=ekiga");
}


static void
ekiga_out_recharge_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
                               gpointer data)
{
  EkigaAssistant *assistant = NULL;

  const char *account = NULL;
  const char *password = NULL;

  gchar *url = NULL;

  assistant = EKIGA_ASSISTANT (data);

  account = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername));
  password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword));

  if (account == NULL || password == NULL)
    return; /* no account configured yet */
  
  url = g_strdup_printf ("https://www.diamondcard.us/exec/voip-login?accId=%s&pinCode=%s&act=rch&spo=ekiga", account, password);
  gm_open_uri (url);
  g_free (url);
}


static void
ekiga_out_history_balance_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
                                      gpointer data)
{
  EkigaAssistant *assistant = NULL;

  const char *account = NULL;
  const char *password = NULL;

  gchar *url = NULL;

  assistant = EKIGA_ASSISTANT (data);

  account = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername));
  password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword));

  if (account == NULL || password == NULL)
    return; /* no account configured yet */
  
  url = g_strdup_printf ("https://www.diamondcard.us/exec/voip-login?accId=%s&pinCode=%s&act=bh&spo=ekiga", account, password);
  gm_open_uri (url);
  g_free (url);
}


static void
ekiga_out_history_calls_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
                                    gpointer data)
{
  EkigaAssistant *assistant = NULL;

  const char *account = NULL;
  const char *password = NULL;

  gchar *url = NULL;

  assistant = EKIGA_ASSISTANT (data);

  account = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername));
  password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword));

  if (account == NULL || password == NULL)
    return; /* no account configured yet */
  
  url = g_strdup_printf ("https://www.diamondcard.us/exec/voip-login?accId=%s&pinCode=%s&act=ch&spo=ekiga", account, password);
  gm_open_uri (url);
  g_free (url);
}


static void
ekiga_net_info_changed_cb (G_GNUC_UNUSED GtkWidget *w,
                           EkigaAssistant *assistant)
{
  gboolean complete;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net)))
    complete = TRUE;
  else {
    const char *username = gtk_entry_get_text (GTK_ENTRY (assistant->priv->username));
    const char *password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->password));
    complete = strcmp(username, "") != 0 && strcmp(password, "") != 0;
  }

  set_current_page_complete (GTK_ASSISTANT (assistant), complete);
}


static void
ekiga_out_info_changed_cb (G_GNUC_UNUSED GtkWidget *w,
                           EkigaAssistant *assistant)
{
  gboolean complete;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_out)))
    complete = TRUE;
  else {
    const char *username = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername));
    const char *password = gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword));
    complete = strcmp(username, "") != 0 && strcmp(password, "") != 0;
  }

  set_current_page_complete (GTK_ASSISTANT (assistant), complete);
}


static void
create_ekiga_net_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;
  GtkWidget *button;
  GtkWidget *align;

  vbox = create_page (assistant, _("Ekiga.net Account"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please enter your username:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->username = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->username), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->username, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please enter your password:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->password = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->password), TRUE);
  gtk_entry_set_visibility (GTK_ENTRY (assistant->priv->password), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->password, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The username and password are used "
                          "to login to your existing account at the ekiga.net "
                          "free SIP service. If you do not have an ekiga.net "
                          "SIP address yet, you may first create an account "
                          "below. This will provide a SIP address that allows "
                          "people to call you.\n\nYou may skip this step if "
                          "you use an alternative SIP service, or if you "
                          "would prefer to specify the login details later."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
                          _("Get an Ekiga.net SIP account"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 10);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (ekiga_net_button_clicked_cb), NULL);

  assistant->priv->skip_ekiga_net = gtk_check_button_new ();
  label = gtk_label_new (_("I do not want to sign up for the ekiga.net free service"));
  gtk_container_add (GTK_CONTAINER (assistant->priv->skip_ekiga_net), label);
  align = gtk_alignment_new (0, 1.0, 0, 0);
  gtk_container_add (GTK_CONTAINER (align), assistant->priv->skip_ekiga_net);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  g_signal_connect (assistant->priv->username, "changed",
                    G_CALLBACK (ekiga_net_info_changed_cb), assistant);
  g_signal_connect (assistant->priv->password, "changed",
                    G_CALLBACK (ekiga_net_info_changed_cb), assistant);
  g_signal_connect (assistant->priv->skip_ekiga_net, "toggled",
                    G_CALLBACK (ekiga_net_info_changed_cb), assistant);

  assistant->priv->ekiga_net_page = vbox;
  gtk_widget_show_all (vbox);
}


static void
prepare_ekiga_net_page (EkigaAssistant *assistant)
{
  gmref_ptr<Opal::Bank> bank = assistant->priv->core->get ("opal-account-store");
  Opal::AccountPtr account = bank->find_account ("ekiga.net");

  if (account && !account->get_username ().empty ())
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->username), account->get_username ().c_str ());
  if (account && !account->get_password ().empty ())
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->password), account->get_password ().c_str ());

  set_current_page_complete (GTK_ASSISTANT (assistant),
                             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net))
                             || (account && !account->get_username ().empty () && !account->get_password ().empty ()));
}


static void
apply_ekiga_net_page (EkigaAssistant *assistant)
{
  gmref_ptr<Opal::Bank> bank = assistant->priv->core->get ("opal-account-store");
  Opal::AccountPtr account = bank->find_account ("ekiga.net");
  bool new_account = !account;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net))) {
	if (new_account)
	  bank->new_account (Opal::Account::Ekiga,
			     gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)),
			     gtk_entry_get_text (GTK_ENTRY (assistant->priv->password)));
	else
	  account->set_authentication_settings (gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)),
											gtk_entry_get_text (GTK_ENTRY (assistant->priv->password)));
  }
}


static void
create_ekiga_out_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;
  GtkWidget *button;
  GtkWidget *align;

  vbox = create_page (assistant, _("Ekiga Call Out Account"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please enter your account ID:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->dusername = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->dusername), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->dusername, FALSE, FALSE, 0);

  label = gtk_label_new (_("Please enter your PIN code:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->dpassword = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (assistant->priv->dpassword), TRUE);
  gtk_entry_set_visibility (GTK_ENTRY (assistant->priv->dpassword), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->dpassword, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>",
                          _("You can make calls to regular phones and cell numbers worldwide using Ekiga.\n\n"
                            "To enable this, you need to do two things:\n"
                            "- First buy an account at the URL below.\n"
                            "- Then enter your account ID and PIN code.\n"
                            "The service will work only if your account is created using the URL in this dialog.\n"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
                          _("Get an Ekiga Call Out account"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (ekiga_out_new_clicked_cb), assistant);

  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Recharge the account"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (ekiga_out_recharge_clicked_cb), assistant);

  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Consult the balance history"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (ekiga_out_history_balance_clicked_cb), assistant);

  button = gtk_button_new ();
  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
			 _("Consult the calls history"));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (button), label);
  gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (button), FALSE, FALSE, 0);
  g_signal_connect (button, "clicked",
                    G_CALLBACK (ekiga_out_history_calls_clicked_cb), assistant);

  assistant->priv->skip_ekiga_out = gtk_check_button_new ();
  label = gtk_label_new (_("I do not want to sign up for the Ekiga Call Out service"));
  gtk_container_add (GTK_CONTAINER (assistant->priv->skip_ekiga_out), label);
  align = gtk_alignment_new (0, 1.0, 0, 0);
  gtk_container_add (GTK_CONTAINER (align), assistant->priv->skip_ekiga_out);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  g_signal_connect (assistant->priv->dusername, "changed",
                    G_CALLBACK (ekiga_out_info_changed_cb), assistant);
  g_signal_connect (assistant->priv->dpassword, "changed",
                    G_CALLBACK (ekiga_out_info_changed_cb), assistant);
  g_signal_connect (assistant->priv->skip_ekiga_out, "toggled",
                    G_CALLBACK (ekiga_out_info_changed_cb), assistant);

  assistant->priv->ekiga_out_page = vbox;
  gtk_widget_show_all (vbox);
}


static void
prepare_ekiga_out_page (EkigaAssistant *assistant)
{
  gmref_ptr<Opal::Bank> account_core = assistant->priv->core->get ("opal-account-store");
  Opal::AccountPtr account = account_core->find_account ("sip.diamondcard.us");

  if (account && !account->get_username ().empty ())
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->dusername), account->get_username ().c_str ());
  if (account && !account->get_password ().empty ())
    gtk_entry_set_text (GTK_ENTRY (assistant->priv->dpassword), account->get_password ().c_str ());

  set_current_page_complete (GTK_ASSISTANT (assistant),
                             gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_out))
                             || (account && !account->get_username ().empty () && !account->get_password ().empty ()));
}


static void
apply_ekiga_out_page (EkigaAssistant *assistant)
{
  /* Some specific Opal stuff for the Ekiga.net account */
  gmref_ptr<Opal::Bank> bank = assistant->priv->core->get ("opal-account-store");
  Opal::AccountPtr account = bank->find_account ("sip.diamondcard.us");
  bool new_account = !account;

  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_out))) {
	if (new_account)
	  bank->new_account (Opal::Account::DiamondCard,
			     gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername)),
			     gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword)));
	else
	  account->set_authentication_settings (gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername)),
						gtk_entry_get_text (GTK_ENTRY (assistant->priv->dpassword)));
  }
}


static void
create_connection_type_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  GtkListStore *store;
  GtkCellRenderer *cell;
  GtkTreeIter iter;

  vbox = create_page (assistant, _("Connection Type"), GTK_ASSISTANT_PAGE_CONTENT);

  /* The connection type */
  label = gtk_label_new (_("Please choose your connection type:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  assistant->priv->connection_type = gtk_combo_box_new_with_model (GTK_TREE_MODEL (store));
  g_object_unref (store);
  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (assistant->priv->connection_type), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (assistant->priv->connection_type), cell,
                                  "text", CNX_LABEL_COLUMN,
                                  NULL);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->connection_type, FALSE, FALSE, 0);

  /* Fill the model with available connection types */
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("56k Modem"),
                      CNX_CODE_COLUMN, NET_PSTN,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("ISDN"),
                      CNX_CODE_COLUMN, NET_ISDN,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("DSL/Cable (128 kbit/s uplink)"),
                      CNX_CODE_COLUMN, NET_DSL128,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("DSL/Cable (512 kbit/s uplink)"),
                      CNX_CODE_COLUMN, NET_DSL512,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("LAN"),
                      CNX_CODE_COLUMN, NET_LAN,
                      -1);
  gtk_list_store_append (store, &iter);
  gtk_list_store_set (store, &iter,
                      CNX_LABEL_COLUMN, _("Keep current settings"),
                      CNX_CODE_COLUMN, NET_CUSTOM,
                      -1);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The connection type will permit "
                          "determining the best quality settings that Ekiga "
                          "will use during calls. You can later change the "
                          "settings individually in the preferences window."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  assistant->priv->connection_type_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}

static void
prepare_connection_type_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box = GTK_COMBO_BOX (assistant->priv->connection_type);
  GtkTreeModel *model = gtk_combo_box_get_model (combo_box);
  GtkTreeIter iter;
  gint connection_type = gm_conf_get_int (GENERAL_KEY "kind_of_net");

  if (gtk_tree_model_get_iter_first (model, &iter)) {
    do {
      gint code;
      gtk_tree_model_get (model, &iter, CNX_CODE_COLUMN, &code, -1);
      if (code == connection_type) {
        gtk_combo_box_set_active_iter (combo_box, &iter);
        break;
      }
    } while (gtk_tree_model_iter_next (model, &iter));
  }
}

static void
apply_connection_type_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box = GTK_COMBO_BOX (assistant->priv->connection_type);
  GtkTreeModel *model = gtk_combo_box_get_model (combo_box);
  GtkTreeIter iter;
  gint connection_type = NET_CUSTOM;

  if (gtk_combo_box_get_active_iter (combo_box, &iter))
    gtk_tree_model_get (model, &iter, CNX_CODE_COLUMN, &connection_type, -1);

  /* Set the connection quality settings */
  switch (connection_type) {
    case NET_PSTN:
    case NET_ISDN:
      gm_conf_set_int (VIDEO_DEVICES_KEY "size", 0); //QCIF
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 32);
      break;

    case NET_DSL128:
      gm_conf_set_int (VIDEO_DEVICES_KEY "size", 0); //QCIF
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 64);
      break;

    case NET_DSL512:
      gm_conf_set_int (VIDEO_DEVICES_KEY "size", 3); // 320x240
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 384);
      break;

    case NET_LAN:
      gm_conf_set_int (VIDEO_DEVICES_KEY "size", 3); // 320x240
      gm_conf_set_int (VIDEO_CODECS_KEY "maximum_video_tx_bitrate", 1024);
      break;

    case NET_CUSTOM:
    default:
      break; /* don't touch anything */
  }

  gm_conf_set_int (GENERAL_KEY "kind_of_net", connection_type);
}


static void
create_audio_devices_page (EkigaAssistant *assistant)
{
  GtkListStore *model;
  GtkWidget *vbox;
  GtkWidget *label;

  GtkCellRenderer *renderer;
  
  gchar *text;

  vbox = create_page (assistant, _("Audio Devices"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please choose the audio ringing device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_BOOLEAN);
  assistant->priv->audio_ringer = gtk_combo_box_new_with_model (GTK_TREE_MODEL (model));
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (assistant->priv->audio_ringer), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (assistant->priv->audio_ringer), renderer,
                                  "text", 0,
                                  "sensitive", 1,
                                  NULL);
  g_object_set (G_OBJECT (renderer), 
                "ellipsize-set", TRUE, 
                "ellipsize", PANGO_ELLIPSIZE_END, 
                "width-chars", 45, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), assistant->priv->audio_ringer);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->audio_ringer, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio ringing device is the device that will be used to play the ringing sound on incoming calls."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  //---
  label = gtk_label_new (_("Please choose the audio output device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_BOOLEAN);
  assistant->priv->audio_player = gtk_combo_box_new_with_model (GTK_TREE_MODEL (model));
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (assistant->priv->audio_player), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (assistant->priv->audio_player), renderer,
                                  "text", 0,
                                  "sensitive", 1,
                                  NULL);
  g_object_set (G_OBJECT (renderer), 
                "ellipsize-set", TRUE, 
                "ellipsize", PANGO_ELLIPSIZE_END, 
                "width-chars", 45, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), assistant->priv->audio_player);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->audio_player, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio output device is the device that will be used to play audio during calls."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  //---
  label = gtk_label_new (_("Please choose the audio input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_BOOLEAN);
  assistant->priv->audio_recorder = gtk_combo_box_new_with_model (GTK_TREE_MODEL (model));
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (assistant->priv->audio_recorder), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (assistant->priv->audio_recorder), renderer,
                                  "text", 0,
                                  "sensitive", 1,
                                  NULL);
  g_object_set (G_OBJECT (renderer), 
                "ellipsize-set", TRUE, 
                "ellipsize", PANGO_ELLIPSIZE_END, 
                "width-chars", 45, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), assistant->priv->audio_recorder);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->audio_recorder, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The audio input device is the device that will be used to record your voice during calls."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  label = gtk_label_new ("");
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  assistant->priv->audio_devices_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}


static void
prepare_audio_devices_page (EkigaAssistant *assistant)
{
  gchar *ringer;
  gchar *player;
  gchar *recorder;
  PStringArray devices;
  char **array;

  ringer = gm_conf_get_string (SOUND_EVENTS_KEY "output_device");
  if (ringer == NULL || !ringer[0])
    ringer = g_strdup (get_default_audio_device_name ());

  player = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
  if (player == NULL || !player[0])
    player = g_strdup (get_default_audio_device_name ());

  recorder = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
  if (recorder == NULL || !recorder[0])
    recorder = g_strdup (get_default_audio_device_name ());

  /* FIXME: We should use DetectDevices, however DetectDevices
   * works only for the currently selected audio and video plugins,
   * not for a random one.
   */
  std::vector <std::string> device_list;

  get_audiooutput_devices_list (assistant->priv->core, device_list);
  array = convert_string_list(device_list);
  update_combo_box (GTK_COMBO_BOX (assistant->priv->audio_ringer), array, ringer);
  update_combo_box (GTK_COMBO_BOX (assistant->priv->audio_player), array, player);
  g_free (array);


  get_audioinput_devices_list (assistant->priv->core, device_list);
  array = convert_string_list(device_list);
  update_combo_box (GTK_COMBO_BOX (assistant->priv->audio_recorder), array, recorder);
  g_free (array);

  g_free (ringer);
  g_free (player);
  g_free (recorder);
}


static void
apply_audio_devices_page (EkigaAssistant *assistant)
{
  GtkComboBox *combo_box;
  gchar *device;

  combo_box = GTK_COMBO_BOX (assistant->priv->audio_ringer);
  device = gtk_combo_box_get_active_text (combo_box);
  if (device) {
    gm_conf_set_string (SOUND_EVENTS_KEY "output_device", device);
    g_free (device);
  }

  combo_box = GTK_COMBO_BOX (assistant->priv->audio_player);
  device = gtk_combo_box_get_active_text (combo_box);
  if (device) {
    gm_conf_set_string (AUDIO_DEVICES_KEY "output_device", device);
    g_free (device);
  }

  combo_box = GTK_COMBO_BOX (assistant->priv->audio_recorder);
  device = gtk_combo_box_get_active_text (combo_box);
  if (device) {
    gm_conf_set_string (AUDIO_DEVICES_KEY "input_device", device);
    g_free (device);
  }

}


static void
create_video_devices_page (EkigaAssistant *assistant)
{
  GtkListStore *model;
  GtkCellRenderer *renderer;
  GtkWidget *vbox;
  GtkWidget *label;
  gchar *text;

  vbox = create_page (assistant, _("Video input device"), GTK_ASSISTANT_PAGE_CONTENT);

  label = gtk_label_new (_("Please choose your video input device:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_BOOLEAN);
  assistant->priv->video_device = gtk_combo_box_new_with_model (GTK_TREE_MODEL (model));
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (assistant->priv->video_device), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (assistant->priv->video_device), renderer,
                                  "text", 0,
                                  "sensitive", 1,
                                  NULL);
  g_object_set (G_OBJECT (renderer), 
                "ellipsize-set", TRUE, 
                "ellipsize", PANGO_ELLIPSIZE_END, 
                "width-chars", 45, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), assistant->priv->video_device);
  gtk_box_pack_start (GTK_BOX (vbox), assistant->priv->video_device, FALSE, FALSE, 0);

  label = gtk_label_new (NULL);
  text = g_strdup_printf ("<i>%s</i>", _("The video input device is the device that will be used to capture video during calls."));
  gtk_label_set_markup (GTK_LABEL (label), text);
  g_free (text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, TRUE, 0);

  assistant->priv->video_devices_page = vbox;
  gtk_widget_show_all (vbox);
  gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant), vbox, TRUE);
}

static void
prepare_video_devices_page (EkigaAssistant *assistant)
{
  std::vector <std::string> device_list;
  gchar** array;
  gchar* current_plugin;

  get_videoinput_devices_list (assistant->priv->core, device_list);
  array = convert_string_list (device_list);
  current_plugin = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
  if (current_plugin == NULL || !current_plugin[0]) {
    g_free (current_plugin);
    current_plugin = g_strdup (get_default_video_device_name (array));
  }
  update_combo_box (GTK_COMBO_BOX (assistant->priv->video_device),
                    array, current_plugin);
  g_free (array);
  g_free (current_plugin);
}

static void
apply_video_devices_page (EkigaAssistant *assistant)
{

  GtkComboBox *combo_box;
  gchar *video_device;

  combo_box = GTK_COMBO_BOX (assistant->priv->video_device);
  video_device = gtk_combo_box_get_active_text (combo_box);

  if (video_device) {
    gm_conf_set_string (VIDEO_DEVICES_KEY "input_device", video_device);
    g_free (video_device);
  }
}


// FIXME: duplicate to gm_prefs_window_get_video_devices_list
void 
get_audiooutput_devices_list (Ekiga::ServiceCore *core,
                              std::vector<std::string> & device_list)
{
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = core->get ("audiooutput-core");
  std::vector <Ekiga::AudioOutputDevice> devices;

  device_list.clear();
  audiooutput_core->get_devices(devices);

  for (std::vector<Ekiga::AudioOutputDevice>::iterator iter = devices.begin ();
       iter != devices.end ();
       iter++) {

    device_list.push_back(iter->GetString());
  }

  if (device_list.size() == 0) {
    device_list.push_back(_("No device found"));
  }
}


void 
get_audioinput_devices_list (Ekiga::ServiceCore *core,
                             std::vector<std::string> & device_list)
{
  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = core->get ("audioinput-core");
  std::vector <Ekiga::AudioInputDevice> devices;

  device_list.clear();
  audioinput_core->get_devices(devices);

  for (std::vector<Ekiga::AudioInputDevice>::iterator iter = devices.begin ();
       iter != devices.end ();
       iter++) {

    device_list.push_back(iter->GetString());
  }

  if (device_list.size() == 0) {
    device_list.push_back(_("No device found"));
  }
}


void 
get_videoinput_devices_list (Ekiga::ServiceCore *core,
                                        std::vector<std::string> & device_list)
{
  gmref_ptr<Ekiga::VideoInputCore> videoinput_core
    = core->get ("videoinput-core");
  std::vector<Ekiga::VideoInputDevice> devices;

  device_list.clear();
  videoinput_core->get_devices(devices);

  for (std::vector<Ekiga::VideoInputDevice>::iterator iter = devices.begin ();
       iter != devices.end ();
       iter++) {

    device_list.push_back(iter->GetString());
  }

  if (device_list.size () == 0) {
    device_list.push_back(_("No device found"));
  }
}


// FIXME: duplicate to gm_prefs_window_convert_string_list
gchar**
convert_string_list (const std::vector<std::string> & list)
{
  gchar **array = NULL;
  unsigned i;

  array = (gchar**) g_malloc (sizeof(gchar*) * (list.size() + 1));
  for (i = 0; i < list.size(); i++)
    array[i] = (gchar*) list[i].c_str();
  array[i] = NULL;

  return array;
}


static void
create_summary_page (EkigaAssistant *assistant)
{
  GtkWidget *vbox;
  GtkWidget *label;
  GtkWidget *sw;
  GtkWidget *tree;
  GtkTreeViewColumn *column;
  GtkCellRenderer *cell;

  vbox = create_page (assistant, _("Configuration Complete"), GTK_ASSISTANT_PAGE_CONFIRM);

  label = gtk_label_new (_("You have now finished the Ekiga configuration. All "
                         "the settings can be changed in the Ekiga preferences. "
                         "Enjoy!"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Configuration summary:"));
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  GtkWidget *align = gtk_alignment_new (0.5, 0.0, 0.0, 1.0);
  gtk_box_pack_start (GTK_BOX (vbox), align, TRUE, TRUE, 0);

  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (align), sw);

  assistant->priv->summary_model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);

  tree = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree),
                           GTK_TREE_MODEL (assistant->priv->summary_model));
  gtk_container_add (GTK_CONTAINER (sw), tree);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Option", cell,
                                                     "text", SUMMARY_KEY_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  cell = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Value", cell,
                                                     "text", SUMMARY_VALUE_COLUMN,
                                                     NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);

  assistant->priv->summary_page = vbox;
  gtk_widget_show_all (vbox);
}


static void 
prepare_summary_page (EkigaAssistant *assistant)
{
  GtkListStore *model = assistant->priv->summary_model;
  GtkTreeIter iter;
  GtkTreeIter citer;
  gchar *value;

  gtk_list_store_clear (model);

  /* The full name */
  gtk_list_store_append (model, &iter);
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, _("Full Name"),
                      SUMMARY_VALUE_COLUMN, gtk_entry_get_text (GTK_ENTRY (assistant->priv->name)),
                      -1);

  /* The connection type */
  if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX (assistant->priv->connection_type), &citer)) {
    GtkTreeModel *cmodel = gtk_combo_box_get_model (GTK_COMBO_BOX (assistant->priv->connection_type));
    gtk_tree_model_get (cmodel, &citer, CNX_LABEL_COLUMN, &value, -1);

    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter,
                        SUMMARY_KEY_COLUMN, _("Connection Type"),
                        SUMMARY_VALUE_COLUMN, value,
                        -1);
    g_free (value);
  }

  /* The audio ringing device */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_ringer));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, _("Audio Ringing Device"),
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The audio playing device */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_player));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, _("Audio Output Device"),
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The audio recording device */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->audio_recorder));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, _("Audio Input Device"),
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The video manager */
  gtk_list_store_append (model, &iter);
  value = gtk_combo_box_get_active_text (GTK_COMBO_BOX (assistant->priv->video_device));
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, _("Video Input Device"),
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);

  /* The ekiga.net account */
  gtk_list_store_append (model, &iter);
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_net)))
    value = g_strdup_printf ("sip:%s@ekiga.net",
                             gtk_entry_get_text (GTK_ENTRY (assistant->priv->username)));
  else
	value = g_strdup ("None");
  gtk_list_store_set (model, &iter,
					  SUMMARY_KEY_COLUMN, _("SIP URI"),
					  SUMMARY_VALUE_COLUMN, value,
					  -1);
  g_free (value);

  /* Ekiga Call Out */
  gtk_list_store_append (model, &iter);
  if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (assistant->priv->skip_ekiga_out)))
    value = g_strdup (gtk_entry_get_text (GTK_ENTRY (assistant->priv->dusername)));
  else
	value = g_strdup ("None");
  gtk_list_store_set (model, &iter,
                      SUMMARY_KEY_COLUMN, _("Ekiga Call Out"),
                      SUMMARY_VALUE_COLUMN, value,
                      -1);
  g_free (value);
}


static void
ekiga_assistant_init (EkigaAssistant *assistant)
{
  assistant->priv = G_TYPE_INSTANCE_GET_PRIVATE (assistant, EKIGA_TYPE_ASSISTANT,
                                                 EkigaAssistantPrivate);

  gtk_window_set_default_size (GTK_WINDOW (assistant), 500, 300);
  gtk_window_set_position (GTK_WINDOW (assistant), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (assistant), 12);

  assistant->priv->last_active_page = 0;
  assistant->priv->icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
                                                    PACKAGE_NAME, 48,
                                                    (GtkIconLookupFlags) 0, NULL);

  create_welcome_page (assistant);
  create_personal_data_page (assistant);
  create_ekiga_net_page (assistant);
  create_ekiga_out_page (assistant);
  create_connection_type_page (assistant);
  create_audio_devices_page (assistant);
  create_video_devices_page (assistant);
  create_summary_page (assistant);

  /* FIXME: what the hell is it needed for? */
  g_object_set_data (G_OBJECT (assistant), "window_name", (gpointer) "assistant");
}


static void
ekiga_assistant_prepare (GtkAssistant *gtkassistant,
                         GtkWidget    *page)
{
  EkigaAssistant *assistant = EKIGA_ASSISTANT (gtkassistant);
  gchar *title = NULL;
  bool forward = false;

  title = g_strdup_printf (_("Ekiga Configuration Assistant (%d of %d)"),
                           gtk_assistant_get_current_page (gtkassistant) + 1,
                           gtk_assistant_get_n_pages (gtkassistant));

  gtk_window_set_title (GTK_WINDOW (assistant), title);
  g_free (title);

  if (assistant->priv->last_active_page < gtk_assistant_get_current_page (gtkassistant))
    forward = true;
  assistant->priv->last_active_page = gtk_assistant_get_current_page (gtkassistant);

  if (!forward)
    return;

  if (page == assistant->priv->personal_data_page) {
    prepare_personal_data_page (assistant);
    return;
  }

  if (page == assistant->priv->ekiga_net_page) {
    prepare_ekiga_net_page (assistant);
    return;
  }

  if (page == assistant->priv->ekiga_out_page) {
    prepare_ekiga_out_page (assistant);
    return;
  }

  if (page == assistant->priv->connection_type_page) {
    prepare_connection_type_page (assistant);
    return;
  }

  if (page == assistant->priv->audio_devices_page) {
    prepare_audio_devices_page (assistant);
    return;
  }

  if (page == assistant->priv->video_devices_page) {
    prepare_video_devices_page (assistant);
    return;
  }

  if (page == assistant->priv->summary_page) {
    prepare_summary_page (assistant);
    return;
  }
}


static void
ekiga_assistant_apply (GtkAssistant *gtkassistant)
{
  EkigaAssistant *assistant = EKIGA_ASSISTANT (gtkassistant);

  GtkWidget *main_window;

  apply_personal_data_page (assistant);
  apply_ekiga_net_page (assistant);
  apply_ekiga_out_page (assistant);
  apply_connection_type_page (assistant);
  apply_audio_devices_page (assistant);
  apply_video_devices_page (assistant);

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  /* Hide the druid and show the main Ekiga window */
  gtk_widget_hide (GTK_WIDGET (assistant));
  gtk_assistant_set_current_page (gtkassistant, 0);
  gtk_widget_show (main_window);
}


static void
ekiga_assistant_cancel (GtkAssistant *gtkassistant)
{
  GtkWidget *main_window;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  gtk_widget_hide (GTK_WIDGET (gtkassistant));
  gtk_assistant_set_current_page (gtkassistant, 0);
  gtk_widget_show (main_window);
}


static void
ekiga_assistant_finalize (GObject *object)
{
  EkigaAssistant *assistant = EKIGA_ASSISTANT (object);

  g_object_unref (assistant->priv->icon);

  G_OBJECT_CLASS (ekiga_assistant_parent_class)->finalize (object);
}


static void
ekiga_assistant_class_init (EkigaAssistantClass *klass)
{
  GtkAssistantClass *assistant_class = GTK_ASSISTANT_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  assistant_class->prepare = ekiga_assistant_prepare;
  assistant_class->apply = ekiga_assistant_apply;
  assistant_class->cancel = ekiga_assistant_cancel;

  object_class->finalize = ekiga_assistant_finalize;

  g_type_class_add_private (klass, sizeof (EkigaAssistantPrivate));
}


static gboolean
ekiga_assistant_key_press_cb (GtkWidget *widget,
			      GdkEventKey *event,
			      G_GNUC_UNUSED gpointer user_data)
{
  if (event->keyval == GDK_Escape) {

    gtk_widget_hide (widget);
    return TRUE;  /* do not propagate the key to parent */
  }

  return FALSE; /* propagate what we don't treat */
}


GtkWidget *
ekiga_assistant_new (Ekiga::ServiceCore *core)
{
  EkigaAssistant *assistant;

  assistant = EKIGA_ASSISTANT (g_object_new (EKIGA_TYPE_ASSISTANT, NULL));
  assistant->priv->core = core;

  /* FIXME: move this into the caller */
  g_signal_connect (assistant, "cancel",
                    G_CALLBACK (gtk_widget_hide), NULL);
  g_signal_connect (assistant, "key-press-event",
                    G_CALLBACK (ekiga_assistant_key_press_cb), NULL);

  sigc::connection conn;
  gmref_ptr<Ekiga::VideoInputCore> videoinput_core
    = core->get ("videoinput-core");
  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = core->get ("audioinput-core");
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = core->get ("audiooutput-core");

  conn = videoinput_core->device_added.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_added_cb), assistant));
  assistant->priv->connections.push_back (conn);
  conn = videoinput_core->device_removed.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_removed_cb), assistant));
  assistant->priv->connections.push_back (conn);

  conn = audioinput_core->device_added.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_added_cb), assistant));
  assistant->priv->connections.push_back (conn);
  conn = audioinput_core->device_removed.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_removed_cb), assistant));
  assistant->priv->connections.push_back (conn);

  conn = audiooutput_core->device_added.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_added_cb), assistant));
  assistant->priv->connections.push_back (conn);
  conn = audiooutput_core->device_removed.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_removed_cb), assistant));
  assistant->priv->connections.push_back (conn);

  /* Notifiers for the VIDEO_CODECS_KEY keys */
  gm_conf_notifier_add (VIDEO_CODECS_KEY "maximum_video_tx_bitrate",
			kind_of_net_changed_nt, NULL);
  gm_conf_notifier_add (VIDEO_CODECS_KEY "temporal_spatial_tradeoff",
			kind_of_net_changed_nt, NULL);

  return GTK_WIDGET (assistant);
}
