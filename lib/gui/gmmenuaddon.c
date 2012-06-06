
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
 *                         gtk_menu_extensions.c  -  description 
 *                         -------------------------------------
 *   begin                : Mon Sep 29 2003, but based on older code
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Helpers to create the menus.
 *
 */


#include "gmmenuaddon.h"

#include <libintl.h>
#include <gdk/gdkkeysyms.h>


/* Notice, this implementation sets the menu item name as data of the menu
 * widget, the statusbar and also the given structure.
 */
static void menus_have_icons_changed_nt (gpointer,
					 GmConfEntry *,
					 gpointer);
     
static gint popup_menu_callback (GtkWidget *,
				 GdkEventButton *,
				 gpointer);

static void menu_item_selected (GtkWidget *,
				gpointer);

static void menu_widget_destroyed (GtkWidget *,
				   gpointer);


/* DESCRIPTION  :  This notifier is called when the menu_have_icons key is
 *                 modified.
 * BEHAVIOR     :  Show/hide icons in the menu.
 * PRE          :  data = the GtkWidget for the menu.
 */
static void
menus_have_icons_changed_nt (G_GNUC_UNUSED gpointer cid,
			     GmConfEntry *entry,
			     gpointer data)
{
  gboolean show_icons = TRUE;

  g_return_if_fail (gm_conf_entry_get_type (entry) == GM_CONF_BOOL && data);
  
  show_icons = gm_conf_entry_get_bool (entry);
  gtk_menu_show_icons (GTK_WIDGET (data), show_icons);
}


/* DESCRIPTION  :  This callback is called when the user clicks on an 
 *                 event-box.
 * BEHAVIOR     :  Displays the menu given as data if it was a right click.
 * PRE          :  data != NULL.
 */
static gint
popup_menu_callback (G_GNUC_UNUSED GtkWidget *widget,
		     GdkEventButton *event,
		     gpointer data)
{
  GtkMenu *menu;
  GdkEventButton *event_button;

  menu = GTK_MENU (data);
  
  if (event->type == GDK_BUTTON_PRESS) {

    event_button = (GdkEventButton *) event;
    if (event_button->button == 3) {

      gtk_menu_popup (menu, NULL, NULL, NULL, NULL,
		      event_button->button, event_button->time);
      return TRUE;
    }
  }

  return FALSE;
}


/* DESCRIPTION  :  This callback is called when a menu item is selected or
 *                 deselected.
 * BEHAVIOR     :  Displays the data in the statusbar.
 * PRE          :  If data is NULL, clears the statusbar, else displays data
 *                 as message in the statusbar.
 */
static void 
menu_item_selected (GtkWidget *w,
		    gpointer data)
{
  GtkWidget *statusbar = NULL;

  gint id = 0;
  int len = 0;
  int i = 0;
  
  statusbar = (GtkWidget *) g_object_get_data (G_OBJECT (w), "statusbar");

  if (!statusbar)
    return;
  else {

    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "statusbar");
    
    if (data) {
      
      gtk_statusbar_push (GTK_STATUSBAR (statusbar), id, (gchar *) data);
    }
    else {

      
      len = g_slist_length ((GSList *) (GTK_STATUSBAR (statusbar)->messages));
     
      for (i = 0 ; i < len ; i++)
	gtk_statusbar_pop (GTK_STATUSBAR (statusbar), id);
    }
  }
}


/* DESCRIPTION  :  This callback is called when the widget associated to a menu
 *                 is destroyed.
 * BEHAVIOR     :  Removes the notifier watching the "menus_have_icons" key.
 * PRE          :  data = the notifier id.
 */
static void
menu_widget_destroyed (G_GNUC_UNUSED GtkWidget *w,
		       gpointer data)
{  
  gm_conf_notifier_remove (data);
}


/* The public functions */
void 
radio_menu_changed_cb (GtkWidget *widget,
		       gpointer data)
{
  GSList *group = NULL;

  int group_last_pos = 0;
  int active = 0;

  g_return_if_fail (data != NULL);

  
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


void 
toggle_menu_changed_cb (GtkWidget *widget, 
			gpointer data)
{
  g_return_if_fail (data != NULL);
  
  gm_conf_set_bool ((gchar *) data, 
		    GTK_CHECK_MENU_ITEM (widget)->active);
}


void 
menu_toggle_changed_nt (G_GNUC_UNUSED gpointer id, 
                        GmConfEntry *entry, 
                        gpointer data)
{
  GtkWidget *e = NULL;
  
  g_return_if_fail (data != NULL);
  

  if (gm_conf_entry_get_type (entry) == GM_CONF_BOOL) {
   
    e = GTK_WIDGET (data);

    /* We set the new value for the widget */
    GTK_CHECK_MENU_ITEM (e)->active = gm_conf_entry_get_bool (entry);

    gtk_widget_queue_draw (GTK_WIDGET (e));
  }
}


void
radio_menu_changed_nt (G_GNUC_UNUSED gpointer id,
		       GmConfEntry *entry,
		       gpointer data)
{
  g_return_if_fail (data != NULL);


  if (gm_conf_entry_get_type (entry) == GM_CONF_INT) {

    gtk_radio_menu_select_with_widget (GTK_WIDGET (data),
				       gm_conf_entry_get_int (entry));
  }
}


void 
gtk_build_menu (GtkWidget *menubar,
		MenuEntry *menu,
		GtkAccelGroup *accel,
		GtkWidget *statusbar)
{
  GtkWidget *menu_widget = menubar;
  GtkWidget *old_menu = NULL;
  GtkWidget *image = NULL;
  GtkStockItem item;

  GSList *group = NULL;
  
  int i = 0;
  gchar *menu_name = NULL;

  gpointer id = NULL;
  gboolean show_icons = TRUE;

  show_icons =
    gm_conf_get_bool ("/desktop/gnome/interface/menus_have_icons");
    
  while (menu [i].type != MENU_END) {

    GSList *new_group = NULL;
    
    if (menu [i].type != MENU_RADIO_ENTRY) 
      group = NULL;

    if (menu [i].stock_id && !menu [i].stock_is_theme && !menu [i].name) {

      if (gtk_stock_lookup (menu [i].stock_id, &item))
        menu_name = g_strdup (gettext (item.label));
      else
        menu_name = g_strdup (menu [i].name);
    }
    else 
      menu_name = g_strdup (menu [i].name);

    if (menu_name) {

      if (menu [i].type == MENU_ENTRY 
	  || menu [i].type == MENU_SUBMENU_NEW
	  || menu [i].type == MENU_NEW)
	menu [i].widget = 
	  gtk_image_menu_item_new_with_mnemonic (menu_name);
      else if (menu [i].type == MENU_TOGGLE_ENTRY) {
	
	menu [i].widget = 
	  gtk_check_menu_item_new_with_mnemonic (menu_name);
	GTK_CHECK_MENU_ITEM (menu [i].widget)->active =
	  menu [i].enabled;
	gtk_widget_queue_draw (menu [i].widget);
      }
      else if (menu [i].type == MENU_RADIO_ENTRY) {

	if (group == NULL) 
	  group = new_group;
	
	menu [i].widget = 
	  gtk_radio_menu_item_new_with_mnemonic (group, 
						 menu_name);

	GTK_CHECK_MENU_ITEM (menu [i].widget)->active =
	  menu [i].enabled;
	gtk_widget_queue_draw (menu [i].widget);

	group = 
	  gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu[i].widget));
      }

      if (menu [i].stock_id && show_icons) {

	if (menu [i].stock_is_theme)
	  image = gtk_image_new_from_icon_name(menu [i].stock_id,
					       GTK_ICON_SIZE_MENU);
	else
	  image = gtk_image_new_from_stock (menu [i].stock_id,
					    GTK_ICON_SIZE_MENU);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu[i].widget),
				       image);
	gtk_widget_show (image);
      }

      if (menu [i].accel && accel)
	{
/*        if ((menu [i].accel == GDK_F1) || (menu [i].accel == GDK_F11))
          gtk_widget_add_accelerator (menu [i].widget, "activate", 
                                      accel, menu [i].accel, 
                                      0, GTK_ACCEL_VISIBLE);
        else
          gtk_widget_add_accelerator (menu [i].widget, "activate", 
                                      accel, menu [i].accel, 
                                      GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
*/
	  switch (menu [i].accel) {
		case GDK_F1:
		case GDK_F11:
		case GDK_Escape:
		case GDK_h:
		case GDK_t:
		case GDK_m:
		case GDK_p:
          		gtk_widget_add_accelerator (menu [i].widget, "activate",
                                      accel, menu [i].accel,
                                      0, GTK_ACCEL_VISIBLE);
			break;
		default:
          		gtk_widget_add_accelerator (menu [i].widget, "activate",
                                      accel, menu [i].accel,
                                      GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
			break;
	  }		
	}

      if (menu [i].func) {

        if (menu [i].clofunc) {
          g_signal_connect_data (G_OBJECT (menu [i].widget),
                                 "activate", menu [i].func,
                                 menu [i].data,
                                 (GClosureNotify) menu [i].clofunc, 
                                 (GConnectFlags) G_CONNECT_AFTER);
        }
        else {
          g_signal_connect (G_OBJECT (menu [i].widget),
                            "activate", menu [i].func,
                            menu [i].data);
        }
      }

      g_object_set_data (G_OBJECT (menu [i].widget),
			 "statusbar", statusbar);
      g_signal_connect (G_OBJECT (menu [i].widget),
			"select", G_CALLBACK (menu_item_selected), 
			(gpointer) menu [i].tooltip);
      g_signal_connect (G_OBJECT (menu [i].widget),
			"deselect", G_CALLBACK (menu_item_selected), 
			NULL);
    }
    g_free (menu_name);

    if (menu [i].type == MENU_SEP) {

      menu [i].widget = 
	gtk_separator_menu_item_new ();      

      if (old_menu) {

	menu_widget = old_menu;
	old_menu = NULL;
      }
    }    

    if (menu [i].type == MENU_NEW
	|| menu [i].type == MENU_SUBMENU_NEW) {
	
      if (menu [i].type == MENU_SUBMENU_NEW) 
	old_menu = menu_widget;
      menu_widget = gtk_menu_new ();
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu [i].widget),
				 menu_widget);

      if (menu [i].type == MENU_NEW)
	gtk_menu_shell_append (GTK_MENU_SHELL (menubar), 
			       menu [i].widget);
      else
	gtk_menu_shell_append (GTK_MENU_SHELL (old_menu), 
			       menu [i].widget);
    }
    else
      gtk_menu_shell_append (GTK_MENU_SHELL (menu_widget),
			     menu [i].widget);      

    
    if (menu [i].id) {
      
      if (menu [i].type != MENU_SUBMENU_NEW)
	g_object_set_data (G_OBJECT (menubar), menu [i].id,
			   menu [i].widget);
      else
	g_object_set_data (G_OBJECT (menubar), menu [i].id,
			   menu_widget);
    }
    
    if (!menu [i].sensitive)
      gtk_widget_set_sensitive (GTK_WIDGET (menu [i].widget), FALSE);
    
    gtk_widget_show (menu [i].widget);

    i++;
  }

  g_object_set_data (G_OBJECT (menubar), "menu_entry", menu);

  id = gm_conf_notifier_add ("/desktop/gnome/interface/menus_have_icons",
			     menus_have_icons_changed_nt, 
			     menubar);

  g_signal_connect (G_OBJECT (menubar), "destroy",
		    G_CALLBACK (menu_widget_destroyed), id);
}


GtkWidget *
gtk_build_popup_menu (GtkWidget *widget,
		      MenuEntry *menu,
		      GtkAccelGroup *accel)
{
  GtkWidget *popup_menu_widget = NULL;
  popup_menu_widget = gtk_menu_new ();

  gtk_build_menu (popup_menu_widget, menu, accel, NULL);
  gtk_widget_show_all (popup_menu_widget);

  if (widget) {

    g_signal_connect (G_OBJECT (widget), "button_press_event",
                      G_CALLBACK (popup_menu_callback), 
                      (gpointer) popup_menu_widget);

    gtk_widget_add_events (widget, GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK);
  }

  return popup_menu_widget;
}


void
gtk_menu_set_sensitive (GtkWidget *menu,
			const char *id,
			gboolean sensitive)
{
  GtkWidget *menu_item = NULL;

  g_return_if_fail (menu != NULL && id != NULL);

  menu_item = (GtkWidget *) g_object_get_data (G_OBJECT (menu), id);

  if (menu_item) 
    gtk_widget_set_sensitive (GTK_WIDGET (menu_item), sensitive);
}


void
gtk_menu_section_set_sensitive (GtkWidget *menu,
				const char *id,
				gboolean sensitive)
{
  GtkWidget *menu_item = NULL;
  MenuEntry *menu_entry = NULL;

  int i = 0;
  
  g_return_if_fail (menu != NULL && id != NULL);

  menu_item = (GtkWidget *) g_object_get_data (G_OBJECT (menu), id);
  menu_entry = (MenuEntry *) g_object_get_data (G_OBJECT (menu), "menu_entry");

  if (menu_item && menu_item) {

    while (menu_entry [i].type != MENU_END
	   && menu_entry [i].widget != menu_item)
      i++;

    while (menu_entry [i].type != MENU_END
	   && menu_entry [i].type != MENU_SEP
	   && menu_entry [i].type != MENU_NEW
	   && menu_entry [i].type != MENU_SUBMENU_NEW) {
      
      gtk_widget_set_sensitive (GTK_WIDGET (menu_entry [i].widget),
				sensitive);
      i++;
    }
  }
}


GtkWidget *
gtk_menu_get_widget (GtkWidget *menu,
		     const char *id)
{
  g_return_val_if_fail (menu != NULL && id != NULL, NULL);

  return (GtkWidget *) g_object_get_data (G_OBJECT (menu), id);
}


void
gtk_toggle_menu_enable (GtkWidget *e, 
			gboolean show)
{
  GTK_CHECK_MENU_ITEM (e)->active = show;

  gtk_widget_queue_draw (GTK_WIDGET (e));
}


void
gtk_radio_menu_select_with_id (GtkWidget *menu,
			       const gchar *id,
			       int active)
{
  GtkWidget *widget = NULL;

  GSList *group = NULL;

  int group_last_pos = 0;
  int i = 0;
  
  widget = gtk_menu_get_widget (menu, id);

  if (!widget)
    return;
  
  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, 
						  last pos is 0 */
  while (group) {

#if GTK_CHECK_VERSION(2,20,0)
    if (gtk_widget_get_sensitive (GTK_WIDGET (GTK_CHECK_MENU_ITEM (group->data))))
#else
    if (GTK_WIDGET_SENSITIVE (GTK_CHECK_MENU_ITEM (group->data)))
#endif
      GTK_CHECK_MENU_ITEM (group->data)->active =
        (i == group_last_pos - active);
    else
      GTK_CHECK_MENU_ITEM (group->data)->active = FALSE;

    gtk_widget_queue_draw (GTK_WIDGET (group->data));

    group = g_slist_next (group);
    i++;
  }
}


void
gtk_radio_menu_select_with_widget (GtkWidget *widget,
				   int active)
{
  GSList *group = NULL;

  int group_last_pos = 0;
  int i = 0;
  
  g_return_if_fail (widget != NULL);
  
  group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
  group_last_pos = g_slist_length (group) - 1; /* If length 1, 
						  last pos is 0 */
  while (group) {

    GTK_CHECK_MENU_ITEM (group->data)->active = 
      (i == group_last_pos - active);
    gtk_widget_queue_draw (GTK_WIDGET (group->data));
      
    group = g_slist_next (group);
    i++;
  }
}


void
gtk_menu_show_icons (GtkWidget *menu, gboolean show_icons)
{
  MenuEntry *menu_entry = NULL;
  GtkWidget *image = NULL;
  int i = 0;
  
  menu_entry = (MenuEntry *) g_object_get_data (G_OBJECT (menu), "menu_entry");

  while (menu_entry && menu_entry [i].type != MENU_END) {

    if (menu_entry [i].stock_id) {

      image = gtk_image_menu_item_get_image (GTK_IMAGE_MENU_ITEM (menu_entry [i].widget));

      if (show_icons) {

	if (!image) {
	  
	  if (menu_entry [i].stock_is_theme)
	    image = gtk_image_new_from_icon_name(menu_entry [i].stock_id,
						 GTK_ICON_SIZE_MENU);
	  else
	    image = gtk_image_new_from_stock (menu_entry [i].stock_id,
					      GTK_ICON_SIZE_MENU);
	  gtk_widget_show (image);

	  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menu_entry [i].widget), image);
	}
	else
	  gtk_widget_show (image);
      }
      else
	if (image)
	  gtk_widget_hide (image);
	
    }
    
    i++;
  } 
}
