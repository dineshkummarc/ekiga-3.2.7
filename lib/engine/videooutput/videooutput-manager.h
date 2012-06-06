
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
 *                         videooutput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Matthias Schneider
 *   copyright            : (c) 2007 by Matthias Schneider
 *   description          : Declaration of the interface of a videooutput manager
 *                          implementation backend.
 *
 */


#ifndef __VIDEOOUTPUT_MANAGER_H__
#define __VIDEOOUTPUT_MANAGER_H__

#include <sigc++/sigc++.h>

#include "videooutput-info.h"

namespace Ekiga
{

/**
 * @addtogroup videooutput
 * @{
 */

  /** Generic implementation for the Ekiga::VideoOutputManager class.
   *
   * Each VideoOutputManager represents a sink for video frames.
   * A VideoOutputManager can display the video signal, record single frames or record video signal.
   */
  class VideoOutputManager
    {

  public:

      /** The constructor
       */
      VideoOutputManager () {}

      /** The destructor
       */
      virtual ~VideoOutputManager () {}


      /*** API for video output ***/

      /** Open the device.
       * The device must be opened before calling set_frame_data().
       */
      virtual void open () { };

      /** Close the device.
       */
      virtual void close () { };

      /** Set one video frame buffer.
       * Requires the device to be opened.
       * @param data a pointer to the buffer with the data to be written. It will not be freed.
       * @param width the width in pixels of the frame to be written.
       * @param height the height in pixels of the frame to be written.
       * @param local true if the frame is a frame of the local video source, false if it is from the remote end.
       * @param devices_nbr 1 if only local or remote device has been opened, 2 if both have been opened.
       */
      virtual void set_frame_data (const char *data,
                                   unsigned width,
                                   unsigned height,
                                   bool local,
                                   int devices_nbr) = 0;

      virtual void set_display_info (const DisplayInfo &) { };


      /*** API to act on VideoOutputDevice events ***/

      /** This signal is emitted when a video output device is opened.
       * @param videooutput_accel actual hardware acceleration support active on the video output device opened.
       * @param mode the mode in which the device was initially opened.
       * @param zoom the initial zoom level when de device was opened.
       * @param both_streams if a frame from both local and remote stream has been received.
       */
      sigc::signal4<void, VideoOutputAccel, VideoOutputMode, unsigned, bool> device_opened;

      /** This signal is emitted when a video output device is closed.
       */
      sigc::signal0<void> device_closed;

      /** This signal is emitted when an error occurs when opening a video output device.
       * @param error_code the video output device error code.
       */
      sigc::signal1<void, VideoOutputErrorCodes> device_error;

      /** This signal is emitted when a manager switches autonomously into or out of fullscreen mode.
       * Some managers like DX and XV  allow the user to switch between FS
       * by pressing a key or clicking a mouse button on top of the video. In order to
       * This signal is called whenever the size of the widget carrying the video signal
       * has to be changed. This happens when the displayed video changes in resolution
       * or when it is being zoomed in or out.
       * @param toggle VO_FS_ON or VO_FS_OFF depending on whether FS was activated or deactivated.
       */
      sigc::signal1<void, VideoOutputFSToggle> fullscreen_mode_changed;

      /** This signal is emitted the video output size has changed.
       * This signal is called whenever the size of the widget carrying the video signal
       * has to be changed. This happens when the displayed video changes in resolution
       * or when it is being zoomed in or out.
       * @param width the new width of the widget.
       * @param height the new height of the widget.
       */
      sigc::signal2<void, unsigned, unsigned> size_changed;

  protected:  
      virtual void get_display_info (DisplayInfo &) { };
    };

/**
 * @}
 */

};

#endif
