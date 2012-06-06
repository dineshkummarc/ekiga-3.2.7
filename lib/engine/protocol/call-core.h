
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
 *                         call-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : declaration of the interface of a call core.
 *                          A call core manages CallManagers.
 *
 */

#ifndef __CALL_CORE_H__
#define __CALL_CORE_H__

#include "form-request.h"
#include "chain-of-responsibility.h"
#include "services.h"
#include "call.h"
#include "call-manager.h"
#include "call-protocol-manager.h"
#include "gmref.h"

#include <sigc++/sigc++.h>
#include <set>
#include <map>
#include <iostream>


namespace Ekiga
{

/**
 * @defgroup calls Calls and protocols
 * @{
 */

  class CallManager;

  class CallCore
    : public Service
    {

  public:
      typedef std::set<gmref_ptr<CallManager> >::iterator iterator;
      typedef std::set<gmref_ptr<CallManager> >::const_iterator const_iterator;

      /** The constructor
       */
      CallCore () { nr_ready = 0; }

      /** The destructor
       */
      ~CallCore ();


      /*** Service Implementation ***/

      /** Returns the name of the service.
       * @return The service name.
       */
      const std::string get_name () const
        { return "call-core"; }


      /** Returns the description of the service.
       * @return The service description.
       */
      const std::string get_description () const
        { return "\tCall Core managing Call Manager objects"; }


      /** Adds a call handled by the CallCore serice.
       * @param call is the call to be added.
       * @param manager is the CallManager handling it.
       */
      void add_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);

      /** Remove a call handled by the CallCore serice.
       * @param call is the call to be removed.
       */
      void remove_call (gmref_ptr<Call> call);

      /** Adds a CallManager to the CallCore service.
       * @param The manager to be added.
       */
      void add_manager (gmref_ptr<CallManager> manager);

      /** Return iterator to beginning
       * @return iterator to beginning
       */
      iterator begin ();
      const_iterator begin () const;

      /** Return iterator to end
       * @return iterator to end 
       */
      iterator end ();
      const_iterator end () const;

      /** This signal is emitted when a Ekiga::CallManager has been
       * added to the CallCore Service.
       */
      sigc::signal1<void, gmref_ptr<CallManager> > manager_added;


      /*** Call Management ***/                 

      /** Create a call based on the remote uri given as parameter
       * @param: an uri to call
       * @return: true if a Ekiga::Call could be created
       */
      bool dial (const std::string uri); 

      /*** Call Related Signals ***/
      
      /** See call.h for the API
       */
      sigc::signal2<void, gmref_ptr<CallManager> , gmref_ptr<Call> > ringing_call;
      sigc::signal2<void, gmref_ptr<CallManager> , gmref_ptr<Call> > setup_call;
      sigc::signal2<void, gmref_ptr<CallManager> , gmref_ptr<Call> > missed_call;
      sigc::signal3<void, gmref_ptr<CallManager> , gmref_ptr<Call>, std::string> cleared_call;
      sigc::signal2<void, gmref_ptr<CallManager> , gmref_ptr<Call> > established_call;
      sigc::signal2<void, gmref_ptr<CallManager> , gmref_ptr<Call> > held_call;
      sigc::signal2<void, gmref_ptr<CallManager> , gmref_ptr<Call> > retrieved_call;
      sigc::signal5<void, gmref_ptr<CallManager> , gmref_ptr<Call>, std::string, Call::StreamType, bool> stream_opened;
      sigc::signal5<void, gmref_ptr<CallManager> , gmref_ptr<Call>, std::string, Call::StreamType, bool> stream_closed;
      sigc::signal4<void, gmref_ptr<CallManager> , gmref_ptr<Call>, std::string, Call::StreamType> stream_paused;
      sigc::signal4<void, gmref_ptr<CallManager> , gmref_ptr<Call>, std::string, Call::StreamType> stream_resumed;

      /*** Misc ***/
      sigc::signal1<void, gmref_ptr<CallManager> > manager_ready;
      sigc::signal0<void> ready;

      /** This chain allows the CallCore to report errors to the user
       */
      ChainOfResponsibility<std::string> errors;

  private:
      void on_new_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_ringing_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_setup_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_missed_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_cleared_call (std::string, gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_established_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_held_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_retrieved_call (gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_stream_opened (std::string name, Call::StreamType type, bool is_transmitting, gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_stream_closed (std::string name, Call::StreamType type, bool is_transmitting, gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_stream_paused (std::string name, Call::StreamType type, gmref_ptr<Call> call, gmref_ptr<CallManager> manager);
      void on_stream_resumed (std::string name, Call::StreamType type, gmref_ptr<Call> call, gmref_ptr<CallManager> manager);

      void on_im_failed (std::string, std::string, gmref_ptr<CallManager> manager);
      void on_im_sent (std::string, std::string, gmref_ptr<CallManager> manager);
      void on_im_received (std::string, std::string, std::string, gmref_ptr<CallManager> manager);
      void on_new_chat (std::string, std::string, gmref_ptr<CallManager> manager);

      void on_manager_ready (gmref_ptr<CallManager> manager);

      void on_call_removed (gmref_ptr<Call> call);

      
      std::set<gmref_ptr<CallManager> > managers;
      std::list<sigc::connection> manager_connections;
      std::map<std::string, std::list<sigc::connection> > call_connections;
      unsigned nr_ready;
    };

/**
 * @}
 */

};


#endif
