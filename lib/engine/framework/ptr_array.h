
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
 *                         ptr_array.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of an array container for pointers
 *
 */

#ifndef __PTR_ARRAY_H__
#define __PTR_ARRAY_H__ 1

namespace Ekiga
{

  template<typename ObjectType>
  class ptr_array
  {
  public:

    ptr_array();

    ~ptr_array();

    void add (ObjectType* obj);

    void remove (unsigned int pos);

    void remove (ObjectType* obj);

    unsigned int size () const;

    ObjectType* operator[](unsigned int pos) const;

  private:

    unsigned int reserved_size;
    unsigned int used_size;
    ObjectType **store;
  };

};

template<typename ObjectType>
Ekiga::ptr_array<ObjectType>::ptr_array ()
{
  reserved_size = 10;
  used_size = 0;
  store = new ObjectType*[reserved_size];
}

template<typename ObjectType>
Ekiga::ptr_array<ObjectType>::~ptr_array ()
{
  for (unsigned int ii = 0; ii < used_size; ii++)
    delete store[ii];
  delete [] store;
}

template<typename ObjectType>
void
Ekiga::ptr_array<ObjectType>::add (ObjectType* obj)
{
  if (used_size >= reserved_size) {

    ObjectType** new_store = new ObjectType*[reserved_size + 10];
    for (unsigned int ii = 0 ; ii < used_size ; ii++)
      new_store[ii] = store[ii];
    reserved_size += 10;
    delete [] store;
    store = new_store;
  }

  store[used_size] = obj;
  used_size++;
}

template<typename ObjectType>
void
Ekiga::ptr_array<ObjectType>::remove (unsigned int pos)
{
  if (pos < used_size) {

    delete store[pos];
    for (unsigned int jj = pos; jj < used_size - 1; jj++)
      store[jj] = store[jj+1];
    used_size--;
  }
}

template<typename ObjectType>
void
Ekiga::ptr_array<ObjectType>::remove (ObjectType* obj)
{
  unsigned int ii;

  for (ii = 0; ii < used_size; ii++)
    if (store[ii] == obj)
      break;

  remove (ii);
}

template<typename ObjectType>
unsigned int
Ekiga::ptr_array<ObjectType>::size () const
{
  return used_size;
}

template<typename ObjectType>
ObjectType*
Ekiga::ptr_array<ObjectType>::operator[](unsigned int pos) const
{
  if (pos < used_size)
    return store[pos];
  else
    return 0;
}

#endif
