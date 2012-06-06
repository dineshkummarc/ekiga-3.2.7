
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
 *                         local-heap.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of the heap of the local roster
 *
 */

#include <algorithm>
#include <iostream>
#include <set>
#include <glib/gi18n.h>

#include "config.h"

#include "gmconf.h"
#include "form-request-simple.h"

#include "local-heap.h"

#define KEY "/apps/" PACKAGE_NAME "/contacts/roster"


/*
 * Public API
 */
Local::Heap::Heap (Ekiga::ServiceCore &_core): core (_core), doc ()
{
  xmlNodePtr root;
  gchar *c_raw = gm_conf_get_string (KEY);

  // Build the XML document representing the contacts list from the configuration
  if (c_raw != NULL) {

    const std::string raw = c_raw;
    doc = std::tr1::shared_ptr<xmlDoc> (xmlRecoverMemory (raw.c_str (), raw.length ()), xmlFreeDoc);
    if ( !doc)
      doc = std::tr1::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);

    root = xmlDocGetRootElement (doc.get ());
    if (root == NULL) {

      root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
      xmlDocSetRootElement (doc.get (), root);
    }

    for (xmlNodePtr child = root->children; child != NULL; child = child->next)
      if (child->type == XML_ELEMENT_NODE
	  && child->name != NULL
	  && xmlStrEqual (BAD_CAST ("entry"), child->name))
	add (child);

    g_free (c_raw);

    // Or create a new XML document
  }
  else {

    doc = std::tr1::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);
    root = xmlNewDocNode (doc.get (), NULL, BAD_CAST "list", NULL);
    xmlDocSetRootElement (doc.get (), root);

    {
      // add 500 and 501 at ekiga.net in this case!
      std::set<std::string> groups;

      groups.insert (_("Services"));
      add (_("Echo test"), "sip:500@ekiga.net", groups);
      add (_("Conference room"), "sip:501@ekiga.net", groups);
    }
  }
}


Local::Heap::~Heap ()
{
}


const std::string
Local::Heap::get_name () const
{
  return _("Local roster");
}


bool
Local::Heap::populate_menu (Ekiga::MenuBuilder &builder)
{
  builder.add_action ("new", _("New contact"),
		      sigc::bind (sigc::mem_fun (this, &Local::Heap::new_presentity), "", ""));
  return true;
}


bool
Local::Heap::populate_menu_for_group (const std::string name,
				      Ekiga::MenuBuilder& builder)
{
  builder.add_action ("rename_group", _("Rename"),
		      sigc::bind (sigc::mem_fun (this, &Local::Heap::on_rename_group), name));
  return true;
}

struct has_presentity_with_uri_helper
{
  has_presentity_with_uri_helper (const std::string uri_): uri(uri_),
							   found(false)
  {}

  const std::string uri;

  bool found;

  bool test (Local::PresentityPtr presentity)
  {
    if (presentity->get_uri () == uri) {

      found = true;
    }

    return !found;
  }
};

bool
Local::Heap::has_presentity_with_uri (const std::string uri)
{
  has_presentity_with_uri_helper helper(uri);

  visit_presentities (sigc::mem_fun (helper, &has_presentity_with_uri_helper::test));

  return helper.found;
}

struct existing_groups_helper
{
  std::set<std::string> groups;

  bool test (Local::PresentityPtr presentity)
  {
    const std::set<std::string> presentity_groups = presentity->get_groups ();

    groups.insert (presentity_groups.begin (),
		   presentity_groups.end ());

    return true;
  }
};

const std::set<std::string>
Local::Heap::existing_groups ()
{
  std::set<std::string> result;

  {
    existing_groups_helper helper;

    visit_presentities (sigc::mem_fun (helper, &existing_groups_helper::test));
    result = helper.groups;
  }

  result.insert (_("Family"));
  result.insert (_("Friend"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; associate means
     someone who is at the same "level" than you.
  */
  result.insert (_("Associate"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; assistant means
     someone who is at a lower "level" than you.
  */
  result.insert (_("Assistant"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; supervisor means
     someone who is at a higher "level" than you.
  */
  result.insert (_("Supervisor"));
  /* Translator: http://www.ietf.org/rfc/rfc4480.txt proposes several
     relationships between you and your contact; self means yourself.
  */
  result.insert (_("Self"));

  return result;
}


void
Local::Heap::new_presentity (const std::string name,
			     const std::string uri)
{
  if (!has_presentity_with_uri (uri)) {

    gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
    Ekiga::FormRequestSimple request(sigc::mem_fun (this, &Local::Heap::new_presentity_form_submitted));
    std::set<std::string> groups = existing_groups ();

    request.title (_("Add to local roster"));
    request.instructions (_("Please fill in this form to add a new contact "
			    "to ekiga's internal roster"));
    request.text ("name", _("Name:"), name);
    if (presence_core->is_supported_uri (uri)) {

      request.hidden ("good-uri", "yes");
      request.hidden ("uri", uri);
    } else {

      request.hidden ("good-uri", "no");
      if ( !uri.empty ())
	request.text ("uri", _("Address:"), uri);
      else
	request.text ("uri", _("Address:"), "sip:"); // let's put a default
    }

    request.editable_set ("groups",
			  _("Put contact in groups:"),
			  std::set<std::string>(), groups);

    if (!questions.handle_request (&request)) {

      // FIXME: better error reporting
#ifdef __GNUC__
      std::cout << "Unhandled form request in "
		<< __PRETTY_FUNCTION__ << std::endl;
#endif
    }
  }
}

struct push_presence_helper
{
  push_presence_helper (const std::string uri_,
			const std::string presence_): uri(uri_),
						      presence(presence_)
  {}

  bool test (Local::PresentityPtr presentity)
  {
    if (presentity->get_uri () == uri) {

      presentity->set_presence (presence);
    }

    return true;
  }

  const std::string uri;
  const std::string presence;
};

void
Local::Heap::push_presence (const std::string uri,
			    const std::string presence)
{
  push_presence_helper helper(uri, presence);

  visit_presentities (sigc::mem_fun (helper, &push_presence_helper::test));
}

struct push_status_helper
{
  push_status_helper (const std::string uri_,
		      const std::string status_): uri(uri_),
						  status(status_)
  {}

  bool test (Local::PresentityPtr presentity)
  {
    if (presentity->get_uri () == uri) {

      presentity->set_status (status);
    }

    return true;
  }

  const std::string uri;
  const std::string status;
};

void
Local::Heap::push_status (const std::string uri,
			  const std::string status)
{
  push_status_helper helper(uri, status);

  visit_presentities (sigc::mem_fun (helper, &push_status_helper::test));
}



/*
 * Private API
 */
void
Local::Heap::add (xmlNodePtr node)
{
  PresentityPtr presentity (new Presentity (core, doc, node));

  common_add (presentity);
}


void
Local::Heap::add (const std::string name,
		  const std::string uri,
		  const std::set<std::string> groups)
{
  xmlNodePtr root = NULL;

  root = xmlDocGetRootElement (doc.get ());
  PresentityPtr presentity (new Presentity (core, doc, name, uri, groups));

  xmlAddChild (root, presentity->get_node ());

  save ();
  common_add (presentity);
}


void
Local::Heap::common_add (PresentityPtr presentity)
{
  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");

  // Add the presentity to this Heap
  add_presentity (presentity);

  // Fetch presence
  presence_core->fetch_presence (presentity->get_uri ());

  // Connect the Local::Presentity signals.
  add_connection (presentity, presentity->trigger_saving.connect (sigc::mem_fun (this, &Local::Heap::save)));
}


void
Local::Heap::save () const
{
  xmlChar *buffer = NULL;
  int size = 0;

  xmlDocDumpMemory (doc.get (), &buffer, &size);

  gm_conf_set_string (KEY, (const char *)buffer);

  xmlFree (buffer);
}


void
Local::Heap::new_presentity_form_submitted (bool submitted,
					    Ekiga::Form &result)
{
  if (!submitted)
    return;

  gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
  const std::string name = result.text ("name");
  const std::string good_uri = result.hidden ("good-uri");
  std::string uri;
  const std::set<std::string> groups = result.editable_set ("groups");

  if (good_uri == "yes")
    uri = result.hidden ("uri");
  else
    uri = result.text ("uri");

  size_t pos = uri.find_first_of (' ');
  if (pos != std::string::npos)
    uri = uri.substr (0, pos);
  if (presence_core->is_supported_uri (uri)
      && !has_presentity_with_uri (uri)) {

    add (name, uri, groups);
    save ();
  } else {

    Ekiga::FormRequestSimple request(sigc::mem_fun (this, &Local::Heap::new_presentity_form_submitted));

    result.visit (request);
    if (!presence_core->is_supported_uri (uri))
      request.error (_("You supplied an unsupported address"));
    else
      request.error (_("You already have a contact with this address!"));

    if (!questions.handle_request (&request)) {

      // FIXME: better error handling
#ifdef __GNUC__
      std::cout << "Unhandled form request in "
		<< __PRETTY_FUNCTION__ << std::endl;
#endif
    }
  }
}

void
Local::Heap::on_rename_group (std::string name)
{
  Ekiga::FormRequestSimple request(sigc::bind<0>(sigc::mem_fun (this, &Local::Heap::rename_group_form_submitted), name));

  request.title (_("Rename group"));
  request.instructions (_("Please edit this group name"));
  request.text ("name", _("Name:"), name);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

struct rename_group_form_submitted_helper
{
  rename_group_form_submitted_helper (const std::string old_name_,
				      const std::string new_name_):
    old_name(old_name_),
    new_name(new_name_)
  {}

  const std::string old_name;
  const std::string new_name;

  bool rename_group (Local::PresentityPtr presentity)
  {
    presentity->rename_group (old_name, new_name);
    return true;
  }
};

void
Local::Heap::rename_group_form_submitted (std::string old_name,
					  bool submitted,
					  Ekiga::Form& result)
{
  if (!submitted)
    return;

  const std::string new_name = result.text ("name");

  if ( !new_name.empty () && new_name != old_name) {

    rename_group_form_submitted_helper helper (old_name, new_name);
    visit_presentities (sigc::mem_fun (helper, &rename_group_form_submitted_helper::rename_group));
  }
}
