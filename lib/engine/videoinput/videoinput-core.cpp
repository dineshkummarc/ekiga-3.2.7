
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
 *                         videoinput-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a videoinput core.
 *                          A videoinput core manages VideoInputManagers.
 *
 */

#include <iostream>
#include <sstream>

#include "config.h"

#include "videoinput-core.h"
#include "videoinput-manager.h"

using namespace Ekiga;

VideoInputCore::VideoPreviewManager::VideoPreviewManager (VideoInputCore& _videoinput_core, VideoOutputCore& _videooutput_core)
: PThread (1000, NoAutoDeleteThread, HighestPriority, "VideoPreviewManager"),
    videoinput_core (_videoinput_core),
  videooutput_core (_videooutput_core)
{
  videooutput_core.reference ();
  width = 176;
  height = 144;;
  pause_thread = true;
  end_thread = false;
  frame = NULL;
  // Since windows does not like to restart a thread that 
  // was never started, we do so here
  this->Resume ();
  thread_paused.Wait();
}

VideoInputCore::VideoPreviewManager::~VideoPreviewManager ()
{
  if (!pause_thread)
    stop();
  end_thread = true;
  run_thread.Signal();
  thread_ended.Wait();
  videooutput_core.unreference ();
}

void VideoInputCore::VideoPreviewManager::start (unsigned _width, unsigned _height)
{
  PTRACE(4, "PreviewManager\tStarting Preview");
  width = _width;
  height = _height;
  end_thread = false;
  frame = (char*) malloc (unsigned (width * height * 3 / 2));

  videooutput_core.start();
  pause_thread = false;
  run_thread.Signal();
}

void VideoInputCore::VideoPreviewManager::stop ()
{
  PTRACE(4, "PreviewManager\tStopping Preview");
  pause_thread = true;
  thread_paused.Wait();

  if (frame) {
    free (frame);
    frame = NULL;
  }  
  videooutput_core.stop();
}

void VideoInputCore::VideoPreviewManager::Main ()
{
  PWaitAndSignal m(thread_ended);

    
  while (!end_thread) {

    thread_paused.Signal ();
    run_thread.Wait ();
    
    while (!pause_thread) {
      if (frame) {
        videoinput_core.get_frame_data(frame);
        videooutput_core.set_frame_data(frame, width, height, true, 1);
      }
      // We have to sleep some time outside the mutex lock
      // to give other threads time to get the mutex
      // It will be taken into account by PAdaptiveDelay
      Current()->Sleep (5);
    }

  }
}

VideoInputCore::VideoInputCore (VideoOutputCore& _videooutput_core)
:  preview_manager(*this, _videooutput_core)
{
  PWaitAndSignal m_var(core_mutex);
  PWaitAndSignal m_set(settings_mutex);

  preview_config.active = false;
  preview_config.width = 176;
  preview_config.height = 144;
  preview_config.fps = 30;

  stream_config.active = false;
  stream_config.width = 176;
  stream_config.height = 144;
  stream_config.fps = 30;

  current_settings.brightness = 0;
  current_settings.whiteness = 0;
  current_settings.colour = 0;
  current_settings.contrast = 0;

  desired_settings.brightness = 0;
  desired_settings.whiteness = 0;
  desired_settings.colour = 0;
  desired_settings.contrast = 0;

  current_manager = NULL;
  videoinput_core_conf_bridge = NULL;
}

VideoInputCore::~VideoInputCore ()
{
  PWaitAndSignal m(core_mutex);

  if (videoinput_core_conf_bridge)
    delete videoinput_core_conf_bridge;

  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    delete (*iter);

  managers.clear();
}

void VideoInputCore::setup_conf_bridge ()
{
  PWaitAndSignal m(core_mutex);

  videoinput_core_conf_bridge = new VideoInputCoreConfBridge (*this);
}

void VideoInputCore::add_manager (VideoInputManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.device_opened.connect (sigc::bind (sigc::mem_fun (this, &VideoInputCore::on_device_opened), &manager));
  manager.device_closed.connect (sigc::bind (sigc::mem_fun (this, &VideoInputCore::on_device_closed), &manager));
  manager.device_error.connect (sigc::bind (sigc::mem_fun (this, &VideoInputCore::on_device_error), &manager));
}


void VideoInputCore::visit_managers (sigc::slot1<bool, VideoInputManager &> visitor)
{
  PWaitAndSignal m(core_mutex);
  bool go_on = true;

  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}

void VideoInputCore::get_devices (std::vector <VideoInputDevice> & devices)
{
  PWaitAndSignal m(core_mutex);

  devices.clear();

  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->get_devices (devices);

#if PTRACING
  for (std::vector<VideoInputDevice>::iterator iter = devices.begin ();
       iter != devices.end ();
       iter++) {
    PTRACE(4, "VidInputCore\tDetected Device: " << *iter);
  }
#endif
}

void VideoInputCore::set_device(const VideoInputDevice & device, int channel, VideoInputFormat format)
{
  PWaitAndSignal m(core_mutex);
  internal_set_device(device, channel, format);
  desired_device  = device;
}

void VideoInputCore::add_device (const std::string & source, const std::string & device_name, unsigned capabilities, HalManager* /*manager*/)
{
  PTRACE(4, "VidInputCore\tAdding Device " << device_name);
  PWaitAndSignal m(core_mutex);

  VideoInputDevice device;
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
    if ((*iter)->has_device (source, device_name, capabilities, device)) {

      if ( desired_device == device )
        internal_set_device(device, current_channel, current_format);

      device_added.emit(device, desired_device == device);
    }
  }
}

void VideoInputCore::remove_device (const std::string & source, const std::string & device_name, unsigned capabilities, HalManager* /*manager*/)
{
  PTRACE(4, "VidInputCore\tRemoving Device " << device_name);
  PWaitAndSignal m(core_mutex);

  VideoInputDevice device;
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (source, device_name, capabilities, device)) {
       if ( (current_device == device) && (preview_config.active || stream_config.active) ) {

            VideoInputDevice new_device;
            new_device.type   = VIDEO_INPUT_FALLBACK_DEVICE_TYPE;
            new_device.source = VIDEO_INPUT_FALLBACK_DEVICE_SOURCE;
            new_device.name   = VIDEO_INPUT_FALLBACK_DEVICE_NAME;
            internal_set_device(new_device, current_channel, current_format);
       }

       device_removed.emit(device, current_device == device);
     }
  }
}

void VideoInputCore::set_preview_config (unsigned width, unsigned height, unsigned fps)
{
  PWaitAndSignal m(core_mutex);

  VideoDeviceConfig new_preview_config(width, height, fps);

  PTRACE(4, "VidInputCore\tSetting new preview config: " << new_preview_config);
  // There is only one state where we have to reopen the preview device:
  // we have preview enabled, no stream is active and some value has changed
  if ( ( preview_config.active && !stream_config.active) &&
       ( preview_config        !=  new_preview_config) )
  {
    preview_manager.stop();
    internal_close();

    internal_open(new_preview_config.width, new_preview_config.height, new_preview_config.fps);
    preview_manager.start(new_preview_config.width, new_preview_config.height);
  }

  preview_config = new_preview_config;
}


void VideoInputCore::start_preview ()
{
  PWaitAndSignal m(core_mutex);

  PTRACE(4, "VidInputCore\tStarting preview " << preview_config);
  if (!preview_config.active && !stream_config.active) {
    internal_open(preview_config.width, preview_config.height, preview_config.fps);
    preview_manager.start(preview_config.width, preview_config.height);
  }

  preview_config.active = true;
}

void VideoInputCore::stop_preview ()
{
  PWaitAndSignal m(core_mutex);

  PTRACE(4, "VidInputCore\tStopping Preview");
  if (preview_config.active && !stream_config.active) {
    preview_manager.stop();
    internal_close();
    internal_set_manager(desired_device, current_channel, current_format);
  }

  preview_config.active = false;
}

void VideoInputCore::set_stream_config (unsigned width, unsigned height, unsigned fps)
{
  PWaitAndSignal m(core_mutex);

  VideoDeviceConfig new_stream_config(width, height, fps);
  PTRACE(4, "VidInputCore\tSetting new stream config: " << new_stream_config);

  // We do not support switching of framerate or resolution within a stream
  // since many endpoints will probably have problems with that. Also, it would add
  // a lot of complexity due to the capabilities exchange. Thus these values will 
  // not be used until the next start_stream.

  if (!stream_config.active)
    stream_config = new_stream_config;
}

void VideoInputCore::start_stream ()
{
  PWaitAndSignal m(core_mutex);

  PTRACE(4, "VidInputCore\tStarting stream " << stream_config);
  if (preview_config.active && !stream_config.active) {
    preview_manager.stop();
    if ( preview_config != stream_config ) 
    {
      internal_close();
      internal_open(stream_config.width, stream_config.height, stream_config.fps);
    }
  }

  if (!preview_config.active && !stream_config.active) {
    internal_open(stream_config.width, stream_config.height, stream_config.fps);
  }

  stream_config.active = true;
}

void VideoInputCore::stop_stream ()
{
  PWaitAndSignal m(core_mutex);

  PTRACE(4, "VidInputCore\tStopping Stream");
  if (preview_config.active && stream_config.active) {
    if ( preview_config != stream_config ) 
    {
      internal_close();
      internal_set_manager(desired_device, current_channel, current_format);
      internal_open(preview_config.width, preview_config.height, preview_config.fps);
    }
    preview_manager.start(preview_config.width, preview_config.height);
  }

  if (!preview_config.active && stream_config.active) {
    internal_close();
    internal_set_manager(desired_device, current_channel, current_format);
  }

  stream_config.active = false;
}

void VideoInputCore::get_frame_data (char *data)
{
  PWaitAndSignal m(core_mutex);

  if (current_manager) {
    if (!current_manager->get_frame_data(data)) {

      internal_close();

      internal_set_fallback();

      if (preview_config.active && !stream_config.active)
        internal_open(preview_config.width, preview_config.height, preview_config.fps);

      if (stream_config.active)
        internal_open(stream_config.width, stream_config.height, stream_config.fps);

      if (current_manager)
        current_manager->get_frame_data(data); // the default device must always return true
    }
    internal_apply_settings();
  }
}

void VideoInputCore::set_colour (unsigned colour)
{
  PWaitAndSignal m(settings_mutex);
  desired_settings.colour = colour;
}

void VideoInputCore::set_brightness (unsigned brightness)
{
  PWaitAndSignal m(settings_mutex);
  desired_settings.brightness = brightness;
}

void VideoInputCore::set_whiteness  (unsigned whiteness)
{
  PWaitAndSignal m(settings_mutex);
  desired_settings.whiteness = whiteness;
}

void VideoInputCore::set_contrast   (unsigned contrast)
{
  PWaitAndSignal m(settings_mutex);
  desired_settings.contrast = contrast;
}

void VideoInputCore::on_device_opened (VideoInputDevice device,
                                     VideoInputSettings settings, 
                                     VideoInputManager *manager)
{
  device_opened.emit (*manager, device, settings);
}

void VideoInputCore::on_device_closed (VideoInputDevice device, VideoInputManager *manager)
{
  device_closed.emit (*manager, device);
}

void VideoInputCore::on_device_error (VideoInputDevice device, VideoInputErrorCodes error_code, VideoInputManager *manager)
{
  device_error.emit (*manager, device, error_code);
}

void VideoInputCore::internal_set_device(const VideoInputDevice & device, int channel, VideoInputFormat format)
{
  PTRACE(4, "VidInputCore\tSetting device: " << device);

  if (preview_config.active && !stream_config.active)
    preview_manager.stop();

  if (preview_config.active || stream_config.active)
    internal_close();

  internal_set_manager (device, channel, format);

  if (preview_config.active && !stream_config.active) {
    internal_open(preview_config.width, preview_config.height, preview_config.fps);
    preview_manager.start(preview_config.width,preview_config.height);
  }

  if (stream_config.active)
    internal_open(stream_config.width, stream_config.height, stream_config.fps);
}

void VideoInputCore::internal_set_manager (const VideoInputDevice & device, int channel, VideoInputFormat format)
{
  current_manager = NULL;
  for (std::set<VideoInputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->set_device (device, channel, format)) {
       current_manager = (*iter);
     }
  }

  // If the desired manager could not be found,
  // we se the default device. The default device
  // MUST ALWAYS be loaded and openable
  if (current_manager) {
    current_device  = device;
  }
  else {
    PTRACE(1, "VidInputCore\tTried to set unexisting device " << device);
    internal_set_fallback();
  }

  current_channel = channel;
  current_format  = format;
}


void VideoInputCore::internal_set_fallback ()
{
  current_device.type   = VIDEO_INPUT_FALLBACK_DEVICE_TYPE;
  current_device.source = VIDEO_INPUT_FALLBACK_DEVICE_SOURCE;
  current_device.name   = VIDEO_INPUT_FALLBACK_DEVICE_NAME;
  PTRACE(3, "VidInputCore\tFalling back to " << current_device);

  internal_set_manager(current_device, current_channel, current_format);
}

void VideoInputCore::internal_open (unsigned width, unsigned height, unsigned fps)
{
  PTRACE(4, "VidInputCore\tOpening device with " << width << "x" << height << "/" << fps );

  if (current_manager && !current_manager->open(width, height, fps)) {

    internal_set_fallback();
    if (current_manager)
      current_manager->open(width, height, fps);
  }
}

void VideoInputCore::internal_close()
{
  PTRACE(4, "VidInputCore\tClosing current device");
  if (current_manager)
    current_manager->close();
}

void VideoInputCore::internal_apply_settings()
{
  PWaitAndSignal m_set(settings_mutex);

  if (desired_settings.colour != current_settings.colour) {
    current_manager->set_colour (desired_settings.colour);
    current_settings.colour = desired_settings.colour;
  }

  if (desired_settings.brightness != current_settings.brightness) {
    current_manager->set_brightness (desired_settings.brightness);
    current_settings.brightness = desired_settings.brightness;
  }

  if (desired_settings.whiteness != current_settings.whiteness) {
    current_manager->set_whiteness (desired_settings.whiteness);
    current_settings.whiteness = desired_settings.whiteness;
  }

  if (desired_settings.contrast != current_settings.contrast) {
    current_manager->set_contrast (desired_settings.contrast);
    current_settings.contrast = desired_settings.contrast;
  }
}
