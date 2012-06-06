
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
 *                         personal-details.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of the representation of personal details
 *
 */

#ifndef __PERSONAL_DETAILS_H__
#define __PERSONAL_DETAILS_H__

#include <string>
#include <sigc++/sigc++.h>

#include "services.h"

namespace Ekiga
{
  class PersonalDetails: public Service
  {
  public:
    PersonalDetails () {}

    virtual ~PersonalDetails () {}

    virtual const std::string get_display_name () const = 0;

    virtual const std::string get_presence () const = 0;

    virtual const std::string get_status () const = 0;

    virtual void set_display_name (const std::string display_name) = 0;

    virtual void set_presence (const std::string presence) = 0;

    virtual void set_status (const std::string status) = 0;

    virtual void set_presence_info (const std::string presence,
                                    const std::string status) = 0;

    sigc::signal0<void> updated;
  };
};

#endif
