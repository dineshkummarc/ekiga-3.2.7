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
 *                         XWindow.h  -  description
 *                         --------------------------
 *   begin                : Fri Oct 26 2007
 *   copyright            : (C) 2007 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class unaccelerated X Video output 
 */


#ifndef XWINDOW_H
#define XWINDOW_H

#include <stdint.h>
#include "config.h"

#include <tr1/memory>

#include <ptlib.h>
#include <ptlib/vconvert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#ifdef HAVE_SHM
#include <X11/extensions/XShm.h>
#endif


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
class XWindow
{
public:

  XWindow();

  virtual ~XWindow();
  
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

  virtual bool ProcessEvents();
  
  virtual void Sync();

  virtual void ToggleFullscreen ();
 
  virtual void ToggleOntop ();
  
  virtual void ToggleDecoration ();

  virtual void SetWindow (int x, 
                  int y, 
                  unsigned int windowWidth, 
                  unsigned int windowHeight);
  
  virtual void GetWindow (int *x, 
                  int *y, 
                  unsigned int *windowWidth, 
                  unsigned int *windowHeight);

  virtual bool IsFullScreen () const { return _state.fullscreen; };
 
  virtual bool HasDecoration () const { return _state.decoration; };

  virtual bool IsOntop () const { return _state.ontop; };

  virtual Window GetWindowHandle () const { return _XWindow; };

  virtual Display* GetDisplay () const { return _display; };

  virtual GC GetGC () const { return _gc; };

  virtual int GetYUVWidth () const { return _imageWidth; };

  virtual int GetYUVHeight() const { return _imageHeight; };

  virtual void RegisterMaster (XWindow *master) { _master = master; };

  virtual void RegisterSlave (XWindow *slave) { _slave = slave; };

  virtual void SetSwScalingAlgo (unsigned int algorithm) { _scalingAlgorithm = algorithm; };

#ifdef HAVE_SHM
  static bool _shmError;
#endif
protected:
  Display *_display;
  Window _rootWindow;
  Window _XWindow;
  GC _gc;
  int _depth;
  XWindow * _master;
  XWindow * _slave;
  XVisualInfo _XVInfo;

  int _imageWidth;
  int _imageHeight;

  bool _useShm;
  bool _paintColorKey;
  int _colorKey;
  int _wmType;
  bool _isInitialized;
  bool _embedded;

  typedef struct 
  {
    bool fullscreen;
    bool ontop;
    bool decoration;
    int oldx;
    int oldy;
    int oldWidth;
    int oldHeight;
    int curX;
    int curY;
    int curWidth;
    int curHeight;
    int origLayer;
  } State;

  State _state;

  unsigned int _scalingAlgorithm;

  Atom XA_NET_SUPPORTED;
  Atom XA_WIN_PROTOCOLS;
  Atom XA_WIN_LAYER;
  Atom XA_NET_WM_STATE;
  Atom XA_NET_WM_STATE_FULLSCREEN;
  Atom XA_NET_WM_STATE_ABOVE;
  Atom XA_NET_WM_STATE_STAYS_ON_TOP;
  Atom XA_NET_WM_STATE_BELOW;
  Atom WM_DELETE_WINDOW;

  virtual bool CreateAtomsAndWindow(GC gc,
                                    int x, 
                                    int y,
                                    int windowWidth,
                                    int windowHeight);

  /**
   * Sets the layer for the window.
   */
  virtual void SetLayer (int layer);

  /**
   * Fullscreen for ewmh WMs.
   */
  virtual void SetEWMHFullscreen (int action);

  /**
   * Toggle window decoration on and off
   */
  virtual void SetDecoration (bool d);

  /**
   * Detects window manager type.
   */
  virtual int GetWMType ();

  virtual int GetGnomeLayer ();

  /**
   * Tests an atom.
   */
  virtual int GetSupportedState (Atom atom);

  /**
   * Returns the root window's.
   */
  virtual int GetWindowProperty (Atom type, 
                         Atom **args, 
                         unsigned long *nitems);

  virtual void CalculateSize (int width, 
                              int height, 
                              bool doAspectCorrection);

  virtual void SetSizeHints (int x,  
                     int y, 
                     int imageWidth, 
                     int imageHeight, 
                     int windowWidth, 
                     int windowHeight);
private:

  XImage * _XImage;
  char* _imageDataOrig;
  int _outOffset;
  char _colorFormat[6];
  int _planes;

  PColourConverter* _colorConverter;
  std::tr1::shared_ptr<void> _frameBuffer;
  
#ifdef HAVE_SHM
  XShmSegmentInfo _XShmInfo;
#endif

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

  virtual void CreateXImage(int width, int height);

  virtual void DumpVisuals();
};

#define wm_LAYER         1
#define wm_FULLSCREEN    2
#define wm_STAYS_ON_TOP  4
#define wm_ABOVE         8
#define wm_BELOW         16
#define wm_NETWM (wm_FULLSCREEN | wm_STAYS_ON_TOP | wm_ABOVE | wm_BELOW)

#define WIN_LAYER_ONBOTTOM    2
#define WIN_LAYER_NORMAL      4
#define WIN_LAYER_ONTOP       6
#define WIN_LAYER_ABOVE_DOCK 10

#define _NET_WM_STATE_REMOVE  0  /* remove/unset property */
#define _NET_WM_STATE_ADD     1  /* add/set property */
#define _NET_WM_STATE_TOGGLE  2  /* toggle property */

#define MWM_HINTS_FUNCTIONS   (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_FUNC_RESIZE       (1L << 1)
#define MWM_FUNC_MOVE         (1L << 2)
#define MWM_FUNC_MINIMIZE     (1L << 3)
#define MWM_FUNC_MAXIMIZE     (1L << 4)
#define MWM_FUNC_CLOSE        (1L << 5)
#define MWM_DECOR_ALL         (1L << 0)
#define MWM_DECOR_MENU        (1L << 4)

#define PIP_RATIO_WIN  3
#define PIP_RATIO_FS   5
#define DEFAULT_X 1
#define DEFAULT_Y 1

typedef struct
{
  int flags;
  long functions;
  long decorations;
  long input_mode;
  long state;
} MotifWmHints;

#endif //XWINDOW_H
