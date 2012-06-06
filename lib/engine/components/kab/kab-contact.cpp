
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
 *                         kab-contact.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a KDE addressbook contact
 *
 */

#include <iostream>

#include "kab-contact.h"

KAB::Contact::Contact (Ekiga::ContactCore& _core,
		       KABC::Addressee* addressee_)
  : core(_core), addressee(*addressee_)
{
  QStringList categories = addressee.categories ();
  for (QStringList::const_iterator iter = categories.begin ();
       iter != categories.end ();
       iter++) {

    groups.insert ((*iter).toUtf8 ().data ());
  }
}

KAB::Contact::~Contact ()
{
}

const std::string
KAB::Contact::get_name () const
{
  return addressee.formattedName ().toUtf8 ().constData ();
}

const std::set<std::string>
KAB::Contact::get_groups () const
{
  return groups;
}

bool
KAB::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool result = false;

  /* FIXME: add here the specific actions we want to allow
   * (before or after the uri-specific actions)
   */

  KABC::PhoneNumber::List phoneNumbers = addressee.phoneNumbers ();
  for (KABC::PhoneNumber::List::const_iterator iter = phoneNumbers.begin ();
       iter != phoneNumbers.end ();
       iter++) {

    std::string precision = (*iter).typeLabel ().toUtf8 ().constData ();
    result = result
      || core.populate_contact_menu (Ekiga::ContactPtr(this),
				     (*iter).number ().toUtf8 ().constData (),
				     builder);
  }

  return result;
}

bool
KAB::Contact::is_found (const std::string /*test*/) const
{
  return false;
}
