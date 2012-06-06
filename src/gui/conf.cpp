
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
 *                         config.cpp  -  description
 *                         --------------------------
 *   begin                : Wed Feb 14 2001
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : This file contains most of config stuff.
 *                          All notifiers are here.
 *                          Callbacks that updates the config cache 
 *                          are in their file, except some generic one that
 *                          are in this file.
 *   Additional code      : Miguel Rodríguez Pérez  <miguelrp@gmail.com>
 *
 */


#include "config.h"

#include "conf.h"

#include "ekiga.h"
#include "preferences.h"
#include "assistant.h"
#include "accounts.h"
#include "main.h"
#include "misc.h"

#include "gmdialog.h"
#include "gmstockicons.h"
#include "gmmenuaddon.h"
#include "gmconfwidgets.h"



/* Declarations */




/* The functions */
void 
gnomemeeting_conf_upgrade ()
{
  gchar *conf_url = NULL;

  int version = 0;

  version = gm_conf_get_int (GENERAL_KEY "version");
  
  /* Install the sip:, h323: and callto: GNOME URL Handlers */
  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/callto/command");
					       
  if (!conf_url
      || !strcmp (conf_url, "gnomemeeting -c \"%s\"")) {

    
    gm_conf_set_string ("/desktop/gnome/url-handlers/callto/command", 
			"ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/needs_terminal", 
		      false);
    
    gm_conf_set_bool ("/desktop/gnome/url-handlers/callto/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/h323/command");
  if (!conf_url 
      || !strcmp (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/h323/command", 
                        "ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/needs_terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/h323/enabled", true);
  }
  g_free (conf_url);

  conf_url = gm_conf_get_string ("/desktop/gnome/url-handlers/sip/command");
  if (!conf_url 
      || !strcmp (conf_url, "gnomemeeting -c \"%s\"")) {

    gm_conf_set_string ("/desktop/gnome/url-handlers/sip/command", 
                        "ekiga -c \"%s\"");

    gm_conf_set_bool ("/desktop/gnome/url-handlers/sip/needs_terminal", false);

    gm_conf_set_bool ("/desktop/gnome/url-handlers/sip/enabled", true);
  }
  g_free (conf_url);

  /* New full name key */
  conf_url = gm_conf_get_string (PERSONAL_DATA_KEY "full_name");
  if (!conf_url || (conf_url && !strcmp (conf_url, ""))) {

    gchar *fullname = NULL;
    gchar *firstname = gm_conf_get_string (PERSONAL_DATA_KEY "firstname");
    gchar *lastname = gm_conf_get_string (PERSONAL_DATA_KEY "lastname");

    if (firstname && lastname && strcmp (firstname, "") && strcmp (lastname, "")) {
      fullname = g_strdup_printf ("%s %s", firstname, lastname);
      gm_conf_set_string (PERSONAL_DATA_KEY "firstname", "");
      gm_conf_set_string (PERSONAL_DATA_KEY "lastname", "");
      gm_conf_set_string (PERSONAL_DATA_KEY "full_name", fullname);
      g_free (fullname);
    }
    g_free (firstname);
    g_free (lastname);
  }
  g_free (conf_url);

  /* diamondcard is now set at sip.diamondcard.us */
  GSList *accounts = gm_conf_get_string_list ("/apps/" PACKAGE_NAME "/protocols/accounts_list");
  GSList *accounts_iter = accounts;
  while (accounts_iter) {

    PString acct = (gchar *) accounts_iter->data;
    acct.Replace ("eugw.ast.diamondcard.us", "sip.diamondcard.us", TRUE);
    g_free (accounts_iter->data);
    accounts_iter->data = g_strdup ((const char *) acct);
    accounts_iter = g_slist_next (accounts_iter);
  }
  gm_conf_set_string_list ("/apps/" PACKAGE_NAME "/protocols/accounts_list", accounts);
  g_slist_foreach (accounts, (GFunc) g_free, NULL);
  g_slist_free (accounts);

  /* Audio devices */
  gchar *plugin = NULL;
  gchar *device = NULL;
  gchar *new_device = NULL;
  plugin = gm_conf_get_string (AUDIO_DEVICES_KEY "plugin");
  if (plugin && strcmp (plugin, "")) {
    device = gm_conf_get_string (AUDIO_DEVICES_KEY "input_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (AUDIO_DEVICES_KEY "plugin", "");
    gm_conf_set_string (AUDIO_DEVICES_KEY "input_device", new_device);
    g_free (device);
    g_free (new_device);

    device = gm_conf_get_string (AUDIO_DEVICES_KEY "output_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (AUDIO_DEVICES_KEY "plugin", "");
    gm_conf_set_string (AUDIO_DEVICES_KEY "output_device", new_device);
    g_free (device);
    g_free (new_device);

    device = gm_conf_get_string (SOUND_EVENTS_KEY "output_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (SOUND_EVENTS_KEY "plugin", "");
    gm_conf_set_string (SOUND_EVENTS_KEY "output_device", new_device);
    g_free (device);
    g_free (new_device);
  }
  g_free (plugin);

  /* Video devices */
  plugin = gm_conf_get_string (VIDEO_DEVICES_KEY "plugin");
  if (plugin && strcmp (plugin, "")) {
    device = gm_conf_get_string (VIDEO_DEVICES_KEY "input_device");
    new_device = g_strdup_printf ("%s (PTLIB/%s)", device, plugin);
    gm_conf_set_string (VIDEO_DEVICES_KEY "plugin", "");
    gm_conf_set_string (VIDEO_DEVICES_KEY "input_device", new_device);
    g_free (device);
    g_free (new_device);
  }
  g_free (plugin);
}
