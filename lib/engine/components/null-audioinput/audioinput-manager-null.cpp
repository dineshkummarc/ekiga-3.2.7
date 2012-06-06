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
 *                         audioinput-manager-null.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of a NULL audio input manager
 *
 */

#include "audioinput-manager-null.h"

#define DEVICE_TYPE   "Ekiga"
#define DEVICE_SOURCE "Ekiga"
#define DEVICE_NAME   "SILENT"


GMAudioInputManager_null::GMAudioInputManager_null (Ekiga::ServiceCore & _core)
:    core (_core)
{
  current_state.opened = false;
}

GMAudioInputManager_null::~GMAudioInputManager_null ()
{
}

void GMAudioInputManager_null::get_devices(std::vector <Ekiga::AudioInputDevice> & devices)
{
  Ekiga::AudioInputDevice device;
  device.type   = DEVICE_TYPE;
  device.source = DEVICE_SOURCE;
  device.name   = DEVICE_NAME;
  devices.push_back(device);
}

bool GMAudioInputManager_null::set_device (const Ekiga::AudioInputDevice & device)
{
  if ( ( device.type   == DEVICE_TYPE ) &&
       ( device.source == DEVICE_SOURCE) &&
       ( device.name   == DEVICE_NAME) ) {

    PTRACE(4, "GMAudioInputManager_null\tSetting Device " << device);
    current_state.device = device;
    return true;
  }
  return false;
}

bool GMAudioInputManager_null::open (unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PTRACE(4, "GMAudioInputManager_null\tOpening Device " << current_state.device);
  PTRACE(4, "GMAudioInputManager_null\tOpening Device with " << channels << "-" << samplerate << "/" << bits_per_sample);

  current_state.channels        = channels;
  current_state.samplerate      = samplerate;
  current_state.bits_per_sample = bits_per_sample;
  current_state.opened = true;

  adaptive_delay.Restart();

  Ekiga::AudioInputSettings settings;
  settings.volume = 0;
  settings.modifyable = false;
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMAudioInputManager_null::device_opened_in_main), current_state.device, settings));

  return true;
}

void GMAudioInputManager_null::close()
{
  current_state.opened = false;
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMAudioInputManager_null::device_closed_in_main), current_state.device));
}


bool GMAudioInputManager_null::get_frame_data (char *data, 
                                                unsigned size, unsigned & bytes_read)
{
  if (!current_state.opened) {
    PTRACE(1, "GMAudioInputManager_null\tTrying to get frame from closed device");
    return true;
  }

  memset (data, 0, size);

  bytes_read = size;

  adaptive_delay.Delay(size * 8 / current_state.bits_per_sample * 1000 / current_state.samplerate);

  return true;
}

bool GMAudioInputManager_null::has_device(const std::string & /*source*/, const std::string & /*device_name*/, Ekiga::AudioInputDevice & /*device*/)
{
  return false;
}

void
GMAudioInputManager_null::device_opened_in_main (Ekiga::AudioInputDevice device,
						 Ekiga::AudioInputSettings settings)
{
  device_opened.emit (device, settings);
}

void
GMAudioInputManager_null::device_closed_in_main (Ekiga::AudioInputDevice device)
{
  device_closed.emit (device);
}
