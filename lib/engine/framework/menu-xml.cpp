
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
 *                         menu-xml.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an XML-based menu
 *
 */

#include <iostream>

#include <glib.h>

#include "menu-xml.h"
#include "trigger.h"

/* declaration of helpers */

static void populate_item (Ekiga::ServiceCore &core,
			   Ekiga::MenuBuilder &builder,
			   xmlNodePtr item,
			   bool is_external);

static void run_command (const std::string command)
{
  g_spawn_command_line_async (command.c_str (), NULL);
}

static void pull_trigger (gmref_ptr<Ekiga::Trigger> trigger)
{
  trigger->pull ();
}

/* implementation of the class */

Ekiga::MenuXML::MenuXML (ServiceCore &_core,
			 const std::string filename): core(_core)
{
  doc = xmlReadFile (filename.c_str (), NULL, 0);
}

Ekiga::MenuXML::~MenuXML ()
{
}

void
Ekiga::MenuXML::populate (MenuBuilder &builder)
{

  if (doc != NULL) {

    xmlNodePtr root = xmlDocGetRootElement (doc);

    if (root != NULL
	&& root->name != NULL
	&& xmlStrEqual (BAD_CAST "toolbar", root->name)) {

      for (xmlNodePtr child = root->children ;
	   child != NULL;
	   child = child->next) {

	if (child->type == XML_ELEMENT_NODE
	    && child->name != NULL) {

	  if (xmlStrEqual (BAD_CAST "item", child->name)) {

	    xmlChar *attr = xmlGetProp (child, BAD_CAST "type");
	    if (attr != NULL) {

	      if (xmlStrEqual (BAD_CAST "external", attr))
		populate_item (core, builder, child, true);
	      if (xmlStrEqual (BAD_CAST "internal", attr))
		populate_item (core, builder, child, false);
	      xmlFree (attr);
	    }
	  }
	  if (xmlStrEqual (BAD_CAST "separator", child->name))
	    builder.add_separator ();
	}
      }
    }
  }
}

/* implementation of the helpers */

static void
populate_item (Ekiga::ServiceCore &core,
	       Ekiga::MenuBuilder &builder,
	       xmlNodePtr item,
	       bool is_external)
{
  std::string label;
  bool got_label = false;
  std::string icon;
  bool got_icon = false;
  std::string command;
  bool got_command = false;

  for (xmlNodePtr child = item->children ;
       child != NULL ;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL) {

      if (!got_label && xmlStrEqual (BAD_CAST "label", child->name)) {

	xmlChar *xml_str = xmlNodeGetContent (child);

	if (xml_str != NULL) {

	  label = (const char *)xml_str;
	  got_label = true;
	  xmlFree (xml_str);
	}
      }

      if (!got_icon && xmlStrEqual (BAD_CAST "icon", child->name)) {

	xmlChar *xml_str = xmlNodeGetContent (child);

	if (xml_str != NULL) {

	  icon = (const char *)xml_str;
	  got_icon = true;
	  xmlFree (xml_str);
	}
      }

      if (!got_command && xmlStrEqual (BAD_CAST "command", child->name)) {

	xmlChar *xml_str = xmlNodeGetContent (child);

	if (xml_str != NULL) {

	  command = (const char *)xml_str;
	  got_command = true;
	  xmlFree (xml_str);
	}
      }

      if (got_label && got_icon && got_command) {

	if (is_external) {

	  builder.add_action (icon, label,
			      sigc::bind (sigc::ptr_fun (run_command),
					  command));
	} else {

	  gmref_ptr<Ekiga::Trigger> trigger = core.get (command);
	  if (trigger)
	    builder.add_action (icon, label,
				sigc::bind (sigc::ptr_fun (pull_trigger),
					    trigger));
	}
	break;
      }


    }
  }
}
