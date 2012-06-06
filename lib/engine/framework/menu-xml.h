
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
 *                         menu-xml.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an XML-based menu
 *
 */

#ifndef __MENU_XML_H__
#define __MENU_XML_H__

#include <libxml/tree.h>

#include "services.h"
#include "menu-builder.h"

/* FIXME: write a proper XML schema
 * Here is how what a toolbar description looks like :
 * <toolbar>
 *   <item type="...">...</item>
 *   <separator/>
 *   <item.../>
 * </toolbar>
 *
 * where item generally looks like :
 * <item type="...">
 *  <label>Short description</label>
 *  <icon>icon-id</icon>
 *  <command>run_this</command>
 * </item>
 *
 * and where the type can be :
 * - 'external' : the command will be run as a shell command
 * - 'internal' : the command will be interpreted as an Ekiga::Trigger to get
 *                (if the trigger isn't available, the item will be dropped)
 */

namespace Ekiga
{

/**
 * @addtogroup actions
 * @{
 */

  class MenuXML
  {
  public:

    MenuXML (ServiceCore &core,
		const std::string filename);

    ~MenuXML ();

    void populate (MenuBuilder &visitor);

  private:

    ServiceCore &core;
    xmlDocPtr doc;
  };

/**
 * @}
 */

};

#endif
