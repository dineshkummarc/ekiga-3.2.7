
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
 *                         audioinput-gmconf-bridge.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the bridge between the gmconf
 *                          and the audioinput-core..
 *
 */

#include "config.h"

#include "audioinput-gmconf-bridge.h"
#include "audioinput-core.h"

#define AUDIO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/audio/"
#define AUDIO_CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/audio/"

using namespace Ekiga;

AudioInputCoreConfBridge::AudioInputCoreConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &AudioInputCoreConfBridge::on_property_changed));

  keys.push_back (AUDIO_DEVICES_KEY "input_device"); 
  load (keys);
}

void AudioInputCoreConfBridge::on_property_changed (std::string key, GmConfEntry *entry)
{
  AudioInputCore & audioinput_core = (AudioInputCore &) service;

  if (key == AUDIO_DEVICES_KEY "input_device") {

    PTRACE(4, "AudioInputCoreConfBridge\tUpdating device");

    AudioInputDevice device;
    if (gm_conf_entry_get_string (entry) == NULL) {
      PTRACE(1, "AudioInputCoreConfBridge\t" << AUDIO_DEVICES_KEY "input_device" << " is NULL");
    }
    else {
      device.SetFromString(gm_conf_entry_get_string (entry));
    }

    if ( (device.type == "" )   ||
         (device.source == "")  ||
         (device.name == "" ) ) {
      PTRACE(1, "AudioInputCore\tTried to set malformed device");
      device.type = AUDIO_INPUT_FALLBACK_DEVICE_TYPE;
      device.source = AUDIO_INPUT_FALLBACK_DEVICE_SOURCE;
      device.name = AUDIO_INPUT_FALLBACK_DEVICE_NAME;
    }

    audioinput_core.set_device (device);
  }
}

