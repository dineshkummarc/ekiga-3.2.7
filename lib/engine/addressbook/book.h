
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
 *                         book.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of an addressbook
 *
 */

#ifndef __BOOK_H__
#define __BOOK_H__

#include "contact.h"

namespace Ekiga {

  class Book: public virtual GmRefCounted
  {

  public:

    /** The destructor.
     */
    virtual ~Book ()
    { }


    /** Returns the name of the Book.
     * @return The name of the book.
     */
    virtual const std::string get_name () const = 0;


    /** Visit all contacts of the book and trigger the given callback.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Book.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    virtual void visit_contacts (sigc::slot1<bool, ContactPtr>) = 0;


    /** Create the menu for that book and its actions.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Book, ie BookImpl.
     * @param A MenuBuilder object to populate.
     */
    virtual bool populate_menu (MenuBuilder &) = 0;


    /** Set the search filter.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Book, ie BookImpl or one
     * of its descendant.
     * @param The search filter to respect when refreshing a Book.
     */
    virtual void set_search_filter (std::string _filter_string) = 0;


    /** Get the current status.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Book, ie BookImpl or one
     * of its descendant.
     */
    virtual const std::string get_status () const = 0;

    
    /** Get the icon for the Book.
     * The icon is a string and could be something like "local-object"
     * or "remote-object"
     *
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Book, ie BookImpl or one
     * of its descendant.
     */
    virtual const std::string get_icon () const = 0;


    /*
     * Signals on that object
     */

    /** This signal is emitted when the Book has been updated.
     */
    sigc::signal0<void> updated;


    /** This signal is emitted when the Book has been removed from the Source.
     */
    sigc::signal0<void> removed;


    /** This signal is emitted when a Contact has been added to the Book.
     */
    sigc::signal1<void, ContactPtr > contact_added;


    /** This signal is emitted when a Contact has been removed from the Book.
     */
    sigc::signal1<void, ContactPtr > contact_removed;


    /** This signal is emitted when a Contact has been updated in the Book.
     */
    sigc::signal1<void, ContactPtr > contact_updated;


    /** This chain allows the Book to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;
  };

  typedef gmref_ptr<Book> BookPtr;
};

#endif
