
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
 *                         opal-codec-description.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in January 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : Opal codec description.
 *
 */

#include <algorithm>
#include <iostream>
#include <sstream>

#include "opal-codec-description.h"


using namespace Opal;


static bool
same_codec_desc (Ekiga::CodecDescription a, Ekiga::CodecDescription b)
{
  return (a.name == b.name && a.rate == b.rate);
}


CodecDescription::CodecDescription (OpalMediaFormat & format)
  : Ekiga::CodecDescription ()
{
  name = (const char *) format.GetEncodingName ();
  if (name == "G722")
    rate = 16000;
  else
    rate = format.GetClockRate ();
  audio = (format.GetMediaType () == OpalMediaType::Audio ());
  if (format.IsValidForProtocol ("SIP"))
    protocols.push_back ("SIP");
  if (format.IsValidForProtocol ("H.323"))
    protocols.push_back ("H.323");
  protocols.sort ();
}


CodecList::CodecList (OpalMediaFormatList & list)
{
  for (PINDEX i = 0 ; i < list.GetSize () ; i++) {

    if (list [i].IsTransportable ()) {

      Ekiga::CodecDescription desc = Opal::CodecDescription (list [i]);

      if (desc.name.empty ())
        continue;

      Ekiga::CodecList::iterator it =
        search_n (begin (), end (), 1, desc, same_codec_desc);
      if (it == end ())
        append (desc);
      else {
        it->protocols.sort ();
        it->protocols.merge (desc.protocols);
        it->protocols.unique ();
      }
    }
  }
}
