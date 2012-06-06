
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
 *                         chat-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the main chat managing object
 *
 */

#ifndef __CHAT_CORE_H__
#define __CHAT_CORE_H__

#include "services.h"
#include "dialect.h"

/* FIXME: probably it should have a decorator system, so we can for example
 * hook a logger
 */

namespace Ekiga
{
  /**
   * @defgroup chats Chats and protocols
   * @{
   */

  /*  Core object for text chat support.
   *
   * Notice that you give dialects to this object as references, so they won't
   * be freed here : it's up to you to free them somehow.
   */
  class ChatCore: public Service
  {
  public:

    /** The constructor.
     */
    ChatCore () {}

    /** The destructor.
     */
    ~ChatCore ();

    /*** service implementation ***/
  public:

    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
    { return "chat-core"; }

    /** Returns the description of the service.
     * @return: The service description.
     */
    const std::string get_description () const
    { return "\tChat managing object"; }

    /*** Public API ***/
  public:

    /** Adds a dialect to the ContactCore service.
     * @param The dialect to be added.
     */
    void add_dialect (DialectPtr dialect);

    /** Triggers a callback for all Ekiga::Dialect dialects of the
     * ChatCore service.
     * @param The callback (the return value means "go on" and allows stopping
     * the visit)
     */
    void visit_dialects (sigc::slot1<bool, DialectPtr > visitor);

    /** This signal is emitted when an Ekiga::Dialect has been added to
     * the ChatCore service.
     */
    sigc::signal1<void, DialectPtr > dialect_added;

  private:

    std::list<DialectPtr > dialects;

    /*** Misc ***/
  public:

    /** Create the menu for the ChatCore and its actions.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);

    /** This signal is emitted when the ChatCore service has been updated.
     */
    sigc::signal0<void> updated;

    /** This chain allows the ChatCore to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;

  };

  /**
   * @}
   */
};

#endif
