
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
 *                         history-contact.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a call history entry
 *
 */

#ifndef __HISTORY_CONTACT_H__
#define __HISTORY_CONTACT_H__

#include <libxml/tree.h>

#include <tr1/memory>

#include "services.h"
#include "contact-core.h"

namespace History
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  typedef enum {

    RECEIVED,
    PLACED,
    MISSED
  } call_type;

  class Contact: public Ekiga::Contact
  {
  public:

    Contact (Ekiga::ServiceCore &_core,
	     std::tr1::shared_ptr<xmlDoc> _doc,
	     xmlNodePtr _node);

    Contact (Ekiga::ServiceCore &_core,
	     std::tr1::shared_ptr<xmlDoc> _doc,
	     const std::string _name,
	     const std::string _uri,
             time_t call_start,
             const std::string call_duration,
	     call_type c_t);

    ~Contact ();

    /*** generic contact api ***/

    const std::string get_name () const;

    const std::set<std::string> get_groups () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    bool is_found (std::string test) const;

    /*** more specific api ***/

    xmlNodePtr get_node ();

    call_type get_type () const;

    time_t get_call_start () const;

    const std::string get_call_duration () const;

  private:

    Ekiga::ServiceCore &core;

    std::tr1::shared_ptr<xmlDoc> doc;
    xmlNodePtr node;
    std::string name;
    std::string uri;
    time_t call_start;
    std::string call_duration;
    call_type m_type;
  };

  typedef gmref_ptr<Contact> ContactPtr;

/**
 * @}
 */

};

#endif
