
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
 *                         avahi-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the avahi heap
 *
 */

#ifndef __AVAHI_HEAP_H__
#define __AVAHI_HEAP_H__

#include "presence-core.h"
#include "heap-impl.h"
#include "uri-presentity.h"

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>

namespace Avahi
{

/**
 * @addtogroup presence
 * @internal
 * @{
 */

  class Heap:
    public Ekiga::PresenceFetcher,
    public Ekiga::HeapImpl<Ekiga::URIPresentity>,
    public sigc::trackable
  {
  public:

    Heap (Ekiga::ServiceCore &_core);

    ~Heap ();

    const std::string get_name () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    bool populate_menu_for_group (const std::string name,
				  Ekiga::MenuBuilder& builder);

    /* the PresenceFetcher interface : we don't do what we're told ;-) */
    void fetch (std::string) {}
    void unfetch (std::string) {}

    /* these should be private but are called from C code */

    void ClientCallback (AvahiClient *client,
			 AvahiClientState state);

    void BrowserCallback (AvahiServiceBrowser *browser,
			  AvahiIfIndex interface,
			  AvahiProtocol protocol,
			  AvahiBrowserEvent event,
			  const char *name,
			  const char *type,
			  const char *domain,
			  AvahiLookupResultFlags flags);

    void ResolverCallback (AvahiServiceResolver *_resolver,
			   AvahiIfIndex interface,
			   AvahiProtocol protocol,
			   AvahiResolverEvent event,
			   const char *name,
			   const char *type,
			   const char *domain,
			   const char *host_name,
			   const AvahiAddress *address,
			   uint16_t port,
			   AvahiStringList *txt,
			   AvahiLookupResultFlags flags);

  private:

    Ekiga::ServiceCore &core;
    AvahiGLibPoll *poll;
    AvahiClient *client;

    bool remover (Ekiga::PresentityPtr presentity,
		  const std::string name);
  };

  typedef gmref_ptr<Heap> HeapPtr;

/**
 * @}
 */

};

#endif
