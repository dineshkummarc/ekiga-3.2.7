
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

#include <glib.h>
#include <iostream>

#include "config.h"
#include "gmconf.h"
#include "gmconf-personal-details.h"

static void
display_name_changed_nt (G_GNUC_UNUSED gpointer id,
                         GmConfEntry* entry,
                         gpointer data)
{
  Gmconf::PersonalDetails *details = (Gmconf::PersonalDetails *) data;
  const gchar* val = gm_conf_entry_get_string (entry);

  if (val != NULL)
    details->display_name_changed (val);
}

static void
presence_changed_nt (G_GNUC_UNUSED gpointer id,
                     GmConfEntry* entry,
                     gpointer data)
{
  Gmconf::PersonalDetails *details = (Gmconf::PersonalDetails *) data;
  const gchar* val = gm_conf_entry_get_string (entry);

  if (val != NULL)
    details->presence_changed (val);
}

static void
status_changed_nt (G_GNUC_UNUSED gpointer id,
                   GmConfEntry* entry,
                   gpointer data)
{
  Gmconf::PersonalDetails *details = (Gmconf::PersonalDetails *) data;
  const gchar* val = gm_conf_entry_get_string (entry);

  if (val != NULL)
    details->status_changed (val);
}

Gmconf::PersonalDetails::PersonalDetails ()
{
  gchar* str = NULL;

  display_name_notifier
    = gm_conf_notifier_add ("/apps/" PACKAGE_NAME "/general/personal_data/full_name",
                            display_name_changed_nt, this);
  presence_notifier
    = gm_conf_notifier_add ("/apps/" PACKAGE_NAME "/general/personal_data/short_status",
                            presence_changed_nt, this);
  status_notifier
    = gm_conf_notifier_add ("/apps/" PACKAGE_NAME "/general/personal_data/long_status",
                            status_changed_nt, this);

  str = gm_conf_get_string ("/apps/" PACKAGE_NAME "/general/personal_data/full_name");
  if (str != NULL) {

    display_name = str;
    g_free (str);
  } else
    display_name = "";

  str = gm_conf_get_string ("/apps/" PACKAGE_NAME "/general/personal_data/short_status");
  if (str != NULL) {

    presence = str;
    g_free (str);
  } else
    presence = "";

  str = gm_conf_get_string ("/apps/" PACKAGE_NAME "/general/personal_data/long_status");
  if (str != NULL) {

    status = str;
    g_free (str);
  } else
    status = "";
}

Gmconf::PersonalDetails::~PersonalDetails ()
{
  gm_conf_notifier_remove (display_name_notifier);
  gm_conf_notifier_remove (presence_notifier);
  gm_conf_notifier_remove (status_notifier);
}

const std::string
Gmconf::PersonalDetails::get_display_name () const
{
  return display_name;
}

const std::string
Gmconf::PersonalDetails::get_presence () const
{
  return presence;
}

const std::string
Gmconf::PersonalDetails::get_status () const
{
  return status;
}

void
Gmconf::PersonalDetails::set_display_name (const std::string display_name_)
{
  gm_conf_set_string ("/apps/" PACKAGE_NAME "/general/personal_data/full_name",
                      display_name_.c_str ());
}

void
Gmconf::PersonalDetails::set_presence (const std::string presence_)
{
  gm_conf_set_string ("/apps/" PACKAGE_NAME "/general/personal_data/short_status",
                      presence_.c_str ());
}

void
Gmconf::PersonalDetails::set_status (const std::string status_)
{
  gm_conf_set_string ("/apps/" PACKAGE_NAME "/general/personal_data/long_status",
                      status_.c_str ());
}

void
Gmconf::PersonalDetails::set_presence_info (const std::string _presence,
                                            const std::string _status)
{
  presence = _presence;
  status = _status;

  set_presence (_presence);
  set_status (_status);

  updated.emit ();
}

void
Gmconf::PersonalDetails::display_name_changed (std::string val)
{
  display_name = val;
  updated.emit ();
}

void
Gmconf::PersonalDetails::presence_changed (std::string val)
{
  if (presence != val) {
    presence = val;
    updated.emit ();
  }
}

void
Gmconf::PersonalDetails::status_changed (std::string val)
{
  if (status != val) {
    status = val;
    updated.emit ();
  }
}
