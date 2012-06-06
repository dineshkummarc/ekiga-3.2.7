
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
 *                         chain-of-responsibility.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : a chain of responsibility implementation
 *
 */

#ifndef __CHAIN_OF_RESPONSIBILITY_H__
#define __CHAIN_OF_RESPONSIBILITY_H__

#include <sigc++/sigc++.h>

/* This code uses sigc++ signals to implement the "chain of responsibility"
 * design pattern -- big words, but simple concept!
 *
 * BASIC IDEA
 *
 * Here is the situation you're interested in : one of your objects sometimes
 * needs to get some kind of requests processed, but doesn't have the required
 * competences to do it itself.
 *
 * The obvious solution is to have handlers register to your object, which
 * will try to process requests on its behalf. Trying to use signals and slots
 * to achieve that is quite natural too, but leads to the problem that a
 * signal is one-to-many, so using them directly will lead to the bad situation
 * where all requests get handled not once, but once by each handler!
 *
 * PRINCIPLE
 *
 * So we need to adapt the signal and slot to make sure only one handler will
 * really process a request. This is done by making the slots return a boolean,
 * so we know if the request was really processed or not. If it wasn't, the
 * next handler is polled -- if it was, the treatment stops.
 *
 *
 * HOW TO USE
 *
 * A request has type T_request, the object will declare :
 * Ekiga::ChainOfResponsibility<T_request> chain;
 *
 * The handlers will register like this :
 * chain.add_handler (sigc::mem_fun (this, &Foo::request_handler));
 *
 * A request handler looks like :
 * bool
 * Foo::request_handler (const T_request request)
 * {
 *   if (can_handle_request (request)) {
 *
 *     <handle request here>
 *     return true;
 *   else
 *     return false;
 * }
 *
 * Trying to get a request handled looks like :
 * chain.handle_request (request);
 *
 * or for better error reporting :
 * if (!chain.handle_request (request)) {
 *
 *   <report that you couldn't deal with a request>
 * }
 *
 */

namespace Ekiga
{

  struct responsibility_accumulator
  {
    typedef bool result_type;

    template <typename T_slot_iterator>
    bool operator() (T_slot_iterator begin, T_slot_iterator end)
    {
      bool result = false;

      for (T_slot_iterator iter = begin ;
	   iter != end && !result;
	   iter++)
	result = *iter;

      return result;
    }
  };

  template<typename T_request>
  struct ChainOfResponsibility:
    public sigc::signal1<bool,
			 T_request,
			 responsibility_accumulator>
  {
    typedef typename sigc::signal1<bool,
				   T_request,
				   responsibility_accumulator>::iterator iterator;
    typedef typename sigc::signal1<bool,
				   T_request,
				   responsibility_accumulator>::slot_type slot_type;

    iterator add_handler (const slot_type& slot_)
    { return this->connect (slot_); }

    bool handle_request (typename sigc::type_trait<T_request>::take request)
    { return this->emit (request); }

  };
};

#endif
