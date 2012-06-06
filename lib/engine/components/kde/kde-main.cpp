
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
 *                         kde-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : code to make ekiga a KApplication
 *
 */

#include "config.h"

#include "services.h"

#include "kde-main.h"

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>

struct KDESpark: public Ekiga::Spark
{
  KDESpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    gmref_ptr<Ekiga::Service> kde = core.get ("kde-core");

    if ( !kde) {

      KAboutData about(PACKAGE, PACKAGE, ki18n("PACKAGE_NAME"), PACKAGE_VERSION,
		       ki18n("VoIP application"),
		       KAboutData::License_Custom,
		       ki18n("Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>"));

      KCmdLineArgs::init (&about);
      new KApplication (false);
      core.add (gmref_ptr<Ekiga::Service> (new Ekiga::BasicService ("kde-core",
								    "\tKDE support")));
      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "KDE"; }

  bool result;

};

void
kde_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new KDESpark);
  kickstart.add_spark (spark);
}
