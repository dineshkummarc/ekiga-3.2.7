
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
 *                         kickstart.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Julien Puydt
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : implementation of a kickstart object
 *
 */

#include "kickstart.h"

#define KICKSTART_DEBUG 0

#include <algorithm>

#if KICKSTART_DEBUG
#include <iostream>
#endif

Ekiga::KickStart::KickStart ()
{
}

Ekiga::KickStart::~KickStart ()
{
#if KICKSTART_DEBUG
  std::cout << "KickStart(final log):"
	    << std::endl;

  std::cout << "\tBLANK: ";
  for (std::list<gmref_ptr<Spark> >::iterator iter = blanks.begin ();
       iter != blanks.end ();
       ++iter) {
    std::cout << (*iter)->get_name () << ", ";
  }
  std::cout << std::endl;

  std::cout << "\tPARTIAL: ";
  for (std::list<gmref_ptr<Spark> >::iterator iter = partials.begin ();
       iter != partials.end ();
       ++iter) {
    std::cout << (*iter)->get_name () << ", ";
  }
  std::cout << std::endl;
#endif
}

void
Ekiga::KickStart::add_spark (gmref_ptr<Ekiga::Spark>& spark)
{
  blanks.push_back (spark);
#if KICKSTART_DEBUG
  std::cout << "KickStart(add_spark): " << spark->get_name () << std::endl;
#endif
}

void
Ekiga::KickStart::kick (Ekiga::ServiceCore& core,
			int* argc,
			char** argv[])
{
  std::list<std::string> disabled;
  bool went_on;

  for (int arg = 2; arg <= *argc; arg++) {

    std::string argument = (*argv)[arg - 1];
    if (argument.find ("--kickstart-disabled=") == 0) {

      std::string::size_type last_pos = argument.find_first_of ('=') + 1;
      std::string::size_type pos = argument.find_first_of (',', last_pos);

      while (pos != std::string::npos || last_pos != std::string::npos) {

	disabled.push_back (argument.substr (last_pos, pos - last_pos));
	last_pos = argument.find_first_not_of (',', pos);
	pos = argument.find_first_of (',', last_pos);
      }
    }
  }

  // this makes sure we loop only if something needs to be done
  went_on = !(blanks.empty () && partials.empty ());

  // we are going to try things as long as something happens
  while (went_on) {

    went_on = false;

    { // first try the blanks
      std::list<gmref_ptr<Spark> > temp;
      temp.swap (blanks);

#if KICKSTART_DEBUG
      if (!temp.empty ())
	std::cout << "KickStart(kick): looping on BLANK sparks" << std::endl;
#endif

      for (std::list<gmref_ptr<Spark> >::iterator iter = temp.begin ();
	   iter != temp.end ();
	   ++iter) {

	bool result = false;
	if (std::find (disabled.begin (),
		       disabled.end (), (*iter)->get_name ())
	    == disabled.end ()) {

	  result = (*iter)->try_initialize_more (core, argc, argv);
	} else {

#if KICKSTART_DEBUG
	  std::cout << "KickStart(kick): " << (*iter)->get_name ()
		    << " is disabled" << std::endl;
#endif
	}

	if (result) {

	  went_on = true;
	  switch ((*iter)->get_state ()) {

	  case Spark::BLANK:

	    // shouldn't happen!
	    blanks.push_back (*iter);
	    break;
	  case Spark::PARTIAL:

#if KICKSTART_DEBUG
	    std::cout << "KickStart(kick): "
		      << (*iter)->get_name ()
		      << " was promoted to PARTIAL"
		      << std::endl;
#endif
	    partials.push_back (*iter);
	    break;

	  case Spark::FULL:

	    // good!
#if KICKSTART_DEBUG
	    std::cout << "KickStart(kick): "
		      << (*iter)->get_name ()
		      << " was promoted to FULL"
		      << std::endl;
#endif
	    break;

	  default:

	    // shouldn't happen
	    break;
	  }

	} else {

#if KICKSTART_DEBUG
	    std::cout << "KickStart(kick): "
		      << (*iter)->get_name ()
		      << " is still BLANK"
		      << std::endl;
#endif
	  blanks.push_back (*iter);
	}
      }
    }

    { // then try the partials
      std::list<gmref_ptr<Spark> > temp;
      temp.swap (partials);

#if KICKSTART_DEBUG
      if (!temp.empty ())
	std::cout << "KickStart(kick): looping on PARTIAL sparks" << std::endl;
#endif

      for (std::list<gmref_ptr<Spark> >::iterator iter = temp.begin ();
	   iter != temp.end ();
	   ++iter) {

	bool result = (*iter)->try_initialize_more (core, argc, argv);

	if (result) {

	  went_on = true;
	  switch ((*iter)->get_state ()) {

	  case Spark::BLANK:

	    // can't happen unless it went back
	    break;
	  case Spark::PARTIAL:

#if KICKSTART_DEBUG
	    std::cout << "KickStart(kick): "
		      << (*iter)->get_name ()
		      << " was promoted to PARTIAL"
		      << std::endl;
#endif
	    partials.push_back (*iter);
	    break;

	  case Spark::FULL:

	    // good!
#if KICKSTART_DEBUG
	    std::cout << "KickStart(kick): "
		      << (*iter)->get_name ()
		      << " was promoted to FULL"
		      << std::endl;
#endif
	    break;

	  default:

	    // shouldn't happen
	    break;
	  }

	} else {

#if KICKSTART_DEBUG
	    std::cout << "KickStart(kick): "
		      << (*iter)->get_name ()
		      << " is still PARTIAL"
		      << std::endl;
#endif
	  partials.push_back (*iter);
	}
      }
    }
  }
}
