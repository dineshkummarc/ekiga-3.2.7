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
 *                         utils.cpp  -  description
 *                         -----------------------------
 *   begin                : Tue Apr 21 2009
 *   copyright            : (C) 2009 by Eugen Dedu
 *   description          : Functions to change encoding of std::string
 */


#include <glib.h>

#include "utils.h"

const std::string
latin2utf (const std::string str)
{
  gchar *utf8_str;
  std::string result;

#ifdef WIN32
  utf8_str = g_locale_to_utf8 (str.c_str (), -1,
                               NULL, NULL, NULL);
#else
  utf8_str = g_convert (str.c_str (), -1,
                        "UTF-8", "ISO-8859-1",
                        NULL, NULL, NULL);
#endif
  g_warn_if_fail (utf8_str != NULL);
  if (utf8_str == NULL)  /* conversion error */
    return "";
  result = std::string (utf8_str);
  g_free (utf8_str);
  return result;
}


const std::string
utf2latin (const std::string str)
{
  gchar *latin_str;
  std::string result;

  g_warn_if_fail (g_utf8_validate (str.c_str (), -1, NULL));
#ifdef WIN32
  latin_str = g_locale_from_utf8 (str.c_str (), -1,
                                  NULL, NULL, NULL);
#else
  latin_str = g_convert (str.c_str (), -1,
                         "ISO-8859-1", "UTF-8",
                         NULL, NULL, NULL);
#endif
  g_warn_if_fail (latin_str != NULL);
  if (latin_str == NULL)  /* conversion error */
    return "";
  result = std::string (latin_str);
  g_free (latin_str);
  return result;
}
