
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
 *                         audiooutput-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of an audiooutput manager
 *                          implementation backend.
 *
 */


#ifndef __AUDIOOUTPUT_MANAGER_H__
#define __AUDIOOUTPUT_MANAGER_H__

#include <vector>
#include <sigc++/sigc++.h>

#include "audiooutput-info.h"

namespace Ekiga
{

/**
 * @addtogroup audiooutput
 * @{
 */

  /** Generic implementation for the Ekiga::AudioOutputManager class.
   *
   * Each AudioOutputManager will represent a specific backend able to play back audio.
   * Each AudioOutputManager will manage one or more devices.
   * The AudioOutputCore will control the different managers and their devices.
   */
  class AudioOutputManager
    {

  public:

      /** The constructor
       */
      AudioOutputManager () {}

      /** The destructor
       */
      virtual ~AudioOutputManager () {}


      /*** API for audio output ***/

      /** Get a list of all devices supported by the manager.
       * Add it to the list of devices already collected by the core.
       * @param devices a vector of device names to be filled by the manager.
       */
      virtual void get_devices (std::vector <AudioOutputDevice> & devices) = 0;

      /** Set the current device.
       * Must be called before opening the device.
       * In case a different device of the same manager was opened before, it must be 
       * closed before setting the new device.
       * @param prim whether to set the primary or secondary device.
       * @param device the device to be used.
       */
      virtual bool set_device (AudioOutputPS ps, const AudioOutputDevice & device) = 0;

      /** Open the device.
       * The device must be opened before calling set_frame_data(), set_buffer_size() and set_volume().
       * Requires the set_device() to be called before.
       * Returns false if the device cannot be opened. Also sends a GUI callback to the main thread in that case.
       * @param prim whether the device shall be opened as primary or secondary device.
       * @param channels number of channels (1=mono, 2=stereo).
       * @param samplerate the samplerate.
       * @param bits_per_sample the number bits per sample.
       * @return true if the opening succeeded. False if the device cannot be opened.
       */
      virtual bool open (AudioOutputPS ps, unsigned channels, unsigned samplerate, unsigned bits_per_sample) = 0;

      /** Close the device.
       */
      virtual void close (AudioOutputPS /*ps*/) {};

      /** Set the buffer size.
       * The buffer size must be set before calling set_frame_data().
       * Requires the device to be opened.
       * @param prim whether buffers of the primary or secodnary device shall be set.
       * @param buffer_size the size of each buffer in bytes.
       * @param num_buffers the number of buffers.
       */
      virtual void set_buffer_size (AudioOutputPS /*ps*/, unsigned /*buffer_size*/, unsigned /*num_buffers*/) {};

      /** Set one audio buffer.
       * Requires the device to be opened and the buffer size to be set.
       * Returns false if writing the device fails. Also sends a GUI callback to the main thread in that case.
       * @param data a pointer to the buffer with the data to be written. It will not be freed.
       * @param size the size of the buffer to be written.
       * @param bytes_written returns the number of bytes actually written. Should be equal to size.
       * @return false if the writing failed.
       */
      virtual bool set_frame_data (AudioOutputPS ps, 
                                   const char *data,
                                   unsigned size,
                                   unsigned & bytes_written) = 0;

      /** Set the volume level for the current device.
       * Requires the device to be opened.
       * @param prim wether the volume of the primary or secondary device shall be set.
       * @param volume the new volume (0..255).
       */
      virtual void set_volume (AudioOutputPS /*ps*/, unsigned /* volume */ ) {};

      /** Returns true if a specific device is supported by the manager.
       * If the device specified by sink and device_name is supported by the manager, true
       * is returned and an AudioOutputDevice structure filled with the respective details.
       * @param sink the sink type of the device (e.g. alsa, oss, etc.).
       * @param device_name the name of the device.
       * @param device in case the device is supported by the manager, this structure will be filled with the device details.
       * @return true if the device is supported by the manager.
       */
      virtual bool has_device (const std::string & sink, const std::string & device_name, AudioOutputDevice & device) = 0;


      /*** API to act on AudioOutputDevice events ***/

      /** This signal is emitted when an audio output device is opened.
       * @param prim whether the primary or secondary audio output device was opened.
       * @param device the audio output device that was opened.
       * @param config the current audio output device configuration (current volume, etc.).
       */
      sigc::signal3<void, AudioOutputPS, AudioOutputDevice, AudioOutputSettings> device_opened;

      /** This signal is emitted when an audio output device is closed.
       * @param prim whether the primary or secondary audio output device was closed.
       * @param device the audio output device that was closed.
       */
      sigc::signal2<void, AudioOutputPS, AudioOutputDevice> device_closed;

      /** This signal is emitted when an error occurs when opening an audio output device.
       * @param prim whether the primary or secondary audio output device caused the error.
       * @param device the audio output device that caused the error.
       * @param error_code the audio output device error code.
       */
      sigc::signal3<void, AudioOutputPS, AudioOutputDevice, AudioOutputErrorCodes> device_error;

  protected:  
      typedef struct ManagerState {
        bool opened;
        unsigned channels;
        unsigned samplerate;
        unsigned bits_per_sample;
        AudioOutputDevice device;
      } ManagerState;

      ManagerState current_state[2];
  };
/**
 * @}
 */

};

#endif
