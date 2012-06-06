
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
 *                         gm_conf.h  -  description 
 *                         ------------------------------------------
 *   begin                : Fri Oct 17 2003, but based on older code
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : helper notifiers for gnomemeeting's widgets
 *
 */

/* Common notice
 *
 * This file provides a few generic signal handlers and notifiers for
 * GTK widgets associated with config keys. If you associate the good callback
 * and the good notifier to a widget, the notifier will update the widget
 * after having blocked the signal when the config key is modified and
 * the config key will be updated when the widget changes.
 *
 * Default notifiers and default callbacks are given for GtkEntry,
 * GtkAdjustment, GtkToggleButton, GtkOptionMenu associated with a string
 * config key and with an int config key.
 */

#include <gtk/gtk.h>
#include "gmconf.h"

#ifndef __GM_CONF_WIDGET_EXTENSIONS_H
#define __GM_CONF_WIDGET_EXTENSIONS_H
 
G_BEGIN_DECLS


/* DESCRIPTION  :  This function is called when an entry is activated.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the string config key
 *                 to modify.
 */
void entry_activate_changed (GtkWidget *,
                             gpointer);

/* DESCRIPTION  :  This function is called when the focus of an entry changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 entry.  
 * PRE          :  Non-Null data corresponding to the string config key
 *                 to modify.
 */
gboolean entry_focus_changed (GtkWidget *,
                              GdkEventFocus *,
                              gpointer);


/* DESCRIPTION  :  Generic notifiers for entries.
 *                 This callback is called when a specific key of
 *                 the config database associated with an entry changes.
 * BEHAVIOR     :  It updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *                 should be of type string.
 */
void entry_changed_nt (gpointer,
		       GmConfEntry *,
		       gpointer);


/* DESCRIPTION  :  This function is called when a toggle changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 toggle.  
 * PRE          :  Non-Null data corresponding to the boolean config key to
 *                 modify.
 */
void toggle_changed (GtkCheckButton *,
		     gpointer);


/* DESCRIPTION  :  Generic notifiers for toggles.
 *                 This callback is called when a specific key of
 *                 the config database associated with a toggle changes, this
 *                 only updates the toggle.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *"                should be of type boolean.
 */
void toggle_changed_nt (gpointer,
			GmConfEntry *,
			gpointer);


/* DESCRIPTION  :  This function is called when an adjustment changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 adjustment.  
 * PRE          :  Non-Null data corresponding to the int config key to modify.
 */
void adjustment_changed (GtkAdjustment *,
			 gpointer);


/* DESCRIPTION  :  Generic notifiers for adjustments.
 *                 This callback is called when a specific key of
 *                 the config database associated with an adjustment changes.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *                 should be of type integer.
 */
void adjustment_changed_nt (gpointer,
			    GmConfEntry *,
			    gpointer);


/* DESCRIPTION  :  This function is called when an int option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 int option menu.  
 * PRE          :  Non-Null data corresponding to int the config key to modify.
 */

void int_option_menu_changed (GtkWidget *,
			      gpointer);


/* DESCRIPTION  :  Generic notifiers for int-based option menus.
 *                 This callback is called when a specific key of
 *                 the config database associated with an option menu changes,
 *                 it only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modifiction
 *                 should be of type integer.
 */
void int_option_menu_changed_nt (gpointer,
				 GmConfEntry *,
				 gpointer);


/* DESCRIPTION  :  This function is called when a string option menu changes.
 * BEHAVIOR     :  Updates the key given as parameter to the new value of the
 *                 string option menu.  
 * PRE          :  Non-Null data corresponding to the string config key to
 *                 modify.
 */
void string_option_menu_changed (GtkWidget *,
				 gpointer);


/* DESCRIPTION  :  Generic notifiers for string-based option_menus.
 *                 This callback is called when a specific key of
 *                 the config database associated with an option menu changes,
 *                 this only updates the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The config key triggering that notifier on modification
 *                 should be of type string.
 */
void string_option_menu_changed_nt (gpointer,
				    GmConfEntry *,
				    gpointer);

G_END_DECLS
 
#endif // __GM_CONF_WIDGET_EXTENSIONS_H
