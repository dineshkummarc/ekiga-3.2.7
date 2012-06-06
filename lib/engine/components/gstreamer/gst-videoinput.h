
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
 *                         gst-videoinput.h  -  description
 *                         ------------------------------------
 *   begin                : Wed 17 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Gstreamer video input code
 *
 */

#ifndef __GST_VIDEOINPUT_H__
#define __GST_VIDEOINPUT_H__

#include "videoinput-manager.h"
#include <gst/gst.h>
#include <map>

namespace GST
{
  class VideoInputManager: public Ekiga::VideoInputManager
  {
  public:

    VideoInputManager ();

    ~VideoInputManager ();

    void get_devices (std::vector<Ekiga::VideoInputDevice>& devices);

    bool set_device (const Ekiga::VideoInputDevice& device,
		     int channel,
		     Ekiga::VideoInputFormat format);

    bool open (unsigned width,
	       unsigned height,
	       unsigned fps);

    void close ();

    bool get_frame_data (char* data);

    bool has_device (const std::string& source,
		     const std::string& device_name,
		     unsigned capabilities,
		     Ekiga::VideoInputDevice& device);
  private:

    void detect_devices ();
    void detect_videotestsrc_devices ();
    void detect_v4l2src_devices ();
    void detect_dv1394src_devices ();
    void detect_crazy_devices ();

    /* we take a user-readable name, and get the string describing
     * the actual device */
    std::map<std::pair<std::string, std::string>, std::string> devices_by_name;

    GstElement* pipeline;
  };
};

#endif
