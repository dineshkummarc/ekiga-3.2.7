
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
 *                         dbus_helper.cpp  -  description
 *                         --------------------------
 *   begin                : Sat 29 Oct 2005
 *   copyright            : (C) 2005 by Julien Puydt
 *   description          : Implementation of a DBUS helper.
 *
 */

#include "config.h"

#include <dbus/dbus-glib.h>

/* Here is the description of method calls that this little program
 * manages for gnomemeeting:
 *
 * "GetVersion"
 * in    : nil
 * out   : array of uint (major, minor version of the dbus api)
 *
 * "GetSupportedProtocols"
 * in    : nil
 * out   : array of strings (supported protocols, example: "SIP" and "H.323")
 *
 */

#define GM_HELPER_SERVICE   "org.ekiga.Helper"
#define GM_HELPER_OBJECT  "/org/ekiga/Helper"
#define DBUS_COMPONENT_MAJOR_VERSION 0
#define DBUS_COMPONENT_MINOR_VERSION 1

/* Beginning of a classic GObject declaration */

typedef struct Helper Helper;
typedef struct HelperClass HelperClass;

GType helper_get_type (void);

struct Helper
{
  GObject parent;
};

struct HelperClass
{
  GObjectClass parent;
};

#define HELPER_TYPE (helper_get_type ())
#define HELPER(object) (G_TYPE_CHECK_INSTANCE_CAST ((object), HELPER_TYPE, Helper))
#define HELPER_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), HELPER_TYPE, HelperClass))
#define IS_HELPER(object) (G_TYPE_CHECK_INSTANCE_TYPE ((object), HELPER_TYPE))
#define IS_HELPER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), HELPER_TYPE))

G_DEFINE_TYPE(Helper, helper, G_TYPE_OBJECT);

/* End of a classic GObject declaration */

/* here is this GObject's api */
static gboolean helper_get_version (Helper *self,
				    GArray **version,
				    GError **error);
static gboolean helper_get_supported_protocols (Helper *self,
						char ***protocols,
						GError **error);

#include "dbus-helper-stub.h"

/* this function is the callback function for the quit timeout : we don't want
 * the helper to stay up too long
 */
static gboolean hara_kiri (gpointer loop);

/* implementations */
static void
helper_init (G_GNUC_UNUSED Helper *self)
{
  /* nothing to do */
}

static void
helper_class_init (G_GNUC_UNUSED HelperClass *klass)
{
  /* initializing as dbus object */
  dbus_g_object_type_install_info (HELPER_TYPE,
                                   &dbus_glib_helper_object_info);

}

static gboolean
helper_get_version (G_GNUC_UNUSED Helper *self,
		    GArray **version,
		    G_GNUC_UNUSED GError **error)
{
  guint val;

  *version = g_array_new (TRUE, TRUE, sizeof(guint));

  val = DBUS_COMPONENT_MAJOR_VERSION;
  g_array_append_val (*version, val);

  val = DBUS_COMPONENT_MINOR_VERSION;
  g_array_append_val (*version, val);

  return TRUE;
}

static gboolean
helper_get_supported_protocols (G_GNUC_UNUSED Helper *self,
				char ***protocols,
				G_GNUC_UNUSED GError **error)
{
  *protocols = g_new (char *, 3);
  (*protocols)[0] = g_strdup ("SIP");
  (*protocols)[1] = g_strdup ("H.323");
  (*protocols)[2] = NULL;

  return TRUE;
}

static gboolean
hara_kiri (gpointer data)
{
  GMainLoop *loop = (GMainLoop *)data;

  g_main_loop_quit (loop);

  return FALSE;
}

int
main (G_GNUC_UNUSED int argc,
      G_GNUC_UNUSED char *argv[])
{
  GMainLoop *mainloop = NULL;
  GObject *helper = NULL;
  DBusGConnection *bus = NULL;
  DBusGProxy *bus_proxy = NULL;
  guint request_name_result;
  GError *error = NULL;

  g_type_init ();

  mainloop = g_main_loop_new (NULL, FALSE);

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {

    g_error ("Couldn't connect to session bus : %s\n", error->message);
    g_main_loop_unref (mainloop);
    return -1;
  }

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
                                         "/org/freedesktop/DBus",
                                         "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
                          G_TYPE_STRING, GM_HELPER_SERVICE,
                          G_TYPE_UINT, DBUS_NAME_FLAG_DO_NOT_QUEUE,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &request_name_result,
                          G_TYPE_INVALID)) {

    g_error ("Couldn't request the name : %s\n",
             error->message);
    g_main_loop_unref (mainloop);
    return -1;
  }

  helper = G_OBJECT (g_object_new (HELPER_TYPE, NULL));

  dbus_g_connection_register_g_object (bus, GM_HELPER_OBJECT, helper);

#if GLIB_CHECK_VERSION (2,14,0)
  g_timeout_add_seconds (5, hara_kiri, mainloop);
#else
  g_timeout_add (5000, hara_kiri, mainloop);
#endif
  g_main_loop_run (mainloop);

  g_object_unref (helper);
  g_main_loop_unref (mainloop);

  return 0;
}
