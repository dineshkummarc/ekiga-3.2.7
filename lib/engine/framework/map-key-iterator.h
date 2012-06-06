
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
 *                         map-key-iterator.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : iterator on the keys of a std::map
 *
 */

#ifndef __MAP_KEY_ITERATOR_H__
#define __MAP_KEY_ITERATOR_H__

#include <map>

namespace Ekiga
{
  template<typename map_type>
  class map_key_iterator: public std::forward_iterator_tag
  {
  public:

    map_key_iterator (typename map_type::iterator it_);

    map_key_iterator& operator= (const map_key_iterator& other);

    bool operator== (const map_key_iterator& other);
    bool operator!= (const map_key_iterator& other);

    map_key_iterator& operator++ ();
    map_key_iterator operator++ (int);

    typename map_type::key_type operator* ();

    typename map_type::key_type* operator-> ();

  private:
    typename map_type::iterator it;
  };

};

template<typename map_type>
Ekiga::map_key_iterator<map_type>::map_key_iterator (typename map_type::iterator it_): it(it_)
{
}

template<typename map_type>
Ekiga::map_key_iterator<map_type>&
Ekiga::map_key_iterator<map_type>::operator= (const map_key_iterator<map_type>& other)
{
  it = other.it;
  return *this;
}

template<typename map_type>
bool
Ekiga::map_key_iterator<map_type>::operator== (const map_key_iterator<map_type>& other)
{
  return it == other.it;
}

template<typename map_type>
bool
Ekiga::map_key_iterator<map_type>::operator!= (const map_key_iterator<map_type>& other)
{
  return it != other.it;
}

template<typename map_type>
Ekiga::map_key_iterator<map_type>&
Ekiga::map_key_iterator<map_type>::operator++ ()
{
  it++;
  return *this;
}

template<typename map_type>
Ekiga::map_key_iterator<map_type>
Ekiga::map_key_iterator<map_type>::operator++ (int)
{
  map_key_iterator<map_type> tmp = *this;
  ++it;
  return tmp;
}

template<typename map_type>
typename map_type::key_type
Ekiga::map_key_iterator<map_type>::operator* ()
{
  return it->first;
}

template<typename map_type>
typename map_type::key_type*
Ekiga::map_key_iterator<map_type>::operator-> ()
{
  return it->first;
}

#endif
