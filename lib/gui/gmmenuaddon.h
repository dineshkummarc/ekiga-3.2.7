
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
 *                         gtk_menu_extensions.h  -  description 
 *                         -------------------------------------
 *   begin                : Mon Sep 29 2003, but based on older code
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Helpers to create the menus.
 *
 */


#ifndef _GTK_MENU_EXTENSIONS_H_
#define _GTK_MENU_EXTENSIONS_H_

#include <gtk/gtk.h>

#include "gmconf.h"


G_BEGIN_DECLS

/* Possible values for the Menu entries */
typedef enum {

  MENU_ENTRY,
  MENU_TOGGLE_ENTRY,
  MENU_RADIO_ENTRY,
  MENU_SEP,
  MENU_TEAROFF,
  MENU_NEW,
  MENU_SUBMENU_NEW,
  MENU_END
} MenuEntryType;


/* The structure defining a Menu Entry */
typedef struct _MenuEntry {

  const char *id;
  const char *name;
  const char *tooltip;
  const char *stock_id;
  gboolean stock_is_theme;
  guint accel;
  MenuEntryType type;
  GCallback func;
  GClosureNotify clofunc;
  gpointer data;
  GtkWidget *widget;
  gboolean enabled;
  gboolean sensitive;
} MenuEntry;


#define GTK_MENU_SEPARATOR \
{NULL, NULL, NULL, NULL, FALSE, 0, MENU_SEP, NULL, NULL, NULL, NULL, TRUE, TRUE}

#define GTK_MENU_ENTRY(a, b, c, d, e, f, g, h) \
{a, b, c, d, FALSE, e, MENU_ENTRY, f, NULL, g, NULL, TRUE, h}

#define GTK_MENU_THEME_ENTRY(a, b, c, d, e, f, g, h) \
{a, b, c, d, TRUE, e, MENU_ENTRY, f, NULL, g, NULL, TRUE, h}

#define GTK_MENU_ENTRY_WITH_CLOSURE(a, b, c, d, e, f, g, h, i) \
{a, b, c, d, FALSE, e, MENU_ENTRY, f, g, h, NULL, TRUE, i}

#define GTK_MENU_RADIO_ENTRY(a, b, c, d, e, f, g, h, i) \
{a, b, c, d, FALSE, e, MENU_RADIO_ENTRY, f, NULL, g, NULL, h, i}

#define GTK_MENU_TOGGLE_ENTRY(a, b, c, d, e, f, g, h, i) \
{a, b, c, d, FALSE, e, MENU_TOGGLE_ENTRY, f, NULL, g, NULL, h, i}

#define GTK_MENU_NEW(a) \
{NULL, a, NULL, NULL, FALSE, 0, MENU_NEW, NULL, NULL, NULL, NULL, TRUE, TRUE}

#define GTK_SUBMENU_NEW(a, b) \
{a, b, NULL, NULL, FALSE, 0, MENU_SUBMENU_NEW, NULL, NULL, NULL, NULL, TRUE, TRUE}

#define GTK_MENU_END \
{NULL, NULL, NULL, NULL, FALSE, 0, MENU_END, NULL, NULL, NULL, NULL, TRUE, TRUE}


/* DESCRIPTION  :  This callback is called when the user 
 *                 selects a different option in a radio menu.
 * BEHAVIOR     :  Sets the config key.
 * PRE          :  data is the config key.
 */
void radio_menu_changed_cb (GtkWidget *,
			    gpointer);


/* DESCRIPTION  :  This callback is called when the user toggles an
 * BEHAVIOR     :  Updates the config key given as parameter.
 * PRE          :  data is the key.
 */
void toggle_menu_changed_cb (GtkWidget *, 
			     gpointer);


/* DESCRIPTION  :  Generic notifiers for toggles in the menu.
 *                 This callback is called when a specific key of
 *                 the config database associated with a toggle changes, this
 *                 only updates the toggle in the menu.
 * BEHAVIOR     :  It only updates the widget.
 * PRE          :  The toggle menu item.
 */
void menu_toggle_changed_nt (gpointer, 
			     GmConfEntry *, 
			     gpointer);


/* DESCRIPTION  :  Notifiers for radios menu.
 *                 This callback is called when a specific key of
 *                 the config database associated with a radio menu changes,
 *                 this only updates the radio in the menu.
 * BEHAVIOR     :  It updates the widget.
 * PRE          :  One of the GtkCheckMenuItem of the radio menu.
 */
void radio_menu_changed_nt (gpointer,
			    GmConfEntry *,
			    gpointer);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Change the sensitivity of a whole Menu section given the
 *                 Menu widget, the identifier of one of the first Menu item
 *                 in the section, and a boolean indicating the sensitivity.
 *                 A section is separated by 2 MENU_SEPARATOR elements.
 * PRE          :  /
 */
void gtk_menu_section_set_sensitive (GtkWidget *,
				     const char *,
				     gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Change the sensitivity of a Menu item given the Menu widget,
 *                 the text ID of the Menu item, and a boolean indicating if
 *                 the given Menu item should be sensitive or not.
 * PRE          :  /
 */
void gtk_menu_set_sensitive (GtkWidget *,
			     const char *,
			     gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Returns the GtkMenuItem of a menu given the Menu widget
 *                 and teh text ID of the Menu item.
 * PRE          :  /
 */
GtkWidget *gtk_menu_get_widget (GtkWidget *,
				const char *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds a menu given a first menu, a structure, an
 *                 AccelGroup and a possible GtkStatusBar where to push
 *                 the tooltips. 
 * PRE          :  /
 */
void gtk_build_menu (GtkWidget *,
		     MenuEntry *,
		     GtkAccelGroup *,
		     GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds a menu given a structure and an
 *                 AccelGroup and attach it to the give widget.
 * PRE          :  Returns the popup menu widget.
 */
GtkWidget *gtk_build_popup_menu (GtkWidget *,
				 MenuEntry *,
				 GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the menu toggle with the new value. 
 * PRE          :  /
 */
void gtk_toggle_menu_enable (GtkWidget *, 
			     gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Selects the i th element in the radio menu identified
 *                 by its ID without triggering the callbacks.
 * PRE          :  /
 */
void gtk_radio_menu_select_with_id (GtkWidget *,
				    const gchar *,
				    int i);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Selects the i th element in the radio menu identified
 *                 by its first item without triggering the callbacks.
 * PRE          :  /
 */
void gtk_radio_menu_select_with_widget (GtkWidget *,
					int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Show or hide the images in the menu following the gboolean
 *                 is true or false.
 * PRE          :  /
 */
void gtk_menu_show_icons (GtkWidget *,
			  gboolean);

G_END_DECLS

#endif
