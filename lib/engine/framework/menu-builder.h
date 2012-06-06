
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
 *                         menu-builder.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of a menu builder
 *
 */

#ifndef __MENU_BUILDER_H__
#define __MENU_BUILDER_H__

#include <string>
#include <sigc++/sigc++.h>


namespace Ekiga
{

/**
 * @defgroup actions Actions
 * @{
 */

  /** Menu builder.
   *
   * This object serves to make objects toolkit-independent: an object can
   * add actions to a MenuBuilder, and the specific implementation will
   * take care of the GUI.
   * For example you can do in your GUI:
   * <pre>
   *   MyToolkitMenuType *menu = new MyToolkitMenuType(whatever);
   *   MyMenuBuilder builder(menu);
   *
   *   toolkit_independent_object.populate_menu (builder);
   * </pre>
   *
   * now use builder->menu, which is toolkit-dependent!
   *
   * ! Notice !
   * You better be sure the toolkit_independent_object will still live
   * when using the obtained menu, or the user may trigger a now-dead
   * action... for example it could be an object with "removed" &
   * "updated" signals, and you watch those, which allow you to regenerate
   * or discard the menu in due time.
   */

  class MenuBuilder
  {
  public:

    virtual ~MenuBuilder ();

    virtual void add_action (const std::string icon,
			     const std::string label,
			     sigc::slot0<void> callback);

    virtual void add_separator ();

    virtual void add_ghost (const std::string icon,
			    const std::string label);

    virtual bool empty () const;

    virtual int size () const;
  };

/**
 * @}
 */

};

#endif
