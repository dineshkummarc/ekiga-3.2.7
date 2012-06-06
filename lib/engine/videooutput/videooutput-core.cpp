
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
 *                         videooutput-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : declaration of the interface of a videooutput core.
 *                          A videooutput core manages VideoOutputManagers.
 *
 */

#include <iostream>
#include <sstream>

#include "config.h"

#include "videooutput-core.h"
#include "videooutput-manager.h"

#include <math.h>

using namespace Ekiga;

VideoOutputCore::VideoOutputCore ()
{
  PWaitAndSignal m(core_mutex);

  videooutput_stats.rx_width = videooutput_stats.rx_height = videooutput_stats.rx_fps = 0;
  videooutput_stats.tx_width = videooutput_stats.tx_height = videooutput_stats.tx_fps = 0;
  videooutput_stats.rx_frames = 0;
  videooutput_stats.tx_frames = 0;
  number_times_started = 0;
  videooutput_core_conf_bridge = NULL;
}

VideoOutputCore::~VideoOutputCore ()
{
  PWaitAndSignal m(core_mutex);

  if (videooutput_core_conf_bridge)
    delete videooutput_core_conf_bridge;

  for (std::set<VideoOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    delete (*iter);

  managers.clear();
}

void VideoOutputCore::setup_conf_bridge ()
{
  PWaitAndSignal m(core_mutex);

  videooutput_core_conf_bridge = new VideoOutputCoreConfBridge (*this);
}

void VideoOutputCore::add_manager (VideoOutputManager &manager)
{
  PWaitAndSignal m(core_mutex);

  managers.insert (&manager);
  manager_added.emit (manager);

  manager.device_opened.connect (sigc::bind (sigc::mem_fun (this, &VideoOutputCore::on_device_opened), &manager));
  manager.device_closed.connect (sigc::bind (sigc::mem_fun (this, &VideoOutputCore::on_device_closed), &manager));
  manager.device_error.connect (sigc::bind (sigc::mem_fun (this, &VideoOutputCore::on_device_error), &manager));
  manager.fullscreen_mode_changed.connect (sigc::bind (sigc::mem_fun (this, &VideoOutputCore::on_fullscreen_mode_changed), &manager));
  manager.size_changed.connect (sigc::bind (sigc::mem_fun (this, &VideoOutputCore::on_size_changed), &manager));
}


void VideoOutputCore::visit_managers (sigc::slot1<bool, VideoOutputManager &> visitor)
{
  bool go_on = true;

  for (std::set<VideoOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
    go_on = visitor (*(*iter));
}


void VideoOutputCore::start ()
{
   PWaitAndSignal m(core_mutex);

   number_times_started++;
   if (number_times_started > 1)
     return;

  g_get_current_time (&last_stats);

  for (std::set<VideoOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->open ();
  }
}

void VideoOutputCore::stop ()
{
  PWaitAndSignal m(core_mutex);

  number_times_started--;

  if (number_times_started < 0) {
    number_times_started = 0;
    return;
  }

  if (number_times_started != 0)
    return;
    
  for (std::set<VideoOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->close ();
  }
  videooutput_stats.rx_width = videooutput_stats.rx_height = videooutput_stats.rx_fps = 0;
  videooutput_stats.tx_width = videooutput_stats.tx_height = videooutput_stats.tx_fps = 0;
  videooutput_stats.rx_frames = 0;
  videooutput_stats.tx_frames = 0;
}

void VideoOutputCore::set_frame_data (const char *data,
                                  unsigned width,
                                  unsigned height,
                                  bool local,
                                  int devices_nbr)
{
  core_mutex.Wait ();

  if (local) {
    videooutput_stats.tx_frames++;
    videooutput_stats.tx_width = width;
    videooutput_stats.tx_height = height;
  }
  else {
    videooutput_stats.rx_frames++;
    videooutput_stats.rx_width = width;
    videooutput_stats.rx_height = height;
  }

  GTimeVal current_time;
  g_get_current_time (&current_time);

  long unsigned milliseconds = ((current_time.tv_sec - last_stats.tv_sec) * 1000) 
                             + ((current_time.tv_usec - last_stats.tv_usec) / 1000);

  if (milliseconds > 2000) {
    videooutput_stats.tx_fps = round ((videooutput_stats.tx_frames * 1000) / milliseconds);
    videooutput_stats.rx_fps = round ((videooutput_stats.rx_frames * 1000) / milliseconds);
    videooutput_stats.rx_frames = 0;
    videooutput_stats.tx_frames = 0;
    g_get_current_time (&last_stats);
  }

  core_mutex.Signal ();
  
  for (std::set<VideoOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->set_frame_data (data,width, height, local, devices_nbr);
  }
}

void VideoOutputCore::set_display_info (const DisplayInfo & _display_info)
{
  PWaitAndSignal m(core_mutex);

  for (std::set<VideoOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    (*iter)->set_display_info (_display_info);
  }
}


void VideoOutputCore::on_device_opened (VideoOutputAccel videooutput_accel, VideoOutputMode mode, unsigned zoom, bool both_streams, VideoOutputManager *manager)
{
  device_opened.emit (*manager, videooutput_accel, mode, zoom, both_streams);
}

void VideoOutputCore::on_device_closed ( VideoOutputManager *manager)
{
  device_closed.emit (*manager);
}

void VideoOutputCore::on_device_error (VideoOutputErrorCodes error_code, VideoOutputManager *manager)
{
  device_error.emit (*manager, error_code);
}

void VideoOutputCore::on_fullscreen_mode_changed ( VideoOutputFSToggle toggle, VideoOutputManager *manager)
{
  fullscreen_mode_changed.emit (*manager, toggle);
}

void VideoOutputCore::on_size_changed ( unsigned width, unsigned height, VideoOutputManager *manager)
{
  size_changed.emit (*manager, width, height);
}

