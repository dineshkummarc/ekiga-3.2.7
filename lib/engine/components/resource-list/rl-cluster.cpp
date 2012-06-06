
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
 *                         rlist-cluster.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list cluster implementation
 *
 */

#include "config.h"
#include <glib/gi18n.h>

#include "rl-cluster.h"

#include "gmconf.h"
#include "form-request-simple.h"
#include "presence-core.h"

#include <iostream>

#define KEY "/apps/" PACKAGE_NAME "/contacts/resource-lists"

RL::Cluster::Cluster (Ekiga::ServiceCore& core_): core(core_), doc()
{
  gchar* c_raw = NULL;

  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");

  presence_core->presence_received.connect (sigc::mem_fun (this, &RL::Cluster::on_presence_received));
  presence_core->status_received.connect (sigc::mem_fun (this, &RL::Cluster::on_status_received));

  c_raw = gm_conf_get_string (KEY);

  if (c_raw != NULL) {

    const std::string raw = c_raw;
    doc = std::tr1::shared_ptr<xmlDoc> (xmlRecoverMemory (raw.c_str (), raw.length ()), xmlFreeDoc);
    if ( !doc)
      doc = std::tr1::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);

    xmlNodePtr root = xmlDocGetRootElement (doc.get ());
    if (root == NULL) {

      root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc.get (), root);
    } else {

      for (xmlNodePtr child = root->children;
	   child != NULL;
	   child = child->next)
	if (child->type == XML_ELEMENT_NODE
	    && child->name != NULL
	    && xmlStrEqual (BAD_CAST "entry", child->name))
	  add (child);
    }
    g_free (c_raw);

  } else {

    doc = std::tr1::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);
    xmlNodePtr root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
    xmlDocSetRootElement (doc.get (), root);
    add ("https://xcap.sipthor.net/xcap-root", "alice", "123", "alice@example.com", "XCAP Test", false); // FIXME: remove
  }
}

RL::Cluster::~Cluster ()
{
}

bool
RL::Cluster::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("new", _("New resource list"),
		      sigc::bind (sigc::mem_fun (this, &RL::Cluster::new_heap),
				  "", "", "", "", "", false));
  return true;
}

void
RL::Cluster::add (xmlNodePtr node)
{
  HeapPtr heap (new Heap (core, doc, node));

  common_add (heap);
}

void
RL::Cluster::add (const std::string uri,
		  const std::string username,
		  const std::string password,
		  const std::string user,
		  const std::string name,
		  bool writable)
{
  HeapPtr heap (new Heap (core, doc, name, uri, user, username, password, writable));
  xmlNodePtr root = xmlDocGetRootElement (doc.get ());

  xmlAddChild (root, heap->get_node ());

  save ();
  common_add (heap);
}

void
RL::Cluster::common_add (HeapPtr heap)
{
  add_heap (heap);

  // FIXME: here we should ask for presence for the heap...

  heap->trigger_saving.connect (sigc::mem_fun (this, &RL::Cluster::save));
}

void
RL::Cluster::save () const
{
  xmlChar* buffer = NULL;
  int size = 0;

  xmlDocDumpMemory (doc.get (), &buffer, &size);

  gm_conf_set_string (KEY, (const char*)buffer);

  xmlFree (buffer);
}

void
RL::Cluster::new_heap (const std::string name,
		       const std::string uri,
		       const std::string username,
		       const std::string password,
		       const std::string user,
		       bool writable)
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &RL::Cluster::on_new_heap_form_submitted));

  request.title (_("Add new resource-list"));
  request.instructions (_("Please fill in this form to add a new "
			  "contact list to ekiga's remote roster"));
  request.text ("name", _("Name:"), name);
  request.text ("uri", _("Address:"), uri);
  request.boolean ("writable", _("Writable:"), writable);
  request.text ("username", _("Username:"), username);
  request.private_text ("password", _("Password:"), password);
  request.text ("user", _("User:"), user);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
RL::Cluster::on_new_heap_form_submitted (bool submitted,
					 Ekiga::Form& result)
{
  if (!submitted)
    return;

  const std::string name = result.text ("name");
  const std::string uri = result.text ("uri");
  const std::string username = result.text ("username");
  const std::string password = result.private_text ("password");
  const std::string user = result.text ("user");
  bool writable = result.boolean ("writable");

  add (name, uri, username, password, user, writable);
}


void
RL::Cluster::on_presence_received (std::string uri,
				   std::string presence)
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter) {

    (*iter)->push_presence (uri, presence);
  }
}

void
RL::Cluster::on_status_received (std::string uri,
				 std::string status)
{
  for (iterator iter = begin ();
       iter != end ();
       ++iter) {

    (*iter)->push_status (uri, status);
  }
}
