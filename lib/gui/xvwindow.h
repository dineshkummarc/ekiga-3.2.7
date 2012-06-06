
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
 *                         XVWindow.h  -  description
 *                         --------------------------
 *   begin                : Sun May 7 2006
 *   copyright            : (C) 2006 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering X-Video hardware 
 *                          acceleration.
 */


#ifndef XVWINDOW_H
#define XVWINDOW_H

#include <stdint.h>
#include <set>

#include "config.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#ifdef HAVE_SHM
#include <X11/extensions/XShm.h>
#endif
#include <X11/extensions/Xvlib.h>

#include "xwindow.h"

#define NUM_BUFFERS 1

/**
 * String: wrapper/helper.
 *
 * This class provides XVideo support under Linux if it is supported by the graphics hardware and driver.
 * XVideo makes use of hardware capabilities in order to do
 * - colorspace transformation
 * - scaling
 * - anti-aliasing
 *
 * This class features a fullscreen mode, an always-on-top mode and allows to enable and disable the window
 * manager decorations. A picture-in-picture functionality is provided by registering a second XVWindow class
 * window as a slave window. This class should work with most if not all window managers. It has to initialized
 * with the display and window where it shall appear and the original image and intial window size
 * After having been initialized successfully a frame is passed via PutFrame which takes care of the presentation.
 *
 * @author Matthias Schneider
 */
class XVWindow : public XWindow
{
public:

  XVWindow();

  virtual ~XVWindow();
  
  virtual int Init (Display *dp, 
            Window xvWindow, 
            GC gc, 
            int x, 
            int y, 
            int windowWidth, 
            int windowHeight, 
            int imageWidth, 
            int imageHeight);

  virtual void PutFrame (uint8_t *frame, 
                 uint16_t width, 
                 uint16_t height);

  virtual void Sync();

private:
  unsigned int _XVPort;
  XvImage * _XVImage[NUM_BUFFERS];
#ifdef HAVE_SHM
  XShmSegmentInfo _XShmInfo[NUM_BUFFERS];
#endif
  unsigned int _curBuffer;

  Atom XV_SYNC_TO_VBLANK;
  Atom XV_COLORKEY;
  Atom XV_AUTOPAINT_COLORKEY;

  virtual void SetSizeHints (int x, //different
                             int y, 
                             int imageWidth, 
                             int imageHeight, 
                             int windowWidth, 
                             int windowHeight);

  virtual unsigned int FindXVPort ();

  virtual void DumpCapabilities (int port);

  /**
   * Check if an atom exists and return it
   */
  virtual Atom GetXVAtom( char const * name );

  /**
   * Check and initialize colorkeying
   */
  virtual bool InitColorkey();

  /**
   * Check if image size is supported by XV
   */
  virtual bool checkMaxSize(unsigned int width, unsigned int height);

  /**
   * Verify pixel depth
   */
  virtual bool checkDepth ();

  /**
   * Attach to Shared Memory
   */
#ifdef HAVE_SHM  
  virtual void ShmAttach(int imageWidth, int imageHeight);
#endif

  static std::set <XvPortID> grabbedPorts;
};

#endif //XVWINDOW_H
