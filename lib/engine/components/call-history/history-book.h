
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
 *                         history-book.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the book for the call history
 *
 */

#ifndef __HISTORY_BOOK_H__
#define __HISTORY_BOOK_H__

#include "call-core.h"
#include "call-manager.h"

#include "book-impl.h"
#include "history-contact.h"

namespace History
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Book:
    public Ekiga::BookImpl<Contact>,
    public sigc::trackable
  {
  public:

    /* generic api */
    
    Book (Ekiga::ServiceCore &_core);

    ~Book ();

    const std::string get_name () const;

    bool populate_menu (Ekiga::MenuBuilder &);

    const std::set<std::string> existing_groups () const;

    const std::string get_status () const;

    const std::string get_icon () const
    { return "local-object"; }

    void set_search_filter (std::string);

    /* more specific api */

    void add (const std::string & name,
              const std::string & uri,
              const time_t & call_start,
              const std::string & call_duration,
              const call_type c_t);

    void clear ();

    sigc::signal0<void> cleared;

  private:

    void parse_entry (xmlNodePtr entry);

    void save () const;

    void add (xmlNodePtr node);

    void common_add (Contact &contact);

    void on_missed_call (gmref_ptr<Ekiga::CallManager> manager,
			 gmref_ptr<Ekiga::Call> call);

    void on_cleared_call (gmref_ptr<Ekiga::CallManager> manager,
			  gmref_ptr<Ekiga::Call> call,
			  std::string message);

    Ekiga::ServiceCore &core;
    std::tr1::shared_ptr<xmlDoc> doc;
  };

  typedef gmref_ptr<Book> BookPtr;

/**
 * @}
 */

};

#endif
