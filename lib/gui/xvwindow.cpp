
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
 *                         XVWindow.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun May 7 2006
 *   copyright            : (C) 2006 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering X-Video hardware 
 *                          acceleration.
 */


#include "xvwindow.h"

#include <ptlib/object.h>

#define GUID_I420_PLANAR 0x30323449
#define GUID_YV12_PLANAR 0x32315659

#ifdef HAVE_SHM
#include <sys/shm.h>
#include <sys/ipc.h>

static void catchXShmError(Display * , XErrorEvent * )
{
  XWindow::_shmError = true;
}
#endif

std::set <XvPortID> XVWindow::grabbedPorts;

XVWindow::XVWindow()
{
  // initialize class variables

  _XVPort = 0;
  unsigned int i = 0;
   for (i = 0; i < NUM_BUFFERS; i++) {
     _XVImage[i] = NULL;
#ifdef HAVE_SHM
     _XShmInfo[i].shmaddr = NULL;
#endif
   }
   _curBuffer = 0;
}


XVWindow::~XVWindow()
{
  unsigned int i = 0; 

  XLockDisplay (_display);
#ifdef HAVE_SHM
    if (_useShm) {
      for (i = 0; i < NUM_BUFFERS; i++)
        if (_isInitialized && _XShmInfo[i].shmaddr) {
          XShmDetach (_display, &_XShmInfo[i]);
          shmdt (_XShmInfo[i].shmaddr);
        }
    } else
#endif
    {
      for (i = 0; i < NUM_BUFFERS; i++)
        if ((_XVImage[i]) && (_XVImage[i]->data)) {
           free (_XVImage[i]->data);
           _XVImage[i]->data = NULL;
        }
    }

  for (i = 0; i < NUM_BUFFERS; i++)
    if (_XVImage[i]) {
      XFree (_XVImage[i]);
     _XVImage[i] = NULL;
    }

  if (_XVPort) {

    XvUngrabPort (_display, _XVPort, CurrentTime);
    grabbedPorts.erase(_XVPort);
    _XVPort = 0;
  }

  XUnlockDisplay(_display);
}


int 
XVWindow::Init (Display* dp, 
                Window rootWindow, 
                GC gc, 
                int x, 
                int y,
                int windowWidth, 
                int windowHeight, 
                int imageWidth, 
                int imageHeight)
{
  // local variables needed for creation of window 
  // and initialization of XV extension
  unsigned int ver = 0;
  unsigned int rel = 0;
  unsigned int req = 0;
  unsigned int ev = 0;
  unsigned int err = 0;
  int ret = 0;
  unsigned int i = 0;

  _display = dp;
  _rootWindow = rootWindow;
  _imageWidth = imageWidth;
  _imageHeight = imageHeight;

  PTRACE(4, "XVideo\tInitializing XV window with " << windowWidth << "x" << windowHeight << " at " << x << "," << y);
  XLockDisplay (_display);


  // check if SHM XV window is possible
  ret = XvQueryExtension (_display, &ver, &rel, &req, &ev, &err);
  PTRACE(4, "XVideo\tXvQueryExtension: Version: " << ver << " Release: " << rel 
         << " Request Base: " << req << " Event Base: " << ev << " Error Base: " << err  );

  if (Success != ret) {
    if (ret == XvBadExtension)
      PTRACE(1, "XVideo\tXvQueryExtension failed - XvBadExtension");
    else if (ret == XvBadAlloc)
      PTRACE(1, "XVideo\tXvQueryExtension failed - XvBadAlloc");
    else
      PTRACE(1, "XVideo\tXQueryExtension failed");
    XUnlockDisplay (_display);
    return 0;
  }
  
  // Find XV port
  _XVPort = FindXVPort ();
  if (!_XVPort) {
    PTRACE(1, "XVideo\tFindXVPort failed");
    XUnlockDisplay(_display);
    return 0;
  } 
  PTRACE(4, "XVideo\tUsing XVideo port: " << _XVPort);

  if (!CreateAtomsAndWindow(gc, x, y, windowWidth, windowHeight)) {
    XUnlockDisplay(_display);
    return 0;
  }

  XV_SYNC_TO_VBLANK = GetXVAtom("XV_SYNC_TO_VBLANK");
  XV_COLORKEY = GetXVAtom( "XV_COLORKEY" );
  XV_AUTOPAINT_COLORKEY = GetXVAtom( "XV_AUTOPAINT_COLORKEY" );    

  if ( !InitColorkey() )
  {
    PTRACE(1, "XVideo\tColorkey initialization failed");
    XUnlockDisplay(_display);
    return 0; 
  } 

  if (XV_SYNC_TO_VBLANK != None)
    if (XvSetPortAttribute(_display, _XVPort, XV_SYNC_TO_VBLANK, 1) == Success)
      PTRACE(4, "XVideo\tVertical sync successfully activated" );
     else
      PTRACE(4, "XVideo\tFailure when trying to activate vertical sync" );
  else
    PTRACE(4, "XVideo\tVertical sync not supported");

  if (!checkMaxSize (imageWidth, imageHeight)) {
    PTRACE(1, "XVideo\tCheck of image size failed");
    XUnlockDisplay(_display);
    return 0; 
  }

#ifdef HAVE_SHM
   if (XShmQueryExtension (_display)) {
     _useShm = true;
     PTRACE(1, "XVideo\tXQueryShmExtension success");
   }
   else {
     _useShm = false;
     PTRACE(1, "XVideo\tXQueryShmExtension failed");
   }

  if (_useShm)
    ShmAttach(imageWidth, imageHeight);

  if (!_useShm) {
#endif
  for (i = 0; i < NUM_BUFFERS; i++) {

    _XVImage[i] = (XvImage *) XvCreateImage( _display, _XVPort, GUID_YV12_PLANAR, 0, imageWidth, imageHeight);

    if (!_XVImage[i]) {
      PTRACE(1, "XVideo\tUnable to create XVideo Image");
      XUnlockDisplay (_display);
      return 0;
    }

    _XVImage[i]->data = (char*) malloc(_XVImage[i]->data_size);
  }

    PTRACE(1, "XVideo\tNot using SHM extension");
#ifdef HAVE_SHM
  }
  else {
      PTRACE(1, "XVideo\tUsing SHM extension");
  }
#endif

  XSync(_display, False);

  _isInitialized = true;
  XUnlockDisplay (_display);

  // detect the window manager type
  _wmType = GetWMType ();
  CalculateSize (windowWidth, windowHeight, true);

  return 1;
}


void 
XVWindow::PutFrame (uint8_t* frame, 
                    uint16_t width, 
                    uint16_t height)
{
  if (!_XVImage[_curBuffer]) 
    return;
  
  if (width != _XVImage[_curBuffer]->width || height != _XVImage[_curBuffer]->height) {
     PTRACE (1, "XVideo\tDynamic switching of resolution not supported\n");
     return;
  }

  XLockDisplay (_display);

  if (_XVImage[_curBuffer]->pitches [0] ==_XVImage[_curBuffer]->width
      && _XVImage[_curBuffer]->pitches [2] == (int) (_XVImage[_curBuffer]->width / 2) 
      && _XVImage[_curBuffer]->pitches [1] == (int) (_XVImage[_curBuffer]->width / 2)) {
  
    memcpy (_XVImage[_curBuffer]->data, 
            frame, 
            (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height));
    memcpy (_XVImage[_curBuffer]->data + (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height), 
            frame + _XVImage[_curBuffer]->offsets [2], 
            (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height / 4));
    memcpy (_XVImage[_curBuffer]->data + (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height * 5 / 4), 
            frame + _XVImage[_curBuffer]->offsets [1], 
            (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height / 4));
  } 
  else {
  
    unsigned int i = 0;
    int width2 = (int) (_XVImage[_curBuffer]->width / 2);

    uint8_t* dstY = (uint8_t*) _XVImage[_curBuffer]->data;
    uint8_t* dstV = (uint8_t*) _XVImage[_curBuffer]->data + (_XVImage[_curBuffer]->pitches [0] * _XVImage[_curBuffer]->height);
    uint8_t* dstU = (uint8_t*) _XVImage[_curBuffer]->data + (_XVImage[_curBuffer]->pitches [0] * _XVImage[_curBuffer]->height) 
                                              + (_XVImage[_curBuffer]->pitches [1] * (_XVImage[_curBuffer]->height/2));

    uint8_t* srcY = frame;
    uint8_t* srcV = frame + (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height * 5 / 4);
    uint8_t* srcU = frame + (int) (_XVImage[_curBuffer]->width * _XVImage[_curBuffer]->height);

    for (i = 0 ; i < (unsigned int)_XVImage[_curBuffer]->height ; i+=2) {

      memcpy (dstY, srcY, _XVImage[_curBuffer]->width); 
      dstY +=_XVImage[_curBuffer]->pitches [0]; 
      srcY +=_XVImage[_curBuffer]->width;
      
      memcpy (dstY, srcY, _XVImage[_curBuffer]->width); 
      dstY +=_XVImage[_curBuffer]->pitches [0]; 
      srcY +=_XVImage[_curBuffer]->width;
      
      memcpy (dstV, srcV, width2); 
      dstV +=_XVImage[_curBuffer]->pitches [1]; 
      srcV += width2;
      
      memcpy(dstU, srcU, width2); dstU+=_XVImage[_curBuffer]->pitches [2]; 
      srcU += width2;
    }
  }
#ifdef HAVE_SHM
  if (_useShm) 
  {
    XvShmPutImage (_display, _XVPort, _XWindow, _gc, _XVImage[_curBuffer], 
                  0, 0, _XVImage[_curBuffer]->width, _XVImage[_curBuffer]->height, 
                  _state.curX, _state.curY, _state.curWidth, _state.curHeight, false);
  }
  else
#endif
  {
    XvPutImage (_display, _XVPort, _XWindow, _gc, _XVImage[_curBuffer], 
                  0, 0, _XVImage[_curBuffer]->width, _XVImage[_curBuffer]->height, 
                  _state.curX, _state.curY, _state.curWidth, _state.curHeight);
  }

  _curBuffer = (_curBuffer + 1) % NUM_BUFFERS;

  XUnlockDisplay (_display);

}


void XVWindow::Sync()
{
  XLockDisplay(_display);
  XSync (_display, false);
  XUnlockDisplay(_display);
}


void 
XVWindow::SetSizeHints (int x, 
                        int y, 
                        int imageWidth, 
                        int imageHeight, 
                        int windowWidth, 
                        int windowHeight)
{
  XSizeHints xshints;

  xshints.flags = PPosition | PSize | PAspect | PMinSize;

  xshints.min_aspect.x = imageWidth;
  xshints.min_aspect.y = imageHeight;
  xshints.max_aspect.x = imageWidth;
  xshints.max_aspect.y = imageHeight;

  xshints.x = x;
  xshints.y = y;
  xshints.width = windowWidth;
  xshints.height = windowHeight;
  xshints.min_width = 25;
  xshints.min_height = 25;
  
  XSetStandardProperties (_display, _XWindow, "Video", "Video", None, NULL, 0, &xshints);
}


unsigned int 
XVWindow::FindXVPort () 
{
  XvAdaptorInfo *xvainfo = NULL;
  XvImageFormatValues *xviformats = NULL;

  unsigned int numXvainfo = 0;
  unsigned int i = 0;
  unsigned int candidateXVPort = 0;
  unsigned int busyPorts = 0;

  int numXviformats = 0;
  int j = 0;

  int result = 0;
  bool supportsYV12 = false;

  if (Success != XvQueryAdaptors (_display, _rootWindow, &numXvainfo, &xvainfo)) {
    PTRACE (1, "XVideo\tXQueryAdaptor failed");
    return 0;
  }

  for (i = 0; i < numXvainfo; i++) {

    char adaptorInfo [512];
    sprintf (adaptorInfo, "XVideo\t#%d, Adaptor: %s, type: %s%s%s%s%s, ports: %ld, first port: %ld", i,
             xvainfo[i].name,
             (xvainfo[i].type & XvInputMask)?"input | ":"",
             (xvainfo[i].type & XvOutputMask)?"output | ":"",
             (xvainfo[i].type & XvVideoMask)?"video | ":"",
             (xvainfo[i].type & XvStillMask)?"still | ":"",
             (xvainfo[i].type & XvImageMask)?"image | ":"",
             xvainfo[i].num_ports,
             xvainfo[i].base_id);
    PTRACE (4, adaptorInfo);

    if ((xvainfo[i].type & XvInputMask) && (xvainfo[i].type & XvImageMask)) {

      for (candidateXVPort = xvainfo [i].base_id ; candidateXVPort < (xvainfo [i].base_id + xvainfo [i].num_ports) ; ++candidateXVPort) {

        if (grabbedPorts.find(candidateXVPort) != grabbedPorts.end()) {
          PTRACE(4, "XVideo\tPort " << candidateXVPort << " already grabbed by ourselves");
          ++busyPorts;
          continue;
        }

#if PTRACING
        DumpCapabilities (candidateXVPort);
#endif

        // Check if the Port supports YV12/YUV colorspace
        supportsYV12 = false;
        xviformats = XvListImageFormats (_display, candidateXVPort, &numXviformats);

        for (j = 0 ; j < numXviformats ; j++)
          if (xviformats [j].id == GUID_YV12_PLANAR)
            supportsYV12 = true;

        if (xviformats)
          XFree (xviformats);

        if (!supportsYV12) {

          PTRACE(4, "XVideo\tPort " << candidateXVPort << " does not support YV12 colorspace");
        }
        else {

          result = XvGrabPort (_display, candidateXVPort, CurrentTime);
          if ( result == Success) {

            PTRACE(4, "XVideo\tGrabbed Port: " << candidateXVPort);
            XvFreeAdaptorInfo (xvainfo);
            grabbedPorts.insert(candidateXVPort);

            return candidateXVPort;
          } 
          else {

            switch (result) 
              {
              case XvInvalidTime: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": requested time is older than the current port time"); 
                break;

              case XvAlreadyGrabbed: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": port is already grabbed by another client"); 
                break;

              case XvBadExtension: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": XV extension is unavailable"); 
                break;

              case XvBadAlloc: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": XvGrabPort failed to allocate memory to process the request"); 
                break;

              case XvBadPort: 
                PTRACE(4, "XVideo\tCould not grab port " << candidateXVPort << ": port does not exist"); 
                break;

              default: 
                PTRACE(4, "XVideo\tCould not grab port " << candidateXVPort);
              }

            ++busyPorts;
          }
        }
      }
    }
  }

  if (busyPorts) 
    PTRACE(1, "XVideo\tCould not find any free Xvideo port - maybe other processes are already using them");
  else 
    PTRACE(1, "XVideo\tIt seems there is no Xvideo support for your video card available");
  XvFreeAdaptorInfo (xvainfo);

  return 0;
}


void 
XVWindow::DumpCapabilities (int port) 
{
  XvImageFormatValues *xviformats = 0;
  XvAttribute *xvattributes = NULL;
  XvEncodingInfo *xveinfo = NULL;

  unsigned int numXveinfo = 0;
  unsigned int i = 0;

  int numXvattributes = 0;
  int j = 0;
  int numXviformats = 0;

  char info[512];

  if (XvQueryEncodings (_display, port, &numXveinfo, &xveinfo) != Success) {
    PTRACE(4, "XVideo\tXvQueryEncodings failed\n");
    return;
  }

  for (i = 0 ; i < numXveinfo ; i++) {
    PTRACE(4, "XVideo\tEncoding List for Port " << port << ": "
           << " id="          << xveinfo [i].encoding_id
           << " name="        << xveinfo [i].name 
           << " size="        << xveinfo [i].width << "x" << xveinfo[i].height
           << " numerator="   << xveinfo [i].rate.numerator
           << " denominator=" << xveinfo [i].rate.denominator);
  }
  XvFreeEncodingInfo(xveinfo);

  PTRACE(4, "XVideo\tAttribute List for Port " << port << ":");
  xvattributes = XvQueryPortAttributes (_display, port, &numXvattributes);
  for (j = 0 ; j < numXvattributes ; j++) {
    PTRACE(4, "  name:       " << xvattributes [j].name);
    PTRACE(4, "  flags:     " << ((xvattributes [j].flags & XvGettable) ? " get" : "") << ((xvattributes [j].flags & XvSettable) ? " set" : ""));
    PTRACE(4, "  min_color:  " << xvattributes [j].min_value);
    PTRACE(4, "  max_color:  " << xvattributes [j].max_value);
  }

  if (xvattributes) 
    XFree (xvattributes);

  PTRACE (4, "XVideo\tImage format list for Port " << port << ":");
  xviformats = XvListImageFormats (_display, port, &numXviformats);
  for (j = 0 ; j < numXviformats ; j++) {

    sprintf (info, "  0x%x (%4.4s) %s, order: %s",
             xviformats [j].id,
             (char *) &xviformats [j].id,
             (xviformats [j].format == XvPacked) ? "packed" : "planar",
             xviformats [j].component_order);
    PTRACE(4, info);
  }

  if (xviformats) 
    XFree (xviformats);
}


Atom
XVWindow::GetXVAtom ( char const * name )
{
  XvAttribute * attributes;
  int numAttributes = 0;
  int i;
  Atom atom = None;

  attributes = XvQueryPortAttributes( _display, _XVPort, &numAttributes );
  if( attributes != NULL ) {
  
    for ( i = 0; i < numAttributes; ++i ) {

      if ( strcmp(attributes[i].name, name ) == 0 ) {

        atom = XInternAtom( _display, name, False );
        break; 
      }
    }
    XFree( attributes );
  }

  return atom;
}

bool
XVWindow::InitColorkey()
{
  if( XV_COLORKEY != None ) {

    if ( XvGetPortAttribute(_display,_XVPort, XV_COLORKEY, &_colorKey) == Success )
      PTRACE(4, "XVideo\tUsing colorkey " << _colorKey );
    else {

      PTRACE(1, "XVideo\tCould not get colorkey! Maybe the selected Xv port has no overlay." );
      return false; 
    }

    if ( XV_AUTOPAINT_COLORKEY != None ) {

      if ( XvSetPortAttribute( _display, _XVPort, XV_AUTOPAINT_COLORKEY, 1 ) == Success )
        PTRACE(4, "XVideo\tColorkey method: AUTOPAINT");
      else {

        _paintColorKey = true;
        PTRACE(4, "XVideo\tFailed to set XV_AUTOPAINT_COLORKEY");
        PTRACE(4, "XVideo\tColorkey method: MANUAL");
      }
    }
    else {

      _paintColorKey = true;
      PTRACE(4, "XVideo\tXV_AUTOPAINT_COLORKEY not supported");
      PTRACE(4, "XVideo\tColorkey method: MANUAL");
    }
  }
  else {

    PTRACE(4, "XVideo\tColorkey method: NONE");
  } 

  return true; 
}

bool
XVWindow::checkMaxSize(unsigned int width, unsigned int height)
{
  XvEncodingInfo * xveinfo;
  unsigned int numXveinfo = 0;
  unsigned int i;
  bool ret = false;

  if (XvQueryEncodings (_display, _XVPort, &numXveinfo, &xveinfo) != Success) {

    PTRACE(4, "XVideo\tXvQueryEncodings failed\n");
    return false;
  }

  for (i = 0 ; i < numXveinfo ; i++) {

    if ( strcmp( xveinfo[i].name, "XV_IMAGE" ) == 0 ) {

      if ( (width <= xveinfo[i].width  ) ||
           (height <= xveinfo[i].height) )
        ret = true;
      else {

        PTRACE(1, "XVideo\tRequested resolution " << width << "x" << height 
	         << " higher than maximum supported resolution " 
		 << xveinfo[i].width << "x" << xveinfo[i].height);
        ret = false;
      }
      break;
    }
  }

  XvFreeEncodingInfo(xveinfo);
  return ret;
}

bool
XVWindow::checkDepth () {
  XWindowAttributes xwattributes;
  XGetWindowAttributes (_display, _rootWindow, &xwattributes);
  _depth = xwattributes.depth;

  if (_depth != 15 && _depth != 16 && _depth != 24 && _depth != 32)
    _depth = 24;

  if (!XMatchVisualInfo (_display, DefaultScreen (_display), _depth, TrueColor, &_XVInfo)) {
    PTRACE(1, "XVideo\tCould not visual with colordepth of " << _depth  << "bits per pixel");
    return false;
  }

  PTRACE(4, "XVideo\tFound visual with colordepth of " << _depth  << "bits per pixel");
  return true;
}


#ifdef HAVE_SHM
void XVWindow::ShmAttach(int imageWidth, int imageHeight)
{
  unsigned int i = 0;
  for (i = 0; i < NUM_BUFFERS; i++) {

    if (_useShm) {
      // create the shared memory portion
      _XVImage[i] = XvShmCreateImage (_display, _XVPort, GUID_YV12_PLANAR, 0, imageWidth, imageHeight, &_XShmInfo[i]);
  
      if (_XVImage[i] == NULL) {
        PTRACE(1, "XVideo\tXShmCreateImage failed");
        _useShm = false;
      }
  
      if ((_XVImage[i]) && (_XVImage[i]->id != GUID_YV12_PLANAR)) {
        PTRACE(1, "XVideo\t  XvShmCreateImage returned a different colorspace than YV12");
        XFree (_XVImage[i]);
        _useShm = false;
      }
    }

#if PTRACING
    if (_useShm) {
      int j = 0;
      PTRACE(4, "XVideo\tCreated XvImage (" << _XVImage[i]->width << "x" << _XVImage[i]->height
             << ", data size: " << _XVImage[i]->data_size << ", num_planes: " << _XVImage[i]->num_planes);

      for (j = 0 ; j < _XVImage[i]->num_planes ; j++)
        PTRACE(4, "XVideo\t  Plane " << j << ": pitch=" << _XVImage[i]->pitches [j] << ", offset=" << _XVImage[i]->offsets [j]);
    }
#endif

    if (_useShm) {
      _XShmInfo[i].shmid = shmget (IPC_PRIVATE, _XVImage[i]->data_size, IPC_CREAT | 0777);
      if (_XShmInfo[i].shmid < 0) {
        XFree (_XVImage[i]);
        PTRACE(1, "XVideo\tshmget failed"); //strerror(errno)
        _useShm = FALSE;
      }
    }
  
    if (_useShm) {
      _XShmInfo[i].shmaddr = (char *) shmat(_XShmInfo[i].shmid, 0, 0);
      if (_XShmInfo[i].shmaddr == ((char *) -1)) {
        XFree (_XVImage[i]);
        _XVImage[i] = NULL;
        PTRACE(1, "XVideo\tshmat failed"); //strerror(errno)
        _useShm = FALSE;
      }
    }
  
    if (_useShm) {
      _XVImage[i]->data = _XShmInfo[i].shmaddr;
      _XShmInfo[i].readOnly = False;
  
      // Attaching the shared memory to the display
      XErrorHandler oldHandler = XSetErrorHandler((XErrorHandler) catchXShmError);
      Status status = XShmAttach (_display, &_XShmInfo[i]);
      XSync(_display, False);
      XSetErrorHandler((XErrorHandler) oldHandler);

      if ( (status != True) || (_shmError) ) {
        XFree (_XVImage[i]);
        _XVImage[i] = NULL;
        if (_XShmInfo[i].shmaddr != ((char *) -1)) {
          shmdt(_XShmInfo[i].shmaddr);
        }
        PTRACE(1, "XVideo\t  XShmAttach failed");
        if ( (status == True) && (_shmError) ) {
          PTRACE(1, "XVideo\t  X server supports SHM but apparently we are remotely connected...");
        }
        _useShm = false;
      } 
    } 
  
    if (_useShm) {
      shmctl(_XShmInfo[i].shmid, IPC_RMID, 0);
    }
  }
}
#endif
