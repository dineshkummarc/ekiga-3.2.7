
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
*                         videooutput-manager-dx.h -  description
 *                         ----------------------------------
 *   begin                : Sun Nov 19 2006
 *   copyright            : (C) 2006-2008 by Matthias Schneider
 *                          (C) 2000-2008 by Damien Sandras
 *   description          : Class to allow video output to a DirectX
 *                          accelerated window
 
 */


#ifndef _VIDEOOUTPUT_MANAGER_DX_H_
#define _VIDEOOUTPUT_MANAGER_DX_H_

#include "videooutput-manager-common.h"
#include "dxwindow.h"

/**
 * @addtogroup videooutput
 * @{
 */

  class GMVideoOutputManager_dx
    : public  GMVideoOutputManager
  {
  public:
    GMVideoOutputManager_dx (Ekiga::ServiceCore & _core);

    virtual ~GMVideoOutputManager_dx ();
  
    virtual bool frame_display_change_needed ();

    virtual void setup_frame_display ();

    virtual void close_frame_display ();

    virtual void display_frame (const char *frame,
                                unsigned width,
                                unsigned height);

    virtual void display_pip_frames (const char *local_frame,
                                  unsigned lf_width,
                                  unsigned lf_height,
                                  const char *remote_frame,
                                  unsigned rf_width,
                                  unsigned rf_height);

  protected:

    virtual void sync(UpdateRequired sync_required);

    DXWindow *dxWindow;

  private:

  void size_changed_in_main (unsigned width,
			     unsigned height);

  void device_opened_in_main (Ekiga::VideoOutputAccel accel,
			      Ekiga::VideoOutputMode mode,
			      unsigned zoom,
			      bool both);

  void device_closed_in_main ();

  void device_error_in_main (Ekiga::VideoOutputErrorCodes code);

  void fullscreen_mode_changed_in_main (Ekiga::VideoOutputFSToggle val);
  };

/**
 * @}
 */

#endif /* _VIDEOOUTPUT_MANAGER_DX_H_ */
