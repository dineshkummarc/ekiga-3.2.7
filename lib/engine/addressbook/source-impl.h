
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
 *                         source-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of a partial
 *                          implementation of an addressbook
 *                          implementation backend
 *
 */

#ifndef __SOURCE_IMPL_H__
#define __SOURCE_IMPL_H__

#include "reflister.h"
#include "source.h"



namespace Ekiga
{

  /**
   * @addtogroup contacts
   * @{
   */

  /** Generic implementation for the Ekiga::Source abstract class.
   *
   * This class is there to make it easy to implement a new type of
   * addressbook source: it will take care of implementing the external api,
   * you just have to decide when to add and remove books.
   *
   * It also provides basic memory management for books, with the second
   * (optional) template argument:
   *  - either no management (the default) ;
   *  - or the book is considered bound to one Ekiga::Source, which will
   *    trigger its destruction (using delete) when removed from it.
   *
   * You can remove a Book from an Ekiga::Source in two ways:
   *  - either by calling the remove_book method,
   *  - or by emission of the book's removed signal.
   *
   * Notice that this class won't take care of removing the book from a
   * backend -- only from the Ekiga::Source.
   * If you want the Book <b>deleted</b> from the real backend, then you
   * probably should have an organization like :
   *  - the book has a 'deleted' signal;
   *  - the source listens for this signal;
   *  - when the signal is received, then do a remove_book followed by calling
   *    the appropriate api function to delete the Book in your backend.
   */
  template<typename BookType = Book>
  class SourceImpl:
    public Source,
    protected RefLister<BookType>
  {

  public:

    typedef typename RefLister<BookType>::iterator iterator;
    typedef typename RefLister<BookType>::const_iterator const_iterator;

    /** The constructor
     */
    SourceImpl ();

    /** The destructor.
     */
    ~SourceImpl ();


    /** Visit all books of the source and trigger the given callback.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_books (sigc::slot1<bool, BookPtr > visitor);

  protected:

    /** Adds a book to the Ekiga::Source.
     * @param: The Ekiga::Book to be added.
     * @return: The Ekiga::Source 'book_added' signal is emitted when the
     * Ekiga::Book has been added. The Ekiga::Source 'book_updated' signal will
     * be emitted when the Ekiga::Book has been updated and the Ekiga::Source
     * 'book_removed' signal will be emitted when the Ekiga::Book has been
     * removed from the Ekiga::Source.
     */
    void add_book (gmref_ptr<BookType> book);


    /** Removes a book from the Ekiga::Source.
     * @param: The Ekiga::Book to be removed.
     * @return: The Ekiga::Source 'book_removed' signal is emitted when the
     * Ekiga::Book has been removed.
     */
    void remove_book (gmref_ptr<BookType> book);

    using RefLister<BookType>::add_connection;

  protected:

    /** Returns an iterator to the first Book of the collection
     */
    iterator begin ();

    /** Returns an iterator to the last Book of the collection
     */
    iterator end ();

    /** Returns a const iterator to the first Book of the collection
     */
    const_iterator begin () const;

    /** Returns a const iterator to the last Book of the collection
     */
    const_iterator end () const;


  private:

    /** Disconnects the signals for the Ekiga::Book, emits the 'book_removed'
     * signal on the Ekiga::Source and takes care of the release of that
     * Ekiga::Book.
     * @param: The Book to remove.
     */
    void common_removal_steps (gmref_ptr<BookType> book);


    /** This callback is triggered when the 'updated' signal is emitted on an
     * Ekiga::Book. Emits the Ekiga::Source 'book_updated' signal for that
     * Ekiga::Book.
     * @param: The updated book.
     */
    void on_book_updated (gmref_ptr<BookType> book);


    /** This callback is triggered when the 'removed' signal is emitted on an
     * Ekiga::Book. Emits the Ekiga::Source 'book_removed' signal for that book
     * and takes care of the deletion of the book.
     * @param: The removed book.
     */
    void on_book_removed (gmref_ptr<BookType> book);
  };

  /**
   * @}
   */

};


/* here comes the implementation of the template functions */


template<typename BookType>
Ekiga::SourceImpl<BookType>::SourceImpl ()
{
  /* signal forwarding */
  RefLister<BookType>::object_added.connect (book_added.make_slot ());
  RefLister<BookType>::object_removed.connect (book_removed.make_slot ());
  RefLister<BookType>::object_updated.connect (book_updated.make_slot ());
}

template<typename BookType>
Ekiga::SourceImpl<BookType>::~SourceImpl ()
{
}


template<typename BookType>
void
Ekiga::SourceImpl<BookType>::visit_books (sigc::slot1<bool, BookPtr > visitor)
{
  RefLister<BookType>::visit_objects (visitor);
}


template<typename BookType>
void
Ekiga::SourceImpl<BookType>::add_book (gmref_ptr<BookType> book)
{
  add_object (book);

  add_connection (book, book->contact_added.connect (sigc::bind<0> (contact_added.make_slot (), book)));

  add_connection (book, book->contact_removed.connect (sigc::bind<0> (contact_removed.make_slot (), book)));

  add_connection (book, book->contact_updated.connect (sigc::bind<0> (contact_updated.make_slot (), book)));

  add_connection (book, book->questions.add_handler (questions.make_slot ()));
}


template<typename BookType>
void
Ekiga::SourceImpl<BookType>::remove_book (gmref_ptr<BookType> book)
{
  remove_object (book);
}

template<typename BookType>
typename Ekiga::SourceImpl<BookType>::iterator
Ekiga::SourceImpl<BookType>::begin ()
{
  return RefLister<BookType>::begin ();
}


template<typename BookType>
typename Ekiga::SourceImpl<BookType>::iterator
Ekiga::SourceImpl<BookType>::end ()
{
  return RefLister<BookType>::end ();
}


template<typename BookType>
typename Ekiga::SourceImpl<BookType>::const_iterator
Ekiga::SourceImpl<BookType>::begin () const
{
  return RefLister<BookType>::begin ();
}


template<typename BookType>
typename Ekiga::SourceImpl<BookType>::const_iterator
Ekiga::SourceImpl<BookType>::end () const
{
  return RefLister<BookType>::end ();
}

#endif
