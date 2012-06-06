
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
 *                         chat-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the main chat managing object
 *
 */

#include "chat-core.h"

#include <iostream>

Ekiga::ChatCore::~ChatCore ()
{
}

void
Ekiga::ChatCore::add_dialect (DialectPtr dialect)
{
  dialects.push_back (dialect);
  dialect->questions.add_handler (questions.make_slot ());
  dialect_added.emit (dialect);
}

void
Ekiga::ChatCore::visit_dialects (sigc::slot1<bool, DialectPtr > visitor)
{
  bool go_on = true;

  for (std::list<DialectPtr >::iterator iter = dialects.begin ();
       iter != dialects.end () && go_on;
       iter++)
    go_on = visitor (*iter);
}

bool
Ekiga::ChatCore::populate_menu (MenuBuilder &builder)
{
  bool result = false;

  for (std::list<DialectPtr >::iterator iter = dialects.begin ();
       iter != dialects.end ();
       ++iter)
    result = (*iter)->populate_menu (builder) || result;

  return result;
}
