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
 *                         audiooutput-manager-null.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of a NULL audio output manager
 *
 */

#include "audiooutput-manager-null.h"

#define DEVICE_TYPE   "Ekiga"
#define DEVICE_SOURCE "Ekiga"
#define DEVICE_NAME   "SILENT"

GMAudioOutputManager_null::GMAudioOutputManager_null (Ekiga::ServiceCore & _core)
: core (_core)
{
  current_state[Ekiga::primary].opened = false;
  current_state[Ekiga::secondary].opened = false;
}

GMAudioOutputManager_null::~GMAudioOutputManager_null ()
{
}

void GMAudioOutputManager_null::get_devices(std::vector <Ekiga::AudioOutputDevice> & devices)
{
  Ekiga::AudioOutputDevice device;
  device.type   = DEVICE_TYPE;
  device.source = DEVICE_SOURCE;
  device.name   = DEVICE_NAME;
  devices.push_back(device);
}


bool GMAudioOutputManager_null::set_device (Ekiga::AudioOutputPS ps, const Ekiga::AudioOutputDevice & device)
{
  if ( ( device.type   == DEVICE_TYPE ) &&
       ( device.source == DEVICE_SOURCE) &&
       ( device.name   == DEVICE_NAME) ) {

    PTRACE(4, "GMAudioOutputManager_null\tSetting Device[" << ps << "] " << device);
    current_state[ps].device = device;
    return true;
  }
  return false;
}

bool GMAudioOutputManager_null::open (Ekiga::AudioOutputPS ps, unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  current_state[ps].channels        = channels;
  current_state[ps].samplerate      = samplerate;
  current_state[ps].bits_per_sample = bits_per_sample;
  current_state[ps].opened = true;

  PTRACE(4, "GMAudioOutputManager_null\tOpening Device[" << ps << "] " << current_state[ps].device);
  PTRACE(4, "GMAudioOutputManager_null\tOpening Device with " << channels << "-" << samplerate << "/" << bits_per_sample);

  adaptive_delay[ps].Restart();

  Ekiga::AudioOutputSettings settings;
  settings.volume = 0;
  settings.modifyable = false;
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMAudioOutputManager_null::device_opened_in_main), ps, current_state[ps].device, settings));

  return true;
}

void GMAudioOutputManager_null::close(Ekiga::AudioOutputPS ps)
{
  current_state[ps].opened = false;
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMAudioOutputManager_null::device_closed_in_main), ps, current_state[ps].device));
}


bool GMAudioOutputManager_null::set_frame_data (Ekiga::AudioOutputPS ps, 
                     const char */*data*/, 
                     unsigned size,
		     unsigned & bytes_written)
{
  if (!current_state[ps].opened) {
    PTRACE(1, "GMAudioOutputManager_null\tTrying to get frame from closed device[" << ps << "]");
    return true;
  }

  bytes_written = size;

  adaptive_delay[ps].Delay(size * 8 / current_state[ps].bits_per_sample * 1000 / current_state[ps].samplerate);
  return true;
}

bool GMAudioOutputManager_null::has_device(const std::string & /*sink*/, const std::string & /*device_name*/, Ekiga::AudioOutputDevice & /*device*/)
{
  return false;
}

void
GMAudioOutputManager_null::device_opened_in_main (Ekiga::AudioOutputPS ps,
						  Ekiga::AudioOutputDevice device,
						  Ekiga::AudioOutputSettings settings)
{
  device_opened.emit (ps, device, settings);
}

void
GMAudioOutputManager_null::device_closed_in_main (Ekiga::AudioOutputPS ps,
						  Ekiga::AudioOutputDevice device)
{
  device_closed.emit (ps, device);
}
