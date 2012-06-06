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
 *                         videooutput-manager-x.cpp -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2008 by Matthias Schneider
 *                          (C) 2000-2008 by Damien Sandras
 *   description          : Class to allow video output to a X/XVideo
 *                          accelerated window
 */

#include "videooutput-manager-x.h"

#include "xwindow.h"

#ifdef HAVE_XV
#include "xvwindow.h"
#endif

GMVideoOutputManager_x::GMVideoOutputManager_x (Ekiga::ServiceCore & _core)
: GMVideoOutputManager(_core)
{
  /* Internal stuff */
  lxWindow = NULL;
  rxWindow = NULL;

  rDisplay = XOpenDisplay (NULL);
  lDisplay = XOpenDisplay (NULL);

  pip_window_available = true;

  end_thread = false;
  init_thread = false;
  uninit_thread = false;

  this->Resume ();
  thread_created.Wait ();
}

GMVideoOutputManager_x::~GMVideoOutputManager_x ()
{
  end_thread = true;
  run_thread.Signal();
  thread_ended.Wait();

  if (lDisplay) 
    XCloseDisplay (lDisplay);
  if (rDisplay)
    XCloseDisplay (rDisplay);
}

bool
GMVideoOutputManager_x::frame_display_change_needed ()
{
    switch (current_frame.mode) 
    {
      case Ekiga::VO_MODE_LOCAL:
          if (!lxWindow) 
            return true;
          break;
      case Ekiga::VO_MODE_REMOTE:
          if (!rxWindow) 
            return true;
          break;
      case Ekiga::VO_MODE_FULLSCREEN:
      case Ekiga::VO_MODE_PIP:
      case Ekiga::VO_MODE_PIP_WINDOW:
          if (!rxWindow || (pip_window_available && (!lxWindow)) )
              return true;
          break;
     case Ekiga::VO_MODE_UNSET:
     default:
          break;
    }
  return GMVideoOutputManager::frame_display_change_needed ();

}

void
GMVideoOutputManager_x::setup_frame_display ()
{
  Ekiga::DisplayInfo local_display_info;

  if (video_disabled)
    return;

  get_display_info(local_display_info);

  switch (current_frame.mode) {
  case Ekiga::VO_MODE_LOCAL:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::size_changed_in_main), (unsigned) (current_frame.local_width * current_frame.zoom / 100), (unsigned) (current_frame.local_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_REMOTE:
  case Ekiga::VO_MODE_PIP:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::size_changed_in_main), (unsigned) (current_frame.remote_width * current_frame.zoom / 100), (unsigned) (current_frame.remote_height * current_frame.zoom / 100)));
    break;
  case Ekiga::VO_MODE_FULLSCREEN:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::size_changed_in_main), 176, 144));
    break;
  case Ekiga::VO_MODE_PIP_WINDOW:
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::size_changed_in_main), 176, 144));
    break;
  case Ekiga::VO_MODE_UNSET:
  default:
    PTRACE (1, "GMVideoOutputManager_X\tDisplay variable not set");
    return;
    break;
  }

  if ((!local_display_info.widget_info_set) || (!local_display_info.config_info_set) ||
      (local_display_info.mode == Ekiga::VO_MODE_UNSET) || (local_display_info.zoom == 0) || (current_frame.zoom == 0)) {
    PTRACE(4, "GMVideoOutputManager_X\tWidget not yet realized or gconf info not yet set, not opening display");
    return;
  }

  close_frame_display ();

  pip_window_available = false;

  current_frame.accel = Ekiga::VO_ACCEL_NONE;

  switch (current_frame.mode) {
// VO_MODE_LOCAL ------------------------------------------------------------------
  case Ekiga::VO_MODE_LOCAL:
    PTRACE(4, "GMVideoOutputManager_X\tOpening VO_MODE_LOCAL display with image of " << current_frame.local_width << "x" << current_frame.local_height);
#ifdef HAVE_XV
    if (!local_display_info.disable_hw_accel) {
      lxWindow = new XVWindow ();
      if (lxWindow->Init (lDisplay, 
                            local_display_info.window, 
                            local_display_info.gc,
                            local_display_info.x,
                            local_display_info.y,
                            (int) (current_frame.local_width * current_frame.zoom / 100), 
                            (int) (current_frame.local_height * current_frame.zoom / 100),
                            current_frame.local_width, 
                            current_frame.local_height)) {
	current_frame.accel = Ekiga::VO_ACCEL_ALL;
        PTRACE(4, "GMVideoOutputManager_X\tVO_MODE_LOCAL: Successfully opened XV Window");
      }
      else {
	delete lxWindow;
	lxWindow = NULL;
	current_frame.accel = Ekiga::VO_ACCEL_NONE;
        PTRACE(4, "GMVideoOutputManager_X\tVO_MODE_LOCAL: Could not open XV Window");
      }
    }
#endif			   
    if (current_frame.accel == Ekiga::VO_ACCEL_NONE) {
      PTRACE(3, "GMVideoOutputManager_X\tFalling back to SW" << ((local_display_info.disable_hw_accel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      lxWindow = new XWindow ();
      if (lxWindow->Init (lDisplay, 
                            local_display_info.window, 
                            local_display_info.gc,
                            local_display_info.x,
                            local_display_info.y,
                           (int) (current_frame.local_width * current_frame.zoom / 100), 
                           (int) (current_frame.local_height * current_frame.zoom / 100),
                           current_frame.local_width, 
                           current_frame.local_height)) {
       lxWindow->SetSwScalingAlgo(local_display_info.sw_scaling_algorithm);
       PTRACE(4, "GMVideoOutputManager_X\tVO_MODE_LOCAL: Successfully opened X Window");
      }
      else {
        delete lxWindow;
        lxWindow = NULL;
        video_disabled = true;
        current_frame.accel = Ekiga::VO_ACCEL_NO_VIDEO;
        PTRACE(1, "GMVideoOutputManager_X\tVO_MODE_LOCAL: Could not open X Window - no video");
      }
    }
    
    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_LOCAL;
    last_frame.local_width = current_frame.local_width;
    last_frame.local_height = current_frame.local_height;
    last_frame.zoom = current_frame.zoom;
    break;

// VO_MODE_REMOTE ----------------------------------------------------------------
  case Ekiga::VO_MODE_REMOTE:
    PTRACE(4, "GMVideoOutputManager_X\tOpening VO_MODE_REMOTE display with image of " << current_frame.remote_width << "x" << current_frame.remote_height);
#ifdef HAVE_XV
    if (!local_display_info.disable_hw_accel) {
      rxWindow = new XVWindow ();
      if (rxWindow->Init (rDisplay, 
                          local_display_info.window, 
                          local_display_info.gc,
                          local_display_info.x,
                          local_display_info.y,
                          (int) (current_frame.remote_width * current_frame.zoom / 100), 
                          (int) (current_frame.remote_height * current_frame.zoom / 100),
                          current_frame.remote_width, 
                          current_frame.remote_height)) {
       current_frame.accel = Ekiga::VO_ACCEL_ALL;
       PTRACE(4, "GMVideoOutputManager_X\tVO_MODE_REMOTE: Successfully opened XV Window");
     }
     else {
       delete rxWindow;
       rxWindow = NULL;
       current_frame.accel = Ekiga::VO_ACCEL_NONE;
       PTRACE(1, "GMVideoOutputManager_X\tVO_MODE_LOCAL: Could not open XV Window");

     }
    }
#endif			   
    if (current_frame.accel == Ekiga::VO_ACCEL_NONE) {
      PTRACE(3, "GMVideoOutputManager_X\tFalling back to SW" << ((local_display_info.disable_hw_accel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      rxWindow = new XWindow ();
      if ( rxWindow->Init (rDisplay, 
                             local_display_info.window, 
                             local_display_info.gc,
                             local_display_info.x,
                             local_display_info.y,
                             (int) (current_frame.remote_width * current_frame.zoom / 100), 
                             (int) (current_frame.remote_height * current_frame.zoom / 100),
                             current_frame.remote_width, 
                             current_frame.remote_height)) {
        rxWindow->SetSwScalingAlgo(local_display_info.sw_scaling_algorithm);
        PTRACE(4, "GMVideoOutputManager_X\tVO_MODE_REMOTE: Successfully opened X Window");
      }
      else {
        delete rxWindow;
        rxWindow = NULL;
        video_disabled = true;
        current_frame.accel = Ekiga::VO_ACCEL_NO_VIDEO;
        PTRACE(1, "GMVideoOutputManager_X\tVO_MODE_REMOTE: Could not open X Window - no video");
      }
    }

    last_frame.embedded_x = local_display_info.x;
    last_frame.embedded_y = local_display_info.y;

    last_frame.mode = Ekiga::VO_MODE_REMOTE;
    last_frame.remote_width = current_frame.remote_width;
    last_frame.remote_height = current_frame.remote_height;
    last_frame.zoom = current_frame.zoom;
    break;

// PIP_VIDEO ------------------------------------------------------------------
  case Ekiga::VO_MODE_FULLSCREEN:
  case Ekiga::VO_MODE_PIP:
  case Ekiga::VO_MODE_PIP_WINDOW:
    PTRACE(4, "GMVideoOutputManager_X\tOpening display " << current_frame.mode << " with images of " 
            << current_frame.local_width << "x" << current_frame.local_height << "(local) and " 
	    << current_frame.remote_width << "x" << current_frame.remote_height << "(remote)");
#ifdef HAVE_XV
    if (!local_display_info.disable_hw_accel) {
      rxWindow = new XVWindow ();
      if (rxWindow->Init ( rDisplay, 
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.window : DefaultRootWindow (rDisplay), 
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.gc : NULL,
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.x : 0,
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.y : 0,
                             (int) (current_frame.remote_width * current_frame.zoom / 100), 
                             (int) (current_frame.remote_height * current_frame.zoom / 100),
                             current_frame.remote_width, 
                             current_frame.remote_height)) {
        current_frame.accel = Ekiga::VO_ACCEL_REMOTE_ONLY;
        PTRACE(4, "GMVideoOutputManager_X\tPIP: Successfully opened remote XV Window");
      }
      else 
      {
        delete rxWindow;
	rxWindow = NULL;
	current_frame.accel = Ekiga::VO_ACCEL_NONE;
        PTRACE(1, "GMVideoOutputManager_X\tPIP: Could not open remote XV Window");
      }
    }
#endif			   
    if (current_frame.accel == Ekiga::VO_ACCEL_NONE) {
      PTRACE(3, "GMVideoOutputManager_X\tFalling back to SW" << ((local_display_info.disable_hw_accel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      rxWindow = new XWindow ();
      if (rxWindow->Init ( rDisplay, 
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.window : DefaultRootWindow (rDisplay), 
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.gc : NULL,
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.x : 0,
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.y : 0,
                             (int) (current_frame.remote_width * current_frame.zoom / 100), 
                             (int) (current_frame.remote_height * current_frame.zoom / 100),
                             current_frame.remote_width, 
                             current_frame.remote_height)) {
        rxWindow->SetSwScalingAlgo(local_display_info.sw_scaling_algorithm);
        PTRACE(4, "GMVideoOutputManager_X\tPIP: Successfully opened remote X Window");
      }
      else {
        delete rxWindow;
        rxWindow = NULL;
        video_disabled = true;
        current_frame.accel = Ekiga::VO_ACCEL_NO_VIDEO;
        PTRACE(1, "GMVideoOutputManager_X\tPIP: Could not open remote X Window - no video");
      }
    }


#ifdef HAVE_XV
    if (current_frame.accel == Ekiga::VO_ACCEL_REMOTE_ONLY) {
      lxWindow = new XVWindow();
      if (lxWindow->Init (   rxWindow->GetDisplay (), 
                             rxWindow->GetWindowHandle (),
                             rxWindow->GetGC (),
                             (int) (current_frame.remote_width * current_frame.zoom  / 100 * 2 / 3), 
                             (int) (current_frame.remote_height * current_frame.zoom  / 100 * 2 / 3), 
                             (int) (current_frame.remote_width * current_frame.zoom  / 100 / 3), 
                             (int) (current_frame.remote_height * current_frame.zoom  / 100 / 3),
                             current_frame.local_width, 
                             current_frame.local_height)) {
        current_frame.accel = Ekiga::VO_ACCEL_ALL;
        pip_window_available = true;
        PTRACE(4, "GMVideoOutputManager_X\tPIP: Successfully opened local XV Window");
      }
      else {
        delete lxWindow;
	lxWindow = NULL;
        pip_window_available = false;
        PTRACE(1, "GMVideoOutputManager_X\tPIP: Could not open local XV Window");
      }
    }
#endif
    if ((current_frame.accel != Ekiga::VO_ACCEL_ALL) && (local_display_info.allow_pip_sw_scaling)) {
      PTRACE(3, "GMVideoOutputManager_X\tFalling back to SW" << ((local_display_info.disable_hw_accel) 
                                      ? " since HW acceleration was deactivated by configuration" 
                                      : " since HW acceleration failed to initalize"));
      lxWindow = new XWindow ();
      if (lxWindow->Init ( lDisplay, 
                             rxWindow->GetWindowHandle (),
                             (current_frame.mode == Ekiga::VO_MODE_PIP) ? local_display_info.gc : NULL,
                             (int) (current_frame.remote_width * current_frame.zoom  / 100 * 2 / 3), 
                             (int) (current_frame.remote_height * current_frame.zoom  / 100 * 2 / 3), 
                             (int) (current_frame.remote_width * current_frame.zoom  / 100 / 3), 
                             (int) (current_frame.remote_height * current_frame.zoom  / 100 / 3),
                             current_frame.local_width, 
                             current_frame.local_height)) {
       lxWindow->SetSwScalingAlgo(local_display_info.sw_scaling_algorithm);
       pip_window_available = true;
       PTRACE(4, "GMVideoOutputManager_X\tPIP: Successfully opened local X Window");
     }
     else {
       delete lxWindow;
       lxWindow = NULL;
       pip_window_available = false;
       PTRACE(4, "GMVideoOutputManager_X\tPIP: Could not open local X Window - picture-in-picture disabled");
      }
    }

    if ((current_frame.accel != Ekiga::VO_ACCEL_ALL) && (!local_display_info.allow_pip_sw_scaling)) {
      PTRACE(3, "GMVideoOutputManager_X\tNot opening PIP window since HW acceleration is not available and SW fallback is disabled by configuration");
      current_frame.accel = Ekiga::VO_ACCEL_ALL;
    }

    if (rxWindow && lxWindow) {

      rxWindow->RegisterSlave (lxWindow);
      lxWindow->RegisterMaster (rxWindow);
    }	  

    if (rxWindow && current_frame.mode == Ekiga::VO_MODE_FULLSCREEN) 
      rxWindow->ToggleFullscreen ();
    
    if ((current_frame.mode != Ekiga::VO_MODE_PIP_WINDOW) && (current_frame.mode != Ekiga::VO_MODE_FULLSCREEN)) {
      last_frame.embedded_x = local_display_info.x;
      last_frame.embedded_y = local_display_info.y;
    }

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

  if (local_display_info.on_top) {

    if (lxWindow)
      lxWindow->ToggleOntop ();
    if (rxWindow)
      rxWindow->ToggleOntop ();
  }

  last_frame.both_streams_active = current_frame.both_streams_active;

  if (video_disabled) {
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::device_error_in_main), Ekiga::VO_ERROR));
  }
  else {
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::device_opened_in_main), current_frame.accel, current_frame.mode, current_frame.zoom, current_frame.both_streams_active));
  }
}

void
GMVideoOutputManager_x::close_frame_display ()
{
  Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &GMVideoOutputManager_x::device_closed_in_main));

  if (rxWindow) 
    rxWindow->RegisterSlave (NULL);
  if (lxWindow) 
    lxWindow->RegisterMaster (NULL);

  if (lxWindow) {
    delete lxWindow;
    lxWindow = NULL;
  }

  if (rxWindow) {
    delete rxWindow;
    rxWindow = NULL;
  }
}

void
GMVideoOutputManager_x::display_frame (const char *frame,
                             unsigned width,
                             unsigned height)
{
  if (rxWindow)
    rxWindow->ProcessEvents();

  if (lxWindow)
    lxWindow->ProcessEvents();

  if  ((current_frame.mode == Ekiga::VO_MODE_LOCAL) && (lxWindow))
    lxWindow->PutFrame ((uint8_t *) frame, width, height);

  if  ((current_frame.mode == Ekiga::VO_MODE_REMOTE) && (rxWindow))
    rxWindow->PutFrame ((uint8_t *) frame, width, height);
}

void
GMVideoOutputManager_x::display_pip_frames (const char *local_frame,
                                 unsigned lf_width,
                                 unsigned lf_height,
                                 const char *remote_frame,
                                 unsigned rf_width,
                                 unsigned rf_height)
{
  if (rxWindow)
    rxWindow->ProcessEvents();

  if (lxWindow)
    lxWindow->ProcessEvents();

  if (current_frame.mode == Ekiga::VO_MODE_FULLSCREEN && rxWindow && !rxWindow->IsFullScreen ())
    Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoOutputManager_x::fullscreen_mode_changed_in_main), Ekiga::VO_FS_OFF));

  if (rxWindow && (update_required.remote || (!update_required.remote && !update_required.local)))
    rxWindow->PutFrame ((uint8_t *) remote_frame, rf_width, rf_height);

  if (lxWindow && (update_required.local  || (!update_required.remote && !update_required.local)))
    lxWindow->PutFrame ((uint8_t *) local_frame, lf_width, lf_height);
}

void
GMVideoOutputManager_x::sync (UpdateRequired sync_required)
{
  if (rxWindow && (sync_required.remote || (!sync_required.remote && !sync_required.local))) {
    rxWindow->Sync();
  }

  if (lxWindow && (sync_required.local  || (!sync_required.remote && !sync_required.local))) {
    lxWindow->Sync();
  }
}

void
GMVideoOutputManager_x::size_changed_in_main (unsigned width,
					      unsigned height)
{
  size_changed.emit (width, height);
}

void
GMVideoOutputManager_x::device_opened_in_main (Ekiga::VideoOutputAccel accel,
					       Ekiga::VideoOutputMode mode,
					       unsigned zoom,
					       bool both)
{
  device_opened.emit (accel, mode, zoom, both);
}

void
GMVideoOutputManager_x::device_closed_in_main ()
{
  device_closed.emit ();
}

void
GMVideoOutputManager_x::device_error_in_main (Ekiga::VideoOutputErrorCodes code)
{
  device_error.emit (code);
}

void
GMVideoOutputManager_x::fullscreen_mode_changed_in_main (Ekiga::VideoOutputFSToggle val)
{
  fullscreen_mode_changed.emit (val);
}
