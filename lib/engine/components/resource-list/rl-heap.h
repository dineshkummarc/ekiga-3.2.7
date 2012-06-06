
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
 *                         rl-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : resource-list heap declaration
 *
 */

#ifndef __RL_HEAP_H__
#define __RL_HEAP_H__

#include "heap.h"

#include "rl-presentity.h"

namespace RL {

  class Heap: public Ekiga::Heap
  {
  public:

    Heap (Ekiga::ServiceCore& services_,
	  std::tr1::shared_ptr<xmlDoc> doc_,
	  xmlNodePtr node);

    /* name: the name of the Heap in the GUI
     * root: the XCAP root address
     * user: the user as XCAP user
     * username: the username on the HTTP server
     * password: the password on the HTTP server
     * writable: whether we have write rights on the server
     *
     * Don't complain to me(Snark) it's complex : read RFC4825 and cry with me
     *
     */
    Heap (Ekiga::ServiceCore& core_,
	  std::tr1::shared_ptr<xmlDoc> doc_,
	  const std::string name_,
	  const std::string root_,
	  const std::string user_,
	  const std::string username_,
	  const std::string password_,
	  bool writable_);

    ~Heap ();

    const std::string get_name () const;

    void visit_presentities (sigc::slot1<bool, Ekiga::PresentityPtr > visitor);

    bool populate_menu (Ekiga::MenuBuilder& builder);

    bool populate_menu_for_group (std::string group,
				  Ekiga::MenuBuilder& builder);

    xmlNodePtr get_node () const;

    void push_presence (const std::string uri,
			const std::string presence);

    void push_status (const std::string uri,
		      const std::string status);

    sigc::signal0<void> trigger_saving;

  private:

    Ekiga::ServiceCore& services;

    xmlNodePtr node;
    xmlNodePtr name;
    xmlNodePtr root;
    xmlNodePtr user;
    xmlNodePtr username;
    xmlNodePtr password;

    std::tr1::shared_ptr<xmlDoc> doc;
    xmlNodePtr list_node;

    std::map<PresentityPtr, std::list<sigc::connection> > presentities;

    void refresh ();

    void on_document_received (bool error,
			       std::string doc);

    void parse_doc (std::string doc);
    void parse_list (xmlNodePtr node);

    void edit ();
    void on_edit_form_submitted (bool submitted,
				 Ekiga::Form& result);

    void new_entry ();
    void on_new_entry_form_submitted (bool submitted,
				      Ekiga::Form& result);
    void new_entry_result (std::string error);
  };

  typedef gmref_ptr<Heap> HeapPtr;

};

#endif
