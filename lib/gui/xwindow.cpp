
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
 *                         XWindow.cpp  -  description
 *                         ----------------------------
 *   begin                : Fri Oct 26 2007
 *   copyright            : (C) 2007 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering direct X server drawing
 *
 */


#include "xwindow.h"

#include <ptlib/object.h>

extern "C" {
#include <pixops.h>
}

#ifdef HAVE_SHM
#include <sys/shm.h>
#include <sys/ipc.h>
#endif

#ifdef WORDS_BIGENDIAN
#define BO_NATIVE    MSBFirst
#define BO_NONNATIVE LSBFirst
#else
#define BO_NATIVE    LSBFirst
#define BO_NONNATIVE MSBFirst
#endif

struct xFormatsentry {
  const char* name;
  int depth;
  int planes;
  int byte_order;
  unsigned red_mask;
  unsigned green_mask;
  unsigned blue_mask;
} xFormats[] = {
  {"RGB24", 24, 3, MSBFirst,     0x00FF0000, 0x0000FF00, 0x000000FF}, //-RGB
  {"RGB24", 24, 3, LSBFirst,     0x000000FF, 0x0000FF00, 0x00FF0000}, //-BGR
  {"BGR24", 24, 3, MSBFirst,     0x000000FF, 0x0000FF00, 0x00FF0000}, //-BGR
  {"BGR24", 24, 3, LSBFirst,     0x00FF0000, 0x0000FF00, 0x000000FF}, //-RGB
  {"RGB32", 32, 4, BO_NATIVE,    0x000000FF, 0x0000FF00, 0x00FF0000}, //XBGR
  {"RGB32", 32, 4, BO_NONNATIVE, 0xFF000000, 0x00FF0000, 0x0000FF00}, //RGBX
  {"BGR32", 32, 4, BO_NATIVE,    0x00FF0000, 0x0000FF00, 0x000000FF}, //XRGB
  {"BGR32", 32, 4, BO_NONNATIVE, 0x0000FF00, 0x00FF0000, 0xFF000000}, //BGRX
  {"ARGB",   1, 0, MSBFirst,     0x00FF0000, 0x0000FF00, 0x000000FF},  //ARGB *
  {"BGR32",  1, 4, LSBFirst,     0x0000FF00, 0x00FF0000, 0xFF000000},  //ARGB
  {"ABGR",   1, 0, MSBFirst,     0x000000FF, 0x0000FF00, 0x00FF0000},  //ABGR *
  {"RGB32",  1, 4, LSBFirst,     0xFF000000, 0x00FF0000, 0x0000FF00},  //ABGR
  {"RGB32",  1, 4, MSBFirst,     0xFF000000, 0x00FF0000, 0x0000FF00},  //RGBA
  {"RGBA",   1, 0, LSBFirst,     0x000000FF, 0x0000FF00, 0x00FF0000},  //RGBA *
  {"BGR32",  1, 4, MSBFirst,     0x0000FF00, 0x00FF0000, 0xFF000000},  //BGRA
  {"BGRA",   1, 0, LSBFirst,     0x00FF0000, 0x0000FF00, 0x000000FF},  //BGRA *
  {"RGB16", 16, 2, LSBFirst,     0x0000F800, 0x000007E0, 0x0000001F},  //RGB16
  {NULL, 0, 0, 0, 0, 0, 0}
};

#ifdef HAVE_SHM
bool XWindow::_shmError = false;

static void catchXShmError(Display * , XErrorEvent * )
{
  XWindow::_shmError = true;
}
#endif

XWindow::XWindow()
{
  // initialize class variables
  _display = NULL;
  _rootWindow = 0;
  _XWindow = 0;
  _gc = NULL;
  _master = NULL;
  _slave = NULL;

  _imageWidth = 0;
  _imageHeight = 0;

  _useShm = false;
  _paintColorKey = false;
  _colorKey = 0;
  _wmType = 0;
  _isInitialized = false;
  _embedded = false;
  _state.fullscreen = false;
  _state.ontop = false;
  _state.decoration = true;
  _state.origLayer=0;
  _depth = 0;

  _XImage = NULL;
  _imageDataOrig = NULL;
  _outOffset = 0;
   snprintf (_colorFormat, sizeof(_colorFormat), "NONE");
  _planes = 0;
  _colorConverter = NULL;

#ifdef HAVE_SHM
  _XShmInfo.shmaddr = NULL;
#endif
 _scalingAlgorithm = NULL;
}


XWindow::~XWindow()
{
  XLockDisplay (_display);

#ifdef HAVE_SHM
    if (_useShm) {
      if (_isInitialized && _XShmInfo.shmaddr) {
        XShmDetach (_display, &_XShmInfo);
        shmdt (_XShmInfo.shmaddr);
      }
    } else
#endif
    {
      if (_XImage) {
        _XImage->data = _imageDataOrig;
      }
    }

  if (_XImage) {
    XDestroyImage(_XImage);
    _XImage = NULL;
  }

  _imageDataOrig = NULL;

  // shared data structures
  if (!_embedded && _gc)
    XFreeGC (_display, _gc);

  if (_XWindow) {

    PTRACE(4, "X11\tUnmapping and destroying Window with ID " << _XWindow);
    XUnmapWindow (_display, _XWindow);
    XDestroyWindow (_display, _XWindow);
    XFlush (_display);
  }

  XUnlockDisplay (_display);

  if (_colorConverter)
    delete (_colorConverter);
}


int 
XWindow::Init (Display* dp, 
                Window rootWindow, 
                GC gc, 
                int x, 
                int y,
                int windowWidth, 
                int windowHeight, 
                int imageWidth, 
                int imageHeight)
{
  _display = dp;
  _rootWindow = rootWindow;
  _imageWidth = imageWidth;
  _imageHeight = imageHeight;

  PTRACE(4, "X11\tInitiasing new X11 window with " << windowWidth << "x" << windowHeight << " at " << x << "," << y);
  XLockDisplay (_display);

#if PTRACING
  DumpVisuals();
#endif

  if (!CreateAtomsAndWindow(gc, x, y, windowWidth, windowHeight)) {
    XUnlockDisplay(_display);
    return 0;
  }
  
  CreateXImage(windowWidth, windowHeight);

  _isInitialized = true;
  XUnlockDisplay (_display);

  // check if that format is supported 
  struct xFormatsentry* xFormat = xFormats;
  while (xFormat->name) {
    if (xFormat->depth == _XImage->bits_per_pixel &&
        xFormat->byte_order == _XImage->byte_order &&
        xFormat->red_mask   == _XImage->red_mask   &&
        xFormat->green_mask == _XImage->green_mask &&
        xFormat->blue_mask  == _XImage->blue_mask)
      break;
    xFormat++;
  }
  PTRACE(4, "X11\tXImage created with format: " << _XImage->bits_per_pixel <<" BPP,  "
          << "Byte order: " << (_XImage->byte_order ? "MSBFirst" : "LSBFirst")
          << " Native: " << (BO_NATIVE ? "MSBFirst" : "LSBFirst"));
  PTRACE(4, std::hex << "X11\tMask: Red: 0x" <<  _XImage->red_mask
                      <<         " Green: 0x" << _XImage->green_mask 
                      <<          " Blue: 0x" << _XImage->blue_mask << std::dec);
  if (!xFormat->name) {
    PTRACE(1, "X11\tX server image format not supported, please contact the developers");
    return 0;
  }

  snprintf (_colorFormat, sizeof(_colorFormat), "%s", xFormat->name);
  _outOffset = 0;
  _planes = xFormat->planes;

#ifdef WORDS_BIGENDIAN
  if (strcmp (xFormat->name, "BGRA") == 0) {
    snprintf (_colorFormat, sizeof(_colorFormat), "RGB32");
    _outOffset = 1;
    _planes = 4;
  } 
  if (strcmp (xFormat->name, "RGBA") == 0) {
    snprintf (_colorFormat, sizeof(_colorFormat), "BGR32");
    _outOffset = 1;
    _planes = 4;
  } 
#else
  if (strcmp (xFormat->name, "ABGR") == 0) {
    snprintf (_colorFormat, sizeof(_colorFormat), "BGR32");
    _outOffset = -1;
    _planes = 4;
  } 
  if (strcmp (xFormat->name, "ARGB") == 0) {
    snprintf (_colorFormat, sizeof(_colorFormat), "RGB32");
    _outOffset = -1;
    _planes = 4;
  } 
#endif
  PTRACE(4, "X11\tUsing color format: " << _colorFormat);
  PTRACE(4, "X11\tPlanes: " << _planes);

  PVideoFrameInfo srcFrameInfo, dstFrameInfo;
  srcFrameInfo.SetFrameSize(_imageWidth,_imageHeight);
  dstFrameInfo.SetFrameSize(_imageWidth,_imageHeight);
  dstFrameInfo.SetColourFormat(_colorFormat);
  _colorConverter = PColourConverter::Create(srcFrameInfo, dstFrameInfo);
  if (!_colorConverter)
    return 0;

  _frameBuffer = std::tr1::shared_ptr<void> (malloc (_imageWidth * _imageHeight * _planes), free);

  // detect the window manager type
  _wmType = GetWMType ();
  CalculateSize (windowWidth, windowHeight, true);

  return 1;
}


void 
XWindow::PutFrame (uint8_t* frame, 
                    uint16_t width, 
                    uint16_t height)
{
  if (!_XImage) 
    return;

  if (width != _imageWidth || height != _imageHeight) {
    PTRACE (1, "X11\tDynamic switching of resolution not supported\n");
    return;
  }

  XLockDisplay (_display);


  if ((_state.curWidth != _XImage->width) || (_state.curHeight!=_XImage->height))
    CreateXImage(_state.curWidth, _state.curHeight);

  _colorConverter->Convert((BYTE*)frame, (BYTE*)_frameBuffer.get ());

  pixops_scale ((guchar*) _XImage->data,
                 0,0,
                 _state.curWidth, _state.curHeight,
                 _state.curWidth * _planes, //dest_rowstride
                 _planes,                   //dest_channels,
                 FALSE,                     //dest_has_alpha,

		(const guchar*) _frameBuffer.get (),
                 width,
                 height,
                 width * _planes,           //src_rowstride
                 _planes,                   //src_channels,
                 FALSE,                     //src_has_alpha,

                 (double) _state.curWidth / width,
                 (double) _state.curHeight / height,
                 (PixopsInterpType) _scalingAlgorithm);

       _XImage->data += _outOffset;
#ifdef HAVE_SHM
  if (_useShm)
  {
    XShmPutImage(_display, _XWindow, _gc, _XImage,
                 0, 0,
                 _state.curX, _state.curY,
                 _state.curWidth, _state.curHeight, false);
  } else
#endif
  {
    XPutImage(_display, _XWindow, _gc, _XImage,
              0, 0,
              _state.curX, _state.curY,
              _state.curWidth,_state.curHeight);
  }
  _XImage->data -= _outOffset;

  XUnlockDisplay (_display);
}

bool
XWindow::ProcessEvents()
{
  XEvent event;
  bool ret = false;

  XLockDisplay (_display);
  while (XCheckWindowEvent (_display, _XWindow, StructureNotifyMask 
                                              | SubstructureRedirectMask
					      | ExposureMask
					      | KeyPressMask
					      | ButtonPressMask, &event) == True) {

    switch (event.type) {
      case ClientMessage:
	      // If "closeWindow" is clicked do nothing right now 
	      // (window is closed from the GUI)
	      break;

      case ConfigureNotify:
              {
                // the window size has changed
                XConfigureEvent *xce = &(event.xconfigure);

                    // if a slave window exists it has to be resized as well
                if (_slave)
                  _slave->SetWindow (xce->width - (int) (xce->width / ( _state.fullscreen ? PIP_RATIO_FS : PIP_RATIO_WIN)),
                                     xce->height - (int) (_slave->GetYUVHeight () * xce->width / ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN) / _slave->GetYUVWidth ()),
                                     (int) (xce->width / ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN)),
                                     (int) (_slave->GetYUVHeight () * xce->width / ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN) / _slave->GetYUVWidth ()));

                CalculateSize (xce->width, xce->height, true);

                if( _paintColorKey ) {

                  XSetForeground( _display, _gc, _colorKey );
                  XFillRectangle( _display, _XWindow, _gc, _state.curX, _state.curY, _state.curWidth, _state.curHeight);
                }
              }
              break;

      case Expose:
              if (_paintColorKey) {
                XSetForeground( _display, _gc, _colorKey );
                XFillRectangle( _display, _XWindow, _gc, _state.curX, _state.curY, _state.curWidth, _state.curHeight);
              }
              ret = true;
              break;

      case KeyPress:
              // a key is pressed
              {
                XKeyEvent *xke = &(event.xkey);
                switch (xke->keycode) {
                  case 41:  
                    if (_master) 
                      _master->ToggleFullscreen (); 
                    else 
                      ToggleFullscreen (); // "f"
                    break;
                  case 40:  
                    if (_master) 
                      _master->ToggleDecoration (); 
                    else 
                      ToggleDecoration (); // "d"
                    break;
                  case 32:  
                    if (_master) 
                      _master->ToggleOntop (); 
                    else 
                      ToggleOntop ();      // "o"
                    break;
                  case 9:   
                    if (_master) { 
                      if (_master->IsFullScreen ()) 
                        _master->ToggleFullscreen(); 
                    } // esc
                    else { 
                      if (IsFullScreen ()) 
                        ToggleFullscreen(); 
                    }
                    break;
                  default:
                    break;
                }
              }
              break;

    case ButtonPress:
              // a mouse button is clicked

              if (_master)
                if (!_master->HasDecoration())
                  _master->ToggleDecoration();
                else
                  _master->ToggleFullscreen();
              else 
                if (!_state.decoration)
                  ToggleDecoration();
                else
                  ToggleFullscreen();
              break;

    case DestroyNotify:
              PTRACE(4, "X11\tWindow is being destroyed");
              break;

    default:
              PTRACE(1, "X11\tUnknown X Event " << event.type << " received");
    }
  }
  XUnlockDisplay (_display);

  return ret;
}


void
XWindow::Sync()
{
  XLockDisplay(_display);
  XSync (_display, False);
  XUnlockDisplay(_display);
}


void 
XWindow::ToggleFullscreen ()
{
  if (_embedded)
    return;

  Window childWindow;
  XWindowAttributes xwattributes;

  int newX = 0;
  int newY = 0;
  int newWidth = 0;
  int newHeight = 0;

  if (_state.fullscreen) {
    
    // not needed with EWMH fs
    if (!(_wmType & wm_FULLSCREEN)) {
      
      newX = _state.oldx;
      newY = _state.oldy;
      newWidth = _state.oldWidth;
      newHeight = _state.oldHeight;
      SetDecoration (true);
    }

    // removes fullscreen state if wm supports EWMH
    SetEWMHFullscreen (_NET_WM_STATE_REMOVE);
  } 
  else {

    // sets fullscreen state if wm supports EWMH
    SetEWMHFullscreen (_NET_WM_STATE_ADD);

    // not needed with EWMH fs - save window coordinates/size 
    // and discover fullscreen window size
    if (!(_wmType & wm_FULLSCREEN)) {

      XLockDisplay (_display);

      newX = 0;
      newY = 0;
      newWidth = DisplayWidth (_display, DefaultScreen (_display));
      newHeight = DisplayHeight (_display, DefaultScreen (_display));

      SetDecoration (false);
      XFlush (_display);

      XTranslateCoordinates (_display, _XWindow, RootWindow (_display, DefaultScreen (_display)), 
                             0, 0, &_state.oldx, &_state.oldy, &childWindow);

      XGetWindowAttributes (_display, _XWindow, &xwattributes);
      XUnlockDisplay (_display);
      
      _state.oldWidth = xwattributes.width;
      _state.oldHeight = xwattributes.height;
    }
  }
  
  // not needed with EWMH fs - create a screen-filling window on top 
  // and turn of decorations
  if (!(_wmType & wm_FULLSCREEN)) {

    SetSizeHints (newX, newY, _XImage->width, _XImage->height, newWidth, newHeight);

    XLockDisplay (_display);
    SetLayer (!_state.fullscreen ? 0 : 1);
    XMoveResizeWindow (_display, _XWindow, newX, newY, newWidth, newHeight);
    XUnlockDisplay (_display);
  }

  // some WMs lose ontop after fullscreeen
  if (_state.fullscreen & _state.ontop)
    SetLayer (1);

  XLockDisplay (_display);
  XMapRaised (_display, _XWindow);
  XRaiseWindow (_display, _XWindow);
  XSync (_display, False);
  XUnlockDisplay (_display);

  _state.fullscreen = !_state.fullscreen;
}


void 
XWindow::ToggleOntop ()
{
  if (_embedded)
    return;
  SetLayer (_state.ontop ? 0 : 1);
  _state.ontop = !_state.ontop;
}


void 
XWindow::ToggleDecoration ()
{
  if (_embedded)
    return;
  SetDecoration (!_state.decoration);
}


void 
XWindow::SetWindow (int x, 
                    int y, 
                    unsigned int windowWidth, 
                    unsigned int windowHeight)
{
  PTRACE(4, "X11\tSetWindow " << x << "," << y << " " << windowWidth << "x" << windowHeight);
  XLockDisplay (_display);
  XMoveResizeWindow (_display, _XWindow, x, y, windowWidth, windowHeight);
  XUnlockDisplay (_display);
  CalculateSize (windowWidth, windowHeight, true);
}


void 
XWindow::GetWindow (int *x, 
                     int *y, 
                     unsigned int *windowWidth, 
                     unsigned int *windowHeight)
{
  unsigned int ud = 0; 
  Window _dw;
  
  int oldx = 0; 
  int oldy = 0;
  
  Window root;
  bool decoration = false;
  
  decoration = _state.decoration;
  SetDecoration (false);

  XLockDisplay (_display);
  XSync (_display, False); 
  XGetGeometry (_display, _XWindow, &root, &oldx, &oldy, windowWidth, windowHeight, &ud, &ud);
  XTranslateCoordinates (_display, _XWindow, root, oldx, oldy, x, y, &_dw);
  XUnlockDisplay (_display);

  SetDecoration (decoration);
}



bool
XWindow::CreateAtomsAndWindow(GC gc,
                              int x, 
                              int y,
                              int windowWidth,
                              int windowHeight)
{
  XSetWindowAttributes xswattributes;

  // initialize atoms
  WM_DELETE_WINDOW = XInternAtom (_display, "WM_DELETE_WINDOW", False);
  XA_WIN_PROTOCOLS = XInternAtom (_display, "_WIN_PROTOCOLS", False);
  XA_NET_SUPPORTED = XInternAtom (_display, "_NET_SUPPORTED", False);
  XA_NET_WM_STATE = XInternAtom (_display, "_NET_WM_STATE", False);
  XA_NET_WM_STATE_FULLSCREEN = XInternAtom (_display, "_NET_WM_STATE_FULLSCREEN", False);
  XA_NET_WM_STATE_ABOVE = XInternAtom (_display, "_NET_WM_STATE_ABOVE", False);
  XA_NET_WM_STATE_STAYS_ON_TOP = XInternAtom (_display, "_NET_WM_STATE_STAYS_ON_TOP", False);
  XA_NET_WM_STATE_BELOW = XInternAtom (_display, "_NET_WM_STATE_BELOW", False);

  XSync (_display, False);

  if (!checkDepth()) 
    return false;

  // define window properties and create the window
  xswattributes.colormap = XCreateColormap (_display, _rootWindow, _XVInfo.visual, AllocNone);
  xswattributes.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask | ButtonPressMask;

  xswattributes.background_pixel = WhitePixel (_display, DefaultScreen (_display));

  xswattributes.border_pixel = WhitePixel (_display, DefaultScreen (_display));

  _XWindow = XCreateWindow (_display, _rootWindow, x, y, windowWidth, windowHeight, 
                             0, _XVInfo.depth, InputOutput, _XVInfo.visual, 
                             CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,  &xswattributes);

  PTRACE(4, "X11\tCreated Window with ID " << _XWindow);

  SetSizeHints (DEFAULT_X,DEFAULT_Y, _imageWidth, _imageHeight, windowWidth, windowHeight);
  
  // map the window
  XMapWindow (_display, _XWindow);

  XSetWMProtocols (_display, _XWindow, &WM_DELETE_WINDOW, 1);

  // Checking if we are going embedded or not
  if (gc) {
    _gc = gc; 
    _embedded = true;
  }
  else {
    _gc = XCreateGC (_display, _XWindow, 0, 0);
    _embedded = false;
  }
  return true;
}


void 
XWindow::SetLayer (int layer)
{
  char *state = NULL;

  Window mRootWin = RootWindow (_display, DefaultScreen (_display));
  XEvent xev;
  memset (&xev, 0, sizeof(xev));

  if (_wmType & wm_LAYER) {

    if (!_state.origLayer) 
      _state.origLayer = GetGnomeLayer ();
    
    xev.type = ClientMessage;
    xev.xclient.display = _display;
    xev.xclient.window = _XWindow;
    xev.xclient.message_type = XA_WIN_LAYER;
    xev.xclient.format = 32;
    xev.xclient.data.l [0] = layer ? WIN_LAYER_ABOVE_DOCK : _state.origLayer;
    xev.xclient.data.l [1] = CurrentTime;
    PTRACE(4, "X11\tLayered style stay on top (layer " << xev.xclient.data.l[0] << ")");
    
    XLockDisplay (_display);
    XSendEvent (_display, mRootWin, FALSE, SubstructureNotifyMask,  &xev);
    XUnlockDisplay (_display);

  } 
  else if (_wmType & wm_NETWM) {

    xev.type = ClientMessage;
    xev.xclient.message_type = XA_NET_WM_STATE;
    xev.xclient.display = _display;
    xev.xclient.window = _XWindow;
    xev.xclient.format = 32;
    xev.xclient.data.l [0] = layer;

    if (_wmType & wm_STAYS_ON_TOP) 
      xev.xclient.data.l [1] = XA_NET_WM_STATE_STAYS_ON_TOP;
    else 
      if (_wmType & wm_ABOVE) 
        xev.xclient.data.l [1] = XA_NET_WM_STATE_ABOVE;
    else 
      if (_wmType & wm_FULLSCREEN) 
        xev.xclient.data.l [1] = XA_NET_WM_STATE_FULLSCREEN;
    else 
      if (_wmType & wm_BELOW) 
        xev.xclient.data.l [1] = XA_NET_WM_STATE_BELOW;

    XLockDisplay (_display);
    XSendEvent (_display, mRootWin, FALSE, SubstructureRedirectMask, &xev);
    state = XGetAtomName (_display, xev.xclient.data.l [1]);
    PTRACE(4, "X11\tNET style stay on top (layer " << layer << "). Using state " << state );
    XFree (state);
    XUnlockDisplay (_display);
  }
}


void 
XWindow::SetEWMHFullscreen (int action)
{
  if (_wmType & wm_FULLSCREEN) {

    // create an event event to toggle fullscreen mode
    XEvent xev;
    
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.message_type = XInternAtom (_display, "_NET_WM_STATE", False);
    xev.xclient.window = _XWindow;
    xev.xclient.format = 32;
    
    xev.xclient.data.l[0] = action;
    xev.xclient.data.l[1] = XInternAtom (_display, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    // send the event to the window
    XLockDisplay (_display);
    if (!XSendEvent (_display, _rootWindow, FALSE, SubstructureRedirectMask | SubstructureNotifyMask, &xev)) {
      PTRACE(1, "X11\tSetEWMHFullscreen failed");
    }
    XUnlockDisplay (_display);
  }
}


void 
XWindow::SetDecoration (bool d)
{
  Atom motifHints;
  Atom mType;
  
  MotifWmHints setHints;
  MotifWmHints *getHints = NULL;
  
  unsigned char *args = NULL;

  int mFormat = 0;
  unsigned long mn = 0;
  unsigned long mb = 0;

  static unsigned int oldDecor = MWM_DECOR_ALL;
  static unsigned int oldFuncs = MWM_FUNC_MOVE | MWM_FUNC_CLOSE | MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;

  XLockDisplay (_display);

  motifHints = XInternAtom (_display, "_MOTIF_WM_HINTS", 0);

  if (motifHints != None) {

    memset (&setHints, 0, sizeof (setHints));

    if (!d) {

      XGetWindowProperty (_display, _XWindow, motifHints, 0, 20, False, motifHints, &mType, &mFormat, &mn, &mb, &args);
      getHints = (MotifWmHints*) args;

      if (getHints) {

        if (getHints->flags & MWM_HINTS_DECORATIONS) 
          oldDecor = getHints->decorations;
        
        if (getHints->flags & MWM_HINTS_FUNCTIONS) 
          oldFuncs = getHints->functions;
        
        XFree(getHints);
      }

      
      setHints.decorations = 0;
    } 
    else {
    
      setHints.functions = oldFuncs;
      setHints.decorations = oldDecor;
    }

    setHints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    
    XChangeProperty (_display, _XWindow, motifHints, motifHints, 32, PropModeReplace, (unsigned char *) &setHints, 5);
    
    _state.decoration=!_state.decoration;
  }

  XUnlockDisplay (_display);
}


int 
XWindow::GetWMType ()
{
  Atom *args = NULL;

  unsigned int i = 0;
  int wmType = 0;
  int metacityHack = 0;
  unsigned long nitems = 0;

  // check if WM supports layers
  if (GetWindowProperty (XA_WIN_PROTOCOLS, &args, &nitems)) {
    
    PTRACE(4, "X11\tDetected WM supports layers");
    for (i = 0; i < nitems; i++) {
      
      if (args [i] == XA_WIN_LAYER) {
        wmType |= wm_LAYER;
        metacityHack |= 1;
      } 
      else 
        metacityHack |= 2;
    }

    XLockDisplay (_display);
    XFree (args);
    XUnlockDisplay (_display);

    // metacity WM reports that it supports layers, 
    // but it is not really true :-)
    if (wmType && metacityHack == 1) {
      wmType ^= wm_LAYER;
      PTRACE(4, "X11\tUsing workaround for Metacity bug");
    }
  }

  // NETWM
  if (GetWindowProperty (XA_NET_SUPPORTED, &args, &nitems)) {
    
    PTRACE(4, "X11\tDetected wm supports NetWM.");

    for (i = 0; i < nitems; i++) 
      wmType |= GetSupportedState (args[i]);
    
    XLockDisplay (_display);
    XFree (args);
    XUnlockDisplay (_display);
  }

  // unknown WM
  if (wmType == 0) {
    PTRACE(4, "X11\tUnknown wm type...");
  }
  
  return wmType;
}


int 
XWindow::GetGnomeLayer ()
{
  Atom type;

  int format = 0;
  unsigned long count = 0;
  unsigned long bytesafter = 0;
  unsigned char *prop = NULL;

  long layer = WIN_LAYER_NORMAL;

  XLockDisplay (_display);
  if (XGetWindowProperty (_display, _XWindow,XA_WIN_LAYER, 0, 16384, false, XA_CARDINAL, &type, &format, &count, &bytesafter, &prop) 
      == Success && prop) {
    
    if (type == XA_CARDINAL && format == 32 && count == 1) 
      layer = ((long *) prop) [0];

    XFree(prop);
  }
  XUnlockDisplay(_display);

  return layer;
}


int 
XWindow::GetSupportedState (Atom atom)
{
  if (atom == XA_NET_WM_STATE_FULLSCREEN)
    return wm_FULLSCREEN;

  if (atom == XA_NET_WM_STATE_ABOVE)
    return wm_ABOVE;

  if (atom == XA_NET_WM_STATE_STAYS_ON_TOP)
    return wm_STAYS_ON_TOP;

  if (atom==XA_NET_WM_STATE_BELOW)
    return wm_BELOW;

  return 0;
}


int 
XWindow::GetWindowProperty (Atom type, 
                             Atom **args, 
                             unsigned long *nitems)
{
  int format = 0;
  unsigned long bytesafter = 0;
  int ret = 0;

  XLockDisplay(_display);
  ret = (Success == XGetWindowProperty (_display, _rootWindow, type, 0, 16384, false,
         AnyPropertyType, &type, &format, nitems, &bytesafter, (unsigned char **) args) && *nitems > 0);
  XUnlockDisplay(_display);

  return ret; 
}


void 
XWindow::CalculateSize (int windowWidth,
                        int windowHeight,
                        bool doAspectCorrection) 
{
  if ( doAspectCorrection && ((windowWidth * _imageHeight / _imageWidth) > windowHeight)) {

    _state.curX = (int) ((windowWidth - (windowHeight * _imageWidth / _imageHeight)) / 2);
    _state.curY = 0;
    _state.curWidth = (int) (windowHeight * _imageWidth / _imageHeight);
    _state.curHeight = windowHeight;

  } 
  else if ( doAspectCorrection && ((windowHeight * _imageWidth / _imageHeight) > windowWidth)) {

    _state.curX = 0;
    _state.curY = (int) ((windowHeight - (windowWidth * _imageHeight / _imageWidth)) / 2);
    _state.curWidth = windowWidth;
    _state.curHeight = (int) (windowWidth * _imageHeight / _imageWidth);
  } 
  else {

    _state.curX = 0;
    _state.curY = 0;
    _state.curWidth = windowWidth;
    _state.curHeight = windowHeight;
  }
}


void 
XWindow::SetSizeHints (int x, 
                        int y, 
                        int imageWidth, 
                        int imageHeight, 
                        int windowWidth, 
                        int windowHeight)
{
  XSizeHints xshints;

  xshints.flags = PPosition | PSize | PAspect | PMinSize | PMaxSize;

  xshints.min_aspect.x = imageWidth;
  xshints.min_aspect.y = imageHeight;
  xshints.max_aspect.x = imageWidth;
  xshints.max_aspect.y = imageHeight;

  xshints.x = x;
  xshints.y = y;
  xshints.width = windowWidth;
  xshints.height = windowHeight;
  xshints.min_width = windowWidth;
  xshints.min_height = windowHeight;
  xshints.max_width = windowWidth;
  xshints.max_height = windowHeight;
  
  XSetStandardProperties (_display, _XWindow, "Video", "Video", None, NULL, 0, &xshints);
}


bool XWindow::checkDepth ()
{

  XWindowAttributes xwattributes;
  XGetWindowAttributes (_display, _rootWindow, &xwattributes);
  if (xwattributes.depth == 32) {
    _depth = 32;
    if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
      PTRACE(4, "X11\tCould not find visual with colordepth of " << _depth  << " bits per pixel");
      _depth = 24;
      if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
        PTRACE(1, "X11\tCould neither find visual with colordepth of 32 bits per pixel nor with 24 bits per pixel");
        return false;
      }
    }
  }
  else if (xwattributes.depth == 16) {
    _depth = 16;
    if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
      PTRACE(4, "X11\tCould not find visual with colordepth of " << _depth  << " bits per pixel");
      _depth = 24;
      if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
        PTRACE(1, "X11\tCould neither find visual with colordepth of 16 bits per pixel nor with 24 bits per pixel");
        return false;
      }
    }
  }
  else {
    _depth = 24;
    if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
      PTRACE(4, "X11\tCould not find visual with colordepth of " << _depth  << " bits per pixel");
      _depth = 32;
      if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
        PTRACE(1, "X11\tCould neither find visual with colordepth of 24 bits per pixel nor with 32 bits per pixel");
        return false;
      }
    }
  }
  return true;
}


#ifdef HAVE_SHM
void XWindow::ShmAttach(int imageWidth, int imageHeight)
{
  if (_useShm) {
    _XImage = XShmCreateImage(_display, _XVInfo.visual, _depth, ZPixmap, NULL, &_XShmInfo, imageWidth, imageHeight);
    if (_XImage == NULL) {
      PTRACE(1, "X11\tXShmCreateImage failed");
      _useShm = FALSE;
    }
  }

  if (_useShm) {
    _XShmInfo.shmid = shmget(IPC_PRIVATE, _XImage->bytes_per_line * _XImage->height, IPC_CREAT | 0777);
    if (_XShmInfo.shmid < 0) {
       XDestroyImage(_XImage);
       _XImage = NULL;
       PTRACE(1, "X11\tshmget failed"); //strerror(errno)
       _useShm = FALSE;
    }
  }

  if (_useShm) {
    _XShmInfo.shmaddr = (char *) shmat(_XShmInfo.shmid, 0, 0);
    if (_XShmInfo.shmaddr == ((char *) -1)) { 
      XDestroyImage(_XImage);
      _XImage = NULL;
      PTRACE(1, "X11\tshmat failed"); //strerror(errno)
      _useShm = FALSE;
    }
  }

  if (_useShm) {
    _XImage->data = _XShmInfo.shmaddr;
    _XShmInfo.readOnly = False;

    // Attaching the shared memory to the display
    XErrorHandler oldHandler = XSetErrorHandler((XErrorHandler) catchXShmError);
    Status status = XShmAttach (_display, &_XShmInfo);
    XSync(_display, False);
    XSetErrorHandler((XErrorHandler) oldHandler);

    if ( (status != True) || (_shmError) ) {
      XDestroyImage(_XImage);
      _XImage = NULL;
      if (_XShmInfo.shmaddr != ((char *) -1)) {
        shmdt(_XShmInfo.shmaddr);
      }
      PTRACE(1, "X11\t  XShmAttach failed");
      if ( (status == True) && (_shmError) ) {
        PTRACE(1, "X11\t  X server supports SHM but apparently we are remotely connected...");
      }
      _useShm = false;
    } 
  } 

  if (_useShm) {
    shmctl(_XShmInfo.shmid, IPC_RMID, 0);
  }
}
#endif


void XWindow::CreateXImage(int width, int height)
{
#ifdef HAVE_SHM
    if (_useShm) {
      if (_isInitialized && _XShmInfo.shmaddr) {
        XShmDetach (_display, &_XShmInfo);
        shmdt (_XShmInfo.shmaddr);
      }
    } else
#endif
    {
      if (_XImage) {
        _XImage->data = _imageDataOrig;
      }
    }

  if (_XImage)
    XDestroyImage(_XImage);

  _imageDataOrig = NULL;

#ifdef HAVE_SHM
   if (XShmQueryExtension (_display)) {
     _useShm = true;
     PTRACE(1, "X11\tXQueryShmExtension success");
   }
   else {
     _useShm = false;
     PTRACE(1, "X11\tXQueryShmExtension failed");
   }

  if (_useShm)
     ShmAttach(width, height);

  if (_useShm) {
     PTRACE(4, "X11\tUsing shm extension");
  }
  else
#endif
  {
        _XImage = XCreateImage(_display, _XVInfo.visual, _depth, ZPixmap, 0, NULL,  width, height, 8, 0);
         _imageDataOrig = (char*)malloc(width  * height * 4 + 32);
         _XImage->data = _imageDataOrig + 16 - ((long)_imageDataOrig & 15);
         memset(_XImage->data, 0, width * 4 * height);
  }
}


void
XWindow::DumpVisuals()
{
    XVisualInfo visualTemplate;
    XVisualInfo *visuals;
    int nbVisuals = 0;
    int i = 0;

    visualTemplate.screen = DefaultScreen(_display);
    visuals = XGetVisualInfo(_display, VisualScreenMask , &visualTemplate, &nbVisuals);
    if (visuals != NULL) {
        for (i = 0; i < nbVisuals; i++) {
            PTRACE(4, "X11\tVisual #"  << i << " ID: " << visuals[i].visualid
                       << " Class: "   << visuals[i].c_class
                       << " BPRGB: "     << visuals[i].bits_per_rgb
                       << " Depth: "   << visuals[i].depth << std::hex
                       << " Red: 0x"   << visuals[i].red_mask
                       << " Green: 0x" << visuals[i].green_mask
                       << " Blue 0x"   << visuals[i].blue_mask << std::dec);
        }
        XFree(visuals);
    }
}
