
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

#ifndef __KAB_CONTACT_H__
#define __KAB_CONTACT_H__

#include "contact-core.h"

// include this *after* the rest, or qt will suicide sigc++
#include <kabc/addressee.h>

namespace KAB
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Contact: public Ekiga::Contact
  {
  public:

    Contact (Ekiga::ContactCore &_core,
	     KABC::Addressee *addressee);

    ~Contact ();

    const std::string get_name () const;

    const std::set<std::string> get_groups () const;

    const std::map<std::string, std::string> get_uris () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    bool is_found (const std::string test) const;

  private:

    Ekiga::ContactCore& core;
    KABC::Addressee& addressee;
    std::set<std::string> groups;
  };

  typedef gmref_ptr<Contact> ContactPtr;

/**
 * @}
 */

};

#endif
