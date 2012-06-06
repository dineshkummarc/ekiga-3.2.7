
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */


/*
 *                         rl-entry-ref.cpp -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list entry-ref class
 *
 */

#include "config.h"

#include <glib/gi18n-lib.h>

#include "rl-entry-ref.h"

#include "presence-core.h"

RL::EntryRef::EntryRef (Ekiga::ServiceCore& core_,
			const std::string path_,
			int pos,
			const std::string group,
			xmlNodePtr node_):
  core(core_), path(path_), position(pos), doc(NULL), node(node_),
  link_doc(NULL), link_node(NULL), name_node(NULL),
  presence("unknown"), status(_("Click to fetch"))
{
  groups.insert (group);

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {


    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST "display-name", child->name)) {

      name_node = child;
    }
  }
}

RL::EntryRef::~EntryRef ()
{
  if (doc != NULL)
    xmlFreeDoc (doc);
}

const std::string
RL::EntryRef::get_uri () const
{
  std::string result;
  xmlChar* str = xmlGetProp (node, BAD_CAST "uri");

  if (str != NULL) {

    result = ((const char*)str);
    xmlFree (str);
  }

  return result;
}


void
RL::EntryRef::set_presence (const std::string presence_)
{
  presence = presence_;
  updated.emit ();
}

void
RL::EntryRef::set_status (const std::string status_)
{
  status = status_;
  updated.emit ();
}

const std::string
RL::EntryRef::get_name () const
{
  std::string result;

  if (link_node != NULL) {

    xmlChar* str = xmlNodeGetContent (node);

    if (str != NULL) {

      result = ((const char*)str);
      xmlFree (str);
    }
  } else
    result = _("Distant contact");

  return result;
}

bool
RL::EntryRef::populate_menu (Ekiga::MenuBuilder& builder)
{
  bool populated = false;
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  std::string uri(get_uri ());

  builder.add_action ("refresh", _("_Refresh"),
		      sigc::mem_fun (this, &RL::EntryRef::refresh));

  if ( !uri.empty ())
    populated = presence_core->populate_presentity_menu (Ekiga::PresentityPtr (this), uri, builder)
      || populated;

  return populated;
}

void
RL::EntryRef::refresh ()
{
  std::cout << "FIXME: should refresh on " << path << std::endl;
}
