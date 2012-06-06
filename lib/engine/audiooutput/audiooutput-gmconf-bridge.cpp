
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
 *                          audiooutput-gmconf-bridge.h -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the bridge between the gmconf
 *                          and the audiooutput-core..
 *
 */

#include "config.h"

#include "audiooutput-gmconf-bridge.h"
#include "audiooutput-core.h"

#define AUDIO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/audio/"
#define AUDIO_CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/audio/"
#define SOUND_EVENTS_KEY  "/apps/" PACKAGE_NAME "/general/sound_events/"

using namespace Ekiga;

AudioOutputCoreConfBridge::AudioOutputCoreConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &AudioOutputCoreConfBridge::on_property_changed));

  keys.push_back (AUDIO_DEVICES_KEY "output_device"); 
  keys.push_back (SOUND_EVENTS_KEY "output_device"); 
  keys.push_back (SOUND_EVENTS_KEY "busy_tone_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "incoming_call_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "new_message_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "new_voicemail_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "ring_tone_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "enable_busy_tone_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "enable_incoming_call_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "enable_new_message_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "enable_new_voicemail_sound"); 
  keys.push_back (SOUND_EVENTS_KEY "enable_ring_tone_sound"); 

  load (keys);
}

void AudioOutputCoreConfBridge::on_property_changed (std::string key, GmConfEntry *entry)
{
  std::string name;
  std::string file_name;
  bool enabled;

  AudioOutputCore & audioinput_core = (AudioOutputCore &) service;

  if (key == AUDIO_DEVICES_KEY "output_device") {

    PTRACE(4, "AudioOutputCoreConfBridge\tUpdating device");

    AudioOutputDevice device;
    const gchar *audio_device = NULL;
    audio_device = gm_conf_entry_get_string (entry);

    if (audio_device == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" << AUDIO_DEVICES_KEY "output_device" << " is NULL");
    }
    else {
      device.SetFromString(audio_device);
    }
  
    if ( (device.type   == "" )   ||
         (device.source == "")  ||
         (device.name   == "" ) ) {
      PTRACE(1, "AudioOutputCore\tTried to set malformed device");
      device.type   = AUDIO_OUTPUT_FALLBACK_DEVICE_TYPE;
      device.source = AUDIO_OUTPUT_FALLBACK_DEVICE_SOURCE;
      device.name   = AUDIO_OUTPUT_FALLBACK_DEVICE_NAME;
    }

    audioinput_core.set_device (primary, device);
  }

  if (key == SOUND_EVENTS_KEY "output_device") {

    PTRACE(4, "AudioOutputCoreConfBridge\tUpdating device");
    AudioOutputDevice device;
    const gchar *audio_device = NULL;
    audio_device = gm_conf_entry_get_string (entry);

    if (audio_device == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" << AUDIO_DEVICES_KEY "output_device" << " is NULL");
    }
    else {
      device.SetFromString(audio_device);
    }

    if ( (device.type   == "" )   ||
         (device.source == "")  ||
         (device.name   == "" ) ) {
      PTRACE(1, "AudioOutputCore\tTried to set malformed device");
      device.type   = AUDIO_OUTPUT_FALLBACK_DEVICE_TYPE;
      device.source = AUDIO_OUTPUT_FALLBACK_DEVICE_SOURCE;
      device.name   = AUDIO_OUTPUT_FALLBACK_DEVICE_NAME;
    }
    audioinput_core.set_device (secondary, device);
  }

  if ( (key == SOUND_EVENTS_KEY "busy_tone_sound") ||
       (key == SOUND_EVENTS_KEY "enable_busy_tone_sound") ) {

    gchar *c_file_name = NULL;
    c_file_name = gm_conf_get_string (SOUND_EVENTS_KEY "busy_tone_sound");
    if (c_file_name == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" << SOUND_EVENTS_KEY "busy_tone_sound" << " is NULL");
      return;
    }

    name = "busy_tone_sound";
    file_name = c_file_name;
    g_free (c_file_name);
    enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_busy_tone_sound");
    audioinput_core.add_event (name, file_name, primary, enabled);
  }

  if ( (key == SOUND_EVENTS_KEY "incoming_call_sound") ||
       (key == SOUND_EVENTS_KEY "enable_incoming_call_sound") ) {

    gchar *c_file_name = NULL;
    c_file_name = gm_conf_get_string (SOUND_EVENTS_KEY "incoming_call_sound");
    if (c_file_name == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" <<  SOUND_EVENTS_KEY "incoming_call_sound" << " is NULL");
      return;
    }

    name = "incoming_call_sound";
    file_name = c_file_name;
    g_free (c_file_name);
    enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_incoming_call_sound");
    audioinput_core.add_event (name, file_name, secondary, enabled);
  }

  if ( (key == SOUND_EVENTS_KEY "new_message_sound") ||
       (key == SOUND_EVENTS_KEY "enable_new_message_sound") ) {

    gchar *c_file_name = NULL;
    c_file_name = gm_conf_get_string (SOUND_EVENTS_KEY "new_message_sound");
    if (c_file_name == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" <<  SOUND_EVENTS_KEY "new_message_sound" << " is NULL");
      return;
    }

    name = "new_message_sound";
    file_name = c_file_name;
    g_free (c_file_name);
    enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_new_message_sound");
    audioinput_core.add_event (name, file_name, secondary, enabled);

  }

  if ( (key == SOUND_EVENTS_KEY "new_voicemail_sound") ||
       (key == SOUND_EVENTS_KEY "enable_new_voicemail_sound") ) {

    gchar *c_file_name = NULL;
    c_file_name = gm_conf_get_string (SOUND_EVENTS_KEY "new_voicemail_sound");
    if (c_file_name == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" <<  SOUND_EVENTS_KEY "new_voicemail_sound" << " is NULL");
      return;
    }

    name = "new_voicemail_sound";
    file_name = c_file_name;
    g_free (c_file_name);
    enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_new_voicemail_sound");
    audioinput_core.add_event (name, file_name, secondary, enabled);
  }

  if ( (key == SOUND_EVENTS_KEY "ring_tone_sound") ||
       (key == SOUND_EVENTS_KEY "enable_ring_tone_sound") ) {

    gchar *c_file_name = NULL;
    c_file_name = gm_conf_get_string (SOUND_EVENTS_KEY "ring_tone_sound");
    if (c_file_name == NULL) {
      PTRACE(1, "AudioOutputCoreConfBridge\t" <<  SOUND_EVENTS_KEY "ring_tone_sound" << " is NULL");
      return;
    }

    name = "ring_tone_sound";
    file_name = c_file_name;
    g_free (c_file_name);
    enabled = gm_conf_get_bool (SOUND_EVENTS_KEY "enable_ring_tone_sound");
    audioinput_core.add_event (name, file_name, primary, enabled);
  }
}

