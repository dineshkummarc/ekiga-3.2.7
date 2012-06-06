
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
 *                         gnomesession-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Julien Puydt
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : code to hook gnomesession into the main program
 *
 */

#include "config.h"

#include <iostream>
#include <map>
#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

#include "services.h"
#include "call-core.h"

#include "gnome-session-main.h"

struct GNOMESESSIONService: public Ekiga::Service
{
  GNOMESESSIONService (gmref_ptr<Ekiga::CallCore> call_core);

  ~GNOMESESSIONService ();

  const std::string get_name () const
  { return "gnome-session"; }

  const std::string get_description () const
  { return "\tComponent connecting ekiga to the gnome session"; }

private:

  void on_established_call (gmref_ptr<Ekiga::CallManager> manager,
			    gmref_ptr<Ekiga::Call> call);

  void on_cleared_call (gmref_ptr<Ekiga::CallManager> manager,
			gmref_ptr<Ekiga::Call> call);

  DBusGProxy* proxy;
  std::map<gmref_ptr<Ekiga::Call>, guint> cookies;
};

struct GNOMESESSIONSpark: public Ekiga::Spark
{
  GNOMESESSIONSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    gmref_ptr<Ekiga::CallCore> call_core = core.get ("call-core");
    gmref_ptr<Ekiga::Service> service = core.get ("gnome-session");

    if (call_core && !service) {

      result = true;
      core.add (gmref_ptr<Ekiga::Service> (new GNOMESESSIONService (call_core)));
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "GNOMESESSION"; }

  bool result;
};

void
gnomesession_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new GNOMESESSIONSpark);
  kickstart.add_spark (spark);
}

GNOMESESSIONService::GNOMESESSIONService (gmref_ptr<Ekiga::CallCore> call_core)
{
  GError* error = NULL;

  DBusGConnection* connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);

  if (error == NULL) {

    proxy = dbus_g_proxy_new_for_name_owner (connection,
					     "org.gnome.SessionManager",
					     "/org/gnome/SessionManager",
					     "org.gnome.SessionManager",
					     &error);

    if (error == NULL) {

      call_core->established_call.connect (sigc::mem_fun (this, &GNOMESESSIONService::on_established_call));
      call_core->setup_call.connect (sigc::mem_fun (this, &GNOMESESSIONService::on_cleared_call));
    } else {

      proxy = NULL;
      g_error_free (error);
    }
  } else {

    proxy = NULL;
    g_error_free (error);
  }
}

GNOMESESSIONService::~GNOMESESSIONService ()
{
  if (proxy != NULL)
    g_object_unref (proxy);
}

void
GNOMESESSIONService::on_established_call (gmref_ptr<Ekiga::CallManager> /*manager*/,
					  gmref_ptr<Ekiga::Call> call)
{
  guint cookie;

  gboolean ret = dbus_g_proxy_call (proxy, "Inhibit", NULL,
				    G_TYPE_STRING, PACKAGE_NAME,
				    G_TYPE_UINT, 0,
				    G_TYPE_STRING, "Call in progress",
				    G_TYPE_UINT, 8,
				    G_TYPE_INVALID,
				    G_TYPE_UINT, &cookie,
				    G_TYPE_INVALID);
  if (ret) {

    cookies[call] = cookie;
  }
}

void
GNOMESESSIONService::on_cleared_call (gmref_ptr<Ekiga::CallManager> /*manager*/,
				      gmref_ptr<Ekiga::Call> call)
{
  std::map<gmref_ptr<Ekiga::Call>, guint>::iterator iter = cookies.find (call);

  if (iter != cookies.end ()) {

    dbus_g_proxy_call (proxy, "Uninhibit", NULL,
		       G_TYPE_UINT, iter->second,
		       G_TYPE_INVALID, G_TYPE_INVALID);
    cookies.erase (iter);
  }
}
