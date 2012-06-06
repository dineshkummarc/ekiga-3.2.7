
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
 *                         presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a presence entity
 *
 */

#ifndef __PRESENTITY_H__
#define __PRESENTITY_H__

#include <set>
#include <string>

#include "gmref.h"
#include "chain-of-responsibility.h"
#include "form-request.h"
#include "menu-builder.h"

namespace Ekiga
{

/**
 * @addtogroup presence
 * @{
 */

  /** A presentity is a piece of presence information for a single URI.
   */
  class Presentity: public virtual GmRefCounted
  {
  public:

    /** The destructor.
     */
    virtual ~Presentity () {}

    /** Returns the name of the Presentity.
     * @return The Presentity's name.
     */
    virtual const std::string get_name () const = 0;

    /** Returns the presence of the Presentity.
    * @return The Presentity's presence.
    */
    virtual const std::string get_presence () const = 0;

    /** Returns the status of the Presentity.
     * @return The Presentity's status.
     */
    virtual const std::string get_status () const = 0;

    /** Returns the avatar of the Presentity.
     * @return The Presentity's avatar.
     */
    virtual const std::string get_avatar () const = 0;

    /** Returns the set of groups the Presentity belongs to.
     * @return The Presentity's set of groups.
     */
    virtual const std::set<std::string> get_groups () const = 0;

    /** Populates a menu with the actions possible on the Presentity.
     * @param The builder to populate.
     */
    virtual bool populate_menu (MenuBuilder &) = 0;

    /** This signal is emitted when the Presentity has been updated.
     */
    sigc::signal0<void> updated;

    /** This signal is emitted when the Presentity has been removed.
     */
    sigc::signal0<void> removed;

    /** This chain allows the Presentity to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;
  };

  typedef gmref_ptr<Presentity> PresentityPtr;
/**
 * @}
 */
};

#endif
