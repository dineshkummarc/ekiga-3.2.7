
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
 *                         contact-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the main contact managing object
 *
 */

#include <iostream>
#include <glib/gi18n.h>

#include "config.h"

#include "contact-core.h"

static void
on_search ()
{
  std::cout << "Search not implemented yet" << std::endl;
}

Ekiga::ContactCore::~ContactCore ()
{
  for (std::list<sigc::connection>::iterator iter = conns.begin (); iter != conns.end (); ++iter)
    iter->disconnect ();
}

bool
Ekiga::ContactCore::populate_menu (MenuBuilder &builder)
{
  bool populated = false;

  builder.add_action ("search", _("_Find"), sigc::ptr_fun (on_search));
  populated = true;

  for (std::list<SourcePtr >::const_iterator iter = sources.begin ();
       iter != sources.end ();
       ++iter) {

    if (populated)
      builder.add_separator ();
    populated = (*iter)->populate_menu (builder);
  }

  return populated;
}

void
Ekiga::ContactCore::add_source (SourcePtr source)
{
  sources.push_back (source);
  source_added.emit (source);
  conns.push_back (source->book_added.connect (sigc::bind<0> (book_added.make_slot (), source)));
  conns.push_back (source->book_removed.connect (sigc::bind<0> (book_removed.make_slot (), source)));
  conns.push_back (source->book_updated.connect (sigc::bind<0> (book_updated.make_slot (), source)));
  conns.push_back (source->contact_added.connect (sigc::bind<0> (contact_added.make_slot (), source)));
  conns.push_back (source->contact_removed.connect (sigc::bind<0> (contact_removed.make_slot (), source)));
  conns.push_back (source->contact_updated.connect (sigc::bind<0> (contact_updated.make_slot (), source)));
  source->questions.add_handler (questions.make_slot ());
}

void
Ekiga::ContactCore::visit_sources (sigc::slot1<bool, SourcePtr > visitor)
{
  bool go_on = true;

  for (std::list<SourcePtr >::iterator iter = sources.begin ();
       iter != sources.end () && go_on;
       ++iter)
    go_on = visitor (*iter);
}

void
Ekiga::ContactCore::add_contact_decorator (gmref_ptr<ContactDecorator> decorator)
{
  contact_decorators.push_back (decorator);
}


bool
Ekiga::ContactCore::populate_contact_menu (ContactPtr contact,
					   const std::string uri,
					   MenuBuilder &builder)
{
  bool populated = false;

  for (std::list<gmref_ptr<ContactDecorator> >::const_iterator iter
	 = contact_decorators.begin ();
       iter != contact_decorators.end ();
       ++iter) {

    populated = (*iter)->populate_menu (contact, uri, builder) || populated;
  }

  return populated;
}
