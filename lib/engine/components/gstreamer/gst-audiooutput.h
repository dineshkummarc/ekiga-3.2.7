
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
 *                         gst-audiooutput.h  -  description
 *                         ------------------------------------
 *   begin                : Sat 27 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Gstreamer audio output code
 *
 */

#ifndef __GST_AUDIOOUTPUT_H__
#define __GST_AUDIOOUTPUT_H__

#include "audiooutput-manager.h"
#include <gst/gst.h>

#include <map>

namespace GST
{
  class AudioOutputManager: public Ekiga::AudioOutputManager
  {
  public:

    AudioOutputManager ();

    ~AudioOutputManager ();

    void get_devices (std::vector<Ekiga::AudioOutputDevice>& devices);

    bool set_device (Ekiga::AudioOutputPS ps,
		     const Ekiga::AudioOutputDevice& device);

    bool open (Ekiga::AudioOutputPS ps,
	       unsigned channels,
	       unsigned samplerate,
	       unsigned bits_per_sample);

    void close (Ekiga::AudioOutputPS ps);

    void set_buffer_size (Ekiga::AudioOutputPS ps,
			  unsigned buffer_size,
			  unsigned num_buffers);

    bool set_frame_data (Ekiga::AudioOutputPS ps,
			 const char* data,
			 unsigned size,
			 unsigned& written);

    void set_volume (Ekiga::AudioOutputPS ps,
		     unsigned volume);

    bool has_device (const std::string& source,
		     const std::string& device_name,
		     Ekiga::AudioOutputDevice& device);
  private:

    void detect_devices ();
    void detect_fakesink_devices ();
    void detect_alsasink_devices ();
    void detect_pulsesink_devices ();
    void detect_sdlsink_devices ();

    /* we take a user-readable name, and get the string describing
     * the actual device */
    std::map<std::pair<std::string, std::string>, std::string> devices_by_name;

    GstElement* pipeline[2];
  };
};

#endif
