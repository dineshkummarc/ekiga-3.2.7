
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
 *                         connectbutton.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a connectbutton widget 
 *
 */


#include "gmconnectbutton.h"

#include <string.h>

/* Static functions and declarations */
static void gm_connect_button_clicked_cb (GtkWidget *button,
                                          gpointer data);

static void gm_connect_button_class_init (GmConnectButtonClass *);

static void gm_connect_button_init (GmConnectButton *);

static void gm_connect_button_destroy (GtkObject *);

static GtkHBoxClass *parent_class = NULL;


static void
gm_connect_button_clicked_cb (G_GNUC_UNUSED GtkWidget *button,
                              gpointer data)
{
  GmConnectButton *cb = NULL;

  g_return_if_fail (data != NULL);

  cb = GM_CONNECT_BUTTON (data);

  g_signal_emit_by_name (GM_CONNECT_BUTTON (data), "clicked", NULL);
}


static void
gm_connect_button_class_init (GmConnectButtonClass *klass)
{
  static gboolean initialized = FALSE;

  GObjectClass *object_class = NULL;
  GtkObjectClass *gtkobject_class = NULL;
  GmConnectButtonClass *connect_button_class = NULL;

  gtkobject_class = GTK_OBJECT_CLASS (klass);
  object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
  connect_button_class = GM_CONNECT_BUTTON_CLASS (klass);

  gtkobject_class->destroy = gm_connect_button_destroy;

  if (!initialized) {

    klass->clicked_signal =
      g_signal_new ("clicked",
                    G_OBJECT_CLASS_TYPE (klass),
                    G_SIGNAL_RUN_FIRST,
                    0, NULL, NULL,
                    g_cclosure_marshal_VOID__VOID,
                    G_TYPE_NONE,
                    0, NULL);

    initialized = TRUE;
  }
}

static void
gm_connect_button_init (GmConnectButton *cb)
{
  g_return_if_fail (cb != NULL);
  g_return_if_fail (GM_IS_CONNECT_BUTTON (cb));

  cb->pickup_button = NULL;
  cb->hangup_button = NULL;
}


static void
gm_connect_button_destroy (GtkObject *object)
{
  GmConnectButton *cb = NULL;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GM_IS_CONNECT_BUTTON (object));

  cb = GM_CONNECT_BUTTON (object);
  
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (*GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}


/* Global functions */
GType
gm_connect_button_get_type (void)
{
  static GType gm_connect_button_type = 0;
  
  if (gm_connect_button_type == 0)
  {
    static const GTypeInfo connect_button_info =
    {
      sizeof (GmConnectButtonClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_connect_button_class_init,
      NULL,
      NULL,
      sizeof (GmConnectButton),
      0,
      (GInstanceInitFunc) gm_connect_button_init,
      NULL
    };
    
    gm_connect_button_type =
      g_type_register_static (GTK_TYPE_HBOX,
			      "GmConnectButton",
			      &connect_button_info,
			      (GTypeFlags) 0);
  }
  
  return gm_connect_button_type;
}


GtkWidget *
gm_connect_button_new (const char *pickup,
		       const char *hangup,
		       GtkIconSize size)
{
  GmConnectButton *cb = NULL;
  
  GtkWidget *image = NULL;
  
  g_return_val_if_fail (pickup != NULL, NULL);
  g_return_val_if_fail (hangup != NULL, NULL);
  
  cb = GM_CONNECT_BUTTON (g_object_new (GM_CONNECT_BUTTON_TYPE, NULL));

  cb->pickup_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (cb->pickup_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (pickup, size); 
  gtk_container_add (GTK_CONTAINER (cb->pickup_button), image);
  g_signal_connect (G_OBJECT (cb->pickup_button), "clicked", 
                    G_CALLBACK (gm_connect_button_clicked_cb), cb);

  cb->hangup_button = gtk_button_new ();
  gtk_button_set_relief (GTK_BUTTON (cb->hangup_button), GTK_RELIEF_NONE);
  image = gtk_image_new_from_stock (hangup, size); 
  gtk_container_add (GTK_CONTAINER (cb->hangup_button), image);
  g_signal_connect (G_OBJECT (cb->hangup_button), "clicked", 
                    G_CALLBACK (gm_connect_button_clicked_cb), cb);

  gtk_box_pack_start (GTK_BOX (cb), cb->pickup_button, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (cb), cb->hangup_button, FALSE, FALSE, 0);

  gm_connect_button_set_connected (cb, FALSE);

  return GTK_WIDGET (cb);
}


void 
gm_connect_button_set_connected (GmConnectButton *cb,
				 gboolean state)
{
  g_return_if_fail (cb != NULL);
  g_return_if_fail (GM_IS_CONNECT_BUTTON (cb));

  gtk_widget_set_sensitive (state ? cb->hangup_button : cb->pickup_button, TRUE);
  gtk_widget_set_sensitive (!state ? cb->hangup_button : cb->pickup_button, FALSE);
}


gboolean 
gm_connect_button_get_connected (GmConnectButton *cb)
{
  g_return_val_if_fail (cb != NULL, FALSE);
  g_return_val_if_fail (GM_IS_CONNECT_BUTTON (cb), FALSE);

#if GTK_CHECK_VERSION(2,20,0)
  return (gtk_widget_get_sensitive (GTK_WIDGET (cb->hangup_button)));
#else
  return (GTK_WIDGET_SENSITIVE (cb->hangup_button));
#endif
}


