
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
 *                         hal-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Matthias Schneider
 *   copyright            : (c) 2008 by Matthias Schneider
 *   description          : declaration of the interface of a hal core.
 *                          A hal core manages a HalManager.
 *
 */

#include <iostream>
#include <sstream>

#include "config.h"

#include "hal-core.h"
#include "hal-manager.h"


using namespace Ekiga;

HalCore::HalCore ()
{
}

HalCore::~HalCore ()
{
}


void HalCore::add_manager (HalManager &manager)
{
  managers.insert (&manager);
  manager_added.emit (manager);

  manager.videoinput_device_added.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_videoinput_device_added), &manager));
  manager.videoinput_device_removed.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_videoinput_device_removed), &manager));

  manager.audioinput_device_added.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_audioinput_device_added), &manager));
  manager.audioinput_device_removed.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_audioinput_device_removed), &manager));

  manager.audiooutput_device_added.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_audiooutput_device_added), &manager));
  manager.audiooutput_device_removed.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_audiooutput_device_removed), &manager));

  manager.network_interface_up.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_network_interface_up), &manager));
  manager.network_interface_down.connect (sigc::bind (sigc::mem_fun (this, &HalCore::on_network_interface_down), &manager));
}


void HalCore::visit_managers (sigc::slot1<bool, HalManager &> visitor)
{
  bool go_on = true;

  for (std::set<HalManager *>::iterator iter = managers.begin ();
       iter != managers.end () && go_on;
       iter++)
      go_on = visitor (*(*iter));
}

void HalCore::on_videoinput_device_added (std::string source, std::string device, unsigned capabilities, HalManager* manager) {
  videoinput_device_added.emit (source, device, capabilities, manager);
}

void HalCore::on_videoinput_device_removed (std::string source, std::string device, unsigned capabilities, HalManager* manager) {
  videoinput_device_removed.emit (source, device, capabilities, manager);
}

void HalCore::on_audioinput_device_added (std::string source, std::string device, HalManager* manager) {
  audioinput_device_added.emit (source, device, manager);
}

void HalCore::on_audioinput_device_removed (std::string source, std::string device, HalManager* manager) {
  audioinput_device_removed.emit (source, device, manager);
}

void HalCore::on_audiooutput_device_added (std::string sink, std::string device, HalManager* manager) {
  audiooutput_device_added.emit (sink, device, manager);
}

void HalCore::on_audiooutput_device_removed (std::string sink, std::string device, HalManager* manager) {
  audiooutput_device_removed.emit (sink, device, manager);
}

void HalCore::on_network_interface_up (std::string interface_name, std::string ip4_address, HalManager* manager) {
  network_interface_up.emit (interface_name, ip4_address, manager);
}

void HalCore::on_network_interface_down (std::string interface_name, std::string ip4_address, HalManager* manager) {
  network_interface_down.emit (interface_name,ip4_address, manager);
}
