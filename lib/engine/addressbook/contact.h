
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
 *                         contact.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of an addressbook
 *                          contact
 *
 */

#ifndef __CONTACT_H__
#define __CONTACT_H__

#include <set>
#include <map>
#include <string>

#include "gmref.h"
#include "chain-of-responsibility.h"
#include "form-request.h"
#include "menu-builder.h"

namespace Ekiga
{

/**
 * @addtogroup contacts
 * @{
 */

  class Contact: public virtual GmRefCounted
  {
  public:

    /** The destructor.
     */
    virtual ~Contact () { }


    /** Returns the name of the Ekiga::Contact.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Contact descendant.
     * @return The name of the Ekiga::Contact.
     */
    virtual const std::string get_name () const = 0;


    /** Returns the groups of the Ekiga::Contact.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Contact descendant.
     * @return The set of group names of the Ekiga::Contact.
     */
    virtual const std::set<std::string> get_groups () const = 0;


    /** Returns if a contact is found or not.
     * @return True if the contact has been found.
     */
    virtual bool is_found (const std::string) const = 0;


    /** Create the menu for that contact and its actions.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Contact.
     * @param A MenuBuilder object to populate.
     */
    virtual bool populate_menu (MenuBuilder &) = 0;


    /**
     * Signals on that object
     */

    /** This signal is emitted when the Contact has been updated.
     */
    sigc::signal0<void> updated;


    /** This signal is emitted when the Contact has been removed.
     */
    sigc::signal0<void> removed;

    /** This chain allows the Contact to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;
  };


  typedef gmref_ptr<Contact> ContactPtr;
/**
 * @}
 */

};
#endif
