
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
 *                         audioinput-manager-null.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of a PTLIB audio input manager
 *
 */


#ifndef __AUDIOOUTPUT_MANAGER_NULL_H__
#define __AUDIOOUTPUT_MANAGER_NULL_H__

#include "audiooutput-manager.h"
#include "runtime.h"

#include <ptlib.h>
#include <ptclib/delaychan.h>

/**
 * @addtogroup audioinput
 * @{
 */

  class GMAudioOutputManager_null
   : public Ekiga::AudioOutputManager
    {
  public:

       GMAudioOutputManager_null (Ekiga::ServiceCore & core);

      ~GMAudioOutputManager_null ();

      virtual void get_devices (std::vector <Ekiga::AudioOutputDevice> & devices);

      virtual bool set_device (Ekiga::AudioOutputPS ps, const Ekiga::AudioOutputDevice & device);

      virtual bool open (Ekiga::AudioOutputPS ps, unsigned channels, unsigned samplerate, unsigned bits_per_sample);

      virtual void close(Ekiga::AudioOutputPS ps);

      virtual bool set_frame_data (Ekiga::AudioOutputPS ps, 
                                   const char *data, 
                                   unsigned size,
                                   unsigned & bytes_written);

      virtual bool has_device     (const std::string & sink, const std::string & device_name, Ekiga::AudioOutputDevice & device);

    protected:
      Ekiga::ServiceCore & core;

      PAdaptiveDelay adaptive_delay[2];

    private:
      void device_opened_in_main (Ekiga::AudioOutputPS ps,
				  Ekiga::AudioOutputDevice device,
				  Ekiga::AudioOutputSettings settings);
      void device_closed_in_main (Ekiga::AudioOutputPS ps,
				  Ekiga::AudioOutputDevice device);

  };

/**
 * @}
 */

#endif
