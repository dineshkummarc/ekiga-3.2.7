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
 *                         dbus_component.cpp  -  description
 *                         -----------------------------
 *   begin                : Tue Nov 1  2005
 *   copyright            : (c) 2005 by Julien Puydt
 *                          (c) 2007 by Damien Sandras
 *                          (c) 2008 by Steve Frécinaux
 *   description          : This files contains the implementation of the DBUS
 *                          interface of ekiga.
 *
 */

#include "config.h"

#include <dbus/dbus-glib.h>

#include "dbus.h"

#include "ekiga.h"
#include "gmmarshallers.h"
#include "gmconf.h"
#include "callbacks.h"
#include "misc.h"
#include "accounts.h"

#include "call-core.h"

/* Those defines the namespace and path we want to use. */
#define EKIGA_DBUS_NAMESPACE "org.ekiga.Ekiga"
#define EKIGA_DBUS_PATH      "/org/ekiga/Ekiga"
#define EKIGA_DBUS_INTERFACE "org.ekiga.Ekiga"

G_DEFINE_TYPE(EkigaDBusComponent, ekiga_dbus_component, G_TYPE_OBJECT);

struct _EkigaDBusComponentPrivate
{
  Ekiga::ServiceCore *core;
};

/**************************
 * GOBJECT / DBUS METHODS *
 **************************/

static gboolean ekiga_dbus_component_show (EkigaDBusComponent *self,
                                           GError **error);
static gboolean ekiga_dbus_component_shutdown (EkigaDBusComponent *self,
                                               GError **error);
static gboolean ekiga_dbus_component_call (EkigaDBusComponent *self,
                                           const gchar *uri,
                                           GError **error);
static gboolean ekiga_dbus_component_get_user_name (EkigaDBusComponent *self,
                                                    char **name,
                                                    GError **error);
static gboolean ekiga_dbus_component_get_user_location (EkigaDBusComponent *self,
                                                        char **location,
                                                        GError **error);
static gboolean ekiga_dbus_component_get_user_comment (EkigaDBusComponent *self,
                                                       char **comment,
                                                       GError **error);

/* get the code to make the GObject accessible through dbus
 * (this is especially where we get dbus_glib_dbus_component_object_info !)
 */
#include "dbus-stub.h"

static void
ekiga_dbus_component_init (EkigaDBusComponent *self)
{
  self->priv = G_TYPE_INSTANCE_GET_PRIVATE (self, EKIGA_TYPE_DBUS_COMPONENT,
                                            EkigaDBusComponentPrivate);
}

static void
ekiga_dbus_component_class_init (EkigaDBusComponentClass *klass)
{
  g_type_class_add_private (klass, sizeof (EkigaDBusComponentPrivate));

  /* initializing as dbus object */
  dbus_g_object_type_install_info (G_TYPE_FROM_CLASS (klass),
                                   &dbus_glib_ekiga_dbus_component_object_info);
}

static gboolean
ekiga_dbus_component_show (G_GNUC_UNUSED EkigaDBusComponent *self,
                           G_GNUC_UNUSED GError **error)
{
  PTRACE (1, "DBus\tShow");

  GtkWidget *window = GnomeMeeting::Process ()->GetMainWindow ();
#if GTK_CHECK_VERSION(2,18,0)
  if (gtk_widget_get_visible (GTK_WIDGET (window)))
#else
  if (GTK_WIDGET_VISIBLE (window))
#endif
    gtk_window_set_urgency_hint (GTK_WINDOW (window), TRUE);
  else
    gtk_window_present (GTK_WINDOW (window));

  return TRUE;
}

static gboolean
ekiga_dbus_component_shutdown (G_GNUC_UNUSED EkigaDBusComponent *self,
                               G_GNUC_UNUSED GError **error)
{
  quit_callback (NULL, NULL);

  return TRUE;
}

static gboolean
ekiga_dbus_component_call (EkigaDBusComponent *self,
                           const gchar *uri,
                           G_GNUC_UNUSED GError **error)
{
  gmref_ptr<Ekiga::CallCore> call_core
    = self->priv->core->get ("call-core");
  call_core->dial (uri);

  return TRUE;
}

static gboolean
ekiga_dbus_component_get_user_name (G_GNUC_UNUSED EkigaDBusComponent *self,
                                    char **name,
                                    G_GNUC_UNUSED GError **error)
{
  gchar * full_name;
  PTRACE (1, "DBus\tGetName");

  full_name = gm_conf_get_string (PERSONAL_DATA_KEY "full_name");
  if (full_name)
    *name = full_name;

  /* not freeing the full name is not a leak : dbus will do it for us ! */

  return TRUE;
}

static gboolean
ekiga_dbus_component_get_user_location (G_GNUC_UNUSED EkigaDBusComponent *self,
                                        char **location,
                                        G_GNUC_UNUSED GError **error)
{
  PTRACE (1, "DBus\tGetLocation");

  *location = gm_conf_get_string (PERSONAL_DATA_KEY "location");

  return TRUE;
}

static gboolean
ekiga_dbus_component_get_user_comment (G_GNUC_UNUSED EkigaDBusComponent *self,
                                       char **comment,
                                       G_GNUC_UNUSED GError **error)
{
  PTRACE (1, "DBus\tGetComment");

  *comment = gm_conf_get_string (PERSONAL_DATA_KEY "comment");

  return TRUE;
}

/**************
 * PUBLIC API *
 **************/

/** Claim ownership on the EKIGA_DBUS_NAMESPACE namespace.
 * This function will return false if the namespace is already taken, ie if
 * another instance of Ekiga is already running.
 */
gboolean
ekiga_dbus_claim_ownership ()
{
  DBusGConnection *bus = NULL;
  DBusGProxy *bus_proxy = NULL;
  guint request_name_result;
  GError *error = NULL;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {
    PTRACE (1, "Couldn't connect to session bus : " << error->message);
    g_error_free (error);
    return TRUE; // if we return FALSE here, ekiga won't even start without DBUS
  }

  bus_proxy = dbus_g_proxy_new_for_name (bus, "org.freedesktop.DBus",
                                         "/org/freedesktop/DBus",
                                         "org.freedesktop.DBus");

  if (!dbus_g_proxy_call (bus_proxy, "RequestName", &error,
                          G_TYPE_STRING, EKIGA_DBUS_NAMESPACE,
                          G_TYPE_UINT, DBUS_NAME_FLAG_DO_NOT_QUEUE,
                          G_TYPE_INVALID,
                          G_TYPE_UINT, &request_name_result,
                          G_TYPE_INVALID)) {

    PTRACE (1, "Couldn't get ownership on the " EKIGA_DBUS_NAMESPACE " D-Bus namespace : "
               << error->message);
    g_error_free (error);
    return FALSE;
  }

  PTRACE (4, "Ekiga registered on D-Bus: " EKIGA_DBUS_NAMESPACE);

  return request_name_result == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER;
}

/** Create the server object for the D-Bus interface.
 * This object acts mostly as a proxy for the manager and other common objects.
 * NOTE: We expect we have claimed the namespace successfully before, and that
 *       the manager and other key components are running.
 */
EkigaDBusComponent *
ekiga_dbus_component_new (Ekiga::ServiceCore *core)
{
  DBusGConnection *bus;
  GError *error = NULL;
  EkigaDBusComponent *obj;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {
    PTRACE (1, "Couldn't connect to session bus : " << error->message);
    g_error_free (error);
    return NULL;
  }

  obj = EKIGA_DBUS_COMPONENT (g_object_new (EKIGA_TYPE_DBUS_COMPONENT, NULL));
  obj->priv->core = core;
  dbus_g_connection_register_g_object (bus, EKIGA_DBUS_PATH, G_OBJECT (obj));

  return obj;
}

static DBusGProxy *
get_ekiga_client_proxy ()
{
  DBusGConnection *bus = NULL;
  GError *error = NULL;

  bus = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (!bus) {
    PTRACE (1, "Couldn't connect to session bus : " << error->message);
    g_error_free (error);
    return NULL;
  }

  return dbus_g_proxy_new_for_name (bus,
                                    EKIGA_DBUS_NAMESPACE,
                                    EKIGA_DBUS_PATH,
                                    EKIGA_DBUS_INTERFACE);
}

/** Tell to a remote instance of Ekiga to connect to a remote SIP or H.323 
 * address.
 * You will typically use this function when claim_ownership failed.
 */
void
ekiga_dbus_client_connect (const gchar *uri)
{
  DBusGProxy *proxy = get_ekiga_client_proxy ();

  g_return_if_fail (DBUS_IS_G_PROXY (proxy));

  dbus_g_proxy_call_no_reply (proxy, "Call", G_TYPE_STRING, uri, G_TYPE_INVALID);
  g_object_unref (proxy);
}

/** Tell to a remote instance of Ekiga to show the main window.
 * You will typically use this function when claim_ownership failed.
 */
void
ekiga_dbus_client_show ()
{
  DBusGProxy *proxy = get_ekiga_client_proxy ();

  g_return_if_fail (DBUS_IS_G_PROXY (proxy));

  dbus_g_proxy_call_no_reply (proxy, "Show", G_TYPE_INVALID);
  g_object_unref (proxy);
}
