
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
 *                         rl-heap.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list heap implementation
 *
 */

#include <glib/gi18n.h>

#include <iostream>

#include "robust-xml.h"
#include "form-request-simple.h"
#include "xcap-core.h"

#include "rl-heap.h"

RL::Heap::Heap (Ekiga::ServiceCore& services_,
		std::tr1::shared_ptr<xmlDoc> doc_,
		xmlNodePtr node_):
  services(services_),
  node(node_), name(NULL),
  root(NULL), user(NULL),
  username(NULL), password(NULL),
  doc(doc_), list_node(NULL)
{
  {
    xmlChar* xml_str = NULL;

    xml_str = xmlGetProp (node, BAD_CAST "writable");
    if (xml_str != NULL)
      xmlFree (xml_str);
    else {
      xmlSetProp (node, BAD_CAST "writable", BAD_CAST "0");
    }
  }

  for (xmlNodePtr child = node->children; child != NULL; child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

	name = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("root"), child->name)) {

	root = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("user"), child->name)) {

	user = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("username"), child->name)) {

	username = child;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("password"), child->name)) {

	password = child;
	continue;
      }
    }
  }

  if (name == NULL)
    name = xmlNewChild (node, NULL, BAD_CAST "name",
			BAD_CAST robust_xmlEscape(doc.get (),
						  _("Unnamed")).c_str ());
  if (root == NULL)
    root = xmlNewChild (node, NULL, BAD_CAST "root", BAD_CAST "");
  if (user == NULL)
    user = xmlNewChild (node, NULL, BAD_CAST "user", BAD_CAST "");
  if (username == NULL)
    username = xmlNewChild (node, NULL, BAD_CAST "username", BAD_CAST "");
  if (password == NULL)
    password = xmlNewChild (node, NULL, BAD_CAST "password", BAD_CAST "");

  refresh ();
}

RL::Heap::Heap (Ekiga::ServiceCore& services_,
		std::tr1::shared_ptr<xmlDoc> doc_,
		const std::string name_,
		const std::string root_,
		const std::string user_,
		const std::string username_,
		const std::string password_,
		bool writable_):
  services(services_),
  node(NULL), name(NULL),
  root(NULL), user(NULL),
  username(NULL), password(NULL),
  doc(doc_), list_node(NULL)
{
  node = xmlNewNode (NULL, BAD_CAST "entry");
  if (writable_)
    xmlSetProp (node, BAD_CAST "writable", BAD_CAST "1");
  else
    xmlSetProp (node, BAD_CAST "writable", BAD_CAST "0");

  if ( !name_.empty ())
    name = xmlNewChild (node, NULL,
			BAD_CAST "name",
			BAD_CAST robust_xmlEscape (node->doc,
						   name_).c_str ());
  else
    name = xmlNewChild (node, NULL,
			BAD_CAST "name",
			BAD_CAST robust_xmlEscape (node->doc,
						   _("Unnamed")).c_str ());
  root = xmlNewChild (node, NULL,
		      BAD_CAST "root",
		      BAD_CAST robust_xmlEscape (node->doc,
						 root_).c_str ());
  user = xmlNewChild (node, NULL,
		      BAD_CAST "user",
		      BAD_CAST robust_xmlEscape (node->doc,
						 user_).c_str ());
  username = xmlNewChild (node, NULL,
			  BAD_CAST "username",
			  BAD_CAST robust_xmlEscape (node->doc,
						     username_).c_str ());
  password = xmlNewChild (node, NULL,
			  BAD_CAST "password",
			  BAD_CAST robust_xmlEscape (node->doc,
						     password_).c_str ());
  refresh ();
}

RL::Heap::~Heap ()
{
}

const std::string
RL::Heap::get_name () const
{
  std::string result;
  xmlChar* str = xmlNodeGetContent (name);
  if (str != NULL)
    result = (const char*)str;
  else
    result = _("Unnamed");

  xmlFree (str);

  return result;
}

void
RL::Heap::visit_presentities (sigc::slot1<bool, Ekiga::PresentityPtr > visitor)
{
  bool go_on = true;

  for (std::map<PresentityPtr,std::list<sigc::connection> >::iterator
	 iter = presentities.begin ();
       go_on && iter != presentities.end ();
       ++iter)
    go_on = visitor (iter->first);
}

bool
RL::Heap::populate_menu (Ekiga::MenuBuilder& builder)
{
  builder.add_action ("add", _("_Add a new contact"),
		      sigc::mem_fun (this, &RL::Heap::new_entry));
  builder.add_action ("refresh", _("_Refresh contact list"),
		      sigc::mem_fun (this, &RL::Heap::refresh));
  builder.add_action ("properties", _("Contact list _properties"),
		      sigc::mem_fun (this, &RL::Heap::edit));
  return true;
}

bool
RL::Heap::populate_menu_for_group (std::string /*group*/,
				   Ekiga::MenuBuilder& /*builder*/)
{
  return false; // FIXME
}

xmlNodePtr
RL::Heap::get_node () const
{
  return node;
}

void
RL::Heap::refresh ()
{
  gmref_ptr<XCAP::Core> xcap(services.get ("xcap-core"));
  std::string root_str;
  std::string username_str;
  std::string password_str;
  std::string user_str;

  {
    xmlChar* str = xmlNodeGetContent (root);
    if (str != NULL)
      root_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (user);
    if (str != NULL)
      user_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (username);
    if (str != NULL)
      username_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (password);
    if (str != NULL)
      password_str = (const char*)str;
  }
  gmref_ptr<XCAP::Path> path(new XCAP::Path (root_str, "resource-lists",
					     user_str));
  path->set_credentials (username_str, password_str);
  path = path->build_child ("resource-lists");

  while (presentities.begin () != presentities.end ()) {

    presentities.begin()->first->removed.emit ();
    for (std::list<sigc::connection>::iterator iter2
	   = presentities.begin()->second.begin ();
	 iter2 != presentities.begin()->second.end ();
	 ++iter2)
      iter2->disconnect ();
    presentities.erase (presentities.begin()->first);
  }

  doc.reset ();

  xcap->read (path, sigc::mem_fun (this, &RL::Heap::on_document_received));
}

void
RL::Heap::on_document_received (bool error,
				std::string value)
{
  if (error) {

    // FIXME: do something
    std::cout << "XCAP error: " << value << std::endl;
  } else {

    parse_doc (value);
  }
}

void
RL::Heap::parse_doc (std::string raw)
{
  doc = std::tr1::shared_ptr<xmlDoc> (xmlRecoverMemory (raw.c_str (), raw.length ()), xmlFreeDoc);
  if ( !doc)
    doc = std::tr1::shared_ptr<xmlDoc> (xmlNewDoc (BAD_CAST "1.0"), xmlFreeDoc);
  xmlNodePtr doc_root = xmlDocGetRootElement (doc.get ());

  if (doc_root == NULL
      || doc_root->name == NULL
      || !xmlStrEqual (BAD_CAST "resource-lists", doc_root->name)) {

    std::cout << "Invalid document in " << __PRETTY_FUNCTION__ << std::endl;
    // FIXME: warn the user somehow?
    doc.reset ();
  } else {


    for (xmlNodePtr child = doc_root->children;
	 child != NULL;
	 child = child->next)
      if (child->type == XML_ELEMENT_NODE
	  && child->name != NULL
	  && xmlStrEqual (BAD_CAST ("list"), child->name)) {

	parse_list (child);
	break; // read only one!
      }
  }
}

void
RL::Heap::parse_list (xmlNodePtr list)
{
  std::string root_str;
  std::string user_str;
  std::string username_str;
  std::string password_str;
  bool writable = false;

  list_node = list;
  {
    xmlChar* str = xmlNodeGetContent (root);
    if (str != NULL)
      root_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (user);
    if (str != NULL)
      user_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (username);
    if (str != NULL)
      username_str = (const char*)str;
  }
  {
    xmlChar* str = xmlNodeGetContent (password);
    if (str != NULL)
      password_str = (const char*)str;
  }
  {
    xmlChar* str = xmlGetProp (node, BAD_CAST "writable");
    if (str != NULL) {

      if (xmlStrEqual (str, BAD_CAST "1"))
	writable = true;
      xmlFree (str);
    }
  }

  gmref_ptr<XCAP::Path> path(new XCAP::Path (root_str, "resource-lists",
					     user_str));
  path->set_credentials (username_str, password_str);
  path = path->build_child ("resource-lists");
  path = path->build_child ("list");

  for (xmlNodePtr child = list->children;
       child != NULL;
       child = child->next)
    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL
	&& xmlStrEqual (BAD_CAST ("entry"), child->name)) {

      PresentityPtr presentity(new Presentity (services, path, doc, child, writable));
      std::list<sigc::connection> conns;
      conns.push_back (presentity->updated.connect (sigc::bind (presentity_updated.make_slot (),presentity)));
      conns.push_back (presentity->removed.connect (sigc::bind(presentity_removed.make_slot (),presentity)));
      conns.push_back (presentity->trigger_reload.connect (sigc::mem_fun (this, &RL::Heap::refresh)));
      conns.push_back (presentity->questions.connect (questions.make_slot()));
      presentities[presentity]=conns;
      presentity_added.emit (presentity);
      continue;
    }
}

void
RL::Heap::push_presence (const std::string uri_,
			 const std::string presence)
{
  for (std::map<PresentityPtr,std::list<sigc::connection> >::iterator
	 iter = presentities.begin ();
       iter != presentities.end ();
       ++iter) {

    if (iter->first->get_uri () == uri_)
      iter->first->set_presence (presence);
  }
}

void
RL::Heap::push_status (const std::string uri_,
		       const std::string status)
{
  for (std::map<PresentityPtr,std::list<sigc::connection> >::iterator
	 iter = presentities.begin ();
       iter != presentities.end ();
       ++iter) {

    if (iter->first->get_uri () == uri_)
      iter->first->set_status (status);
  }
}

void
RL::Heap::edit ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &RL::Heap::on_edit_form_submitted));

  std::string name_str;
  std::string root_str;
  std::string username_str;
  std::string password_str;
  std::string user_str;
  bool writable = false;

  {
    xmlChar* str = xmlNodeGetContent (root);
    if (str != NULL) {

      root_str = (const char*)str;
      xmlFree (str);
    }
  }
  {
    xmlChar* str = xmlNodeGetContent (user);
    if (str != NULL) {

      user_str = (const char*)str;
      xmlFree (str);
    }
  }
  {
    xmlChar* str = xmlNodeGetContent (username);
    if (str != NULL) {

      username_str = (const char*)str;
      xmlFree (str);
    }
  }
  {
    xmlChar* str = xmlNodeGetContent (password);
    if (str != NULL) {

      password_str = (const char*)str;
      xmlFree (str);
    }
  }
  {
    xmlChar* str = xmlGetProp (node, BAD_CAST "writable");
    if (str != NULL) {

      if (xmlStrEqual (str, BAD_CAST "1"))
	writable = true;
      xmlFree (str);
    }
  }

  request.title (_("Edit contact list properties"));

  request.instructions (_("Please edit the following fields (no identifier"
			  " means global)"));

  request.text ("name", _("Contact list's name"), get_name ());
  /* "Document" used as a name -- uri point to the root of a document tree */
  request.text ("root", _("Document root"), root_str);
  request.text ("user", _("Identifier"), user_str);
  request.boolean ("writable", _("Writable"), writable);
  request.text ("username", _("Server username"), username_str);
  request.private_text ("password", _("Server password"), password_str);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
RL::Heap::on_edit_form_submitted (bool submitted,
				  Ekiga::Form& result)
{
  if (!submitted)
    return;

  std::string name_str = result.text ("name");
  std::string root_str = result.text ("root");
  std::string user_str = result.text ("user");
  std::string username_str = result.text ("username");
  std::string password_str = result.private_text ("password");
  bool writable = result.boolean ("writable");

  if (writable)
    xmlSetProp (node, BAD_CAST "writable", BAD_CAST "1");
  else
    xmlSetProp (node, BAD_CAST "writable", BAD_CAST "0");
  robust_xmlNodeSetContent (node, &name, "name", name_str);
  robust_xmlNodeSetContent (node, &root, "root", root_str);
  robust_xmlNodeSetContent (node, &user, "user", user_str);
  robust_xmlNodeSetContent (node, &username, "username", username_str);
  robust_xmlNodeSetContent (node, &password, "password", password_str);

  trigger_saving.emit ();
  updated.emit ();
  refresh ();
}

void
RL::Heap::new_entry ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &RL::Heap::on_new_entry_form_submitted));

  request.title (_("Add a remote contact"));
  request.instructions (_("Please fill in this form to create a new "
			  "contact on a remote server"));

  std::set<std::string> all_groups;
  for (std::map<PresentityPtr,std::list<sigc::connection> >::iterator
	 iter = presentities.begin ();
       iter != presentities.end ();
       ++iter) {

    std::set<std::string> groups = iter->first->get_groups ();
    all_groups.insert (groups.begin (), groups.end ());
  }

  request.text ("name", _("Name:"), "");
  request.text ("uri", _("Address:"), "");
  request.editable_set ("groups", _("Choose groups:"),
			std::set<std::string>(), all_groups);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
RL::Heap::on_new_entry_form_submitted (bool submitted,
				       Ekiga::Form& result)
{
  if (!submitted)
    return;

  std::string entry_name = result.text ("name");
  std::string entry_uri = result.text ("uri");
  std::set<std::string> entry_groups = result.editable_set ("groups");

  xmlNodePtr entry_node = xmlNewChild (list_node, NULL,
				       BAD_CAST "entry", NULL);
  xmlSetProp (entry_node, BAD_CAST "uri",
	      BAD_CAST robust_xmlEscape (doc.get (), entry_uri).c_str ());
  xmlNewChild (entry_node, NULL, BAD_CAST "display-name",
	       BAD_CAST robust_xmlEscape (doc.get (), entry_name).c_str ());
  xmlNsPtr ns = xmlSearchNsByHref (doc.get (), entry_node,
				   BAD_CAST "http://www.ekiga.org");
  if (ns == NULL) {

    // FIXME: we should handle the case, even if it shouldn't happen
  }

  for (std::set<std::string>::const_iterator iter = entry_groups.begin ();
       iter != entry_groups.end ();
       ++iter) {

    xmlNewChild (entry_node, ns, BAD_CAST "group",
		 BAD_CAST robust_xmlEscape (doc.get (), *iter).c_str ());
  }

  xmlBufferPtr buffer = xmlBufferCreate ();
  int res = xmlNodeDump (buffer, doc.get (), entry_node, 0, 0);

  if (res >= 0) {

    std::string root_str;
    std::string username_str;
    std::string password_str;
    std::string user_str;

    {
      xmlChar* str = xmlNodeGetContent (root);
      if (str != NULL)
	root_str = (const char*)str;
    }
    {
      xmlChar* str = xmlNodeGetContent (user);
      if (str != NULL)
	user_str = (const char*)str;
    }
    {
      xmlChar* str = xmlNodeGetContent (username);
      if (str != NULL)
	username_str = (const char*)str;
    }
    {
      xmlChar* str = xmlNodeGetContent (password);
      if (str != NULL)
	password_str = (const char*)str;
    }
    gmref_ptr<XCAP::Path> path(new XCAP::Path (root_str, "resource-lists",
					       user_str));
    path->set_credentials (username_str, password_str);
    path = path->build_child ("resource-lists");
    path = path->build_child ("list");
    path = path->build_child_with_attribute ("entry", "uri", entry_uri);
    gmref_ptr<XCAP::Core> xcap(services.get ("xcap-core"));
    xcap->write (path, "application/xcap-el+xml",
		 (const char*)xmlBufferContent (buffer),
		 sigc::mem_fun (this, &RL::Heap::new_entry_result));
  }
  xmlBufferFree (buffer);
}

void
RL::Heap::new_entry_result (std::string error)
{
  if ( !error.empty ()) {

    std::cout << "XCAP Error: " << error << std::endl;
  }

  refresh ();
}
