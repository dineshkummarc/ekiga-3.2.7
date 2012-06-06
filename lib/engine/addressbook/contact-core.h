
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
 *                         contact-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : interface of the main contact managing object
 *
 */

#ifndef __CONTACT_CORE_H__
#define __CONTACT_CORE_H__

#include "services.h"
#include "source.h"

/* declaration of a few helper classes */
namespace Ekiga
{

/**
 * @defgroup contacts Address Book
 * @{
 */

  class ContactDecorator: public virtual GmRefCounted
  {
  public:

    virtual ~ContactDecorator ()
    {}

    virtual bool populate_menu (ContactPtr /*contact*/,
				const std::string /*uri*/,
				MenuBuilder& /*builder*/) = 0;
  };

  /** Core object for address book support.
   *
   * Notice that you give sources to this object as references, so they won't
   * be freed here : it's up to you to free them somehow.
   */
  class ContactCore:
    public Service
  {
  public:

    /** The constructor.
     */
    ContactCore ()
    {}

    /** The destructor.
     */
    ~ContactCore ();


    /*** Service Implementation ***/

    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
    { return "contact-core"; }


    /** Returns the description of the service.
     * @return The service description.
     */
    const std::string get_description () const
    { return "\tContact managing object"; }


    /*** Public API ***/

    /** Adds a source to the ContactCore service.
     * @param The source to be added.
     */
    void add_source (SourcePtr source);


    /** Triggers a callback for all Ekiga::Source sources of the
     * ContactCore service.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_sources (sigc::slot1<bool, SourcePtr > visitor);


    /** This signal is emitted when a Ekiga::Source has been
     * added to the ContactCore Service.
     */
    sigc::signal1<void, SourcePtr > source_added;

    /** This signal is emitted when a book has been added to one of
     * the sources
     */
    sigc::signal2<void, SourcePtr, BookPtr > book_added;

    /** This signal is emitted when a book has been removed from one of
     * the sources
     */
    sigc::signal2<void, SourcePtr, BookPtr > book_removed;

    /** This signal is emitted when a book has been updated in one of
     * the sources
     */
    sigc::signal2<void, SourcePtr, BookPtr > book_updated;

    /** This signal is emitted when a contact has been added to one of
     * the book of one of the sources
     */
    sigc::signal3<void, SourcePtr, BookPtr, ContactPtr > contact_added;

    /** This signal is emitted when a contact has been removed from one of
     * the book of one of the sources
     */
    sigc::signal3<void, SourcePtr, BookPtr, ContactPtr > contact_removed;

    /** This signal is emitted when a contact has been updated in one of
     * the book of one of the sources
     */
    sigc::signal3<void, SourcePtr, BookPtr, ContactPtr > contact_updated;

  private:

    std::list<SourcePtr > sources;

    /*** Contact Helpers ***/

  public:

    void add_contact_decorator (gmref_ptr<ContactDecorator> decorator);

    /** Create the menu for a given Contact and its actions.
     * @param The Ekiga::Contact for which the actions could be made available.
     * @param The uri for which actions could be made available.
     * @param A MenuBuilder object to populate.
     */
    bool populate_contact_menu (ContactPtr contact,
				const std::string uri,
                                MenuBuilder &builder);

  private:

    std::list<gmref_ptr<ContactDecorator> > contact_decorators;


    /*** Misc ***/

  public:

    /** Create the menu for the ContactCore and its actions.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);

    /** This signal is emitted when the ContactCore Service has been
     * updated.
     */
    sigc::signal0<void> updated;


    /** This chain allows the ContactCore to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;

    std::list<sigc::connection> conns;
  };

/**
 * @}
 */

};
#endif
