
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
 *                         codec-description.h  -  description
 *                         ------------------------------------------
 *   begin                : written in January 2008 by Damien Sandras 
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of the interface of a codec description.
 *
 */

#ifndef __CODEC_DESCRIPTION_H__
#define __CODEC_DESCRIPTION_H__

#include <iostream>
#include <list>

#include <glib.h>

namespace Ekiga
{

/**
 * @addtogroup calls
 * @{
 */

  /** This class holds the representation of a codec. 
   * That representation is different from the codec itself, but can be used
   * at several places in the engine.
   */
  class CodecDescription 
    {
  public:

      /** Create an empty codec description
      */
      CodecDescription ();

      /** Create a codec description based on the parameters
       * @param name is the codec name as defined in the RFC
       * @param rate is the clock rate 
       * @param audio is true if it reprensents an audio codec
       * @param protocols is a comma separated list of protocols supported 
       *        by the codec
       * @param active is true if the codec is active
       */
      CodecDescription (std::string name,
                        unsigned rate,
                        bool audio,
                        std::string protocols,
                        bool active);

      /** Create a codec description from a string
       * @param codec is a string representing the codec description.
       *        All fields are separated by a *
       */
      CodecDescription (std::string codec);

      virtual ~CodecDescription ()
      {}

      /** Return the codec description under the form of a string.
       * @return the std::string representing the string description.
       */
      std::string str ();


      /** name is the codec name as defined in the RFC
      */
      std::string name;

      /** rate is the clock rate 
      */
      unsigned rate;

      /** active is true if the codec is active
      */
      bool active;

      /** audio is true if it reprensents an audio codec
      */
      bool audio;

      /** protocols is a list of protocols supported by the codec
      */
      std::list<std::string> protocols;


      /** Return true if both CodecDescription are identical, false otherwise
       * @return true if both CodecDescription are identical, false otherwise
       */
      bool operator== (const CodecDescription & c) const;

      /** Return true if both CodecDescription are different, false otherwise
       * @return true if both CodecDescription are different, false otherwise
       */
      bool operator!= (const CodecDescription & c) const;
    };


  class CodecList
    {
  public :

      typedef std::list<CodecDescription> container_type;
      typedef container_type::iterator iterator;
      typedef container_type::const_iterator const_iterator;

      /** Constructor that creates an empty CodecList
       */
      CodecList () {};

      /** Constructor that creates a CodecList from a GSList whose elements
       * are CodecDescription objects formatted as a string.
       */
      CodecList (GSList *);

      virtual ~CodecList ()
      {}

      /** Iterators to loop on the list
       *
       */
      iterator begin ();
      const_iterator begin () const;
      iterator end ();
      const_iterator end () const;

      /** Append the given CodecList at the end of the current CodecList.
       * @param other is the CodecList to append to the current one
       */
      void append (CodecList& other);

      /** Append the given codec description to the current CodecList.
       * @param descr is the CodecDescription to append to the current list
       */
      void append (CodecDescription& descr);

      /** Remove the codec description pointed to by the iterator
       * @param iter is the iterator
       */
      void remove (iterator it);

      /** Return the list of audio codecs descriptions in the current CodecList 
       * @return the list of audio CodecDescription
       */
      CodecList get_audio_list ();


      /** Return the list of video codecs descriptions in the current CodecList 
       * @return the list of video CodecDescription
       */
      CodecList get_video_list ();


      /** Return the list of codecs descriptions under their str form
       * @return the list of CodecDescription
       */
      GSList *gslist ();


      /** Return true if both CodecList are identical, false otherwise
       * @return true if both CodecList are identical, false otherwise
       */
      bool operator== (const CodecList & c) const;


      /** Return true if both CodecList are different, false otherwise
       * @return true if both CodecList are different, false otherwise
       */
      bool operator!= (const CodecList & c) const;

    private:
      container_type codecs;
    };

/**
 * @}
 */

}

/** Output the CodecList
 */
std::ostream& operator<< (std::ostream & os, const Ekiga::CodecList & c);
#endif
