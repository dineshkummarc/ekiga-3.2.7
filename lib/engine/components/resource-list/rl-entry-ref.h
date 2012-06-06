

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
 *                         rl-entry-ref.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list entry-ref class
 *
 */

#ifndef __RL_ENTRY_REF_H__
#define __RL_ENTRY_REF_H__

#include "gmref.h"

#include "services.h"
#include "presentity.h"

#include <libxml/tree.h>

namespace RL {

  class EntryRef:
    public virtual GmRefCounted,
    public Ekiga::Presentity
  {
  public:

    EntryRef (Ekiga::ServiceCore& core_,
	      const std::string path_,
	      int pos,
	      const std::string group,
	      xmlNodePtr node_);

    ~EntryRef ();

    /* needed so presence can be pushed into this presentity */

    const std::string get_uri () const;

    void set_presence (const std::string presence_);

    void set_status (const std::string status_);

    /* Ekiga::Presentity interface */

    const std::string get_name () const;

    const std::string get_presence () const
    { return presence; }

    const std::string get_status () const
    { return status; }

    const std::string get_avatar () const
    { return ""; }

    const std::set<std::string> get_groups () const
    { return groups; }

    bool populate_menu (Ekiga::MenuBuilder& builder);

  private:
    Ekiga::ServiceCore& core;

    std::string path;
    int position;

    std::set<std::string> groups;

    xmlDocPtr doc;
    xmlNodePtr node;

    xmlDocPtr link_doc;
    xmlNodePtr link_node;
    xmlNodePtr name_node;

    std::string presence;
    std::string status;

    void refresh ();
  };
};

#endif
