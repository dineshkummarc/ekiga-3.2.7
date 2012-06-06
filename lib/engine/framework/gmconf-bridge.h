
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
 *                         gmconf-bridge.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of an object able to do the bridging
 *                          between gmconf and any object
 *
 */

#ifndef __GMCONF_BRIDGE_H__
#define __GMCONF_BRIDGE_H__

#include <vector>
#include <sigc++/sigc++.h>

#include "gmconf.h"
#include "services.h"

namespace Ekiga
{
  /**
   * This is a vector of string corresponding to the GMConf keys
   */
  typedef std::vector<std::string> ConfKeys;


  /** 
   * This class implements a bridge between an Ekiga::Service and its
   * configuration stored in GMConf.
   *
   * This class has several purposes :
   * - abstract the configuration stuff from the engine classes
   * - keep the GMConf idea of notifiers triggered when the value associated
   *   with a key changes, but instead of reacting to it in the object, 
   *   directly, update it using its public API
   *
   * This could be replaced at some point by a C++ implementation of GMConf,
   * but for now it is a good abstraction layer : The service exposes its API
   * but does not care about storing the configuration. GMConf stores the 
   * configuration (the model), and the bridge updates the Ekiga::Service (the
   * view), when a controller updates the configuration (the prefs window or
   * an external program).
   *
   * It should only monitor keys representing properties of the Ekiga::Service.
   */

  class ConfBridge
    {
  public:

      /** The constructor
       * @param _service is the Ekiga::Service the bridge will update
       */
      ConfBridge (Ekiga::Service & _service) : service (_service) {};


      /** Load notifiers for the vector of keys given as argument
       * @param keys is a vector of keys for which the ConfBridge will trigger 
       * GmConf notifers. They are triggered when the value associated with the
       * key changes, or when this method is called.
       */
      void load (ConfKeys & keys);


      /** This signal is emitted when a notifier is triggered for a key
       * @param key is the GmConf key whose value changed
       * @param entry is the new GmConf entry
       */
      sigc::signal2<void, std::string /*key*/, GmConfEntry * /*entry*/> property_changed;

  protected :
      Ekiga::Service & service;
    };
};
#endif
