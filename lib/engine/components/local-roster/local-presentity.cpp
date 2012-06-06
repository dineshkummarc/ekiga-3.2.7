
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
 *                         local-presentity.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a presentity for the local roster
 *
 */

#include <algorithm>
#include <iostream>
#include <set>
#include <glib/gi18n.h>

#include "form-request-simple.h"
#include "local-cluster.h"
#include "robust-xml.h"
#include "local-presentity.h"


/*
 * Public API
 */
Local::Presentity::Presentity (Ekiga::ServiceCore &_core,
			       std::tr1::shared_ptr<xmlDoc> _doc,
			       xmlNodePtr _node) :
  core(_core), doc(_doc), node(_node), name_node(NULL), presence("unknown")
{
  xmlChar *xml_str = NULL;

  xml_str = xmlGetProp (node, (const xmlChar *)"uri");
  if (xml_str != NULL) {

    uri = (const char *)xml_str;
    xmlFree (xml_str);
  }

  for (xmlNodePtr child = node->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
        && child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL)
	  name = (const char *)xml_str;
	name_node = child;
        xmlFree (xml_str);
      }

      if (xmlStrEqual (BAD_CAST ("group"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL)
	  group_nodes[(const char *)xml_str] = child;
	else
	  group_nodes[""] = child;
        xmlFree (xml_str);
      }
    }
  }

  for (std::map<std::string, xmlNodePtr>::const_iterator iter
	 = group_nodes.begin ();
       iter != group_nodes.end ();
       iter++)
    groups.insert (iter->first);
}


Local::Presentity::Presentity (Ekiga::ServiceCore &_core,
			       std::tr1::shared_ptr<xmlDoc> _doc,
			       const std::string _name,
			       const std::string _uri,
			       const std::set<std::string> _groups) :
  core(_core), doc(_doc), name_node(NULL), name(_name), uri(_uri),
  presence("unknown"), groups(_groups)
{
  node = xmlNewNode (NULL, BAD_CAST "entry");
  xmlSetProp (node, BAD_CAST "uri", BAD_CAST uri.c_str ());
  name_node = xmlNewChild (node, NULL,
			   BAD_CAST "name",
			   BAD_CAST robust_xmlEscape (node->doc,
						      name).c_str ());
  for (std::set<std::string>::const_iterator iter = groups.begin ();
       iter != groups.end ();
       iter++)
    group_nodes[*iter] = xmlNewChild (node, NULL,
				      BAD_CAST "group",
				      BAD_CAST robust_xmlEscape (node->doc,
								 *iter).c_str ());
}


Local::Presentity::~Presentity ()
{
}


const std::string
Local::Presentity::get_name () const
{
  return name;
}


const std::string
Local::Presentity::get_presence () const
{
  return presence;
}


const std::string
Local::Presentity::get_status () const
{
  return status;
}


const std::string
Local::Presentity::get_avatar () const
{
  return avatar;
}


const std::set<std::string>
Local::Presentity::get_groups () const
{
  return groups;
}


const std::string
Local::Presentity::get_uri () const
{
  return uri;
}


void
Local::Presentity::set_presence (const std::string _presence)
{
  presence = _presence;
  updated.emit ();
}

void
Local::Presentity::set_status (const std::string _status)
{
  status = _status;
  updated.emit ();
}


bool
Local::Presentity::populate_menu (Ekiga::MenuBuilder &builder)
{
  bool populated = false;
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");

  populated
    = presence_core->populate_presentity_menu (PresentityPtr(this),
					       uri, builder);

  if (populated)
    builder.add_separator ();

  builder.add_action ("edit", _("_Edit"),
		      sigc::mem_fun (this, &Local::Presentity::edit_presentity));
  builder.add_action ("remove", _("_Remove"),
		      sigc::mem_fun (this, &Local::Presentity::remove));

  return true;
}


xmlNodePtr
Local::Presentity::get_node () const
{
  return node;
}


void
Local::Presentity::edit_presentity ()
{
  ClusterPtr cluster = core.get ("local-cluster");
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &Local::Presentity::edit_presentity_form_submitted));

  std::set<std::string> all_groups = cluster->existing_groups ();

  request.title (_("Edit roster element"));
  request.instructions (_("Please fill in this form to change an existing "
			  "element of ekiga's internal roster"));
  request.text ("name", _("Name:"), name);
  request.text ("uri", _("Address:"), uri);

  request.editable_set ("groups", _("Choose groups:"),
			groups, all_groups);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}


void
Local::Presentity::edit_presentity_form_submitted (bool submitted,
						   Ekiga::Form &result)
{
  if (!submitted)
    return;

  const std::string new_name = result.text ("name");
  const std::set<std::string> new_groups = result.editable_set ("groups");
  std::string new_uri = result.text ("uri");
  std::map<std::string, xmlNodePtr> future_group_nodes;
  size_t pos = new_uri.find_first_of (' ');
  if (pos != std::string::npos)
    new_uri = new_uri.substr (0, pos);

  name = new_name;
  if (uri != new_uri) {

    gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
    presence_core->unfetch_presence (uri);
    uri = new_uri;
    presence = "unknown";
    presence_core->fetch_presence (uri);
    xmlSetProp (node, (const xmlChar*)"uri", (const xmlChar*)uri.c_str ());
  }

  robust_xmlNodeSetContent (node, &name_node, "name", name);

  // the first loop looks at groups we were in : are we still in ?
  for (std::map<std::string, xmlNodePtr>::const_iterator iter
	 = group_nodes.begin ();
       iter != group_nodes.end () ;
       iter++) {

    if (new_groups.find (iter->first) == new_groups.end ()) {

      xmlUnlinkNode (iter->second);
      xmlFreeNode (iter->second);
    }
    else {
      future_group_nodes[iter->first] = iter->second;
    }
  }

  // the second loop looking for groups we weren't in but are now
  for (std::set<std::string>::const_iterator iter = new_groups.begin ();
       iter != new_groups.end ();
       iter++) {

    if (std::find (groups.begin (), groups.end (), *iter) == groups.end ())
      future_group_nodes[*iter] = xmlNewChild (node, NULL,
					       BAD_CAST "group",
					       BAD_CAST robust_xmlEscape (node->doc, *iter).c_str ());
  }

  // ok, now we know our groups
  group_nodes = future_group_nodes;
  groups = new_groups;

  updated.emit ();
  trigger_saving.emit ();
}


void
Local::Presentity::rename_group (const std::string old_name,
				 const std::string new_name)
{
  std::map<std::string, xmlNodePtr>::iterator iter
    = group_nodes.find (old_name);

  if (iter != group_nodes.end ()) {

    if (group_nodes.find (new_name) != group_nodes.end ()) {

      /* we're already in the new group */
      xmlUnlinkNode (iter->second);
      xmlFreeNode (iter->second);
      group_nodes.erase (iter);
    } else {

      /* it is a real renaming */
      xmlNodePtr group_node = iter->second;
      robust_xmlNodeSetContent (node, &group_node,
				"group", new_name.c_str ());
      group_nodes.erase (iter);
      group_nodes[new_name] = group_node;
    }

    groups.clear ();

    for (std::map<std::string, xmlNodePtr>::iterator it = group_nodes.begin ();
	 it != group_nodes.end ();
	 ++it) {

      groups.insert (iter->first);
    }

    updated.emit ();
    trigger_saving.emit ();
  }
}


void
Local::Presentity::remove ()
{
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  presence_core->unfetch_presence (uri);

  xmlUnlinkNode (node);
  xmlFreeNode (node);

  trigger_saving.emit ();
  removed.emit ();
}
