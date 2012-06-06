
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
 *                         hal-manager-mlogo.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : Declaration of the interface of a hal core.
 *                          A hal core manages HalManagers.
 *
 */


#ifndef __HAL_MANAGER_DBUS_H__
#define __HAL_MANAGER_DBUS_H__

#include "hal-core.h"
#include "hal-manager.h"
#include "runtime.h"

extern "C" {
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <glib-object.h>
}

#include <vector>

/**
 * @addtogroup hal
 * @{
 */
  typedef struct HalDevice {
    std::string key;
    std::string category;
    std::string name;
    std::string type;
    unsigned video_capabilities;
  } HalDevice;

  typedef struct NmInterface {
    std::string key;
    std::string name;
    std::string ip4_address;
    bool active;
  } NmInterface;

  class HalManager_dbus
   : public Ekiga::HalManager
    {
  public:

      /* The constructor
       */
      HalManager_dbus (Ekiga::ServiceCore & core);
      /* The destructor
       */
      ~HalManager_dbus ();


      /*                 
       * DISPLAY MANAGEMENT 
       */               

      /** Create a call based on the remote uri given as parameter
       * @param uri  an uri
       * @return     true if a Ekiga::Call could be created
       */
      static void device_added_cb_proxy (DBusGProxy *object, const char *device, gpointer user_data);
      static void device_removed_cb_proxy (DBusGProxy *object, const char *device, gpointer user_data);

      static void interface_no_longer_active_cb_proxy (DBusGProxy *object, const char *interface, gpointer user_data);
      static void interface_now_active_cb_proxy (DBusGProxy *object, const char *interface, gpointer user_data);
      static void interface_ip4_address_change_cb_proxy (DBusGProxy *object, const char *interface, gpointer user_data);

      void device_added_cb (const char *device);
      void device_removed_cb (const char *device);

      void interface_now_active_cb (const char *interface);
      void interface_no_longer_active_cb (const char *interface);
      void interface_ip4_address_change_cb (const char *interface);

  protected:  
      void populate_devices_list();
      void populate_interfaces_list ();

      bool get_device_type_name (const char * device, HalDevice & hal_device);
      void get_interface_name_ip (const char * interface, NmInterface & nm_interface);

      void get_string_property (DBusGProxy *proxy, const char * property, std::string & value);

      Ekiga::ServiceCore & core;

      DBusGConnection * bus;
      DBusGProxy * hal_proxy;
      DBusGProxy * nm_proxy;

      std::vector <HalDevice> hal_devices;
      std::vector <NmInterface> nm_interfaces;

  };
/**
 * @}
 */


#endif
