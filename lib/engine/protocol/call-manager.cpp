
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
 *                         call-manager.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras 
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : Implementation of the interface of a call manager
 *                          implementation backend. A call manager handles calls,
 *                          sometimes simultaneously.
 *
 */


#include "call-manager.h"
#include "call-protocol-manager.h"


using namespace Ekiga;


void CallManager::add_protocol_manager (gmref_ptr<CallProtocolManager> manager)
{
  managers.insert (manager);
  manager_added.emit (manager);
}


gmref_ptr<CallProtocolManager> CallManager::get_protocol_manager (const std::string &protocol) const
{
  for (CallManager::iterator iter = begin ();
       iter != end ();
       iter++)
    if ((*iter)->get_protocol_name () == protocol)
      return (*iter);

  return gmref_ptr<CallProtocolManager>(0);
}


CallManager::iterator CallManager::begin ()
{
  return managers.begin ();
}


CallManager::const_iterator CallManager::begin () const
{
  return managers.begin ();
}


CallManager::iterator CallManager::end ()
{
  return managers.end (); 
}


CallManager::const_iterator CallManager::end () const
{
  return managers.end (); 
}


const std::list<std::string> CallManager::get_protocol_names () const
{
  std::list<std::string> protocols;

  for (CallManager::iterator iter = begin ();
       iter != end ();
       iter++)
    protocols.push_back ((*iter)->get_protocol_name ());

  return protocols;
}


const CallManager::InterfaceList CallManager::get_interfaces () const
{
  InterfaceList list;

  for (CallManager::iterator iter = begin ();
       iter != end ();
       iter++)
    list.push_back ((*iter)->get_listen_interface ());

  return list;
}

