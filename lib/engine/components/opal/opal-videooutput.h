
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         videooutput.h  -  description
 *                         ----------------------------
 *   begin                : Sat Feb 17 2001
 *   copyright            : (C) 2000-2007 by Damien Sandras
 *   description          : PVideoOutputDevice class to permit to display via 
 *                          GMVideoDisplay class
 *
 */


#ifndef _EKIGA_VIDEO_OUTPUT_H_
#define _EKIGA_VIDEO_OUTPUT_H_

#include "videooutput-core.h"

class GMManager;

class PVideoOutputDevice_EKIGA : public PVideoOutputDevice
{
  PCLASSINFO(PVideoOutputDevice_EKIGA, PVideoOutputDevice);

  public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  PVideoOutputDevice_EKIGA (Ekiga::ServiceCore & core);


  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~PVideoOutputDevice_EKIGA ();

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Open the device given the device name.
   * PRE          :  Device name to open, immediately start device.
   */
  virtual bool Open (const PString &name,
                     bool unused);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Return a list of all of the drivers available.
   * PRE          :  /
   */
  virtual PStringArray GetDeviceNames() const;


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Get the maximum frame size in bytes.
   * PRE          :  /
   */
  virtual PINDEX GetMaxFrameBytes() { return 352 * 288 * 3 * 2; }

  
  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the output device is open.
   * PRE          :  /
   */
  virtual bool IsOpen ();

  /* DESCRIPTION  :  /
   * BEHAVIOR     :  If data for the end frame is received, then we convert
   *                 it to the correct colour format and we display it.
   * PRE          :  x and y positions in the picture (>=0),
   *                 width and height (>=0),
   *                 the data, and a boolean indicating if it is the end
   *                 frame or not.
   */
  virtual bool SetFrameData (unsigned x,
                             unsigned y,
                             unsigned width,
                             unsigned height,
                             const BYTE *data,
                             bool endFrame);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Returns TRUE if the colour format is supported (ie RGB24).
   * PRE          :  /
   */
  virtual bool SetColourFormat (const PString &colour_format);


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Start displaying.
   * PRE          :  /
   */
  virtual bool Start () { return TRUE; };


  /* DESCRIPTION  :  /
   * BEHAVIOR     :  Stop displaying.
   * PRE          :  /
   */
  virtual bool Stop () { return TRUE; };

 protected:

  static int devices_nbr; /* The number of devices opened */
  int device_id;          /* The current device : local or remote */

  static PMutex videoDisplay_mutex;  

  bool is_active;

  enum {REMOTE, LOCAL};

  Ekiga::ServiceCore & core;
  Ekiga::VideoOutputCore* videooutput_core;
};

#endif
