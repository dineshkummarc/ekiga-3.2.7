
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
 *                         menu-builder-tools.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of tools to build menus
 *
 */

#include "menu-builder-tools.h"

/* First, the implementation of the short menu builder
 */

Ekiga::ShortMenuBuilder::ShortMenuBuilder (MenuBuilder &builder_)
  : builder(builder_), active(true)
{
  /* nothing more */
}

void
Ekiga::ShortMenuBuilder::add_action (const std::string icon,
				     const std::string label,
				     sigc::slot0<void> callback)
{
  if (active)
    builder.add_action (icon, label, callback);
}

void
Ekiga::ShortMenuBuilder::add_separator ()
{
  active = false;
}

int
Ekiga::ShortMenuBuilder::size () const
{
  return builder.size ();
}


/* Second, the implementation of the trigger menu builder
 */

Ekiga::TriggerMenuBuilder::TriggerMenuBuilder ()
  : active(true)
{
  /* nothing more */
}

void
Ekiga::TriggerMenuBuilder::add_action (const std::string /*icon*/,
				       const std::string /*label*/,
				       sigc::slot0<void> callback)
{
  if (active) {

    active = false;
    callback ();
  }
}

int
Ekiga::TriggerMenuBuilder::size () const
{
  if (active)
    return 0;
  else
    return 1;
}


/* third, the implementation of the temporary menu builder
 * (with first the definiton of its helpers)
 */

class TemporaryMenuBuilderHelperAction:
  public Ekiga::TemporaryMenuBuilderHelper
{
public:

  TemporaryMenuBuilderHelperAction (const std::string icon_,
				    const std::string label_,
				    sigc::slot0<void> callback_):
    icon(icon_), label(label_), callback(callback_)
  {}

  ~TemporaryMenuBuilderHelperAction ()
  {}

  bool populate_menu (Ekiga::MenuBuilder& builder)
  {
    builder.add_action (icon, label, callback);
    return true;
  }

private:

  std::string icon;
  std::string label;
  sigc::slot0<void> callback;
};

class TemporaryMenuBuilderHelperSeparator:
  public Ekiga::TemporaryMenuBuilderHelper
{
public:

  TemporaryMenuBuilderHelperSeparator ()
  {}

  ~TemporaryMenuBuilderHelperSeparator ()
  {}

  bool populate_menu (Ekiga::MenuBuilder& builder)
  {
    builder.add_separator ();
    return false;
  }
};

class TemporaryMenuBuilderHelperGhost:
  public Ekiga::TemporaryMenuBuilderHelper
{
public:

  TemporaryMenuBuilderHelperGhost (const std::string icon_,
				   const std::string label_):
    icon(icon_), label(label_)
  {}

  ~TemporaryMenuBuilderHelperGhost ()
  {}

  bool populate_menu (Ekiga::MenuBuilder& builder)
  {
    builder.add_ghost (icon, label);
    return false;
  }

private:

  std::string icon;
  std::string label;
};

Ekiga::TemporaryMenuBuilder::TemporaryMenuBuilder ()
  : count(0)
{
  /* nothing more */
}

Ekiga::TemporaryMenuBuilder::~TemporaryMenuBuilder ()
{
  clear ();
}

void
Ekiga::TemporaryMenuBuilder::add_action (const std::string icon,
					 const std::string label,
					 sigc::slot0<void> callback)
{
  TemporaryMenuBuilderHelperAction* helper = NULL;

  helper = new TemporaryMenuBuilderHelperAction (icon, label, callback);

  count++;
  helpers.push_back (helper);
}

void
Ekiga::TemporaryMenuBuilder::add_separator ()
{
  TemporaryMenuBuilderHelperSeparator* helper = NULL;

  helper = new TemporaryMenuBuilderHelperSeparator;

  helpers.push_back (helper);
}

void
Ekiga::TemporaryMenuBuilder::add_ghost (const std::string icon,
					const std::string label)
{
  TemporaryMenuBuilderHelperGhost* helper = NULL;

  helper = new TemporaryMenuBuilderHelperGhost (icon, label);

  helpers.push_back (helper);
}

bool
Ekiga::TemporaryMenuBuilder::empty () const
{
  return (count == 0);
}

int
Ekiga::TemporaryMenuBuilder::size () const
{
  return count;
}

bool
Ekiga::TemporaryMenuBuilder::populate_menu (MenuBuilder& builder)
{
  bool result = false;

  for (std::list<TemporaryMenuBuilderHelper*>::iterator iter = helpers.begin ();
       iter != helpers.end ();
       ++iter)
    result =  (*iter)->populate_menu (builder) || result;

  clear ();

  return result;
}

void
Ekiga::TemporaryMenuBuilder::clear ()
{
  count = 0;

  for (std::list<TemporaryMenuBuilderHelper*>::iterator iter = helpers.begin ();
       iter != helpers.end ();
       ++iter)
    delete *iter;
  helpers.clear ();
}
