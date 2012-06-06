
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
 *                         ptr_array_const_iterator.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of const_iterator for ptr_array
 *
 */

#ifndef __ARRAY_CONST_ITERATOR_H__
#define __ARRAY_CONST_ITERATOR_H__ 1

#include <iterator>

#include "ptr_array.h"

namespace Ekiga
{

  template<typename ObjectType>
  class ptr_array_const_iterator: public std::forward_iterator_tag
  {
  public:

    ptr_array_const_iterator (const ptr_array<ObjectType>& arr_,
			      unsigned int pos_ = 0);

    ptr_array_const_iterator& operator= (const ptr_array_const_iterator &other);

    bool operator== (const ptr_array_const_iterator &other);

    bool operator!= (const ptr_array_const_iterator &other);

    ptr_array_const_iterator &operator++ ();

    ptr_array_const_iterator operator++(int);

    const ObjectType& operator* ();

    const ObjectType* operator-> ();

    bool is_valid () const;

    void set_position (unsigned int pos_);

  private:

    const ptr_array<ObjectType>* arr;
    unsigned int pos;
  };

};

template<typename ObjectType>
Ekiga::ptr_array_const_iterator<ObjectType>::ptr_array_const_iterator (const ptr_array<ObjectType>& arr_, unsigned int pos_)
  : arr(&arr_), pos(pos_)
{
}

template<typename ObjectType>
Ekiga::ptr_array_const_iterator<ObjectType>&
Ekiga::ptr_array_const_iterator<ObjectType>::operator= (const ptr_array_const_iterator<ObjectType> &other)
{
  arr = other.arr;
  pos = other.pos;
  return *this;
}


template<typename ObjectType>
bool
Ekiga::ptr_array_const_iterator<ObjectType>::operator== (const ptr_array_const_iterator<ObjectType> &other)
{
  return (arr == other.arr && pos == other.pos);
}

template<typename ObjectType>
bool
Ekiga::ptr_array_const_iterator<ObjectType>::operator!= (const ptr_array_const_iterator<ObjectType> &other)
{
  return !(arr == other.arr && pos == other.pos);
}

template<typename ObjectType>
Ekiga::ptr_array_const_iterator<ObjectType>&
Ekiga::ptr_array_const_iterator<ObjectType>::operator++ ()
{
  pos++;
  return *this;
}

template<typename ObjectType>
Ekiga::ptr_array_const_iterator<ObjectType>
Ekiga::ptr_array_const_iterator<ObjectType>::operator++(int)
{
  ptr_array_const_iterator<ObjectType> tmp = *this;
  pos++;
  return tmp;
}

template<typename ObjectType>
const ObjectType&
Ekiga::ptr_array_const_iterator<ObjectType>::operator* ()
{
  return *(*arr)[pos];
}

template<typename ObjectType>
const ObjectType*
Ekiga::ptr_array_const_iterator<ObjectType>::operator-> ()
{
  return (*arr)[pos];
}

template<typename ObjectType>
bool
Ekiga::ptr_array_const_iterator<ObjectType>::is_valid () const
{
  return (pos < arr->size());
}

template<typename ObjectType>
void
Ekiga::ptr_array_const_iterator<ObjectType>::set_position (unsigned int pos_)
{
  pos = pos_;
}

#endif
