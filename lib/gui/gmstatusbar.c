
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
 *                         gmstatusbar.c  -  description
 *                         -------------------------------
 *   begin                : Tue Nov 01 2005
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a statusbar widget 
 *
 */


#include "gmstatusbar.h"


/* Static functions and declarations */
static void gm_statusbar_class_init (GmStatusbarClass *);

static void gm_statusbar_init (GmStatusbar *);

static void gm_sb_push_message (GmStatusbar *, 
				gboolean,
				gboolean,
				const char *,
				va_list args);

static int  gm_statusbar_clear_msg_cb (gpointer);


static GtkStatusbarClass *parent_class = NULL;
static GmStatusbar *object = NULL;


static void
gm_statusbar_class_init (GmStatusbarClass *klass)
{
  GObjectClass *object_class = NULL;
  GtkObjectClass *gtkobject_class = NULL;
  GmStatusbarClass *statusbar_class = NULL;

  gtkobject_class = GTK_OBJECT_CLASS (klass);
  object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
  statusbar_class = GM_STATUSBAR_CLASS (klass);
}


static void
gm_statusbar_init (GmStatusbar *sb)
{
  g_return_if_fail (sb != NULL);
  g_return_if_fail (GM_IS_STATUSBAR (sb));

  object = sb;
}


static int 
gm_statusbar_clear_msg_cb (gpointer data)
{
  gint id = 0;
  
  g_return_val_if_fail (data != NULL, FALSE);
  
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (object), "statusbar");
  gtk_statusbar_remove (GTK_STATUSBAR (object), id, GPOINTER_TO_INT (data));

  return FALSE;
}


static void 
gm_sb_push_message (GmStatusbar *sb, 
		    gboolean flash_message,
		    gboolean info_message,
		    const char *msg, 
		    va_list args)
{
  gint id = 0;
  gint msg_id = 0;
  int len = 0;
  int i = 0;
  static guint timer_source;

  g_return_if_fail (sb != NULL);

  len = g_slist_length ((GSList *) (GTK_STATUSBAR (sb)->messages));
  if (info_message)
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (sb), "info");
  else
    id = gtk_statusbar_get_context_id (GTK_STATUSBAR (sb), "statusbar");
  
  for (i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (sb), id);

  if (msg) {

    char buffer [1025];

    vsnprintf (buffer, 1024, msg, args);

    msg_id = gtk_statusbar_push (GTK_STATUSBAR (sb), id, buffer);

    if (flash_message)
    {
      if (timer_source != 0)
      {
        g_source_remove (timer_source);
        timer_source = 0;
      }

#if GLIB_CHECK_VERSION (2, 14, 0)
      timer_source = g_timeout_add_seconds (15, gm_statusbar_clear_msg_cb, 
		       GINT_TO_POINTER (msg_id));
#else
      timer_source = g_timeout_add (15000, gm_statusbar_clear_msg_cb, 
		       GINT_TO_POINTER (msg_id));
#endif
    }
  }
}


/* Global functions */
GType
gm_statusbar_get_type (void)
{
  static GType gm_statusbar_type = 0;
  
  if (gm_statusbar_type == 0)
  {
    static const GTypeInfo statusbar_info =
    {
      sizeof (GmStatusbarClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_statusbar_class_init,
      NULL,
      NULL,
      sizeof (GmStatusbar),
      0,
      (GInstanceInitFunc) gm_statusbar_init,
      NULL
    };
    
    gm_statusbar_type =
      g_type_register_static (GTK_TYPE_STATUSBAR,
			      "GmStatusbar",
			      &statusbar_info,
			      (GTypeFlags) 0);
  }
  
  return gm_statusbar_type;
}


GtkWidget *
gm_statusbar_new ()
{
  GmStatusbar *sb = NULL;
  
  sb = GM_STATUSBAR (g_object_new (GM_STATUSBAR_TYPE, NULL));

  gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR (object), FALSE);

  return GTK_WIDGET (sb);
}


void 
gm_statusbar_flash_message (GmStatusbar *sb, 
			    const char *msg, 
			    ...)
{
  va_list args;

  va_start (args, msg);
  gm_sb_push_message (sb, TRUE, FALSE, msg, args);

  va_end (args);
}


void 
gm_statusbar_push_message (GmStatusbar *sb, 
			   const char *msg, 
			   ...)
{
  va_list args;

  va_start (args, msg);
  gm_sb_push_message (sb, FALSE, FALSE, msg, args);

  va_end (args);
}


void 
gm_statusbar_push_info_message (GmStatusbar *sb, 
				const char *msg, 
				...)
{
  va_list args;

  va_start (args, msg);
  gm_sb_push_message (sb, FALSE, TRUE, msg, args);

  va_end (args);
}
