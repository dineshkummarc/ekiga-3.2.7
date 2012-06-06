
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
 *                         proxy-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of a presentity proxying another
 *
 */

#include "proxy-presentity.h"

Ekiga::ProxyPresentity::ProxyPresentity (Ekiga::Presentity& presentity_)
  : presentity(presentity_)
{
  presentity.updated.connect (updated.make_slot ());
  presentity.removed.connect (removed.make_slot ());
}

Ekiga::ProxyPresentity::~ProxyPresentity ()
{
}

const std::string
Ekiga::ProxyPresentity::get_name () const
{
  return presentity.get_name ();
}

const std::string
Ekiga::ProxyPresentity::get_presence () const
{
  return presentity.get_presence ();
}

const std::string
Ekiga::ProxyPresentity::get_status () const
{
  return presentity.get_status ();
}

const std::string
Ekiga::ProxyPresentity::get_avatar () const
{
  return presentity.get_avatar ();
}

const std::set<std::string>
Ekiga::ProxyPresentity::get_groups () const
{
  return presentity.get_groups ();
}

bool
Ekiga::ProxyPresentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  return presentity.populate_menu (builder);
}
