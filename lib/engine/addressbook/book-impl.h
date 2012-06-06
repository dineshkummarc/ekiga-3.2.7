
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
 *                         book-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a partial implementation
 *                          of an addressbook
 *
 */

#ifndef __BOOK_IMPL_H__
#define __BOOK_IMPL_H__

#include "reflister.h"
#include "book.h"



namespace Ekiga
{

/**
 * @addtogroup contacts
 * @{
 */

  /** Generic implementation for the Ekiga::Book abstract class.
   *
   * This class is there to make it easy to implement a new type of contact
   * addressbook: it will take care of implementing the external api, you
   * just have to decide when to add and remove contacts.
   *
   * You can remove a Contact from an Ekiga::Book in two ways:
   *  - either by calling the remove_contact method,
   *  - or by emission of the contact's removed signal.
   *
   * Notice that this class won't take care of removing the Contact from a
   * backend -- only from the Ekiga::Book.
   * If you want the Contact <b>deleted</b> from the real backend, then you
   * probably should have an organization like:
   *  - the contact has a 'deleted' signal ;
   *  - the book listens to this signal ;
   *  - when the signal is received, then do a remove_contact followed by
   *    calling the appropriate api function to delete the contact in your
   *    backend.
   */
  template<typename ContactType = Contact>
  class BookImpl:
    public Book,
    protected RefLister<ContactType>
  {

  public:

    typedef typename RefLister<ContactType>::iterator iterator;
    typedef typename RefLister<ContactType>::const_iterator const_iterator;

    /** The constructor
     */
    BookImpl ();

    /** The destructor.
     */
    ~BookImpl ();


    /** Visit all contacts of the book and trigger the given callback.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_contacts (sigc::slot1<bool, ContactPtr > visitor);

  protected:

    /** Returns an iterator to the first Contact of the collection
     */
    iterator begin ();

    /** Returns an iterator to the last Contact of the collection
     */
    iterator end ();

    /** Returns a const iterator to the first Contact of the collection
     */
    const_iterator begin () const;

    /** Returns a const iterator to the last Contact of the collection
     */
    const_iterator end () const;

    /** Adds a contact to the Ekiga::Book.
     * @param: The contact to be added.
     * @return: The Ekiga::Book 'contact_added' signal is emitted when the contact
     * has been added. The Ekiga::Book 'contact_updated' signal will be emitted
     * when the contact has been updated and the Ekiga::Book 'contact_removed' signal
     * will be emitted when the contact has been removed from the Ekiga::Book.
     */
    void add_contact (gmref_ptr<ContactType> contact);


    /** Removes a contact from the Ekiga::Book.
     * @param: The contact to be removed.
     * @return: The Ekiga::Book 'contact_removed' signal is emitted when the contact
     * has been removed.
     */
    void remove_contact (gmref_ptr<ContactType> contact);

    using RefLister<ContactType>::add_connection;
  };

/**
 * @}
 */

};


/* here begins the code from the template functions */

template<typename ContactType>
Ekiga::BookImpl<ContactType>::BookImpl ()
{
  /* this is signal forwarding */
  RefLister<ContactType>::object_added.connect (contact_added.make_slot ());
  RefLister<ContactType>::object_removed.connect (contact_removed.make_slot ());
  RefLister<ContactType>::object_updated.connect (contact_updated.make_slot ());
}


template<typename ContactType>
Ekiga::BookImpl<ContactType>::~BookImpl ()
{
}


template<typename ContactType>
void
Ekiga::BookImpl<ContactType>::visit_contacts (sigc::slot1<bool, ContactPtr > visitor)
{
  RefLister<ContactType>::visit_objects (visitor);
}


template<typename ContactType>
typename Ekiga::BookImpl<ContactType>::iterator
Ekiga::BookImpl<ContactType>::begin ()
{
  return RefLister<ContactType>::begin ();
}


template<typename ContactType>
typename Ekiga::BookImpl<ContactType>::iterator
Ekiga::BookImpl<ContactType>::end ()
{
  return RefLister<ContactType>::end ();
}


template<typename ContactType>
typename Ekiga::BookImpl<ContactType>::const_iterator
Ekiga::BookImpl<ContactType>::begin () const
{
  return RefLister<ContactType>::begin ();
}


template<typename ContactType>
typename Ekiga::BookImpl<ContactType>::const_iterator
Ekiga::BookImpl<ContactType>::end () const
{
  return RefLister<ContactType>::end ();
}


template<typename ContactType>
void
Ekiga::BookImpl<ContactType>::add_contact (gmref_ptr<ContactType> contact)
{
  contact->questions.add_handler (questions.make_slot ());
  add_object (contact);
}


template<typename ContactType>
void
Ekiga::BookImpl<ContactType>::remove_contact (gmref_ptr<ContactType> contact)
{
  remove_object (contact);
}

#endif
