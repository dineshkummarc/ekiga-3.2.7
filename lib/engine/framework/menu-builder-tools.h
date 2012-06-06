
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
 *                         menu-builder-tools.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of tools to build menus
 *
 */

#ifndef __MENU_BUILDER_TOOLS_H__
#define __MENU_BUILDER_TOOLS_H__

#include "menu-builder.h"

namespace Ekiga
{
  /** Short menu builder
   *
   * This object is in fact a filter for a MenuBuilder ; you sometimes don't
   * want a long menu like this :
   *  Action 1
   *  Action 2
   *  Separator
   *  Action 3
   *  Action 4
   *  Action 5
   *  Et caetera
   * But would rather just have :
   *  Action 1
   *  Action 2
   * since presumably only the actions before the separator are of interest.
   *
   * For example, you can do :
   * <pre>
   *   MyToolkitMenuType *menu = new MyToolkitMenuType (whatever);
   *   MyMenuBuilder builder(menu);
   *   Ekiga::ShortMenuBuilder shorter(builder);
   *
   *   toolkit_independent_object.populate_menu (shorter);
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

  class ShortMenuBuilder: public MenuBuilder
  {
  public:

    ShortMenuBuilder (MenuBuilder& builder);

    void add_action (const std::string icon,
		     const std::string label,
		     sigc::slot0<void> callback);

    void add_separator ();

    int size () const;

  private:
    MenuBuilder& builder;
    bool active;
  };

  /** Trigger menu builder
   *
   * This menu builder doesn't really build a menu : it just makes it
   * look like it does, but it just triggers the first action given to it.
   * The goal is to make it easy to add a default action on an object : the
   * first one in its populated menu.
   *
   * For example, you can do :
   * <pre>
   *   TriggerMenuBuilder builder;
   *
   *   object->populate_menu (builder);
   * </pre>

   */
  class TriggerMenuBuilder: public MenuBuilder
  {
  public:

    TriggerMenuBuilder ();

    void add_action (const std::string icon,
		     const std::string label,
		     sigc::slot0<void> callback);

    int size () const;

  private:
    bool active;
  };


  /** Temporary menu builder
   *
   * Again, this menu builder doesn't really build a menu itself ; instead
   * it stores what is given to it, to give it later to a real menu builder.
   * The idea is that the fact the populate_menu methods return a boolean
   * allows knowing whether something was given, but there is no way to
   * know if something will be given.
   *
   * For example, you can do :
   * <pre>
   *  SomeRealMenuBuilder builder;
   *  TemporartyMenuBuilder tmp_builder;
   *
   *  if (object1->populate_menu (tmp_builder)) {
   *
   *    builder.add_ghost ("", "Object 1");
   *    tmp_builder.populate_menu (builder);
   *  }
   *  if (object2->populate_menu (tmp_builder)) {
   *
   *    builder.add_ghost ("", "Object 2");
   *    tmp_builder.populate_menu (builder);
   *  }
   * </pre>
   *
   */

  /* it's stupid to have to make it public, but... */
  class TemporaryMenuBuilderHelper
  {
  public:

    virtual ~TemporaryMenuBuilderHelper () {}

    virtual bool populate_menu (Ekiga::MenuBuilder& builder) = 0;
  };

  class TemporaryMenuBuilder: public MenuBuilder
  {
  public:

    TemporaryMenuBuilder ();

    ~TemporaryMenuBuilder ();

    void add_action (const std::string icon,
		     const std::string label,
		     sigc::slot0<void> callback);

    void add_separator ();

    void add_ghost (const std::string icon,
		    const std::string label);

    bool empty () const;

    int size () const;

    /* this empties this temporary builder, so it can be reused */
    bool populate_menu (MenuBuilder& builder);

  private:

    int count;

    std::list<TemporaryMenuBuilderHelper*> helpers;

    void clear ();
  };
};

#endif
