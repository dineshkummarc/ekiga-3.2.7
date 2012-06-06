
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
 *                         main_window.cpp  -  description
 *                         -------------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */

#include "revision.h"

#include "config.h"

#include "main.h"

#include "ekiga.h"
#include "conf.h"
#include "misc.h"
#include "callbacks.h"
#include "statusicon.h"
#include "dialpad.h"
#include "statusmenu.h"

#include "gmdialog.h"
#include "gmentrydialog.h"
#include "gmstatusbar.h"
#include "gmconnectbutton.h"
#include "gmstockicons.h"
#include "gmconf.h"
#include "gmref.h"
#include "gmwindow.h"
#include "gmmenuaddon.h"
#include "gmlevelmeter.h"
#include "gmpowermeter.h"
#include "gmconfwidgets.h"
#include "trigger.h"
#include "menu-builder-gtk.h"

#include "platform/gm-platform.h"

#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

#ifdef HAVE_DBUS
#include "dbus-helper/dbus.h"
#endif

#ifndef WIN32
#include <signal.h>
#include <gdk/gdkx.h>
#else
#include "platform/winpaths.h"
#include <gdk/gdkwin32.h>
#include <cstdio>
#endif

#ifdef HAVE_NOTIFY
#include <libnotify/notify.h>
#endif

#if defined(P_FREEBSD) || defined (P_MACOSX)
#include <libintl.h>
#endif

#include <libxml/parser.h>

#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"

#include "call-core.h"
#include "account.h"
#include "gtk-frontend.h"
#include "roster-view-gtk.h"
#include "call-history-view-gtk.h"
#include "history-source.h"
#include "services.h"
#include "form-dialog-gtk.h"

#include <algorithm>

enum CallingState {Standby, Calling, Connected, Called};

enum DeviceType { AudioInput, AudioOutput, VideoInput} ;
struct deviceStruct {
  char name[256];
  DeviceType deviceType;
};

G_DEFINE_TYPE (EkigaMainWindow, ekiga_main_window, GM_WINDOW_TYPE);

struct _EkigaMainWindowPrivate
{
  Ekiga::ServiceCore *core;

  GtkAccelGroup *accel;
  GtkWidget *main_menu;
  GtkWidget *main_notebook;
  GtkWidget *hpaned;

  /* URI Toolbar */
  GtkWidget *main_toolbar;
  GtkWidget *entry;
  GtkListStore *completion;
  GtkWidget *connect_button;

  /* Status Toolbar */
  GtkWidget *status_toolbar;
  GtkWidget *status_option_menu;

  /* Statusbar */
  GtkWidget *statusbar;
  GtkWidget *statusbar_ebox;
  GtkWidget *qualitymeter;

  /* Call panel */
  GtkWidget *call_panel_frame;
  GtkWidget *video_frame;
  GtkWidget *main_video_image;
  GtkWidget *info_text;
  GtkTextTag *status_tag;
  GtkTextTag *codecs_tag;
  GtkTextTag *call_duration_tag;
  GtkWidget *call_panel_toolbar;
  GtkWidget *preview_button;
  GtkWidget *hold_button;
  GtkWidget *audio_settings_button;
  GtkWidget *video_settings_button;
#ifndef WIN32
  GdkGC* video_widget_gc;
#endif

  /* Audio Settings Window */
  GtkWidget *audio_settings_window;
  GtkWidget *audio_input_volume_frame;
  GtkWidget *audio_output_volume_frame;
  GtkWidget *input_signal;
  GtkWidget *output_signal;
  GtkObject *adj_input_volume;
  GtkObject *adj_output_volume;
  unsigned int levelmeter_timeout_id;

  /* Video Settings Window */
  GtkWidget *video_settings_window;
  GtkWidget *video_settings_frame;
  GtkObject *adj_whiteness;
  GtkObject *adj_brightness;
  GtkObject *adj_colour;
  GtkObject *adj_contrast;

  /* Misc Dialogs */
  GtkWidget *transfer_call_popup;

  /* Calls */
  gmref_ptr<Ekiga::Call> current_call;
  unsigned timeout_id;
  unsigned calling_state;
  bool audio_transmission_active;
  bool audio_reception_active;
  bool video_transmission_active;
  bool video_reception_active;
  std::string transmitted_video_codec;
  std::string transmitted_audio_codec;
  std::string received_video_codec;
  std::string received_audio_codec;

  std::list<std::string> accounts;
  Ekiga::Presentity* presentity;

  std::vector<sigc::connection> connections;
};

/* properties */
enum {
  PROP_0,
  PROP_SERVICE_CORE
};

/* channel types */
enum {
  CHANNEL_FIRST,
  CHANNEL_AUDIO,
  CHANNEL_VIDEO,
  CHANNEL_LAST
};


/* GUI Functions */


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the video settings popup of the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static GtkWidget *gm_mw_video_settings_window_new (EkigaMainWindow *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Builds the audio settings popup for the main window.
 * PRE          : The given GtkWidget pointer must be the main window GMObject. 
 */
static GtkWidget *gm_mw_audio_settings_window_new (EkigaMainWindow *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  enables/disables the zoom related menuitems according
 *                 to zoom factor
 * PRE          :  The main window GMObject.
 */
static void ekiga_main_window_zooms_menu_update_sensitivity (EkigaMainWindow *main_window,
							     unsigned int zoom);
static void gm_main_window_toggle_fullscreen (Ekiga::VideoOutputFSToggle toggle,
                                              GtkWidget   *main_window);

static void ekiga_main_window_set_stay_on_top (EkigaMainWindow *mw,
                                               gboolean stay_on_top);

static void ekiga_main_window_show_call_panel (EkigaMainWindow *mw);

static void ekiga_main_window_hide_call_panel (EkigaMainWindow *mw);

void ekiga_main_window_clear_signal_levels (EkigaMainWindow *mw);

static void ekiga_main_window_selected_presentity_build_menu (EkigaMainWindow *mw);

static void ekiga_main_window_incoming_call_dialog_show (EkigaMainWindow *mw,
                                                      gmref_ptr<Ekiga::Call>  call);

#ifdef HAVE_NOTIFY
static void ekiga_main_window_incoming_call_notify (EkigaMainWindow *mw,
                                                    gmref_ptr<Ekiga::Call>  call);
#endif


/* Callbacks */
/* DESCRIPTION  :  This callback is called when the user selects a presentity
 *                 in the roster
 * BEHAVIOR     :  Populates the Chat->Contact submenu
 * PRE          :  /
 */
static void on_presentity_selected (GtkWidget* view,
				    Ekiga::Presentity* presentity,
				    gpointer data);

/* DESCRIPTION  :  This callback is called when the chat window alerts about
 *                 unread messages
 * BEHAVIOR     :  Plays a sound (if enabled)
 * PRE          :  /
 */
static void on_chat_unread_alert (GtkWidget*,
				  gpointer);

/* DESCRIPTION  :  This callback is called when the control panel 
 *                 section key changes.
 * BEHAVIOR     :  Sets the right page, and also sets 
 *                 the good value for the radio menu. 
 * PRE          :  /
 */
static void panel_section_changed_nt (gpointer id,
                                      GmConfEntry *entry,
                                      gpointer data);


/* DESCRIPTION  :  This callback is called when the call panel 
 *                 section key changes.
 * BEHAVIOR     :  Show it or hide it, resize the window appropriately.
 * PRE          :  /
 */
static void show_call_panel_changed_nt (G_GNUC_UNUSED gpointer id, 
                                        GmConfEntry *entry, 
                                        gpointer data);


/* DESCRIPTION  :  This callback is called when the "stay_on_top" 
 *                 config value changes.
 * BEHAVIOR     :  Changes the hint for the video windows.
 * PRE          :  /
 */
static void stay_on_top_changed_nt (G_GNUC_UNUSED gpointer id,
                                    GmConfEntry *entry, 
                                    gpointer data);


/** Pull a trigger from a Ekiga::Service
 *
 * @param data is a pointer to the Ekiga::Trigger
 */
static void pull_trigger_cb (GtkWidget * /*widget*/,
                             gpointer data);


/** Show the widget passed as parameter
 *
 * @param data is a pointer to the widget to show
 */
static void  show_widget_cb (GtkWidget * /*widget*/,
                             gpointer data);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call on hold.
 * PRE          :  /
 */
static void hold_current_call_cb (GtkWidget *,
				  gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call audio channel on pause or not
 * PRE          :  a pointer to the main window 
 */
static void toggle_audio_stream_pause_cb (GtkWidget *, 
                                          gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set the current active call video channel on pause or not
 * PRE          :  a pointer to the main window 
 */
static void toggle_video_stream_pause_cb (GtkWidget *, 
                                          gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a dialog to transfer the current call and transfer
 * 		   it if required.
 * PRE          :  The parent window.
 */
static void transfer_current_call_cb (GtkWidget *,
				      gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 audio settings sliders in the main notebook.
 * BEHAVIOR     :  Update the volume of the choosen mixers. If the update
 *                 fails, the sliders are put back to 0.
 * PRE          :  The main window GMObject.
 */
static void audio_volume_changed_cb (GtkAdjustment *, 
				     gpointer);


/* DESCRIPTION  :  This callback is called when the user changes one of the 
 *                 video settings sliders in the main notebook.
 * BEHAVIOR     :  Updates the value in real time, if it fails, reset 
 * 		   all sliders to 0.
 * PRE          :  gpointer is a valid pointer to the main window GmObject.
 */
static void video_settings_changed_cb (GtkAdjustment *, 
				       gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the
 *                 page in the main notebook.
 * BEHAVIOR     :  Update the config key accordingly.
 * PRE          :  A valid pointer to the main window GmObject.
 */
static void panel_section_changed_cb (GtkNotebook *, 
                                      GtkNotebookPage *,
                                      gint, 
                                      gpointer);


/* DESCRIPTION  :  This callback is called when the user 
 *                 clicks on the dialpad button.
 * BEHAVIOR     :  Generates a dialpad event.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static void dialpad_button_clicked_cb (EkigaDialpad  *dialpad,
				       const gchar *button_text,
				       EkigaMainWindow *main_window);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the application using the window manager.
 * BEHAVIOR     :  Calls the real callback if the notification icon is 
 *                 not shown else hide GM.
 * PRE          :  A valid pointer to the main window GMObject.
 */
static gint window_closed_cb (GtkWidget *, 
			      GdkEvent *, 
			      gpointer);


/* DESCRIPTION  :  This callback is called when the user tries to close
 *                 the main window using the FILE-menu
 * BEHAVIOUR    :  Directly calls window_closed_cb (i.e. it's just a wrapper)
 * PRE          :  ---
 */

static void window_closed_from_menu_cb (GtkWidget *,
                                       gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu, and chooses to zoom in.
 * BEHAVIOR     :  zoom *= 2.
 * PRE          :  The GConf key to update with the new zoom.
 */
static void zoom_in_changed_cb (GtkWidget *,
				gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu, and chooses to zoom in.
 * BEHAVIOR     :  zoom /= 2.
 * PRE          :  The GConf key to update with the new zoom.
 */
static void zoom_out_changed_cb (GtkWidget *,
				 gpointer);


/* DESCRIPTION  :  This callback is called when the user changes the zoom
 *                 factor in the menu, and chooses to zoom in.
 * BEHAVIOR     :  zoom = 1.
 * PRE          :  The GConf key to update with the new zoom.
 */
static void zoom_normal_changed_cb (GtkWidget *,
				    gpointer);

void 
display_changed_cb (GtkWidget *widget,
		    gpointer data);

/* DESCRIPTION  :  This callback is called when the user toggles fullscreen
 *                 factor in the popup menu.
 * BEHAVIOR     :  Toggles the fullscreen configuration key. 
 * PRE          :  / 
 */
static void fullscreen_changed_cb (GtkWidget *,
				   gpointer);

/* DESCRIPTION  :  This callback is called when the user changes the URL
 * 		   in the URL bar.
 * BEHAVIOR     :  It udpates the tooltip with the new URL
 *                 and the completion cache.
 * PRE          :  A valid pointer to the main window GMObject. 
 */
static void url_changed_cb (GtkEditable *, 
			    gpointer);

/* DESCRIPTION  :  This callback is called when the user presses a
 *                 button in the toolbar. 
 *                 (See menu_toggle_changed)
 * BEHAVIOR     :  Updates the config cache.
 * PRE          :  data is the key.
 */
static void toolbar_toggle_button_changed_cb (GtkWidget *, 
					      gpointer);


/* DESCRIPTION  :  This callback is called when the status bar is clicked.
 * BEHAVIOR     :  Clear all info message, not normal messages.
 * PRE          :  The main window GMObject.
 */
static gboolean statusbar_clicked_cb (GtkWidget *,
				      GdkEventButton *,
				      gpointer);


static void audio_volume_window_shown_cb (GtkWidget *widget,
	                                  gpointer data);

static void audio_volume_window_hidden_cb (GtkWidget *widget,
	                                   gpointer data);

static void ekiga_main_window_add_device_dialog_show (EkigaMainWindow *main_window,
                                                      const Ekiga::Device & device,
                                                      DeviceType device_type);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Returns the currently called URL in the URL bar.
 * PRE           : The main window GMObject.
 */
static const std::string ekiga_main_window_get_call_url (EkigaMainWindow *mw);

/* 
 * Engine Callbacks 
 */
static void
on_registration_event (Ekiga::BankPtr /*bank*/,
		       Ekiga::AccountPtr account,
		       Ekiga::Account::RegistrationState state,
		       std::string /*info*/,
		       gpointer self)
{
  EkigaMainWindow *mw = NULL;

  gchar *msg = NULL;
  std::string aor = account->get_aor ();

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (self));
  mw = EKIGA_MAIN_WINDOW (self);

  switch (state) {
  case Ekiga::Account::Registered:
    /* Translators: Is displayed once an account "%s" is registered. */
    msg = g_strdup_printf (_("Registered %s"), aor.c_str ()); 
    if (std::find (mw->priv->accounts.begin (), mw->priv->accounts.end (), account->get_host ()) == mw->priv->accounts.end ())
      mw->priv->accounts.push_back (account->get_host ());
    break;

  case Ekiga::Account::Unregistered:
    /* Translators: Is displayed once an account "%s" is unregistered. */
    msg = g_strdup_printf (_("Unregistered %s"), aor.c_str ());
    mw->priv->accounts.remove (account->get_host ());
    break;

  case Ekiga::Account::UnregistrationFailed:
    msg = g_strdup_printf (_("Could not unregister %s"), aor.c_str ());
    break;

  case Ekiga::Account::RegistrationFailed:
    msg = g_strdup_printf (_("Could not register %s"), aor.c_str ());
    break;

  case Ekiga::Account::Processing:
  default:
    break;
  }

  if (msg)
    ekiga_main_window_flash_message (mw, "%s", msg);

  g_free (msg);
}


static void on_setup_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                              gmref_ptr<Ekiga::Call>  call,
                              gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  if (!call->is_outgoing ()) {
    ekiga_main_window_update_calling_state (mw, Called);
    audiooutput_core->start_play_event ("incoming_call_sound", 4000, 256);
#ifdef HAVE_NOTIFY
    ekiga_main_window_incoming_call_notify (mw, call);
#else
    ekiga_main_window_incoming_call_dialog_show (mw, call);
#endif
    mw->priv->current_call = call;
  }
  else {
    ekiga_main_window_update_calling_state (mw, Calling);
    if (!call->get_remote_uri ().empty ())
      ekiga_main_window_set_call_url (mw, call->get_remote_uri ().c_str());
    mw->priv->current_call = call;
  }
}


static void on_ringing_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                gmref_ptr<Ekiga::Call>  call,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  if (call->is_outgoing ()) {
    audiooutput_core->start_play_event("ring_tone_sound", 3000, 256);
  }
}



static gboolean on_stats_refresh_cb (gpointer self) 
{
  gchar *msg = NULL;

  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  
  if (mw->priv->calling_state == Connected && mw->priv->current_call) {

    Ekiga::VideoOutputStats videooutput_stats;
    gmref_ptr<Ekiga::VideoOutputCore> videooutput_core
      = mw->priv->core->get ("videooutput-core");
    videooutput_core->get_videooutput_stats(videooutput_stats);
  
    msg = g_strdup_printf (_("A:%.1f/%.1f   V:%.1f/%.1f   FPS:%d/%d"), 
                           mw->priv->current_call->get_transmitted_audio_bandwidth (),
                           mw->priv->current_call->get_received_audio_bandwidth (),
                           mw->priv->current_call->get_transmitted_video_bandwidth (),
                           mw->priv->current_call->get_received_video_bandwidth (),
                           videooutput_stats.tx_fps,
                           videooutput_stats.rx_fps);
    ekiga_main_window_flash_message (mw, msg);
    ekiga_main_window_set_call_duration (mw, mw->priv->current_call->get_duration ().c_str ());
    g_free (msg);

    unsigned int jitter = mw->priv->current_call->get_jitter_size ();
    double lost = mw->priv->current_call->get_lost_packets ();
    double late = mw->priv->current_call->get_late_packets ();
    double out_of_order = mw->priv->current_call->get_out_of_order_packets ();

    ekiga_main_window_update_stats (mw, lost, late, out_of_order, jitter, 
                                    videooutput_stats.rx_width,
                                    videooutput_stats.rx_height,
                                    videooutput_stats.tx_width,
                                    videooutput_stats.tx_height);
  }
  return true;
}

static gboolean on_signal_level_refresh_cb (gpointer self) 
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = mw->priv->core->get ("audioinput-core");
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  gm_level_meter_set_level (GM_LEVEL_METER (mw->priv->output_signal), audiooutput_core->get_average_level());
  gm_level_meter_set_level (GM_LEVEL_METER (mw->priv->input_signal), audioinput_core->get_average_level());
  return true;
}

static void on_established_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                    gmref_ptr<Ekiga::Call>  call,
                                    gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gchar* info = NULL;

  info = g_strdup_printf (_("Connected with %s"),
			  call->get_remote_party_name ().c_str ());

  if (!call->get_remote_uri ().empty ())
    ekiga_main_window_set_call_url (mw, call->get_remote_uri ().c_str());
  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top"))
    ekiga_main_window_set_stay_on_top (mw, TRUE);
  ekiga_main_window_set_status (mw, info);
  ekiga_main_window_flash_message (mw, "%s", info);
  if (!gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_call_panel"))
    ekiga_main_window_show_call_panel (mw);
  ekiga_main_window_update_calling_state (mw, Connected);

  mw->priv->current_call = call;

#if GLIB_CHECK_VERSION (2, 14, 0)
  mw->priv->timeout_id = g_timeout_add_seconds (1, on_stats_refresh_cb, self);
#else
  mw->priv->timeout_id = g_timeout_add (1000, on_stats_refresh_cb, self);
#endif

  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  audiooutput_core->stop_play_event("incoming_call_sound");
  audiooutput_core->stop_play_event("ring_tone_sound");

  g_free (info);
}


static void on_cleared_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                gmref_ptr<Ekiga::Call>  call,
                                std::string reason, 
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top"))
    ekiga_main_window_set_stay_on_top (mw, FALSE);
  ekiga_main_window_update_calling_state (mw, Standby);
  ekiga_main_window_set_status (mw, _("Standby"));
  ekiga_main_window_set_call_url (mw, "sip:");
  ekiga_main_window_set_call_duration (mw, NULL);
  ekiga_main_window_set_call_info (mw, NULL, NULL, NULL, NULL);
  if (!gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_call_panel"))
    ekiga_main_window_hide_call_panel (mw);
  ekiga_main_window_clear_stats (mw);
  ekiga_main_window_push_message (mw, "%s", reason.c_str ());
  ekiga_main_window_update_logo_have_window (mw);

  if (mw->priv->current_call && mw->priv->current_call->get_id () == call->get_id ()) {
    mw->priv->current_call = gmref_ptr<Ekiga::Call>(0);
    g_source_remove (mw->priv->timeout_id);
    mw->priv->timeout_id = -1;
  }
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  audiooutput_core->stop_play_event("incoming_call_sound");
  audiooutput_core->stop_play_event("ring_tone_sound");

  ekiga_main_window_clear_signal_levels (mw);

}


static void on_cleared_incoming_call_cb (std::string /*reason*/,
                                         gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (GnomeMeeting::Process ()->GetMainWindow ());

  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");
  audiooutput_core->stop_play_event("incoming_call_sound");
  audiooutput_core->stop_play_event("ring_tone_sound");

#ifdef HAVE_NOTIFY
  notify_notification_close (NOTIFY_NOTIFICATION (self), NULL);
#else
  gtk_widget_destroy (GTK_WIDGET (self));
#endif
}


static void on_missed_incoming_call_cb (gpointer self)
{
#ifdef HAVE_NOTIFY
  notify_notification_close (NOTIFY_NOTIFICATION (self), NULL);
#else
  gtk_widget_destroy (GTK_WIDGET (self));
#endif
}


static void on_held_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                             gmref_ptr<Ekiga::Call>  /*call*/,
                             gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  ekiga_main_window_set_call_hold (mw, true);
  ekiga_main_window_flash_message (mw, "%s", _("Call on hold"));
}


static void on_retrieved_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                  gmref_ptr<Ekiga::Call>  /*call*/,
                                  gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  ekiga_main_window_set_call_hold (mw, false);
  ekiga_main_window_flash_message (mw, "%s", _("Call retrieved"));
}


static void on_missed_call_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                               gmref_ptr<Ekiga::Call>  call,
                               gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  audiooutput_core->stop_play_event ("incoming_call_sound");
  audiooutput_core->stop_play_event ("ring_tone_sound");

  gchar* info = NULL;
  info = g_strdup_printf (_("Missed call from %s"),
			  call->get_remote_party_name ().c_str ());
  ekiga_main_window_push_message (mw, "%s", info);
  g_free (info);

  ekiga_main_window_update_calling_state (mw, Standby);
}


static void on_stream_opened_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                 gmref_ptr<Ekiga::Call>  /* call */,
                                 std::string name,
                                 Ekiga::Call::StreamType type,
                                 bool is_transmitting,
                                 gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  bool is_closing = false;
  bool is_encoding = is_transmitting;
  bool is_video = (type == Ekiga::Call::Video);

  /* FIXME: This should not be needed anymore */
  if (type == Ekiga::Call::Video) {

    is_closing ?
      (is_encoding ? mw->priv->video_transmission_active = false : mw->priv->video_reception_active = false)
      :(is_encoding ? mw->priv->video_transmission_active = true : mw->priv->video_reception_active = true);

    if (is_encoding)
      is_closing ? mw->priv->transmitted_video_codec = "" : mw->priv->transmitted_video_codec = name;
    else
      is_closing ? mw->priv->received_video_codec = "" : mw->priv->received_video_codec = name;
  }
  else {
    
    is_closing ?
      (is_encoding ? mw->priv->audio_transmission_active = false : mw->priv->audio_reception_active = false)
      :(is_encoding ? mw->priv->audio_transmission_active = true : mw->priv->audio_reception_active = true);

    if (is_encoding)
      is_closing ? mw->priv->transmitted_audio_codec = "" : mw->priv->transmitted_audio_codec = name;
    else
      is_closing ? mw->priv->received_audio_codec = "" : mw->priv->received_audio_codec = name;
  }

  ekiga_main_window_update_sensitivity (mw,
                                        is_video,
                                        is_video ? mw->priv->video_reception_active : mw->priv->audio_reception_active,
                                        is_video ? mw->priv->video_transmission_active : mw->priv->audio_transmission_active);
  ekiga_main_window_set_call_info (mw, 
                                   mw->priv->transmitted_audio_codec.c_str (), 
                                   mw->priv->received_audio_codec.c_str (),
                                   mw->priv->transmitted_video_codec.c_str (), 
                                   mw->priv->received_audio_codec.c_str ());
}


static void on_stream_closed_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                 gmref_ptr<Ekiga::Call>  /* call */,
                                 std::string name,
                                 Ekiga::Call::StreamType type,
                                 bool is_transmitting,
                                 gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  bool is_closing = true;
  bool is_encoding = is_transmitting;
  bool is_video = (type == Ekiga::Call::Video);

  /* FIXME: This should not be needed anymore */
  if (type == Ekiga::Call::Video) {
    
    is_closing ?
      (is_encoding ? mw->priv->video_transmission_active = false : mw->priv->video_reception_active = false)
      :(is_encoding ? mw->priv->video_transmission_active = true : mw->priv->video_reception_active = true);

    if (is_encoding)
      is_closing ? mw->priv->transmitted_video_codec = "" : mw->priv->transmitted_video_codec = name;
    else
      is_closing ? mw->priv->received_video_codec = "" : mw->priv->received_video_codec = name;
  }
  else {
    
    is_closing ?
      (is_encoding ? mw->priv->audio_transmission_active = false : mw->priv->audio_reception_active = false)
      :(is_encoding ? mw->priv->audio_transmission_active = true : mw->priv->audio_reception_active = true);

    if (is_encoding)
      is_closing ? mw->priv->transmitted_audio_codec = "" : mw->priv->transmitted_audio_codec = name;
    else
      is_closing ? mw->priv->received_audio_codec = "" : mw->priv->received_audio_codec = name;
  }

  ekiga_main_window_update_sensitivity (mw,
                                        is_video,
                                        is_video ? mw->priv->video_reception_active : mw->priv->audio_reception_active,
                                        is_video ? mw->priv->video_transmission_active : mw->priv->audio_transmission_active);
  ekiga_main_window_set_call_info (mw, 
                                   mw->priv->transmitted_audio_codec.c_str (), 
                                   mw->priv->received_audio_codec.c_str (),
                                   mw->priv->transmitted_video_codec.c_str (), 
                                   mw->priv->received_audio_codec.c_str ());
}


static void on_stream_paused_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                 gmref_ptr<Ekiga::Call>  /*call*/,
                                 std::string /*name*/,
                                 Ekiga::Call::StreamType type,
                                 gpointer self)
{
  ekiga_main_window_set_channel_pause (EKIGA_MAIN_WINDOW (self), true, (type == Ekiga::Call::Video));
}


static void on_stream_resumed_cb (gmref_ptr<Ekiga::CallManager>  /*manager*/,
                                  gmref_ptr<Ekiga::Call>  /*call*/,
                                  std::string /*name*/,
                                  Ekiga::Call::StreamType type,
                                  gpointer self)
{
  ekiga_main_window_set_channel_pause (EKIGA_MAIN_WINDOW (self), false, (type == Ekiga::Call::Video));
}


static bool on_handle_errors (std::string error,
                              gpointer data)
{
  g_return_val_if_fail (data != NULL, false);

  GtkWidget *dialog = gtk_message_dialog_new (GTK_WINDOW (data), 
                                              GTK_DIALOG_MODAL, 
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK, NULL);

  gtk_window_set_title (GTK_WINDOW (dialog), _("Error"));
  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label), error.c_str ());
  
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog));
  
  gtk_widget_show_all (dialog);

  return true;
}



/* 
 * Display Engine Callbacks 
 */

static void 
on_videooutput_device_opened_cb (Ekiga::VideoOutputManager & /* manager */, 
                                 Ekiga::VideoOutputAccel /* accel */, 
                                 Ekiga::VideoOutputMode mode, 
                                 unsigned zoom, 
                                 bool both_streams,
                                 gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (both_streams) {
       gtk_menu_section_set_sensitive (mw->priv->main_menu, "local_video", TRUE);
       gtk_menu_section_set_sensitive (mw->priv->main_menu, "fullscreen", TRUE);
  } 
  else {

    if (mode == Ekiga::VO_MODE_LOCAL)
      gtk_menu_set_sensitive (mw->priv->main_menu, "local_video", TRUE);

    if (mode == Ekiga::VO_MODE_REMOTE)
       gtk_menu_set_sensitive (mw->priv->main_menu, "remote_video", TRUE);
  }

  gtk_radio_menu_select_with_id (mw->priv->main_menu, "local_video", mode);

  gtk_menu_set_sensitive (mw->priv->main_menu, "zoom_in", zoom != 200);
  gtk_menu_set_sensitive (mw->priv->main_menu, "zoom_out", zoom != 50);
  gtk_menu_set_sensitive (mw->priv->main_menu, "normal_size", zoom != 100);
}

void 
on_videooutput_device_closed_cb (Ekiga::VideoOutputManager & /* manager */, gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  gtk_menu_section_set_sensitive (mw->priv->main_menu, "local_video", FALSE);

  gtk_menu_section_set_sensitive (mw->priv->main_menu, "fullscreen", TRUE);

  gtk_menu_section_set_sensitive (mw->priv->main_menu, "zoom_in", FALSE);
}

void 
on_videooutput_device_error_cb (Ekiga::VideoOutputManager & /* manager */, 
                                Ekiga::VideoOutputErrorCodes error_code,
                                gpointer self)
{
  const gchar *dialog_title =  _("Error while initializing video output");
  const gchar *tmp_msg = _("No video will be displayed during this call->");
  gchar *dialog_msg = NULL;

  switch (error_code) {

    case Ekiga::VO_ERROR_NONE:
      break;
    case Ekiga::VO_ERROR:
    default:
#ifdef WIN32  
      dialog_msg = g_strconcat (_("There was an error opening or initializing the video output. Please verify that no other application is using the accelerated video output."), "\n\n", tmp_msg, NULL);
#else
      dialog_msg = g_strconcat (_("There was an error opening or initializing the video output. Please verify that you are using a color depth of 24 or 32 bits per pixel."), "\n\n", tmp_msg, NULL);
#endif      
      break;
  }

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (self),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
  g_free (dialog_msg);
}

void 
on_fullscreen_mode_changed_cb (Ekiga::VideoOutputManager & /* manager */, Ekiga::VideoOutputFSToggle toggle,  gpointer self)
{
  gm_main_window_toggle_fullscreen (toggle, GTK_WIDGET (self));
}

static void
ekiga_main_window_set_video_size (EkigaMainWindow *mw, int width, int height)
{
  int pw, ph;
  GtkWidget *panel;

  g_return_if_fail (width > 0 && height > 0);

  gtk_widget_get_size_request (mw->priv->main_video_image, &pw, &ph);

  /* No size requisition yet
   * It's our first call so we silently set the new requisition and exit...
   */
  if (pw == -1) {
    gtk_widget_set_size_request (mw->priv->main_video_image, width, height);
    return;
  }

  /* Do some kind of filtering here. We often get duplicate "size-changed" events...
   * Note that we currently only bother about the width of the video.
   */
  if (pw == width)
    return;

  panel = gtk_paned_get_child2 (GTK_PANED (mw->priv->hpaned));
#if GTK_CHECK_VERSION(2,18,0)
  if (gtk_widget_get_visible (GTK_WIDGET (panel))) {
#else
  if (GTK_WIDGET_VISIBLE (panel)) {
#endif
    int x, y;
    int rw, pos;
    GtkRequisition req;

    gtk_window_get_size (GTK_WINDOW (mw), &x, &y);
    pos = gtk_paned_get_position (GTK_PANED (mw->priv->hpaned));

    rw = x - panel->allocation.width;

    gtk_widget_set_size_request (mw->priv->main_video_image, width, height);
    gtk_widget_size_request (panel, &req);

    gtk_window_resize (GTK_WINDOW (mw), rw + req.width, y);
    gtk_paned_set_position (GTK_PANED (mw->priv->hpaned), pos);
  }
  else {
    gtk_widget_set_size_request (mw->priv->main_video_image, width, height);
  }

  gdk_window_invalidate_rect (GTK_WIDGET (mw)->window,
                              &(GTK_WIDGET (mw)->allocation), TRUE);
}

static void 
on_size_changed_cb (Ekiga::VideoOutputManager & /* manager */, unsigned width, unsigned height, gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  ekiga_main_window_set_video_size (EKIGA_MAIN_WINDOW (mw), width, height);
}

void
on_videoinput_device_opened_cb (Ekiga::VideoInputManager & /* manager */,
                                Ekiga::VideoInputDevice & /* device */,
                                Ekiga::VideoInputSettings & settings,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  gtk_widget_set_sensitive (mw->priv->video_settings_frame,  settings.modifyable ? TRUE : FALSE);
  gtk_widget_set_sensitive (mw->priv->video_settings_button,  settings.modifyable ? TRUE : FALSE);
  GTK_ADJUSTMENT (mw->priv->adj_whiteness)->value = settings.whiteness;
  GTK_ADJUSTMENT (mw->priv->adj_brightness)->value = settings.brightness;
  GTK_ADJUSTMENT (mw->priv->adj_colour)->value = settings.colour;
  GTK_ADJUSTMENT (mw->priv->adj_contrast)->value = settings.contrast;

  gtk_widget_queue_draw (mw->priv->video_settings_frame);
}


void 
on_videoinput_device_closed_cb (Ekiga::VideoInputManager & /* manager */, Ekiga::VideoInputDevice & /*device*/, gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  ekiga_main_window_update_sensitivity (mw, TRUE, FALSE, FALSE);
  ekiga_main_window_update_logo_have_window (mw);

  gtk_widget_set_sensitive (mw->priv->video_settings_button,  FALSE);
}


void 
on_videoinput_device_added_cb (const Ekiga::VideoInputDevice & device, bool is_desired, gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gchar *message;

  /* Translators: This is a hotplug status */
  message = g_strdup_printf (_("Added video input device %s"),
			       device.GetString().c_str ());
  ekiga_main_window_flash_message (mw, "%s", message);
  g_free (message);
  if (!is_desired && mw->priv->calling_state == Standby && !mw->priv->current_call) 
    ekiga_main_window_add_device_dialog_show (mw, device, VideoInput);
}

void 
on_videoinput_device_removed_cb (const Ekiga::VideoInputDevice & device, bool, gpointer self)
{
  /* Translators: This is a hotplug status */
  gchar *message = g_strdup_printf (_("Removed video input device %s"),
				    device.GetString().c_str ());
  ekiga_main_window_flash_message (EKIGA_MAIN_WINDOW (self), "%s", message);
  g_free (message);
}

void 
on_videoinput_device_error_cb (Ekiga::VideoInputManager & /* manager */, 
                               Ekiga::VideoInputDevice & device, 
                               Ekiga::VideoInputErrorCodes error_code, 
                               gpointer self)
{
  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title =
  g_strdup_printf (_("Error while accessing video device %s"),
                   (const char *) device.name.c_str());

  tmp_msg = g_strdup (_("A moving logo will be transmitted during calls. Notice that you can always transmit a given image or the moving logo by choosing \"Picture\" as video plugin and \"Moving logo\" or \"Static picture\" as device."));
  switch (error_code) {

    case Ekiga::VI_ERROR_DEVICE:
      dialog_msg = g_strconcat (_("There was an error while opening the device. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your permissions and make sure that the appropriate driver is loaded."), "\n\n", tmp_msg, NULL);
      break;

    case Ekiga::VI_ERROR_FORMAT:
      dialog_msg = g_strconcat (_("Your video driver doesn't support the requested video format."), "\n\n", tmp_msg, NULL);
      break;

    case Ekiga::VI_ERROR_CHANNEL:
      dialog_msg = g_strconcat (_("Could not open the chosen channel."), "\n\n", tmp_msg, NULL);
      break;

    case Ekiga::VI_ERROR_COLOUR:
      dialog_msg = g_strconcat (_("Your driver doesn't seem to support any of the color formats supported by Ekiga.\n Please check your kernel driver documentation in order to determine which Palette is supported."), "\n\n", tmp_msg, NULL);
      break;

    case Ekiga::VI_ERROR_FPS:
      dialog_msg = g_strconcat (_("Error while setting the frame rate."), "\n\n", tmp_msg, NULL);
      break;

    case Ekiga::VI_ERROR_SCALE:
      dialog_msg = g_strconcat (_("Error while setting the frame size."), "\n\n", tmp_msg, NULL);
      break;

    case Ekiga::VI_ERROR_NONE:
    default:
      dialog_msg = g_strconcat (_("Unknown error."), "\n\n", tmp_msg, NULL);
      break;
  }

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (GTK_WIDGET (self)),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
  g_free (dialog_msg);
  g_free (dialog_title);
  g_free (tmp_msg);
}

void
on_audioinput_device_opened_cb (Ekiga::AudioInputManager & /* manager */,
                                Ekiga::AudioInputDevice & /* device */,
                                Ekiga::AudioInputSettings & settings,
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  gtk_widget_set_sensitive (mw->priv->audio_input_volume_frame, settings.modifyable);
  gtk_widget_set_sensitive (mw->priv->audio_settings_button, settings.modifyable);
  GTK_ADJUSTMENT (mw->priv->adj_input_volume)->value = settings.volume;
  
  gtk_widget_queue_draw (mw->priv->audio_input_volume_frame);
}



void 
on_audioinput_device_closed_cb (Ekiga::AudioInputManager & /* manager */, 
                                Ekiga::AudioInputDevice & /*device*/, 
                                gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  gtk_widget_set_sensitive (mw->priv->audio_settings_button, FALSE);
  gtk_widget_set_sensitive (mw->priv->audio_input_volume_frame, FALSE);
}

void 
on_audioinput_device_added_cb (const Ekiga::AudioInputDevice & device, 
                               bool is_desired,
                               gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);
  gchar* message = NULL;

  /* Translators: This is a hotplug status */
  message = g_strdup_printf (_("Added audio input device %s"),
			     device.GetString().c_str ());
  ekiga_main_window_flash_message (mw, "%s", message);
  g_free (message);
  if (!is_desired  && mw->priv->calling_state == Standby && !mw->priv->current_call)
    ekiga_main_window_add_device_dialog_show (mw, device,  AudioInput);
    
}

void 
on_audioinput_device_removed_cb (const Ekiga::AudioInputDevice & device, 
                                 bool,
                                 gpointer self)
{
  gchar* message = NULL;

  /* Translators: This is a hotplug status */
  message = g_strdup_printf (_("Removed audio input device %s"),
			     device.GetString().c_str ());
  ekiga_main_window_flash_message (EKIGA_MAIN_WINDOW (self), "%s", message);
  g_free (message);
}

void 
on_audioinput_device_error_cb (Ekiga::AudioInputManager & /* manager */, 
                               Ekiga::AudioInputDevice & device, 
                               Ekiga::AudioInputErrorCodes error_code, 
                               gpointer self)
{
  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title =
  g_strdup_printf (_("Error while opening audio input device %s"),
                   (const char *) device.name.c_str());

  /* Translators: This happens when there is an error with audio input:
   * Nothing ("silence") will be transmitted */
  tmp_msg = g_strdup (_("Only silence will be transmitted."));
  switch (error_code) {

    case Ekiga::AI_ERROR_DEVICE:
      dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unable to open the selected audio device for recording. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup, the permissions and that the device is not busy."), NULL);
      break;

    case Ekiga::AI_ERROR_READ:
      dialog_msg = g_strconcat (tmp_msg, "\n\n", _("The selected audio device was successfully opened but it is impossible to read data from this device. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup."), NULL);
      break;

    case Ekiga::AI_ERROR_NONE:
    default:
      dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unknown error."), NULL);
      break;
  }

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (self),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
  g_free (dialog_msg);
  g_free (dialog_title);
  g_free (tmp_msg);

}

void
on_audiooutput_device_opened_cb (Ekiga::AudioOutputManager & /*manager*/,
                                 Ekiga::AudioOutputPS ps,
                                 Ekiga::AudioOutputDevice & /*device*/,
                                 Ekiga::AudioOutputSettings & settings,
                                 gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (ps == Ekiga::secondary)
    return;

  gtk_widget_set_sensitive (mw->priv->audio_settings_button, settings.modifyable);
  gtk_widget_set_sensitive (mw->priv->audio_output_volume_frame, settings.modifyable);
  GTK_ADJUSTMENT (mw->priv->adj_output_volume)->value = settings.volume;

  gtk_widget_queue_draw (mw->priv->audio_output_volume_frame);
}



void 
on_audiooutput_device_closed_cb (Ekiga::AudioOutputManager & /*manager*/, 
                                 Ekiga::AudioOutputPS ps, 
                                 Ekiga::AudioOutputDevice & /*device*/, 
                                 gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  if (ps == Ekiga::secondary)
    return;

  gtk_widget_set_sensitive (mw->priv->audio_settings_button, FALSE);
  gtk_widget_set_sensitive (mw->priv->audio_output_volume_frame, FALSE);
}

void 
on_audiooutput_device_added_cb (const Ekiga::AudioOutputDevice & device, 
                                bool is_desired,
                                gpointer self)
{
  EkigaMainWindow *mw;
  gchar *message;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (self));

  mw = EKIGA_MAIN_WINDOW (self);

  message = g_strdup_printf (_("Added audio output device %s"), device.GetString().c_str ());
  ekiga_main_window_flash_message (mw, "%s", message);
  g_free (message);
  if (!is_desired && mw->priv->calling_state == Standby && !mw->priv->current_call)
    ekiga_main_window_add_device_dialog_show (mw, device, AudioOutput);
}

void 
on_audiooutput_device_removed_cb (const Ekiga::AudioOutputDevice & device, 
                                  bool,
                                  gpointer self)
{
  gchar *message;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (self));

  message = g_strdup_printf (_("Removed audio output device %s"),
			     device.GetString().c_str ());
  ekiga_main_window_flash_message (EKIGA_MAIN_WINDOW (self), "%s", message);
  g_free (message);
}

void 
on_audiooutput_device_error_cb (Ekiga::AudioOutputManager & /*manager */, 
                                Ekiga::AudioOutputPS ps,
                                Ekiga::AudioOutputDevice & device, 
                                Ekiga::AudioOutputErrorCodes error_code, 
                                gpointer self)
{
  if (ps == Ekiga::secondary)
    return;

  gchar *dialog_title = NULL;
  gchar *dialog_msg = NULL;
  gchar *tmp_msg = NULL;

  dialog_title =
  g_strdup_printf (_("Error while opening audio output device %s"),
                   (const char *) device.name.c_str());

  tmp_msg = g_strdup (_("No incoming sound will be played."));
  switch (error_code) {

    case Ekiga::AO_ERROR_DEVICE:
      dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unable to open the selected audio device for playing. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup, the permissions and that the device is not busy."), NULL);
      break;

    case Ekiga::AO_ERROR_WRITE:
      dialog_msg = g_strconcat (tmp_msg, "\n\n", _("The selected audio device was successfully opened but it is impossible to write data to this device. In case it is a pluggable device it may be sufficient to reconnect it. If not, or if it still is not accessible, please check your audio setup."), NULL);
      break;

    case Ekiga::AO_ERROR_NONE:
    default:
      dialog_msg = g_strconcat (tmp_msg, "\n\n", _("Unknown error."), NULL);
      break;
  }

  gnomemeeting_warning_dialog_on_widget (GTK_WINDOW (GTK_WIDGET (self)),
                                         "show_device_warnings",
                                         dialog_title,
                                         "%s", dialog_msg);
  g_free (dialog_msg);
  g_free (dialog_title);
  g_free (tmp_msg);
}


/* Implementation */
static void
incoming_call_response_cb (GtkDialog *incoming_call_popup,
                           gint response,
                           gpointer main_window)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (main_window);

  gtk_widget_hide (GTK_WIDGET (incoming_call_popup));

  if (mw->priv->current_call) {

    switch (response) {
    case 2:
      mw->priv->current_call->answer ();
      break;

    default:
    case 0:
      mw->priv->current_call->hangup ();
      break;
    }
  }
}

static void
add_device_response_cb (GtkDialog *add_device_popup,
                           gint response,
                           gpointer data)
{
  deviceStruct *device_struct = (deviceStruct*) data;

  gtk_widget_hide (GTK_WIDGET (add_device_popup));

  if (response == 2) {

    switch (device_struct->deviceType)
    {
     case AudioInput:
       gm_conf_set_string (AUDIO_DEVICES_KEY "input_device", device_struct->name);
       break;
     case AudioOutput:
       gm_conf_set_string (AUDIO_DEVICES_KEY "output_device", device_struct->name);
       break;
     case VideoInput:
       gm_conf_set_string (VIDEO_DEVICES_KEY "input_device", device_struct->name);
       break;	                
     default:;
    }
  }
}

static void
place_call_cb (GtkWidget * /*widget*/,
               gpointer data)
{
  std::string uri;
  EkigaMainWindow *mw = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (data));

  mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->calling_state == Standby && !mw->priv->current_call) {

    size_t pos;

    ekiga_main_window_update_calling_state (mw, Calling);
    gmref_ptr<Ekiga::CallCore> call_core = mw->priv->core->get ("call-core");
    uri = ekiga_main_window_get_call_url (mw);
    pos = uri.find ("@");
    if (pos == std::string::npos
	&& uri.find ("h323:") == std::string::npos
	&& mw->priv->accounts.begin () != mw->priv->accounts.end ()) {

      std::list<std::string>::iterator it = mw->priv->accounts.begin ();
      uri = uri + "@" + (*it);
      ekiga_main_window_set_call_url (mw, uri.c_str ());
    }
    pos = uri.find_first_of (' ');
    if (pos != std::string::npos)
      uri = uri.substr (0, pos);
    if (call_core->dial (uri)) {

      pos = uri.find ("@");
      if (pos != std::string::npos) {

        std::string host = uri.substr (pos + 1);
        mw->priv->accounts.remove (host);
        mw->priv->accounts.push_front (host);
      }

    }
    else {
      ekiga_main_window_flash_message (mw, _("Could not connect to remote host"));
      ekiga_main_window_update_calling_state (mw, Standby);
    }
  }
  else if (mw->priv->calling_state == Called && mw->priv->current_call)
    mw->priv->current_call->answer ();
}


static void
hangup_call_cb (GtkWidget * /*widget*/,
                gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->current_call)
    mw->priv->current_call->hangup ();
}


static void 
toggle_call_cb (GtkWidget *widget,
                gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  if (gm_connect_button_get_connected (GM_CONNECT_BUTTON (mw->priv->connect_button)))
    hangup_call_cb (widget, data);
  else {
    if (!mw->priv->current_call)
      place_call_cb (widget, data);
    else
      mw->priv->current_call->answer ();
  }
}


static void
on_status_icon_embedding_change (G_GNUC_UNUSED GObject obj,
				 G_GNUC_UNUSED GParamSpec param,
				 G_GNUC_UNUSED gpointer data)
{
  GtkWidget *main_window = NULL;
  GtkStatusIcon *status_icon = NULL;

  status_icon = GTK_STATUS_ICON (GnomeMeeting::Process ()->GetStatusicon ());
  main_window = GnomeMeeting::Process ()->GetMainWindow ();

  /* force the main window to show if no status icon for the user */
  if (!gtk_status_icon_is_embedded (GTK_STATUS_ICON (status_icon)))
    gtk_widget_show (main_window);
}

	
static GtkWidget * 
gm_mw_video_settings_window_new (EkigaMainWindow *mw)
{
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *image = NULL;
  GtkWidget *window = NULL;

  GtkWidget *hscale_brightness = NULL;
  GtkWidget *hscale_colour = NULL;
  GtkWidget *hscale_contrast = NULL;
  GtkWidget *hscale_whiteness = NULL;

  int brightness = 0, colour = 0, contrast = 0, whiteness = 0;

  /* Build the window */
  window = gtk_dialog_new ();
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("video_settings_window"), g_free); 
  gtk_dialog_add_button (GTK_DIALOG (window), 
                         GTK_STOCK_CLOSE, 
                         GTK_RESPONSE_CANCEL);

  gtk_window_set_title (GTK_WINDOW (window), 
                        _("Video Settings"));

  /* Webcam Control Frame, we need it to disable controls */		
  mw->priv->video_settings_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->priv->video_settings_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->priv->video_settings_frame), 5);
  
  /* Category */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (mw->priv->video_settings_frame), vbox);
  
  /* Brightness */
  hbox = gtk_hbox_new (FALSE, 0);
  image = gtk_image_new_from_icon_name (GM_ICON_BRIGHTNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->priv->adj_brightness = gtk_adjustment_new (brightness, 0.0,
                                                 255.0, 1.0, 5.0, 1.0);
  hscale_brightness = gtk_hscale_new (GTK_ADJUSTMENT (mw->priv->adj_brightness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_brightness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_brightness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_brightness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_brightness, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_widget_set_tooltip_text (hscale_brightness, _("Adjust brightness"));

  g_signal_connect (G_OBJECT (mw->priv->adj_brightness), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) mw);

  /* Whiteness */
  hbox = gtk_hbox_new (FALSE, 0);
  image = gtk_image_new_from_icon_name (GM_ICON_WHITENESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->priv->adj_whiteness = gtk_adjustment_new (whiteness, 0.0, 
						255.0, 1.0, 5.0, 1.0);
  hscale_whiteness = gtk_hscale_new (GTK_ADJUSTMENT (mw->priv->adj_whiteness));
  gtk_range_set_update_policy (GTK_RANGE (hscale_whiteness),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_whiteness), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_whiteness), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_whiteness, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_widget_set_tooltip_text (hscale_whiteness, _("Adjust whiteness"));

  g_signal_connect (G_OBJECT (mw->priv->adj_whiteness), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) mw);

  /* Colour */
  hbox = gtk_hbox_new (FALSE, 0);
  image = gtk_image_new_from_icon_name (GM_ICON_COLOURNESS, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

  mw->priv->adj_colour = gtk_adjustment_new (colour, 0.0, 
					     255.0, 1.0, 5.0, 1.0);
  hscale_colour = gtk_hscale_new (GTK_ADJUSTMENT (mw->priv->adj_colour));
  gtk_range_set_update_policy (GTK_RANGE (hscale_colour),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_colour), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_colour), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_colour, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_widget_set_tooltip_text (hscale_colour, _("Adjust color"));

  g_signal_connect (G_OBJECT (mw->priv->adj_colour), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) mw);

  /* Contrast */
  hbox = gtk_hbox_new (FALSE, 0);
  image = gtk_image_new_from_icon_name (GM_ICON_CONTRAST, GTK_ICON_SIZE_MENU);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
  
  mw->priv->adj_contrast = gtk_adjustment_new (contrast, 0.0, 
					       255.0, 1.0, 5.0, 1.0);
  hscale_contrast = gtk_hscale_new (GTK_ADJUSTMENT (mw->priv->adj_contrast));
  gtk_range_set_update_policy (GTK_RANGE (hscale_contrast),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_draw_value (GTK_SCALE (hscale_contrast), FALSE);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_contrast), GTK_POS_RIGHT);
  gtk_box_pack_start (GTK_BOX (hbox), hscale_contrast, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_widget_set_tooltip_text (hscale_contrast, _("Adjust contrast"));

  g_signal_connect (G_OBJECT (mw->priv->adj_contrast), "value-changed",
		    G_CALLBACK (video_settings_changed_cb), 
		    (gpointer) mw);
  
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), 
                     mw->priv->video_settings_frame);
  gtk_widget_show_all (mw->priv->video_settings_frame);

  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->video_settings_frame), FALSE);
  
  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window), 
		    "delete-event", 
		    G_CALLBACK (delete_window_cb), NULL);

  return window;
}



static GtkWidget * 
gm_mw_audio_settings_window_new (EkigaMainWindow *mw)
{
  GtkWidget *hscale_play = NULL; 
  GtkWidget *hscale_rec = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *small_vbox = NULL;
  GtkWidget *window = NULL;
  
  /* Build the window */
  window = gtk_dialog_new ();
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup ("audio_settings_window"), g_free); 
  gtk_dialog_add_button (GTK_DIALOG (window), 
                         GTK_STOCK_CLOSE, 
                         GTK_RESPONSE_CANCEL);

  gtk_window_set_title (GTK_WINDOW (window), 
                        _("Audio Settings"));

  /* Audio control frame, we need it to disable controls */		
  mw->priv->audio_output_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->priv->audio_output_volume_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->priv->audio_output_volume_frame), 5);


  /* The vbox */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (mw->priv->audio_output_volume_frame), vbox);

  /* Output volume */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), 
		      gtk_image_new_from_icon_name (GM_ICON_AUDIO_VOLUME_HIGH, 
						    GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);
  
  small_vbox = gtk_vbox_new (FALSE, 0);
  mw->priv->adj_output_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_play = gtk_hscale_new (GTK_ADJUSTMENT (mw->priv->adj_output_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_play),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_play), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_play), FALSE);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_play, TRUE, TRUE, 0);

  mw->priv->output_signal = gm_level_meter_new ();
  gtk_box_pack_start (GTK_BOX (small_vbox), mw->priv->output_signal, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), 
                     mw->priv->audio_output_volume_frame);
  gtk_widget_show_all (mw->priv->audio_output_volume_frame);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->audio_output_volume_frame),  FALSE);

  /* Audio control frame, we need it to disable controls */		
  mw->priv->audio_input_volume_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->priv->audio_input_volume_frame), 
			     GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (mw->priv->audio_input_volume_frame), 5);

  /* The vbox */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (mw->priv->audio_input_volume_frame), vbox);

  /* Input volume */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox),
		      gtk_image_new_from_icon_name (GM_ICON_MICROPHONE, 
						    GTK_ICON_SIZE_SMALL_TOOLBAR),
		      FALSE, FALSE, 0);

  small_vbox = gtk_vbox_new (FALSE, 0);
  mw->priv->adj_input_volume = gtk_adjustment_new (0, 0.0, 101.0, 1.0, 5.0, 1.0);
  hscale_rec = gtk_hscale_new (GTK_ADJUSTMENT (mw->priv->adj_input_volume));
  gtk_range_set_update_policy (GTK_RANGE (hscale_rec),
			       GTK_UPDATE_DELAYED);
  gtk_scale_set_value_pos (GTK_SCALE (hscale_rec), GTK_POS_RIGHT); 
  gtk_scale_set_draw_value (GTK_SCALE (hscale_rec), FALSE);
  gtk_box_pack_start (GTK_BOX (small_vbox), hscale_rec, TRUE, TRUE, 0);

  mw->priv->input_signal = gm_level_meter_new ();
  gtk_box_pack_start (GTK_BOX (small_vbox), mw->priv->input_signal, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), small_vbox, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 3);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (window)->vbox), 
                     mw->priv->audio_input_volume_frame);
  gtk_widget_show_all (mw->priv->audio_input_volume_frame);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->audio_input_volume_frame),  FALSE);

  g_signal_connect (G_OBJECT (mw->priv->adj_output_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), mw);

  g_signal_connect (G_OBJECT (mw->priv->adj_input_volume), "value-changed",
		    G_CALLBACK (audio_volume_changed_cb), mw);

  /* That's an usual GtkWindow, connect it to the signals */
  g_signal_connect_swapped (GTK_OBJECT (window), 
			    "response", 
			    G_CALLBACK (gnomemeeting_window_hide),
			    (gpointer) window);

  g_signal_connect (GTK_OBJECT (window),
		    "delete-event", 
		    G_CALLBACK (delete_window_cb), NULL);

  g_signal_connect (G_OBJECT (window), "show", 
                    G_CALLBACK (audio_volume_window_shown_cb), mw);

  g_signal_connect (G_OBJECT (window), "hide", 
                    G_CALLBACK (audio_volume_window_hidden_cb), mw);

  return window;
}



static void
ekiga_main_window_zooms_menu_update_sensitivity (EkigaMainWindow *mw,
                                                 unsigned int zoom)
{
  /* between 0.5 and 2.0 zoom */
  /* like above, also update the popup menus of the separate video windows */
  gtk_menu_set_sensitive (mw->priv->main_menu, "zoom_in", zoom != 200);
  gtk_menu_set_sensitive (mw->priv->main_menu, "zoom_out", zoom != 50);
  gtk_menu_set_sensitive (mw->priv->main_menu, "normal_size", zoom != 100);
}


/* GTK callbacks */
static gint
gnomemeeting_tray_hack_cb (G_GNUC_UNUSED gpointer data)
{
  GtkWidget *main_window = NULL;
  GtkStatusIcon *statusicon = NULL;

  statusicon = GTK_STATUS_ICON (GnomeMeeting::Process ()->GetStatusicon ());
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (!gtk_status_icon_is_embedded (GTK_STATUS_ICON (statusicon))) 
    gtk_widget_show (main_window);
  
  return FALSE;
}


static void
on_presentity_selected (G_GNUC_UNUSED GtkWidget* view,
			Ekiga::Presentity* presentity,
			gpointer self)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (self);

  mw->priv->presentity = presentity;

  ekiga_main_window_selected_presentity_build_menu (mw);
}


static void
on_chat_unread_alert (G_GNUC_UNUSED GtkWidget* widget,
		      G_GNUC_UNUSED gpointer data)
{
  if (!gm_conf_get_bool (SOUND_EVENTS_KEY "enable_new_message_sound"))
    return;

  Ekiga::ServiceCore *core = GnomeMeeting::Process ()->GetServiceCore ();
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = core->get ("audiooutput-core");

  std::string file_name_string = gm_conf_get_string (SOUND_EVENTS_KEY "new_message_sound");

  if (!file_name_string.empty ())
    audiooutput_core->play_file(file_name_string);
}


static void 
panel_section_changed_nt (G_GNUC_UNUSED gpointer id, 
                          GmConfEntry *entry, 
                          gpointer data)
{
  gint section = 0;

  g_return_if_fail (data != NULL);
  
  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    section = gm_conf_entry_get_int (entry);
    ekiga_main_window_set_panel_section (EKIGA_MAIN_WINDOW (data), section);
  }
}


static void 
show_call_panel_changed_nt (G_GNUC_UNUSED gpointer id, 
                            GmConfEntry *entry, 
                            gpointer data)
{
  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    if (gm_conf_entry_get_bool (entry)) 
      ekiga_main_window_show_call_panel (EKIGA_MAIN_WINDOW (data));
    else 
      ekiga_main_window_hide_call_panel (EKIGA_MAIN_WINDOW (data));
  }
}

static void 
stay_on_top_changed_nt (G_GNUC_UNUSED gpointer id,
                        GmConfEntry *entry, 
                        gpointer data)
{
  bool val = false;
    
  g_return_if_fail (data != NULL);

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {

    val = gm_conf_entry_get_bool (entry);
    ekiga_main_window_set_stay_on_top (EKIGA_MAIN_WINDOW (data), val);
  }
}


static void 
pull_trigger_cb (GtkWidget * /*widget*/,
                 gpointer data)
{
  Ekiga::Trigger *trigger = (Ekiga::Trigger *) data;

  g_return_if_fail (trigger != NULL);

  trigger->pull ();
}


static void 
show_widget_cb (GtkWidget * /*widget*/,
                gpointer data)
{
  g_return_if_fail (data != NULL);

  gtk_widget_show_all (GTK_WIDGET (data));
}


static void 
hold_current_call_cb (G_GNUC_UNUSED GtkWidget *widget,
		      gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->current_call)
    mw->priv->current_call->toggle_hold ();
}


static void
toggle_audio_stream_pause_cb (GtkWidget * /*widget*/,
                              gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->current_call)
    mw->priv->current_call->toggle_stream_pause (Ekiga::Call::Audio);
}


static void
toggle_video_stream_pause_cb (GtkWidget * /*widget*/,
                              gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  if (mw->priv->current_call)
    mw->priv->current_call->toggle_stream_pause (Ekiga::Call::Video);
}


static void 
transfer_current_call_cb (G_GNUC_UNUSED GtkWidget *widget,
			  gpointer data)
{
  GtkWidget *mw = NULL;
  
  g_return_if_fail (data != NULL);
  
  mw = GnomeMeeting::Process ()->GetMainWindow ();

  ekiga_main_window_transfer_dialog_run (EKIGA_MAIN_WINDOW (mw), GTK_WIDGET (data), NULL);  
}


static void 
audio_volume_changed_cb (GtkAdjustment * /*adjustment*/,
			 gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = mw->priv->core->get ("audioinput-core");
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  audiooutput_core->set_volume (Ekiga::primary, (unsigned) GTK_ADJUSTMENT (mw->priv->adj_output_volume)->value);
  audioinput_core->set_volume ((unsigned) GTK_ADJUSTMENT (mw->priv->adj_input_volume)->value);
}

static void 
audio_volume_window_shown_cb (GtkWidget * /*widget*/,
	                      gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = mw->priv->core->get ("audioinput-core");
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  audioinput_core->set_average_collection (true);
  audiooutput_core->set_average_collection (true);
  mw->priv->levelmeter_timeout_id = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 50, on_signal_level_refresh_cb, data, NULL);
}


static void 
audio_volume_window_hidden_cb (GtkWidget * /*widget*/,
                               gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = mw->priv->core->get ("audioinput-core");
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  g_source_remove (mw->priv->levelmeter_timeout_id);
  audioinput_core->set_average_collection (false);
  audiooutput_core->set_average_collection (false);
}

static void 
video_settings_changed_cb (GtkAdjustment * /*adjustment*/,
			   gpointer data)
{ 
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);

  gmref_ptr<Ekiga::VideoInputCore> videoinput_core
    = mw->priv->core->get ("videoinput-core");

  videoinput_core->set_whiteness ((unsigned) GTK_ADJUSTMENT (mw->priv->adj_whiteness)->value);
  videoinput_core->set_brightness ((unsigned) GTK_ADJUSTMENT (mw->priv->adj_brightness)->value);
  videoinput_core->set_colour ((unsigned) GTK_ADJUSTMENT (mw->priv->adj_colour)->value);
  videoinput_core->set_contrast ((unsigned) GTK_ADJUSTMENT (mw->priv->adj_contrast)->value);
}


static void 
panel_section_changed_cb (G_GNUC_UNUSED GtkNotebook *notebook,
                          G_GNUC_UNUSED GtkNotebookPage *page,
                          G_GNUC_UNUSED gint page_num,
                          gpointer data) 
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);
  gint current_page = 0;

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (mw->priv->main_notebook));
  gm_conf_set_int (USER_INTERFACE_KEY "main_window/panel_section", current_page);
}


static void
dialpad_button_clicked_cb (EkigaDialpad  * /* dialpad */,
			   const gchar *button_text,
			   EkigaMainWindow *mw)
{
  if (mw->priv->current_call)
    mw->priv->current_call->send_dtmf (button_text[0]);
  else
    ekiga_main_window_append_call_url (mw, button_text);
}


static gint 
window_closed_cb (G_GNUC_UNUSED GtkWidget *widget,
		  G_GNUC_UNUSED GdkEvent *event,
		  gpointer data)
{
  GtkStatusIcon *statusicon = NULL;
  GtkWidget *main_window = NULL;

  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  statusicon = GTK_STATUS_ICON (GnomeMeeting::Process ()->GetStatusicon ());

  if (!gtk_status_icon_is_embedded (GTK_STATUS_ICON (statusicon)))
    quit_callback (NULL, data);
  else
    gtk_widget_hide (GTK_WIDGET (data));

  return (TRUE);
}


static void
window_closed_from_menu_cb (GtkWidget *widget,
                           gpointer data)
{
window_closed_cb (widget, NULL, data);
}


static void 
zoom_in_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		    gpointer data)
{
  GtkWidget *main_window = GnomeMeeting::Process ()->GetMainWindow ();
  g_return_if_fail (main_window != NULL);

  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;

  display_info.zoom = gm_conf_get_int ((char *) data);

  if (display_info.zoom < 200)
    display_info.zoom = display_info.zoom * 2;

  gm_conf_set_int ((char *) data, display_info.zoom);
  ekiga_main_window_zooms_menu_update_sensitivity (EKIGA_MAIN_WINDOW (main_window), display_info.zoom);
}


static void 
zoom_out_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		     gpointer data)
{
  GtkWidget *main_window = GnomeMeeting::Process ()->GetMainWindow ();
  g_return_if_fail (main_window != NULL);

  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;

  display_info.zoom = gm_conf_get_int ((char *) data);

  if (display_info.zoom  > 50)
    display_info.zoom  = (unsigned int) (display_info.zoom  / 2);

  gm_conf_set_int ((char *) data, display_info.zoom);
  ekiga_main_window_zooms_menu_update_sensitivity (EKIGA_MAIN_WINDOW (main_window), display_info.zoom);
}


static void 
zoom_normal_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
			gpointer data)
{
  GtkWidget *main_window = GnomeMeeting::Process ()->GetMainWindow ();
  g_return_if_fail (main_window != NULL);

  g_return_if_fail (data != NULL);

  Ekiga::DisplayInfo display_info;

  display_info.zoom  = 100;

  gm_conf_set_int ((char *) data, display_info.zoom);
  ekiga_main_window_zooms_menu_update_sensitivity (EKIGA_MAIN_WINDOW (main_window), display_info.zoom);
}


void 
display_changed_cb (GtkWidget *widget,
		       gpointer data)
{
  GtkWidget *main_window = GnomeMeeting::Process ()->GetMainWindow ();
  g_return_if_fail (main_window != NULL);

  g_return_if_fail (data != NULL);

  GSList *group = NULL;
  int group_last_pos = 0;
  int active = 0;
  Ekiga::DisplayInfo display_info;

  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, last pos is 0 */

  /* Only do something when a new CHECK_MENU_ITEM becomes active,
     not when it becomes inactive */
  if (GTK_CHECK_MENU_ITEM (widget)->active) {

    while (group) {

      if (group->data == widget) 
	break;
      
      active++;
      group = g_slist_next (group);
    }

    gm_conf_set_int ((gchar *) data, group_last_pos - active);
  }
}


static void 
fullscreen_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
		       G_GNUC_UNUSED gpointer data)
{
  GtkWidget* main_window = GnomeMeeting::Process()->GetMainWindow ();
  g_return_if_fail (main_window != NULL);
  gm_main_window_toggle_fullscreen (Ekiga::VO_FS_TOGGLE, main_window);
}


static void
url_changed_cb (GtkEditable *e,
		gpointer data)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (data);
  GtkTreeIter iter;
  const char *tip_text = NULL;
  gchar *entry = NULL;

  tip_text = gtk_entry_get_text (GTK_ENTRY (e));

  if (g_strrstr (tip_text, "@") == NULL) {

    gtk_list_store_clear (mw->priv->completion);

    for (std::list<std::string>::iterator it = mw->priv->accounts.begin ();
         it != mw->priv->accounts.end ();
         it++) {

      entry = g_strdup_printf ("%s@%s", tip_text, it->c_str ());
      gtk_list_store_append (mw->priv->completion, &iter);
      gtk_list_store_set (mw->priv->completion, &iter, 0, entry, -1);
      g_free (entry);
    }
  }

  gtk_widget_set_tooltip_text (GTK_WIDGET (e), tip_text);
}


static void 
toolbar_toggle_button_changed_cb (G_GNUC_UNUSED GtkWidget *widget,
				  gpointer data)
{
  bool shown = gm_conf_get_bool ((gchar *) data);

  gm_conf_set_bool ((gchar *) data, !shown);
}


static gboolean 
statusbar_clicked_cb (G_GNUC_UNUSED GtkWidget *widget,
		      G_GNUC_UNUSED GdkEventButton *event,
		      gpointer data)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (data), FALSE);

  ekiga_main_window_push_message (EKIGA_MAIN_WINDOW (data), NULL);

  return FALSE;
}


/* Public functions */
void 
gm_main_window_press_dialpad (GtkWidget *main_window,
			      const char c)
{
  guint key = 0;

  if (c == '*')
    key = GDK_KP_Multiply;
  else if (c == '#')
    key = GDK_numbersign;
  else
    key = GDK_KP_0 + atoi (&c);

  gtk_accel_groups_activate (G_OBJECT (main_window), key, (GdkModifierType) 0);
}


GtkWidget*
ekiga_main_window_get_video_widget (EkigaMainWindow *mw)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (mw), NULL);
  return mw->priv->main_video_image;
}


void 
ekiga_main_window_update_logo_have_window (EkigaMainWindow *mw)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  g_object_set (G_OBJECT (mw->priv->main_video_image),
		"icon-name", GM_ICON_LOGO,
		"pixel-size", 72,
		NULL);
  
  ekiga_main_window_set_video_size (mw, GM_QCIF_WIDTH, GM_QCIF_HEIGHT);
}


void 
ekiga_main_window_set_call_hold (EkigaMainWindow *mw,
                                 bool is_on_hold)
{
  GtkWidget *child = NULL;
  
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  child = GTK_BIN (gtk_menu_get_widget (mw->priv->main_menu, "hold_call"))->child;

  if (is_on_hold) {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Retrieve Call"));

    /* Set the audio and video menu to unsensitive */
    gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_audio", FALSE);
    gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_video", FALSE);
    
    ekiga_main_window_set_channel_pause (mw, TRUE, FALSE);
    ekiga_main_window_set_channel_pause (mw, TRUE, TRUE);
  }
  else {

    if (GTK_IS_LABEL (child))
      gtk_label_set_text_with_mnemonic (GTK_LABEL (child),
					_("_Hold Call"));

    gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_audio", TRUE);
    gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_video", TRUE);

    ekiga_main_window_set_channel_pause (mw, FALSE, FALSE);
    ekiga_main_window_set_channel_pause (mw, FALSE, TRUE);
  }
  
  g_signal_handlers_block_by_func (G_OBJECT (mw->priv->hold_button),
                                   (gpointer) hold_current_call_cb,
                                   mw);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->priv->hold_button), 
                                is_on_hold);
  g_signal_handlers_unblock_by_func (G_OBJECT (mw->priv->hold_button),
                                     (gpointer) hold_current_call_cb,
                                     mw);
}


void 
ekiga_main_window_set_channel_pause (EkigaMainWindow *mw,
				     gboolean pause,
				     gboolean is_video)
{
  GtkWidget *widget = NULL;
  GtkWidget *child = NULL;
  gchar *msg = NULL;
  
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  if (!pause && !is_video)
    msg = _("Suspend _Audio");
  else if (!pause && is_video)
    msg = _("Suspend _Video");
  else if (pause && !is_video)
    msg = _("Resume _Audio");
  else if (pause && is_video)
    msg = _("Resume _Video");

  widget = gtk_menu_get_widget (mw->priv->main_menu,
			        is_video ? "suspend_video" : "suspend_audio");
  child = GTK_BIN (widget)->child; 

  if (GTK_IS_LABEL (child)) 
    gtk_label_set_text_with_mnemonic (GTK_LABEL (child), msg);
}


void
ekiga_main_window_update_calling_state (EkigaMainWindow *mw,
					unsigned calling_state)
{
  g_return_if_fail (mw != NULL);

  ekiga_main_window_selected_presentity_build_menu (mw);
  switch (calling_state)
    {
    case Standby:
      
      /* Update the hold state */
      ekiga_main_window_set_call_hold (mw, FALSE);

      /* Update the sensitivity, all channels are closed */
      ekiga_main_window_update_sensitivity (mw, TRUE, FALSE, FALSE);
      ekiga_main_window_update_sensitivity (mw, FALSE, FALSE, FALSE);
      
      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->priv->main_menu, "connect", TRUE);
      gtk_menu_set_sensitive (mw->priv->main_menu, "disconnect", FALSE);
      gtk_menu_section_set_sensitive (mw->priv->main_menu, "hold_call", FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->hold_button), FALSE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), TRUE);
      
      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->priv->connect_button),
				       FALSE);
	
      /* Destroy the transfer call popup */
      if (mw->priv->transfer_call_popup) 
	gtk_dialog_response (GTK_DIALOG (mw->priv->transfer_call_popup),
			     GTK_RESPONSE_REJECT);
      break;


    case Calling:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->priv->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->priv->main_menu, "disconnect", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), FALSE);

      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->priv->connect_button),
				       TRUE);
      
      break;


    case Connected:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->priv->main_menu, "connect", FALSE);
      gtk_menu_set_sensitive (mw->priv->main_menu, "disconnect", TRUE);
      gtk_menu_section_set_sensitive (mw->priv->main_menu, "hold_call", TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->hold_button), TRUE);
      gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->preview_button), FALSE);

      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->priv->connect_button),
				       TRUE);
      break;


    case Called:

      /* Update the menus and toolbar items */
      gtk_menu_set_sensitive (mw->priv->main_menu, "disconnect", TRUE);

      /* Update the connect button */
      gm_connect_button_set_connected (GM_CONNECT_BUTTON (mw->priv->connect_button),
 				       TRUE);
      
      break;

    default:
      break;
    }

  mw->priv->calling_state = calling_state;
}


void
ekiga_main_window_update_sensitivity (EkigaMainWindow *mw,
				      bool is_video,
				      bool /*is_receiving*/,
				      bool is_transmitting)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  if (is_transmitting) {

    if (!is_video) 
      gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_audio", TRUE);
    else 
      gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_video", TRUE);
  }	
  else {

    if (!is_video)
      gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_audio", FALSE);
    else
      gtk_menu_set_sensitive (mw->priv->main_menu, "suspend_video", FALSE);

  }
}


void
gm_main_window_toggle_fullscreen (Ekiga::VideoOutputFSToggle toggle,
                                  G_GNUC_UNUSED GtkWidget   *main_window)
{
  Ekiga::VideoOutputMode videooutput_mode;

  switch (toggle) {
    case Ekiga::VO_FS_OFF:
      if (gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") == Ekiga::VO_MODE_FULLSCREEN) {

        videooutput_mode = (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", videooutput_mode);
      }
      break;
    case Ekiga::VO_FS_ON:
      if (gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") != Ekiga::VO_MODE_FULLSCREEN) {

        videooutput_mode = (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen", videooutput_mode);
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", Ekiga::VO_MODE_FULLSCREEN);
      }
      break;

    case Ekiga::VO_FS_TOGGLE:
    default:
      if (gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") == Ekiga::VO_MODE_FULLSCREEN) {

        videooutput_mode = (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", videooutput_mode);
      }
      else {

        videooutput_mode =  (Ekiga::VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view_before_fullscreen", videooutput_mode);
        gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", Ekiga::VO_MODE_FULLSCREEN);
      }
      break;
  }
}

static void 
ekiga_main_window_set_stay_on_top (EkigaMainWindow *mw,
				   gboolean stay_on_top)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* Update the stay-on-top attribute */
  gdk_window_set_always_on_top (GTK_WIDGET (mw)->window, stay_on_top);
}

static void 
ekiga_main_window_show_call_panel (EkigaMainWindow *mw)
{
  int x, y = 0;
  GtkWidget *call_panel = gtk_paned_get_child2 (GTK_PANED (mw->priv->hpaned));

#if GTK_CHECK_VERSION(2,18,0)
  if (!gtk_widget_get_visible (GTK_WIDGET (call_panel))) {
#else
  if (!GTK_WIDGET_VISIBLE (call_panel)) {
#endif
    gtk_window_get_size (GTK_WINDOW (mw), &x, &y);
    gtk_widget_show_all (call_panel);
    gtk_window_resize (GTK_WINDOW (mw), x + call_panel->allocation.width, y);
  }
}


static void 
ekiga_main_window_hide_call_panel (EkigaMainWindow *mw)
{
  int x, y = 0;
  GtkWidget *call_panel = gtk_paned_get_child2 (GTK_PANED (mw->priv->hpaned));

#if GTK_CHECK_VERSION(2,18,0)
  if (gtk_widget_get_visible (GTK_WIDGET (call_panel))) {
#else
  if (GTK_WIDGET_VISIBLE (call_panel)) {
#endif
    gtk_window_get_size (GTK_WINDOW (mw), &x, &y);
    gtk_widget_hide (call_panel);
    x = x - call_panel->allocation.width;
    gtk_window_resize (GTK_WINDOW (mw), x, y);
  }
}


void
ekiga_main_window_set_busy (EkigaMainWindow *mw,
			    bool busy)
{
  GdkCursor *cursor = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  gtk_widget_set_sensitive (mw->priv->main_toolbar, !busy);
  gtk_widget_set_sensitive (mw->priv->main_menu, !busy);

  if (busy) {

    cursor = gdk_cursor_new (GDK_WATCH);
    gdk_window_set_cursor (GTK_WIDGET (mw)->window, cursor);
    gdk_cursor_unref (cursor);
  }
  else
    gdk_window_set_cursor (GTK_WIDGET (mw)->window, NULL);
}

void
ekiga_main_window_clear_signal_levels (EkigaMainWindow *mw)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  gm_level_meter_clear (GM_LEVEL_METER (mw->priv->output_signal));
  gm_level_meter_clear (GM_LEVEL_METER (mw->priv->input_signal));
}

static void
ekiga_main_window_selected_presentity_build_menu (EkigaMainWindow *mw)
{
  GtkWidget *menu = gtk_menu_get_widget (mw->priv->main_menu, "contact");

  if (mw->priv->presentity != NULL) {

    MenuBuilderGtk builder;
    gtk_widget_set_sensitive (menu, TRUE);
    if (mw->priv->presentity->populate_menu (builder)) {

      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), builder.menu);
      gtk_widget_show_all (builder.menu);
    } 
    else {

      gtk_widget_set_sensitive (menu, FALSE);
      g_object_ref_sink (builder.menu);
      g_object_unref (builder.menu);
    }
  } 
  else {

    gtk_widget_set_sensitive (menu, FALSE);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu), NULL);
  }
}


static void
ekiga_main_window_incoming_call_dialog_show (EkigaMainWindow *mw,
                                             gmref_ptr<Ekiga::Call>  call)
{
  GdkPixbuf *pixbuf = NULL;

  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *b1 = NULL;
  GtkWidget *b2 = NULL;
  GtkWidget *incoming_call_popup = NULL;

  gchar *msg = NULL;

  // FIXME could the call become invalid ?
  const char *utf8_name = call->get_remote_party_name ().c_str ();
  const char *utf8_app = call->get_remote_application ().c_str ();
  const char *utf8_url = call->get_remote_uri ().c_str ();
  const char *utf8_local = call->get_local_party_name ().c_str ();

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  incoming_call_popup = gtk_dialog_new ();
  b2 = gtk_dialog_add_button (GTK_DIALOG (incoming_call_popup),
			      _("Reject"), 0);
  b1 = gtk_dialog_add_button (GTK_DIALOG (incoming_call_popup),
			      _("Accept"), 2);

  gtk_dialog_set_default_response (GTK_DIALOG (incoming_call_popup), 2);

  vbox = GTK_DIALOG (incoming_call_popup)->vbox;

  msg = g_strdup_printf ("%s <i>%s</i>", _("Incoming call from"), (const char*) utf8_name);
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), msg);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 10);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  g_free (msg);

  pixbuf = gtk_widget_render_icon (GTK_WIDGET (incoming_call_popup),
				   GM_STOCK_PHONE_PICK_UP_16,
				   GTK_ICON_SIZE_MENU, NULL);
  gtk_window_set_icon (GTK_WINDOW (incoming_call_popup), pixbuf);
  g_object_unref (pixbuf);

  if (utf8_url) {
    
    label = gtk_label_new (NULL);
    msg = g_strdup_printf ("<b>%s</b> <span foreground=\"blue\"><u>%s</u></span>",
                           _("Remote URI:"), utf8_url);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  if (utf8_app) {

    label = gtk_label_new (NULL);
    msg = g_strdup_printf ("<b>%s</b> %s",
			   _("Remote Application:"), utf8_app);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  if (utf8_local) {
    
    label = gtk_label_new (NULL);
    msg =
      g_strdup_printf ("<b>%s</b> %s",
		       _("Account ID:"), utf8_local);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  msg = g_strdup_printf (_("Call from %s"), (const char*) utf8_name);
  gtk_window_set_title (GTK_WINDOW (incoming_call_popup), msg);
  g_free (msg);
  gtk_window_set_modal (GTK_WINDOW (incoming_call_popup), TRUE);
  gtk_window_set_keep_above (GTK_WINDOW (incoming_call_popup), TRUE);
  gtk_window_set_urgency_hint (GTK_WINDOW (mw), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (incoming_call_popup),
				GTK_WINDOW (mw));

  gtk_widget_show_all (incoming_call_popup);

  g_signal_connect (G_OBJECT (incoming_call_popup), "delete_event",
                    G_CALLBACK (gtk_widget_hide_on_delete), NULL);
  g_signal_connect (G_OBJECT (incoming_call_popup), "response",
                    G_CALLBACK (incoming_call_response_cb), mw);

  call->cleared.connect (sigc::bind (sigc::ptr_fun (on_cleared_incoming_call_cb),
                                    (gpointer) incoming_call_popup));
  call->missed.connect (sigc::bind (sigc::ptr_fun (on_missed_incoming_call_cb), 
                                   (gpointer) incoming_call_popup));
}


#ifdef HAVE_NOTIFY
static void
notify_action_cb (NotifyNotification *notification,
                  gchar *action,
                  gpointer main_window)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (main_window);

  notify_notification_close (notification, NULL);

  if (mw->priv->current_call) {

    if (!strcmp (action, "accept"))
      mw->priv->current_call->answer ();
    else
      mw->priv->current_call->hangup ();
  }
}


static void
closed_cb (NotifyNotification* /*notify*/, 
           gpointer main_window)
{
  EkigaMainWindow *mw;

  g_return_if_fail (main_window != NULL);

  mw = EKIGA_MAIN_WINDOW (main_window);

  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");
  if (audiooutput_core) 
    audiooutput_core->stop_play_event ("incoming_call_sound");
}

static void
ekiga_main_window_incoming_call_notify (EkigaMainWindow *mw,
                                        gmref_ptr<Ekiga::Call>  call)
{
  NotifyNotification *notify = NULL;
  
  GtkStatusIcon *statusicon = NULL;

  gchar *uri = NULL;
  gchar *app = NULL;
  gchar *account = NULL;
  gchar *body = NULL;
  gchar *title = NULL;

  statusicon = GTK_STATUS_ICON (GnomeMeeting::Process ()->GetStatusicon ());

  const char *utf8_name = call->get_remote_party_name ().c_str ();
  const char *utf8_app = call->get_remote_application ().c_str ();
  const char *utf8_url = call->get_remote_uri ().c_str ();
  const char *utf8_local = call->get_local_party_name ().c_str ();

  title = g_strdup_printf ("%s %s", _("Incoming call from"), (const char*) utf8_name);

  if (utf8_url)
    uri = g_strdup_printf ("<b>%s</b> %s", _("Remote URI:"), utf8_url);
  if (utf8_app)
    app = g_strdup_printf ("<b>%s</b> %s", _("Remote Application:"), utf8_app);
  if (utf8_local)
    account = g_strdup_printf ("<b>%s</b> %s", _("Account ID:"), utf8_local);

  body = g_strdup_printf ("%s\n%s\n%s", uri, app, account);
  
  notify = notify_notification_new (title, body, GM_ICON_LOGO, NULL);
  notify_notification_add_action (notify, "accept", _("Accept"), notify_action_cb, mw, NULL);
  notify_notification_add_action (notify, "reject", _("Reject"), notify_action_cb, mw, NULL);
  notify_notification_set_timeout (notify, NOTIFY_EXPIRES_NEVER);
  notify_notification_set_urgency (notify, NOTIFY_URGENCY_CRITICAL);
  notify_notification_attach_to_status_icon (notify, statusicon);
  if (!notify_notification_show (notify, NULL)) {
    ekiga_main_window_incoming_call_dialog_show (mw, call);
  }
  else {
    call->cleared.connect (sigc::bind (sigc::ptr_fun (on_cleared_incoming_call_cb),
                                      (gpointer) notify));
    call->missed.connect (sigc::bind (sigc::ptr_fun (on_missed_incoming_call_cb), 
                                     (gpointer) notify));
  }

  g_signal_connect (notify, "closed", G_CALLBACK (closed_cb), mw);

  g_free (uri);
  g_free (app);
  g_free (account);
  g_free (title);
  g_free (body);
}
#endif

void 
ekiga_main_window_set_panel_section (EkigaMainWindow *mw,
                                     int section)
{
  GtkWidget *menu = NULL;
  
  g_return_if_fail (EKIGA_MAIN_WINDOW (mw));
  
  gtk_notebook_set_current_page (GTK_NOTEBOOK (mw->priv->main_notebook), section);
  
  menu = gtk_menu_get_widget (mw->priv->main_menu, "dialpad");
  gtk_radio_menu_select_with_widget (GTK_WIDGET (menu), section);
}


void 
ekiga_main_window_set_call_info (EkigaMainWindow *mw,
				const char *tr_audio_codec,
				G_GNUC_UNUSED const char *re_audio_codec,
				const char *tr_video_codec,
				G_GNUC_UNUSED const char *re_video_codec)
{
  GtkTextIter iter;
  GtkTextIter *end_iter = NULL;
  GtkTextBuffer *buffer = NULL;
  
  gchar *info = NULL;
  
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));
  
  if (!tr_audio_codec && !tr_video_codec)
    info = g_strdup (" ");
  else
    info = g_strdup_printf ("%s - %s",
                            tr_audio_codec?tr_audio_codec:"", 
                            tr_video_codec?tr_video_codec:"");
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->priv->info_text));
  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_iter_forward_lines (&iter, 2);
  end_iter = gtk_text_iter_copy (&iter);
  gtk_text_iter_forward_line (end_iter);
  gtk_text_buffer_delete (buffer, &iter, end_iter);
  gtk_text_iter_free (end_iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "codecs", NULL);
  g_free (info);
}


void 
ekiga_main_window_set_status (EkigaMainWindow *mw,
			      const char *status)
{
  GtkTextIter iter;
  GtkTextIter* end_iter = NULL;
  GtkTextBuffer *buffer = NULL;

  gchar *info = NULL;
  
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));
  
  info = g_strdup_printf ("%s\n", status);
  
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->priv->info_text));
  gtk_text_buffer_get_start_iter (buffer, &iter);
  end_iter = gtk_text_iter_copy (&iter);
  gtk_text_iter_forward_line (end_iter);
  gtk_text_buffer_delete (buffer, &iter, end_iter);
  gtk_text_iter_free (end_iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "status", NULL);
  g_free (info);
}


void 
ekiga_main_window_set_call_duration (EkigaMainWindow *mw,
                                     const char *duration)
{
  GtkTextIter iter;
  GtkTextIter* end_iter = NULL;
  GtkTextBuffer *buffer = NULL;

  gchar *info = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));
  
  if (duration)
    info = g_strdup_printf (_("Call Duration: %s\n"), duration);
  else
    info = g_strdup ("\n");

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->priv->info_text));
  gtk_text_buffer_get_start_iter (buffer, &iter);
  gtk_text_iter_forward_line (&iter);
  end_iter = gtk_text_iter_copy (&iter);
  gtk_text_iter_forward_line (end_iter);
  gtk_text_buffer_delete (buffer, &iter, end_iter);
  gtk_text_iter_free (end_iter);
  gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, info, 
                                            -1, "call-duration", NULL);

  g_free (info);
}


gboolean 
ekiga_main_window_transfer_dialog_run (EkigaMainWindow *mw,
				       GtkWidget *parent_window,
				       const char *u)
{
  gint answer = 0;
  
  const char *forward_url = NULL;

  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (mw), FALSE);
  g_return_val_if_fail (GTK_IS_WINDOW (parent_window), FALSE);
  
  mw->priv->transfer_call_popup = 
    gm_entry_dialog_new (_("Transfer call to:"),
			 _("Transfer"));
  
  gtk_window_set_transient_for (GTK_WINDOW (mw->priv->transfer_call_popup),
				GTK_WINDOW (parent_window));
  
  gtk_dialog_set_default_response (GTK_DIALOG (mw->priv->transfer_call_popup),
				   GTK_RESPONSE_ACCEPT);
  
  if (u && !strcmp (u, ""))
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->priv->transfer_call_popup), u);
  else
    gm_entry_dialog_set_text (GM_ENTRY_DIALOG (mw->priv->transfer_call_popup), "sip:");

  gnomemeeting_threads_dialog_show (mw->priv->transfer_call_popup);

  answer = gtk_dialog_run (GTK_DIALOG (mw->priv->transfer_call_popup));
  switch (answer) {

  case GTK_RESPONSE_ACCEPT:

    forward_url = gm_entry_dialog_get_text (GM_ENTRY_DIALOG (mw->priv->transfer_call_popup));
    if (strcmp (forward_url, "") && mw->priv->current_call)
      mw->priv->current_call->transfer (forward_url);
    break;

  default:
    break;
  }

  gtk_widget_destroy (mw->priv->transfer_call_popup);
  mw->priv->transfer_call_popup = NULL;

  return (answer == GTK_RESPONSE_ACCEPT);
}


static void 
ekiga_main_window_add_device_dialog_show (EkigaMainWindow *mw,
                                          const Ekiga::Device & device,
                                          DeviceType device_type)
{
  GtkWidget *label = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *b1 = NULL;
  GtkWidget *b2 = NULL;
  GtkWidget *add_device_popup = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  add_device_popup = gtk_dialog_new ();
  b2 = gtk_dialog_add_button (GTK_DIALOG (add_device_popup),
			      _("No"), 0);
  b1 = gtk_dialog_add_button (GTK_DIALOG (add_device_popup),
			      _("Yes"), 2);

  gtk_dialog_set_default_response (GTK_DIALOG (add_device_popup), 2);

  vbox = GTK_DIALOG (add_device_popup)->vbox;

  std::string msg;
  std::string title;

  switch (device_type) {
    case AudioInput:
      msg = _("Detected new audio input device:");
      title = _("Audio Devices");
      break;
    case AudioOutput:
      msg = _("Detected new audio output device:");
      title = _("Audio Devices");
      break;
    case VideoInput:
      msg = _("Detected new video input device:");
      title = _("Video Devices");
      break;
    default:
      break;
  }
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), msg.c_str());
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  
  msg  = "<b>" + device.GetString() + "</b>";
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), msg.c_str());
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  msg  = _("Do you want to use it as default device?");
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), msg.c_str());
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

  gtk_window_set_title (GTK_WINDOW (add_device_popup), title.c_str ());
  gtk_window_set_modal (GTK_WINDOW (add_device_popup), TRUE);
  gtk_window_set_keep_above (GTK_WINDOW (add_device_popup), TRUE);
  gtk_window_set_urgency_hint (GTK_WINDOW (mw), TRUE);
  gtk_window_set_transient_for (GTK_WINDOW (add_device_popup),
				GTK_WINDOW (mw));

  gtk_widget_show_all (add_device_popup);


//  g_signal_connect (G_OBJECT (add_device_popup), "delete_event",
//                    G_CALLBACK (gtk_widget_hide_on_delete), NULL);
//  g_signal_connect (G_OBJECT (add_device_popup), "response",
//                    G_CALLBACK (add_device_response_cb), &device);

  deviceStruct* device_struct = g_new(deviceStruct, 1);
  snprintf (device_struct->name, sizeof (device_struct->name), "%s", (device.GetString()).c_str());
  device_struct->deviceType = device_type;

  g_signal_connect_data (G_OBJECT (add_device_popup), "delete_event",
                         G_CALLBACK (gtk_widget_hide_on_delete), 
                         (gpointer) device_struct,
                         (GClosureNotify) g_free,
                         (GConnectFlags) 0);

  g_signal_connect_data (G_OBJECT (add_device_popup), "response",
                         G_CALLBACK (add_device_response_cb), 
                         (gpointer) device_struct,
                         (GClosureNotify) g_free,
                         (GConnectFlags) 0);
}

static void
ekiga_main_window_init_menu (EkigaMainWindow *mw)
{
  Ekiga::ServiceCore *services = NULL;
  
  GtkWidget *addressbook_window = NULL;
  GtkWidget *accounts_window = NULL;

  bool show_call_panel = false;

  PanelSection cps = DIALPAD;

  g_return_if_fail (mw != NULL);

  services = GnomeMeeting::Process ()->GetServiceCore ();
  gmref_ptr<Ekiga::Trigger> local_cluster_trigger
    = services->get ("local-cluster");
  gmref_ptr<GtkFrontend> gtk_frontend = services->get ("gtk-frontend");
  addressbook_window = GTK_WIDGET (gtk_frontend->get_addressbook_window ()); 
  accounts_window = GnomeMeeting::Process ()->GetAccountsWindow ();

  mw->priv->main_menu = gtk_menu_bar_new ();

  /* Default values */
  show_call_panel = gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_call_panel");
  cps = (PanelSection) gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section");

  static MenuEntry gnomemeeting_menu [] =
    {
      GTK_MENU_NEW (_("_Chat")),

      GTK_MENU_ENTRY("connect", _("Ca_ll"), _("Place a new call"), 
		     GM_STOCK_PHONE_PICK_UP_16, 'o',
		     G_CALLBACK (place_call_cb), mw, TRUE),
      GTK_MENU_ENTRY("disconnect", _("_Hang up"),
		     _("Terminate the current call"), 
 		     GM_STOCK_PHONE_HANG_UP_16, GDK_Escape,
		     G_CALLBACK (hangup_call_cb), mw, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("add_contact", _("A_dd Contact"), _("Add a contact to the roster"),
		     GTK_STOCK_ADD, 'n', 
		     G_CALLBACK (pull_trigger_cb), &*local_cluster_trigger, true),

      GTK_MENU_THEME_ENTRY("address_book", _("Address _Book"),
			   _("Find contacts"),
 			   GM_ICON_ADDRESSBOOK, 'b',
			   G_CALLBACK (show_widget_cb),
			   (gpointer) addressbook_window, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("contact", _("_Contact"),
		     _("Act on selected contact"),
		     GTK_STOCK_EXECUTE, 0,
		     NULL, NULL, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("hold_call", _("H_old Call"), _("Hold the current call"),
		     NULL, GDK_h, 
		     G_CALLBACK (hold_current_call_cb), mw,
		     FALSE),
      GTK_MENU_ENTRY("transfer_call", _("_Transfer Call"),
		     _("Transfer the current call"),
 		     NULL, GDK_t, 
		     G_CALLBACK (transfer_current_call_cb), mw,
		     FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("suspend_audio", _("Suspend _Audio"),
		     _("Suspend or resume the audio transmission"),
		     NULL, GDK_m,
		     G_CALLBACK (toggle_audio_stream_pause_cb),
		     mw, FALSE),
      GTK_MENU_ENTRY("suspend_video", _("Suspend _Video"),
		     _("Suspend or resume the video transmission"),
		     NULL, GDK_p, 
		     G_CALLBACK (toggle_video_stream_pause_cb),
		     mw, FALSE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("close", NULL, _("Close the Ekiga window"),
		     GTK_STOCK_CLOSE, 'W', 
		     G_CALLBACK (window_closed_from_menu_cb),
		     (gpointer) mw, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("quit", NULL, _("Quit"),
		     GTK_STOCK_QUIT, 'Q', 
		     G_CALLBACK (quit_callback), NULL, TRUE),

      GTK_MENU_NEW (_("_Edit")),

      GTK_MENU_ENTRY("configuration_assistant", _("_Configuration Assistant"),
		     _("Run the configuration assistant"),
		     NULL, 0, 
		     G_CALLBACK (show_assistant_window_cb),
		     NULL, TRUE),

      GTK_MENU_SEPARATOR,
      
      GTK_MENU_ENTRY("accounts", _("_Accounts"),
		     _("Edit your accounts"), 
		     NULL, 'E',
		     G_CALLBACK (show_window_cb),
		     (gpointer) accounts_window, TRUE),

      GTK_MENU_ENTRY("preferences", NULL,
		     _("Change your preferences"), 
		     GTK_STOCK_PREFERENCES, 0,
		     G_CALLBACK (show_prefs_window_cb),
		     NULL, TRUE),

      GTK_MENU_NEW(_("_View")),

      GTK_MENU_RADIO_ENTRY("contacts", _("Con_tacts"), _("View the contacts list"),
			   NULL, 0,
			   G_CALLBACK (radio_menu_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CONTACTS), TRUE),
      GTK_MENU_RADIO_ENTRY("dialpad", _("_Dialpad"), _("View the dialpad"),
			   NULL, 0,
			   G_CALLBACK (radio_menu_changed_cb), 
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == DIALPAD), TRUE),
      GTK_MENU_RADIO_ENTRY("callhistory", _("_Call History"), _("View the call history"),
			   NULL, 0,
			   G_CALLBACK (radio_menu_changed_cb),
			   (gpointer) USER_INTERFACE_KEY "main_window/panel_section",
			   (cps == CALL), TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_TOGGLE_ENTRY("callpanel", _("_Show Call Panel"), _("Show the call panel"),
                            NULL, 'J', 
                            G_CALLBACK (toggle_menu_changed_cb),
                            (gpointer) USER_INTERFACE_KEY "main_window/show_call_panel", 
                            show_call_panel, TRUE),

      GTK_MENU_SEPARATOR,

      GTK_MENU_RADIO_ENTRY("local_video", _("_Local Video"),
			   _("Local video image"),
			   NULL, '1', 
			   G_CALLBACK (display_changed_cb),
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   TRUE, FALSE),
      GTK_MENU_RADIO_ENTRY("remote_video", _("_Remote Video"),
			   _("Remote video image"),
			   NULL, '2', 
			   G_CALLBACK (display_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_incrusted", _("_Picture-in-Picture"),
			   _("Both video images"),
			   NULL, '3', 
			   G_CALLBACK (display_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_RADIO_ENTRY("both_incrusted_window", _("Picture-in-Picture in Separate _Window"),
			   _("Both video images"),
			   NULL, '4', 
			   G_CALLBACK (display_changed_cb), 
			   (gpointer) VIDEO_DISPLAY_KEY "video_view",
			   FALSE, FALSE),
      GTK_MENU_SEPARATOR,

      GTK_MENU_ENTRY("zoom_in", NULL, _("Zoom in"), 
		     GTK_STOCK_ZOOM_IN, '+', 
		     G_CALLBACK (zoom_in_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom", FALSE),
      GTK_MENU_ENTRY("zoom_out", NULL, _("Zoom out"), 
		     GTK_STOCK_ZOOM_OUT, '-', 
		     G_CALLBACK (zoom_out_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom", FALSE),
      GTK_MENU_ENTRY("normal_size", NULL, _("Normal size"), 
		     GTK_STOCK_ZOOM_100, '0',
		     G_CALLBACK (zoom_normal_changed_cb),
		     (gpointer) VIDEO_DISPLAY_KEY "zoom", FALSE),
      GTK_MENU_ENTRY("fullscreen", _("_Fullscreen"), _("Switch to fullscreen"), 
		     GTK_STOCK_ZOOM_IN, GDK_F11, 
		     G_CALLBACK (fullscreen_changed_cb),
		     (gpointer) mw, FALSE),

      GTK_MENU_NEW(_("_Help")),

      GTK_MENU_ENTRY("help", NULL, 
                     _("Get help by reading the Ekiga manual"),
                     GTK_STOCK_HELP, GDK_F1, 
                     G_CALLBACK (help_cb), NULL, TRUE),

      GTK_MENU_ENTRY("about", NULL,
		     _("View information about Ekiga"),
		     GTK_STOCK_ABOUT, 0, 
		     G_CALLBACK (about_callback), (gpointer) mw,
		     TRUE),
       
      GTK_MENU_END
    };


  gtk_build_menu (mw->priv->main_menu, 
		  gnomemeeting_menu, 
		  mw->priv->accel, 
		  mw->priv->statusbar);

  gtk_widget_show_all (GTK_WIDGET (mw->priv->main_menu));
}


static void
ekiga_main_window_init_uri_toolbar (EkigaMainWindow *mw)
{
  GtkToolItem *item = NULL;
  GtkEntryCompletion *completion = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The main horizontal toolbar */
  mw->priv->main_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->main_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->main_toolbar), FALSE);

  /* URL bar */
  /* Entry */
  item = gtk_tool_item_new ();
  mw->priv->entry = gtk_entry_new ();
  mw->priv->completion = gtk_list_store_new (1, G_TYPE_STRING);
  completion = gtk_entry_completion_new ();
  gtk_entry_completion_set_model (GTK_ENTRY_COMPLETION (completion), GTK_TREE_MODEL (mw->priv->completion));
  gtk_entry_set_completion (GTK_ENTRY (mw->priv->entry), completion);
  gtk_entry_completion_set_inline_completion (GTK_ENTRY_COMPLETION (completion), false);
  gtk_entry_completion_set_popup_completion (GTK_ENTRY_COMPLETION (completion), true);
  gtk_entry_completion_set_text_column (GTK_ENTRY_COMPLETION (completion), 0);

  gtk_container_add (GTK_CONTAINER (item), mw->priv->entry);
  gtk_container_set_border_width (GTK_CONTAINER (item), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), true);
  
  gtk_entry_set_text (GTK_ENTRY (mw->priv->entry), "sip:");

  // activate Ctrl-L to get the entry focus
  gtk_widget_add_accelerator (mw->priv->entry, "grab-focus",
			      mw->priv->accel, GDK_L,
			      (GdkModifierType) GDK_CONTROL_MASK,
			      (GtkAccelFlags) 0);

  gtk_editable_set_position (GTK_EDITABLE (mw->priv->entry), -1);

  g_signal_connect (G_OBJECT (mw->priv->entry), "changed", 
		    G_CALLBACK (url_changed_cb), mw);
  g_signal_connect (G_OBJECT (mw->priv->entry), "activate", 
		    G_CALLBACK (place_call_cb), mw);

  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->main_toolbar), item, 0);

  /* The connect button */
  item = gtk_tool_item_new ();
  mw->priv->connect_button = gm_connect_button_new (GM_STOCK_PHONE_PICK_UP_24,
						    GM_STOCK_PHONE_HANG_UP_24,
						    GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_container_add (GTK_CONTAINER (item), mw->priv->connect_button);
  gtk_container_set_border_width (GTK_CONTAINER (mw->priv->connect_button), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);

  gtk_widget_set_tooltip_text (GTK_WIDGET (mw->priv->connect_button),
			       _("Enter a URI on the left, and click this button to place a call or to hangup"));
  
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->main_toolbar), item, -1);

  g_signal_connect (G_OBJECT (mw->priv->connect_button), "clicked",
                    G_CALLBACK (toggle_call_cb), 
                    mw);

  gtk_widget_show_all (GTK_WIDGET (mw->priv->main_toolbar));
}


static void
ekiga_main_window_init_status_toolbar (EkigaMainWindow *mw)
{
  GtkToolItem *item = NULL;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* The main horizontal toolbar */
  mw->priv->status_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->status_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->status_toolbar), FALSE);

  item = gtk_tool_item_new ();
  mw->priv->status_option_menu = status_menu_new (*mw->priv->core); 
  status_menu_set_parent_window (STATUS_MENU (mw->priv->status_option_menu), 
                                 GTK_WINDOW (mw));
  gtk_container_add (GTK_CONTAINER (item), mw->priv->status_option_menu);
  gtk_container_set_border_width (GTK_CONTAINER (item), 0);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), TRUE);
  
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->status_toolbar), item, 0);

  gtk_widget_show_all (mw->priv->status_toolbar);
}


static void 
ekiga_main_window_init_contact_list (EkigaMainWindow *mw)
{
  GtkWidget *label = NULL;

  Ekiga::ServiceCore *services = NULL;
  GtkWidget* roster_view = NULL;

  services = GnomeMeeting::Process ()->GetServiceCore ();
  g_return_if_fail (services != NULL);

  gmref_ptr<Ekiga::PresenceCore> presence_core = services->get ("presence-core");

  label = gtk_label_new (_("Contacts"));
  roster_view = roster_view_gtk_new (*presence_core);
  gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook),
			      roster_view, label);
  g_signal_connect (G_OBJECT (roster_view), "presentity-selected",
		    G_CALLBACK (on_presentity_selected), mw);
}


static void 
ekiga_main_window_init_dialpad (EkigaMainWindow *mw)
{
  GtkWidget *dialpad = NULL;
  GtkWidget *alignment = NULL;
  GtkWidget *label = NULL;

  dialpad = ekiga_dialpad_new (mw->priv->accel);
  g_signal_connect (G_OBJECT (dialpad), "button-clicked",
                    G_CALLBACK (dialpad_button_clicked_cb), mw);

  alignment = gtk_alignment_new (0.5, 0.5, 0.2, 0.2);
  gtk_container_add (GTK_CONTAINER (alignment), dialpad);

  label = gtk_label_new (_("Dialpad"));
  gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook),
			    alignment, label);
}


static void
ekiga_main_window_init_history (EkigaMainWindow *mw)
{
  Ekiga::ServiceCore *services = NULL;
  GtkWidget *label = NULL;

  services = GnomeMeeting::Process ()->GetServiceCore ();
  g_return_if_fail (services != NULL);

  gmref_ptr<History::Source> history_source = services->get ("call-history-store");
  gmref_ptr<History::Book> history_book = history_source->get_book ();
  GtkWidget* call_history_view = call_history_view_gtk_new (history_book);

  label = gtk_label_new (_("Call history"));
  gtk_notebook_append_page (GTK_NOTEBOOK (mw->priv->main_notebook),
			    call_history_view,
			    label);
}


static void 
ekiga_main_window_init_call_panel (EkigaMainWindow *mw)
{
  GtkWidget *event_box = NULL;
  GtkWidget *table = NULL;

  GtkToolItem *item = NULL;

  GtkWidget *image = NULL;
  GtkWidget *alignment = NULL;

  /* The main table */
  mw->priv->call_panel_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->priv->call_panel_frame), GTK_SHADOW_IN);
  event_box = gtk_event_box_new ();
  table = gtk_table_new (3, 4, FALSE);
  gtk_container_add (GTK_CONTAINER (event_box), table);
  gtk_container_add (GTK_CONTAINER (mw->priv->call_panel_frame), event_box);

  /* The frame that contains the video */
  mw->priv->video_frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (mw->priv->video_frame), 
                             GTK_SHADOW_NONE);
  
  mw->priv->main_video_image = gtk_image_new ();
  gtk_container_set_border_width (GTK_CONTAINER (mw->priv->video_frame), 0);
  gtk_container_add (GTK_CONTAINER (mw->priv->video_frame), mw->priv->main_video_image);
  gtk_table_attach (GTK_TABLE (table), GTK_WIDGET (mw->priv->video_frame),
                    0, 4, 0, 1,         
                    (GtkAttachOptions) GTK_EXPAND,      
                    (GtkAttachOptions) GTK_EXPAND,      
                    24, 24);

  /* The frame that contains information about the call */
  /* Text buffer */
  GtkTextBuffer *buffer = NULL;
  
  mw->priv->info_text = gtk_text_view_new ();
  gtk_text_view_set_editable (GTK_TEXT_VIEW (mw->priv->info_text), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->info_text), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (mw->priv->info_text),
			       GTK_WRAP_WORD);

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (mw->priv->info_text));
  gtk_text_view_set_cursor_visible  (GTK_TEXT_VIEW (mw->priv->info_text), FALSE);

  mw->priv->status_tag =
    gtk_text_buffer_create_tag (buffer, "status",
                                "justification", GTK_JUSTIFY_CENTER,
                                "weight", PANGO_WEIGHT_BOLD,
                                "scale", 1.2,
                                NULL);
  mw->priv->codecs_tag =
    gtk_text_buffer_create_tag (buffer, "codecs",
                                "justification", GTK_JUSTIFY_RIGHT,
                                "stretch", PANGO_STRETCH_CONDENSED,
                                NULL);

  mw->priv->call_duration_tag =
    gtk_text_buffer_create_tag (buffer, "call-duration",
                                "justification", GTK_JUSTIFY_CENTER,
                                "weight", PANGO_WEIGHT_BOLD,
                                NULL);

  ekiga_main_window_set_status (mw, _("Standby"));
  ekiga_main_window_set_call_duration (mw, NULL);
  ekiga_main_window_set_call_info (mw, NULL, NULL, NULL, NULL);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (alignment), mw->priv->info_text);
  gtk_table_attach (GTK_TABLE (table), alignment,
                    0, 4, 1, 2,         
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    0, 0);
 
  /* The toolbar */
  mw->priv->call_panel_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (mw->priv->call_panel_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (mw->priv->call_panel_toolbar), FALSE);

  /* Audio Volume */
  item = gtk_tool_item_new ();
  mw->priv->audio_settings_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (mw->priv->audio_settings_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_icon_name (GM_ICON_AUDIO_VOLUME_HIGH,
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (mw->priv->audio_settings_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->priv->audio_settings_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->priv->audio_settings_button);
  gtk_widget_set_sensitive (mw->priv->audio_settings_button, FALSE);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				  _("Change the volume of your soundcard"));
  g_signal_connect (G_OBJECT (mw->priv->audio_settings_button), "clicked",
		    G_CALLBACK (show_window_cb),
		    (gpointer) mw->priv->audio_settings_window);
  
  /* Video Settings */
  item = gtk_tool_item_new ();
  mw->priv->video_settings_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (mw->priv->video_settings_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (GM_STOCK_COLOR_BRIGHTNESS_CONTRAST,
                                    GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (mw->priv->video_settings_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->priv->video_settings_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->priv->video_settings_button);
  gtk_widget_set_sensitive (mw->priv->video_settings_button, FALSE);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				   _("Change the color settings of your video device"));

  g_signal_connect (G_OBJECT (mw->priv->video_settings_button), "clicked",
		    G_CALLBACK (show_window_cb),
		    (gpointer) mw->priv->video_settings_window);

  /* Video Preview Button */
  item = gtk_tool_item_new ();
  mw->priv->preview_button = gtk_toggle_button_new ();
  gtk_button_set_relief (GTK_BUTTON (mw->priv->preview_button), GTK_RELIEF_NONE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (mw->priv->preview_button),
                                gm_conf_get_bool (VIDEO_DEVICES_KEY "enable_preview"));
  image = gtk_image_new_from_icon_name (GM_ICON_CAMERA_VIDEO, 
                                        GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (mw->priv->preview_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->priv->preview_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->priv->preview_button);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				  _("Display images from your camera device"));

  g_signal_connect (G_OBJECT (mw->priv->preview_button), "toggled",
		    G_CALLBACK (toolbar_toggle_button_changed_cb),
		    (gpointer) VIDEO_DEVICES_KEY "enable_preview");

  /* Call Pause */
  item = gtk_tool_item_new ();
  mw->priv->hold_button = gtk_toggle_button_new ();
  image = gtk_image_new_from_icon_name (GM_ICON_MEDIA_PLAYBACK_PAUSE,
                                        GTK_ICON_SIZE_MENU);
  gtk_button_set_relief (GTK_BUTTON (mw->priv->hold_button), GTK_RELIEF_NONE);
  gtk_container_add (GTK_CONTAINER (mw->priv->hold_button), image);
  gtk_container_add (GTK_CONTAINER (item), mw->priv->hold_button);
  gtk_tool_item_set_expand (GTK_TOOL_ITEM (item), FALSE);
  
  gtk_widget_show (mw->priv->hold_button);
  gtk_toolbar_insert (GTK_TOOLBAR (mw->priv->call_panel_toolbar),
		      GTK_TOOL_ITEM (item), -1);
  gtk_tool_item_set_tooltip_text (GTK_TOOL_ITEM (item),
				  _("Hold the current call"));
  gtk_widget_set_sensitive (GTK_WIDGET (mw->priv->hold_button), FALSE);

  g_signal_connect (G_OBJECT (mw->priv->hold_button), "clicked",
		    G_CALLBACK (hold_current_call_cb), mw);

  alignment = gtk_alignment_new (0.0, 0.0, 1.0, 0.0);
  gtk_container_add (GTK_CONTAINER (alignment), mw->priv->call_panel_toolbar);
  gtk_table_attach (GTK_TABLE (table), alignment,
                    1, 3, 2, 3,         
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                    0, 0);

  gtk_paned_pack2 (GTK_PANED (mw->priv->hpaned), mw->priv->call_panel_frame, true, false);
  gtk_widget_realize (mw->priv->main_video_image);
}

static void
ekiga_main_window_style_notify (GtkWidget *widget)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (widget);
  GtkStyle *style = gtk_widget_get_style (widget);
  GdkColor *bgcolor = &style->light[GTK_STATE_NORMAL];
  GtkWidget *w;

  /**
   * We defer setting the custom colors till the "notify::style" signal is
   * emitted. The reason is the style settings are not set correctly before
   * and so we'd get wrong colors.
   * Also, this signal allows us to keep the colors fine when the user changes
   * his theme from the gnome capplet.
   */

  gtk_widget_modify_bg (mw->priv->call_panel_frame, GTK_STATE_PRELIGHT, bgcolor);
  gtk_widget_modify_bg (mw->priv->call_panel_frame, GTK_STATE_NORMAL, bgcolor);

  w = gtk_bin_get_child (GTK_BIN (mw->priv->call_panel_frame));
  gtk_widget_modify_bg (w, GTK_STATE_PRELIGHT, bgcolor);
  gtk_widget_modify_bg (w, GTK_STATE_NORMAL, bgcolor);

  gtk_widget_modify_bg (mw->priv->info_text, GTK_STATE_PRELIGHT, bgcolor);
  gtk_widget_modify_bg (mw->priv->info_text, GTK_STATE_NORMAL, bgcolor);
  gtk_widget_modify_bg (mw->priv->info_text, GTK_STATE_INSENSITIVE, bgcolor);
  gtk_widget_modify_base (mw->priv->info_text, GTK_STATE_PRELIGHT, bgcolor);
  gtk_widget_modify_base (mw->priv->info_text, GTK_STATE_NORMAL, bgcolor);
  gtk_widget_modify_base (mw->priv->info_text, GTK_STATE_INSENSITIVE, bgcolor);

  g_object_set (mw->priv->status_tag,
                "foreground-gdk", &style->text[GTK_STATE_NORMAL],
                NULL);
  g_object_set (mw->priv->codecs_tag,
                "foreground-gdk", &style->text_aa[GTK_STATE_NORMAL],
                NULL);
  g_object_set (mw->priv->call_duration_tag,
                "foreground-gdk", &style->text[GTK_STATE_NORMAL],
                NULL);

  gtk_widget_modify_bg (mw->priv->call_panel_toolbar, GTK_STATE_PRELIGHT, bgcolor);
  gtk_widget_modify_bg (mw->priv->call_panel_toolbar, GTK_STATE_NORMAL, bgcolor);
}

static void
ekiga_main_window_init_gui (EkigaMainWindow *mw)
{
  GtkWidget *window_vbox;
  GtkShadowType shadow_type;
  GtkWidget *frame;

  gtk_window_set_title (GTK_WINDOW (mw), _("Ekiga"));

  window_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (mw), window_vbox);
  gtk_widget_show_all (window_vbox);

  /* The main menu */
  mw->priv->statusbar = gm_statusbar_new ();

  ekiga_main_window_init_menu (mw); 
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->main_menu,
                      FALSE, FALSE, 0);

  /* The URI toolbar */
  ekiga_main_window_init_uri_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->main_toolbar,
                      false, false, 0); 

  /* The Audio & Video Settings windows */
  mw->priv->audio_settings_window = gm_mw_audio_settings_window_new (mw);
  mw->priv->video_settings_window = gm_mw_video_settings_window_new (mw);

  /* The 2 parts of the gui */
  mw->priv->hpaned = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->hpaned,
                      true, true, 0);

  mw->priv->main_notebook = gtk_notebook_new ();
  gtk_notebook_popup_enable (GTK_NOTEBOOK (mw->priv->main_notebook));
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (mw->priv->main_notebook), true);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (mw->priv->main_notebook), true);

  ekiga_main_window_init_contact_list (mw);
  ekiga_main_window_init_dialpad (mw);
  ekiga_main_window_init_history (mw);
  gtk_paned_pack1 (GTK_PANED (mw->priv->hpaned), mw->priv->main_notebook, true, false);

  ekiga_main_window_init_call_panel (mw);

  /* The status toolbar */
  ekiga_main_window_init_status_toolbar (mw);
  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->status_toolbar,
                      false, false, 0); 

  /* The statusbar with qualitymeter */
  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (mw->priv->statusbar), TRUE);
  gtk_widget_style_get (mw->priv->statusbar, "shadow-type", &shadow_type, NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), shadow_type);
  gtk_box_pack_start (GTK_BOX (mw->priv->statusbar), frame, FALSE, TRUE, 0);

  gtk_box_reorder_child (GTK_BOX (mw->priv->statusbar), frame, 0);

  mw->priv->qualitymeter = gm_powermeter_new ();
  gtk_container_add (GTK_CONTAINER (frame), mw->priv->qualitymeter);

  mw->priv->statusbar_ebox = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (mw->priv->statusbar_ebox), mw->priv->statusbar);

  gtk_box_pack_start (GTK_BOX (window_vbox), mw->priv->statusbar_ebox,
                      FALSE, FALSE, 0);
  gtk_widget_show_all (mw->priv->statusbar_ebox);

  g_signal_connect (G_OBJECT (mw->priv->statusbar_ebox), "button-press-event",
		    G_CALLBACK (statusbar_clicked_cb), mw);
 
  g_signal_connect (mw, "notify::style",
                    G_CALLBACK (ekiga_main_window_style_notify), NULL);

  gtk_widget_realize (GTK_WIDGET (mw));
  ekiga_main_window_update_logo_have_window (mw);
  g_signal_connect_after (G_OBJECT (mw->priv->main_notebook), "switch-page",
			  G_CALLBACK (panel_section_changed_cb), 
			  mw);

  /* Show the current panel section */
  PanelSection section = (PanelSection)
               gm_conf_get_int (USER_INTERFACE_KEY "main_window/panel_section");
  gtk_widget_show (mw->priv->hpaned);
  gtk_widget_show_all (gtk_paned_get_child1 (GTK_PANED (mw->priv->hpaned)));
  if (gm_conf_get_bool (USER_INTERFACE_KEY "main_window/show_call_panel"))
    gtk_widget_show_all (gtk_paned_get_child2 (GTK_PANED (mw->priv->hpaned)));
  ekiga_main_window_set_panel_section (mw, section);
}

static void
ekiga_main_window_init (EkigaMainWindow *mw)
{
  mw->priv = new EkigaMainWindowPrivate ();

  /* Accelerators */
  mw->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (mw), mw->priv->accel);
  g_object_unref (mw->priv->accel);

  mw->priv->presentity = NULL;
  mw->priv->transfer_call_popup = NULL;
  mw->priv->current_call = gmref_ptr<Ekiga::Call>(0);
  mw->priv->timeout_id = -1;
  mw->priv->levelmeter_timeout_id = -1;
  mw->priv->calling_state = Standby;
  mw->priv->audio_transmission_active = false;
  mw->priv->audio_reception_active = false;
  mw->priv->video_transmission_active = false;
  mw->priv->video_reception_active = false;
#ifndef WIN32
  mw->priv->video_widget_gc = NULL;
#endif
}

static GObject *
ekiga_main_window_constructor (GType the_type,
                               guint n_construct_properties,
                               GObjectConstructParam *construct_params)
{
  GObject *object;

  object = G_OBJECT_CLASS (ekiga_main_window_parent_class)->constructor
                          (the_type, n_construct_properties, construct_params);

  ekiga_main_window_init_gui (EKIGA_MAIN_WINDOW (object));

  /* GConf Notifiers */
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/panel_section",
                        panel_section_changed_nt, object);
  gm_conf_notifier_add (USER_INTERFACE_KEY "main_window/show_call_panel",
                        show_call_panel_changed_nt, object);
  gm_conf_notifier_add (VIDEO_DISPLAY_KEY "stay_on_top", 
			stay_on_top_changed_nt, object);


  return object;
}

static void
ekiga_main_window_finalize (GObject *gobject)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (gobject);

  gtk_widget_destroy (mw->priv->audio_settings_window);
  gtk_widget_destroy (mw->priv->video_settings_window);

  delete mw->priv;

  G_OBJECT_CLASS (ekiga_main_window_parent_class)->finalize (gobject);
}

static void
ekiga_main_window_show (GtkWidget *widget)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (widget);
  if (gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top") && mw->priv->current_call)
    gdk_window_set_always_on_top (widget->window, TRUE);
  GTK_WIDGET_CLASS (ekiga_main_window_parent_class)->show (widget);
}

static gboolean
ekiga_main_window_expose_event (GtkWidget      *widget,
                                GdkEventExpose *event)
{
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (widget);
  GtkWidget* video_widget = mw->priv->main_video_image;
  Ekiga::DisplayInfo display_info;
  gboolean handled = FALSE;

  handled = GTK_WIDGET_CLASS (ekiga_main_window_parent_class)->expose_event (widget, event);

  display_info.x = video_widget->allocation.x;
  display_info.y = video_widget->allocation.y;

#ifdef WIN32  
  display_info.hwnd = ((HWND) GDK_WINDOW_HWND (video_widget->window));
#else 
  if (!mw->priv->video_widget_gc) {
    mw->priv->video_widget_gc = gdk_gc_new (video_widget->window);
    g_return_val_if_fail (mw->priv->video_widget_gc != NULL, handled);
  }

  display_info.gc = GDK_GC_XGC (mw->priv->video_widget_gc);
  display_info.xdisplay = GDK_GC_XDISPLAY (mw->priv->video_widget_gc);
  display_info.window = GDK_WINDOW_XWINDOW (video_widget->window);

  g_return_val_if_fail (display_info.window != 0, handled);

  gdk_flush();
#endif

  display_info.widget_info_set = TRUE;

  gmref_ptr<Ekiga::VideoOutputCore> videooutput_core = mw->priv->core->get ("videooutput-core");
  videooutput_core->set_display_info (display_info);

  return handled;
}

static gboolean
ekiga_main_window_focus_in_event (GtkWidget     *widget,
                                  GdkEventFocus *event)
{
  if (gtk_window_get_urgency_hint (GTK_WINDOW (widget)))
    gtk_window_set_urgency_hint (GTK_WINDOW (widget), FALSE);

  return GTK_WIDGET_CLASS (ekiga_main_window_parent_class)->focus_in_event (widget, event);
}

static gboolean
ekiga_main_window_delete_event (GtkWidget   *widget,
				G_GNUC_UNUSED GdkEventAny *event)
{
  GtkStatusIcon *statusicon = NULL;

  statusicon = GTK_STATUS_ICON (GnomeMeeting::Process ()->GetStatusicon ());

  if (!gtk_status_icon_is_embedded (statusicon))
    quit_callback (NULL, widget);
  else
    gtk_widget_hide (widget);

  return TRUE;
}

static void
ekiga_main_window_get_property (GObject *object,
                                guint property_id,
                                GValue *value,
                                GParamSpec *pspec)
{
  EkigaMainWindow *mw;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (object));

  mw = EKIGA_MAIN_WINDOW (object);

  switch (property_id) {
    case PROP_SERVICE_CORE:
      g_value_set_pointer (value, mw->priv->core);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}


static void
ekiga_main_window_set_property (GObject *object,
                                guint property_id,
                                const GValue *value,
                                GParamSpec *pspec)
{
  EkigaMainWindow *mw;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (object));

  mw = EKIGA_MAIN_WINDOW (object);

  switch (property_id) {
    case PROP_SERVICE_CORE:
      mw->priv->core = static_cast<Ekiga::ServiceCore *>
                                                 (g_value_get_pointer (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}


static void
ekiga_main_window_class_init (EkigaMainWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = ekiga_main_window_constructor;
  object_class->finalize = ekiga_main_window_finalize;
  object_class->get_property = ekiga_main_window_get_property;
  object_class->set_property = ekiga_main_window_set_property;

  widget_class->show = ekiga_main_window_show;
  widget_class->expose_event = ekiga_main_window_expose_event;
  widget_class->focus_in_event = ekiga_main_window_focus_in_event;
  widget_class->delete_event = ekiga_main_window_delete_event;

  g_object_class_install_property (object_class,
                                   PROP_SERVICE_CORE,
                                   g_param_spec_pointer ("service-core",
                                                         "Service Core",
                                                         "Service Core",
                                                         (GParamFlags)
                                                         (G_PARAM_READWRITE |
                                                          G_PARAM_CONSTRUCT_ONLY)));
}

static void
ekiga_main_window_connect_engine_signals (EkigaMainWindow *mw)
{
  sigc::connection conn;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  /* New Display Engine signals */
  gmref_ptr<Ekiga::VideoOutputCore> videooutput_core
    = mw->priv->core->get ("videooutput-core");

  conn = videooutput_core->device_opened.connect (sigc::bind (sigc::ptr_fun (on_videooutput_device_opened_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videooutput_core->device_closed.connect (sigc::bind (sigc::ptr_fun (on_videooutput_device_closed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videooutput_core->device_error.connect (sigc::bind (sigc::ptr_fun (on_videooutput_device_error_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videooutput_core->size_changed.connect (sigc::bind (sigc::ptr_fun (on_size_changed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videooutput_core->fullscreen_mode_changed.connect (sigc::bind (sigc::ptr_fun (on_fullscreen_mode_changed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  /* New VideoInput Engine signals */
  gmref_ptr<Ekiga::VideoInputCore> videoinput_core
    = mw->priv->core->get ("videoinput-core");

  conn = videoinput_core->device_opened.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_opened_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videoinput_core->device_closed.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_closed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videoinput_core->device_added.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_added_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videoinput_core->device_removed.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_removed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = videoinput_core->device_error.connect (sigc::bind (sigc::ptr_fun (on_videoinput_device_error_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  /* New AudioInput Engine signals */
  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = mw->priv->core->get ("audioinput-core");

  conn = audioinput_core->device_opened.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_opened_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audioinput_core->device_closed.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_closed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audioinput_core->device_added.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_added_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audioinput_core->device_removed.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_removed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audioinput_core->device_error.connect (sigc::bind (sigc::ptr_fun (on_audioinput_device_error_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  /* New AudioOutput Engine signals */
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = mw->priv->core->get ("audiooutput-core");

  conn = audiooutput_core->device_opened.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_opened_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audiooutput_core->device_closed.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_closed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audiooutput_core->device_added.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_added_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audiooutput_core->device_removed.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_removed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = audiooutput_core->device_error.connect (sigc::bind (sigc::ptr_fun (on_audiooutput_device_error_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
    
  /* New Call Engine signals */
  gmref_ptr<Ekiga::CallCore> call_core = mw->priv->core->get ("call-core");
  gmref_ptr<Ekiga::AccountCore> account_core = mw->priv->core->get ("account-core");

  /* Engine Signals callbacks */
  conn = account_core->registration_event.connect (sigc::bind (sigc::ptr_fun (on_registration_event), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->setup_call.connect (sigc::bind (sigc::ptr_fun (on_setup_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->ringing_call.connect (sigc::bind (sigc::ptr_fun (on_ringing_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
  
  conn = call_core->established_call.connect (sigc::bind (sigc::ptr_fun (on_established_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
  
  conn = call_core->cleared_call.connect (sigc::bind (sigc::ptr_fun (on_cleared_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
  
  conn = call_core->held_call.connect (sigc::bind (sigc::ptr_fun (on_held_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
  
  conn = call_core->retrieved_call.connect (sigc::bind (sigc::ptr_fun (on_retrieved_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
  
  conn = call_core->missed_call.connect (sigc::bind (sigc::ptr_fun (on_missed_call_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->stream_opened.connect (sigc::bind (sigc::ptr_fun (on_stream_opened_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);
  
  conn = call_core->stream_closed.connect (sigc::bind (sigc::ptr_fun (on_stream_closed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->stream_paused.connect (sigc::bind (sigc::ptr_fun (on_stream_paused_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->stream_resumed.connect (sigc::bind (sigc::ptr_fun (on_stream_resumed_cb), (gpointer) mw));
  mw->priv->connections.push_back (conn);

  conn = call_core->errors.add_handler (sigc::bind (sigc::ptr_fun (on_handle_errors), (gpointer) mw));
  mw->priv->connections.push_back (conn);
}

GtkWidget *
ekiga_main_window_new (Ekiga::ServiceCore *core)
{
  EkigaMainWindow *mw;

  mw = EKIGA_MAIN_WINDOW (g_object_new (EKIGA_TYPE_MAIN_WINDOW,
                                        "service-core", core, NULL));
  gm_window_set_key (GM_WINDOW (mw), USER_INTERFACE_KEY "main_window");
  ekiga_main_window_connect_engine_signals (mw);

  return GTK_WIDGET (mw);
}

GtkWidget *
gm_main_window_new (Ekiga::ServiceCore & core)
{
  GtkWidget *window = NULL;
  EkigaMainWindow *mw = NULL;
  
  GtkStatusIcon *status_icon = NULL;

  GtkWidget *chat_window = NULL;

  /* initialize the callback to play IM message sound */
  /* FIXME: move this to the chat window code */
  gmref_ptr<GtkFrontend> gtk_frontend = core.get ("gtk-frontend");
  chat_window = GTK_WIDGET (gtk_frontend->get_chat_window ());

  g_signal_connect (chat_window, "unread-alert",
		    G_CALLBACK (on_chat_unread_alert), NULL);

  /* The Top-level window */
  window = ekiga_main_window_new (&core);
  mw = EKIGA_MAIN_WINDOW (window);

  /* Track status icon embed changes */
  /* FIXME: move this to the status icon code */
  status_icon = GTK_STATUS_ICON (GnomeMeeting::Process ()->GetStatusicon ());
  g_signal_connect (G_OBJECT (status_icon), "notify::embedded",
		    G_CALLBACK (on_status_icon_embedding_change), NULL);

  return window;
}


void 
ekiga_main_window_flash_message (EkigaMainWindow *mw,
				 const char *msg,
				 ...)
{
  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else 
    vsnprintf (buffer, 1024, msg, args);

  gm_statusbar_flash_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  va_end (args);
}


void 
ekiga_main_window_push_message (EkigaMainWindow *mw, 
				const char *msg, 
				...)
{
  char buffer [1025];
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);

  if (msg == NULL)
    buffer[0] = 0;
  else 
    vsnprintf (buffer, 1024, msg, args);

  gm_statusbar_push_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  va_end (args);
}


void 
ekiga_main_window_push_info_message (EkigaMainWindow *mw, 
				     const char *msg, 
				     ...)
{
  char *buffer;
  va_list args;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  va_start (args, msg);
  buffer = g_strdup_vprintf (msg, args);
  gm_statusbar_push_info_message (GM_STATUSBAR (mw->priv->statusbar), "%s", buffer);
  g_free (buffer);
  va_end (args);
}


void 
ekiga_main_window_set_call_url (EkigaMainWindow *mw, 
				const char *url)
{
  g_return_if_fail (mw != NULL && url != NULL);

  gtk_entry_set_text (GTK_ENTRY (mw->priv->entry), url);
  gtk_editable_set_position (GTK_EDITABLE (mw->priv->entry), -1);
  gtk_widget_grab_focus (GTK_WIDGET (mw->priv->entry));
  gtk_editable_select_region (GTK_EDITABLE (mw->priv->entry), -1, -1);
}


void 
ekiga_main_window_append_call_url (EkigaMainWindow *mw, 
				   const char *url)
{
  int pos = -1;
  GtkEditable *entry;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));
  g_return_if_fail (url != NULL);

  entry = GTK_EDITABLE (mw->priv->entry);

  if (gtk_editable_get_selection_bounds (entry, NULL, NULL)) 
    gtk_editable_delete_selection (entry);

  pos = gtk_editable_get_position (entry);
  gtk_editable_insert_text (entry, url, strlen (url), &pos);
  gtk_editable_select_region (entry, -1, -1);
  gtk_editable_set_position (entry, pos);
}


static const std::string
ekiga_main_window_get_call_url (EkigaMainWindow *mw)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (mw), NULL);

  const gchar* entry_text = gtk_entry_get_text (GTK_ENTRY (mw->priv->entry));

  if (entry_text != NULL)
    return entry_text;
  else
    return "";
}


void 
ekiga_main_window_clear_stats (EkigaMainWindow *mw)
{
  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  ekiga_main_window_update_stats (mw, 0, 0, 0, 0, 0, 0, 0, 0);
  if (mw->priv->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (mw->priv->qualitymeter), 0.0);
}


void 
ekiga_main_window_update_stats (EkigaMainWindow *mw,
				float lost,
				float late,
				float out_of_order,
				int jitter,
				unsigned int re_width,
				unsigned int re_height,
				unsigned int tr_width,
				unsigned int tr_height)
{
  gchar *stats_msg = NULL;
  gchar *stats_msg_tr = NULL;
  gchar *stats_msg_re = NULL;

  int jitter_quality = 0;
  gfloat quality_level = 0.0;

  g_return_if_fail (EKIGA_IS_MAIN_WINDOW (mw));

  if ((tr_width > 0) && (tr_height > 0))
    /* Translators:
     * TX is a common abbreviation for "transmit" */
    stats_msg_tr = g_strdup_printf (_("TX: %dx%d "), tr_width, tr_height);

  if ((re_width > 0) && (re_height > 0))
    /* Translators:
     * RX is a common abbreviation for "receive" */
    stats_msg_re = g_strdup_printf (_("RX: %dx%d "), re_width, re_height);

  stats_msg = g_strdup_printf (_("Lost packets: %.1f %%\nLate packets: %.1f %%\nOut of order packets: %.1f %%\nJitter buffer: %d ms%s%s%s"), 
                                  lost, 
                                  late, 
                                  out_of_order, 
                                  jitter,
                                  (stats_msg_tr || stats_msg_re) ? "\nResolution: " : "", 
                                  (stats_msg_tr) ? stats_msg_tr : "", 
                                  (stats_msg_re) ? stats_msg_re : "");

  g_free(stats_msg_tr);
  g_free(stats_msg_re);


  if (mw->priv->statusbar_ebox) {
    gtk_widget_set_tooltip_text (GTK_WIDGET (mw->priv->statusbar_ebox), stats_msg);
  }
  g_free (stats_msg);

  /* "arithmetics" for the quality level */
  /* Thanks Snark for the math hints */
  if (jitter < 30)
    jitter_quality = 100;
  if (jitter >= 30 && jitter < 50)
    jitter_quality = 100 - (jitter - 30);
  if (jitter >= 50 && jitter < 100)
    jitter_quality = 80 - (jitter - 50) * 20 / 50;
  if (jitter >= 100 && jitter < 150)
    jitter_quality = 60 - (jitter - 100) * 20 / 50;
  if (jitter >= 150 && jitter < 200)
    jitter_quality = 40 - (jitter - 150) * 20 / 50;
  if (jitter >= 200 && jitter < 300)
    jitter_quality = 20 - (jitter - 200) * 20 / 100;
  if (jitter >= 300 || jitter_quality < 0)
    jitter_quality = 0;

  quality_level = (float) jitter_quality / 100;

  if ( (lost > 0.0) ||
       (late > 0.0) ||
       ((out_of_order > 0.0) && quality_level > 0.2) ) {
    quality_level = 0.2;
  }

  if ( (lost > 0.02) ||
       (late > 0.02) ||
       (out_of_order > 0.02) ) {
    quality_level = 0;
  }

  if (mw->priv->qualitymeter)
    gm_powermeter_set_level (GM_POWERMETER (mw->priv->qualitymeter),
			     quality_level);
}


GdkPixbuf *
ekiga_main_window_get_current_picture (EkigaMainWindow *mw)
{
  g_return_val_if_fail (EKIGA_IS_MAIN_WINDOW (mw), NULL);

  return gtk_image_get_pixbuf (GTK_IMAGE (mw->priv->main_video_image));
}

#ifdef WIN32
// the linker must not find main
#define main(c,v,e) ekigas_real_main(c,v,e)
#endif

/* The main () */
int 
main (int argc, 
      char ** argv, 
      char ** /*envp*/)
{
  GOptionContext *context = NULL;

  GtkWidget *main_window = NULL;
  GtkWidget *assistant_window = NULL;

  GtkWidget *dialog = NULL;
  
  gchar *path = NULL;
  gchar *url = NULL;
  gchar *msg = NULL;
  gchar *title = NULL;

  int debug_level = 0;
  int debug_level_up = 0;
  int error = -1;

  /* Globals */
#ifndef WIN32
  if (!XInitThreads ())
    exit (1);
#endif

  /* GTK+ initialization */
  g_type_init ();
  g_thread_init (NULL);
#ifndef WIN32
  signal (SIGPIPE, SIG_IGN);
#endif

  /* initialize platform-specific code */
  gm_platform_init ();

  /* Configuration backend initialization */
  gm_conf_init ();

  /* Gettext initialization */
  path = g_build_filename (DATA_DIR, "locale", NULL);
  textdomain (GETTEXT_PACKAGE);
  bindtextdomain (GETTEXT_PACKAGE, path);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  g_free (path);

  /* Arguments initialization */
  GOptionEntry arguments [] =
    {
      {
	"debug", 'd', 0, G_OPTION_ARG_INT, &debug_level, 
       N_("Prints debug messages in the console (level between 1 and 5)"), 
       NULL
      },
      {
	"debug_user_plane", 'u', 0, G_OPTION_ARG_INT, &debug_level_up, 
       N_("Prints user plane debug messages in the console (level between 1 and 4)"), 
       NULL
      },
      {
	"call", 'c', 0, G_OPTION_ARG_STRING, &url,
	N_("Makes Ekiga call the given URI"),
	NULL
      },
      {
	NULL, 0, 0, (GOptionArg)0, NULL,
	NULL,
	NULL
      }
    };
  context = g_option_context_new (NULL);
  g_option_context_add_main_entries (context, arguments, PACKAGE_NAME);
  g_option_context_set_help_enabled (context, TRUE);

  g_option_context_add_group (context, gtk_get_option_group (TRUE));
  g_option_context_parse (context, &argc, &argv, NULL);
  g_option_context_free (context);

#ifdef HAVE_NOTIFY
  notify_init (PACKAGE_NAME);
#endif

#ifndef WIN32
  char* text_label =  g_strdup_printf ("%d", debug_level);
  setenv ("PTLIB_TRACE_CODECS", text_label, TRUE);
  g_free (text_label);
  text_label =  g_strdup_printf ("%d", debug_level_up);
  setenv ("PTLIB_TRACE_CODECS_USER_PLANE", text_label, TRUE);
  g_free (text_label);
#else
  char* text_label =  g_strdup_printf ("PTLIB_TRACE_CODECS=%d", debug_level);
  _putenv (text_label);
  g_free (text_label);
  text_label =  g_strdup_printf ("PTLIB_TRACE_CODECS_USER_PLANE=%d", debug_level_up);
  _putenv (text_label);
  g_free (text_label);
  if (debug_level != 0) {
    std::string desk_path = g_get_user_special_dir (G_USER_DIRECTORY_DESKTOP);
    if (!desk_path.empty ())
      std::freopen((desk_path + "\\ekiga-stderr.txt").c_str (), "w", stderr);
  }
#endif

  /* Ekiga initialisation */
  static GnomeMeeting instance;

#if PTRACING
  if (debug_level != 0)
    PTrace::Initialise (PMAX (PMIN (5, debug_level), 0), NULL,
			PTrace::Timestamp | PTrace::Thread
			| PTrace::Blocks | PTrace::DateAndTime);
#endif

#ifdef EKIGA_REVISION
  PTRACE(1, "Ekiga git revision: " << EKIGA_REVISION);
#endif

#ifdef HAVE_DBUS
  if (!ekiga_dbus_claim_ownership ()) {
    ekiga_dbus_client_show ();
    if (url != NULL)
      ekiga_dbus_client_connect (url);
    exit (0);
  }
#endif

  /* Init gm_conf */
  gm_conf_watch ();

  GnomeMeeting::Process ()->InitEngine (argc, argv);
  GnomeMeeting::Process ()->BuildGUI ();

  /* Show the window if there is no error, exit with a popup if there
   * is a fatal error.
   */
  main_window = GnomeMeeting::Process ()->GetMainWindow ();
  EkigaMainWindow *mw = EKIGA_MAIN_WINDOW (main_window); //TODO no priv here
  gmref_ptr<Ekiga::CallCore> call_core = mw->priv->core->get ("call-core");
  if (error == -1) {

    if (gm_conf_get_int (GENERAL_KEY "version") 
        < 1000 * MAJOR_VERSION + 10 * MINOR_VERSION + BUILD_NUMBER) {

      gnomemeeting_conf_upgrade ();
      // Only show the assistant window if version older than 2.00
      if (gm_conf_get_int (GENERAL_KEY "version") < 2000) {
        assistant_window = GnomeMeeting::Process ()->GetAssistantWindow ();
        gtk_widget_show_all (assistant_window);
      }
      const int schema_version = MAJOR_VERSION * 1000
                               + MINOR_VERSION * 10
                               + BUILD_NUMBER;

      /* Update the version number */
      gm_conf_set_int (GENERAL_KEY "version", schema_version);
    }
    else {

      /* Show the main window */
      if (!gm_conf_get_bool (USER_INTERFACE_KEY "start_hidden")) 
        gtk_widget_show (main_window);
      else
#if GLIB_CHECK_VERSION (2, 14, 0)
        g_timeout_add_seconds (15, (GtkFunction) gnomemeeting_tray_hack_cb, NULL);
#else
        g_timeout_add (15000, (GtkFunction) gnomemeeting_tray_hack_cb, NULL);
#endif
    }

    /* Call the given host if needed */
    if (url) 
      call_core->dial (url);
  }
  else {

    switch (error) {

    case 1:
      title = g_strdup (_("No usable audio plugin detected"));
      msg = g_strdup (_("Ekiga didn't find any usable audio plugin. Make sure that your installation is correct."));
      break;
    case 2:
      title = g_strdup (_("No usable audio codecs detected"));
      msg = g_strdup (_("Ekiga didn't find any usable audio codec. Make sure that your installation is correct."));
      break;
    default:
      break;
    }

    dialog = gtk_message_dialog_new (GTK_WINDOW (main_window), 
                                     GTK_DIALOG_MODAL, 
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_OK, NULL);

    gtk_window_set_title (GTK_WINDOW (dialog), title);
    gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label), msg);
  
    g_signal_connect (GTK_OBJECT (dialog), "response",
                      G_CALLBACK (quit_callback),
                      GTK_OBJECT (dialog));
    g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                              G_CALLBACK (gtk_widget_destroy),
                              GTK_OBJECT (dialog));
  
    gtk_widget_show_all (dialog);

    g_free (title);
    g_free (msg);
  }

#ifdef HAVE_DBUS
  /* Create the dbus server instance */
  EkigaDBusComponent *dbus_component = ekiga_dbus_component_new (mw->priv->core);
#endif

  /* The GTK loop */
  gtk_main ();

#ifdef HAVE_DBUS
  g_object_unref (dbus_component);
#endif

  /* Exit Ekiga */
  GnomeMeeting::Process ()->Exit ();

  GnomeMeeting::Process ()->StopEngine ();

  /* Save and shutdown the configuration */
  gm_conf_save ();
  gm_conf_shutdown ();

  /* deinitialize platform-specific code */
  gm_platform_shutdown ();

#ifdef HAVE_NOTIFY
  notify_uninit ();
#endif

  return 0;
}


#ifdef WIN32

typedef struct {
  int newmode;
} _startupinfo;

extern "C" void __getmainargs (int *argcp, char ***argvp, char ***envp, int glob, _startupinfo *sinfo);
int 
APIENTRY WinMain (HINSTANCE hInstance,
		  HINSTANCE hPrevInstance,
		  LPSTR     lpCmdLine,
		  int       nCmdShow)
{
  HANDLE ekr_mutex;
  int iresult;
  char **env;
  char **argv;
  int argc;
  _startupinfo info = {0};

  ekr_mutex = CreateMutex (NULL, FALSE, "EkigaIsRunning");
  if (GetLastError () == ERROR_ALREADY_EXISTS)
    MessageBox (NULL, "Ekiga is running already !", "Ekiga - 2nd instance", MB_ICONEXCLAMATION | MB_OK);
  else {

    /* use msvcrt.dll to parse command line */
    __getmainargs (&argc, &argv, &env, 0, &info);

    iresult = main (argc, argv, env);
  }
  CloseHandle (ekr_mutex);
  return iresult;
}
#endif

