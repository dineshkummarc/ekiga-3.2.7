
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
 *                         gmconf-gconf.c  -  description
 *                         ------------------------------------------
 *   begin                : Mar 2004, derived from gconf_widgets_extensions.c
 *                          started on Fri Oct 17 2003.
 *   copyright            : (c) 2000-2006 by Damien sandras,
 *                          (c) 2004 by Julien Puydt
 *   description          : gconf implementation of gnomemeeting's
 *                          configuration system
 *
 */


#include "../../config.h"

#include <gconf/gconf-client.h>
#include <string.h>

#include <gmconf/gmconf.h>

/* Using a global variable is dirty, but the api is built like this
 */
static GConfClient *client;

/* this is needed in order to really hide gconf: one needs to be able to
 * call the GmConfNotifier from inside a gconf notifier, so we hide the real
 * notifier and its associated user data into the user data of a gconf
 * notifier, that will do the unwrapping, and call the real stuff */
typedef struct _GConfNotifierWrap GConfNotifierWrap;

struct _GConfNotifierWrap {
  GmConfNotifier real_notifier;
  gpointer real_user_data;
};

static GConfNotifierWrap *gconf_notifier_wrapper_new (GmConfNotifier,
                                                      gpointer);

/* gpointer, because it is a callback */
static void gconf_notifier_wrapper_destroy (gpointer);

/* this is the universal gconf notifier, that interprets its fourth
 * argument as a GConfNotifierWrap*, and fires it */
static void gconf_notifier_wrapper_trigger (GConfClient *,
                                            guint,
                                            GConfEntry *,
                                            gpointer);


/* this functions expects a non-NULL conf notifier, and wraps it for
 * use by the universal gconf notifier */
static GConfNotifierWrap *
gconf_notifier_wrapper_new (GmConfNotifier notifier,
                            gpointer user_data)
{
  GConfNotifierWrap *result = NULL;

  g_return_val_if_fail (notifier != NULL, NULL);

  result = g_new (GConfNotifierWrap, 1);
  result->real_notifier = notifier;
  result->real_user_data = user_data;

  return result;
}

/* this function is automatically called to free the notifiers' wrappers */
static void
gconf_notifier_wrapper_destroy (gpointer wrapper)
{
  g_free ((GConfNotifierWrap *) wrapper);
}


/* this is the universal gconf notification unwrapper: it
 * expects a wrapped gm conf notifier in its user_data argument,
 * and calls it  */
static void
gconf_notifier_wrapper_trigger (G_GNUC_UNUSED GConfClient *client_,
				guint identifier,
				GConfEntry *entry,
				gpointer user_data)
{
  GConfNotifierWrap *wrapper = NULL;

  g_return_if_fail (user_data != NULL);

  wrapper = (GConfNotifierWrap *)user_data;
  wrapper->real_notifier (GUINT_TO_POINTER (identifier),
			  (GmConfEntry *)entry,
			  wrapper->real_user_data);
}


/* From now on, the rest is just calling the gconf functions,
 * just checking for the key's validity */
void
gm_conf_set_bool (const gchar *key,
		  const gboolean b)
{
  g_return_if_fail (key != NULL);

  gconf_client_set_bool (client, key, b, NULL);
}


gboolean
gm_conf_get_bool (const gchar *key)
{
  g_return_val_if_fail (key != NULL, FALSE);

  return gconf_client_get_bool (client, key, NULL);
}


void
gm_conf_set_string (const gchar *key,
		    const gchar *v)
{
  g_return_if_fail (key != NULL);

  gconf_client_set_string (client, key, v, NULL);
}


gchar *
gm_conf_get_string (const gchar *key)
{
  g_return_val_if_fail (key != NULL, NULL);

  return gconf_client_get_string (client, key, NULL);
}


void
gm_conf_set_int (const gchar *key,
		 const int v)
{
  g_return_if_fail (key != NULL);

  gconf_client_set_int (client, key, v, NULL);
}


int
gm_conf_get_int (const gchar *key)
{
  g_return_val_if_fail (key != NULL, 0);

  return gconf_client_get_int (client, key, NULL);
}


void
gm_conf_set_float (const gchar *key,
		   const float v)
{
  g_return_if_fail (key != NULL);

  gconf_client_set_float (client, key, v, NULL);
}


gfloat
gm_conf_get_float (const gchar *key)
{
  g_return_val_if_fail (key != NULL, (float)0);

  return gconf_client_get_float (client, key, NULL);
}


void
gm_conf_set_string_list (const gchar *key,
			 GSList *l)
{
  g_return_if_fail (key != NULL);

  gconf_client_set_list (client, key, GCONF_VALUE_STRING, l, NULL);
}


GSList *
gm_conf_get_string_list (const gchar *key)
{
  g_return_val_if_fail (key != NULL, NULL);

  return gconf_client_get_list (client, key, GCONF_VALUE_STRING, NULL);
}


gchar *
gm_conf_escape_key (const gchar *key,
                    gint len)
{
  return gconf_escape_key (key, len);
}


gchar *
gm_conf_unescape_key (const gchar *key,
                      gint len)
{
  return gconf_unescape_key (key, len);
}


gboolean
gm_conf_is_key_writable (const gchar *key)
{
  g_return_val_if_fail (key != NULL, FALSE);

  return gconf_client_key_is_writable (client, key, NULL);
}


void
gm_conf_init ()
{
  client = gconf_client_get_default ();
  gconf_client_set_error_handling (client, GCONF_CLIENT_HANDLE_UNRETURNED);
}


void
gm_conf_shutdown ()
{
  g_object_unref (client);
}


void
gm_conf_save ()
{
  /* nothing needed */
}


GmConfEntryType
gm_conf_entry_get_type (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, GM_CONF_OTHER);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value == NULL)
    return GM_CONF_OTHER;

  switch (gconf_entry->value->type)
    {
    case GCONF_VALUE_BOOL:
      return GM_CONF_BOOL;
    case GCONF_VALUE_INT:
      return GM_CONF_INT;
    case GCONF_VALUE_STRING:
      return GM_CONF_STRING;
    case GCONF_VALUE_LIST:
      return GM_CONF_LIST;
    case GCONF_VALUE_INVALID:
    case GCONF_VALUE_FLOAT:
    case GCONF_VALUE_SCHEMA:
    case GCONF_VALUE_PAIR:
    default:
      return GM_CONF_OTHER;
    }
}


const gchar *
gm_conf_entry_get_key (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;
  return gconf_entry_get_key (gconf_entry);
}


gboolean
gm_conf_entry_get_bool (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, FALSE);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value)
    return gconf_value_get_bool (gconf_entry->value);

  return FALSE;
}


gint
gm_conf_entry_get_int (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, 0);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value)
    return gconf_value_get_int (gconf_entry->value);

  return 0;
}


const gchar *
gm_conf_entry_get_string (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;
  if (gconf_entry->value)
    return gconf_value_get_string (gconf_entry->value);

  return NULL;
}


GSList *
gm_conf_entry_get_list (GmConfEntry *entry)
{
  GConfEntry *gconf_entry = NULL;
  GSList *list = NULL;
  GSList *it = NULL;

  g_return_val_if_fail (entry != NULL, NULL);

  gconf_entry = (GConfEntry *)entry;

  if (gconf_entry->value)
    it = gconf_value_get_list (gconf_entry->value);
  while (it) {

    list = g_slist_append (list, g_strdup ((char *) gconf_value_get_string (it->data)));
    it = g_slist_next (it);
  }

  return list;
}


gpointer
gm_conf_notifier_add (const gchar *namespac,
		      GmConfNotifier func,
		      gpointer user_data)
{
  gpointer result;
  GConfNotifierWrap *wrapper = NULL;

  g_return_val_if_fail (namespac != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);

  wrapper = gconf_notifier_wrapper_new (func, user_data);
  return GUINT_TO_POINTER(gconf_client_notify_add (client, namespac,
						     gconf_notifier_wrapper_trigger,
						     wrapper,
						     gconf_notifier_wrapper_destroy, NULL));

  return result;
}


void
gm_conf_notifier_remove (gpointer identifier)
{
  g_return_if_fail (identifier != NULL);

  gconf_client_notify_remove (client, GPOINTER_TO_UINT (identifier));
}

void
gm_conf_notifier_trigger (const gchar *namespac)
{
  g_return_if_fail (namespac != NULL);

  gconf_client_notify (client, namespac);
}

void
gm_conf_watch ()
{
  gconf_client_add_dir (client, "/apps/" PACKAGE_NAME,
			GCONF_CLIENT_PRELOAD_NONE, NULL);
}


void
gm_conf_unwatch ()
{
  gconf_client_remove_dir (client, "/apps/" PACKAGE_NAME, NULL);
}


void
gm_conf_destroy (const gchar *namespac)
{
  g_return_if_fail (namespac != NULL);

  gconf_client_recursive_unset (client, namespac,
				(GConfUnsetFlags)0, NULL);
}
