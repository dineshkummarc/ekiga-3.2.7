
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
 *                         echo-dialect.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of an echo chat backend
 *
 */

#ifndef __ECHO_DIALECT_H__
#define __ECHO_DIALECT_H__

#include "services.h"
#include "dialect-impl.h"
#include "echo-simple.h"

namespace Echo
{
  class Dialect:
    public Ekiga::Service,
    public Ekiga::DialectImpl<SimpleChat>
  {
  public:
    
    Dialect ();

    ~Dialect ();

    bool populate_menu (Ekiga::MenuBuilder &builder);

    const std::string get_name () const
    { return "echo-dialect"; }

    const std::string get_description () const
    { return "\tProvides an echo chat for testing purposes"; }

  private:

    void new_chat ();
  };

  typedef gmref_ptr<Dialect> DialectPtr;

};

#endif
