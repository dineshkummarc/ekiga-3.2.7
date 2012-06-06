
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
 *                         videooutput-info.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider 
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : Declaration of structs and classes used for communication
 *                          with the VideoOutputManagers
 *
 */

#ifndef __VIDEOOUTPUT_INFO_H__
#define __VIDEOOUTPUT_INFO_H__

#ifdef WIN32
#include <windows.h>
#else
#include <X11/Xlib.h>
#ifdef None
#undef None
#endif
#ifdef BadRequest
#undef BadRequest
#endif
#endif

namespace Ekiga {

/**
 * @addtogroup display
 * @{
 */

  /* Video modes */
  typedef enum {
  
    VO_MODE_LOCAL, 
    VO_MODE_REMOTE, 
    VO_MODE_PIP,
    VO_MODE_PIP_WINDOW,
    VO_MODE_FULLSCREEN,
    VO_MODE_UNSET
  } VideoOutputMode;
  
  /* Toggle operations for Fullscreen */
  typedef enum {
  
    VO_FS_ON,
    VO_FS_OFF,
    VO_FS_TOGGLE
  } VideoOutputFSToggle;
  
  /* Video Acceleration Status */
  typedef enum {

    VO_ACCEL_NONE,
    VO_ACCEL_REMOTE_ONLY,
    VO_ACCEL_ALL,
    VO_ACCEL_NO_VIDEO
  } VideoOutputAccel;

  enum VideoOutputErrorCodes {
    VO_ERROR_NONE = 0,
    VO_ERROR,
  };


  typedef struct {
    unsigned rx_fps;
    unsigned rx_width;
    unsigned rx_height;
    unsigned rx_frames;
    unsigned tx_fps;
    unsigned tx_width;
    unsigned tx_height;
    unsigned tx_frames;
  } VideoOutputStats;

  class DisplayInfo
  {
  public:
    DisplayInfo() {
      widget_info_set = false;
      x = 0;
      y = 0;
  #ifdef WIN32
      hwnd = 0;
  #else
      gc = 0;
      window = 0;
      xdisplay = NULL;
  #endif
  
      config_info_set = false;
      on_top = false;
      disable_hw_accel = false;
      allow_pip_sw_scaling = true;
      sw_scaling_algorithm = 0;
  
      mode = VO_MODE_UNSET;
      zoom = 0;
    };
    
    void operator= ( const DisplayInfo& rhs) {
  
    if (rhs.widget_info_set) {
        widget_info_set = rhs.widget_info_set;
        x = rhs.x;
        y = rhs.y;
  #ifdef WIN32
        hwnd = rhs.hwnd;
  #else
        gc = rhs.gc;
        window = rhs.window;
        xdisplay = rhs.xdisplay;
  #endif
      }
  
      if (rhs.config_info_set) {
        config_info_set = rhs.config_info_set;
        on_top = rhs.on_top;
        disable_hw_accel = rhs.disable_hw_accel;
        allow_pip_sw_scaling = rhs.allow_pip_sw_scaling;
        sw_scaling_algorithm =  rhs.sw_scaling_algorithm;
      }
      if (rhs.mode != VO_MODE_UNSET) mode = rhs.mode;
      if (rhs.zoom != 0) zoom = rhs.zoom;
    };
  
    bool widget_info_set;
    int x;
    int y;
              
  #ifdef WIN32
    HWND hwnd;
  #else
    GC gc;
    Window window;
    Display* xdisplay;
  #endif
  
    bool config_info_set;
    bool on_top;
    bool disable_hw_accel;
    bool allow_pip_sw_scaling;
    unsigned int sw_scaling_algorithm;
  
    VideoOutputMode mode;
    unsigned int zoom;
  };

/**
 * @}
 */

};

#endif
