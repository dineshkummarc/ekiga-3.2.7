
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
 *                         gmconf-personal-details-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : code to hook gmconf personal details into the main program
 *
 */

#include "gmconf-personal-details-main.h"
#include "gmconf-personal-details.h"
#include <iostream>

bool 
gmconf_personal_details_init (Ekiga::ServiceCore &core,
                              int * /*argc*/,
                              char ** /*argv*/[])
{
  bool result = false;
  gmref_ptr<Ekiga::PersonalDetails> personal_details
    = core.get ("personal-details");

  if ( !personal_details) {

    gmref_ptr<Gmconf::PersonalDetails> gmconf_details (new Gmconf::PersonalDetails ());
    core.add (gmconf_details);
    result = true;
  }

  return result;
}
