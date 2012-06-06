
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
 *                         uri-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a presentity around a simple URI
 *
 */

#include "uri-presentity.h"

Ekiga::URIPresentity::URIPresentity (Ekiga::ServiceCore &_core,
				     std::string name_,
				     std::string uri_,
				     std::set<std::string> groups_)
  : core(_core), name(name_), uri(uri_), presence("unknown"), groups(groups_)
{
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  presence_core->presence_received.connect (sigc::mem_fun (this, &Ekiga::URIPresentity::on_presence_received));
  presence_core->status_received.connect (sigc::mem_fun (this, &Ekiga::URIPresentity::on_status_received));
  presence_core->fetch_presence (uri);
}

Ekiga::URIPresentity::~URIPresentity ()
{
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  if (presence_core)
    presence_core->unfetch_presence (uri);
}

const std::string
Ekiga::URIPresentity::get_name () const
{
  return name;
}

const std::string
Ekiga::URIPresentity::get_presence () const
{
  return presence;
}

const std::string
Ekiga::URIPresentity::get_status () const
{
  return status;
}

const std::string
Ekiga::URIPresentity::get_avatar () const
{
  return avatar;
}

const std::set<std::string>
Ekiga::URIPresentity::get_groups () const
{
  return groups;
}

const std::string
Ekiga::URIPresentity::get_uri () const
{
  return uri;
}

bool
Ekiga::URIPresentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  return presence_core->populate_presentity_menu (PresentityPtr(this),
						  uri, builder);
}

void
Ekiga::URIPresentity::on_presence_received (std::string uri_,
					    std::string presence_)
{
  if (uri == uri_) {

    presence = presence_;
    updated.emit ();
  }
}

void
Ekiga::URIPresentity::on_status_received (std::string uri_,
					  std::string status_)
{
  if (uri == uri_) {

    status = status_;
    updated.emit ();
  }
}
