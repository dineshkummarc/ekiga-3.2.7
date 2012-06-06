
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
 *                         platform-win32.c  -  description
 *                         ------------------------------------------
 *   begin                : Nov 2006
 *   copyright            : (C) 2006-2007 by Julien Puydt
 *   description          : Implementation of platform-specific workarounds
 */

#include "gm-platform.h"
#include "winpaths.h"

/* Yes, static variables should be avoided -- but we will need those paths
 * during all application lifetime!
 */

static gchar *basedir = NULL;
static gchar *sysconfdir = NULL;
static gchar *datadir = NULL;
static gchar *plugindir = NULL;

void
gm_platform_init ()
{
#if GLIB_CHECK_VERSION (2, 18, 0)
  basedir = g_strdup (g_win32_get_package_installation_directory_of_module (NULL));
#else  
  basedir = g_strdup (g_win32_get_package_installation_directory (NULL, NULL));
#endif
  sysconfdir = g_strdup (basedir);
  datadir = g_strdup (basedir);
  plugindir = g_strdup_printf ("%s/lib/contacts", basedir);
}

void gm_platform_shutdown ()
{
  g_free (basedir);
  g_free (sysconfdir);
  g_free (datadir);
  g_free (plugindir);
}

const gchar *
win32_sysconfdir ()
{
  return sysconfdir;
}

const gchar *
win32_datadir ()
{
  return datadir;
}

const gchar *
win32_plugindir ()
{
  return plugindir;
}
