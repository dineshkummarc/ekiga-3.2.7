
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
 *                         ldap-book.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *                        : completed in 2008 by Howard Chu
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of a LDAP book
 *
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <string.h>

#include <glib.h>

#include "config.h"

#include <sasl/sasl.h>

#include "ldap-book.h"
#include "robust-xml.h"

/* little helper function... can probably be made more complete */
static const std::string
fix_to_utf8 (const std::string str)
{
  gchar *utf8_str = NULL;
  std::string result;

  if (g_utf8_validate (str.c_str (), -1, NULL))
    utf8_str = g_strdup (str.c_str ());
  else
    utf8_str =  g_convert (str.c_str (), -1,
                           "UTF-8", "ISO-8859-1",
                           0, 0, 0);

  result = std::string (utf8_str);

  g_free (utf8_str);

  return result;
}

/* parses a message to construct a nice contact */
OPENLDAP::ContactPtr
OPENLDAP::Book::parse_result (LDAPMessage* message)
{
  ContactPtr result;
  BerElement *ber = NULL;
  struct berval bv, *bvals;
  std::string username;
  std::map<std::string, std::string> call_addresses;
  char **attributes = bookinfo.urld->lud_attrs;
  int i, rc;

  /* skip past entry DN */
  rc = ldap_get_dn_ber (ldap_context, message, &ber, &bv);

  while (rc == LDAP_SUCCESS) {
    rc = ldap_get_attribute_ber (ldap_context, message, ber, &bv, &bvals);
    if (bv.bv_val == NULL) break;
    if (attributes[0] == NULL || !strcasecmp(bv.bv_val, attributes[0])) {
      username = std::string (bvals[0].bv_val, bvals[0].bv_len);
    } else {
      for (i=1; attributes[i]; i++) {
        if (!strcasecmp(bv.bv_val,attributes[i]) && bvals && bvals[0].bv_val ) {
	  call_addresses[attributes[i]] =
	    /* FIXME: next line is annoying */
	    std::string ("sip:") +
	    std::string (bvals[0].bv_val, bvals[0].bv_len);
        }
      }
    }
    if (bvals) ber_memfree(bvals);
  }

  ber_free (ber, 0);

  if (!username.empty () && !call_addresses.empty()) {

    result = ContactPtr(new Contact (core, fix_to_utf8 (username), call_addresses));
  }

  return result;
}


#if 0  /* seems to be unused / unneeded */
/* this allows us to do the refreshing in a thread: we put all needed
 * data in this structure, let everything happen elsewhere, then push back
 * into the main thread
 */
struct RefreshData
{

  RefreshData (Ekiga::ServiceCore &_core,
	       const std::string _name,
	       const std::string _uri,
	       const std::string _base,
	       const std::string _scope,
	       const std::string _call_attribute,
	       const std::string _authcID,
	       const std::string _password,
	       const std::string _search_string,
	       sigc::slot1<void, std::vector<OPENLDAP::Contact *> > _publish_results):
    core(_core), name(_name), uri(_uri),
    base(_base), scope(_scope), call_attribute(_call_attribute),
    authcID(_authcID), password(_password), search_string(_search_string),
    error(false), publish_results (_publish_results)
  {
  }

  /* search data */
  Ekiga::ServiceCore &core;
  std::string name;
  std::string uri;
  std::string base;
  std::string scope;
  std::string call_attribute;
  std::string authcID;
  std::string password;
  std::string search_string;

  /* result data */
  bool error;
  std::vector<OPENLDAP::Contact *> contacts;
  std::string error_message;

  /* callback */
  sigc::slot1<void, std::vector<OPENLDAP::Contact *> > publish_results;
};
#endif

/* actual implementation */

OPENLDAP::Book::Book (Ekiga::ServiceCore &_core,
		      std::tr1::shared_ptr<xmlDoc> _doc,
		      xmlNodePtr _node):
  saslform(NULL), core(_core), doc(_doc), node(_node),
  name_node(NULL), uri_node(NULL), authcID_node(NULL), password_node(NULL),
  ldap_context(NULL), patience(0)
{
  xmlChar *xml_str;
  bool upgrade_config = false;

  /* for previous config */
  std::string hostname="", port="", base="", scope="", call_attribute="";
  xmlNodePtr hostname_node = NULL, port_node = NULL, base_node = NULL,
    scope_node = NULL, call_attribute_node = NULL;

  bookinfo.name = "";
  bookinfo.uri = "";
  bookinfo.authcID = "";
  bookinfo.password = "";
  bookinfo.saslMech = "";
  bookinfo.urld = NULL;
  bookinfo.sasl = false;
  bookinfo.starttls = false;

  for (xmlNodePtr child = node->children ;
       child != NULL;
       child = child->next) {

    if (child->type == XML_ELEMENT_NODE
	&& child->name != NULL) {

      if (xmlStrEqual (BAD_CAST ("name"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	bookinfo.name = (const char *)xml_str;
	xmlFree (xml_str);
	name_node = child;
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("uri"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	bookinfo.uri = (const char *)xml_str;
	xmlFree (xml_str);
	uri_node = child;
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("hostname"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	hostname = (const char *)xml_str;
	hostname_node = child;
	xmlFree (xml_str);
	upgrade_config = true;
	continue;
      }
      if (xmlStrEqual (BAD_CAST ("port"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	port = (const char *)xml_str;
	port_node = child;
	xmlFree (xml_str);
	upgrade_config = true;
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("base"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	base = (const char *)xml_str;
	base_node = child;
	xmlFree (xml_str);
	upgrade_config = true;
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("scope"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	scope = (const char *)xml_str;
	scope_node = child;
	xmlFree (xml_str);
	upgrade_config = true;
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("call_attribute"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	call_attribute = (const char *)xml_str;
	call_attribute_node = child;
	xmlFree (xml_str);
	upgrade_config = true;
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("authcID"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	bookinfo.authcID = (const char *)xml_str;
	authcID_node = child;
	xmlFree (xml_str);
	continue;
      }

      if (xmlStrEqual (BAD_CAST ("password"), child->name)) {

	xml_str = xmlNodeGetContent (child);
	bookinfo.password = (const char *)xml_str;
	password_node = child;
	xmlFree (xml_str);
	continue;
      }
    }
  }
  if (upgrade_config) {

    if (!uri_node) {

      LDAPURLDesc *url_tmp = NULL;
      char *url_str;
      std::string new_uri;
      if (hostname.empty())
        hostname="localhost";
      new_uri = std::string("ldap://") + hostname;
      if (!port.empty())
        new_uri += std::string(":") + port;
      new_uri += "/?cn," + call_attribute + "?" + scope;
      ldap_url_parse (new_uri.c_str(), &url_tmp);
      url_tmp->lud_dn = (char *)base.c_str();
      url_str = ldap_url_desc2str (url_tmp);
      bookinfo.uri = std::string(url_str);
      ldap_memfree (url_str);
      robust_xmlNodeSetContent (node, &uri_node, "uri", bookinfo.uri);
      url_tmp->lud_dn = NULL;
      ldap_free_urldesc (url_tmp);
    }
    if (hostname_node) {

      xmlUnlinkNode (hostname_node);
      xmlFreeNode (hostname_node);
    }
    if (port_node) {

      xmlUnlinkNode (port_node);
      xmlFreeNode (port_node);
    }
    if (base_node) {

      xmlUnlinkNode (base_node);
      xmlFreeNode (base_node);
    }
    if (scope_node) {

      xmlUnlinkNode (scope_node);
      xmlFreeNode (scope_node);
    }
    if (call_attribute_node) {

      xmlUnlinkNode (call_attribute_node);
      xmlFreeNode (call_attribute_node);
    }
    trigger_saving.emit ();
  }
  OPENLDAP::BookInfoParse (bookinfo);
}

OPENLDAP::Book::Book (Ekiga::ServiceCore &_core,
		      std::tr1::shared_ptr<xmlDoc> _doc,
		      OPENLDAP::BookInfo _bookinfo):
  saslform(NULL), core(_core), doc(_doc), name_node(NULL),
  uri_node(NULL), authcID_node(NULL), password_node(NULL),
  ldap_context(NULL), patience(0)
{
  node = xmlNewNode (NULL, BAD_CAST "server");

  bookinfo = _bookinfo;

  name_node = xmlNewChild (node, NULL,
			   BAD_CAST "name",
			   BAD_CAST robust_xmlEscape (node->doc,
						      bookinfo.name).c_str ());

  uri_node = xmlNewChild (node, NULL,
			  BAD_CAST "uri",
			  BAD_CAST robust_xmlEscape (node->doc,
						     bookinfo.uri).c_str ());

  authcID_node = xmlNewChild (node, NULL,
			      BAD_CAST "authcID",
			      BAD_CAST robust_xmlEscape (node->doc,
							 bookinfo.authcID).c_str ());

  password_node = xmlNewChild (node, NULL,
			       BAD_CAST "password",
			       BAD_CAST robust_xmlEscape (node->doc,
							  bookinfo.password).c_str ());
  OPENLDAP::BookInfoParse (bookinfo);
}

OPENLDAP::Book::~Book ()
{
  if (bookinfo.urld) ldap_free_urldesc(bookinfo.urld);
}

void
OPENLDAP::BookInfoParse (struct BookInfo &info)
{
  LDAPURLDesc *url_tmp;
  std::string uri;
  size_t pos;

  ldap_url_parse (info.uri.c_str(), &url_tmp);
  if (url_tmp->lud_exts) {
    for (int i=0; url_tmp->lud_exts[i]; i++) {
      if (!strcasecmp(url_tmp->lud_exts[i], "StartTLS")) {
        info.starttls = true;
      } else if (!strncasecmp(url_tmp->lud_exts[i], "SASL", 4)) {
        info.sasl = true;
	if (url_tmp->lud_exts[i][4] == '=')
	  info.saslMech = std::string(url_tmp->lud_exts[i]+5);
      }
    }
  }
  info.urld = url_tmp;
  pos = info.uri.find ('/', strlen(info.urld->lud_scheme) + 3);
  if (pos != std::string::npos)
    info.uri_host = info.uri.substr (0,pos);
  else
    info.uri_host = info.uri;
}

const std::string
OPENLDAP::Book::get_name () const
{
  return bookinfo.name;
}

bool
OPENLDAP::Book::populate_menu (Ekiga::MenuBuilder &builder)
{
  builder.add_action ("refresh", _("_Refresh"),
		      sigc::mem_fun (this, &OPENLDAP::Book::refresh));
  builder.add_separator ();
  builder.add_action ("remove", _("_Remove addressbook"),
		      sigc::mem_fun (this, &OPENLDAP::Book::remove));
  builder.add_action ("properties", _("Addressbook _properties"),
		      sigc::mem_fun (this, &OPENLDAP::Book::edit));

  return true;
}

void
OPENLDAP::Book::set_search_filter (std::string _search_filter)
{
  search_filter = _search_filter;
  refresh ();
}

const std::string
OPENLDAP::Book::get_status () const
{
  return status;
}


void
OPENLDAP::Book::refresh ()
{
  /* we flush */
  remove_all_objects ();

  if (ldap_context == NULL)
    refresh_start ();
}

void
OPENLDAP::Book::remove ()
{
  xmlUnlinkNode (node);
  xmlFreeNode (node);

  trigger_saving.emit ();
  removed.emit ();
}

xmlNodePtr
OPENLDAP::Book::get_node ()
{
  return node;
}

void
OPENLDAP::Book::on_sasl_form_submitted (bool submitted,
					Ekiga::Form &result)
{
  if (!submitted)
    return;

  result.visit (*saslform);
}

extern "C" {

  typedef struct interctx {
    OPENLDAP::Book *book;
    std::string authcID;
    std::string password;
    std::list<std::string> results;
  } interctx;

  static int
  book_saslinter(LDAP *ld, unsigned flags __attribute__((unused)),
		 void *def, void *inter)
  {
    sasl_interact_t *in = (sasl_interact_t *)inter;
    interctx *ctx = (interctx *)def;
    struct berval p;
    int i, nprompts = 0;

    /* Fill in the prompts we have info for; count
     * how many we're missing.
     */
    for (;in->id != SASL_CB_LIST_END;in++)
      {
	p.bv_val = NULL;
	switch(in->id)
	  {
	  case SASL_CB_GETREALM:
	    ldap_get_option(ld, LDAP_OPT_X_SASL_REALM, &p.bv_val);
	    if (p.bv_val) p.bv_len = strlen(p.bv_val);
	    break;
	  case SASL_CB_AUTHNAME:
	    p.bv_len = ctx->authcID.length();
	    if (p.bv_len)
	      p.bv_val = (char *)ctx->authcID.c_str();
	    break;
	  case SASL_CB_USER:
	    /* If there was a default authcID, just ignore the authzID */
	    if (ctx->authcID.length()) {
	      p.bv_val = (char *)"";
	      p.bv_len = 0;
	    }
	    break;
	  case SASL_CB_PASS:
	    p.bv_len = ctx->password.length();
	    if (p.bv_len)
	      p.bv_val = (char *)ctx->password.c_str();
	    break;
	  default:
	    break;
	  }
	if (p.bv_val)
	  {
	    in->result = p.bv_val;
	    in->len = p.bv_len;
	  } else
	  {
	    nprompts++;
	    in->result = NULL;
	  }
      }

    /* If there are missing items, try to get them all in one dialog */
    if (nprompts) {
      Ekiga::FormRequestSimple request(sigc::mem_fun (ctx->book, &OPENLDAP::Book::on_sasl_form_submitted));
      Ekiga::FormBuilder result;
      std::string prompt;
      std::string ctxt = "";
      char resbuf[32];

      request.title (_("LDAP SASL Interaction"));

      for (i=0, in = (sasl_interact_t *)inter;
	   in->id != SASL_CB_LIST_END;in++)
	{
	  bool noecho = false, challenge = false;

	  if (in->result) continue;

	  /* Give each dialog item a unique name */
	  sprintf(resbuf, "res%02x", i);
	  i++;

	  /* Check for prompts that need special handling */
	  switch(in->id)
	    {
	    case SASL_CB_PASS:
	      noecho = true;
	      break;
	    case SASL_CB_NOECHOPROMPT:
	      noecho = true;
	      challenge = true;
	      break;
	    case SASL_CB_ECHOPROMPT:
	      challenge = true;
	      break;
	    default:
	      break;
	    }

	  /* accumulate any challenge strings */
	  if (challenge && in->challenge) {

	    /* Translators, Howard explained : "Challenge" is a generic term
	     * in authentication. It's a prompt from the authentication mechanism
	     * for some type of credential. Exactly what kind of challenge and
	     * what kind of credential depends on the specific authentication
	     * mechanism. Since SASL is a generic interface, and can dynamically
	     * load arbitrary mechanisms, there's not much more specific you can
	     * say about it. You might google for "challenge response
	     * authentication" if you'd like more background context.
	     */
	    ctxt += std::string (_("Challenge: ")) +
	      std::string (in->challenge) +"\n";
	  }

	  /* use the provided prompt text, or our default? */
	  if (in->prompt)
	    prompt = std::string (in->prompt);
	  else
	    prompt = std::string (_("Interact"));

	  /* private text or not? */
	  if (noecho) {
	    request.private_text (std::string (resbuf), prompt, "");
	  } else {
	    std::string dflt;
	    if (in->defresult)
	      dflt = std::string (in->defresult);
	    else
	      dflt = "";
	    request.text (std::string(resbuf), prompt, dflt);
	  }
	}

      /* If we had any challenge text, set it now */
      if (!ctxt.empty())
	request.instructions (ctxt);

      /* Save a pointer for storing the form result */
      ctx->book->saslform = &result;
      if (!ctx->book->questions.handle_request (&request)) {
	return LDAP_LOCAL_ERROR;
      }

      /* Extract answers from the result form */
      for (i=0, in = (sasl_interact_t *)inter;
	   in->id != SASL_CB_LIST_END;in++)
	{
	  bool noecho = false;

	  if (in->result) continue;

	  sprintf(resbuf, "res%02x", i);
	  i++;
	  switch(in->id)
	    {
	    case SASL_CB_PASS:
	    case SASL_CB_NOECHOPROMPT:
	      noecho = true;
	      break;
	    default:
	      break;
	    }
	  if (noecho)
	    prompt = result.private_text (std::string (resbuf));
	  else
	    prompt = result.text (std::string (resbuf));

	  /* Save the answers so they don't disappear before our
	   * caller can see them; return the saved copies.
	   */
	  ctx->results.push_back (prompt);
	  in->result = ctx->results.back().c_str();
	  in->len = ctx->results.back().length();
	}
    }
    return LDAP_SUCCESS;
  }

} /* extern "C" */

void
OPENLDAP::Book::refresh_start ()
{
  int msgid = -1;
  int result = LDAP_SUCCESS;
  int ldap_version = LDAP_VERSION3;

  status = std::string (_("Refreshing"));
  updated.emit ();

  result = ldap_initialize (&ldap_context, bookinfo.uri_host.c_str());
  if (result != LDAP_SUCCESS) {

    status = std::string (_("Could not initialize server"));
    updated.emit ();
    return;
  }

  /* the openldap code shows I don't have to check the result of this
   * (see for example tests/prog/slapd-search.c)
   */
  (void)ldap_set_option (ldap_context,
			 LDAP_OPT_PROTOCOL_VERSION, &ldap_version);

  if (bookinfo.starttls) {
    result = ldap_start_tls_s (ldap_context, NULL, NULL);
    if (result != LDAP_SUCCESS) {
      status = std::string (_("LDAP Error: ")) +
        std::string (ldap_err2string (result));
      updated.emit ();
      ldap_unbind_ext (ldap_context, NULL, NULL);
      ldap_context = NULL;
      return;
    }
  }

  if (bookinfo.sasl) {
    interctx ctx;

    ctx.book = this;
    ctx.authcID = bookinfo.authcID;
    ctx.password = bookinfo.password;
    result = ldap_sasl_interactive_bind_s (ldap_context, NULL,
					   bookinfo.saslMech.c_str(), NULL, NULL, LDAP_SASL_QUIET,
					   book_saslinter, &ctx);

  } else {
    /* Simple Bind */
    if (bookinfo.password.empty ()) {
      struct berval bv={0,NULL};

      result = ldap_sasl_bind (ldap_context, NULL,
			       LDAP_SASL_SIMPLE, &bv,
			       NULL, NULL,
			       &msgid);
    } else {

      struct berval passwd = { 0, NULL };
      passwd.bv_val = g_strdup (bookinfo.password.c_str ());
      passwd.bv_len = bookinfo.password.length();

      result = ldap_sasl_bind (ldap_context, bookinfo.authcID.c_str(),
			       LDAP_SASL_SIMPLE, &passwd,
			       NULL, NULL,
			       &msgid);

      g_free (passwd.bv_val);
    }
  }

  if (result != LDAP_SUCCESS) {

    status = std::string (_("LDAP Error: ")) +
      std::string (ldap_err2string (result));
    updated.emit ();

    ldap_unbind_ext (ldap_context, NULL, NULL);
    ldap_context = NULL;
    return;
  }

  status = std::string (_("Contacted server"));
  updated.emit ();

  patience = 3;
  refresh_bound ();
}

void
OPENLDAP::Book::refresh_bound ()
{
  int result = LDAP_SUCCESS;
  struct timeval timeout = { 1, 0}; /* block 1s */
  LDAPMessage *msg_entry = NULL;
  int msgid;
  std::string filter, fterm;
  const char *fstr;
  size_t pos;

  if (bookinfo.sasl)
    goto sasl_bound;

  result = ldap_result (ldap_context, LDAP_RES_ANY, LDAP_MSG_ALL,
			&timeout, &msg_entry);

  if (result <= 0) {

    if (patience == 3) {
      patience--;
      Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &OPENLDAP::Book::refresh_bound), 12);
    } else if (patience == 2) {

      patience--;
      Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &OPENLDAP::Book::refresh_bound), 21);
    } else if (patience == 1) {

      patience--;
      Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &OPENLDAP::Book::refresh_bound), 30);
    } else { // patience == 0

      status = std::string (_("Could not connect to server"));
      updated.emit ();

      ldap_unbind_ext (ldap_context, NULL, NULL);
      ldap_context = NULL;
    }

    if (msg_entry != NULL)
      ldap_msgfree (msg_entry);

    return;
  }
  (void) ldap_msgfree (msg_entry);

 sasl_bound:
  if (!search_filter.empty ()) {
    if (search_filter[0] == '(' &&
        search_filter[search_filter.length()-1] == ')') {
      fstr = search_filter.c_str();
      goto do_search;
    }
    fterm = "*" + search_filter + "*";
  } else {
    fterm = "*";
  }
  if (bookinfo.urld->lud_filter != NULL)
    filter = std::string (bookinfo.urld->lud_filter);
  else
    filter="";
  pos = 0;
  while ((pos=filter.find('$', pos)) != std::string::npos) {
    filter.replace (pos, 1, fterm);
    pos += fterm.length();
  }
  fstr = filter.c_str();

 do_search:
  msgid = ldap_search_ext (ldap_context,
			   bookinfo.urld->lud_dn,
			   bookinfo.urld->lud_scope,
			   fstr,
			   bookinfo.urld->lud_attrs,
			   0, /* attrsonly */
			   NULL, NULL,
			   NULL, 0, &msgid);

  if (msgid == -1) {

    status = std::string (_("Could not search"));
    updated.emit ();

    ldap_unbind_ext (ldap_context, NULL, NULL);
    ldap_context = NULL;
    return;
  } else {

    status = std::string (_("Waiting for search results"));
    updated.emit ();
  }

  patience = 3;
  refresh_result ();
}

void
OPENLDAP::Book::refresh_result ()
{
  int result = LDAP_SUCCESS;
  int nbr = 0;
  struct timeval timeout = { 1, 0}; /* block 1s */
  LDAPMessage *msg_entry = NULL;
  LDAPMessage *msg_result = NULL;
  gchar* c_status = NULL;

  result = ldap_result (ldap_context, LDAP_RES_ANY, LDAP_MSG_ALL,
			&timeout, &msg_entry);


  if (result <= 0) {

    if (patience == 3) {

      patience--;
      Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &OPENLDAP::Book::refresh_result),
				   12);
    } else if (patience == 2) {

      patience--;
      Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &OPENLDAP::Book::refresh_result),
				   21);
    } else if (patience == 1) {

      patience--;
      Ekiga::Runtime::run_in_main (sigc::mem_fun (this, &OPENLDAP::Book::refresh_result),
				   30);
    } else { // patience == 0

      status = std::string (_("Could not search"));
      updated.emit ();

      ldap_unbind_ext (ldap_context, NULL, NULL);
      ldap_context = NULL;
    }

    if (msg_entry != NULL)
      ldap_msgfree (msg_entry);

    return;
  }

  msg_result = ldap_first_message (ldap_context, msg_entry);
  do {

    if (ldap_msgtype (msg_result) == LDAP_RES_SEARCH_ENTRY) {

      ContactPtr contact = parse_result (msg_result);
      if (contact) {
	add_contact (contact);
        nbr++;
      }
    }
    msg_result = ldap_next_message (ldap_context, msg_result);
  } while (msg_result != NULL);

  // Do not count ekiga.net's first entry "Search Results ... 100 entries"
  if (strcmp (bookinfo.uri_host.c_str(), "ldap://ekiga.net") == 0)
    nbr--;
  c_status = g_strdup_printf (ngettext ("%d user found",
					"%d users found", nbr), nbr);
  status = c_status;
  g_free (c_status);

  updated.emit ();

  (void)ldap_msgfree (msg_entry);

  ldap_unbind_ext (ldap_context, NULL, NULL);
  ldap_context = NULL;
}

void
OPENLDAP::BookForm (Ekiga::FormRequestSimple &request,
		    struct BookInfo &info,
		    std::string title)
{
  std::string callAttr = "";

  request.title (title);

  request.instructions (_("Please edit the following fields"));

  request.text ("name", _("Book _Name"), info.name);
  request.text ("uri", _("Server _URI"), info.uri_host);
  request.text ("base", _("_Base DN"), info.urld->lud_dn);

  {
    std::map<std::string, std::string> choices;
    std::string scopes[]= {"base","one","sub"};

    choices["sub"] = _("Subtree");
    choices["onelevel"] = _("Single Level");
    request.single_choice ("scope", _("_Search Scope"),
			   scopes[info.urld->lud_scope], choices);
  }

  /* attrs[0] is the name attribute */
  for (int i=1; info.urld->lud_attrs[i]; i++) {
    if (i>1) callAttr += ",";
    callAttr += std::string(info.urld->lud_attrs[i]);
  }
  request.text ("nameAttr", _("_DisplayName Attribute"), info.urld->lud_attrs[0]);
  request.text ("callAttr", _("Call _Attributes"), callAttr);
  if (info.urld->lud_filter != NULL)
    request.text ("filter", _("_Filter Template"), info.urld->lud_filter);
  else
    request.text ("filter", _("_Filter Template"), "");

  request.text ("authcID", _("Bind _ID"), info.authcID);
  request.private_text ("password", _("_Password"), info.password);
  request.boolean ("startTLS", _("Use TLS"), info.starttls);
  request.boolean ("sasl", _("Use SASL"), info.sasl);
  {
    std::map<std::string, std::string> mechs;
    const char **mechlist;

    mechlist = sasl_global_listmech();

    mechs[""] = "<default>";
    if (mechlist) {
      for (int i=0; mechlist[i]; i++) {
        std::string mech = std::string(mechlist[i]);
        mechs[mech] = mech;
      }
    }
    request.single_choice ("saslMech", _("SASL _Mechanism"),
			   info.saslMech, mechs);
  }
}

void
OPENLDAP::Book::edit ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &OPENLDAP::Book::on_edit_form_submitted));

  OPENLDAP::BookForm (request, bookinfo, std::string(_("Edit LDAP directory")));

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

int
OPENLDAP::BookFormInfo (Ekiga::Form &result,
			struct BookInfo &bookinfo,
			std::string &errmsg)
{
  LDAPURLDesc *url_base = NULL, *url_host = NULL;
  char *url_str;

  std::string name = result.text ("name");
  std::string uri = result.text ("uri");
  std::string nameAttr = result.text ("nameAttr");
  std::string callAttr = result.text ("callAttr");
  std::string filter = result.text ("filter");

  errmsg = "";

  if (name.empty())
    errmsg += _("Please provide a Book Name for this directory\n");

  if (uri.empty())
    errmsg += _("Please provide a Server URI\n");

  if (nameAttr.empty())
    errmsg += _("Please provide a DisplayName Attribute\n");

  if (callAttr.empty())
    errmsg += _("Please provide a Call Attribute\n");

  if (ldap_url_parse (uri.c_str(), &url_host))
    errmsg += _("Invalid Server URI\n");

  if (!errmsg.empty()) {
    return -1;
  }

  if (filter.empty())
    filter = "(cn=$)";

  bookinfo.name = name;
  std::string base = result.text ("base");
  std::string new_bits = "ldap:///?" +
    result.text ("nameAttr") + "," +
    result.text ("callAttr") + "?" +
    result.single_choice ("scope") + "?" +
    result.text ("filter");
  bookinfo.authcID = result.text ("authcID");
  bookinfo.password = result.private_text ("password");
  bookinfo.starttls = result.boolean ("startTLS");
  bookinfo.sasl = result.boolean ("sasl");
  bookinfo.saslMech = result.single_choice ("saslMech");

  if (bookinfo.sasl || bookinfo.starttls) {
    new_bits += "?";
    if (bookinfo.starttls)
      new_bits += "StartTLS";
    if (bookinfo.sasl) {
      if (bookinfo.starttls)
        new_bits += ",";
      new_bits += "SASL";
      if (!bookinfo.saslMech.empty())
        new_bits += "=" + bookinfo.saslMech;
    }
  }

  ldap_url_parse (new_bits.c_str(), &url_base);
  url_host->lud_dn = ldap_strdup (base.c_str());
  url_host->lud_attrs = url_base->lud_attrs;
  url_host->lud_scope = url_base->lud_scope;
  url_host->lud_filter = url_base->lud_filter;
  if (!url_host->lud_exts) {
    url_host->lud_exts = url_base->lud_exts;
    url_base->lud_exts = NULL;
  }
  url_base->lud_attrs = NULL;
  url_base->lud_filter = NULL;
  ldap_free_urldesc (url_base);

  if (bookinfo.urld) ldap_free_urldesc (bookinfo.urld);
  bookinfo.urld = url_host;
  url_str = ldap_url_desc2str (url_host);
  bookinfo.uri = std::string(url_str);
  ldap_memfree (url_str);

  {
    size_t pos;
    pos = bookinfo.uri.find ('/', strlen(url_host->lud_scheme) + 3);
    if (pos != std::string::npos)
      bookinfo.uri_host = bookinfo.uri.substr (0,pos);
    else
      bookinfo.uri_host = bookinfo.uri;
  }
  return 0;
}

void
OPENLDAP::Book::on_edit_form_submitted (bool submitted,
					Ekiga::Form &result)
{
  if (!submitted)
    return;

  std::string errmsg;
  if (OPENLDAP::BookFormInfo (result, bookinfo, errmsg)) {
    Ekiga::FormRequestSimple request(sigc::mem_fun (this, &OPENLDAP::Book::on_edit_form_submitted));

    result.visit (request);
    request.error (errmsg);

    if (!questions.handle_request (&request)) {

      // FIXME: better error reporting
#ifdef __GNUC__
      std::cout << "Unhandled form request in "
		<< __PRETTY_FUNCTION__ << std::endl;
#endif
    }
    return;
  }

  robust_xmlNodeSetContent (node, &name_node, "name", bookinfo.name);

  robust_xmlNodeSetContent (node, &uri_node, "uri", bookinfo.uri);

  robust_xmlNodeSetContent (node, &authcID_node, "authcID", bookinfo.authcID);

  robust_xmlNodeSetContent (node, &password_node, "password", bookinfo.password);
  updated.emit ();
  trigger_saving.emit ();
}
