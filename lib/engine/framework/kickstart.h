
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
 *                         kickstart.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2009 by Julien Puydt
 *   copyright            : (c) 2009 by Julien Puydt
 *   description          : declaration of a kickstart object
 *
 */

#ifndef __KICKSTART_H__
#define __KICKSTART_H__

/* We want the engine startup to be as automatic as possible -- in particular,
 * it should handle dependancies as transparently as possible (external plugins
 * may come at one point).
 *
 * This kickstart object works like this : some objects are registered to it,
 * and are responsible of initializing a portion of code. Those objects are
 * called sparks, and have basically three states :
 * - blank -- they haven't managed to initialize anything yet ;
 * - partial -- they have initialized something, but could do more ;
 * - full -- they have initialized all they could.
 *
 * This means that a spark object will have a method which will try to
 * initialize more, and a method which will tell in which state it is : indeed,
 * the kickstart object will need to know when it reached a point where all
 * sparks have done as much as they could, so we need a strictly increasing
 * function up until then.
 *
 * We want the following to be guaranteed by each Spark implementation :
 * - if try_initialize_more returns 'true', then the state should be at least
 * partial ;
 * - try_initialize_more shouldn't return 'true' if no new service could be
 * registered ;
 * - states should always evolve as BLANK -> PARTIAL -> FULL : no coming back!
 */

#include "services.h"

namespace Ekiga
{

  struct Spark: public virtual GmRefCounted
  {
    typedef enum { BLANK, PARTIAL, FULL } state;

    virtual ~Spark () {}

    virtual bool try_initialize_more (ServiceCore& core,
				      int* argc,
				      char** argv[]) = 0;

    virtual state get_state () const = 0;

    // this method is useful for debugging purposes
    virtual const std::string get_name () const = 0;
  };

  class KickStart
  {
  public:

    KickStart ();

    ~KickStart ();

    void add_spark (gmref_ptr<Spark>& spark);

    /* try to do more with the known blank/partial sparks */
    void kick (Ekiga::ServiceCore& core,
	       int* argc,
	       char** argv[]);

  private:
    std::list<gmref_ptr<Spark> > blanks;
    std::list<gmref_ptr<Spark> > partials;
  };
};

#endif
