
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
 *                         source.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of an addressbook
 *                          implementation backend
 *
 */

#ifndef __SOURCE_H__
#define __SOURCE_H__

#include "book.h"

namespace Ekiga {

  class Source: public virtual GmRefCounted
  {
  public:

    /** The destructor.
     */
    virtual ~Source ()
    {}


    /** Visit all books of the source and trigger the given callback.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Source.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    virtual void visit_books (sigc::slot1<bool, BookPtr >) = 0;


    /** Create the menu for that source and its actions.
     * This function is purely virtual and should be implemented by
     * the descendant of the Ekiga::Source.
     * @param A MenuBuilder object to populate.
     */
    virtual bool populate_menu (MenuBuilder &) = 0;


    /** This signal is emitted when a Book has been added to the Source.
     */
    sigc::signal1<void, BookPtr > book_added;
    
    
    /** This signal is emitted when a Book has been updated in the Source.
     */
    sigc::signal1<void, BookPtr > book_updated;
    
    
    /** This signal is emitted when a Book has been removed in the Source.
     */
    sigc::signal1<void, BookPtr > book_removed;

    /** This signal is emitted when a Contact has been added to a book in
     *  this source.
     */
    sigc::signal2<void, BookPtr, ContactPtr > contact_added;

    /** This signal is emitted when a Contact has been removed from a book in
     *  this source.
     */
    sigc::signal2<void, BookPtr, ContactPtr > contact_removed;

    /** This signal is emitted when a Contact has been updated in a book in
     *  this source
     */
    sigc::signal2<void, BookPtr, ContactPtr > contact_updated;

    /** This chain allows the Source to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;
  };

  typedef gmref_ptr<Source> SourcePtr;
};

#endif
