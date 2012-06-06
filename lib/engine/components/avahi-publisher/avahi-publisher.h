
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
 *                         avahi-publisher.h  -  description
 *                         ------------------------------------
 *   begin                : Sun Aug 21 2005
 *   copyright            : (C) 2005 by Sebastien Estienne
 *                          (C) 2008 by Julien Puydt
 *   description          : Avahi publisher declaration
 *
 */

#ifndef __AVAHI_PUBLISHER_H__
#define __AVAHI_PUBLISHER_H__

#include "services.h"
#include "personal-details.h"
#include "presence-core.h"
#include "call-core.h"

#include <avahi-common/alternative.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/strlst.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-glib/glib-watch.h>

namespace Avahi
{
  class PresencePublisher:
    public Ekiga::Service,
    public Ekiga::PresencePublisher
  {
public:

    PresencePublisher (Ekiga::ServiceCore& core,
		       Ekiga::PersonalDetails& details,
		       Ekiga::CallCore& call_core);

    ~PresencePublisher ();

    const std::string get_name () const
    { return "avahi-presence-publisher"; }

    const std::string get_description () const
    { return "\tObject bringing in Avahi presence publishing"; }

    void publish (const Ekiga::PersonalDetails & details);

    /* public only to be called by C callbacks... */
    void client_callback (AvahiClient* client,
			  AvahiClientState state);
    void entry_group_callback (AvahiEntryGroup* group,
			       AvahiEntryGroupState state);

private:

    Ekiga::ServiceCore& core;
    Ekiga::PersonalDetails& details;
    Ekiga::CallCore& call_core;

    AvahiGLibPoll* glib_poll;

    void create_client ();
    void free_client ();
    AvahiClient *client;

    void register_services ();
    void add_services ();
    void remove_services ();
    AvahiEntryGroup* group;
    std::string display_name;
    gchar* name;

    AvahiStringList* prepare_txt_record ();
    void on_details_updated ();
  };
};
#endif
