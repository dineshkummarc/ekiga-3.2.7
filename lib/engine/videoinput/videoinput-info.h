
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
 *                         videoinput-info.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of structs and classes used for communication
 *                          with the VideoInputManagers
 *
 */

#ifndef __VIDEOINPUT_INFO_H__
#define __VIDEOINPUT_INFO_H__

#include "device-def.h"

#define GM_4CIF_WIDTH  704
#define GM_4CIF_HEIGHT 576
#define GM_CIF_WIDTH   352
#define GM_CIF_HEIGHT  288
#define GM_QCIF_WIDTH  176
#define GM_QCIF_HEIGHT 144
#define GM_4SIF_WIDTH  640
#define GM_4SIF_HEIGHT 480
#define GM_SIF_WIDTH   320
#define GM_SIF_HEIGHT  240
#define GM_QSIF_WIDTH  160
#define GM_QSIF_HEIGHT 120

namespace Ekiga
{
#define NB_VIDEO_SIZES 5

  static const struct { 
    int width; 
    int height; 
  } VideoSizes[NB_VIDEO_SIZES] = {
    {  GM_QCIF_WIDTH,  GM_QCIF_HEIGHT },
    {  GM_CIF_WIDTH,   GM_CIF_HEIGHT  },
    {  GM_4CIF_WIDTH,  GM_4CIF_HEIGHT },
    {  GM_SIF_WIDTH,   GM_SIF_HEIGHT  },
    {  GM_4SIF_WIDTH,  GM_4SIF_HEIGHT },
  };

  class VideoInputDevice : public Device {};

  typedef struct VideoInputSettings {
    unsigned whiteness;
    unsigned brightness;
    unsigned colour;
    unsigned contrast;
    bool modifyable;
  } VideoInputSettings;

  enum VideoInputFormat {
    VI_FORMAT_PAL = 0,
    VI_FORMAT_NTSC,
    VI_FORMAT_SECAM,
    VI_FORMAT_Auto,
    VI_FORMAT_MAX
  };

  enum VideoInputErrorCodes {
    VI_ERROR_NONE = 0,
    VI_ERROR_DEVICE,
    VI_ERROR_FORMAT,
    VI_ERROR_CHANNEL,
    VI_ERROR_COLOUR,
    VI_ERROR_FPS,
    VI_ERROR_SCALE
  };
};

#endif
