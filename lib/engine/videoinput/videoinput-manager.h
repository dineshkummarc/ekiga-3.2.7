
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
 *                         videoinput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a videoinput manager
 *                          implementation backend.
 *
 */


#ifndef __VIDEOINPUT_MANAGER_H__
#define __VIDEOINPUT_MANAGER_H__

#include <vector>
#include <sigc++/sigc++.h>

#include "videoinput-info.h"

namespace Ekiga
{

/**
 * @addtogroup videoinput
 * @{
 */

  /** Generic implementation for the Ekiga::VideoInputManager class.
   *
   * Each VideoInputManager will represent a specific backend able to record video.
   * Each VideoInputManager will manage one or more devices.
   * The VideoInputCore will control the different managers and their devices.
   */
  class VideoInputManager
    {

  public:

      /** The constructor
       */
      VideoInputManager () {}

      /** The destructor
       */
      virtual ~VideoInputManager () {}


      /*** API for video input ***/

      /** Get a list of all devices supported by the manager.
       * Add it to the list of devices already collected by the core.
       * @param devices a vector of device names to be filled by the manager.
       */
      virtual void get_devices (std::vector <VideoInputDevice> & devices) = 0;

      /** Set the current device.
       * Must be called before opening the device.
       * In case a different device of the same manager was opened before, it must be 
       * closed before setting the new device.
       * @param device the device to be used.
       * @param channel the channel for which the device shall be opened later.
       * @param format the video formar to be used (PAL, NTSC, ...).

       */
      virtual bool set_device (const VideoInputDevice & device, int channel, VideoInputFormat format) = 0;

      /** Open the device.
       * The device must be opened before calling get_frame_data and set_* functions.
       * Requires the set_device() to be called before.
       * Returns false if the device cannot be opened. Also sends a GUI callback to the main thread in that case.
       * @param width the frame width in pixels for which frames shall be supplied.
       * @param height the frame width in pixels for which frames shall be supplied.
       * @param fps the frame rate in frames per second in which frames shall be supplied.
       * @return true if the opening succeeded. False if the device cannot be opened.
       */
      virtual bool open (unsigned width, unsigned height, unsigned fps) = 0;

      /** Close the device.
       */
      virtual void close() {};

      /** Get one video frame buffer.
       * This function will block until the buffer is completely filled.
       * Requires the device to be opened.
       * Returns false if reading the device fails. Also sends a GUI callback to the main thread in that case.
       * @param data a pointer to the frame buffer that is to be filled. The memory has to be allocated already.
       * @return false if the reading failed.
       */
      virtual bool get_frame_data (char * data) = 0;

      virtual void set_image_data (unsigned /* width */, unsigned /* height */, const char* /*data*/ ) {};

      /** Set the colour for the current input device.
       * Requires the device to be opened.
       * @param colour the new colour (0..255).
       */
      virtual void set_colour     (unsigned /* colour */ ) {};
 
     /** Set the brightness for the current input device.
       * Requires the device to be opened.
       * @param brightness the new brightness (0..255).
       */
      virtual void set_brightness (unsigned /* brightness */ ) {};
 
     /** Set the whiteness for the current input device.
       * Requires the device to be opened.
       * @param whiteness the new whiteness (0..255).
       */
      virtual void set_whiteness  (unsigned /* whiteness */ ) {};
 
     /** Set the contrast for the current input device.
       * Requires the device to be opened.
       * @param contrast the new contrast (0..255).
       */
      virtual void set_contrast   (unsigned /* contrast */ ) {};

      /** Returns true if a specific device is supported by the manager.
       * If the device specified by source and device_name is supported by the manager, true
       * is returned and an VideoInputDevice structure filled with the respective details.
       * This function is used by the core to map added or removed devices to managers and VideoInputDevices.
       * @param source the source type of the device (e.g. alsa, oss, etc.).
       * @param device_name the name of the device.
       * @param device in case the device is supported by the manager, this structure will be filled with the device details.
       * @return true if the device is supported by the manager.
       */
      virtual bool has_device     (const std::string & source, const std::string & device_name, unsigned capabilities, VideoInputDevice & device) = 0;


      /*** API to act on VidInputDevice events ***/

      /** This signal is emitted when a video input device is opened.
       * @param device the video input device that was opened.
       * @param config the current video input device configuration (current brightness, colour, etc.).
       */
      sigc::signal2<void, VideoInputDevice, VideoInputSettings> device_opened;

      /** This signal is emitted when a video input device is closed.
       * @param device the video input device that was closed.
       */
      sigc::signal1<void, VideoInputDevice> device_closed;

      /** This signal is emitted when an error occurs when opening a video input device.
       * @param device the video input device that caused the error.
       * @param error_code the video input device error code.
       */
      sigc::signal2<void, VideoInputDevice, VideoInputErrorCodes> device_error;

  protected:  
      typedef struct ManagerState {
        bool opened;
        unsigned width;
        unsigned height;
        unsigned fps;
        VideoInputDevice device;
        VideoInputFormat format;
        int channel;
      } ManagerState;

      ManagerState current_state;
  };
/**
 * @}
 */

};

#endif
