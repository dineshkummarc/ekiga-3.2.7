
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
 *                         main_window.h  -  description
 *                         -----------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#ifndef _MAIN_INTERFACE_H
#define _MAIN_INTERFACE_H

#include "services.h"
#include "call.h"

#include "gmwindow.h"

G_BEGIN_DECLS

#define EKIGA_TYPE_MAIN_WINDOW               (ekiga_main_window_get_type ())
#define EKIGA_MAIN_WINDOW(obj)               (G_TYPE_CHECK_INSTANCE_CAST ((obj), EKIGA_TYPE_MAIN_WINDOW, EkigaMainWindow))
#define EKIGA_MAIN_WINDOW_CLASS(klass)       (G_TYPE_CHECK_CLASS_CAST ((klass), EKIGA_TYPE_MAIN_WINDOW, EkigaMainWindowClass))
#define EKIGA_IS_MAIN_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_TYPE ((obj), EKIGA_TYPE_MAIN_WINDOW))
#define EKIGA_IS_MAIN_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE ((klass), EKIGA_TYPE_MAIN_WINDOW))

typedef struct _EkigaMainWindowPrivate       EkigaMainWindowPrivate;
typedef struct _EkigaMainWindow              EkigaMainWindow;

struct _EkigaMainWindow {
  GmWindow                parent;
  EkigaMainWindowPrivate *priv;
};

typedef GmWindowClass EkigaMainWindowClass;

GType        ekiga_main_window_get_type   ();
GtkWidget   *ekiga_main_window_new        (Ekiga::ServiceCore *core);

G_END_DECLS



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Press the given dialpad key.
 * PRE          :  The main window GMObject, the key to press (0 - 9, *, #).
 */
void gm_main_window_press_dialpad (GtkWidget *main_window,
				   const char c);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Return the video widget in which we can display.
 * PRE          :  The main window GMObject.
 */
GtkWidget *ekiga_main_window_get_video_widget (EkigaMainWindow *main_window);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the gnomemeeting logo in the video window.
 * PRE          :  The main window GMObject.
 */
void ekiga_main_window_update_logo_have_window (EkigaMainWindow *main_window);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the gnomemeeting logo in the video window.
 * PRE          :  /
 */
void gm_main_window_update_logo ();

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window hold call menu and toolbar items
 * 		   following the call is on hold (TRUE) or not (FALSE).
 * PRE          :  The main window GMObject.
 */
void ekiga_main_window_set_call_hold (EkigaMainWindow *main_window,
                                      bool is_on_hold);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window pause channel menu and toolbar items
 * 		   following the channel is paused (TRUE) or not (FALSE). The
 * 		   last argument is true if we are modifying a video channel
 * 		   item.
 * PRE          :  The main window GMObject.
 */
void ekiga_main_window_set_channel_pause (EkigaMainWindow *main_window,
					  gboolean pause,
					  gboolean is_video);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window sensitivity and state following 
 * 		   the given calling state.
 * 		   The state of widgets that depend on the calling state only
 * 		   is updated: 
 * 		   - the sensitivity of menu and toolbar items,
 * 		   - the stay on top state of windows depending of the main 
 * 		   window, 
 * 		   - the state of the calling button,
 * 		   - the transfer call window,
 * 		   - the incoming call window can be destroyed or not.
 * 		   Widgets for which the state depends on other parameters
 * 		   are udpated in separate functions.
 * PRE          :  The main window GMObject.
 * 		   A valid GMH323Endpoint calling state.
 */
void ekiga_main_window_update_calling_state (EkigaMainWindow *mw,
					     unsigned calling_state);



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window busy state. When the window is busy,
 *                 a busy cursor is displayed and you can not exit.
 * PRE          :  The main window GMObject.
 * 		   The first parameter is TRUE if we are busy.
 */
void ekiga_main_window_set_busy (EkigaMainWindow *main_window,
			      bool busy);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window audio sliders to the given values,
 * 		   notice it only updates the GUI.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the output and input volumes.
 * 		   Their values must be comprised between -1 (no change) and 
 * 		   255.
 */
void gm_main_window_set_volume_sliders_values (GtkWidget *main_window,
					       int output_volume,
					       int input_volume);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Gets the values of the main window audio sliders.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the output and input volumes.
 * 		   Their values will be comprised between 0 and 255 when 
 * 		   the function returns.
 */
void gm_main_window_get_volume_sliders_values (GtkWidget *main_window,
					       int &output_volume,
					       int &input_volume);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window video sliders to the given values,
 * 		   notice it only updates the GUI.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the whiteness, brightness, colourness and contrast.
 * 		   Their values must be comprised between -1 (no change) and 
 * 		   255.
 */
void gm_main_window_set_video_sliders_values (GtkWidget *main_window,
					      int whiteness,
					      int brightness,
					      int colour,
					      int contrast);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Gets the values for the main window video sliders and
 * 		   updates the parameters accordingly.
 * 		   Notice it only reads the values from the GUI, not from
 * 		   the video grabber.
 * PRE          :  A valid pointer to the main window GMObject, followed
 * 		   by the whiteness, brightness, colourness and contrast.
 * 		   Their values will be comprised between 0 and 255 when
 * 		   the function returns.
 */
void gm_main_window_get_video_sliders_values (GtkWidget *main_window,
					      int &whiteness,
					      int &brightness,
					      int &colour,
					      int &contrast);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Select the correct panel section in the menus
 * 		   and in the main window.
 * PRE          :  The main window GMObject and a valid section.
 */
void ekiga_main_window_set_panel_section (EkigaMainWindow *main_window,
                                          int section);



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the information displayed in the info label
 * 		   of the main window.
 * PRE          :  The main window GMObject, 
 * 		   the transmitted audio codec, 
 * 		   the received audio codec (if any), 
 * 		   the transmitted video codec,
 * 		   the received video codec(if any).
 */
void ekiga_main_window_set_call_info (EkigaMainWindow *main_window,
				      const char *tr_audio_codec,
				      const char *re_audio_codec,
				      const char *tr_video_codec,
				      const char *re_video_codec);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Sets the current call duration (as a string) in the GUI.
 * PRE           : The main window GMObject.
 */
void ekiga_main_window_set_call_duration (EkigaMainWindow *main_window,
                                          const char *duration);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Sets the current status in the GUI.
 * PRE           : The main window GMObject.
 */
void ekiga_main_window_set_status (EkigaMainWindow *main_window,
				   const char *status);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the speed dials menu in the call menu given the
 *                 main window GMObject and using the address book.
 * PRE          :  The main window GMObject and the GSList of GmContacts.
 */
void gm_main_window_speed_dials_menu_update (GtkWidget *main_window,
					     GSList *glist);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the urls in the cache of the url entry. It is done
 * 		   using the list of the last 100 given/received/missed calls,
 * 		   but also using the address book contacts.
 * PRE          :  The main window GMObject.
 */
void gm_main_window_urls_history_update (GtkWidget *main_window);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Runs a dialog to transfer a call. 
 * 		   Returns TRUE if the user chose to transfer.
 * PRE          :  The main window GMObject, the parent window, the URL
 * 		   to put in the dialog as default.
 */
gboolean ekiga_main_window_transfer_dialog_run (EkigaMainWindow *main_window,
					        GtkWidget *parent_window,
					        const char *u);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window and adds the popup to the image.
 * PRE          :  Accels.
 */
GtkWidget *gm_main_window_new (Ekiga::ServiceCore & core);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Flashes a message on the statusbar during a few seconds.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
void ekiga_main_window_flash_message (EkigaMainWindow *main_window,
				      const char *msg,
				      ...) G_GNUC_PRINTF(2,3);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Displays a message on the statusbar or clears it if msg = 0.
 *                 Removes the previous message.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
void ekiga_main_window_push_message (EkigaMainWindow *main_window,
				     const char *msg,
				     ...) G_GNUC_PRINTF(2,3);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Displays an info message on the statusbar. An info message
 * 		   is only cleared when the user clicks on it.
 * PRE           : The main window GMObject, followed by printf syntax format.
 */
void ekiga_main_window_push_info_message (EkigaMainWindow *main_window,
					  const char *msg,
					  ...) G_GNUC_PRINTF(2,3);

/* DESCRIPTION   :  /
 * BEHAVIOR      : Sets the given URL as called URL.
 * PRE           : The main window GMObject.
 */
void ekiga_main_window_set_call_url (EkigaMainWindow *mw,
				     const char *url);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Appends the given string to the current URL. Replaces the 
 * 		   current selection if any.
 * PRE           : The main window GMObject.
 */
void ekiga_main_window_append_call_url (EkigaMainWindow *mw,
					const char *url);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Clears the stats area in the control panel. 
 * PRE           : The main window GMObject.
 */
void ekiga_main_window_clear_stats (EkigaMainWindow *main_window);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Updates the stats area in the control panel. 
 * PRE           : The main window GMObject, lost, late packets, rtt, jitter,
 * 		   video bytes received, transmitted, audio bytes received,
 * 		   transmitted. All >= 0.
 */
void ekiga_main_window_update_stats (EkigaMainWindow *main_window,
				     float lost,
				     float late,
				     float out_of_order,
				     int jitter,
				     unsigned int re_width,
				     unsigned int re_height,
				     unsigned int tr_width,
				     unsigned int tr_height);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Returns the currently displayed picture as a pixbuf.
 * PRE           : The main window GMObject.
 */
GdkPixbuf *ekiga_main_window_get_current_picture (EkigaMainWindow *main_window);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window sensitivity following the opened
 *                 and closed audio and video channels. It also updates
 *                 the state of the video preview button.
 * PRE          :  The main window GMObject.
 * 		   The first parameter is TRUE if we are updating video
 *                 channels related items, FALSE if we are updating audio
 *                 channels related items. The second parameter is TRUE
 *                 if we are transmitting audio (or video), the third is TRUE
 *                 if we are receiving audio (or video).
 */
void ekiga_main_window_update_sensitivity (EkigaMainWindow *main_window,
					   bool is_video,
					   bool is_receiving,
					   bool is_transmitting);


/* DESCRIPTION   :  /
 * BEHAVIOR      : Updates the menu sensitivity
 * PRE           : The main window GMObject.
 */
void gm_main_window_fullscreen_menu_update_sensitivity (bool fullscreen);

#endif

