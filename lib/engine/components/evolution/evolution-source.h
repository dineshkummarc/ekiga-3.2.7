
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         evolution-source.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : interface of an evolution source
 *
 */

#ifndef __EVOLUTION_SOURCE_H__
#define __EVOLUTION_SOURCE_H__

#include <libebook/e-book.h>

#include "contact-core.h"
#include "source-impl.h"

#include "evolution-book.h"

namespace Evolution
{

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Source:
    public Ekiga::Service,
    public Ekiga::SourceImpl<Book>
  {
  public:

    Source (Ekiga::ServiceCore &_services);

    ~Source ();

    bool populate_menu (Ekiga::MenuBuilder &builder);

    /* this object is an Ekiga::Service too */
    const std::string get_name () const
    { return "evolution-source"; }

    const std::string get_description () const
    { return "\tComponent bringing in gnome addressbooks"; }

    /* those should be private, but need to be called from C */

    void add_group (ESourceGroup *group);

    void remove_group (ESourceGroup *group);

  private:

    Ekiga::ServiceCore &services;
    ESourceList *source_list;
  };

/**
 * @}
 */

};

#endif
