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
 *                         videooutput-manager-dx.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2008 by Matthias Schneider
 *                          (C) 2000-2008 by Damien Sandras
 *   description          : Class to allow video output to a DirectX
 *                          accelerated window
 */

#include "videooutput-manager-dx.h"

GMVideoOutputManager_dx::GMVideoOutputManager_dx (Ekiga::ServiceCore & _core)
: GMVideoOutputManager(_core)
{
  dxWindow = NULL;

  end_thread = false;
  init_thread = false;
  uninit_thread = false;

  this->Resume ();
  thread_created.Wait ();
}

GMVideoOutputManager_dx::~GMVideoOutputManager_dx ()
{
  end_thread = true;
  run_thread.Signal();
  thread_ended.Wait();
}

bool
GMVideoOutputManager_dx::frame_display_change_needed ()
{
  if (!dxWindow)
    return true;

  return GMVideoOutputManager::frame_display_change_needed ();
}

void
GMVideoOutputManager_dx::setup_frame_display ()
{
  Ekiga::DisplayInfo local_display_info;

  if (video_disabled)
    return;
    
  get_display_info(local_display_info);

  switch (current_frame.mode) {
  case Ekiga::VO_MODE_LOCAL:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::size_changed_in_main), (unsigned) (current_frame.local_width * current_frame.zoom / 100), (unsigned) (current_frame.local_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_REMOTE:
  case Ekiga::VO_MODE_PIP:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::size_changed_in_main), (unsigned) (current_frame.remote_width * current_frame.zoom / 100), (unsigned) (current_frame.remote_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_FULLSCREEN:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::size_changed_in_main), 176, 144));
    break;
  case Ekiga::VO_MODE_PIP_WINDOW:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::size_changed_in_main), 176, 144));
    break;
  case Ekiga::VO_MODE_UNSET:
  default:
    PTRACE (1, "GMVideoOutputManager_dx\tDisplay variable not set");
    return;
    break; 
  }

  if (   (!local_display_info.widget_info_set) || (!local_display_info.config_info_set) 
      || (local_display_info.mode == Ekiga::VO_MODE_UNSET) || (local_display_info.zoom == 0) || (current_frame.zoom == 0)) {
    PTRACE(4, "GMVideoOutputManager_dx\tWidget not yet realized or gconf info not yet set, not opening display");
    return;
  }

  close_frame_display ();

  current_frame.accel = Ekiga::VO_ACCEL_NONE;

  switch (current_frame.mode) {
  case Ekiga::VO_MODE_LOCAL:
    PTRACE(4, "GMVideoOutputManager_dx\tOpening :VO_MODE_LOCAL display with image of " << current_frame.local_width << "x" << current_frame.local_height);
    dxWindow = new DXWindow();
    video_disabled = !dxWindow->Init (local_display_info.hwnd,
                                      local_display_info.x,
                                      local_display_info.y,
                                      (int) (current_frame.local_width * current_frame.zoom / 100), 
                                      (int) (current_frame.local_height * current_frame.zoom / 100),
                                      current_frame.local_width, 
                                      current_frame.local_height);

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_LOCAL;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case Ekiga::VO_MODE_REMOTE:
    PTRACE(4, "GMVideoOutputManager_dx\tOpening VO_MODE_REMOTE display with image of " << current_frame.remote_width << "x" << current_frame.remote_height);
    dxWindow = new DXWindow();
    video_disabled = !dxWindow->Init (local_display_info.hwnd,
                                      local_display_info.x,
                                      local_display_info.y,
                                      (int) (current_frame.remote_width * current_frame.zoom / 100), 
                                      (int) (current_frame.remote_height * current_frame.zoom / 100),
                                      current_frame.remote_width, 
                                      current_frame.remote_height); 

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_REMOTE;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case Ekiga::VO_MODE_FULLSCREEN:
  case Ekiga::VO_MODE_PIP:
  case Ekiga::VO_MODE_PIP_WINDOW:
    PTRACE(4, "GMVideoOutputManager_dx\tOpening display " << current_frame.mode << " with images of " 
            << current_frame.local_width << "x" << current_frame.local_height << "(local) and " 
	    << current_frame.remote_width << "x" << current_frame.remote_height << "(remote)");
    dxWindow = new DXWindow();
    video_disabled = !dxWindow->Init ((current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.hwnd : NULL,
                                      (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.x : 0,
                                      (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.y : 0,
                                      (int) (current_frame.remote_width * current_frame.zoom  / 100), 
                                      (int) (current_frame.remote_height * current_frame.zoom  / 100),
                                      current_frame.remote_width, 
                                      current_frame.remote_height,
                                      current_frame.local_width, 
                                      current_frame.local_height); 

    if (dxWindow && current_frame.mode == Ekiga::VO_MODE_FULLSCREEN) 
      dxWindow->ToggleFullscreen ();

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = current_frame.mode;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;

  case Ekiga::VO_MODE_UNSET:
  default:
    return;
    break;
  }
  PTRACE (4, "GMVideoOutputManager_dx\tSetup display " << current_frame.mode << " with zoom value of " << current_frame.zoom );

  if (local_display_info.on_top && dxWindow)
      dxWindow->ToggleOntop ();

//   if (!status)
//     close_frame_display ();

  last_frame.both_streams_active = current_frame.both_streams_active;

  if (video_disabled) {
    delete dxWindow;
    dxWindow = NULL;
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::device_error_in_main), Ekiga::VO_ERROR));
  }
  else {
    current_frame.accel = Ekiga::VO_ACCEL_ALL; 
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::device_opened_in_main), current_frame.accel, current_frame.mode, current_frame.zoom, current_frame.both_streams_active));
  }
}

void
GMVideoOutputManager_dx::close_frame_display ()
{
  Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &GMVideoOutputManager_dx::device_closed_in_main));

  if (dxWindow) {

    delete dxWindow;
    dxWindow = NULL;
  }
}

void
GMVideoOutputManager_dx::display_frame (const char *frame,
                             unsigned width,
                             unsigned height)
{
  if  (dxWindow) {
    dxWindow->ProcessEvents();
    dxWindow->PutFrame ((uint8_t *) frame, width, height, false);
  }
}

void
GMVideoOutputManager_dx::display_pip_frames (const char *local_frame,
                                 unsigned lf_width,
                                 unsigned lf_height,
                                 const char *remote_frame,
                                 unsigned rf_width,
                                 unsigned rf_height)
{
  if (dxWindow)
    dxWindow->ProcessEvents(); 

  if (current_frame.mode == Ekiga::VO_MODE_FULLSCREEN && dxWindow && !dxWindow->IsFullScreen ())
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_dx::fullscreen_mode_changed_in_main), Ekiga::VO_FS_OFF));

  if (dxWindow) {
    if (update_required.remote || (!update_required.remote && !update_required.local)) {
      dxWindow->PutFrame ((uint8_t *) remote_frame, rf_width, rf_height, false);
      dxWindow->PutFrame ((uint8_t *) local_frame, lf_width, lf_height, true);      
    }
      
    if (update_required.local  || (!update_required.remote && !update_required.local))
      dxWindow->PutFrame ((uint8_t *) local_frame, lf_width, lf_height, true);      
  }
}

void
GMVideoOutputManager_dx::sync (UpdateRequired sync_required)
{
  if (dxWindow)
    dxWindow->Sync(); 
}

void
GMVideoOutputManager_dx::size_changed_in_main (unsigned width,
		      unsigned height)
{
  size_changed.emit (width, height);
}

void
GMVideoOutputManager_dx::device_opened_in_main (Ekiga::VideoOutputAccel accel,
		       Ekiga::VideoOutputMode mode,
		       unsigned zoom,
		       bool both)
{
  device_opened.emit (accel, mode, zoom, both);
}

void
GMVideoOutputManager_dx::device_closed_in_main ()
{
  device_closed.emit ();
}

void
GMVideoOutputManager_dx::device_error_in_main (Ekiga::VideoOutputErrorCodes code)
{
  device_error.emit (code);
}

void
GMVideoOutputManager_dx::fullscreen_mode_changed_in_main (Ekiga::VideoOutputFSToggle val)
{
  fullscreen_mode_changed.emit (val);
}
