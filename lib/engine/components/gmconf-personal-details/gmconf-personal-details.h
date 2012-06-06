
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
 *                         gmconf-personal-details.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of the representation of personal details
 *                          using gmconf
 *
 */

#ifndef __GMCONF_PERSONAL_DETAILS_H__
#define __GMCONF_PERSONAL_DETAILS_H__

#include "personal-details.h"

namespace Gmconf
{
  class PersonalDetails: public Ekiga::PersonalDetails
  {
  public:

    PersonalDetails ();

    virtual ~PersonalDetails ();

    /*** Service Implementation ***/
  public:

    const std::string get_name () const
    { return "personal-details"; }

    const std::string get_description () const
    { return "\tPersonal details management object (using gmconf)"; }

    /*** PersonalDetails Implementation ***/
  public:

    const std::string get_display_name () const;

    const std::string get_presence () const;

    const std::string get_status () const;

    void set_display_name (const std::string display_name);

    void set_presence (const std::string presence);

    void set_status (const std::string status);

    void set_presence_info (const std::string presence, 
                            const std::string status);

    /*** public but only to be called by C callbacks ***/
  public:
    void display_name_changed (std::string val);
    void presence_changed (std::string val);
    void status_changed (std::string val);

  private:
    void* display_name_notifier;
    void* presence_notifier;
    void* status_notifier;
    std::string display_name;
    std::string presence;
    std::string status;
  };
};

#endif
