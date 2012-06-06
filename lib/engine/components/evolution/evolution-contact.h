
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
 *                         evolution-contact.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an evolution contact
 *
 */

#ifndef __EVOLUTION_CONTACT_H__
#define __EVOLUTION_CONTACT_H__

#include <map>

#include <libebook/e-book.h>
#include "contact-core.h"
#include "form.h"

namespace Evolution
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Contact: public Ekiga::Contact
  {
  public:

    Contact (Ekiga::ServiceCore &_services,
	     EBook *ebook = NULL,
	     EContact *econtact = NULL);

    ~Contact ();

    const std::string get_id () const;

    const std::string get_name () const;

    const std::set<std::string> get_groups () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    bool is_found (const std::string) const;

    void update_econtact (EContact *econtact);

    void remove ();

  private:

    // attributes of interest to that code :
    enum {

      ATTR_HOME,
      ATTR_CELL,
      ATTR_WORK,
      ATTR_PAGER,
      ATTR_VIDEO,
      ATTR_NUMBER
    };

    Ekiga::ServiceCore &services;
    EBook *book;
    EContact *econtact;
    EVCardAttribute *attributes[ATTR_NUMBER];

    std::string get_attribute_name_from_type (unsigned int attribute_type) const;

    std::string get_attribute_value (unsigned int attribute_type) const;

    void set_attribute_value (unsigned int attribute_type,
			      std::string value);

    void edit_action ();

    void on_edit_form_submitted (bool submitted,
				 Ekiga::Form &result);

    void remove_action ();

    void on_remove_form_submitted (bool submitted,
				   Ekiga::Form &result);
  };

  typedef gmref_ptr<Contact> ContactPtr;

/**
 * @}
 */

};

#endif
