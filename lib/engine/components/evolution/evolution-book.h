
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
 *                         evolution-book.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : interface of an evolution addressbook
 *
 */

#ifndef __EVOLUTION_BOOK_H__
#define __EVOLUTION_BOOK_H__

#include <libebook/e-book.h>

#include "form.h"
#include "book-impl.h"

#include "evolution-contact.h"

namespace Evolution
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Book:
    public Ekiga::BookImpl<Contact>
  {
  public:

    Book (Ekiga::ServiceCore &_services,
	  EBook *_book);

    ~Book ();

    const std::string get_name () const;

    EBook *get_ebook () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    void set_search_filter (std::string search_filter);

    const std::string get_status () const;

    const std::string get_icon () const
    { return "local-object"; }

    void refresh ();

    /* those are private, but need to be called from C code */
    void on_book_opened (EBookStatus status);
    void on_book_view_obtained (EBookStatus status,
				EBookView *view);
    void on_view_contacts_added (GList *econtacts);
    void on_view_contacts_removed (GList *ids);
    void on_view_contacts_changed (GList *econtacts);

  private:

    void new_contact_action ();
    void set_econtact_attribute_value (EContact *contact,
				       const std::string subtype,
				       const std::string value) const;
    void on_new_contact_form_submitted (bool submitted,
					Ekiga::Form &result);

    Ekiga::ServiceCore &services;
    EBook *book;
    EBookView *view;

    std::string status;
    std::string search_filter;
  };

  typedef gmref_ptr<Book> BookPtr;

/**
 * @}
 */

};

#endif
