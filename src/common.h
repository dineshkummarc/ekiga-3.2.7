
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
 *                         common.h  -  description
 *                         ------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains things common to the whole soft.
 *
 */


#ifndef GM_COMMON_H_
#define GM_COMMON_H_

#include <opal/buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>

#include <opal/manager.h>
#include <opal/pcss.h>

#include <h323/h323.h>
#include <sip/sip.h>

#include <glib.h>
#include <gtk/gtk.h>

#define GENERAL_KEY         "/apps/" PACKAGE_NAME "/general/"
#define USER_INTERFACE_KEY "/apps/" PACKAGE_NAME "/general/user_interface/"
#define CONTACTS_KEY "/apps/" PACKAGE_NAME "/contacts/"
#define VIDEO_DISPLAY_KEY USER_INTERFACE_KEY "video_display/"
#define SOUND_EVENTS_KEY  "/apps/" PACKAGE_NAME "/general/sound_events/"
#define AUDIO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/audio/"
#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"
#define PERSONAL_DATA_KEY "/apps/" PACKAGE_NAME "/general/personal_data/"
#define CALL_OPTIONS_KEY "/apps/" PACKAGE_NAME "/general/call_options/"
#define NAT_KEY "/apps/" PACKAGE_NAME "/general/nat/"
#define PROTOCOLS_KEY "/apps/" PACKAGE_NAME "/protocols/"
#define H323_KEY "/apps/" PACKAGE_NAME "/protocols/h323/"
#define SIP_KEY "/apps/" PACKAGE_NAME "/protocols/sip/"
#define PORTS_KEY "/apps/" PACKAGE_NAME "/protocols/ports/"
#define CALL_FORWARDING_KEY "/apps/" PACKAGE_NAME "/protocols/call_forwarding/"
#define LDAP_KEY "/apps/" PACKAGE_NAME "/protocols/ldap/"
#define CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/"
#define AUDIO_CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/audio/"
#define VIDEO_CODECS_KEY  "/apps/" PACKAGE_NAME "/codecs/video/"

#define GM_FRAME_SIZE  10

#define GNOMEMEETING_PAD_SMALL 1

/* Contact state */
typedef enum {

  CONTACT_ONLINE,
  CONTACT_AWAY,
  CONTACT_DND,
  CONTACT_INVISIBLE,
  CONTACT_OFFLINE,
  CONTACT_UNKNOWN,
  CONTACT_LAST_STATE
} ContactState;

/* Incoming Call Mode */
typedef enum {

  AVAILABLE,
  AUTO_ANSWER,
  DO_NOT_DISTURB,
  FORWARD,
  NUM_MODES
} IncomingCallMode;


/* Control Panel Section */
typedef enum {

  CONTACTS,
  DIALPAD,
  CALL,
  NUM_SECTIONS
} PanelSection;


#endif /* GM_COMMON_H */
