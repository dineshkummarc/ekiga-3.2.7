
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
 *                         DXWindow.h  -  description
 *                         --------------------------
 *   begin                : Sun April 15 2007
 *   copyright            : (C) 2007 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering DirectDraw hardware 
 *                          acceleration.
 */

#ifndef DXWINDOW_H
#define DXWINDOW_H

#include "ddraw.h"
#include <windows.h>
#include <stdint.h>

//	LPCWSTR
#define CHAR_W_P const char*

struct DXSurface {
  LPDIRECTDRAWSURFACE primary;
  LPDIRECTDRAWSURFACE overlay;
  LPDIRECTDRAWSURFACE back; 
  LPDIRECTDRAWCLIPPER clipper;
  RECT mainSrc;    // corresponds to the resolution of the main image
  RECT pipSrc;     // corresponds to the resolution of the pip image

  RECT mainBack;   // corresponds to the scaled main image on the backbuffer (at 0,0)
  RECT pipBack;    // corresponds to the scaled pip image on the backbuffer (at 0,0)

  RECT primaryDst; // corresponds to the scaled main image on the primary surface (and thus the screen)

  RECT embeddedRelative;
};

typedef struct {
  bool fullscreen;
  bool ontop;
  bool decoration;
  RECT oldWinFS;
  RECT oldWinD;
} DXState;


/**
 * DXWindow:
 *
 * This class provides DirectDraw support under Windows if it is supported by the graphics hardware and driver.
 * DirectDraw makes use of hardware capabilities in order to do
 * - colorspace transformation
 * - scaling
 * - anti-aliasing
 *
 * This class features a fullscreen mode, an always-on-top mode and allows to enable and disable the window
 * manager decorations. A picture-in-picture functionality is provided. The class has to initialized
 * with the window where it shall appear and the original image and intial window size (if a PIP is desired also 
 * the PIP image size and factor has to be provided. After having been initialized successfully a frame 
 * is passed via putFrame which takes care of the presentation.
 *
 * @author Matthias Schneider
 */
class DXWindow 
{
public:
    DXWindow ();
    ~DXWindow ();

    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Initialize DirectDraw. In case rootWindow is NULL,
     *                 a new window will be created.
     * PRE          :  x and y positions of the window / area  (>=0),
     *                 width and height of the window / area (>0),
     *                 width and height of the frame that is to be displayed 
     *                 in that window / area
     */
    bool Init (HWND rootWindow, 
               int x, 
               int y,
               int windowWidth, 
               int windowHeight, 
               int imageWidth, 
               int imageHeight);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Initialize DirectDraw. In case rootWindow is NULL,
     *                 a new window will be created. Enable picture-in-picture
     * PRE          :  x and y positions of the window / area  (>=0),
     *                 width and height of the window / area (>0),
     *                 width and height of the frame that is to be displayed 
     *                 in that window / area
     *                 width and height of the frame that is to be displayed 
     *                 as a picture-in-picture in that window / area
     */
    bool Init (HWND rootWindow, 
               int x,
               int y,
               int windowWidth, 
               int windowHeight, 
               int imageWidth, 
               int imageHeight, 
               int PIPimageWidth, 
               int PIPimageHeight);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Display a new frame
     * PRE          :  pointer to the frame data
     *                 width and height of the frame
     *                 changing the framesize after having been 
     *                 initialized is not supported
     */
    void PutFrame (uint8_t *frame,
                   uint16_t width, 
                   uint16_t height,
		   bool pip);


    void ProcessEvents ();
    void Sync ();
    void ToggleOntop ();
    void ToggleFullscreen ();
    void ToggleDecoration ();


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Set window coordinates and size
     * PRE          :  pointer to the main frame data
     *                 width and height of the main frame
     */
    void SetWindow (int x, 
                    int y, 
                    unsigned int windowWidth, 
                    unsigned int windowHeight);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Get current window coordinates and size
     * PRE          :  /
     */
    void GetWindow (int *x, 
                    int *y,
                    unsigned int *windowWidth, 
                    unsigned int *windowHeight);

    bool IsFullScreen ()  const 
      { 
        return _state.fullscreen; 
      }

    bool HasDecoration () const 
      { 
        return _state.decoration; 
      }

    bool IsOntop () const 
      { 
        return _state.ontop;      
      }

    bool HasPIP () const 
      { 
        return _pip;              
      }

private:

    HWND _DXWindow;
    LPDIRECTDRAW _DDraw;	
    DXSurface _DXSurface;
    DXState _state;
    HINSTANCE _instance;
    CHAR_W_P _windowTitle;
    bool _embedded;
    bool _pip;
    bool _sizemove;


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Static temporary message handler
     *                 Extracts the GWLP_USERDATA data from the window and
     *                 calls the correct "real" message handler in the DXWindow class
     * PRE          :
     */
    static LRESULT CALLBACK WndProc (HWND window,
                                     UINT message,
                                     WPARAM wParam, 
                                     LPARAM lParam);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  The real message handler for keys pressed and mouse clicks
     * PRE          :
     */
    LRESULT HandleMessage (HWND window,
                           UINT message,
                           WPARAM wParam,
                           LPARAM lParam);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Copies a frame to the shared memory of the backbuffer
     * PRE          :  pointer to the frame data,
     *                 width and height of the frame,
     *                 source and destination rectangles
     */
    void CopyFrameBackbuffer (uint8_t *mainFrame, 
                              uint16_t width, 
                              uint16_t height,
                              RECT *src, 
                              RECT *dst);


    void CalculateEmbWindCoord ();

    void CalculateBackBuffer ();


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Takes care that the window does not leave the screen.
     Then calls SetOverlayRect with the window coordiates and size 
     * PRE          :
     */
    void SetOverlayToWindow ();


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Sets the destination Rectangle Position and sizes
     *                 Takes into the aspect ratio of the image and the 
     *                 destination and adjusts the destination rectangle 
     *                 accordingly. Also calculates the position and size of the picture-in-picture
     * PRE          :
     */
    void CorrectAspectRatio (RECT uncorrected);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Create a new window
     * PRE          :  x and y positions of the window   (>=0),
     *                 width and height of the window (>0),
     */
    bool NewWindow (int x, 
                    int y, 
                    int windowWidth,
                    int windowHeight);


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Converts Windows error codes to text
     * PRE          :
     */
    const char *ErrorMessage ();


    /* DESCRIPTION  :  /
     * BEHAVIOR     :  Converts DirectDraw error codes to text
     * PRE          :
     */
    const char *DDErrorMessage (int code);
};

#endif	//DXWINDOW_H
