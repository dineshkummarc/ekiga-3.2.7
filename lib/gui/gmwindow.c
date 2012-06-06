
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
 *                         gmwindow.c -  description
 *                         -------------------------
 *   begin                : 16 August 2007 
 *   copyright            : (c) 2007 by Damien Sandras 
 *   description          : Implementation of a GtkWindow able to restore
 *                          its position and size in a GmConf key.
 *
 */

#include "gmwindow.h"

#include "gmconf.h"

#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>


/*
 * The GmWindow
 */
struct _GmWindowPrivate
{
  GtkAccelGroup *accel;
  gboolean hide_on_esc;
  gboolean hide_on_delete;
  const gchar *key;
  int x;
  int y;
  int width;
  int height;
};

enum {
  GM_WINDOW_KEY = 1,
  GM_HIDE_ON_ESC = 2,
  GM_HIDE_ON_DELETE = 3
};

static GObjectClass *parent_class = NULL;

static gboolean
gm_window_delete_event (GtkWidget *w,
			gpointer data);

static void
gm_window_show (GtkWidget *w,
                gpointer data);

static void
gm_window_hide (GtkWidget *w,
                gpointer data);

static gboolean 
gm_window_configure_event (GtkWidget *widget,
                           GdkEventConfigure *event);


/* 
 * GObject stuff
 */
static void
gm_window_dispose (GObject *obj)
{
  GmWindow *window = NULL;

  window = GM_WINDOW (obj);

  parent_class->dispose (obj);
}


static void
gm_window_finalize (GObject *obj)
{
  GmWindow *window = NULL;

  window = GM_WINDOW (obj);

  g_free ((gchar *) window->priv->key);

  parent_class->finalize (obj);
}


static void
gm_window_get_property (GObject *obj,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *spec)
{
  GmWindow *self = NULL;

  self = GM_WINDOW (obj);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_WINDOW_TYPE, GmWindowPrivate);

  switch (prop_id) {

  case GM_WINDOW_KEY:
    g_value_set_string (value, self->priv->key);
    break;

  case GM_HIDE_ON_ESC:
    g_value_set_boolean (value, self->priv->hide_on_esc);
    break;

  case GM_HIDE_ON_DELETE:
    g_value_set_boolean (value, self->priv->hide_on_delete);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
gm_window_set_property (GObject *obj,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *spec)
{
  GmWindow *self = NULL;
  const gchar *str = NULL;

  self = GM_WINDOW (obj);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_WINDOW_TYPE, GmWindowPrivate);

  switch (prop_id) {

  case GM_WINDOW_KEY:
    g_free ((gchar *) self->priv->key);
    str = g_value_get_string (value);
    self->priv->key = g_strdup (str ? str : "");
    break;

  case GM_HIDE_ON_ESC:
    self->priv->hide_on_esc = g_value_get_boolean (value);
    if (!self->priv->hide_on_esc)
      gtk_accel_group_disconnect_key (self->priv->accel, GDK_Escape, (GdkModifierType) 0);
    else
      gtk_accel_group_connect (self->priv->accel, GDK_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                               g_cclosure_new_swap (G_CALLBACK (gtk_widget_hide), (gpointer) self, NULL));
    break;

  case GM_HIDE_ON_DELETE:
    self->priv->hide_on_delete = g_value_get_boolean (value);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, spec);
    break;
  }
}


static void
gm_window_class_init (gpointer g_class,
                      G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass *gobject_class = NULL;
  GParamSpec *spec = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);
  g_type_class_add_private (g_class, sizeof (GmWindowPrivate));

  gobject_class = (GObjectClass *) g_class;
  gobject_class->dispose = gm_window_dispose;
  gobject_class->finalize = gm_window_finalize;
  gobject_class->get_property = gm_window_get_property;
  gobject_class->set_property = gm_window_set_property;

  spec = g_param_spec_string ("key", "Key", "Key", 
                              NULL, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_WINDOW_KEY, spec); 

  spec = g_param_spec_boolean ("hide_on_esc", "Hide on Escape", "Hide on Escape", 
                               TRUE, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_HIDE_ON_ESC, spec); 

  spec = g_param_spec_boolean ("hide_on_delete", "Hide on delete-event", "Hide on delete-event (or just relay the event)",
			       TRUE, (GParamFlags) G_PARAM_READWRITE);
  g_object_class_install_property (gobject_class, GM_HIDE_ON_DELETE, spec);
}


static void
gm_window_init (GTypeInstance *instance,
                gpointer g_class)
{
  GmWindow *self = NULL;

  (void) g_class; /* -Wextra */

  self = GM_WINDOW (instance);
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, GM_WINDOW_TYPE, GmWindowPrivate);
  self->priv->key = g_strdup ("");
  self->priv->hide_on_esc = TRUE;
  self->priv->hide_on_delete = TRUE;

  self->priv->accel = gtk_accel_group_new ();
  gtk_window_add_accel_group (GTK_WINDOW (self), self->priv->accel);
  gtk_accel_group_connect (self->priv->accel, GDK_Escape, (GdkModifierType) 0, GTK_ACCEL_LOCKED,
                           g_cclosure_new_swap (G_CALLBACK (gtk_widget_hide), (gpointer) self, NULL));

  g_signal_connect (G_OBJECT (self), "delete_event",
		    G_CALLBACK (gm_window_delete_event), NULL);

  g_signal_connect (G_OBJECT (self), "show",
                    G_CALLBACK (gm_window_show), self);

  g_signal_connect (G_OBJECT (self), "hide",
                    G_CALLBACK (gm_window_hide), self);

  g_signal_connect (G_OBJECT (self), "configure-event",
                    G_CALLBACK (gm_window_configure_event), self);
}


GType
gm_window_get_type ()
{
  static GType result = 0;

  if (result == 0) {

    static const GTypeInfo info = {
      sizeof (GmWindowClass),
      NULL,
      NULL,
      gm_window_class_init,
      NULL,
      NULL,
      sizeof (GmWindow),
      0,
      gm_window_init,
      NULL
    };

    result = g_type_register_static (GTK_TYPE_WINDOW,
				     "GmWindowType",
				     &info, (GTypeFlags) 0);
  }

  return result;
}


/* 
 * Our own stuff
 */

static gboolean
gm_window_delete_event (GtkWidget *w,
                        G_GNUC_UNUSED gpointer data)
{
  GmWindow* self = NULL;

  self = GM_WINDOW (w);

  if (self->priv->hide_on_delete) {
    gtk_widget_hide (w);
    return TRUE;
  } else {
    return FALSE;
  }
}


static void
gm_window_show (GtkWidget *w,
                G_GNUC_UNUSED gpointer data)
{
  int x = 0;
  int y = 0;

  GmWindow *self = NULL;

  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  gchar **couple = NULL;

  g_return_if_fail (w != NULL);
  
  self = GM_WINDOW (w);

  g_return_if_fail (strcmp (self->priv->key, ""));

  conf_key_position =
    g_strdup_printf ("%s/position", self->priv->key);
  conf_key_size =
    g_strdup_printf ("%s/size", self->priv->key);

  if (gtk_window_get_resizable (GTK_WINDOW (w))) {

    size = gm_conf_get_string (conf_key_size);
    if (size)
      couple = g_strsplit (size, ",", 0);

    if (couple && couple [0])
      x = atoi (couple [0]);
    if (couple && couple [1])
      y = atoi (couple [1]);

    if (x > 0 && y > 0) {
      gtk_window_resize (GTK_WINDOW (w), x, y);
    }

    g_strfreev (couple);
    g_free (size);
  }

  position = gm_conf_get_string (conf_key_position);
  if (position)
    couple = g_strsplit (position, ",", 0);

  if (couple && couple [0])
    x = atoi (couple [0]);
  if (couple && couple [1])
    y = atoi (couple [1]);

  if (x != 0 && y != 0)
    gtk_window_move (GTK_WINDOW (w), x, y);

  g_strfreev (couple);
  couple = NULL;
  g_free (position);

  gtk_widget_realize (GTK_WIDGET (w));

  g_free (conf_key_position);
  g_free (conf_key_size);
}


static void
gm_window_hide (GtkWidget *w,
                G_GNUC_UNUSED gpointer data)
{
  GmWindow *self = NULL;

  gchar *conf_key_size = NULL;
  gchar *conf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  
  g_return_if_fail (w != NULL);
  
  self = GM_WINDOW (w);

  g_return_if_fail (strcmp (self->priv->key, ""));

  conf_key_position =
    g_strdup_printf ("%s/position", self->priv->key);
  conf_key_size =
    g_strdup_printf ("%s/size", self->priv->key);

  position = g_strdup_printf ("%d,%d", self->priv->x, self->priv->y);
  gm_conf_set_string (conf_key_position, position);
  g_free (position);

  if (gtk_window_get_resizable (GTK_WINDOW (w))) {

    size = g_strdup_printf ("%d,%d", self->priv->width, self->priv->height);
    gm_conf_set_string (conf_key_size, size);
    g_free (size);
  }
  
  g_free (conf_key_position);
  g_free (conf_key_size);
}


static gboolean 
gm_window_configure_event (GtkWidget *self,
                           GdkEventConfigure *event)
{
  gtk_window_get_position (GTK_WINDOW (self), &GM_WINDOW (self)->priv->x, &GM_WINDOW (self)->priv->y);

  GM_WINDOW (self)->priv->width = event->width;
  GM_WINDOW (self)->priv->height = event->height;

  return FALSE;
}


/* 
 * Public API
 */
GtkWidget *
gm_window_new ()
{
  return GTK_WIDGET (g_object_new (GM_WINDOW_TYPE, NULL));
}


GtkWidget *
gm_window_new_with_key (const char *key)
{
  GtkWidget *window = NULL;

  g_return_val_if_fail (key != NULL, NULL);

  window = gm_window_new ();
  gm_window_set_key (GM_WINDOW (window), key);

  return window;
}


void
gm_window_set_key (GmWindow *window,
                   const char *key)
{
  g_return_if_fail (window != NULL);
  g_return_if_fail (key != NULL);

  g_object_set (GM_WINDOW (window), "key", key, NULL);
}


void 
gm_window_get_size (GmWindow *self,
                    int *x,
                    int *y)
{
  gchar *conf_key_size = NULL;
  gchar *size = NULL;
  gchar **couple = NULL;

  g_return_if_fail (self != NULL);

  conf_key_size = g_strdup_printf ("%s/size", self->priv->key);
  size = gm_conf_get_string (conf_key_size);
  if (size)
    couple = g_strsplit (size, ",", 0);

  if (x && couple && couple [0])
    *x = atoi (couple [0]);
  if (y && couple && couple [1])
    *y = atoi (couple [1]);

  g_free (conf_key_size);
  g_free (size);
  g_strfreev (couple);
}

void
gm_window_set_hide_on_delete (GmWindow *window,
			      gboolean hide_on_delete)
{
  g_return_if_fail (window != NULL);
  g_return_if_fail (IS_GM_WINDOW (window));

  g_object_set (GM_WINDOW (window), "hide_on_delete", hide_on_delete, NULL);
}

gboolean
gm_window_get_hide_on_delete (GmWindow *window)
{
  g_return_val_if_fail (window != NULL, FALSE);
  g_return_val_if_fail (IS_GM_WINDOW (window), FALSE);

  return window->priv->hide_on_delete;
}

