
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
 *                         local-roster-bridge.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to push contacts into the local roster
 *
 */

#include <iostream>
#include <glib/gi18n.h>

#include "local-roster-bridge.h"
#include "contact-core.h"
#include "local-cluster.h"


/* declaration&implementation of the bridge */

namespace Local
{
  class ContactDecorator:
    public Ekiga::Service,
    public Ekiga::ContactDecorator
  {
  public:

    ContactDecorator (ClusterPtr _cluster): cluster(_cluster)
    {}

    ~ContactDecorator ()
    {
    }

    const std::string get_name () const
    { return "local-roster-bridge"; }

    const std::string get_description () const
    { return "\tComponent to push contacts into the local roster"; }

    bool populate_menu (Ekiga::ContactPtr contact,
			const std::string uri,
			Ekiga::MenuBuilder &builder);

  private:

    ClusterPtr cluster;
  };
};


bool
Local::ContactDecorator::populate_menu (Ekiga::ContactPtr contact,
					const std::string uri,
					Ekiga::MenuBuilder &builder)
{
  bool populated = false;

  if (cluster->is_supported_uri (uri)) {

    HeapPtr heap(cluster->get_heap ());

    if (!heap->has_presentity_with_uri (uri)) {

      builder.add_action ("add", _("Add to local roster"),
			  sigc::bind (sigc::mem_fun (heap.get (), &Local::Heap::new_presentity),
				      contact->get_name (), uri));
      populated = true;
    }
  }

  return populated;
}


/* public api */

struct LOCALROSTERBRIDGESpark: public Ekiga::Spark
{
  LOCALROSTERBRIDGESpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    gmref_ptr<Ekiga::Service> service = core.get ("local-roster-bridge");
    gmref_ptr<Ekiga::ContactCore> contact_core = core.get ("contact-core");
    gmref_ptr<Local::Cluster> cluster = core.get ("local-cluster");

    if (cluster && contact_core && !service) {

      gmref_ptr<Local::ContactDecorator> decorator (new Local::ContactDecorator (cluster));
      core.add (decorator);
      contact_core->add_contact_decorator (decorator);
      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "LOCALROSTERBRIDGE"; }

  bool result;
};

void
local_roster_bridge_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new LOCALROSTERBRIDGESpark);
  kickstart.add_spark (spark);
}
