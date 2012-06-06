
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
 *   begin                : Mar 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : API to access GnomeMeeting's configuration system
 *
 */


#ifndef __GM_CONF_H
#define __GM_CONF_H


#include <glib.h>

G_BEGIN_DECLS


/* 
 * here are the basic types used by the configuration system
 */
typedef enum {
  GM_CONF_OTHER, /* this basically means only 
		  * the specific implementation will know 
		  * what to do with that */
  GM_CONF_BOOL,   
  GM_CONF_INT,  
  GM_CONF_FLOAT,
  GM_CONF_STRING,
  GM_CONF_LIST,  /* list of strings! */
} GmConfEntryType;

/* this is implementation-dependant */
typedef struct _GmConfEntry GmConfEntry;

/* a notifier gets an identifier of itself, the conf entry
 * that triggered its call, and the user data that was given when
 * it was added */
typedef void (*GmConfNotifier) (gpointer identifier,
			        GmConfEntry *entry,
			        gpointer user_data);


/* the following two functions are mostly used at startup and shutdown:
 * - the configuration system may need some kind of initialization, and
 * receive some options from command-line at startup (this is the case of the
 * gconf implementation, for example);
 * - the configuration system may need to be asked to save at shutdown, in 
 * case it only saves things every now and then (this is the case of the glib
 * implementation, for example) */
void gm_conf_init (); /* don't try anything before! */
void gm_conf_shutdown (); /* don't try anything after! */
void gm_conf_save (); /* to forcibly save */

/* to accept/refuse that the notifiers get fired:
 * the configuration is still readable/writable, but
 * the changes aren't propagated to the gui */
void gm_conf_watch ();
void gm_conf_unwatch ();

/* to set/unset notifiers */
/* sets a notifier on namespac, calling func, with user_data*/
gpointer gm_conf_notifier_add (const gchar *namespac, 
			      GmConfNotifier func,
			      gpointer user_data);
void gm_conf_notifier_remove (gpointer identifier);
void gm_conf_notifier_trigger (const gchar *namespac);
 
/* the notifiers' callbacks get a GmConfEntry* as second argument,
 * and use the following set of functions to manipulate them */
GmConfEntryType gm_conf_entry_get_type (GmConfEntry *);
const gchar *gm_conf_entry_get_key (GmConfEntry *);
gboolean gm_conf_entry_get_bool (GmConfEntry *);
gint gm_conf_entry_get_int (GmConfEntry *);
const gchar *gm_conf_entry_get_string (GmConfEntry *);
/* Should be freed! */
GSList *gm_conf_entry_get_list (GmConfEntry *);

/* the following functions are used to get/set keys in the config */

/* all of those get a key name as first argument 
 * when the name matches "*_set_*", then the second argument
 * is the value to set, and will be copied */
void gm_conf_set_bool (const gchar *, const gboolean);
gboolean gm_conf_get_bool (const gchar *);
void gm_conf_set_int (const gchar *, const int);
int gm_conf_get_int (const gchar *);
void gm_conf_set_float (const gchar *, const float);
gfloat gm_conf_get_float (const gchar *);
void gm_conf_set_string (const gchar *, const gchar *);
/* Should be freed! */
gchar *gm_conf_get_string (const gchar *);
void gm_conf_set_string_list (const gchar *, GSList *);
/* Should be freed! */
GSList *gm_conf_get_string_list (const gchar *);

/* to destroy a part of the config */
void gm_conf_destroy (const gchar *namespac);

/* utility functions */
gboolean gm_conf_is_key_writable (const gchar *key);
gchar *gm_conf_escape_key (const gchar *key, gint len);
gchar *gm_conf_unescape_key (const gchar *key, gint len);

G_END_DECLS

#endif // __GM_CONF_H
