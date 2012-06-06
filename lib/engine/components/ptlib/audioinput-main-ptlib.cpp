
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
 *                         audioinput-main-ptlib.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : code to hook the PTLIB audioinput manager
 *                          into the main program
 *
 */

#include "audioinput-main-ptlib.h"
#include "audioinput-core.h"
#include "audioinput-manager-ptlib.h"

struct PTLIBAUDIOINPUTSpark: public Ekiga::Spark
{
  PTLIBAUDIOINPUTSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    gmref_ptr<Ekiga::AudioInputCore> audioinput_core = core.get ("audioinput-core");

    if (audioinput_core) {

      GMAudioInputManager_ptlib *audioinput_manager = new GMAudioInputManager_ptlib(core);

      audioinput_core->add_manager (*audioinput_manager);
      core.add (gmref_ptr<Ekiga::Service> (new Ekiga::BasicService ("ptlib-audio-input",
								    "\tComponent bringing PTLIB's audio input")));
      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "PTLIBAUDIOINPUT"; }

  bool result;
};

void
audioinput_ptlib_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new PTLIBAUDIOINPUTSpark);
  kickstart.add_spark (spark);
}
