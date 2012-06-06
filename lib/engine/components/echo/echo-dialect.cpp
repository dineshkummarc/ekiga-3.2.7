
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
 *                         echo-dialect.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of an echo chat backend
 *
 */

#include <iostream>

#include "echo-dialect.h"

Echo::Dialect::Dialect ()
{
}

Echo::Dialect::~Dialect ()
{
#ifdef __GNUC_
  std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
}

bool
Echo::Dialect::populate_menu (Ekiga::MenuBuilder &builder)

{
  builder.add_action ("FIXME", "New echo", sigc::mem_fun (this, &Echo::Dialect::new_chat));

  return true;
}

void
Echo::Dialect::new_chat ()
{
  SimpleChatPtr chat (new SimpleChat);
  add_simple_chat (chat);
  chat->user_requested.emit ();
}
