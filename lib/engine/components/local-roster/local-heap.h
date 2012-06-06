
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
 *                         local-heap.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the heap for the local roster
 *
 */


#ifndef __LOCAL_HEAP_H__
#define __LOCAL_HEAP_H__

#include "heap-impl.h"
#include "local-presentity.h"


namespace Local
{
/**
 * @addtogroup presence
 * @internal
 * @{
 */

  /**
   * This class implements an Ekiga::Heap.
   *
   * The Heap is internally represented by an XML document with all
   * presentities, and the groups they belong to.
   *
   * The parent class is responsible of emitting the appropriate
   * signals defined in heap.h through the use of global implementations
   * coded in heap-imp.h.
   *
   * When required, the Heap content is being saved in a GmConf entry.
   * Alternatively, it could be saved elsewhere.
   */
  class Heap : public Ekiga::HeapImpl<Presentity>
  {
  public:

    /** The constructor.
     * @param: The Ekiga::ServiceCore to use to trigger operations on other
     * components.
     */
    Heap (Ekiga::ServiceCore &_core);


    /** The destructor.
     */
    ~Heap ();


    /** Return the name of the Heap. Inherits from Ekiga::Heap.
     * @return: The name of the Heap.
     */
    const std::string get_name () const;


    /** Populates the given Ekiga::MenuBuilder with the actions.
     * Inherited from Ekiga::Heap.
     * @param: A MenuBuilder.
     * @return: A populated menu.
     */
    bool populate_menu (Ekiga::MenuBuilder &);


    /** Populates the given Ekiga::MenuBuilder with the actions.
     * Inherited from Ekiga::Heap.
     * @param: The name of the group on which to act.
     * @param: A MenuBuilder.
     * @return: A populated menu.
     */
    bool populate_menu_for_group (const std::string name,
				  Ekiga::MenuBuilder& builder);


    /** Determines if the given uri is already present in the Heap.
     * @param: A string representing an uri.
     * @return: TRUE if that uri is already present in the Heap.
     */
    bool has_presentity_with_uri (const std::string uri);


    /** Returns the list of all groups already in used in the Heap.
     * @return: A list of groups.
     */
    const std::set<std::string> existing_groups ();


    /** This function should be called when a new presentity has
     * to be added to the Heap. It uses a form with the known
     * fields already filled in.
     * @param: The name and uri of the presentity.
     * @return: TRUE if that uri is already present in the Heap.
     */
    void new_presentity (const std::string name,
			 const std::string uri);


    /**
     *  These functions are called by the Local::Cluster to push
     * presence&status information down.
     */
    void push_presence (const std::string uri,
			const std::string presence);
    void push_status (const std::string uri,
		      const std::string status);

  private:

    /** Build a presentity with the given name, uri
     * and groups, add it to the internal XML document
     * and calls common_add for that presentity.
     */
    void add (const std::string name,
	      const std::string uri,
	      const std::set<std::string> groups);


    /** Build a presentity pointed by the xmlNodePtr
     * to the internal XML document and calls common_add.
     * The internal XML document is supposed to be up
     * to date.
     */
    void add (xmlNodePtr node);


    /** Add the Presentity to the Ekiga::Heap.
     *
     * This will trigger the 'presentity_added' signal,
     * and connect it to the public Ekiga::Heap 'removed'
     * and 'updated' signals.
     *
     * It will also ask the PresenceCore to fetch presence
     * information for the newly added presentity.
     *
     * It will also connect the GmConf::Presentity
     * 'save' and 'remove' signals which are specific to the
     * GmConf::Presentity to the save and remove methods.
     *
     * Those signals can be used by a GmConf::Presentity to
     * signal it would like to trigger saving or
     * removing of the Presentity from the Heap.
     */
    void common_add (PresentityPtr presentity);


    /** Save the XML Document in the GmConf key.
     */
    void save () const;


    /** This should be triggered when a new Presentity form
     * built with new_presentity has been submitted.
     *
     * It does error checking and adds the Presentity to the
     * Heap if everything is valid.
     */
    void new_presentity_form_submitted (bool submitted,
					Ekiga::Form &form);

    /** Triggered when the user decides to rename a group.
     * @param The group name
     */
    void on_rename_group (std::string name);

    /**
     */
    void rename_group_form_submitted (std::string old_name,
				      bool submitted,
				      Ekiga::Form& result);

    Ekiga::ServiceCore &core;
    std::tr1::shared_ptr<xmlDoc> doc;
  };

  typedef gmref_ptr<Heap> HeapPtr;

/**
 * @}
 */

};

#endif
