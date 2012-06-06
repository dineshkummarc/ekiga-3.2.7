
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
 *                         lister.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an object able to list others
 *
 */

#ifndef __LISTER_H__
#define __LISTER_H__

#include <sigc++/sigc++.h>

#include "ptr_array.h"
#include "ptr_array_iterator.h"
#include "ptr_array_const_iterator.h"

namespace Ekiga
{
  /** Ekiga::Lister
   *
   * This class is there to help write a dynamic object lister, that is an
   * object which will own objects which will emit "updated"
   * and "removed" signals.
   *
   * You can remove an object from an Ekiga::Lister in two ways:
   *  - either by calling the remove_object method,
   *  - or by emission of the object's removed signal.
   *
   * Notice that this class won't take care of removing the object from a
   * backend -- only from the Ekiga::Lister.
   * If you want the object *deleted* from the real backend, then you
   * probably should have an organization like:
   *  - the object has a 'deleted' signal;
   *  - the lister child-class listens to this signal;
   *  - when the signal is received, then do a remove_object followed by
   *    calling the appropriate api function to delete the object in your
   *    backend.
   */
  template<typename ObjectType>
  class Lister
  {

  public:

    typedef Ekiga::ptr_array<ObjectType> container_type;
    typedef typename Ekiga::ptr_array_iterator<ObjectType> iterator;
    typedef typename Ekiga::ptr_array_const_iterator<ObjectType> const_iterator;


    /** The constructor.
     */
    Lister ();


    /** The destructor.
     */
    virtual ~Lister ();


    /** Allows listing all objects
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_objects (sigc::slot1<bool, ObjectType &> visitor);

    /** Returns a const iterator to the first object of the collection.
     */
    const_iterator begin () const;


    /** Returns an iterator to the first object of the collection.
     */
    iterator begin ();


    /** Returns a const iterator to the first object of the collection.
     */
    const_iterator end () const;


    /** Returns an iterator to the last object of the collection.
     */
    iterator end ();

    /** Adds an object to the Ekiga::Lister.
     * @param: The object to be added.
     * @return: The Ekiga::Lister 'object_added' signal is emitted when
     *          the object has been added. The
     *          Ekiga::Lister 'object_updated' signal will be emitted
     *          when the object has been updated and the
     *          Ekiga::Lister 'object_removed' signal will be emitted when
     *          the object has been removed from the Ekiga::Lister.
     */
    void add_object (ObjectType &object);


    /** Removes an object from the Ekiga::Lister.
     * @param: The object to be removed.
     * @return: The Ekiga::Lister 'object_removed' signal is emitted when
     * the object has been removed.
     */
    void remove_object (ObjectType &object);


    /** Signals emitted by this object
     *
     */
    sigc::signal1<void, ObjectType &> object_added;
    sigc::signal1<void, ObjectType &> object_removed;
    sigc::signal1<void, ObjectType &> object_updated;

  private:

    /** Disconnects the signals for the object, emits the 'object_removed'
     * signal on the Ekiga::Lister and takes care of the release of that
     * object.
     * @param: The object to remove.
     */
    void common_removal_steps (ObjectType &object);


    /** This callback is triggered when the 'updated' signal is emitted on
     * an object.
     * Emits the Ekiga::Lister 'object_updated' signal for that object.
     * @param: The updated object.
     */
    void on_object_updated (ObjectType *object);


    /** This callback is triggered when the 'removed' signal is emitted on
     * an object.
     * Emits the Ekiga::Lister 'object_removed' signal for that object and
     * takes care of the deletion of the object.
     * @param: The removed object.
     */
    void on_object_removed (ObjectType *object);

    /** Object store.
     */
    container_type objects;

    /** Are we shutting done, hence not reacting to our objects saying goodbye?
     */
    bool shutting_down;
  };
};


/* here begins the code from the template functions */

template<typename ObjectType>
Ekiga::Lister<ObjectType>::Lister (): shutting_down(false)
{
}


template<typename ObjectType>
Ekiga::Lister<ObjectType>::~Lister ()
{
  shutting_down = true;

  for (unsigned int ii = 0;
       ii < objects.size ();
       ii++) {

    ObjectType *obj = objects[ii];
    obj->removed.emit ();
  }
}


template<typename ObjectType>
void
Ekiga::Lister<ObjectType>::visit_objects (sigc::slot1<bool, ObjectType &> visitor)
{
  bool go_on = true;
  for (unsigned int ii = 0;
       ii < objects.size () && go_on; ii++)
    go_on = visitor (*objects[ii]);
}


template<typename ObjectType>
typename Ekiga::Lister<ObjectType>::const_iterator
Ekiga::Lister<ObjectType>::begin () const
{
  return const_iterator (objects);
}


template<typename ObjectType>
typename Ekiga::Lister<ObjectType>::iterator
Ekiga::Lister<ObjectType>::begin ()
{
  return iterator (objects);
}


template<typename ObjectType>
typename Ekiga::Lister<ObjectType>::const_iterator
Ekiga::Lister<ObjectType>::end () const
{
  return const_iterator (objects, objects.size ());
}


template<typename ObjectType>
typename Ekiga::Lister<ObjectType>::iterator
Ekiga::Lister<ObjectType>::end ()
{
  return iterator (objects, objects.size ());
}


template<typename ObjectType>
void
Ekiga::Lister<ObjectType>::add_object (ObjectType &object)
{
  object.removed.connect (sigc::bind (sigc::mem_fun (this, &Lister::on_object_removed), &object));
  object.updated.connect (sigc::bind (sigc::mem_fun (this, &Lister::on_object_updated), &object));
  objects.add (&object);
  object_added.emit (object);
}


template<typename ObjectType>
void
Ekiga::Lister<ObjectType>::remove_object (ObjectType &object)
{
  common_removal_steps (object);
}

template<typename ObjectType>
void
Ekiga::Lister<ObjectType>::common_removal_steps (ObjectType &object)
{
  object_removed.emit (object);
  objects.remove (&object);
}


template<typename ObjectType>
void
Ekiga::Lister<ObjectType>::on_object_updated (ObjectType *object)
{
  object_updated.emit (*object);
}


template<typename ObjectType>
void
Ekiga::Lister<ObjectType>::on_object_removed (ObjectType *object)
{
  if ( !shutting_down)
    common_removal_steps (*object);
}

#endif
