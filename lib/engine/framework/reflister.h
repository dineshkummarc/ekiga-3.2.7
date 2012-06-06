
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
 *                         reflister.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : declaration of an object able to list gmref_ptr
 *                          objects
 *
 */

#ifndef __REFLISTER_H__
#define __REFLISTER_H__

#include <sigc++/sigc++.h>
#include <list>

#include "gmref.h"
#include "map-key-iterator.h"
#include "map-key-const-iterator.h"

namespace Ekiga
{
  template<typename ObjectType>
  class RefLister
  {
  protected:

    typedef std::map<gmref_ptr<ObjectType>,std::list<sigc::connection> > container_type;
    typedef Ekiga::map_key_iterator<container_type> iterator;
    typedef Ekiga::map_key_const_iterator<container_type> const_iterator;

    virtual ~RefLister ();

    void visit_objects (sigc::slot1<bool, gmref_ptr<ObjectType> > visitor);

    void add_object (gmref_ptr<ObjectType> obj);

    void add_connection (gmref_ptr<ObjectType> obj,
			 sigc::connection connection);

    void remove_object (gmref_ptr<ObjectType> obj);

    void remove_all_objects ();

    iterator begin ();
    iterator end ();

    const_iterator begin () const;
    const_iterator end () const;

    sigc::signal1<void, gmref_ptr<ObjectType> > object_added;
    sigc::signal1<void, gmref_ptr<ObjectType> > object_removed;
    sigc::signal1<void, gmref_ptr<ObjectType> > object_updated;

  private:
    container_type objects;
  };

};

template<typename ObjectType>
Ekiga::RefLister<ObjectType>::~RefLister ()
{
  for (typename container_type::iterator iter = objects.begin ();
       iter != objects.end ();
       ++iter) {

    for (std::list<sigc::connection>::iterator conn_iter = iter->second.begin ();
	 conn_iter != iter->second.end ();
	 ++conn_iter) {

      conn_iter->disconnect ();
    }
  }
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::visit_objects (sigc::slot1<bool, gmref_ptr<ObjectType> > visitor)
{
  bool go_on = true;
  for (typename container_type::iterator iter = objects.begin ();
       go_on && iter != objects.end ();
       ++iter)
    go_on = visitor (iter->first);
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::add_object (gmref_ptr<ObjectType> obj)
{
  objects[obj].push_back (obj->updated.connect (sigc::bind (object_updated.make_slot (), obj)));
  objects[obj].push_back (obj->removed.connect (sigc::bind (sigc::mem_fun (this, &Ekiga::RefLister<ObjectType>::remove_object), obj)));

  object_added.emit (obj);
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::add_connection (gmref_ptr<ObjectType> obj,
					      sigc::connection connection)
{
  objects[obj].push_back (connection);
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::remove_object (gmref_ptr<ObjectType> obj)
{
  std::list<sigc::connection> connections = objects[obj];
  for (std::list<sigc::connection>::iterator iter = connections.begin ();
       iter != connections.end ();
       ++iter)
    iter->disconnect ();
  objects.erase (objects.find (obj));
  object_removed.emit (obj);
}

template<typename ObjectType>
void
Ekiga::RefLister<ObjectType>::remove_all_objects ()
{
  /* iterators get invalidated as we go, hence the strange loop */
  while (objects.begin () != objects.end ())
    remove_object (objects.begin ()->first);
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::iterator
Ekiga::RefLister<ObjectType>::begin ()
{
  return iterator (objects.begin ());
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::iterator
Ekiga::RefLister<ObjectType>::end ()
{
  return iterator (objects.end ());
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::const_iterator
Ekiga::RefLister<ObjectType>::begin () const
{
  return const_iterator (objects.begin ());
}

template<typename ObjectType>
typename Ekiga::RefLister<ObjectType>::const_iterator
Ekiga::RefLister<ObjectType>::end () const
{
  return const_iterator (objects.end ());
}

#endif
