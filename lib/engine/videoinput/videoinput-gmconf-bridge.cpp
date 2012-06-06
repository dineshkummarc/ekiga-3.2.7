
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
 *                         videoinput-gmconf-bridge.cpp -  description
 *                         -------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the bridge between the gmconf
 *                          and the videoinput-core..
 *
 */

#include "config.h"

#include "videoinput-gmconf-bridge.h"
#include "videoinput-core.h"

#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"
#define VIDEO_CODECS_KEY  "/apps/" PACKAGE_NAME "/codecs/video/"

using namespace Ekiga;

VideoInputCoreConfBridge::VideoInputCoreConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &VideoInputCoreConfBridge::on_property_changed));

  keys.push_back (VIDEO_DEVICES_KEY "size"); 
  keys.push_back (VIDEO_DEVICES_KEY "max_frame_rate"); 
  keys.push_back (VIDEO_DEVICES_KEY "input_device"); 
  keys.push_back (VIDEO_DEVICES_KEY "channel"); 
  keys.push_back (VIDEO_DEVICES_KEY "format"); 
  keys.push_back (VIDEO_DEVICES_KEY "image"); 
  keys.push_back (VIDEO_DEVICES_KEY "enable_preview");
  load (keys);
}

void VideoInputCoreConfBridge::on_property_changed (std::string key, GmConfEntry * /*entry*/)
{
  VideoInputCore & videoinput_core = (VideoInputCore &) service;

  if ( (key == VIDEO_DEVICES_KEY "size") ||
       (key == VIDEO_DEVICES_KEY "max_frame_rate") ) {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating preview size and fps");

    unsigned size = gm_conf_get_int (VIDEO_DEVICES_KEY "size");
    if (size >= NB_VIDEO_SIZES) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "size" << " out of range, ajusting to 0");
      size = 0;
    }

    unsigned max_frame_rate = gm_conf_get_int (VIDEO_DEVICES_KEY "max_frame_rate");
    if ( (max_frame_rate < 1) || (max_frame_rate > 30) ) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "max_frame_rate" << " out of range, ajusting to 30");
      max_frame_rate = 30;
    }
    videoinput_core.set_preview_config (VideoSizes[size].width,
                                        VideoSizes[size].height,
                                        max_frame_rate);
  }
  else if ( (key == VIDEO_DEVICES_KEY "input_device") ||
            (key == VIDEO_DEVICES_KEY "channel") ||
            (key == VIDEO_DEVICES_KEY "format") ) {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating device");

    VideoInputDevice device;
    gchar *input_device = NULL;
    input_device = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
    if (input_device == NULL) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "input_device" << " is NULL");
    }
    else {
      device.SetFromString(input_device);
      g_free (input_device);
    }

    if ( (device.type == "" )   ||
         (device.source == "")  ||
         (device.name == "" ) ) {
      PTRACE(1, "VidinputCore\tTried to set malformed device");
      device.type = VIDEO_INPUT_FALLBACK_DEVICE_TYPE;
      device.source = VIDEO_INPUT_FALLBACK_DEVICE_SOURCE;
      device.name = VIDEO_INPUT_FALLBACK_DEVICE_NAME;
    }

    unsigned video_format = gm_conf_get_int (VIDEO_DEVICES_KEY "format");
    if (video_format >= VI_FORMAT_MAX) {
      PTRACE(1, "VidInputCoreConfBridge\t" << VIDEO_DEVICES_KEY "format" << " out of range, ajusting to 3");
      video_format = 3;
    }

    videoinput_core.set_device (device, gm_conf_get_int (VIDEO_DEVICES_KEY "channel") ,(VideoInputFormat) video_format);
  }
  else if (key == VIDEO_DEVICES_KEY "enable_preview") {

    PTRACE(4, "VidInputCoreConfBridge\tUpdating preview");
    if (gm_conf_get_bool ( VIDEO_DEVICES_KEY "enable_preview"))
        videoinput_core.start_preview(); 
      else
        videoinput_core.stop_preview();
  }
  else if (key == VIDEO_DEVICES_KEY "image") {
    PTRACE(4, "VidInputCoreConfBridge\tUpdating image");
  }
}

