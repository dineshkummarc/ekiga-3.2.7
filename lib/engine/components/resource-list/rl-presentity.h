
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>

 * This program is free software; you can  redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version. This program is distributed in the hope
 * that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Ekiga is licensed under the GPL license and as a special exception, you
 * have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OPAL, OpenH323 and PWLIB
 * programs, as long as you do follow the requirements of the GNU GPL for all
 * the rest of the software thus combined.
 */


/*
 *                         rl-presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of a presentity in a resource-list
 *
 */



#ifndef __RL_PRESENTITY_H__
#define __RL_PRESENTITY_H__

#include <libxml/tree.h>
#include <tr1/memory>

#include "form.h"
#include "presence-core.h"
#include "presentity.h"
#include "xcap-path.h"

namespace RL
{
  class Presentity:
    public virtual GmRefCounted,
    public Ekiga::Presentity
  {
  public:

    Presentity (Ekiga::ServiceCore &_core,
		gmref_ptr<XCAP::Path> path_,
		std::tr1::shared_ptr<xmlDoc> doc_,
		xmlNodePtr _node,
		bool writable_);

    ~Presentity ();

    const std::string get_name () const;

    const std::string get_presence () const;

    const std::string get_status () const;

    const std::string get_avatar () const;

    const std::set<std::string> get_groups () const;

    const std::string get_uri () const;

    void set_presence (const std::string _presence);

    void set_status (const std::string _status);

    bool populate_menu (Ekiga::MenuBuilder &);

    sigc::signal0<void> trigger_reload;

  private:

    void edit_presentity ();

    void edit_presentity_form_submitted (bool submitted,
					 Ekiga::Form &result);

    void save (bool reload);

    void remove ();

    void save_result (std::string error,
		      bool reload);

    void erase_result (std::string error);

    Ekiga::ServiceCore &services;

    gmref_ptr<XCAP::Path> path;
    std::tr1::shared_ptr<xmlDoc> doc;
    xmlNodePtr node;
    bool writable;

    xmlNodePtr name_node;

    std::string uri;
    std::string presence;
    std::string status;
    std::string avatar;

    std::map<std::string, xmlNodePtr> group_nodes;
    std::set<std::string> groups;
  };

  typedef gmref_ptr<Presentity> PresentityPtr;

};

#endif
