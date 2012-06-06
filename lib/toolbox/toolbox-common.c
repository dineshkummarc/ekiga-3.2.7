
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
 *                         toolbox-common.c  -  description 
 *                         ------------------------------------------
 *   begin                : Jan 2006
 *   copyright            : (C) by various authors (see function code)
 *   description          : Various helper functions -- general
 */

#include "toolbox.h"

#include <string.h>

GSList
*gm_string_gslist_remove_dups (GSList *origlist)
{
   /* from a GSList* of gchar*, remove all dup strings
   * (C) Jan Schampera <jan.schampera@web.de> */
  GSList *origlist_iter = NULL;
  GSList *seenlist = NULL;
  GSList *seenlist_iter = NULL;
  gboolean seen = FALSE;

  /* iterate through the original list and compare every stored gchar* to
   * our "seen list", if not there, append it */
  if (!origlist) return NULL;

  for (origlist_iter = origlist;
       origlist_iter != NULL;
       origlist_iter = g_slist_next (origlist_iter))
    {
      if (origlist_iter->data)
	{
	  seen = FALSE;
	  /* check if the string is already in the "seen list" */
	  for (seenlist_iter = seenlist;
	       seenlist_iter != NULL;
	       seenlist_iter = g_slist_next (seenlist_iter))
	    {
	      if (seenlist_iter->data &&
		  !strcmp ((const char*) origlist_iter->data,
			   (const char*) seenlist_iter->data))
		{
		  seen = TRUE;
		}
	    }
	  if (!seen)
	    {
	      /* not in list? append it... */
	      seenlist = g_slist_append (seenlist,
					 (gpointer) g_strdup
					 ((const gchar*) origlist_iter->data));
	    }
	}
    }

  /* free the memory of the original list */
  g_slist_foreach (origlist, (GFunc) g_free, NULL);
  g_slist_free (origlist);

  return seenlist;
}
