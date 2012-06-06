
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
 *                         local-presentity.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a presentity for the local roster
 *
 */



#ifndef __LOCAL_PRESENTITY_H__
#define __LOCAL_PRESENTITY_H__

#include <libxml/tree.h>

#include <tr1/memory>


#include "form.h"
#include "presence-core.h"
#include "presentity.h"

namespace Local
{

/**
 * @addtogroup presence
 * @internal
 * @{
 */

  /**
   * This class implements an Ekiga::Presentity.
   *
   * The Presentity is represented by an internal XML document.
   *
   * There is a private signal, 'trigger_saving', which will be emitted
   * when appropriate to signal the Local::Heap that the XML tree has been
   * modified, and hence needs saving.
   */
  class Presentity: public Ekiga::Presentity
  {
  public:

    /**
     * Constructors (and destructor)
     */
    Presentity (Ekiga::ServiceCore &_core,
		std::tr1::shared_ptr<xmlDoc> _doc,
		xmlNodePtr _node);

    Presentity (Ekiga::ServiceCore &_core,
		std::tr1::shared_ptr<xmlDoc> _doc,
		const std::string _name,
		const std::string _uri,
		const std::set<std::string> _groups);

    ~Presentity ();


    /**
     * Get elements of the presentity
     */
    const std::string get_name () const;

    const std::string get_presence () const;

    const std::string get_status () const;

    const std::string get_avatar () const;

    const std::set<std::string> get_groups () const;

    const std::string get_uri () const;


    /**
     * This will set a new presence string
     * and emit the 'updated' signal to announce
     * to the various components that the GmConf::Presentity
     * has been updated.
     */
    void set_presence (const std::string _presence);


    /**
     * This will set a new status string
     * and emit the 'updated' signal to announce
     * to the various components that the GmConf::Presentity
     * has been updated.
     */
    void set_status (const std::string _status);


    /** Populates the given Ekiga::MenuBuilder with the actions.
     * Inherits from Ekiga::Presentity.
     * @param: A MenuBuilder.
     * @return: A populated menu.
     */
    bool populate_menu (Ekiga::MenuBuilder &);


    /** Return the current node in the XML document
     * describing the Presentity.
     * @return: A pointer to the node.
     */
    xmlNodePtr get_node () const;


    /** Rename the given group, if this Presentity belongs to it
     *
     */
    void rename_group (const std::string old_name,
		       const std::string new_name);


    /** Remove the current node from the XML document,
     * emit the 'removed' signal so the different views
     * know we're going away, and finally emit 'trigger_saving'
     * so the Local::Heap knows about it.
     *
     */
    void remove ();

    /** Private signal.
     * This signal makes the Local::Heap know that the XML tree changed
     * and hence should be saved
     */
    sigc::signal0<void> trigger_saving;


  private:

    /** This function should be called when a presentity has
     * to be edited. It uses a form with the known
     * fields already filled in.
     */
    void edit_presentity ();


    /** This is called when a form from edit_presentity
     * is submitted.
     *
     * It does error checking and edits the Presentity.
     * It will also emit the 'updated' signal and the
     * private 'trigger_saving' signal to trigger saving
     * from the Local::Heap.
     */
    void edit_presentity_form_submitted (bool submitted,
					 Ekiga::Form &result);


    Ekiga::ServiceCore &core;

    std::tr1::shared_ptr<xmlDoc> doc;
    xmlNodePtr node;
    xmlNodePtr name_node;

    std::string name;
    std::string uri;
    std::string presence;
    std::string status;
    std::string avatar;

    std::map<std::string, xmlNodePtr> group_nodes;
    std::set<std::string> groups;
  };

  typedef gmref_ptr<Presentity> PresentityPtr;

/**
 * @}
 */
};

#endif
