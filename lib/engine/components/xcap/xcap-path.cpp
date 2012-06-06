
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
 *                         xcap-path.h  -  description
 *                         ------------------------------------
 *   begin                : Sun 12 October 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Interface of an XCAP path
 *
 */

#include "xcap-path.h"

#include <sstream>

XCAP::Path::Path (std::string root_,
		  std::string application_,
		  std::string user_):
  root(root_), application(application_), user(user_)
{
}

XCAP::Path::~Path ()
{}

std::string
XCAP::Path::to_uri () const
{
  std::string uri;

  uri = root + "/" + application;

  if ( !user.empty ())
    uri = uri + "/users/" + user;
  else
    uri = uri + "/global";
 
  uri = uri + "/index/~~" + relative;

  return uri;
}

const std::string
XCAP::Path::get_username () const
{
  return username;
}

const std::string
XCAP::Path::get_password () const
{
  return password;
}

void
XCAP::Path::set_credentials (std::string username_,
			     std::string password_)
{
  username = username_;
  password = password_;
}

gmref_ptr<XCAP::Path>
XCAP::Path::build_child (const std::string name)
{
  gmref_ptr<Path> result (new Path (root, application, user,
				    relative, username, password));
  result->relative = relative + "/" + name;

  return result;
}

gmref_ptr<XCAP::Path>
XCAP::Path::build_child_with_attribute (const std::string name,
					const std::string attr,
					const std::string value)
{
  gmref_ptr<Path> result (new Path (root, application, user, relative,
				    username, password));
  result->relative = relative + "/" + name +"%5b@" + attr + "=%22" + value + "%22%5d";

  return result;
}

gmref_ptr<XCAP::Path>
XCAP::Path::build_child_with_position (const std::string name,
				       int position)
{
  std::ostringstream stream;
  gmref_ptr<Path> result (new Path (root, application, user, relative,
				    username, password));

  /* notice: Snark hates C++ */
  stream << name << "%5b" << position << "%5d";
  result->relative = relative + "/" + stream.str ();

  return result;
}

XCAP::Path::Path (std::string root_,
		  std::string application_,
		  std::string user_,
		  std::string relative_,
		  std::string username_,
		  std::string password_):
  root(root_), application(application_), user(user_), relative(relative_),
  username(username_), password(password_)
{
}
