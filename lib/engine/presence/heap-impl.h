
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
 *                         heap-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a partial implementation
 *                          of a heap
 *
 */

#ifndef __HEAP_IMPL_H__
#define __HEAP_IMPL_H__

#include "reflister.h"
#include "heap.h"

namespace Ekiga
{

/**
 * @addtogroup presence
 * @{
 */

  /** Generic implementation for the Heap pure virtual class.
   *
   * This class is there to make it easy to implement a new type of presentity
   * heap: it will take care of implementing the external api, you
   * just have to decide when to add and remove presentities.
   *
   * Notice that this class won't take care of removing the presentity from a
   * backend -- only from the heap. If you want the presentity <b>deleted</b>
   * then you probably should have an organization like:
   *  - the presentity has a 'deleted' signal;
   *  - the heap listens for this signal;
   *  - when the signal is received, then do a remove_presentity followed by
   *    calling the appropriate api function to delete the presentity in your
   *    backend.
   */
  template<typename PresentityType = Presentity>
  class HeapImpl:
    public Heap,
    protected RefLister<PresentityType>
  {

  public:

    typedef typename RefLister<PresentityType>::iterator iterator;
    typedef typename RefLister<PresentityType>::const_iterator const_iterator;

    HeapImpl ();

    ~HeapImpl ();

    void visit_presentities (sigc::slot1<bool, PresentityPtr > visitor);

    const_iterator begin () const;

    iterator begin ();

    const_iterator end () const;

    iterator end ();

  protected:

    using RefLister<PresentityType>::add_connection;

    void add_presentity (gmref_ptr<PresentityType> presentity);

    void remove_presentity (gmref_ptr<PresentityType> presentity);
  };

/**
 * @}
 */

};

/* here are the implementations of the template methods */
template<typename PresentityType>
Ekiga::HeapImpl<PresentityType>::HeapImpl ()
{
  /* this is signal forwarding */
  RefLister<PresentityType>::object_added.connect (presentity_added.make_slot ());
  RefLister<PresentityType>::object_removed.connect (presentity_removed.make_slot ());
  RefLister<PresentityType>::object_updated.connect (presentity_updated.make_slot ());
}


template<typename PresentityType>
Ekiga::HeapImpl<PresentityType>::~HeapImpl ()
{
}

template<typename PresentityType>
void
Ekiga::HeapImpl<PresentityType>::visit_presentities (sigc::slot1<bool, PresentityPtr > visitor)
{
  RefLister<PresentityType>::visit_objects (visitor);
}

template<typename PresentityType>
typename Ekiga::HeapImpl<PresentityType>::iterator
Ekiga::HeapImpl<PresentityType>::begin ()
{
  return RefLister<PresentityType>::begin ();
}

template<typename PresentityType>
typename Ekiga::HeapImpl<PresentityType>::iterator
Ekiga::HeapImpl<PresentityType>::end ()
{
  return RefLister<PresentityType>::end ();
}

template<typename PresentityType>
typename Ekiga::HeapImpl<PresentityType>::const_iterator
Ekiga::HeapImpl<PresentityType>::begin () const
{
  return RefLister<PresentityType>::begin ();
}

template<typename PresentityType>
typename Ekiga::HeapImpl<PresentityType>::const_iterator
Ekiga::HeapImpl<PresentityType>::end () const
{
  return RefLister<PresentityType>::end ();
}

template<typename PresentityType>
void
Ekiga::HeapImpl<PresentityType>::add_presentity (gmref_ptr<PresentityType> presentity)
{
  presentity->questions.add_handler (questions.make_slot ());

  add_object (presentity);
}

template<typename PresentityType>
void
Ekiga::HeapImpl<PresentityType>::remove_presentity (gmref_ptr<PresentityType> presentity)
{
  remove_object (presentity);
}

#endif
