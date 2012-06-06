
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
 *                         avahi-publisher.cpp  -  description
 *                         ------------------------------------
 *   begin                : Sat Aug 30 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Avahi publisher implementation
 *
 */

#include "config.h"

#include "avahi-publisher.h"

/* anytime there's a line like :
 * ret = avahi_...
 * then it's possible to get debug information with a line like :
 * g_print ("Error in avahi code: %s\n", avahi_strerror (ret));
 */

/* here are the avahi C callbacks */

static void
client_cb (AvahiClient* client,
	   AvahiClientState state,
	   Avahi::PresencePublisher* publisher)
{
  publisher->client_callback (client, state);
}

static void
entry_group_cb (AvahiEntryGroup* group,
		AvahiEntryGroupState state,
		Avahi::PresencePublisher* publisher)
{
  publisher->entry_group_callback (group, state);
}

/* here is the real code of the Avahi::PresencePublisher implementation */

Avahi::PresencePublisher::PresencePublisher (Ekiga::ServiceCore& core_,
					     Ekiga::PersonalDetails& details_,
					     Ekiga::CallCore& call_core_):
  core(core_), details(details_), call_core(call_core_),
  client(NULL), group(NULL)
{
  display_name = details.get_display_name ();
  details.updated.connect (sigc::mem_fun (this, &Avahi::PresencePublisher::on_details_updated));
  name = avahi_strdup (display_name.c_str ());
  glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
  create_client ();
}

Avahi::PresencePublisher::~PresencePublisher ()
{
  free_client ();

  avahi_glib_poll_free (glib_poll);

  avahi_free (name);
}

void
Avahi::PresencePublisher::publish (G_GNUC_UNUSED const Ekiga::PersonalDetails& details_)
{
  if (group != NULL) {

    Ekiga::CallManager::InterfaceList interfaces;
    AvahiStringList* txt_record = NULL;
    int ret;

    txt_record = prepare_txt_record ();
    for (Ekiga::CallCore::const_iterator iter = call_core.begin ();
	 iter != call_core.end ();
	 ++iter) {

      Ekiga::CallManager::InterfaceList ints = (*iter)->get_interfaces ();
      interfaces.insert (interfaces.begin (), ints.begin (), ints.end ());

    }

    for (Ekiga::CallManager::InterfaceList::const_iterator iter = interfaces.begin ();
	 iter != interfaces.end ();
	 ++iter) {

      gchar *typ = NULL;

      typ = g_strdup_printf ("_%s._%s",
			     iter->voip_protocol.c_str (),
			     iter->protocol.c_str ());

      /* FIXME: no collision checking here */
      ret =
	avahi_entry_group_update_service_txt_strlst (group, AVAHI_IF_UNSPEC,
						     AVAHI_PROTO_UNSPEC,
						     (AvahiPublishFlags)0,
						     name, typ, NULL,
						     txt_record);

      g_free (typ);
    }

    avahi_string_list_free (txt_record);
  }
}

void
Avahi::PresencePublisher::create_client ()
{
  free_client ();
  // don't get the client there : wait what we'll get from the callback
  avahi_client_new (avahi_glib_poll_get (glib_poll), AVAHI_CLIENT_NO_FAIL,
		    (AvahiClientCallback)client_cb, this, NULL);
}

void
Avahi::PresencePublisher::free_client ()
{
  if (client != NULL) {

    avahi_client_free (client);
    client = NULL;
  }
}

void
Avahi::PresencePublisher::client_callback (AvahiClient* client_,
					   AvahiClientState state)
{
  if (client_ == NULL)
    return;

  client = client_;

  switch (state) {

  case AVAHI_CLIENT_FAILURE:

    if (avahi_client_errno (client) == AVAHI_ERR_DISCONNECTED) {

      free_client ();
      create_client ();
    }
    break;
  case AVAHI_CLIENT_S_RUNNING:

    register_services ();
    break;

  case AVAHI_CLIENT_S_REGISTERING:
  case AVAHI_CLIENT_S_COLLISION:
  case AVAHI_CLIENT_CONNECTING:
  default:
    break; // nothing
  }
}

void
Avahi::PresencePublisher::register_services ()
{
  remove_services ();

  avahi_entry_group_new (client,
			 (AvahiEntryGroupCallback)entry_group_cb, this);
}

void
Avahi::PresencePublisher::remove_services ()
{
  if (group != NULL) {

    avahi_entry_group_free (group);
    group = NULL;
  }
}

void
Avahi::PresencePublisher::entry_group_callback (AvahiEntryGroup* group_,
						AvahiEntryGroupState state)
{
  if (group_ == NULL)
    return;

  group = group_;

  switch (state) {

  case AVAHI_ENTRY_GROUP_COLLISION: {

    gchar *new_name = NULL;

    new_name = avahi_alternative_service_name (name);
    g_free (name);
    name = new_name;
    add_services ();
  }
    break;

  case AVAHI_ENTRY_GROUP_UNCOMMITED:

    add_services ();
    break;

  case AVAHI_ENTRY_GROUP_REGISTERING:
  case AVAHI_ENTRY_GROUP_ESTABLISHED:
  case AVAHI_ENTRY_GROUP_FAILURE:
  default:
    break; // nothing
  }
}

void
Avahi::PresencePublisher::add_services ()
{
  Ekiga::CallManager::InterfaceList interfaces;
  AvahiStringList* txt_record = NULL;
  int ret;

  for (Ekiga::CallCore::const_iterator iter = call_core.begin ();
       iter != call_core.end ();
       ++iter) {

    Ekiga::CallManager::InterfaceList ints = (*iter)->get_interfaces ();
    interfaces.insert (interfaces.begin (), ints.begin (), ints.end ());

  }

  txt_record = prepare_txt_record ();

  for (Ekiga::CallManager::InterfaceList::const_iterator iter = interfaces.begin ();
       iter != interfaces.end ();
       ++iter) {

    gchar *typ = NULL;

    typ = g_strdup_printf ("_%s._%s",
			   iter->voip_protocol.c_str (),
			   iter->protocol.c_str ());

    /* FIXME: no collision checking here */
    ret = avahi_entry_group_add_service_strlst (group, AVAHI_IF_UNSPEC,
						AVAHI_PROTO_UNSPEC,
						(AvahiPublishFlags)0,
						name, typ,
						NULL, NULL,
						iter->port, txt_record);

    g_free (typ);
  }
  avahi_string_list_free (txt_record);
  ret = avahi_entry_group_commit (group);
}

AvahiStringList*
Avahi::PresencePublisher::prepare_txt_record ()
{
  AvahiStringList* result = NULL;

  result = avahi_string_list_add_printf (result,
					 "presence=%s",
					 details.get_presence ().c_str ());
  result = avahi_string_list_add_printf (result,
					 "status=%s",
					 details.get_status ().c_str ());
  result = avahi_string_list_add_printf (result,
					 "software=%s %s",
					 PACKAGE_NAME, PACKAGE_VERSION);

  return result;
}

void
Avahi::PresencePublisher::on_details_updated ()
{
  if (display_name != details.get_display_name ()) {

    display_name = details.get_display_name ();
    remove_services ();
    avahi_free (name);
    name = avahi_strdup (display_name.c_str ());
    avahi_entry_group_new (client,
			   (AvahiEntryGroupCallback)entry_group_cb, this);
  }
}
