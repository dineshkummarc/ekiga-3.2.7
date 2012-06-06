
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
 *                         hal-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a hal core.
 *                          A hal core manages hal managers and abstracts 
 *                          their platform specific implementation.
 *
 */

#ifndef __HAL_CORE_H__
#define __HAL_CORE_H__

#include "services.h"

#include <sigc++/sigc++.h>

#include <set>
#include <map>

#define V4L_VERSION_1 (1<<0)
#define V4L_VERSION_2 (1<<1)

namespace Ekiga
{
/**
 * @defgroup hal Hal HAL
 * @{
 */

  class HalManager;

  /** Core object for hal support

      The hal core is used to abstract platform-specific hardware abstraction layer
      managers. It will emit signals when interesting devices and are added to or
      removed from the system or when network devices come up or go down.

      Threads: The callbacks will always be passed back to the main thread,
               independent from the actual manager runs in a separate thread or not.
   */
  class HalCore
    : public Service
    {

  public:

      /* The constructor
      */
      HalCore ();

      /* The destructor
      */
      ~HalCore ();


      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "hal-core"; }

      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tHardware abstraction layer core"; }


      /*** API to list HalManagers ***/

      /** Adds a HalManager to the HalCore service.
       * @param The manager to be added.
       */
       void add_manager (HalManager &manager);

      /** Triggers a callback for all Ekiga::HalManager sources of the
       * HalCore service.
       * @param The callback (the return value means "go on" and allows
       *  stopping the visit)
       */
       void visit_managers (sigc::slot1<bool, HalManager &> visitor);

      /** This signal is emitted when an Ekiga::HalManager has been
       * added to the HalCore Service.
       */
       sigc::signal1<void, HalManager &> manager_added;


      /*** API to act on HAL events ***/

      /** See hal-manager.h for the API
       */
      sigc::signal4<void, const std::string &, const std::string &, unsigned, HalManager*> videoinput_device_added;
      sigc::signal4<void, const std::string &, const std::string &, unsigned, HalManager*> videoinput_device_removed;

      sigc::signal3<void, const std::string &, const std::string &, HalManager*> audioinput_device_added;
      sigc::signal3<void, const std::string &, const std::string &, HalManager*> audioinput_device_removed;

      sigc::signal3<void, const std::string &, const std::string &, HalManager*> audiooutput_device_added;
      sigc::signal3<void, const std::string &, const std::string &, HalManager*> audiooutput_device_removed;

      sigc::signal3<void, const std::string &, const std::string &, HalManager*> network_interface_up;
      sigc::signal3<void, const std::string &, const std::string &, HalManager*> network_interface_down;

  private:

      void on_videoinput_device_added (std::string source, std::string device, unsigned capabilities, HalManager* manager);
      void on_videoinput_device_removed (std::string source, std::string device, unsigned capabilities, HalManager* manager);

      void on_audioinput_device_added (std::string source, std::string device, HalManager* manager);
      void on_audioinput_device_removed (std::string source, std::string device, HalManager* manager);

      void on_audiooutput_device_added (std::string sink, std::string device, HalManager* manager);
      void on_audiooutput_device_removed (std::string sink, std::string device, HalManager* manager);

      void on_network_interface_up (std::string interface_name, std::string ip4_address, HalManager* manager);
      void on_network_interface_down (std::string interface_name, std::string ip4_address, HalManager* manager);

      std::set<HalManager *> managers;

    };
/**
 * @}
 */
};

#endif
