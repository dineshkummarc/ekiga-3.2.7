
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
 *                         gmconf-glib.c  -  description
 *                         ------------------------------------------
 *   begin                : Mar 2004
 *   copyright            : (C) 2004 by Julien Puydt
 *   description          : glib implementation of gnomemeeting's
 *                          configuration system
 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "toolbox/toolbox.h"
#include "gmconf.h"

#ifdef WIN32
#include "platform/winpaths.h"
#endif


/* the following implementation of the gnomemeeting configuration system is
 * based on glib, to be more portable, and is hoped to make a port to unsane
 * systems easier.
 *
 * It features a simple xml-parser, to be able to load the gconf schema as
 * default configuration.
 *
 * It is organised in several layers, to be easier to study&improve:
 * - functions to manipulate individual entries;
 * - functions to manipulate the configuration database;
 * - functions to manipulate notifiers (set them, fire them, propagate them,
 * destroy them, ...);
 * - the implementation of gmconf.h's api.
 */

#define check_entry_type_return(entry,_type,val) G_STMT_START{ \
  if (G_LIKELY(entry != NULL && entry->type == _type)) \
    {} \
  else { \
    gchar *msg = NULL; \
    msg = g_strdup_printf ("gmconf key %s is of type " #_type, entry->key); \
    g_return_if_fail_warning (G_LOG_DOMAIN, __PRETTY_FUNCTION__, msg); \
    g_free (msg); \
    return val; \
  } \
}G_STMT_END

#define check_entry_for_key_return(entry,key,val) G_STMT_START{	\
  if (G_LIKELY(entry != NULL)) \
    {} \
  else { \
    gchar *msg = NULL; \
    msg = g_strdup_printf ("key %s has no corresponding entry! ", key); \
    g_return_if_fail_warning (G_LOG_DOMAIN, __PRETTY_FUNCTION__, msg); \
    g_free (msg); \
    return val; \
  } \
}G_STMT_END


/* the data types used in this file */

/* this is the main structure, in which all known entries are stored
 * we just store them as a list, with a boolean to know if we should trigger
 * the notifiers or not
 */
typedef struct _DataBase
{
  gboolean is_watched;
  GData *entries;
} DataBase;

/* for that implementation, a notifier is the function to call, together with
 * the associated user data
 */
typedef struct _Notifier
{
  GmConfNotifier func;
  gpointer data;
} Notifier;

/* that implementation uses the GM_CONF_OTHER type of entry (the one that is
 * free to use for the specific implementation) to store "redirections", which
 * is the scheme that is used to make notifications propagate to parents in the
 * namespace
 */
struct _GmConfEntry
{
  gchar *key;
  GmConfEntryType type;
  union {
    gboolean boolean;
    gint integer;
    gfloat floa;
    gchar *string;
    GSList *list;
    GmConfEntry *redirect; /* for GM_CONF_OTHER entries */
  } value;
  GSList *notifiers;
};

/* those data types are just for the loading of the gconf schema:
 * it uses the simple xml parsing code in glib to extract the needed
 * informations, and store it in the more crude way used here ; that has the
 * nice advantage that updating the gconf schema is enough to update the
 * no-gnome configuration system too!
 */
/* as the name says, those are the states of the parser ; let me just say:
 * 1) generally, the parser is in state KEY/TYPE/VALUE when it is expecting
 * a key/type/value ;
 * 2) the START state doesn't bear its name well: it is in fact used as the
 * base state of the parser, ie the one it is in at startup and when it's not
 * waiting for something.
 */
typedef enum {
  START,
  KEY,
  TYPE,
  VALUE
} SchParserState;

typedef struct _SchParser
{
  SchParserState state;
  DataBase *db;
  GmConfEntry *entry;
} SchParser;

/* this little structure is needed to get enough data through the
 * iterator when destroying a full namespace
 */
typedef struct _NamespcWrapper
{
  GData **datalist;
  const gchar *namespc;
} NamespcWrapper;

/* the following functions are used to make data manipulation easier
 * (and also give more readable code)
 */
static GSList *string_list_deep_copy (const GSList *);
static void string_list_deep_destroy (GSList *);

static gchar *string_from_bool (const gboolean);
static gchar *string_from_int (const gint);
static gchar *string_from_float (const gfloat);
static gchar *string_from_list (const GSList *);

static gboolean bool_from_string (const gchar *);
static gint int_from_string (const gchar *);
static gfloat float_from_string (const gchar *);
static GSList *list_from_string (const gchar *);

/* notifier functions */
static Notifier *notifier_new (const GmConfNotifier, const gpointer);
static void notifier_destroy (Notifier *);
static void notifier_destroy_in_list (gpointer elt, gpointer unused);
static void notifier_call_on_entry (Notifier *, GmConfEntry *);

/* entries functions */
static GmConfEntry *entry_new ();
static void entry_destroy (gpointer);

static const gchar *entry_get_key (const GmConfEntry *);
static void entry_set_key (GmConfEntry *, const gchar *);

static GmConfEntryType entry_get_type (const GmConfEntry *);
static void entry_set_type (GmConfEntry *, const GmConfEntryType);

static gboolean entry_get_bool (const GmConfEntry *);
static void entry_set_bool (GmConfEntry *, const gboolean);

static gint entry_get_int (const GmConfEntry *);
static void entry_set_int (GmConfEntry *, const gint);

static gfloat entry_get_float (const GmConfEntry *);
static void entry_set_float (GmConfEntry *, const gfloat);

static const gchar *entry_get_string (const GmConfEntry *);
static void entry_set_string (GmConfEntry *, const gchar *);

static GSList *entry_get_list (const GmConfEntry *);
static void entry_set_list (GmConfEntry *, GSList *);

static void entry_set_redirect (GmConfEntry *, GmConfEntry *);

static gboolean entry_call_notifiers_from_g_idle (gpointer);
static void entry_call_notifiers (const GmConfEntry *);
static gpointer entry_add_notifier (GmConfEntry *, GmConfNotifier, gpointer);
static void entry_remove_notifier (GmConfEntry *, gpointer);
static void entry_remove_notifier_in_list (GQuark, gpointer, gpointer);

/* database functions */
static DataBase *database_new ();
static DataBase *database_get_default ();
static void sch_parser_start_element (GMarkupParseContext *context,
				      const gchar *element_name,
				      const gchar **attribute_names,
				      const gchar **attribute_values,
				      gpointer data,
				      GError **error);
static void  sch_parser_end_element (GMarkupParseContext *context,
				     const gchar *element_name,
				     gpointer data,
				     GError **error);
static void sch_parser_characters (GMarkupParseContext *context,
				   const gchar *text,
				   gsize text_len,
				   gpointer data,
				   GError **error);

static const GMarkupParser sch_parser = {
  sch_parser_start_element,
  sch_parser_end_element,
  sch_parser_characters,
  NULL, /* passthrough */
  NULL /* error */
};

static gboolean database_load_file (DataBase *, const gchar *);
static void database_save_entry (GQuark quark, gpointer data,
				 gpointer user_data);
static gboolean database_save_file (DataBase *, const gchar *);
static void database_add_entry (DataBase *, GmConfEntry *);
static void database_remove_namespace_in_datalist (GQuark key_id,
						   gpointer data,
						   gpointer user_data);
static void database_remove_namespace (DataBase *, const gchar *);
static GmConfEntry *database_get_entry_for_key (DataBase *, const gchar *);
static GmConfEntry *database_get_entry_for_key_create (DataBase *,
						       const gchar *);

static void database_set_watched (DataBase *, const gboolean);
static void database_notify_on_namespace (DataBase *, const gchar *);

/*
 * Configuration file functions
 */
static gchar *gm_conf_get_user_conf_filename ();
static gboolean gm_conf_load_user_conf (DataBase *);
static gboolean gm_conf_load_sys_conf (DataBase *);


/* implementations of the data manipulation functions */

static GSList *
string_list_deep_copy (const GSList *orig)
{
  GSList *result = NULL;
  const GSList *ptr;
  for (ptr = orig; ptr != NULL; ptr = ptr->next)
    result = g_slist_append (result, g_strdup ((const gchar *)ptr->data));
  return result;
}

static void
string_list_deep_destroy (GSList *list)
{
  GSList *ptr = NULL;

  for (ptr = list; ptr != NULL; ptr = ptr->next)
    g_free ((gchar *)ptr->data);
  g_slist_free (list);
}


static gchar *
string_from_bool (const gboolean val)
{
  gchar *result = NULL;

  if (val == TRUE)
    result = g_strdup ("true");
  else
    result = g_strdup ("false");

  return result;
}

static gchar *
string_from_int (const gint val)
{
  gchar *result = NULL;
  GString *buffer = NULL;

  buffer = g_string_new (NULL);
  g_string_printf (buffer, "%d", val);
  result = g_string_free (buffer, FALSE);
  return result;
}

static gchar *
string_from_float (const gfloat val)
{
  gchar *result = NULL;
  GString *buffer = NULL;

  buffer = g_string_new (NULL);
  g_string_printf (buffer, "%f", val);

  result = g_string_free (buffer, FALSE);
  return result;
}

static gchar *
string_from_list (const GSList *val)
{
  gchar *result = NULL;
  GString *buffer = NULL;
  gchar *str = NULL;
  int i = 0;
  const GSList *ptr = NULL;

  buffer = g_string_new (NULL);
  for (ptr = val; ptr != NULL; ptr = ptr->next) {
    str = (gchar *)ptr->data;
    for (i = 0; str[i] != 0; i++) {
      if (str[i] == ',')
	g_string_append (buffer, "\\,");
      else
	g_string_append_c (buffer, str[i]);
    }

    if (ptr->next != NULL)
      g_string_append_c (buffer, ',');
  }
  g_string_prepend_c (buffer, '[');
  g_string_append_c (buffer, ']');
  result = g_string_free (buffer, FALSE);
  return result;
}

static gboolean
bool_from_string (const gchar *str)
{
  gboolean result;

  g_return_val_if_fail (str != NULL, FALSE);

  switch (str[0]) {
  case '0':
  case 'f':
  case 'F':
    result = FALSE;
    break;
  default:
    result = TRUE;
  }
  return result;
}

static gint
int_from_string (const gchar *str)
{
  gint result;

  g_return_val_if_fail (str != NULL, 0);

  result = strtol (str, NULL, 10);
  return result;
}

static gfloat
float_from_string (const gchar *str)
{
  gfloat result;
  int err;

  g_return_val_if_fail (str != NULL, 0);

  err = sscanf (str, "%f", &result);
  return result;
}

static GSList *
list_from_string (const gchar *str)
{
  GSList *result = NULL;
  gchar *txt = NULL;
  gchar **tmp_list = NULL;
  gchar **txt_list = NULL;
  GString *item = NULL;
  gboolean must_concat = FALSE;

  g_return_val_if_fail (str != NULL, NULL);
  g_return_val_if_fail (str[0] == '[', NULL);
  g_return_val_if_fail (str[strlen (str) - 1] == ']', NULL);

  txt = g_strdup (str + 1); /* get the '[' out of the way */
  txt[strlen (txt) - 1] = 0; /* get the ']' out of the way */


  if (txt[0] == 0) { /* handle the empty list */
    g_free (txt);
    return NULL;
  }

  txt_list = g_strsplit (txt, ",", 0);
  g_free (txt);
  for (tmp_list = txt_list; *tmp_list != NULL; tmp_list++) {

    if (item == NULL)
      item = g_string_new ("");

    if ((*tmp_list)[strlen (*tmp_list) - 1] == '\\') {

      g_string_append_len (item, *tmp_list,
			   strlen (*tmp_list) - 1);
      must_concat = TRUE;
    } else {

      g_string_append_len (item, *tmp_list,
			   strlen (*tmp_list));
      must_concat = FALSE;
    }

    if (must_concat == FALSE) {

      result = g_slist_append (result, item->str);
      g_string_free (item, FALSE);
      item = NULL;

    } else
      g_string_append_c (item, ',');
  }

  g_strfreev (txt_list);

  if (must_concat == TRUE) { /* bad: shouldn't happen */
    /* try not to lose data... */
    result = g_slist_append (result, item->str);
    g_string_free (item, FALSE);
    item = NULL;
  }


  return result;
}

/* implementation of notifier functions */
static Notifier *
notifier_new (const GmConfNotifier func,
	      const gpointer data)
{
  Notifier *notifier = NULL;

  g_return_val_if_fail (func != NULL, NULL);

  notifier = g_new (Notifier, 1);
  notifier->func = func;
  notifier->data = data;
  return notifier;
}

static void
notifier_destroy (Notifier *notifier)
{
  g_free (notifier);
}

static void
notifier_destroy_in_list (gpointer elt,
			  G_GNUC_UNUSED gpointer unused)
{
  notifier_destroy ((Notifier *)elt);
}

static void
notifier_call_on_entry (Notifier *notifier,
			GmConfEntry *entry)
{
  g_return_if_fail (notifier != NULL);
  g_return_if_fail (entry != NULL);

  notifier->func (entry, entry, notifier->data);
}

/* implementation of entries functions */
static GmConfEntry *
entry_new ()
{
  GmConfEntry *entry = NULL;

  entry = g_new (GmConfEntry, 1);
  entry->key = NULL;
  entry->type = GM_CONF_OTHER;
  entry->value.redirect = NULL;
  entry->notifiers = NULL;
  return entry;
}

static void
entry_destroy (gpointer ent)
{
  GmConfEntry *entry = NULL;

  g_return_if_fail (ent != NULL);

  (void)g_idle_remove_by_data (ent);

  entry = (GmConfEntry *)ent;

  g_free (entry->key);

  switch (entry->type) {
  case GM_CONF_OTHER:
    break;
  case GM_CONF_BOOL:
    break;
  case GM_CONF_INT:
    break;
  case GM_CONF_FLOAT:
    break;
  case GM_CONF_STRING:
    g_free (entry->value.string);
    break;
  case GM_CONF_LIST:
    string_list_deep_destroy (entry->value.list);
    break;
  default:
    break;
  }

  g_slist_foreach (entry->notifiers, notifier_destroy_in_list, NULL);
  g_slist_free (entry->notifiers);

};

static const gchar *
entry_get_key (const GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, NULL);

  return entry->key;
}

static void
entry_set_key (GmConfEntry *entry,
	       const gchar *key)
{
  g_return_if_fail (entry != NULL);
  g_return_if_fail (key != NULL);

  if (entry->key)
    g_free (entry->key);
  entry->key = g_strdup (key);
}

static GmConfEntryType
entry_get_type (const GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, GM_CONF_OTHER);

  return entry->type;
}

static void
entry_set_type (GmConfEntry *entry,
		const GmConfEntryType type)
{
  g_return_if_fail (entry != NULL);

  entry->type = type;
}

static gboolean
entry_get_bool (const GmConfEntry *entry)
{
  check_entry_type_return (entry, GM_CONF_BOOL, FALSE);

  return entry->value.boolean;
}

static void
entry_set_bool (GmConfEntry *entry,
		const gboolean val)
{
  g_return_if_fail (entry != NULL);

  entry->type = GM_CONF_BOOL;
  entry->value.boolean = val;
}

static gint
entry_get_int (const GmConfEntry *entry)
{
  check_entry_type_return (entry, GM_CONF_INT, 0);

  return entry->value.integer;
}

static void
entry_set_int (GmConfEntry *entry,
	       const gint val)
{
  g_return_if_fail (entry != NULL);

  entry->type = GM_CONF_INT;
  entry->value.boolean = val;
}

static gfloat
entry_get_float (const GmConfEntry *entry)
{
  check_entry_type_return (entry, GM_CONF_FLOAT, 0);

  return entry->value.floa;
}

static void
entry_set_float (GmConfEntry *entry,
		 const gfloat val)
{
  g_return_if_fail (entry != NULL);

  entry->type = GM_CONF_FLOAT;
  entry->value.floa = val;
}

static const gchar *
entry_get_string (const GmConfEntry *entry)
{
  check_entry_type_return (entry, GM_CONF_STRING, NULL);

  return entry->value.string;
}

static void
entry_set_string (GmConfEntry *entry, const gchar *val)
{
  g_return_if_fail (entry != NULL);

  entry->type = GM_CONF_STRING;
  entry->value.string = g_strdup (val);
}

static GSList *
entry_get_list (const GmConfEntry *entry)
{
  check_entry_type_return (entry, GM_CONF_LIST, NULL);

  return entry->value.list;
}

static void
entry_set_list (GmConfEntry *entry,
		GSList *val)
{
  g_return_if_fail (entry != NULL);

  entry->type = GM_CONF_LIST;
  entry->value.list = string_list_deep_copy (val);
}

static void
entry_set_redirect (GmConfEntry *entry,
		    GmConfEntry *redirect)
{
  g_return_if_fail (entry != NULL);

  entry->type = GM_CONF_OTHER;
  entry->value.redirect = redirect;
}

static gboolean
entry_call_notifiers_from_g_idle (gpointer data)
{
  GmConfEntry *entry = NULL;
  GSList *ptr = NULL;
  Notifier *notif = NULL;

  /* no check on data: done in entry_call_notifiers */

  entry = (GmConfEntry *)data;
  for (ptr = entry->notifiers; ptr != NULL; ptr = ptr->next) {
    notif = (Notifier *)ptr->data;
    if (entry->type == GM_CONF_OTHER && entry->value.redirect != NULL)
      notifier_call_on_entry (notif, entry->value.redirect);
    else
      notifier_call_on_entry (notif, entry);
  }
  return FALSE;
}

static void
entry_call_notifiers (const GmConfEntry *entry)
{
  g_return_if_fail (entry != NULL);

  if (entry->notifiers != NULL)
    g_idle_add (entry_call_notifiers_from_g_idle, (gpointer)entry);
}

static gpointer
entry_add_notifier (GmConfEntry *entry,
		    GmConfNotifier func,
		    gpointer data)
{
  Notifier *notif = NULL;

  g_return_val_if_fail (entry != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);

  notif = notifier_new (func, data);
  entry->notifiers = g_slist_prepend (entry->notifiers, notif);
  return (gpointer)notif;
}


static void
entry_remove_notifier (GmConfEntry *entry,
		       gpointer identifier)
{
  /* no check, since the only function calling here is
     entry_remove_notifier_in_list */

  entry->notifiers = g_slist_remove (entry->notifiers, identifier);
}

static void
entry_remove_notifier_in_list (G_GNUC_UNUSED GQuark unused,
			       gpointer entry,
			       gpointer identifier)
{
  g_return_if_fail (entry != NULL);
  g_return_if_fail (identifier != NULL);

  entry_remove_notifier ((GmConfEntry *)entry, identifier);
}

/* implementation of the database functions */
static DataBase *
database_new ()
{
  DataBase *db = NULL;

  db = g_new (DataBase, 1);
  db->is_watched = FALSE;
  db->entries = NULL;
  g_datalist_init (&db->entries);
  return db;
}

static void
database_destroy (DataBase *db)
{
  g_datalist_clear (&db->entries);
  g_free (db);
}

static DataBase *
database_get_default ()
{
  static DataBase *db = NULL;
  if (db == NULL)
    db = database_new ();
  return db;
}

static void
sch_parser_start_element (G_GNUC_UNUSED GMarkupParseContext *context,
			  const gchar *element_name,
			  G_GNUC_UNUSED const gchar **attribute_names,
			  G_GNUC_UNUSED const gchar **attribute_values,
			  gpointer data,
			  G_GNUC_UNUSED GError **error)
{
  SchParser *parser = NULL;

  g_return_if_fail (data != NULL);

  parser = (SchParser *)data;
  parser->state = START; /* default */
  if (strcmp (element_name, "schema") == 0)
    parser->entry = entry_new ();
  else if (strcmp (element_name, "applyto") == 0)
    parser->state = KEY;
  else if (strcmp (element_name, "type") == 0)
    parser->state = TYPE;
  else if (strcmp (element_name, "default") == 0)
    parser->state = VALUE;
}

static void
sch_parser_end_element (G_GNUC_UNUSED GMarkupParseContext *context,
			const gchar *element_name,
			gpointer data,
			G_GNUC_UNUSED GError **error)
{
  SchParser *parser = NULL;

  g_return_if_fail (data != NULL);

  parser = (SchParser *)data;

  if (strcmp (element_name, "schema") == 0) {
    database_add_entry (parser->db, parser->entry);
    parser->entry = NULL;
  }

  /* in any case: */
  parser->state = START;
}

static void
sch_parser_characters (G_GNUC_UNUSED GMarkupParseContext *context,
		       const gchar *text,
		       G_GNUC_UNUSED gsize text_len,
		       gpointer data,
		       G_GNUC_UNUSED GError **error)
{
  SchParser *parser = NULL;

  g_return_if_fail (data != NULL);

  parser = (SchParser *)data;

  switch (parser->state) {

  case  START:
    /* we're not interested in that data: just eat it! */
    break;
  case KEY:
    entry_set_key (parser->entry, text);
    break;
  case TYPE:
    if (strcmp (text, "bool") == 0)
      entry_set_type (parser->entry, GM_CONF_BOOL);
    else if (strcmp (text, "int") == 0)
      entry_set_type (parser->entry, GM_CONF_INT);
    else if (strcmp (text, "float") == 0)
      entry_set_type (parser->entry, GM_CONF_FLOAT);
    else if (strcmp (text, "string") == 0)
      entry_set_type (parser->entry, GM_CONF_STRING);
    else if (strcmp (text, "list") == 0)
      entry_set_type (parser->entry, GM_CONF_LIST);
    else
      entry_set_type (parser->entry, GM_CONF_OTHER);
    break;
  case VALUE:
    switch (entry_get_type (parser->entry)) {
    case GM_CONF_BOOL:
      entry_set_bool (parser->entry, bool_from_string (text));
      break;
    case GM_CONF_INT:
      entry_set_int (parser->entry, int_from_string (text));
      break;
    case GM_CONF_FLOAT:
      entry_set_float (parser->entry, float_from_string (text));
      break;
    case GM_CONF_STRING:
      entry_set_string (parser->entry, text);
      break;
    case GM_CONF_LIST:
      entry_set_list (parser->entry, list_from_string (text));
      break;
    case GM_CONF_OTHER: /* shouldn't happen */
      break;
    default:
      break;
    }

  default:
    ;/* nothing */
  }
}

static gboolean
database_load_file (DataBase *db,
		    const gchar *filename)
{
  SchParser *parser = NULL;
  GMarkupParseContext *context = NULL;
  GIOChannel *io = NULL;
  GIOStatus status;
  gchar buffer[4096];
  gsize len = 0;

  g_return_val_if_fail (db != NULL, FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  io = g_io_channel_new_file (filename, "r", NULL);
  if (!io)
    return FALSE;
  parser = g_new (SchParser, 1);
  parser->state = START;
  parser->db = db;
  parser->entry = NULL;
  context = g_markup_parse_context_new (&sch_parser, 0,
					(gpointer)parser, g_free);
  g_io_channel_set_encoding (io, "UTF-8", NULL); /* useful? */
  while (TRUE)
    {
      status = g_io_channel_read_chars (io, buffer, sizeof (buffer),
					&len, NULL);
      switch (status)
        {
        case G_IO_STATUS_ERROR:
	  g_io_channel_unref (io);
          return FALSE;
        case G_IO_STATUS_EOF:
	  g_io_channel_unref (io);
          return g_markup_parse_context_end_parse (context, NULL);
        case G_IO_STATUS_NORMAL:
        case G_IO_STATUS_AGAIN:
	default:
          break;
        }
      if (!g_markup_parse_context_parse (context, buffer, len, NULL))
	{
	  g_io_channel_unref (io);
	  return FALSE;
	}
    }
  /* does it ever come here? */
  g_io_channel_unref (io);
  return TRUE;
}

static void
database_save_entry (G_GNUC_UNUSED GQuark quark,
		     gpointer data,
		     gpointer user_data)
{
  GmConfEntry *entry = NULL;
  GIOChannel *io = NULL;
  gchar *value = NULL;
  const gchar *txt = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (user_data != NULL);

  entry = (GmConfEntry *)data;
  io = (GIOChannel *)user_data;
  g_io_channel_write_chars (io, "<schema>\n", -1, NULL, NULL);

  g_io_channel_write_chars (io, "<applyto>", -1, NULL, NULL);
  g_io_channel_write_chars (io, entry_get_key (entry), -1, NULL, NULL);
  g_io_channel_write_chars (io, "</applyto>\n", -1, NULL, NULL);

  g_io_channel_write_chars (io, "<type>", -1, NULL, NULL);
  switch (entry_get_type (entry)) {
  case GM_CONF_OTHER:
    g_io_channel_write_chars (io, "other", -1, NULL, NULL);
    break;
  case GM_CONF_BOOL:
    g_io_channel_write_chars (io, "bool", -1, NULL, NULL);
    break;
  case GM_CONF_INT:
    g_io_channel_write_chars (io, "int", -1, NULL, NULL);
    break;
  case GM_CONF_FLOAT:
    g_io_channel_write_chars (io, "float", -1, NULL, NULL);
    break;
  case GM_CONF_STRING:
    g_io_channel_write_chars (io, "string", -1, NULL, NULL);
    break;
  case GM_CONF_LIST:
    g_io_channel_write_chars (io, "list", -1, NULL, NULL);
    break;
  default:
    g_io_channel_write_chars (io, "unknown", -1, NULL, NULL);
    break;
  }
  g_io_channel_write_chars (io, "</type>\n", -1, NULL, NULL);

  g_io_channel_write_chars (io, "<default>", -1, NULL, NULL);
  switch (entry_get_type (entry)) {
  case GM_CONF_OTHER:
    value = g_strdup ("none");
    break;
  case GM_CONF_BOOL:
    value = string_from_bool (entry_get_bool (entry));
    break;
  case GM_CONF_INT:
    value = string_from_int (entry_get_int (entry));
    break;
  case GM_CONF_FLOAT:
    value = string_from_float (entry_get_float (entry));
    break;
  case GM_CONF_STRING:
    txt = entry_get_string (entry);
    if (txt != NULL)
      value = g_markup_escape_text (txt, -1);
    else
      value = g_strdup ("");
    break;
  case GM_CONF_LIST:
    value = string_from_list (entry_get_list (entry));
    break;
  default:
    g_warning ("Aie!");
    value = g_strdup ("unknown");
    break;
  }
  g_io_channel_write_chars (io, value, -1, NULL, NULL);
  g_free (value);
  g_io_channel_write_chars (io, "</default>\n", -1, NULL, NULL);

  g_io_channel_write_chars (io, "</schema>\n", -1, NULL, NULL);
}

static gboolean
database_save_file (DataBase *db,
		    const gchar *filename)
{
  GIOChannel *io = NULL;
  gchar *dirname = NULL;

  g_return_val_if_fail (db != NULL, FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  dirname = g_path_get_dirname (filename);
  if (!g_file_test (dirname, G_FILE_TEST_IS_DIR)) {
    if (!g_mkdir_with_parents (dirname, S_IRWXU))
      g_warning ("Unable to create directory %s\n", dirname);
  }
  g_free (dirname);

  io = g_io_channel_new_file (filename, "w", NULL);

  if (io) {

    g_datalist_foreach (&db->entries, database_save_entry, io);
    g_io_channel_unref (io);
  }
  else
    g_warning ("Couldn't save conf database in %s\n", filename);

  return TRUE;
}

static void
database_add_entry (DataBase *db,
		    GmConfEntry *entry)
{
  g_return_if_fail (db != NULL);
  g_return_if_fail (entry != NULL);

  g_datalist_set_data_full (&db->entries, entry_get_key (entry),
			    entry, entry_destroy);
}

static void
database_remove_namespace_in_datalist (G_GNUC_UNUSED GQuark key_id,
				       gpointer data,
				       gpointer user_data)
{
  GmConfEntry *entry = NULL;
  NamespcWrapper *wrapper = NULL;
  const gchar *key = NULL;

  g_return_if_fail (data != NULL);
  g_return_if_fail (user_data != NULL);

  entry = (GmConfEntry *)data;
  wrapper = (NamespcWrapper *)user_data;
  key = entry_get_key (entry);

  if (g_str_has_prefix (key, wrapper->namespc))
    g_datalist_remove_data (wrapper->datalist, key);

}

static void
database_remove_namespace (DataBase *db,
			   const gchar *namespc)
{
  NamespcWrapper *wrapper = NULL;

  g_return_if_fail (db != NULL);
  g_return_if_fail (namespc != NULL);

  wrapper = g_new (NamespcWrapper, 1);
  wrapper->datalist = &db->entries;
  wrapper->namespc = namespc;
  g_datalist_foreach (&db->entries,
		      database_remove_namespace_in_datalist, wrapper);
  g_free (wrapper);
}

static GmConfEntry *
database_get_entry_for_key (DataBase *db,
			    const gchar *key)
{
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (db != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);

  entry = g_datalist_get_data (&db->entries, key);

  return entry;
}

static GmConfEntry *
database_get_entry_for_key_create (DataBase *db,
				   const gchar *key)
{
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (db != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);

  entry = database_get_entry_for_key (db, key);
  if (entry == NULL) {
    entry = entry_new (db);
    entry_set_key (entry, key);
    database_add_entry (db, entry);
  }
  return entry;
}

static void
database_set_watched (DataBase *db,
		      const gboolean bool)
{
  g_return_if_fail (db != NULL);

  db->is_watched = bool;
}

static void
database_notify_on_namespace (DataBase *db,
			      const gchar *namespac)
{
  GmConfEntry *parent_entry = NULL, *entry = NULL;
  gchar *key = NULL;


  g_return_if_fail (db != NULL);
  g_return_if_fail (namespac != NULL);
  g_return_if_fail (namespac[0] == '/'); /* that makes the loop work! */

  entry = database_get_entry_for_key (db, namespac);

  g_return_if_fail (entry != NULL);

  if (db->is_watched == FALSE)
    return;

  for (key = g_strdup (namespac);
       key[0] != 0;
       g_strrstr (key, "/")[0] = 0) {
    parent_entry = database_get_entry_for_key (db, key);
    if (parent_entry != NULL) {
      if (entry_get_type (parent_entry) == GM_CONF_OTHER)
	entry_set_redirect (parent_entry, entry);
      entry_call_notifiers (parent_entry);
      if (entry_get_type (entry) == GM_CONF_OTHER)
	entry_set_redirect (parent_entry, NULL);
    }
  }
  g_free (key);
}


static gchar *
gm_conf_get_user_conf_filename ()
{
  return g_build_filename (g_get_user_config_dir (),
			   "ekiga.conf",
			   NULL);
}


static gboolean
gm_conf_load_user_conf (DataBase *db)
{
  gchar *filename = NULL;
  gboolean result = FALSE;

  g_return_val_if_fail (db != NULL, FALSE);

  filename = gm_conf_get_user_conf_filename ();
  result = database_load_file (db, filename);

  if (G_LIKELY (result))
    {}
  else
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
	   "couldn't read the user configuration in %s", filename);

  g_free (filename);

  return result;
}


static gboolean
gm_conf_load_sys_conf (DataBase *db)
{
  gchar *filename = NULL;
  gboolean result = FALSE;

  g_return_val_if_fail (db != NULL, FALSE);

  filename = g_build_filename (SYSCONFDIR, "ekiga",
			       "ekiga.schemas", NULL);
  result = database_load_file (db, filename);

  if (G_LIKELY (result))
    {}
  else
    g_log (G_LOG_DOMAIN, G_LOG_LEVEL_WARNING,
	   "couldn't read the system configuration in %s", filename);
  g_free (filename);

  return result;
}


/* last but not least, the implementation of the gmconf.h api */
static gboolean
saveconf_timer_callback (G_GNUC_UNUSED gpointer unused)
{
  DataBase *db = database_get_default ();
  gchar *user_conf = NULL;

  user_conf = gm_conf_get_user_conf_filename ();
  database_save_file (db, user_conf);

  g_free (user_conf);

  return TRUE;
}

void
gm_conf_init ()
{
  gboolean result = FALSE;
  DataBase *db = database_get_default ();

  result = gm_conf_load_sys_conf (db);
  result = (gm_conf_load_user_conf (db) || result);
  if (!result)
    g_warning ("Couldn't load system configuration");

  /* those keys aren't found in gnomemeeting's schema */
  gm_conf_set_bool ("/desktop/gnome/interface/menus_have_icons", TRUE);

  /* automatic savings */
#if GLIB_CHECK_VERSION (2, 14, 0)
  g_timeout_add_seconds (5, (GSourceFunc)saveconf_timer_callback, NULL);
#else
  g_timeout_add (5000, (GSourceFunc)saveconf_timer_callback, NULL);
#endif

}


void
gm_conf_shutdown ()
{
  /* a crash is sure to happen if anyone uses gmconf after this... */
  DataBase *db = database_get_default ();
  database_destroy (db);
}


void
gm_conf_save ()
{
  DataBase *db = database_get_default ();
  gchar *user_conf = NULL;

  user_conf = gm_conf_get_user_conf_filename ();

  database_save_file (db, user_conf);

  g_free (user_conf);
}

void
gm_conf_watch ()
{
  DataBase *db = database_get_default ();

  database_set_watched (db, TRUE);
}

void
gm_conf_unwatch ()
{
  DataBase *db = database_get_default ();

  database_set_watched (db, FALSE);
}

gpointer
gm_conf_notifier_add (const gchar *namespac,
		      GmConfNotifier func,
		      gpointer user_data)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (namespac != NULL, NULL);
  g_return_val_if_fail (func != NULL, NULL);

  entry = database_get_entry_for_key_create (db, namespac);

  return entry_add_notifier (entry, func, user_data);
}

void
gm_conf_notifier_remove (gpointer identifier)
{
  DataBase *db = database_get_default ();

  g_return_if_fail (identifier != NULL);

  g_datalist_foreach (&db->entries, entry_remove_notifier_in_list, identifier);

  notifier_destroy (identifier);
}

void
gm_conf_notifier_trigger (const gchar *namespac)
{
  DataBase *db = database_get_default ();

  g_return_if_fail (namespac != NULL);

  database_notify_on_namespace (db, namespac);
}

GmConfEntryType
gm_conf_entry_get_type (GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, GM_CONF_OTHER);

  return entry_get_type (entry);
}

const gchar *
gm_conf_entry_get_key (GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, NULL);

  return entry_get_key (entry);
}

gboolean
gm_conf_entry_get_bool (GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, FALSE);

  return entry_get_bool (entry);
}

gint
gm_conf_entry_get_int (GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, 0);

  return entry_get_int (entry);
}

const gchar *
gm_conf_entry_get_string (GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, NULL);

  return entry_get_string (entry);
}

GSList *
gm_conf_entry_get_list (GmConfEntry *entry)
{
  g_return_val_if_fail (entry != NULL, NULL);

  return string_list_deep_copy (entry_get_list (entry));
}

void
gm_conf_set_bool (const gchar *key,
		  const gboolean val)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_if_fail (key != NULL);

  entry = database_get_entry_for_key_create (db, key);

  g_return_if_fail (entry != NULL);

  entry_set_bool (entry, val);
  database_notify_on_namespace (db, entry_get_key (entry));
}

gboolean
gm_conf_get_bool (const gchar *key)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (key != NULL, FALSE);

  entry = database_get_entry_for_key (db, key);

  check_entry_for_key_return (entry, key, FALSE);

  return entry_get_bool (entry);
}

void
gm_conf_set_int (const gchar *key,
		 const int val)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_if_fail (key != NULL);

  entry = database_get_entry_for_key_create (db, key);

  g_return_if_fail (entry != NULL);

  entry_set_int (entry, val);
  database_notify_on_namespace (db, entry_get_key (entry));
}

int
gm_conf_get_int (const gchar *key)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (key != NULL, 0);

  entry = database_get_entry_for_key (db, key);

  check_entry_for_key_return (entry, key, 0);

  return entry_get_int (entry);
}

void
gm_conf_set_float (const gchar *key,
		   const float val)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_if_fail (key != NULL);

  entry = database_get_entry_for_key_create (db, key);

  g_return_if_fail (entry != NULL);

  entry_set_float (entry, val);
  database_notify_on_namespace (db, entry_get_key (entry));
}

gfloat
gm_conf_get_float (const gchar *key)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (key != NULL, 0);

  entry = database_get_entry_for_key (db, key);

  check_entry_for_key_return (entry, key, 0);

  return entry_get_float (entry);
}

void
gm_conf_set_string (const gchar *key,
		    const gchar *val)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_if_fail (key != NULL);

  entry = database_get_entry_for_key_create (db, key);

  g_return_if_fail (entry != NULL);

  entry_set_string (entry, val);
  database_notify_on_namespace (db, entry_get_key (entry));
}

gchar *
gm_conf_get_string (const gchar *key)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (key != NULL, NULL);

  entry = database_get_entry_for_key (db, key);

  check_entry_for_key_return (entry, key, NULL);

  return g_strdup (entry_get_string (entry));
}

void
gm_conf_set_string_list (const gchar *key,
			 GSList *val)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_if_fail (key != NULL);

  entry = database_get_entry_for_key_create (db, key);

  g_return_if_fail (entry != NULL);

  entry_set_list (entry, val);
  database_notify_on_namespace (db, entry_get_key (entry));
}

GSList *
gm_conf_get_string_list (const gchar *key)
{
  DataBase *db = database_get_default ();
  GmConfEntry *entry = NULL;

  g_return_val_if_fail (key != NULL, NULL);

  entry = database_get_entry_for_key (db, key);

  check_entry_for_key_return (entry, key, NULL);

  return string_list_deep_copy (entry_get_list (entry));
}

void
gm_conf_destroy (const gchar *namespac)
{
  DataBase *db = database_get_default ();

  g_return_if_fail (namespac != NULL);

  database_remove_namespace (db, namespac);
}

gboolean
gm_conf_is_key_writable (const gchar *key)
{
  g_return_val_if_fail (key != NULL, FALSE);

  return TRUE;
}

gchar *
gm_conf_escape_key (const gchar *key,
		    G_GNUC_UNUSED gint len)
{
  return g_strescape (key, NULL); /* we don't honor len */
}

gchar *
gm_conf_unescape_key (const gchar *key,
		      G_GNUC_UNUSED gint len)
{
  return g_strcompress (key); /* we don't honor len */
}
