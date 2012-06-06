
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
 *                         engine.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Vroom.
 *
 */

#include "config.h"

#include "engine.h"

#include "services.h"
#include "kickstart.h"

#include "notification-core.h"
#include "plugin-core.h"
#include "presence-core.h"
#include "account-core.h"
#include "contact-core.h"
#include "call-core.h"
#include "chat-core.h"
#include "videooutput-core.h"
#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"
#include "hal-core.h"
#include "history-main.h"
#include "local-roster-main.h"
#include "local-roster-bridge.h"
#include "gtk-core-main.h"
#include "gtk-frontend.h"
#include "gmconf-personal-details-main.h"

#ifndef WIN32
#include "videooutput-main-x.h"
#endif

#ifdef HAVE_DX
#include "videooutput-main-dx.h"
#endif

#include "videoinput-main-mlogo.h"
#include "audioinput-main-null.h"
#include "audiooutput-main-null.h"

#include "videoinput-main-ptlib.h"
#include "audioinput-main-ptlib.h"
#include "audiooutput-main-ptlib.h"

#ifdef HAVE_DBUS
#include "hal-main-dbus.h"
#ifdef HAVE_GNOME
#include "gnome-session-main.h"
#endif
#endif

#include "opal-main.h"

#ifdef HAVE_AVAHI
#include "avahi-main.h"
#include "avahi-publisher-main.h"
#endif

#ifdef HAVE_EDS
#include "evolution-main.h"
#endif

#ifdef HAVE_LDAP
#include "ldap-main.h"
#endif

#ifdef HAVE_NOTIFY
#include "libnotify-main.h"
#endif

#ifdef HAVE_GSTREAMER
#include "gst-main.h"
#endif

#ifdef HAVE_KDE
#include "kde-main.h"
#endif

#ifdef HAVE_KAB
#include "kab-main.h"
#endif

#ifdef HAVE_XCAP
#include "xcap-main.h"
#include "resource-list-main.h"
#endif

#define DEBUG_STARTUP 0

#if DEBUG_STARTUP
#include <iostream>
#endif

static Ekiga::ServiceCore *service_core = NULL;

void
engine_init (int argc,
             char *argv [])
{
  service_core = new Ekiga::ServiceCore;
  Ekiga::KickStart kickstart;


  service_core->add (gmref_ptr<Ekiga::Service>(new Ekiga::NotificationCore));

  /* VideoInputCore depends on VideoOutputCore and must this              *
   * be constructed thereafter                                      */

  gmref_ptr<Ekiga::AccountCore> account_core (new Ekiga::AccountCore);
  gmref_ptr<Ekiga::ContactCore> contact_core (new Ekiga::ContactCore);
  gmref_ptr<Ekiga::CallCore> call_core (new Ekiga::CallCore);
  gmref_ptr<Ekiga::ChatCore> chat_core (new Ekiga::ChatCore);
  gmref_ptr<Ekiga::VideoOutputCore> videooutput_core (new Ekiga::VideoOutputCore);
  gmref_ptr<Ekiga::VideoInputCore> videoinput_core (new Ekiga::VideoInputCore(*videooutput_core));
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core (new Ekiga::AudioOutputCore);
  gmref_ptr<Ekiga::AudioInputCore> audioinput_core (new Ekiga::AudioInputCore(*audiooutput_core));
  gmref_ptr<Ekiga::HalCore> hal_core (new Ekiga::HalCore);


  /* The last item in the following list will be destroyed first.   *
   * - VideoInputCore must be destroyed before VideoOutputCore since its  *
   *   PreviewManager may call functions of VideoOutputCore.            */

  service_core->add (account_core);
  service_core->add (contact_core);
  service_core->add (chat_core);
  service_core->add (videoinput_core);
  service_core->add (videooutput_core);
  service_core->add (audioinput_core);
  service_core->add (audiooutput_core);
  service_core->add (hal_core);
  service_core->add (call_core);

  if (!gmconf_personal_details_init (*service_core, &argc, &argv)) {
    delete service_core;
    service_core = NULL;
    return;
  }

  gmref_ptr<Ekiga::PresenceCore> presence_core (new Ekiga::PresenceCore (*service_core));
  service_core->add (presence_core);

#ifndef WIN32
  if (!videooutput_x_init (*service_core, &argc, &argv)) {
    delete service_core;
    service_core = NULL;
    return;
  }
#endif

#ifdef HAVE_DX
  if (!videooutput_dx_init (*service_core, &argc, &argv)) {
    delete service_core;
    service_core = NULL;
    return;
  }
#endif

  if (!videoinput_mlogo_init (*service_core, &argc, &argv)) {
    delete service_core;
    service_core = NULL;
    return;
  }

#ifdef HAVE_NOTIFY
  libnotify_init (kickstart);
#endif

#ifdef HAVE_GSTREAMER
  (void)gstreamer_init (*service_core, &argc, &argv);
#endif

  audioinput_null_init (kickstart);

  audiooutput_null_init (kickstart);

  videoinput_ptlib_init (kickstart);

  audioinput_ptlib_init (kickstart);

  audiooutput_ptlib_init (kickstart);

#ifdef HAVE_DBUS
  hal_dbus_init (kickstart);
#endif

  opal_init (kickstart);

#ifdef HAVE_AVAHI
  avahi_init (kickstart);
  avahi_publisher_init (kickstart);
#endif

#ifdef HAVE_EDS
  evolution_init (kickstart);
#endif

#ifdef HAVE_LDAP
  ldap_init (kickstart);
#endif

#ifdef HAVE_GNOME
#ifdef HAVE_DBUS
  gnomesession_init (kickstart);
#endif
#endif

#ifdef HAVE_KDE
  kde_init (kickstart);
#endif

#ifdef HAVE_KAB
  kab_init (kickstart);
#endif

#ifdef HAVE_XCAP
  xcap_init (kickstart);
  resource_list_init (kickstart);
#endif

  history_init (kickstart);

  /* FIXME: this one should go away -- but if I don't put it here, the GUI
   * doesn't work correctly */
  kickstart.kick (*service_core, &argc, &argv);

  if (!gtk_core_init (*service_core, &argc, &argv)) {
    delete service_core;
    service_core = NULL;
    return;
  }

  if (!gtk_frontend_init (*service_core, &argc, &argv)) {
    delete service_core;
    service_core = NULL;
    return;
  }

  local_roster_init (kickstart);

  local_roster_bridge_init (kickstart);

  plugin_init (kickstart);

  kickstart.kick (*service_core, &argc, &argv);

  videooutput_core->setup_conf_bridge();
  videoinput_core->setup_conf_bridge();
  audiooutput_core->setup_conf_bridge();
  audioinput_core->setup_conf_bridge();

  sigc::connection conn;
  conn = hal_core->videoinput_device_added.connect (sigc::mem_fun (*videoinput_core, &Ekiga::VideoInputCore::add_device));
  conn = hal_core->videoinput_device_removed.connect (sigc::mem_fun (*videoinput_core, &Ekiga::VideoInputCore::remove_device));
  conn = hal_core->audiooutput_device_added.connect (sigc::mem_fun (*audiooutput_core, &Ekiga::AudioOutputCore::add_device));
  conn = hal_core->audiooutput_device_removed.connect (sigc::mem_fun (*audiooutput_core, &Ekiga::AudioOutputCore::remove_device));
  conn = hal_core->audioinput_device_added.connect (sigc::mem_fun (*audioinput_core, &Ekiga::AudioInputCore::add_device));
  conn = hal_core->audioinput_device_removed.connect (sigc::mem_fun (*audioinput_core, &Ekiga::AudioInputCore::remove_device));
  // std::vector<sigc::connection> connections;
  //connections.push_back (conn);

#if DEBUG_STARTUP
  std::cout << "Here is what ekiga is made of for this run :" << std::endl;
  service_core->dump (std::cout);
#endif
}


Ekiga::ServiceCore *
engine_get_service_core ()
{
  return service_core;
}

void
engine_stop ()
{
  if (service_core)
    delete service_core;
  service_core = NULL;
}
