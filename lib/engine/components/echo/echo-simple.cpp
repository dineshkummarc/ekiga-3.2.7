
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
 *                         echo-simple.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of an echo simple chat
 *
 */

#include <iostream>

#include "echo-simple.h"

Echo::SimpleChat::SimpleChat (): presentity(new Presentity ())
{
}

Echo::SimpleChat::~SimpleChat ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

Ekiga::PresentityPtr
Echo::SimpleChat::get_presentity () const
{
  return presentity;
}

bool
Echo::SimpleChat::populate_menu (Ekiga::MenuBuilder &/*builder*/)
{
  return false;
}

void
Echo::SimpleChat::connect (gmref_ptr<Ekiga::ChatObserver> observer)
{
  observer->notice ("This is just an echo chat : type and see back");

  observers.push_front (observer);
}

void
Echo::SimpleChat::disconnect (gmref_ptr<Ekiga::ChatObserver> observer)
{
  observers.remove (observer);

  if (observers.empty ())
    removed.emit ();
}

bool
Echo::SimpleChat::send_message (const std::string msg)
{
  for (std::list<gmref_ptr<Ekiga::ChatObserver> >::iterator iter = observers.begin ();
       iter != observers.end ();
       ++iter)
    (*iter)->message ("Echo", msg);

  return true;
}

const std::string
Echo::SimpleChat::get_title() const
{
  return "Echo chat";
}
