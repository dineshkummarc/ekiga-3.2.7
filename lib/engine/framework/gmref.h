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
 *                         gmref.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : Reference-counted memory management helpers
 *
 */

#ifndef __GMREF_H__
#define __GMREF_H__

/* This is a reference-counted pointer class ; all it asks to the wrapped
 * object is a pair of reference/unreference methods.
 *
 * This class is thread-safe in the weak sense : you can use gmref_ptr from
 * any thread -- as long as you don't cross the thread boundary with a same
 * instance.
 *
 * See later down for an helper to implement refcounted objects.
 */

template<typename T>
class gmref_ptr
{
public:

  gmref_ptr ();

  ~gmref_ptr ();

  explicit gmref_ptr (T* obj_);

  gmref_ptr (const gmref_ptr& ptr);

  template<typename Tprim> gmref_ptr (const gmref_ptr<Tprim>& ptr);

  gmref_ptr& operator= (const gmref_ptr& other);

  T* operator-> () const;

  T& operator* () const;

  T* get () const;

  void swap (gmref_ptr& other);

  void reset ();

  operator bool () const;

private:

  template<typename Tprim> friend class gmref_ptr;

  T* obj;
};

template<typename T> bool operator==(const gmref_ptr<T>& a,
				     const gmref_ptr<T>& b);

template<typename T> bool operator!=(const gmref_ptr<T>& a,
				     const gmref_ptr<T>& b);

template<typename T> bool operator<(const gmref_ptr<T>& a,
				    const gmref_ptr<T>& b);

/* base class for a reference counted object
 */
class GmRefCounted
{
public:
  GmRefCounted (): refcount(0)
  {}

  GmRefCounted (const GmRefCounted& /*other*/): refcount(0)
  {}

  GmRefCounted& operator= (const GmRefCounted& /*other*/)
  { return *this; }

  virtual ~GmRefCounted ()
  {}

  void reference () const { ++refcount; }

  void unreference () const { if (--refcount == 0) delete this; }

private:
  mutable int refcount;
};

/* implementation of the templates */

template<typename T>
gmref_ptr<T>::gmref_ptr (): obj(0)
{
}

template<typename T>
gmref_ptr<T>::~gmref_ptr ()
{
  reset ();
}

template<typename T>
gmref_ptr<T>::gmref_ptr (T* obj_): obj(obj_)
{
  if (obj != 0)
    obj->reference ();
}

template<typename T>
gmref_ptr<T>::gmref_ptr (const gmref_ptr<T>& ptr): obj(ptr.obj)
{
  if (obj != 0)
    obj->reference ();
}

template<typename T>
template<typename Tprim>
gmref_ptr<T>::gmref_ptr (const gmref_ptr<Tprim>& ptr): obj(dynamic_cast<T*>(ptr.obj))
{
  if (obj != 0)
    obj->reference ();
}

template<typename T>
gmref_ptr<T>&
gmref_ptr<T>::operator= (const gmref_ptr<T>& other)
{
  gmref_ptr<T> temp(other);
  this->swap (temp);

  return *this;
}

template<typename T>
T*
gmref_ptr<T>::operator-> () const
{
  return obj;
}

template<typename T>
T&
gmref_ptr<T>::operator* () const
{
  return *obj;
}

template<typename T>
T*
gmref_ptr<T>::get () const
{
  return obj;
}

template<typename T>
void
gmref_ptr<T>::swap (gmref_ptr<T>& other)
{
  T* temp = obj;
  obj = other.obj;
  other.obj = temp;
}

template<typename T>
gmref_ptr<T>::operator bool () const
{
  return obj != 0;
}

template<typename T>
void
gmref_ptr<T>::reset ()
{
  if (obj != 0)
    obj->unreference ();
  obj = 0;
}

template<typename T>
bool
operator==(const gmref_ptr<T>& a,
	   const gmref_ptr<T>& b)
{
  return a.get () == b.get ();
}

template<typename T>
bool operator!=(const gmref_ptr<T>& a,
		const gmref_ptr<T>& b)
{
  return !operator==(a, b);
}

template<typename T>
bool
operator<(const gmref_ptr<T>& a,
	  const gmref_ptr<T>& b)
{
  return a.get () < b.get ();
}

#endif
