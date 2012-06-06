
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
 *                         ldap-book.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *                        : completed in 2008 by Howard Chu
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a LDAP book
 *
 */

#ifndef __LDAP_BOOK_H__
#define __LDAP_BOOK_H__

#include <vector>
#include <tr1/memory>
#include <libxml/tree.h>
#include <glib/gi18n.h>

#include "runtime.h"
#include "book-impl.h"
#include "form.h"
#include "form-request-simple.h"

#include "ldap-contact.h"

#include <ldap.h>

namespace OPENLDAP
{

  struct BookInfo {
    std::string name;
    std::string uri;
    std::string uri_host;
    std::string authcID;
    std::string password;
    std::string saslMech;
    LDAPURLDesc *urld;
    bool sasl;
    bool starttls;
  };

  void BookForm (Ekiga::FormRequestSimple &req, struct BookInfo &info,
  	std::string title );

  int BookFormInfo (Ekiga::Form &result, struct BookInfo &info,
  	std::string &errmsg);

  void BookInfoParse (struct BookInfo &info);

/**
 * @addtogroup contacts
 * @internal
 * @{
 */

  class Book:
    public Ekiga::BookImpl<Contact>
  {
  public:

    Book (Ekiga::ServiceCore &_core,
	  std::tr1::shared_ptr<xmlDoc> _doc,
	  xmlNodePtr node);

    Book (Ekiga::ServiceCore &_core,
	  std::tr1::shared_ptr<xmlDoc> _doc,
    	  OPENLDAP::BookInfo _bookinfo);

    ~Book ();

    const std::string get_name () const;

    bool populate_menu (Ekiga::MenuBuilder &builder);

    void set_search_filter (std::string search_string);

    const std::string get_status () const;

    const std::string get_icon () const
    { return "remote-object"; }

    void refresh ();

    void remove ();

    xmlNodePtr get_node ();

    sigc::signal0<void> trigger_saving;

    /* public for access from C */
    void on_sasl_form_submitted (bool, Ekiga::Form &);
    Ekiga::FormBuilder *saslform;

  private:

    void refresh_start ();
    void refresh_bound ();
    void refresh_result ();

    ContactPtr parse_result(struct ldapmsg *);

    void parse_uri();

    void edit ();
    void on_edit_form_submitted (bool submitted,
				 Ekiga::Form &form);

    Ekiga::ServiceCore &core;
    std::tr1::shared_ptr<xmlDoc> doc;
    xmlNodePtr node;

    xmlNodePtr name_node;
    xmlNodePtr uri_node;
    xmlNodePtr authcID_node;
    xmlNodePtr password_node;

    struct BookInfo bookinfo;

    struct ldap *ldap_context;
    unsigned int patience;

    std::string status;
    std::string search_filter;
  };

  typedef gmref_ptr<Book> BookPtr;

/**
 * @}
 */

};

#endif
