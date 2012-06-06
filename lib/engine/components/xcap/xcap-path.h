
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

#ifndef __XCAP_PATH_H__
#define __XCAP_PATH_H__

#include "gmref.h"

#include <string>
#include <list>

namespace XCAP
{
  class Path: public virtual GmRefCounted
  {
  public:

    /* if user is empty, then it's global */
    Path (std::string root,
	  std::string application,
	  std::string user);

    ~Path ();

    std::string to_uri () const;

    const std::string get_username () const;

    const std::string get_password () const;

    /* this sets what you need to connect to the server */
    void set_credentials (std::string username,
			  std::string password);

    gmref_ptr<Path> build_child (const std::string name);

    gmref_ptr<Path> build_child_with_attribute (const std::string name,
						const std::string attr,
						const std::string value);

    gmref_ptr<Path> build_child_with_position (const std::string name,
					       int position);

  private:

    Path (std::string root,
	  std::string application,
	  std::string user,
	  std::string relative,
	  std::string username,
	  std::string password);

    std::string root;
    std::string application;
    std::string user;
    std::string relative;

    std::string username;
    std::string password;
  };
};

#endif
