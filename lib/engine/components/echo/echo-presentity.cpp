
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
 *                         echo-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of an echo presentity
 *
 */

#include <iostream>

#include "echo-presentity.h"

Echo::Presentity::Presentity ()
{
}

Echo::Presentity::~Presentity ()
{
#ifdef __GNUC__
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

const std::string
Echo::Presentity::get_name () const
{
  return "Echo";
}

const std::string
Echo::Presentity::get_presence () const
{
  return "online"; // FIXME
}

const std::string
Echo::Presentity::get_status () const
{
  return "ready";
}

const std::string
Echo::Presentity::get_avatar () const
{
  return ""; // FIXME
}

const std::set<std::string>
Echo::Presentity::get_groups () const
{
  return std::set<std::string>();
}

const std::string
Echo::Presentity::get_uri () const
{
  return ""; // should be good enough
}

bool
Echo::Presentity::populate_menu (Ekiga::MenuBuilder &)
{
  return false; // FIXME?
}
