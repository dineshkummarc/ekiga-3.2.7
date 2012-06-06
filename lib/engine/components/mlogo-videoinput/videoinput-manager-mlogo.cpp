
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
 *                         videoinput-manager-mlogo.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a videoinput core.
 *                          A vidinput core manages VideoInputManagers.
 *
 */

#include "videoinput-manager-mlogo.h"
#include "icon.h"

#define DEVICE_TYPE   "Moving Logo"
#define DEVICE_SOURCE "Moving Logo"
#define DEVICE_NAME   "Moving Logo"

GMVideoInputManager_mlogo::GMVideoInputManager_mlogo (Ekiga::ServiceCore & _core)
: core (_core)
{
  current_state.opened  = false;
}

GMVideoInputManager_mlogo::~GMVideoInputManager_mlogo ()
{
}

void GMVideoInputManager_mlogo::get_devices(std::vector <Ekiga::VideoInputDevice> & devices)
{
  Ekiga::VideoInputDevice device;
  device.type   = DEVICE_TYPE;
  device.source = DEVICE_SOURCE;
  device.name   = DEVICE_NAME;
  devices.push_back(device);
}

bool GMVideoInputManager_mlogo::set_device (const Ekiga::VideoInputDevice & device, int channel, Ekiga::VideoInputFormat format)
{
  if ( ( device.type   == DEVICE_TYPE ) &&
       ( device.source == DEVICE_SOURCE) &&
       ( device.name   == DEVICE_NAME) ) {

    PTRACE(4, "GMVideoInputManager_mlogo\tSetting Device Moving Logo");
    current_state.device  = device;
    current_state.channel = channel;
    current_state.format  = format;
    return true;
  }
  return false;
}

bool GMVideoInputManager_mlogo::open (unsigned width, unsigned height, unsigned fps)
{
  PTRACE(4, "GMVideoInputManager_mlogo\tOpening Moving Logo with " << width << "x" << height << "/" << fps);
  current_state.width  = width;
  current_state.height = height;
  current_state.fps    = fps;

  pos = 0;
  increment = 1;

  background_frame = (char*) malloc ((current_state.width * current_state.height * 3) >> 1);
  memset (background_frame, 0xd3, current_state.width*current_state.height); //ff
  memset (background_frame + (current_state.width * current_state.height), 
          0x7f, 
          (current_state.width*current_state.height) >> 2);
  memset (background_frame + (current_state.width * current_state.height) + 
                            ((current_state.width * current_state.height) >> 2), 
          0x7f,
          (current_state.width*current_state.height) >> 2);

  adaptive_delay.Restart();
  adaptive_delay.SetMaximumSlip((unsigned )( 500.0 / fps));

  current_state.opened  = true;

  Ekiga::VideoInputSettings settings;
  settings.whiteness = 127;
  settings.brightness = 127;
  settings.colour = 127;
  settings.contrast = 127;
  settings.modifyable = false;
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoInputManager_mlogo::device_opened_in_main), current_state.device, settings));
  
  return true;
}

void GMVideoInputManager_mlogo::close()
{
  PTRACE(4, "GMVideoInputManager_mlogo\tClosing Moving Logo");
  free (background_frame);
  current_state.opened  = false;
  Ekiga::Runtime::run_in_main (sigc::bind (sigc::mem_fun (this, &GMVideoInputManager_mlogo::device_closed_in_main), current_state.device));
}

bool GMVideoInputManager_mlogo::get_frame_data (char *data)
{
  if (!current_state.opened) {
    PTRACE(1, "GMVideoInputManager_mlogo\tTrying to get frame from closed device");
    return true;
  }
  
  adaptive_delay.Delay (1000 / current_state.fps);

  memcpy (data, background_frame, (current_state.width * current_state.height * 3) >> 1);

  CopyYUVArea  ((char*)&gm_icon_yuv, 
                gm_icon_width, gm_icon_height, 
                data, 
                (current_state.width - gm_icon_width) >> 1, 
                pos, 
                current_state.width, current_state.height);
  pos = pos + increment;

  if ( pos > current_state.height - gm_icon_height - 10) 
    increment = -1;
  if (pos < 10) 
    increment = +1;

  return true;
}

void GMVideoInputManager_mlogo::CopyYUVArea (const char* srcFrame,
					 unsigned srcWidth,
					 unsigned srcHeight,
					 char* dstFrame,
					 unsigned dstX,
					 unsigned dstY,
					 unsigned dstWidth,
					 unsigned dstHeight)
{
  unsigned line = 0;
//Y
  dstFrame += dstY * dstWidth;
  for (line = 0; line<srcHeight; line++) {
    if (dstY + line < dstHeight)
      memcpy (dstFrame + dstX, srcFrame, srcWidth);
    srcFrame += srcWidth;
    dstFrame += dstWidth;
  }
  dstFrame += (dstHeight - dstY - srcHeight)* dstWidth;

  dstY = dstY >> 1;
  dstX = dstX >> 1;
  srcWidth  = srcWidth >> 1;
  srcHeight = srcHeight >> 1;
  dstWidth  = dstWidth >> 1;
  dstHeight = dstHeight >> 1;

//U
  dstFrame += dstY * dstWidth;
  for (line = 0; line<srcHeight; line++) {
    if (dstY + line < dstHeight)
      memcpy (dstFrame + dstX, srcFrame , srcWidth);
    srcFrame += srcWidth;
    dstFrame += dstWidth;
  }
  dstFrame += (dstHeight - dstY - srcHeight)* dstWidth;

//V
  dstFrame += dstY * dstWidth;
  for (line = 0; line<srcHeight; line++) {
    if (dstY + line < dstHeight)
      memcpy (dstFrame + dstX, srcFrame , srcWidth);
    srcFrame += srcWidth;
    dstFrame += dstWidth;
  }
}

bool GMVideoInputManager_mlogo::has_device     (const std::string & /*source*/, const std::string & /*device_name*/, unsigned /*capabilities*/, Ekiga::VideoInputDevice & /*device*/)
{
  return false;
}

void
GMVideoInputManager_mlogo::device_opened_in_main (Ekiga::VideoInputDevice device,
						  Ekiga::VideoInputSettings settings)
{
  device_opened.emit (device, settings);
}

void
GMVideoInputManager_mlogo::device_closed_in_main (Ekiga::VideoInputDevice device)
{
  device_closed.emit (device);
}
