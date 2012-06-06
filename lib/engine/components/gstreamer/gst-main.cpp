
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
 *                         gst-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : code to hook the gstreamer code to the main program
 *
 */

#include "gst-main.h"

#include "videoinput-core.h"
#include "audioinput-core.h"
#include "audiooutput-core.h"

#include "gst-videoinput.h"
#include "gst-audioinput.h"
#include "gst-audiooutput.h"
#include "gst-videoinput.h"

bool
gstreamer_init (Ekiga::ServiceCore& core,
		int* argc,
		char** argv[])
{
  bool result = false;
  gmref_ptr<Ekiga::AudioInputCore> audioinput_core
    = core.get ("audioinput-core");
  gmref_ptr<Ekiga::AudioOutputCore> audiooutput_core
    = core.get ("audiooutput-core");
  gmref_ptr<Ekiga::VideoInputCore> videoinput_core
    = core.get ("videoinput-core");

  if (audioinput_core && audiooutput_core && videoinput_core) {

    gst_init (argc, argv);

    GST::VideoInputManager* video = new GST::VideoInputManager ();
    GST::AudioInputManager* audioin = new GST::AudioInputManager ();
    GST::AudioOutputManager* audioout = new GST::AudioOutputManager ();

    audioinput_core->add_manager (*audioin);
    audiooutput_core->add_manager (*audioout);
    videoinput_core->add_manager (*video);
    result = true;
  }

  return result;
}
