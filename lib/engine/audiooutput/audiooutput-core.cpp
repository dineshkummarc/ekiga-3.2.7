
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
 *                         audiooutput-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a audiooutput core.
 *                          An audiooutput core manages AudioOutputManagers.
 *
 */
#include "audiooutput-core.h"
#include "audiooutput-manager.h"
#include <algorithm>
#include <math.h>

using namespace Ekiga;

AudioOutputCore::AudioOutputCore ()
:  audio_event_scheduler(*this)
{
  PWaitAndSignal m_pri(core_mutex[primary]);
  PWaitAndSignal m_sec(core_mutex[secondary]);
  PWaitAndSignal m_vol(volume_mutex);

  current_primary_config.active = false;
  current_primary_config.channels = 0;
  current_primary_config.samplerate = 0;
  current_primary_config.bits_per_sample = 0;
  current_primary_config.buffer_size = 0;
  current_primary_config.num_buffers = 0;

  current_primary_volume = 0;
  desired_primary_volume = 0;
  
  current_manager[primary] = NULL;
  current_manager[secondary] = NULL;
  audiooutput_core_conf_bridge = NULL;
  average_level = 0;
  calculate_average = false;
  yield = false;
}

AudioOutputCore::~AudioOutputCore ()
{
  PWaitAndSignal m_pri(core_mutex[primary]);
  PWaitAndSignal m_sec(core_mutex[secondary]);

  if (audiooutput_core_conf_bridge)
    delete audiooutput_core_conf_bridge;

  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    delete (*iter);

  managers.clear();
}

void AudioOutputCore::setup_conf_bridge ()
{
  PWaitAndSignal m_pri(core_mutex[primary]);
  PWaitAndSignal m_sec(core_mutex[secondary]);

   audiooutput_core_conf_bridge = new AudioOutputCoreConfBridge (*this);
}

void AudioOutputCore::add_manager (AudioOutputManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.device_error.connect (sigc::bind (sigc::mem_fun (this, &AudioOutputCore::on_device_error), &manager));
  manager.device_opened.connect (sigc::bind (sigc::mem_fun (this, &AudioOutputCore::on_device_opened), &manager));
  manager.device_closed.connect (sigc::bind (sigc::mem_fun (this, &AudioOutputCore::on_device_closed), &manager));
}

void AudioOutputCore::visit_managers (sigc::slot1<bool, AudioOutputManager &> visitor)
{
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);
  PWaitAndSignal m_sec(core_mutex[secondary]);
  bool go_on = true;
  
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}

void AudioOutputCore::add_event (const std::string & event_name, const std::string & file_name, AudioOutputPS ps,  bool enabled)
{
  audio_event_scheduler.set_file_name(event_name, file_name, ps, enabled);
}

void AudioOutputCore::play_file (const std::string & file_name)
{
  audio_event_scheduler.add_event_to_queue(file_name, true, 0, 0);
}

void AudioOutputCore::play_event (const std::string & event_name)
{
  audio_event_scheduler.add_event_to_queue(event_name, false, 0, 0);
}

void AudioOutputCore::start_play_event (const std::string & event_name, unsigned interval, unsigned repetitions)
{
  audio_event_scheduler.add_event_to_queue(event_name, false, interval, repetitions);
}

void AudioOutputCore::stop_play_event (const std::string & event_name)
{
  audio_event_scheduler.remove_event_from_queue(event_name);
}

void AudioOutputCore::get_devices (std::vector <AudioOutputDevice> & devices)
{
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);
  PWaitAndSignal m_sec(core_mutex[secondary]);

  devices.clear();

  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++)
    (*iter)->get_devices (devices);

#if PTRACING
  for (std::vector<AudioOutputDevice>::iterator iter = devices.begin ();
       iter != devices.end ();
       iter++) {
    PTRACE(4, "AudioOutputCore\tDetected Device: " << *iter);
  }
#endif

}

void AudioOutputCore::set_device(AudioOutputPS ps, const AudioOutputDevice & device)
{
  PTRACE(4, "AudioOutputCore\tSetting device[" << ps << "]: " << device);
  yield = true;
  PWaitAndSignal m_sec(core_mutex[secondary]);

  switch (ps) {
    case primary:
      yield = true;
      core_mutex[primary].Wait();
      internal_set_primary_device (device);
      desired_primary_device = device;
      core_mutex[primary].Signal();

      break;
    case secondary:
        if (device == current_device[primary])
        {
          current_manager[secondary] = NULL;
          current_device[secondary].type = "";
          current_device[secondary].source = "";
          current_device[secondary].name = "";
        }
        else {
          internal_set_manager (secondary, device);
        }
        break;
    default:
      break;
  }
}

void AudioOutputCore::add_device (const std::string & sink, const std::string & device_name, HalManager* /*manager*/)
{
  PTRACE(4, "AudioOutputCore\tAdding Device " << device_name);
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);

  AudioOutputDevice device;
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (sink, device_name, device)) {

       if ( desired_primary_device == device ) {
         internal_set_primary_device(desired_primary_device);
       }

       device_added.emit(device, desired_primary_device == device);
     }
  }
}

void AudioOutputCore::remove_device (const std::string & sink, const std::string & device_name, HalManager* /*manager*/)
{
  PTRACE(4, "AudioOutputCore\tRemoving Device " << device_name);
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);

  AudioOutputDevice device;
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->has_device (sink, device_name, device)) {
       if ( (device == current_device[primary]) && (current_primary_config.active) ) {

         AudioOutputDevice new_device;
         new_device.type   = AUDIO_OUTPUT_FALLBACK_DEVICE_TYPE;
         new_device.source = AUDIO_OUTPUT_FALLBACK_DEVICE_SOURCE;
         new_device.name   = AUDIO_OUTPUT_FALLBACK_DEVICE_NAME;
         internal_set_primary_device(new_device);
       }

       device_removed.emit(device, device == current_device[primary]);
     }
  }
}

void AudioOutputCore::start (unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);

  if (current_primary_config.active) {
    PTRACE(1, "AudioOutputCore\tTrying to start output device although already started");
    return;
  }

  internal_set_manager(primary, desired_primary_device);    /* may be left undetermined after the last call */

  average_level = 0;
  internal_open(primary, channels, samplerate, bits_per_sample);
  current_primary_config.active = true;
  current_primary_config.channels = channels;
  current_primary_config.samplerate = samplerate;
  current_primary_config.bits_per_sample = bits_per_sample;
  current_primary_config.buffer_size = 0;
  current_primary_config.num_buffers = 0;
}

void AudioOutputCore::stop()
{
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);

  average_level = 0;
  internal_close(primary);
  internal_set_manager(primary, desired_primary_device);

  current_primary_config.active = false;
}

void AudioOutputCore::set_buffer_size (unsigned buffer_size, unsigned num_buffers) {
  yield = true;
  PWaitAndSignal m_pri(core_mutex[primary]);

  if (current_manager[primary])
    current_manager[primary]->set_buffer_size (primary, buffer_size, num_buffers);

  current_primary_config.buffer_size = buffer_size;
  current_primary_config.num_buffers = num_buffers;
}

void AudioOutputCore::set_frame_data (const char *data,
                                      unsigned size,
				      unsigned & bytes_written)
{
  if (yield) {
    yield = false;
    PThread::Current()->Sleep(5);
  }
  PWaitAndSignal m_pri(core_mutex[primary]);

  if (current_manager[primary]) {
    if (!current_manager[primary]->set_frame_data(primary, data, size, bytes_written)) {
      internal_close(primary);
      internal_set_primary_fallback();
      internal_open(primary, current_primary_config.channels, current_primary_config.samplerate, current_primary_config.bits_per_sample);
      if (current_manager[primary])
        current_manager[primary]->set_frame_data(primary, data, size, bytes_written); // the default device must always return true
    }

    PWaitAndSignal m_vol(volume_mutex);
    if (desired_primary_volume != current_primary_volume) {
      current_manager[primary]->set_volume(primary, desired_primary_volume);
      current_primary_volume = desired_primary_volume;
    }
  }

  if (calculate_average) 
    calculate_average_level((const short*) data, bytes_written);
}

void AudioOutputCore::set_volume (AudioOutputPS ps, unsigned volume)
{
  PWaitAndSignal m_vol(volume_mutex);

  if (ps == primary) {
    desired_primary_volume = volume;
  }
}

void AudioOutputCore::play_buffer(AudioOutputPS ps, const char* buffer, unsigned long len, unsigned channels, unsigned sample_rate, unsigned bps)
{
  switch (ps) {
    case primary:
      core_mutex[primary].Wait();

      if (!current_manager[primary]) {
        PTRACE(1, "AudioOutputCore\tDropping sound event, primary manager not set");
        core_mutex[primary].Signal();
        return;
      }

      if (current_primary_config.active) {
        PTRACE(1, "AudioOutputCore\tDropping sound event, primary device not set");
        core_mutex[primary].Signal();
        return;
      }
      internal_play(primary, buffer, len, channels, sample_rate, bps);
      core_mutex[primary].Signal();

      break;
    case secondary:
        core_mutex[secondary].Wait();
 
        if (current_manager[secondary]) {
             internal_play(secondary, buffer, len, channels, sample_rate, bps);
          core_mutex[secondary].Signal();
        }
        else {
          core_mutex[secondary].Signal();
          PTRACE(1, "AudioOutputCore\tNo secondary audiooutput device defined, trying primary");
          play_buffer(primary, buffer, len, channels, sample_rate, bps);
        }
      break;
    default:
      break;
  }
}

void AudioOutputCore::on_device_opened (AudioOutputPS ps,
                                        AudioOutputDevice device,
                                        AudioOutputSettings settings, 
                                        AudioOutputManager *manager)
{
  device_opened.emit (*manager, ps, device, settings);
}

void AudioOutputCore::on_device_closed (AudioOutputPS ps, AudioOutputDevice device, AudioOutputManager *manager)
{
  device_closed.emit (*manager, ps, device);
}

void AudioOutputCore::on_device_error (AudioOutputPS ps, AudioOutputDevice device, AudioOutputErrorCodes error_code, AudioOutputManager *manager)
{
  device_error.emit (*manager, ps, device, error_code);
}

void AudioOutputCore::internal_set_primary_device(const AudioOutputDevice & device)
{
  if (current_primary_config.active)
     internal_close(primary);

  if (device == current_device[secondary]) {

    current_manager[secondary] = NULL;
    current_device[secondary].type = "";
    current_device[secondary].source = "";
    current_device[secondary].name = "";
  }

  internal_set_manager(primary, device);

  if (current_primary_config.active)
    internal_open(primary, current_primary_config.channels, current_primary_config.samplerate, current_primary_config.bits_per_sample);

  if ((current_primary_config.buffer_size > 0) && (current_primary_config.num_buffers > 0 ) ) {
    if (current_manager[primary])
      current_manager[primary]->set_buffer_size (primary, current_primary_config.buffer_size, current_primary_config.num_buffers);
  }
}
void AudioOutputCore::internal_set_manager (AudioOutputPS ps, const AudioOutputDevice & device)
{
  current_manager[ps] = NULL;
  for (std::set<AudioOutputManager *>::iterator iter = managers.begin ();
       iter != managers.end ();
       iter++) {
     if ((*iter)->set_device (ps, device)) {
       current_manager[ps] = (*iter);
     }
  }

  if (current_manager[ps]) {
    current_device[ps]  = device;
  }
  else {
    if (ps == primary) {
      PTRACE(1, "AudioOutputCore\tTried to set unexisting primary device " << device);
      internal_set_primary_fallback();
    }
    else {
      PTRACE(1, "AudioOutputCore\tTried to set unexisting secondary device " << device);
      current_device[secondary].type = "";
      current_device[secondary].source = "";
      current_device[secondary].name = "";
    }
  }

}

void AudioOutputCore::internal_set_primary_fallback()
{
  current_device[primary].type   = AUDIO_OUTPUT_FALLBACK_DEVICE_TYPE;
  current_device[primary].source = AUDIO_OUTPUT_FALLBACK_DEVICE_SOURCE;
  current_device[primary].name   = AUDIO_OUTPUT_FALLBACK_DEVICE_NAME;
  PTRACE(1, "AudioOutputCore\tFalling back to " << current_device[primary]);
  internal_set_manager(primary, current_device[primary]);
}

bool AudioOutputCore::internal_open (AudioOutputPS ps, unsigned channels, unsigned samplerate, unsigned bits_per_sample)
{
  PTRACE(4, "AudioOutputCore\tOpening device["<<ps<<"] with " << channels<< "-" << samplerate << "/" << bits_per_sample);

  if (!current_manager[ps]) {
    PTRACE(1, "AudioOutputCore\tUnable to obtain current manager for device["<<ps<<"]");
    return false;
  }

  if (!current_manager[ps]->open(ps, channels, samplerate, bits_per_sample)) {
    PTRACE(1, "AudioOutputCore\tUnable to open device["<<ps<<"]");
    if (ps == primary) {
      internal_set_primary_fallback();
      if (current_manager[primary])
        current_manager[primary]->open(ps, channels, samplerate, bits_per_sample);
      return true;
    }
    else {
      return false;
    }
  }
  return true;
}

void AudioOutputCore::internal_close(AudioOutputPS ps)
{
  PTRACE(4, "AudioOutputCore\tClosing current device");
  if (current_manager[ps])
    current_manager[ps]->close(ps);
}

void AudioOutputCore::internal_play(AudioOutputPS ps, const char* buffer, unsigned long len, unsigned channels, unsigned sample_rate, unsigned bps)
{
  unsigned long pos = 0;
  unsigned bytes_written = 0;
  unsigned buffer_size = (unsigned)((float)sample_rate/25);

  if (!internal_open ( ps, channels, sample_rate, bps))
    return;

  if (current_manager[ps]) {
    current_manager[ps]->set_buffer_size (ps, buffer_size, 4);
    do {
      if (!current_manager[ps]->set_frame_data(ps, buffer+pos, std::min(buffer_size, (unsigned) (len - pos)), bytes_written))
        break;
      pos += buffer_size;
    } while (pos < len);
  }

  internal_close( ps);
}

void AudioOutputCore::calculate_average_level (const short *buffer, unsigned size)
{
  int sum = 0;
  unsigned csize = 0;
  
  while (csize < (size>>1) ) {

    if (*buffer < 0)
      sum -= *buffer++;
    else
      sum += *buffer++;

    csize++;
  }
	  
  average_level = log10 (9.0*sum/size/32767+1)*1.0;
}
