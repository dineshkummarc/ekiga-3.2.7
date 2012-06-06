
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


#include "gmpreferences.h"

#include "gmconfwidgets.h"

#include <string.h>
#include <libintl.h>

enum {
  COLUMN_STRING_RAW = 0, /* must be zero because it's used in gmconfwidgets */
  COLUMN_STRING_TRANSLATED,
  COLUMN_SENSITIVE,
};

static void tree_selection_changed_cb (GtkTreeSelection *,
				       gpointer);


typedef struct _GnomePrefsWindow {

  GtkWidget *notebook;
  GtkWidget *section_label;
  GtkWidget *sections_tree_view;
  GtkTreeIter iter;
  int last_page;
  
} GnomePrefsWindow;


/* GTK Callbacks */

/* DESCRIPTION  :  This callback is called when the user clicks in the
 *                 categories and subcategories GtkTreeView.
 * BEHAVIOR     :  Display the logo if he clicked in a category, or the
 *                 different blocks of options corresponding to a subcategory
 *                 if he clicked in a specific subcategory.
 * PRE          :  /
 */
static void
tree_selection_changed_cb (GtkTreeSelection *selection,
			   gpointer data)
{
  int page = 0;
  gchar *name = NULL;
  GtkTreeIter iter;
  GtkTreeModel *model = NULL;
  GnomePrefsWindow *gpw = NULL;

  if (!data)
    return;
  
  if (gtk_tree_selection_get_selected (selection, &model, &iter)) {

    gpw = (GnomePrefsWindow *) data;
    
    gtk_tree_model_get (GTK_TREE_MODEL (model),
			&iter, 1, &page, -1);

    gtk_tree_model_get (GTK_TREE_MODEL (model),
			&iter, 0, &name, -1);

    gtk_label_set_text (GTK_LABEL (gpw->section_label), name);

    gtk_notebook_set_current_page (GTK_NOTEBOOK (gpw->notebook), page);
  }
}


/* The public functions */
GtkWidget *
gnome_prefs_entry_new (GtkWidget *table,
		       const gchar *label_txt,
		       const gchar *conf_key,
		       const gchar *tooltip,
		       int row,
		       gboolean box)
{
  GnomePrefsWindow *gpw = NULL;
  GValue value = { 0, {{0}, {0}}};
  int cols = 0;
  GtkWidget *entry = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hbox = NULL;
  
  gchar *conf_string = NULL;
  gboolean writable = FALSE;
  
  writable = gm_conf_is_key_writable (conf_key);
  
  if (box) {
    
    hbox = gtk_hbox_new (FALSE, 0);
    g_value_init (&value, G_TYPE_INT);
    g_object_get_property (G_OBJECT (table), "n-columns", &value);
    cols = g_value_get_int (&value);
    g_value_unset (&value);
  }
  
  label = gtk_label_new_with_mnemonic (label_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  
  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), label,
			FALSE, FALSE, 1 * 2);
  else
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  entry = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL(label), entry);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (entry), FALSE);
  
  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), entry,
			FALSE, FALSE, 1 * 2);
  else
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, row, row+1,
		      (GtkAttachOptions) (NULL),
		      (GtkAttachOptions) (NULL),
		      0, 0);
  
  conf_string =
    gm_conf_get_string (conf_key);

  if (conf_string != NULL)
    gtk_entry_set_text (GTK_ENTRY (entry), conf_string);

  g_free (conf_string);

  g_signal_connect_after (G_OBJECT (entry), "focus-out-event",
			  G_CALLBACK (entry_focus_changed),
			  (gpointer)conf_key);

  g_signal_connect_after (G_OBJECT (entry), "activate",
			  G_CALLBACK (entry_activate_changed),
			  (gpointer)conf_key);

  gm_conf_notifier_add (conf_key, entry_changed_nt, (gpointer) entry);

  if (box)
    gtk_table_attach (GTK_TABLE (table), hbox, 0, cols, row, row+1,
		      (GtkAttachOptions) (NULL),
		      (GtkAttachOptions) (NULL),
		      0, 0);

  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && tooltip)
    gtk_widget_set_tooltip_text (entry, tooltip);

  gtk_widget_show_all (table);
  
  return entry;
}                                                                              
                                                                               
                                                                               
GtkWidget *
gnome_prefs_toggle_new (GtkWidget *table,
			const gchar *label_txt,
			const gchar *conf_key,
			const gchar *tooltip,
			int row)
{
  GnomePrefsWindow *gpw = NULL;
  GValue value = { 0, {{0}, {0}}};
  GtkWidget *toggle = NULL;
  gboolean writable = FALSE;
  int cols = 0;
  
  writable = gm_conf_is_key_writable (conf_key);
  
  g_value_init (&value, G_TYPE_INT);
  g_object_get_property (G_OBJECT (table), "n-columns", &value);
  cols = g_value_get_int (&value);
  g_value_unset (&value);
  
  toggle = gtk_check_button_new_with_mnemonic (label_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (toggle), FALSE);
  
  gtk_table_attach (GTK_TABLE (table), toggle, 0, cols, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);
                                                                               
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (toggle),
				gm_conf_get_bool (conf_key));

  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && tooltip)
    gtk_widget_set_tooltip_text (toggle, tooltip);

  g_signal_connect (G_OBJECT (toggle), "toggled",
		    G_CALLBACK (toggle_changed), (gpointer) conf_key);
  
  gm_conf_notifier_add (conf_key, toggle_changed_nt, (gpointer) toggle);

  gtk_widget_show_all (table);
  
  return toggle;
}                                                                              


GtkWidget *
gnome_prefs_scale_new (GtkWidget *table,       
		       const gchar *down_label_txt,
		       const gchar *up_label_txt,
		       const gchar *conf_key,       
		       const gchar *tooltip,
		       double min,
		       double max,
		       double step,
		       int row)
{
  GnomePrefsWindow *gpw = NULL;
  GtkWidget *hbox = NULL;
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *hscale = NULL;

  gboolean writable = FALSE;

  writable = gm_conf_is_key_writable (conf_key);

  hbox = gtk_hbox_new (FALSE, 0);

  label = gtk_label_new_with_mnemonic (down_label_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      1 * 2);
  
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  adj = (GtkAdjustment *) 
    gtk_adjustment_new (gm_conf_get_int (conf_key),
			min, max, step,
			2.0, 1.0);

  hscale = gtk_hscale_new (adj);
  gtk_scale_set_draw_value (GTK_SCALE (hscale), FALSE);
  gtk_widget_set_size_request (GTK_WIDGET (hscale), 150, -1);
  gtk_range_set_update_policy (GTK_RANGE (hscale),
			       GTK_UPDATE_DELAYED);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (hscale), FALSE);

  gtk_box_pack_start (GTK_BOX (hbox), hscale, FALSE, FALSE,
		      1 * 2);


  label = gtk_label_new_with_mnemonic (up_label_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);

  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      1 * 2);

  gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1,
		    (GtkAttachOptions) (GTK_FILL),
		    (GtkAttachOptions) (GTK_FILL),
		    0, 0);

  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && tooltip)
    gtk_widget_set_tooltip_text (hscale, tooltip);

  g_signal_connect (G_OBJECT (adj), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) conf_key);

  gm_conf_notifier_add (conf_key, adjustment_changed_nt,
			(gpointer) adj);

  gtk_widget_show_all (table);

  return hscale;
}


GtkWidget *
gnome_prefs_spin_new (GtkWidget *table,       
		      const gchar *label_txt,
		      const gchar *conf_key,       
		      const gchar *tooltip,
		      double min,
		      double max,
		      double step,
		      int row,
		      const gchar *label_txt2,
		      gboolean box)
{
  GnomePrefsWindow *gpw = NULL;
  GtkWidget *hbox = NULL;
  GtkAdjustment *adj = NULL;
  GtkWidget *label = NULL;
  GtkWidget *spin_button = NULL;

  gboolean writable = FALSE;
  
  writable = gm_conf_is_key_writable (conf_key);

  
  if (box)
    hbox = gtk_hbox_new (FALSE, 0);
  
  label = gtk_label_new_with_mnemonic (label_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  
  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
			1 * 2);
  else
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  
  adj = (GtkAdjustment *) 
    gtk_adjustment_new (gm_conf_get_int (conf_key),
			min, max, step,
			10.0, 0.0);
  
  spin_button = gtk_spin_button_new (adj, 1.0, 0);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (spin_button), FALSE);
  
  if (box)
    gtk_box_pack_start (GTK_BOX (hbox), spin_button, FALSE, FALSE,
			1 * 2);
  else
    gtk_table_attach (GTK_TABLE (table), spin_button, 1, 2, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);

  if (box && label_txt2) {
    
    label = gtk_label_new_with_mnemonic (label_txt2);
    if (!writable)
      gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
    
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
			1 * 2);
  }

  if (box) {
   
    gtk_table_attach (GTK_TABLE (table), hbox, 
		      0, GTK_TABLE (table)->ncols, row, row+1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL),
		      0, 0);
  }

  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && tooltip)
    gtk_widget_set_tooltip_text (spin_button, tooltip);

  g_signal_connect (G_OBJECT (adj), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) conf_key);

  gm_conf_notifier_add (conf_key, adjustment_changed_nt,
			(gpointer) adj);

  gtk_widget_show_all (table);
  
  return spin_button;
}


void
gnome_prefs_range_new (GtkWidget *table,
		       const gchar *label1_txt,
		       GtkWidget **spin1,
		       const gchar *label2_txt,
		       GtkWidget **spin2,
		       const gchar *label3_txt,
		       const gchar *spin1_conf_key,
		       const gchar *spin2_conf_key,
		       const gchar *spin1_tooltip,
		       const gchar *spin2_tooltip,
		       double spin1_min,
		       double spin2_min,
		       double spin1_max,
		       double spin2_max,
		       double spins_step,
		       int row)
{
  GnomePrefsWindow *gpw = NULL;
  int val1 = 0, val2 = 0;
  gboolean writable = FALSE;
  GtkWidget *hbox = NULL;
  GtkAdjustment *adj1 = NULL;
  GtkWidget *spin_button1 = NULL;
  GtkWidget *spin_button2 = NULL;
  GtkAdjustment *adj2 = NULL;
  GtkWidget *label = NULL;

  writable =
    (gm_conf_is_key_writable (spin1_conf_key)
     &&
     gm_conf_is_key_writable (spin2_conf_key));
  
  hbox = gtk_hbox_new (FALSE, 0);
  label = gtk_label_new_with_mnemonic (label1_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      1 * 2);
  
  val1 = gm_conf_get_int (spin1_conf_key);
  adj1 = (GtkAdjustment *) 
    gtk_adjustment_new (val1, spin1_min, spin1_max, spins_step, 2.0, 1.0);
  spin_button1 = gtk_spin_button_new (adj1, 1.0, 0);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (spin_button1), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), spin_button1, FALSE, FALSE,
		      1 * 2);

  label = gtk_label_new_with_mnemonic (label2_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      1 * 2);

  val2 = gm_conf_get_int (spin2_conf_key);
  adj2 = (GtkAdjustment *) 
    gtk_adjustment_new (val2, spin2_min, spin2_max, spins_step, 2.0, 1.0);
  spin_button2 = gtk_spin_button_new (adj2, 1.0, 0);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (spin_button2), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), spin_button2, FALSE, FALSE,
		      1 * 2);
  
  label = gtk_label_new_with_mnemonic (label3_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE,
		      1 * 2);

  gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, row, row+1,
                    (GtkAttachOptions) (NULL),
		    (GtkAttachOptions) (NULL),
		    0, 0);

  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && spin1_tooltip && spin2_tooltip) {
    gtk_widget_set_tooltip_text (spin_button1, spin1_tooltip);
    gtk_widget_set_tooltip_text (spin_button2, spin2_tooltip);
  }
  

  g_signal_connect (G_OBJECT (adj1), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) spin1_conf_key);
  gm_conf_notifier_add (spin1_conf_key, adjustment_changed_nt,
			(gpointer) adj1);
  
  g_signal_connect (G_OBJECT (adj2), "value-changed",
		    G_CALLBACK (adjustment_changed),
		    (gpointer) spin2_conf_key);
  gm_conf_notifier_add (spin2_conf_key, adjustment_changed_nt,
			(gpointer) adj2);

  if (spin1)
    *spin1 = spin_button1;

  if (spin2)
    *spin2 = spin_button2;
}                                                                              


GtkWidget *
gnome_prefs_int_option_menu_new (GtkWidget *table,
				 const gchar *label_txt,
				 const gchar **options,
				 const gchar *conf_key,
				 const gchar *tooltip,
				 int row)
{
  GnomePrefsWindow *gpw = NULL;
  
  GtkWidget *label = NULL;                                                     
  GtkWidget *option_menu = NULL;
  
  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeIter iter;
  
  gboolean writable = FALSE;

  int history = -1;
  int cpt = 0;

  writable = gm_conf_is_key_writable (conf_key);

  label = gtk_label_new_with_mnemonic (label_txt);
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  list_store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
  option_menu = gtk_combo_box_new_with_model (GTK_TREE_MODEL (list_store));
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (option_menu), FALSE);
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (option_menu), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (option_menu), renderer,
                                  "text", COLUMN_STRING_TRANSLATED,
                                  NULL);
  g_object_set (G_OBJECT (renderer), 
                "ellipsize-set", TRUE, 
                "ellipsize", PANGO_ELLIPSIZE_END, 
                "width-chars", 45, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), option_menu);

  history = gm_conf_get_int (conf_key);
  while (options [cpt]) {

    gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                        COLUMN_STRING_RAW, options [cpt],
                        COLUMN_STRING_TRANSLATED, gettext (options [cpt]),
                        -1);
    cpt++;
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (option_menu), history);
  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),                
                    (GtkAttachOptions) (GTK_FILL),                
                    0, 0);           
                                                                               
  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && tooltip)
    gtk_widget_set_tooltip_text (option_menu, tooltip);

  g_signal_connect (GTK_COMBO_BOX (option_menu), 
		    "changed", G_CALLBACK (int_option_menu_changed),
  		    (gpointer) conf_key);                                   
  gm_conf_notifier_add (conf_key, int_option_menu_changed_nt,
			(gpointer) option_menu);
  
  gtk_widget_show_all (table);
  
  return option_menu;
}


GtkWidget *
gnome_prefs_string_option_menu_new (GtkWidget *table,       
				    const gchar *label_txt, 
				    const gchar **options,
				    const gchar *conf_key,       
				    const gchar *tooltip,         
				    int row,
                                    const gchar *default_value)
{
  GnomePrefsWindow *gpw = NULL;
  
  GtkWidget *label = NULL;                                                     
  GtkWidget *option_menu = NULL;
  
  GtkListStore *list_store = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeIter iter;
  
  gchar *conf_string = NULL;
  gboolean writable = FALSE;

  int history = -1;
  int cpt = 0;

  writable = gm_conf_is_key_writable (conf_key);

  label = gtk_label_new (label_txt);                                           
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (label), FALSE);
  
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, row, row+1,                
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL),
                    0, 0);
                                                                               
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);                         
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

  list_store = gtk_list_store_new (3, 
                                   G_TYPE_STRING, 
                                   G_TYPE_STRING, 
                                   G_TYPE_BOOLEAN);
  option_menu = gtk_combo_box_new_with_model (GTK_TREE_MODEL (list_store));
  if (!writable)
    gtk_widget_set_sensitive (GTK_WIDGET (option_menu), FALSE);
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (option_menu), renderer, FALSE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (option_menu), renderer,
                                  "text", COLUMN_STRING_TRANSLATED,
                                  "sensitive", COLUMN_SENSITIVE,
                                  NULL);
  g_object_set (G_OBJECT (renderer), 
                "ellipsize-set", TRUE, 
                "ellipsize", PANGO_ELLIPSIZE_END, 
                "width-chars", 45, NULL);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), option_menu);

  conf_string = gm_conf_get_string (conf_key);
  if (conf_string == NULL)
    conf_string = g_strdup (default_value);
  while (options [cpt]) {

    if (conf_string && !strcmp (conf_string, options [cpt]))
      history = cpt;

    gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                        COLUMN_STRING_RAW, options [cpt],
                        COLUMN_STRING_TRANSLATED, gettext (options [cpt]),
                        COLUMN_SENSITIVE, TRUE,
                        -1);
    cpt++;
  }

  if (history == -1) {

    if (conf_string && strcmp (conf_string, "")) {

      gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                          COLUMN_STRING_RAW, conf_string,
                          COLUMN_STRING_TRANSLATED, gettext (conf_string),
                          COLUMN_SENSITIVE, FALSE, 
                          -1);
      history = cpt;
    }
    else
      history = --cpt;
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (option_menu), history);
  gtk_table_attach (GTK_TABLE (table), option_menu, 1, 2, row, row+1,
                    (GtkAttachOptions) (GTK_FILL),                
                    (GtkAttachOptions) (GTK_FILL),                
                    0, 0);           
                                                                               
  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (table), "gpw");
  if (gpw && tooltip)
    gtk_widget_set_tooltip_text (option_menu, tooltip);

  g_signal_connect (GTK_COMBO_BOX (option_menu), 
		    "changed", G_CALLBACK (string_option_menu_changed),
  		    (gpointer) conf_key);                                   
  gm_conf_notifier_add (conf_key, string_option_menu_changed_nt,
			(gpointer) option_menu);
  
  g_free (conf_string); 

  gtk_widget_show_all (table);
  
  return option_menu;
}


void
gnome_prefs_string_option_menu_update (GtkWidget *option_menu,
				       const gchar **options,
				       const gchar *conf_key,
                                       const gchar *default_value)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;

  gchar *conf_string = NULL;
  
  int history = -1;
  int cpt = 0;

  if (!options || !conf_key)
    return;
  
  conf_string = gm_conf_get_string (conf_key);
  if (conf_string == NULL)
    conf_string = g_strdup (default_value);

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (option_menu));

  gtk_list_store_clear (GTK_LIST_STORE (model));

  cpt = 0;
  while (options [cpt]) {

    if (conf_string && !strcmp (options [cpt], conf_string)) 
      history = cpt;

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                        COLUMN_STRING_RAW, options [cpt],
                        COLUMN_STRING_TRANSLATED, options [cpt],
                        COLUMN_SENSITIVE, TRUE, 
                        -1);
    cpt++;
  }

  if (history == -1) {
    
    if (conf_string && strcmp (conf_string, "")) {

      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                          COLUMN_STRING_RAW, conf_string,
                          COLUMN_STRING_TRANSLATED, gettext (conf_string),
                          COLUMN_SENSITIVE, FALSE, 
                          -1);
      history = cpt;
    }
    else
      history = --cpt;
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (option_menu), history);
  
  g_free (conf_string); 
}


void
gnome_prefs_string_option_menu_add (GtkWidget *option_menu,
				    const gchar *option,
				    gboolean active)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  gboolean found = FALSE;

  if (!option)
    return;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (option_menu));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {
      gchar *value_string = NULL;
      GValue value = { 0, {{0}, {0}} };
      gtk_tree_model_get_value (GTK_TREE_MODEL (model), &iter, 0, &value);
      value_string = (gchar *) g_value_get_string (&value);
      if (g_ascii_strcasecmp  (value_string, option) == 0) {
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                            COLUMN_SENSITIVE, TRUE,
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
                        COLUMN_STRING_RAW, option,
                        COLUMN_STRING_TRANSLATED, option, 
                        COLUMN_SENSITIVE, TRUE,
                        -1);
  }
  if (active == TRUE)
    gtk_combo_box_set_active_iter (GTK_COMBO_BOX (option_menu), &iter);
}




void
gnome_prefs_string_option_menu_remove (GtkWidget *option_menu,
				       const gchar *option)
{
  GtkTreeModel *model = NULL;
  GtkTreeIter iter;
  int cpt = 0;
  int active = 0;

  if (!option)
    return;

  model = gtk_combo_box_get_model (GTK_COMBO_BOX (option_menu));
  active = gtk_combo_box_get_active (GTK_COMBO_BOX (option_menu));

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {
      gchar *value_string = NULL;
      GValue value = { 0, {{0}, {0}} };
      gtk_tree_model_get_value (GTK_TREE_MODEL (model), &iter, 0, &value);
      value_string = (gchar *) g_value_get_string (&value);
      if (g_ascii_strcasecmp  (value_string, option) == 0) {

        if (cpt == active) {
          gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
                              COLUMN_SENSITIVE, FALSE,
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

GtkWidget *
gnome_prefs_subsection_new (GtkWidget *window,
			    GtkWidget *container,
			    const gchar *frame_name,       
			    int rows,
			    int cols)      
{
  GnomePrefsWindow *gpw  = NULL;
  
  GtkWidget *hbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *table = NULL;
  GtkWidget *label = NULL;
  
  PangoAttrList *attrs = NULL;
  PangoAttribute *attr = NULL;

  if (window)
    gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (window), "gpw");
  
  hbox = gtk_hbox_new (FALSE, 6);

  frame = gtk_frame_new (frame_name);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  
  attrs = pango_attr_list_new ();
  attr = pango_attr_weight_new (PANGO_WEIGHT_BOLD);
  attr->start_index = 0;
  attr->end_index = -1;
  pango_attr_list_insert (attrs, attr);

  label = gtk_frame_get_label_widget (GTK_FRAME (frame));
  gtk_label_set_attributes (GTK_LABEL (label), attrs);
  pango_attr_list_unref (attrs);

  gtk_box_pack_start (GTK_BOX (container), frame,
                      FALSE, FALSE, 0);
  table = gtk_table_new (rows, cols, FALSE);                                   
                                                                              
  gtk_container_add (GTK_CONTAINER (frame), hbox); 

  gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 6);

  label = gtk_label_new ("    ");
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), table, FALSE, FALSE, 0);
                                                                               
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);     
  gtk_table_set_col_spacings (GTK_TABLE (table), 6);     

  if (gpw)
    g_object_set_data (G_OBJECT (table), "gpw", gpw);

  gtk_widget_show_all (table);
  
  return table;
}                                                                              


GtkWidget *
gnome_prefs_window_new (const gchar *logo_name)
{
  GnomePrefsWindow *gpw = NULL;
  
  GtkTreeSelection *selection = NULL;
  GtkCellRenderer *cell = NULL;
  GtkTreeStore *model = NULL;
  GtkTreeViewColumn *column = NULL;

  GtkWidget *window = NULL;
  GtkWidget *event_box = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *pixmap = NULL;
  GtkWidget *hsep = NULL;

  GdkColor cwhite;

  PangoAttrList *attrs = NULL; 
  PangoAttribute *attr = NULL; 

  /* Box inside the prefs window */
  GtkWidget *dialog_vbox = NULL;

  /* Build the window */
  window = gtk_dialog_new ();

  gpw = (GnomePrefsWindow *) g_malloc (sizeof (GnomePrefsWindow));
  gpw->last_page = 1;
  
  g_object_set_data_full (G_OBJECT (window), "gpw", (gpointer) gpw, g_free);
  
  gtk_dialog_add_button (GTK_DIALOG (window), GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL);

  
  /* The sections */
  gpw->notebook = gtk_notebook_new ();
  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (gpw->notebook), FALSE);
  gtk_notebook_set_show_border (GTK_NOTEBOOK (gpw->notebook), FALSE);

  pixmap =  gtk_image_new_from_file (logo_name);

  event_box = gtk_event_box_new ();
  gtk_container_add (GTK_CONTAINER (event_box),
		     GTK_WIDGET (pixmap));

  cwhite.red   = 0xff * 0x100;
  cwhite.green = 0xff * 0x100;
  cwhite.blue  = 0xff * 0x100;
  gdk_colormap_alloc_color(gdk_colormap_get_system (), &cwhite, FALSE, TRUE);
  gtk_widget_modify_bg (GTK_WIDGET (event_box),
			GTK_STATE_NORMAL, &cwhite);

  gtk_notebook_prepend_page (GTK_NOTEBOOK (gpw->notebook), event_box, NULL);


  /* The sections */
  dialog_vbox = GTK_DIALOG (window)->vbox;
  
  hbox = gtk_hbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (dialog_vbox), hbox);


  /* Build the TreeView on the left */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
  model = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_INT);
  gpw->sections_tree_view = gtk_tree_view_new ();
  gtk_tree_view_set_model (GTK_TREE_VIEW (gpw->sections_tree_view),
			   GTK_TREE_MODEL (model));
  selection =
    gtk_tree_view_get_selection (GTK_TREE_VIEW (gpw->sections_tree_view));
  gtk_container_add (GTK_CONTAINER (frame), gpw->sections_tree_view);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gpw->sections_tree_view),
				     FALSE);
  cell = gtk_cell_renderer_text_new ();

  column = gtk_tree_view_column_new_with_attributes (NULL, cell, "text", 0,
						     NULL);

  gtk_tree_view_append_column (GTK_TREE_VIEW (gpw->sections_tree_view),
			       GTK_TREE_VIEW_COLUMN (column));
  gtk_tree_selection_set_mode (GTK_TREE_SELECTION (selection),
			       GTK_SELECTION_BROWSE);

  
  /* Some design stuff to put the notebook pages in it */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);
  
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);
  
  
  gpw->section_label = gtk_label_new (NULL);
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 4);
  gtk_misc_set_alignment (GTK_MISC (gpw->section_label), 0.0, 0.5);
  gtk_container_add (GTK_CONTAINER (frame), gpw->section_label);
  gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
  attrs = pango_attr_list_new ();
  attr = pango_attr_scale_new (PANGO_SCALE_LARGE);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT; 
  pango_attr_list_insert (attrs, attr); 
  attr = pango_attr_weight_new (PANGO_WEIGHT_HEAVY);
  attr->start_index = 0;
  attr->end_index = G_MAXUINT;
  pango_attr_list_insert (attrs, attr);
  gtk_label_set_attributes (GTK_LABEL (gpw->section_label), attrs);
  pango_attr_list_unref (attrs);
  gtk_widget_show (gpw->section_label);

  hsep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (vbox), hsep, FALSE, FALSE, 0); 
  gtk_box_pack_start (GTK_BOX (vbox), gpw->notebook, TRUE, TRUE, 0);

  gtk_widget_show_all (GTK_WIDGET (dialog_vbox));
  gtk_widget_show_all (GTK_WIDGET (gpw->sections_tree_view));

  g_signal_connect (selection, "changed",
		    G_CALLBACK (tree_selection_changed_cb),
		    gpw);

  
  return window;
}


void
gnome_prefs_window_section_new (GtkWidget *window,
				const gchar *section_name)
{
  GnomePrefsWindow *gpw = NULL;
  GtkTreeModel *model = NULL;
    
  if (!window)
    return;

  if (window)
    gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (window), "gpw");

  if (!gpw || !section_name)
    return;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpw->sections_tree_view));
  gtk_tree_store_append (GTK_TREE_STORE (model), &gpw->iter, NULL);
  gtk_tree_store_set (GTK_TREE_STORE (model), &gpw->iter, 0,
		      section_name, 1, 0, -1);
  gtk_tree_view_expand_all (GTK_TREE_VIEW (gpw->sections_tree_view));
}


GtkWidget *
gnome_prefs_window_subsection_new (GtkWidget *window,
				   const gchar *section_name)
{
  GnomePrefsWindow *gpw = NULL;
  GtkWidget *container = NULL;
  GtkTreeModel *model = NULL;
  GtkTreeIter child_iter;
    
  if (!window)
    return NULL;
  
  gpw = (GnomePrefsWindow *) g_object_get_data (G_OBJECT (window), "gpw");

  if (!gpw || !section_name)
    return NULL;

  container = gtk_vbox_new (FALSE, 4);
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (gpw->sections_tree_view));
  gtk_tree_store_append (GTK_TREE_STORE (model), &child_iter, &gpw->iter);
  gtk_tree_store_set (GTK_TREE_STORE (model), &child_iter, 0, section_name,
		      1, gpw->last_page, -1);
  gtk_tree_view_expand_all (GTK_TREE_VIEW (gpw->sections_tree_view));
  
  gpw->last_page++;
  
  gtk_notebook_append_page (GTK_NOTEBOOK (gpw->notebook),
			    container, NULL);

  gtk_widget_show_all (container);
  
  return container;
}
