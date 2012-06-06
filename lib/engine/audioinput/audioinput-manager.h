
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
 *                         audioinput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of an audioinput manager
 *                          implementation backend.
 *
 */


#ifndef __AUDIOINPUT_MANAGER_H__
#define __AUDIOINPUT_MANAGER_H__

#include <vector>
#include <sigc++/sigc++.h>

#include "audioinput-info.h"

namespace Ekiga
{

/**
 * @addtogroup audioinput
 * @{
 */

  /** Generic implementation for the Ekiga::AudioInputManager class.
   *
   * Each AudioInputManager will represent a specific backend able to record audio.
   * Each AudioInputManager will manage one or more devices.
   * The AudioInputCore will control the different managers and their devices.
   */
  class AudioInputManager
    {

  public:

      /** The constructor
       */
      AudioInputManager () {}

      /** The destructor
       */
      virtual ~AudioInputManager () {}


      /*** API for audio input ***/

      /** Get a list of all devices supported by the manager. 
       * Add it to the list of devices already collected by the core.
       * @param devices a vector of device names to be filled by the manager.
       */
      virtual void get_devices (std::vector <AudioInputDevice> & devices) = 0;

      /** Set the current device.
       * Must be called before opening the device.
       * In case a different device of the same manager was opened before, it must be 
       * closed before setting the new device.
       * @param device the device to be used.
       */
      virtual bool set_device (const AudioInputDevice & device) = 0;

      /** Open the device.
       * The device must be opened before calling get_frame_data(), set_buffer_size() and set_volume().
       * Requires the set_device() to be called before.
       * Returns false if the device cannot be opened. Also sends a GUI callback to the main thread in that case.
       * @param channels number of channels (1=mono, 2=stereo).
       * @param samplerate the samplerate.
       * @param bits_per_sample the number bits per sample.
       * @return true if the opening succeeded. False if the device cannot be opened.
       */
      virtual bool open (unsigned channels, unsigned samplerate, unsigned bits_per_sample) = 0;

      /** Close the device.
       */
      virtual void close() {};

      /** Set the buffer size.
       * The buffer size must be set before calling get_frame_data().
       * Requires the device to be opened.
       * @param buffer_size the size of each buffer in bytes.
       * @param num_buffers the number of buffers.
       */
      virtual void set_buffer_size (unsigned /*buffer_size*/, unsigned /*num_buffers*/) {};

      /** Get one audio buffer.
       * This function will block until the buffer is completely filled.
       * Requires the device to be opened and the buffer size to be set.
       * Returns false if reading the device fails. Also sends a GUI callback to the main thread in that case.
       * @param data a pointer to the buffer that is to be filled. The memory has to be allocated already.
       * @param size the size of the buffer to be filled.
       * @param bytes_read returns the number of bytes actually read into the buffer. Should be equal to size.
       * @return false if the reading failed.
       */
      virtual bool get_frame_data (char *data, 
                                   unsigned size,
				   unsigned & bytes_read) = 0;

      /** Set the volume level for the current device.
       * Requires the device to be opened.
       * @param volume the new volume (0..255).
       */
      virtual void set_volume (unsigned /*volume*/) {};

      /** Returns true if a specific device is supported by the manager.
       * If the device specified by source and device_name is supported by the manager, true
       * is returned and an AudioOutputDevice structure filled with the respective details.
       * This function is used by the core to map added or removed devices to managers and AudioOutputDevices.
       * @param source the source type of the device (e.g. alsa, oss, etc.).
       * @param device_name the name of the device.
       * @param device in case the device is supported by the manager, this structure will be filled with the device details.
       * @return true if the device is supported by the manager.
       */
      virtual bool has_device (const std::string & source, const std::string & device_name, AudioInputDevice & device) = 0;


      /*** API to act on AudioInputDevice events ***/

      /** This signal is emitted when an audio input device is opened.
       * @param device the audio input device that was opened.
       * @param config the current audio input device configuration (current volume, etc.).
       */
      sigc::signal2<void, AudioInputDevice, AudioInputSettings> device_opened;

      /** This signal is emitted when an audio input device is closed.
       * @param device the audio input device that was closed.
       */
      sigc::signal1<void, AudioInputDevice> device_closed;

      /** This signal is emitted when an error occurs when opening a audio input device.
       * @param device the audio input device that caused the error.
       * @param error_code the audio input device error code.
       */
      sigc::signal2<void, AudioInputDevice, AudioInputErrorCodes> device_error;


  protected:  
      typedef struct ManagerState {
        bool opened;
        unsigned channels;
        unsigned samplerate;
        unsigned bits_per_sample;
        AudioInputDevice device;
      } ManagerState;

      ManagerState current_state;
  };
/**
 * @}
 */

};

#endif
