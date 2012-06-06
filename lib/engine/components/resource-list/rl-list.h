

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
 *                         rl-list.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list list class
 *
 */

#ifndef __RL_LIST_H__
#define __RL_LIST_H__

#include "rl-entry.h"

namespace RL
{
  class ListImpl; // pimpling : both it and external need to know each other

  class List: public virtual GmRefCounted
  {
  public:

    List (Ekiga::ServiceCore& core_,
	  gmref_ptr<XCAP::Path> path_,
	  int pos,
	  const std::string group_,
	  xmlNodePtr node_);

    ~List ();

    /* the part of the interface which helps the list manage this element */

    bool has_name (const std::string name) const;

    /* we need to push presence&status down */

    void push_presence (const std::string uri_,
			const std::string presence);

    void push_status (const std::string uri_,
		      const std::string status);

    /* make the world know what we have */
    bool visit_presentities (sigc::slot1<bool, Ekiga::Presentity&> visitor);

    void publish () const;

    sigc::signal1<void, gmref_ptr<Entry> > entry_added;
    sigc::signal1<void, gmref_ptr<Entry> > entry_updated;
    sigc::signal1<void, gmref_ptr<Entry> > entry_removed;

    /* this method orders the list to get rid of all its children */
    void flush ();

  private:

    ListImpl *impl;
  };
};

#endif
