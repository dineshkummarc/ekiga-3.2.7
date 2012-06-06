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
 *                         device-def.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of an object representing a device
 *
 */

#ifndef __DEVICE_DEFINITION_H__
#define __DEVICE_DEFINITION_H__

#include <string>
#include <iostream>

namespace Ekiga
{
  /**
   * @addtogroup services
   * @{
   */
  class Device
  {
  public:
    /** The backend type. 
     *  The backend type like "PTLIB", "NULL", etc.
     */
    std::string type;
    
    /** The device source. 
     *  The device source like "V4L", "ALSA", "DC", "OSS", etc.
     */
    std::string source;

    /** The device name. 
     *  The device name like "Logitech Quickcam Zoom"
     */
    std::string name;
    
    /** Set the device identifier from a consendes string.
     * Set the device identifier from a condensed string like "type/source/name"
     * Used for setting devices from the saved gmconf configuration.
     * @param str the string from which to set the device identifier.
     */
    void SetFromString(std::string str)
    {
      unsigned type_sep = str.find_last_of("(");
      unsigned source_sep = str.find_first_of("/", type_sep + 1);

      name   = str.substr ( 0, type_sep - 1);
      type   = str.substr ( type_sep + 1, source_sep - type_sep - 1);
      source = str.substr ( source_sep + 1,  str.size() - source_sep - 2);
    }

    /** Get a condensed string of the device identifier.
     * Return a condesned string in the form "type/source/name".
     * Used when storing the device identifier in gmconf.
     * @return the string in the format "type/source/name".
     */
    std::string GetString() const
    {
      return ( name + " (" + type + "/" + source + ")");
    }
    
    Device & operator= (const Device & rhs)
    {
      type   = rhs.type;
      source = rhs.source;
      name   = rhs.name;
      return *this;
    } 

    bool operator==( const Device & rhs ) const
    {
     if ( (type   == rhs.type)   && 
          (source == rhs.source) &&
          (name   == rhs.name)   )
       return true;
     else
       return false;
    }

    bool operator!=( const Device & rhs ) const 
    {
      return (!(*this==rhs));
    }

    friend std::ostream& operator <<(std::ostream & stream, const Device & device){
      stream << device.GetString();
      return stream;
    }
  };
  /**
   * @}
   */

};  
#endif
