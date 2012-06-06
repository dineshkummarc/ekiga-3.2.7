
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
 *                         videooutput-gmconf-bridge.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the bridge between the gmconf 
 *                          and the videooutput-core.
 *
 */

#include "config.h"

#include "videooutput-gmconf-bridge.h"
#include "videooutput-core.h"

#define USER_INTERFACE_KEY "/apps/" PACKAGE_NAME "/general/user_interface/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"

using namespace Ekiga;

VideoOutputCoreConfBridge::VideoOutputCoreConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &VideoOutputCoreConfBridge::on_property_changed));

  keys.push_back (VIDEO_DISPLAY_KEY "video_view"); 
  keys.push_back (VIDEO_DISPLAY_KEY "zoom"); 
  keys.push_back (VIDEO_DISPLAY_KEY "stay_on_top"); 
  keys.push_back (VIDEO_DISPLAY_KEY "disable_hw_accel"); 
  keys.push_back (VIDEO_DISPLAY_KEY "allow_pip_sw_scaling"); 
  keys.push_back (VIDEO_DISPLAY_KEY "sw_scaling_algorithm"); 

  load (keys);
}

void VideoOutputCoreConfBridge::on_property_changed (std::string key, GmConfEntry */*entry*/)
{
  VideoOutputCore & display_core = (VideoOutputCore &) service;
  if (key == VIDEO_DISPLAY_KEY "video_view")  {

    DisplayInfo display_info;
    PTRACE(4, "VideoOutputCoreConfBridge\tUpdating video view");

    if (( gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") < 0) || ( gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view") > 4))
      gm_conf_set_int (VIDEO_DISPLAY_KEY "video_view", 0);

    display_info.mode = (VideoOutputMode) gm_conf_get_int (VIDEO_DISPLAY_KEY "video_view");
    display_core.set_display_info(display_info);
  }
  else if ( (key == VIDEO_DISPLAY_KEY "zoom") ) {

    DisplayInfo display_info;
    PTRACE(4, "VideoOutputCoreConfBridge\tUpdating zoom");
      
    display_info.zoom = gm_conf_get_int (VIDEO_DISPLAY_KEY "zoom");
    if ((display_info.zoom != 100) && (display_info.zoom != 50) && (display_info.zoom != 200)) {
      display_info.zoom = 100;
      gm_conf_set_int (VIDEO_DISPLAY_KEY "zoom", 100);
    }

    display_core.set_display_info(display_info);

  }
  else {

    PTRACE(4, "VideoOutputCoreConfBridge\tUpdating Video Settings");
    DisplayInfo display_info;

    display_info.on_top = gm_conf_get_bool (VIDEO_DISPLAY_KEY "stay_on_top");
    display_info.disable_hw_accel = gm_conf_get_bool (VIDEO_DISPLAY_KEY "disable_hw_accel");
    display_info.allow_pip_sw_scaling = gm_conf_get_bool (VIDEO_DISPLAY_KEY "allow_pip_sw_scaling");
    display_info.sw_scaling_algorithm = gm_conf_get_int (VIDEO_DISPLAY_KEY "sw_scaling_algorithm");
    if (display_info.sw_scaling_algorithm > 3) {
      display_info.sw_scaling_algorithm = 0;
      gm_conf_set_int (VIDEO_DISPLAY_KEY "sw_scaling_algorithm", 0);
    }
    display_info.config_info_set = TRUE;

    display_core.set_display_info(display_info);
  }
}

