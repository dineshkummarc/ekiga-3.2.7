
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
 *                         hal-v4l-helper.c  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Used to detect the names and supported V4L 
 *                          interfaces of a new video4linux device (this 
 *                          should really be in HALd itself.
 *
 */

#include <fcntl.h>	// for open() 
#include <unistd.h>	// for read(), write(), close() 
#include <sys/ioctl.h>	// for ioctl()
#include <stdlib.h>     // for free(), malloc()
#include <string.h>     // for strlen()
#include <stdio.h>
#include <linux/videodev.h>

int v4l_get_device_names (const char* device, char** v4l1_name, char** v4l2_name) {
  int fp;
  unsigned ret = 0; // Device not valid
  
  *v4l1_name = NULL;
  *v4l2_name = NULL;

  if((fp = open(device, O_RDONLY)) == 0) {
    return -1;  // Unable to open device
  }
 
  struct video_capability  v4l1_caps;
  if (ioctl(fp, VIDIOCGCAP, &v4l1_caps) >= 0 && (v4l1_caps.type & VID_TYPE_CAPTURE) != 0) {
    ret |= 1;
    *v4l1_name = (char*) malloc (strlen(v4l1_caps.name) + 1);
    strcpy (*v4l1_name, v4l1_caps.name);
  }

  struct v4l2_capability v4l2_caps;
  if ( (ioctl(fp, VIDIOC_QUERYCAP, &v4l2_caps) >= 0) &&
       (v4l2_caps.capabilities & V4L2_CAP_VIDEO_CAPTURE) ) {
    ret |= 2;
    *v4l2_name = (char*) malloc (strlen((const char*) v4l2_caps.card) + 1);
    strcpy (*v4l2_name, (const char*) v4l2_caps.card);
  }

  if (fp > 0)
    close(fp);

  return ret;
}

void v4l_free_device_name (char** name)
{
  if (*name)
    free (*name);

  *name = NULL;
}

