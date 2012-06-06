
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
 *                         DXWindow.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun April 15 2007
 *   copyright            : (C) 2007 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering DirectDraw hardware 
 *                          acceleration.
 */

#include "dxwindow.h"
#include <iostream>

#include "../../src/common.h"

#define PIP_RATIO_WIN  3
#define PIP_RATIO_FS   5

DXWindow::DXWindow()
{

  PTRACE(4, "DirectX\tDXWindow Constructor called");
  
  _state.fullscreen = false;
  _state.ontop = false;
  _state.decoration = true;
  _DDraw = NULL;
  _DXSurface.primary = NULL;
  _DXSurface.overlay = NULL;
  _DXSurface.back = NULL;
  _DXSurface.clipper = NULL;
  _DXWindow = NULL;
  _instance = GetModuleHandle(NULL);

  _windowTitle = "Video";
  _embedded = false;
  _pip = true;
  _sizemove = false;
}


DXWindow::~DXWindow()
{
  PTRACE(4, "DirectX\tDXWindow Destructor called");

  MSG message;
  char errormsg [1024];	
  memset (&errormsg, 0, sizeof (errormsg));

  // release DirectDraw Resources
  if (_DXSurface.clipper) 
    _DXSurface.clipper->Release ();

  if (_DXSurface.back) 
    _DXSurface.back->Release ();

  if (_DXSurface.overlay) 
    _DXSurface.overlay->Release ();

  if (_DXSurface.primary) 
    _DXSurface.primary->Release ();

  if (_DDraw) 
    _DDraw->Release ();

  if (!_embedded) {
    //destroy the window
    if (_DXWindow)  {
      if (!DestroyWindow (_DXWindow)) {
        PTRACE (1, "DirectX\tDestroyWindow failed - " << ErrorMessage ());
      }
    }

    // empty the message queue
    while (PeekMessage (&message, NULL, 0, 0, PM_REMOVE)) {
      TranslateMessage (&message );
      DispatchMessage (&message );
    }

    // unregister the class
    if (!UnregisterClass (_windowTitle, _instance)) {
      PTRACE (1, "DirectX\tUnregisterClass failed - " << ErrorMessage ());
    }
  }
}


bool
DXWindow::Init (HWND rootWindow,
                int x, 
                int y,
                int windowWidth,
                int windowHeight,
                int imageWidth,
                int imageHeight)
{
  _pip = false;
  return ( Init(rootWindow, 
                 x,
                 y,
                 windowWidth,
                 windowHeight,
                 imageWidth, 
                 imageHeight,
                 0,
                 0));
}


bool 
DXWindow::Init (HWND rootWindow, 
                int x,
                int y,
                int windowWidth,
                int windowHeight,
                int imageWidth,
                int imageHeight,
                int PIPimageWidth,
                int PIPimageHeight)
{
  HRESULT ddResult;
  DDCAPS ddCaps;
  DDSURFACEDESC ddSurfaceDesc;

  // the window always seems to be 4 pixels short, ugly hack
  if (!rootWindow) {
    windowWidth += 4;
    windowHeight += 4;
  }

  char errormsg [1024];	
  memset (&errormsg, 0, sizeof (errormsg));

  // the destination surface is actually the original YV12 image
  if (!SetRect (&_DXSurface.mainSrc, 0, 0, imageWidth, imageHeight)) {
    PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage ());
    return false;
  }

  if (_pip) {
    if (!SetRect(&_DXSurface.pipSrc, 0, 0, PIPimageWidth, PIPimageHeight)) {
      PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage ());
      return false;
    }
  }

  // create the actual window
  if (rootWindow) {
    _embedded = true;
    _DXWindow = rootWindow;
  } 
  else {
    _embedded = false;
    if (!NewWindow (x, y, windowWidth, windowHeight)) 
      return false; 
  }

  PTRACE(4, "DirectX\tConfiguring DirectX");

  // create a DirectDraw object
  ddResult = DirectDrawCreate (NULL, &_DDraw, NULL);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tDirectDrawCreate failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  ddResult = _DDraw->SetCooperativeLevel (_DXWindow, DDSCL_NORMAL | DDSCL_MULTITHREADED);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tSetCooperativeLevel failed - " << DDErrorMessage (ddResult));  
    return false; 
  }

  // check for overlay and stretch capabilities
  ddCaps.dwSize = sizeof (ddCaps);
  ddResult = _DDraw->GetCaps (&ddCaps, NULL);

  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tGetCaps failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  if (!(ddCaps.dwCaps & DDCAPS_OVERLAY)) {
    PTRACE (1, "DirectX\tno Overlay Capabilities"); 
    return false; 
  }

  if (!(ddCaps.dwCaps & DDCAPS_OVERLAYSTRETCH)) { 
    PTRACE (1, "DirectX\tno Stretch Capabilities"); 
    return false; 
  }

  if (!(ddCaps.dwCaps & DDCAPS_BLT)) {
    PTRACE (1, "DirectX\tno Blt Capabilities"); 
    return false; 
  }

  if (ddCaps.dwCaps & DDCAPS_ALIGNBOUNDARYDEST) 
    PTRACE(4, "DirectX\tdwAlignBoundaryDest " << ddCaps.dwAlignBoundaryDest); 

  if (ddCaps.dwCaps & DDCAPS_ALIGNBOUNDARYSRC) 
    PTRACE(4, "DirectX\tdwAlignBoundarySrc " << ddCaps.dwAlignBoundarySrc);

  if (ddCaps.dwCaps & DDCAPS_ALIGNSIZEDEST) 
    PTRACE(4, "DirectX\tdwAlignSizeDest " << ddCaps.dwAlignSizeDest);

  if (ddCaps.dwCaps & DDCAPS_ALIGNSIZESRC)      
    PTRACE(4, "DirectX\tdwAlignSizeSrc " << ddCaps.dwAlignSizeSrc); 

  // create a primary surface where all other surfaces are located
  PTRACE(4, "DirectX\tCreating Primary Surface");
  memset (&ddSurfaceDesc, 0, sizeof (DDSURFACEDESC));
  ddSurfaceDesc.dwSize = sizeof (DDSURFACEDESC);
  ddSurfaceDesc.dwFlags = DDSD_CAPS;
  ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

  ddResult = _DDraw->CreateSurface (&ddSurfaceDesc, &_DXSurface.primary, NULL);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tCreateSurface(primary) failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  // create a backbuffer surface, this is necessary because we first paint the main frame 
  // and then the PIP frame ontop, which leads to flickering if done on the actual overlay
  PTRACE(4, "DirectX\tCreating Bacbuffer Surface");
  memset ( &ddSurfaceDesc, 0, sizeof (ddSurfaceDesc) );
  ddSurfaceDesc.dwSize = sizeof ( ddSurfaceDesc );
  ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT ;
  ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
  ddSurfaceDesc.dwWidth = GetSystemMetrics(SM_CXSCREEN);
  ddSurfaceDesc.dwHeight = GetSystemMetrics(SM_CYSCREEN);

  ddResult = _DDraw->CreateSurface (&ddSurfaceDesc, &_DXSurface.back, NULL);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tCreateSurface(Backbuffer) failed - " << DDErrorMessage(ddResult));  
    return false; 
  }

  PTRACE(4, "DirectX\tCreating Overlay Surface");
  // this is the actual overlay surface, size is the highest value of width and height 
  // of both frames to be displayed
  memset (&ddSurfaceDesc, 0, sizeof (DDSURFACEDESC));
  ddSurfaceDesc.dwSize = sizeof (DDSURFACEDESC);
  ddSurfaceDesc.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_BACKBUFFERCOUNT | DDSD_PIXELFORMAT;
  ddSurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OVERLAY | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

  if (PIPimageWidth > imageWidth) 
    ddSurfaceDesc.dwWidth = PIPimageWidth;
  else 
    ddSurfaceDesc.dwWidth = imageWidth;

  if (PIPimageHeight > imageHeight) 
    ddSurfaceDesc.dwHeight = PIPimageHeight;
  else 
    ddSurfaceDesc.dwHeight = imageHeight;

  ddSurfaceDesc.dwBackBufferCount = 1;
  ddSurfaceDesc.ddpfPixelFormat.dwSize = sizeof( DDPIXELFORMAT);
  ddSurfaceDesc.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
  ddSurfaceDesc.ddpfPixelFormat.dwFourCC = mmioFOURCC ('Y', 'V', '1', '2');
  ddResult = _DDraw->CreateSurface (&ddSurfaceDesc, &_DXSurface.overlay, NULL);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tCreateSurface(Overlay) failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  PTRACE (4, "DirectX\tCreating Clipper");
  // create a clipper that other windows may cover the overlay when not on top
  ddResult=_DDraw->CreateClipper (0, &_DXSurface.clipper, NULL);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tCreateClipper failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  ddResult = _DXSurface.clipper->SetHWnd (0, _DXWindow);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tSetHWnd failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  ddResult = _DXSurface.primary->SetClipper (_DXSurface.clipper);
  if (ddResult != DD_OK) { 
    PTRACE (1, "DirectX\tSetClipper failed - " << DDErrorMessage (ddResult)); 
    return false; 
  }

  PTRACE(4, "DirectX\tShowing Window");
  if (_embedded) {
    SetWindow (x,y,windowWidth, windowHeight); 
  }
  else {
    ShowWindow (_DXWindow, SW_SHOW);
    SetOverlayToWindow ();
  }

  PTRACE(4, "DirectX\tLeaving Init");
  return true;	
}

void DXWindow::ProcessEvents()
{
  PTRACE(4, "DirectX\tEntering processEvents");
  MSG message;

  // handle messages
  while (PeekMessage (&message, _DXWindow, 0, 0, PM_REMOVE)) {

    TranslateMessage (&message);
    DispatchMessage (&message);
  }

  if (_embedded) 
    CalculateEmbWindCoord (); 

  PTRACE(4, "DirectX\tLeaving processEvents");
}

void
DXWindow::PutFrame (uint8_t *frame, 
                    uint16_t width,
                    uint16_t height,
		    bool pip)
{
  PTRACE(4, "DirectX\tEntering putFrame");

  if (_sizemove) 
    return;

  if (!pip) {
    if (   ( width  != (_DXSurface.mainSrc.right - _DXSurface.mainSrc.left))
        || ( height != (_DXSurface.mainSrc.bottom - _DXSurface.mainSrc.top)) ) {
      PTRACE (1, "DirectX\tDynamic switching of resolution not supported\n");
      PTRACE (1, "DirectX\tMy Resolution: " << (_DXSurface.mainSrc.right  - _DXSurface.mainSrc.left) 
              << "x" << (_DXSurface.mainSrc.bottom - _DXSurface.mainSrc.top )
              << ", frame: " << width << "x" << height);
      return;
    }
    // do a memcopy of the main frame and blt it to the backbuffer
    CopyFrameBackbuffer (frame, width, height, &_DXSurface.mainBack, &_DXSurface.mainSrc);
  }
  else {
      if (   ( width  != (_DXSurface.pipSrc.right  - _DXSurface.pipSrc.left))
        || ( height != (_DXSurface.pipSrc.bottom - _DXSurface.pipSrc.top )) ) {
      PTRACE (1, "DirectX\tDynamic switching of resolution not supported\n");
      PTRACE (1, "DirectX\tMy PIP Resolution: " << (_DXSurface.pipSrc.right  - _DXSurface.pipSrc.left) 
              << "x" << (_DXSurface.pipSrc.bottom - _DXSurface.pipSrc.top )
              << ", PIP frame: " << width << "x" << height);
      return;
    }
    // do a memcopy of the pip frame and blt it to the backbuffer
    CopyFrameBackbuffer (frame, width, height, &_DXSurface.pipBack, &_DXSurface.pipSrc);
  }
  PTRACE(4, "DirectX\tLeaving putFrame");
}

void DXWindow::Sync()
{
  HRESULT ddResult, ddResultRestore;

  PTRACE(4, "DirectX\tEntering Sync");
  // Blt the combined pip and main window from the backbuffer 
  // to the final position on the visible primary surface
  ddResult = _DXSurface.primary->Blt (&_DXSurface.primaryDst,
                                      _DXSurface.back, 
                                      &_DXSurface.mainBack, 
                                      DDBLT_WAIT, 
                                      NULL);

  if (ddResult == DDERR_SURFACELOST) {
    ddResultRestore = _DXSurface.primary->Restore ();
    if ( ddResultRestore != DD_OK) {
      PTRACE (1, "DirectX\tRestore failed - " << DDErrorMessage (ddResultRestore));
      return; 
    }
  }
  else if (ddResult != DD_OK) {
    PTRACE (1, "DirectX\tBlt failed - " << DDErrorMessage (ddResult));
    return;
  }
  PTRACE(4, "DirectX\tLeaving Sync");
}


void 
DXWindow::ToggleOntop ()
{
  char errormsg [1024];	
  memset (&errormsg, 0, sizeof(errormsg));

  if (!SetWindowPos (_DXWindow, 
                     (_state.ontop) ? HWND_TOPMOST : HWND_NOTOPMOST, 
                     0,
                     0,
                     0,
                     0,
                     SWP_NOSIZE | SWP_NOMOVE ))
    PTRACE (1, "DirectX\tSetWindowPos failed - " << ErrorMessage());
  _state.ontop = !_state.ontop;
}


void
DXWindow::ToggleFullscreen ()
{
  WINDOWINFO windowInfo;

  char errormsg [1024];	
  memset (&errormsg, 0, sizeof (errormsg));

  if (!_state.fullscreen) {

    windowInfo.cbSize = sizeof (WINDOWINFO);
    if (!GetWindowInfo (_DXWindow, &windowInfo)) {
      PTRACE (1, "DirectX\tGetWindowInfo failed - " << ErrorMessage ());
      return;
    }

    _state.oldWinFS = windowInfo.rcWindow;

    SetLastError (0);
    if ((!SetWindowLongPtr (_DXWindow, 
                            GWL_STYLE, 
                            WS_OVERLAPPED | WS_VISIBLE))
        && (GetLastError ()))
      PTRACE (1, "DirectX\tSetWindowLongPtr failed - " << ErrorMessage());

    if (!SetWindowPos (_DXWindow, 
                       HWND_TOPMOST,
                       0,
                       0,
                       GetSystemMetrics (SM_CXSCREEN),
                       GetSystemMetrics (SM_CYSCREEN), 
                       SWP_FRAMECHANGED))
      PTRACE (1, "DirectX\tSetWindowPos failed - " << ErrorMessage());

  } 
  else {

    if (_state.decoration)  {

      SetLastError (0);
      if ((!SetWindowLongPtr (_DXWindow,
                              GWL_STYLE,
                              WS_OVERLAPPEDWINDOW | WS_SIZEBOX | WS_VISIBLE))
          && (GetLastError ()))
        PTRACE (1, "DirectX\tSetWindowLongPtr failed - " << ErrorMessage ());
    }

    if (!SetWindowPos (_DXWindow,
                       (_state.ontop) ?  HWND_TOPMOST : HWND_NOTOPMOST, 
                       _state.oldWinFS.left,
                       _state.oldWinFS.top,
                       _state.oldWinFS.right - _state.oldWinFS.left,
                       _state.oldWinFS.bottom - _state.oldWinFS.top,
                       SWP_NOOWNERZORDER | SWP_FRAMECHANGED))
      PTRACE (1, "DirectX\tSetWindowPos failed - " << ErrorMessage());
  }

  PTRACE(4, "DirectX\tCaller: ToggleFullscreen");

  _state.fullscreen = !_state.fullscreen;

  SetOverlayToWindow ();
  SetForegroundWindow (_DXWindow);
}


void
DXWindow::ToggleDecoration ()
{
  char errormsg [1024];	
  memset (&errormsg, 0, sizeof (errormsg));
  WINDOWINFO windowInfo;

  if (_state.decoration) {

    windowInfo.cbSize = sizeof (WINDOWINFO);
    if (!GetWindowInfo (_DXWindow, &windowInfo)) {
      PTRACE (1, "DirectX\tGetWindowInfo failed - " << ErrorMessage ());
      return;
    }

    _state.oldWinD = windowInfo.rcWindow;

    SetLastError (0);
    if ((!SetWindowLongPtr (_DXWindow, 
                            GWL_STYLE, 
                            WS_OVERLAPPED | WS_VISIBLE))
        && (GetLastError ()))
      PTRACE (1, "DirectX\tSetWindowLongPtr failed - " << ErrorMessage());

    if (!SetWindowPos (_DXWindow,
                       (_state.ontop) ?  HWND_TOPMOST : HWND_NOTOPMOST,
                       windowInfo.rcClient.left,
                       windowInfo.rcClient.top,
                       windowInfo.rcClient.right  - windowInfo.rcClient.left,
                       windowInfo.rcClient.bottom - windowInfo.rcClient.top,
                       SWP_NOOWNERZORDER | SWP_FRAMECHANGED))
      PTRACE (1, "DirectX\tSetWindowLongPtr failed - " << ErrorMessage());
  } 
  else {

    SetLastError (0);
    if ((!SetWindowLongPtr (_DXWindow,
                            GWL_STYLE, 
                            WS_OVERLAPPEDWINDOW | WS_SIZEBOX | WS_VISIBLE))
        && (GetLastError ()))
      PTRACE (1, "DirectX\tSetWindowLongPtr failed - " << ErrorMessage ());

    if (!SetWindowPos (_DXWindow, 
                       (_state.ontop) ?  HWND_TOPMOST : HWND_NOTOPMOST,
                       _state.oldWinD.left,
                       _state.oldWinD.top,
                       _state.oldWinD.right  - _state.oldWinD.left,
                       _state.oldWinD.bottom - _state.oldWinD.top,
                       SWP_NOOWNERZORDER | SWP_FRAMECHANGED))
      PTRACE (1, "DirectX\tSetWindowPos failed - " << ErrorMessage());
  }	

  PTRACE(4, "DirectX\tCaller: ToggleDecoration");

  _state.decoration = !_state.decoration;
  SetOverlayToWindow ();
}


void
DXWindow::GetWindow (int *x,
                     int *y, 
                     unsigned int *windowWidth,
                     unsigned int *windowHeight)
{
  WINDOWINFO windowInfo;
  
  char errormsg [1024];	
  memset (&errormsg, 0, sizeof (errormsg));

  windowInfo.cbSize = sizeof (WINDOWINFO);
  if (!GetWindowInfo (_DXWindow, &windowInfo)) 
    PTRACE (1, "DirectX\tGetWindowInfo failed - " << ErrorMessage ()); 

  (*x) = windowInfo.rcClient.left;
  (*y) = windowInfo.rcClient.top;
  (*windowWidth) = windowInfo.rcClient.right - windowInfo.rcClient.left;
  (*windowHeight) = windowInfo.rcClient.bottom - windowInfo.rcClient.top;
  PTRACE (4, "DirectX\tGetWindow - " << (*x) << "," << (*y) << " " << (*windowWidth) << "x"<< (*windowHeight)); 
}


LRESULT CALLBACK 
DXWindow::WndProc (HWND window,
                   UINT message, 
                   WPARAM wParam,
                   LPARAM lParam)
{
  DXWindow *wnd = reinterpret_cast<DXWindow*>(GetWindowLongPtr (window, GWLP_USERDATA));

  if (wnd)
    return wnd->HandleMessage (window, message, wParam, lParam);
  else
    return DefWindowProc (window, message, wParam, lParam);
}


LRESULT
DXWindow::HandleMessage (HWND window, 
                         UINT message, 
                         WPARAM wParam,
                         LPARAM lParam)
{
  switch(message) 
    {
    case WM_CLOSE:
      return 0;
    case WM_EXITSIZEMOVE:
      PTRACE(4, "DirectX\tWM_EXITSIZEMOVE");
      _sizemove = false;
      SetOverlayToWindow ();
      return 0;
    case WM_ENTERSIZEMOVE:
      PTRACE(4, "DirectX\tWM_ENTERSIZEMOVE");
      _sizemove = true;
      return 0;
    case WM_LBUTTONDOWN:
      if (!_state.decoration) 
        ToggleDecoration ();
      else 
        ToggleFullscreen ();
      return 0;
    case WM_CHAR:
      switch (wParam) 
        {
        case 'f' :
        case 'F' :
          ToggleFullscreen ();
          return 0;
        case 'o' :
        case 'O' :
          if (!_state.fullscreen) 
            ToggleOntop ();
          return 0;
        case 'd' :
        case 'D' :
          if (!_state.fullscreen) 
            ToggleDecoration ();
          return 0;
        case 27:
          if (_state.fullscreen) 
            ToggleFullscreen ();
          return 0;
        }
      return 0;	
    case WM_DESTROY:	
      PostQuitMessage (0);
      return 0;
    default:
      return DefWindowProc (window, message, wParam, lParam);
    }

  return 0;
}


void
DXWindow::CopyFrameBackbuffer (uint8_t *frame, 
                               uint16_t width,
                               uint16_t height,
                               RECT *src,
                               RECT *dst)
{
  DDSURFACEDESC ddSurfaceDesc;
  DDBLTFX ddBltFX;
  HRESULT ddResult, ddResultRestore;

  memset (&ddSurfaceDesc, 0, sizeof (ddSurfaceDesc));	
  ddSurfaceDesc.dwSize = sizeof (ddSurfaceDesc);	

  // locks the shared memory area
  ddResult = _DXSurface.overlay->Lock (NULL, 
                                       &ddSurfaceDesc, 
                                       DDLOCK_NOSYSLOCK | DDLOCK_WAIT,
                                       NULL);

  if (ddResult == DDERR_SURFACELOST) {
    ddResultRestore = _DXSurface.overlay->Restore ();
    if ( ddResultRestore != DD_OK) {
      PTRACE (1, "DirectX\tRestore failed - " << DDErrorMessage (ddResultRestore));
      return; 
    }
  }
  else if ( ddResult != DD_OK) {
    PTRACE (1, "DirectX\tLock failed - " << DDErrorMessage(ddResult));
    return; 
  }

  // copy the frame data
  uint16_t i = 0;
  uint16_t width2  = width >> 1;
  uint16_t lPitch2 = ddSurfaceDesc.lPitch >> 1;

  uint8_t *dstY = (uint8_t*) ddSurfaceDesc.lpSurface;
  uint8_t *dstV = (uint8_t*) ddSurfaceDesc.lpSurface +  (unsigned int) (ddSurfaceDesc.lPitch * ddSurfaceDesc.dwHeight);
  uint8_t *dstU = (uint8_t*) ddSurfaceDesc.lpSurface + ((unsigned int) (ddSurfaceDesc.lPitch * ddSurfaceDesc.dwHeight * 5) >> 2);
  uint8_t *srcY = frame;
  uint8_t *srcV = frame + ((unsigned int) (width * height * 5) >> 2);
  uint8_t *srcU = frame + (unsigned int) (width * height);

  for (i = 0 ; i < height ; i += 2) {

    memcpy (dstY, srcY, width);  
    dstY += ddSurfaceDesc.lPitch; 
    srcY += width;

    memcpy (dstY, srcY, width);  
    dstY += ddSurfaceDesc.lPitch; 
    srcY += width;

    memcpy (dstV, srcV, width2); 
    dstV += lPitch2;              
    srcV += width2;

    memcpy (dstU, srcU, width2); 
    dstU += lPitch2;              
    srcU += width2;
  }

  // unlocks the memory area
  ddResult = _DXSurface.overlay->Unlock (NULL);

  if (ddResult == DDERR_SURFACELOST) {
    ddResultRestore = _DXSurface.overlay->Restore ();
    if ( ddResultRestore != DD_OK) {
      PTRACE (1, "DirectX\tRestore failed - " << DDErrorMessage (ddResultRestore));
      return; 
    }
  }
  else if ( ddResult != DD_OK) {
    PTRACE (1, "DirectX\tUnlock failed - " << DDErrorMessage (ddResult));
    return; 
  }

  memset (&ddBltFX, 0, sizeof (ddBltFX));	
  ddBltFX.dwSize = sizeof (DDBLTFX);

  // writes the copied data to the (back) surface
  ddResult = _DXSurface.back->Blt (src,
                                   _DXSurface.overlay,
                                   dst,
                                   DDBLT_WAIT,
                                   &ddBltFX);
  if (ddResult == DDERR_SURFACELOST) {
    ddResultRestore = _DXSurface.back->Restore ();
    if ( ddResultRestore != DD_OK) {
      PTRACE (1, "DirectX\tRestore failed - " << DDErrorMessage(ddResultRestore));
      return; 
    }
  }
  else if ( ddResult != DD_OK) {
    PTRACE (1, "DirectX\tBlt failed - " << DDErrorMessage(ddResult));
    return; 
  }
}


void 
DXWindow::CalculateBackBuffer ()
{
  // The backbuffer surface does not support clippers, we thus  move the window
  // to 0,0, from where it is blted to the final position in putFrame

  if (!SetRect (&_DXSurface.mainBack,
                0,
                0,
                _DXSurface.primaryDst.right - _DXSurface.primaryDst.left,
                _DXSurface.primaryDst.bottom - _DXSurface.primaryDst.top)) {
    PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage());
    return;
  }

  // PIP width depends on PIP_RATIO
  if (_pip) {

    _DXSurface.pipBack.right = _DXSurface.mainBack.right;
    _DXSurface.pipBack.bottom = _DXSurface.mainBack.bottom;
    _DXSurface.pipBack.left = (int) (_DXSurface.mainBack.right - 
                                     ((_DXSurface.mainBack.right - _DXSurface.mainBack.left) /
				     ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN)));
    _DXSurface.pipBack.top = (int) (_DXSurface.pipBack.bottom - 
                                    ((_DXSurface.pipBack.right - _DXSurface.pipBack.left) *
                                     _DXSurface.pipSrc.bottom / _DXSurface.pipSrc.right));
  }
}


void 
DXWindow::CalculateEmbWindCoord ()
{
  WINDOWINFO windowInfo;

  PTRACE(4,"DirectX\tGetting the window coordinates");
  windowInfo.cbSize = sizeof (WINDOWINFO);
  if (!GetWindowInfo (_DXWindow, &windowInfo)) {
    PTRACE (1, "DirectX\tGetWindowInfo failed - " << ErrorMessage());
    return;
  }

  if (!SetRect (&_DXSurface.primaryDst, 
                windowInfo.rcClient.left + _DXSurface.embeddedRelative.left,
                windowInfo.rcClient.top  + _DXSurface.embeddedRelative.top, 
                windowInfo.rcClient.left + _DXSurface.embeddedRelative.right, 
                windowInfo.rcClient.top  + _DXSurface.embeddedRelative.bottom)) {
    PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage ());
    return;
  }

  PTRACE(4, "DirectX\tnew Embedded Coordinates: " << _DXSurface.primaryDst.left << "," << _DXSurface.primaryDst.top << " - " << _DXSurface.primaryDst.right << "," << _DXSurface.primaryDst.bottom);
}


void 
DXWindow::SetOverlayToWindow ()
{
  WINDOWINFO windowInfo;

  PTRACE(4,"DirectX\tSetOverlaytoWindow");
  windowInfo.cbSize = sizeof (WINDOWINFO);
  if (!GetWindowInfo (_DXWindow, &windowInfo)) {
    PTRACE (1, "DirectX\tGetWindowInfo failed - " << ErrorMessage());
    return;
  }

  SetWindow (windowInfo.rcClient.left,
             windowInfo.rcClient.top,
             windowInfo.rcClient.right - windowInfo.rcClient.left,
             windowInfo.rcClient.bottom - windowInfo.rcClient.top);
}


void
DXWindow::SetWindow (int x,
                     int y,
                     unsigned int windowWidth,
                     unsigned int windowHeight)
{
  RECT newRect, windowRect;

  PTRACE (4, "DirectX\tSetWindow - " << x << "," << y << " " << windowWidth << "x" << windowHeight); 
  if (_embedded) {

    if (!SetRect(&_DXSurface.embeddedRelative, 
                 x, 
                 y, 
                 x + windowWidth, 
                 y + windowHeight)) {

      PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage ());
      return;
    }

    CalculateEmbWindCoord ();
    CalculateBackBuffer ();

  } 
  else {

    if (!SetRect(&newRect, 
                 x, 
                 y, 
                 x + windowWidth, 
                 y + windowHeight)) {
      PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage ());
      return;
    }

    PTRACE(4, "DirectX\tnew Window Coordinates: " << newRect.left << "," << newRect.top << " - " << newRect.right << "," << newRect.bottom);

    CorrectAspectRatio (newRect);

    CalculateBackBuffer ();

    PTRACE(4, "DirectX\tnew Corrected Coordinates: " << _DXSurface.primaryDst.left << "," << _DXSurface.primaryDst.top << " - " << _DXSurface.primaryDst.right << "," << _DXSurface.primaryDst.bottom);

    if (!_state.fullscreen && _state.decoration) {

      windowRect = _DXSurface.primaryDst;
      if (!AdjustWindowRect (&windowRect,
                             (_state.decoration) ? WS_OVERLAPPEDWINDOW | WS_SIZEBOX : WS_OVERLAPPED,
                             0))
        PTRACE (1, "DirectX\tAdddddjustWindowRect failed - " << ErrorMessage());

      windowRect.left   -= 2; 
      windowRect.right  += 2;
      windowRect.top    -= 2; 
      windowRect.bottom += 2;	

      if (!SetWindowPos (_DXWindow, 
                         (_state.ontop) ? HWND_TOPMOST : HWND_NOTOPMOST, 
                         windowRect.left,
                         windowRect.top,
                         windowRect.right - windowRect.left,
                         windowRect.bottom - windowRect.top,
                         0))
        PTRACE (1, "DirectX\tSetWindowPos failed - " << ErrorMessage());
    }
  }
}


void 
DXWindow::CorrectAspectRatio (RECT uncorrected)
{
  int imgWidth = _DXSurface.mainSrc.right - _DXSurface.mainSrc.left;
  int imgHeight = _DXSurface.mainSrc.bottom - _DXSurface.mainSrc.top;
  int scrWidth = uncorrected.right - uncorrected.left;
  int scrHeight = uncorrected.bottom - uncorrected.top;

  // We have to limit the window height to a certain minimum  
  // in order to prevent the Blt operation from whining..
  // The width is already limited by windows
  if (scrHeight < 100) {

    uncorrected.top -= (int) ((100 - scrHeight) / 2);
    uncorrected.bottom += (int) ((100 - scrHeight) / 2);

    if (uncorrected.top < 5) {
      uncorrected.bottom -= (uncorrected.top - 5);
      uncorrected.top = 5;
    }

    scrHeight = uncorrected.bottom - uncorrected.top;
  }

  // aspect ratio correction
  if ((scrWidth * imgHeight / imgWidth) > scrHeight) {

    _DXSurface.primaryDst.left = uncorrected.left + (int) ((scrWidth - (scrHeight * imgWidth / imgHeight)) / 2);
    _DXSurface.primaryDst.top = uncorrected.top + 0;
    _DXSurface.primaryDst.right = _DXSurface.primaryDst.left + (int) (scrHeight * imgWidth / imgHeight);
    _DXSurface.primaryDst.bottom = _DXSurface.primaryDst.top + scrHeight;

  } 
  else if ((scrHeight * imgWidth / imgHeight) > scrWidth) {

    _DXSurface.primaryDst.left = uncorrected.left + 0;
    _DXSurface.primaryDst.top = uncorrected.top + (int) ((scrHeight - (scrWidth * imgHeight / imgWidth)) / 2);
    _DXSurface.primaryDst.right = _DXSurface.primaryDst.left + scrWidth;
    _DXSurface.primaryDst.bottom = _DXSurface.primaryDst.top + (int)(scrWidth * imgHeight / imgWidth);

  } 
  else {

    _DXSurface.primaryDst = uncorrected;
  }
}


bool
DXWindow::NewWindow (int x, int y, int windowWidth, int windowHeight)
{
  char errormsg [1024];	
  memset (&errormsg, 0, sizeof(errormsg));
  WNDCLASSEX windowClass;
  RECT windowRect;

  PTRACE(4, "DirectX\tCreating Window");
  if (!SetRect (&windowRect, x, y, x + windowWidth, y + windowHeight)) {
    PTRACE (1, "DirectX\tSetRect failed - " << ErrorMessage ()); 
  }

  // the window has to be larger because of titlebar and border lines
  if (!AdjustWindowRect (&windowRect, 
                         WS_OVERLAPPEDWINDOW | WS_SIZEBOX, 
                         0))
  {
    PTRACE (1, "DirectX\tAdjustWindowRect failed - " << ErrorMessage ()); 
  }

  // define a window class
  windowClass.cbSize        = sizeof (WNDCLASSEX);
  windowClass.style         = CS_VREDRAW | CS_HREDRAW;
  windowClass.lpfnWndProc   = (WNDPROC) WndProc;
  windowClass.cbClsExtra    = 0;
  windowClass.cbWndExtra    = sizeof (DXWindow *);
  windowClass.hInstance     = _instance;
  windowClass.hIcon         = LoadIcon (NULL, IDI_APPLICATION);
  windowClass.hCursor       = LoadCursor (NULL, IDC_ARROW);
  windowClass.hbrBackground = CreateSolidBrush (RGB (0,0,0)); 
  windowClass.lpszMenuName  = NULL;
  windowClass.lpszClassName = _windowTitle; 
  windowClass.hIconSm       = LoadIcon (NULL, IDI_APPLICATION);

  // register the window class
  if (!RegisterClassEx (&windowClass)) {
    PTRACE (1, "DirectX\tRegisterClassEx failed - " << ErrorMessage ()); 
    return false;
  };

  _DXWindow = CreateWindowEx (WS_EX_CLIENTEDGE, 
                              _windowTitle,
                              _windowTitle,
                              WS_OVERLAPPEDWINDOW | WS_SIZEBOX,
                              x,
                              y,
                              windowRect.right - windowRect.left,
                              windowRect.bottom - windowRect.top,
                              GetDesktopWindow (),
                              NULL, 
                              _instance,
                              NULL);
  if (_DXWindow == NULL) {
    PTRACE (1, "DirectX\tCreateWindowEx failed - " << ErrorMessage ()); 
    return false;
  }

  SetLastError (0);
  if ((!SetWindowLongPtr (_DXWindow,
                          GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(this)))
    && (GetLastError()))
  {
    PTRACE (1, "DirectX\tSetWindowLongPtr failed - " << ErrorMessage ());
    return false;
  }

  ShowWindow (_DXWindow, SW_SHOWNORMAL);

  if (!UpdateWindow (_DXWindow)) {
    PTRACE (1, "DirectX\tUpdateWindow failed - " << ErrorMessage ());
  }

 return true;
}


const char* 
DXWindow::ErrorMessage()
{
  static char string [1024];
  DWORD dwMsgLen;

  memset (string, 0, sizeof (string));
  dwMsgLen = FormatMessageA (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL,
                             GetLastError (),
                             MAKELANGID (LANG_NEUTRAL, SUBLANG_DEFAULT),
                             (LPSTR) string,
                             sizeof (string)-1,
                             NULL);
  if (dwMsgLen) {
    string [ strlen(string) - 2 ] = 0;
    snprintf (string, sizeof (string), "%s (%u)", string, (int) GetLastError ());
  }
  else {
    snprintf (string, sizeof (string), "%u", (int) GetLastError ());
  }

  return string;
}


const char*
DXWindow::DDErrorMessage(int code)
{
  static const char *error;
  static char errormsg [1024];	

  switch (code) {
    case DDERR_GENERIC:
      error = "Undefined error!";
      break;
    case DDERR_EXCEPTION:
      error = "Exception encountered";
      break;
    case DDERR_INVALIDOBJECT:
      error = "Invalid object";
      break;
    case DDERR_INVALIDPARAMS:
      error = "Invalid parameters";
      break;
    case DDERR_NOTFOUND:
      error = "Object not found";
      break;
    case DDERR_INVALIDRECT:
      error = "Invalid rectangle";
      break;
    case DDERR_INVALIDCAPS:
      error = "Invalid caps member";
      break;
    case DDERR_INVALIDPIXELFORMAT:
      error = "Invalid pixel format";
      break;
    case DDERR_OUTOFMEMORY:
      error = "Out of memory";
      break;
    case DDERR_OUTOFVIDEOMEMORY:
      error = "Out of video memory";
      break;
    case DDERR_SURFACEBUSY:
      error = "Surface busy";
      break;
    case DDERR_SURFACELOST:
      error = "Surface was lost";
      break;
    case DDERR_WASSTILLDRAWING:
      error = "DirectDraw is still drawing";
      break;
    case DDERR_INVALIDSURFACETYPE:
      error = "Invalid surface type";
      break;
    case DDERR_NOEXCLUSIVEMODE:
      error = "Not in exclusive access mode";
      break;
    case DDERR_NOPALETTEATTACHED:
      error = "No palette attached";
      break;
    case DDERR_NOPALETTEHW:
      error = "No palette hardware";
      break;
    case DDERR_NOT8BITCOLOR:
      error = "Not 8-bit color";
      break;
    case DDERR_EXCLUSIVEMODEALREADYSET:
      error = "Exclusive mode was already set";
      break;
    case DDERR_HWNDALREADYSET:
      error = "Window handle already set";
      break;
    case DDERR_HWNDSUBCLASSED:
      error = "Window handle is subclassed";
      break;
    case DDERR_NOBLTHW:
      error = "No blit hardware";
      break;
    case DDERR_IMPLICITLYCREATED:
      error = "Surface was implicitly created";
      break;
    case DDERR_INCOMPATIBLEPRIMARY:
      error = "Incompatible primary surface";
      break;
    case DDERR_NOCOOPERATIVELEVELSET:
      error = "No cooperative level set";
      break;
    case DDERR_NODIRECTDRAWHW:
      error = "No DirectDraw hardware";
      break;
    case DDERR_NOEMULATION:
      error = "No emulation available";
      break;
    case DDERR_NOFLIPHW:
      error = "No flip hardware";
      break;
    case DDERR_NOTFLIPPABLE:
      error = "Surface not flippable";
      break;
    case DDERR_PRIMARYSURFACEALREADYEXISTS:
      error = "Primary surface already exists";
      break;
    case DDERR_UNSUPPORTEDMODE:
      error = "Unsupported mode";
      break;
    case DDERR_WRONGMODE:
      error = "Surface created in different mode";
      break;
    case DDERR_UNSUPPORTED:
      error = "Operation not supported";
      break;
    case E_NOINTERFACE:
      error = "Interface not present";
      break;
    default:
      snprintf (errormsg, sizeof (errormsg), "%u", code);
      return (errormsg);
  }

  snprintf (errormsg, sizeof(errormsg), "%s (%u)", error, code);
  return (errormsg);
}
