
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
 *                         cluster-impl.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : partial implementation of a cluster
 *
 */

#ifndef __CLUSTER_IMPL_H__
#define __CLUSTER_IMPL_H__

#include <vector>

#include "reflister.h"
#include "cluster.h"

namespace Ekiga
{

/**
 * @addtogroup presence
 * @{
 */

  /** Generic implementation for the Ekiga::Cluster abstract class.
   *
   * This class is there to make it easy to implement a new type of
   * cluster: it will take care of implementing the external api, you
   * just have to decide when to add and remove heaps.
   *
   * Notice that this class won't take care of removing the heap from a
   * backend -- only from the cluster. If you want the heap <b>deleted</b> then
   * you probably should have an organization like:
   *  - the heap has a 'deleted' signal;
   *  - the cluster listens for this signal;
   *  - when the signal is received, then do a remove_heap followed by calling
   *    the appropriate api function to delete the Heap in your backend.
   */


  template<typename HeapType = Heap>
  class ClusterImpl:
    public Cluster,
    protected RefLister<HeapType>
  {

  public:

    typedef typename RefLister<HeapType>::iterator iterator;
    typedef typename RefLister<HeapType>::const_iterator const_iterator;

    ClusterImpl ();

    virtual ~ClusterImpl ();

    void visit_heaps (sigc::slot1<bool, HeapPtr > visitor);

  protected:

    void add_heap (gmref_ptr<HeapType> heap);

    void remove_heap (gmref_ptr<HeapType> heap);

    using RefLister<HeapType>::add_connection;

    iterator begin ();
    iterator end ();
    const_iterator begin () const;
    const_iterator end () const;

  private:

    void common_removal_steps (gmref_ptr<HeapType> heap);

    void on_presentity_added (PresentityPtr presentity, gmref_ptr<HeapType> heap);

    void on_presentity_updated (PresentityPtr presentity, gmref_ptr<HeapType> heap);

    void on_presentity_removed (PresentityPtr presentity, gmref_ptr<HeapType> heap);
  };

/**
 * @}
 */

};

/* here are the implementations of the template methods */

template<typename HeapType>
Ekiga::ClusterImpl<HeapType>::ClusterImpl ()
{
  /* signal forwarding */
  RefLister<HeapType>::object_added.connect (heap_added.make_slot ());
  RefLister<HeapType>::object_removed.connect (heap_removed.make_slot ());
  RefLister<HeapType>::object_updated.connect (heap_updated.make_slot ());
}

template<typename HeapType>
Ekiga::ClusterImpl<HeapType>::~ClusterImpl ()
{
}

template<typename HeapType>
void
Ekiga::ClusterImpl<HeapType>::visit_heaps (sigc::slot1<bool, HeapPtr > visitor)
{
  RefLister<HeapType>::visit_objects (visitor);
}

template<typename HeapType>
void
Ekiga::ClusterImpl<HeapType>::add_heap (gmref_ptr<HeapType> heap)
{
  add_connection (heap, heap->presentity_added.connect (sigc::bind (sigc::mem_fun (this, &ClusterImpl::on_presentity_added), heap)));

  add_connection (heap, heap->presentity_updated.connect (sigc::bind (sigc::mem_fun (this, &ClusterImpl::on_presentity_updated), heap)));

  add_connection (heap, heap->presentity_removed.connect (sigc::bind (sigc::mem_fun (this, &ClusterImpl::on_presentity_removed), heap)));

  add_connection (heap, heap->questions.add_handler (questions.make_slot ()));

  add_object (heap);
}

template<typename HeapType>
void
Ekiga::ClusterImpl<HeapType>::remove_heap (gmref_ptr<HeapType> heap)
{
  remove_object (heap);
}

template<typename HeapType>
void
Ekiga::ClusterImpl<HeapType>::on_presentity_added (PresentityPtr presentity, gmref_ptr<HeapType> heap)
{
  presentity_added.emit (heap, presentity);
}

template<typename HeapType>
void
Ekiga::ClusterImpl<HeapType>::on_presentity_updated (PresentityPtr presentity, gmref_ptr<HeapType> heap)
{
  presentity_updated.emit (heap, presentity);
}

template<typename HeapType>
void
Ekiga::ClusterImpl<HeapType>::on_presentity_removed (PresentityPtr presentity, gmref_ptr<HeapType> heap)
{
  presentity_removed.emit (heap, presentity);
}

template<typename HeapType>
typename Ekiga::ClusterImpl<HeapType>::iterator
Ekiga::ClusterImpl<HeapType>::begin ()
{
  return RefLister<HeapType>::begin ();
}

template<typename HeapType>
typename Ekiga::ClusterImpl<HeapType>::const_iterator
Ekiga::ClusterImpl<HeapType>::begin () const
{
  return RefLister<HeapType>::begin ();
}

template<typename HeapType>
typename Ekiga::ClusterImpl<HeapType>::iterator
Ekiga::ClusterImpl<HeapType>::end ()
{
  return RefLister<HeapType>::end ();
}

template<typename HeapType>
typename Ekiga::ClusterImpl<HeapType>::const_iterator
Ekiga::ClusterImpl<HeapType>::end () const
{
  return RefLister<HeapType>::end ();
}

#endif
