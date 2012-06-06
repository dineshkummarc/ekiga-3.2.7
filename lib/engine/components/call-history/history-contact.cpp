
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
 *                         history-contact.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a call history entry
 *
 */

#include "config.h"

#include <iostream>
#include <glib/gi18n.h>
#include <glib.h>

#include "robust-xml.h"

#include "history-contact.h"


History::Contact::Contact (Ekiga::ServiceCore &_core,
			   std::tr1::shared_ptr<xmlDoc> _doc,
			   xmlNodePtr _node):
  core(_core), doc(_doc), node(_node)
{
  xmlChar* xml_str = NULL;
  gmref_ptr<Ekiga::ContactCore> contact_core = core.get ("contact-core");

  xml_str = xmlGetProp (node, (const xmlChar *)"type");
  if (xml_str != NULL) {

    m_type = (call_type)(xml_str[0] - '0'); // FIXME: I don't like it!
    xmlFree (xml_str);
  }

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
        xmlFree (xml_str);
      }

      if (xmlStrEqual (BAD_CAST ("call_start"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL)
	  call_start = (time_t) atoi ((const char *) xml_str);
        xmlFree (xml_str);
      }

      if (xmlStrEqual (BAD_CAST ("call_duration"), child->name)) {

        xml_str = xmlNodeGetContent (child);
	if (xml_str != NULL)
	  call_duration = (const char *) xml_str;
        xmlFree (xml_str);
      }
    }
  }
}


History::Contact::Contact (Ekiga::ServiceCore &_core,
			   std::tr1::shared_ptr<xmlDoc> _doc,
			   const std::string _name,
			   const std::string _uri,
                           time_t _call_start,
                           const std::string _call_duration,
			   call_type c_t):
  core(_core), doc(_doc), name(_name), uri(_uri), call_start(_call_start), call_duration(_call_duration), m_type(c_t)
{
  gchar* tmp = NULL;
  std::string callp;
  gmref_ptr<Ekiga::ContactCore> contact_core = core.get ("contact-core");

  node = xmlNewNode (NULL, BAD_CAST "entry");

  xmlSetProp (node, BAD_CAST "uri", BAD_CAST uri.c_str ());
  xmlNewChild (node, NULL,
	       BAD_CAST "name",
	       BAD_CAST robust_xmlEscape (node->doc, name).c_str ());

  tmp = g_strdup_printf ("%lu", call_start);
  xmlNewChild (node, NULL,
	       BAD_CAST "call_start", BAD_CAST tmp);
  g_free (tmp);

  xmlNewChild (node, NULL,
	       BAD_CAST "call_duration", BAD_CAST call_duration.c_str ());

  /* FIXME: I don't like the way it's done */
  tmp = g_strdup_printf ("%d", m_type);
  xmlSetProp (node, BAD_CAST "type", BAD_CAST tmp);
  g_free (tmp);
}

History::Contact::~Contact ()
{
}

const std::string
History::Contact::get_name () const
{
  return name;
}

const std::set<std::string>
History::Contact::get_groups () const
{
  std::set<std::string> groups;

  switch (m_type) {
  case RECEIVED:
    groups.insert (_("Received"));
    break;
  case PLACED:
    groups.insert (_("Placed"));
    break;
  case MISSED:
    groups.insert (_("Missed"));
    break;

  default:
    groups.insert ("AIE!!");
  }

  return groups;
}

bool
History::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  gmref_ptr<Ekiga::ContactCore> contact_core = core.get ("contact-core");
  return contact_core->populate_contact_menu (ContactPtr (this),
					      uri, builder);
}

xmlNodePtr
History::Contact::get_node ()
{
  return node;
}

History::call_type
History::Contact::get_type () const
{
  return m_type;
}

time_t
History::Contact::get_call_start () const
{
  return call_start;
}

const std::string 
History::Contact::get_call_duration () const
{
  return call_duration;
}

bool
History::Contact::is_found (std::string /*test*/) const
{
  /* FIXME */
  return true;
}
