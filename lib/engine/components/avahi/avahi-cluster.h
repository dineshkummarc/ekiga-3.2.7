
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
 *                         avahi-cluster.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the avahi cluster
 *
 */

#ifndef __AVAHI_CLUSTER_H__
#define __AVAHI_CLUSTER_H__

#include "cluster-impl.h"
#include "avahi-heap.h"

namespace Avahi
{

/**
 * @addtogroup presence
 * @internal
 * @{
 */

  class Cluster:
    public Ekiga::Service,
    public Ekiga::ClusterImpl<Heap>,
    public sigc::trackable
  {
  public:

    Cluster (Ekiga::ServiceCore &_core);

    ~Cluster ();

    bool populate_menu (Ekiga::MenuBuilder &builder);

    /* this is a service */
    const std::string get_name () const
    { return "avahi-core"; }

    const std::string get_description () const
    { return "\tObject getting presence from the network neighbourhood"; }


  private:

    Ekiga::ServiceCore &core;
    HeapPtr heap;
  };

  typedef gmref_ptr<Cluster> ClusterPtr;

/**
 * @}
 */

};

#endif
