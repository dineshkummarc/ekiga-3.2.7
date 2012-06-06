
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
 *                         gnome_prefs_window.c  -  description 
 *                         ------------------------------------
 *   begin                : Mon Oct 15 2003, but based on older code
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Helpers to create GNOME compliant prefs windows.
 *
 */

#include "config.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS


/* Common notice 
 *
 * The created widgets are associated to a config key. They have the value
 * of the config key as initial value and they get updated when the config
 * value changes.
 *
 * You have to create a prefs window with gnome_prefs_window_new. You
 * can create categories of options with gnome_prefs_window_section_new
 * and subcategories with gnome_prefs_window_subsection_new. You can fill in
 * those subcategories by blocks of options using gnome_prefs_subsection_new
 * and then add entries, toggles and such to those blocks using
 * the functions below.
 */


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkEntry associated with a config key and returns
 *                 the result.
 *                 The first parameter is the section in which 
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the config key, the tooltip, the row where
 *                 to attach it in the section, and if the label and GtkEntry
 *                 should be packed together or aligned with others in the
 *                 section they belong to.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_entry_new (GtkWidget *,
				  const gchar *,
				  const gchar *,
				  const gchar *,
				  int,
				  gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkToggleButton associated with a config key and
 *                 returns the result.
 *                 The first parameter is the section in which the 
 *                 GtkToggleButton should be attached. The other parameters are
 *                 the text label, the config key, the tooltip, the row where
 *                 to attach it in the section.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_toggle_new (GtkWidget *,
				   const gchar *,
				   const gchar *, 
				   const gchar *,
				   int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkHScale associated with a config key and
 *                 returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkHScale should be attached. The other parameters
 *                 are the text labels, the config key, the tooltip, the
 *                 minimal and maximal values, the incrementation step,
 *                 the row where to attach it in the section.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_scale_new (GtkWidget *,       
				  const gchar *,
				  const gchar *,
				  const gchar *,       
				  const gchar *,
				  double,
				  double,
				  double,
				  int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkSpinButton associated with a config key and
 *                 returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkSpinButton should be attached. The other parameters
 *                 are the text label, the config key, the tooltip, the
 *                 minimal and maximal values, the incrementation step,
 *                 the row where to attach it in the section, 
 *                 the rest of the label, if any, and if the label and widget
 *                 should be packed together or aligned with others in the
 *                 section they belong to. 
 * PRE          :  The gboolean must be TRUE if the rest of the label is given.
 */
GtkWidget *gnome_prefs_spin_new (GtkWidget *,
				 const gchar *,
				 const gchar *,
				 const gchar *,
				 double,
				 double,
				 double,
				 int,
				 const gchar *,
				 gboolean);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a range with 2 GtkSpinButtons associated with two
 *                 config keys.
 *                 The first parameter is the section in which 
 *                 the GtkSpinButton should be attached. The other parameters
 *                 are the first part of the label, a pointer that will be
 *                 updated to point to the first GtkSpinButton, the second
 *                 part of the text label, a pointer that will be updated
 *                 to point to the second GtkSpinButton, the third part
 *                 of the text label, the 2 config keys, the 2 tooltips, the
 *                 2 minimal, the 2 maximal values, the incrementation step,
 *                 the row where to attach it in the section.
 * PRE          :  /
 */
void gnome_prefs_range_new (GtkWidget *,
			    const gchar *,
			    GtkWidget **,
			    const gchar *,
			    GtkWidget **,
			    const gchar *,
			    const gchar *,
			    const gchar *,
			    const gchar *,
			    const gchar *,
			    double,
			    double,
			    double,
			    double,
			    double,
			    int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkOptionMenu associated with an integer config
 *                 key and returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the possible values for the menu, the config
 *                 key, the tooltip, the row where to attach it in the section.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_int_option_menu_new (GtkWidget *,
					    const gchar *,
					    const gchar **, 
					    const gchar *,
					    const gchar *,
					    int);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a GtkOptionMenu associated with a string config
 *                 key and returns the result.
 *                 The first parameter is the section in which 
 *                 the GtkEntry should be attached. The other parameters are
 *                 the text label, the possible values for the menu, the config
 *                 key, the tooltip, the row where to attach it in the section and
 *                 the default value if the conf key is associated to a NULL value.
 * PRE          :  The array ends with NULL. 
 */
GtkWidget *gnome_prefs_string_option_menu_new (GtkWidget *,
					       const gchar *,
					       const gchar **,
					       const gchar *,
					       const gchar *,
					       int,
                                               const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the content of a GtkOptionMenu associated with
 *                 a string config key. The first parameter is the menu,
 *                 the second is the array of possible values, and the
 *                 last one is the config key and the default value if the
 *                 conf key is associated to a NULL value. 
 * PRE          :  The array ends with NULL.
 */
void gnome_prefs_string_option_menu_update (GtkWidget *,
					    const gchar **,
					    const gchar *,
                                            const gchar *);



void gnome_prefs_string_option_menu_add (GtkWidget *option_menu,
	    			         const gchar *option,
	    			         gboolean active);

void gnome_prefs_string_option_menu_remove (GtkWidget *option_menu,
	 			            const gchar *option);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a subsection inside a section of a prefs window.
 *                 The parameters are the prefs window, the section of the
 *                 prefs window in which the newly created subsection must
 *                 be added, the title of the frame, the number of rows
 *                 and of columns. Widgets can be attached to the returned
 *                 subsection.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_subsection_new (GtkWidget *,
				       GtkWidget *,
				       const gchar *,
				       int,
				       int);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new prefs window. The parameter is a filename
 *                 corresponding to the logo displayed by default. Returns
 *                 the created window which still has to be connected to the
 *                 signals.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_window_new (const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new section in the given prefs window.
 *                 The parameter are the prefs window and the prefs
 *                 window section name.
 * PRE          :  /
 */
void gnome_prefs_window_section_new (GtkWidget *,
				     const gchar *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new subsection in the given prefs window and
 *                 returns it. The parameter are the prefs window and the
 *                 prefs window subsection name. General subsections can
 *                 be created in the returned gnome prefs window subsection
 *                 and widgets can be attached to them.
 * PRE          :  /
 */
GtkWidget *gnome_prefs_window_subsection_new (GtkWidget *,
					      const gchar *);

G_END_DECLS
