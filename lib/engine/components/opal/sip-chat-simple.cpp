
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
 *                         sip-chat-simple.cpp  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of a simple chat in SIP
 *
 */

#include "sip-chat-simple.h"
#include "uri-presentity.h"
#include "personal-details.h"

SIP::SimpleChat::SimpleChat (Ekiga::ServiceCore& core_,
			     std::string name,
			     std::string uri_,
			     sigc::slot1<bool, std::string> sender_)
  : core(core_), sender(sender_), uri(uri_)
{
  presentity = gmref_ptr<Ekiga::URIPresentity> (new Ekiga::URIPresentity (core, name, uri,
									  std::set<std::string>()));
}

SIP::SimpleChat::~SimpleChat ()
{
  presentity->removed.emit ();
}

const std::string
SIP::SimpleChat::get_uri () const
{
  return uri;
}

const std::string
SIP::SimpleChat::get_title () const
{
  return presentity->get_name ();
}

void
SIP::SimpleChat::connect (gmref_ptr<Ekiga::ChatObserver> observer)
{
  observers.push_front (observer);
}

void
SIP::SimpleChat::disconnect (gmref_ptr<Ekiga::ChatObserver> observer)
{
  observers.remove (observer);

  if (observers.empty ())
    removed.emit ();
}

bool
SIP::SimpleChat::send_message (const std::string msg)
{
  bool result;
  gmref_ptr<Ekiga::PersonalDetails> personal = core.get ("personal-details");
  result = sender (msg);
  for (std::list<gmref_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
       iter != observers.end ();
       ++iter)
    (*iter)->message (personal->get_display_name (), msg);
  return result;
}

void
SIP::SimpleChat::receive_message (const std::string msg)
{
  for (std::list<gmref_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
       iter != observers.end ();
       ++iter)
    (*iter)->message (presentity->get_name (), msg);
}

void
SIP::SimpleChat::receive_notice (const std::string msg)
{
  for (std::list<gmref_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
       iter != observers.end ();
       ++iter)
    (*iter)->notice (msg);
}

Ekiga::PresentityPtr
SIP::SimpleChat::get_presentity () const
{
  return presentity;
}

bool
SIP::SimpleChat::populate_menu (Ekiga::MenuBuilder& /*builder*/)
{
  return false;
}
