
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
 *                         toolbox-gtk.c  -  description
 *                         ------------------------------------------
 *   begin                : Dec 2005
 *   copyright            : (C) 2005 by Julien Puydt
 *   description          : Various helper functions -- gtk implementation
 */

#include "toolbox.h"
#include "toolbox-internal.h"
#include <gtk/gtk.h>

void
gm_open_uri (const gchar *uri)
{
  GError *error = NULL;

  g_return_if_fail (uri != NULL);

#if GTK_CHECK_VERSION(2,13,1)
  if (!gtk_show_uri (NULL, uri, GDK_CURRENT_TIME, &error)) {
    g_error_free (error);
    gm_open_uri_fallback (uri);
  }
#else
  gm_open_uri_fallback (uri);
#endif
}
