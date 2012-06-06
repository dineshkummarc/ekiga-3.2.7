
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
 *                         call-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras 
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : Declaration of the interface of a call manager
 *                          implementation backend. A call manager handles calls,
 *                          sometimes simultaneously.
 *
 */


#ifndef __CALL_MANAGER_H__
#define __CALL_MANAGER_H__

#include <set>
#include <sigc++/sigc++.h>

#include "gmref.h"

#include "call-protocol-manager.h"
#include "codec-description.h"

namespace Ekiga
{

/**
 * @addtogroup calls
 * @{
 */

  class CallManager : public virtual GmRefCounted
  {

    public:

    typedef std::list<CallProtocolManager::Interface> InterfaceList;
    typedef std::set<gmref_ptr<CallProtocolManager> >::iterator iterator;
    typedef std::set<gmref_ptr<CallProtocolManager> >::const_iterator const_iterator;

    /* The constructor
     */
    CallManager () {};

    /* The destructor
     */
    virtual ~CallManager () {}

    /** Add a CallProtocolManager to the CallManager.
     * @param The manager to be added.
     */
    void add_protocol_manager (gmref_ptr<CallProtocolManager> manager);

    /** Return a pointer to a CallProtocolManager of the CallManager.
     * @param protocol is the protcol name.
     * @return a pointer to the CallProtocolManager or NULL if none.
     */
    gmref_ptr<CallProtocolManager> get_protocol_manager (const std::string &protocol) const;

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

    /** This signal is emitted when a Ekiga::CallProtocolManager has been
     * added to the CallManager.
     */
    sigc::signal1<void, gmref_ptr<CallProtocolManager> > manager_added;


    /*                 
     * CALL MANAGEMENT 
     */              

    /** Create a call based on the remote uri given as parameter
     * @param: An uri
     * @return: true if a Ekiga::Call could be created
     */
    virtual bool dial (const std::string & uri) = 0; 


    /*
     * PROTOCOL INFORMATION
     */

    /**
     * @return the protocol name
     */
    const std::list<std::string> get_protocol_names () const;

    /**
     * @return the interface on which we are accepting calls. Generally,
     * under the form protocol:IP:port.
     */
    const CallManager::InterfaceList get_interfaces () const;


    /*
     * Misc
     */

    /** Enable the given codecs
     * @param codecs is a set of the codecs and their descriptions
     *        when the function returns, the list also contains disabled
     *        codecs supported by the CallManager. Unsupported codecs 
     *        have been removed.
     */
    virtual void set_codecs (CodecList & codecs) = 0; 

    /** Return the list of available codecs
     * @return a set of the codecs and their descriptions
     */
    virtual const Ekiga::CodecList & get_codecs () const = 0;

    /** Set the display name used on outgoing calls
     * @param name is the display name to use.
     */
    virtual void set_display_name (const std::string & name) = 0;

    /** Return the display name used on outgoing calls
     */
    virtual const std::string & get_display_name () const = 0;

    /** Enable echo cancellation
     * @param enabled is true if echo cancellation should be enabled, false
     * otherwise.
     */
    virtual void set_echo_cancellation (bool enabled) = 0;

    /** Get echo cancellation setting
     * @return true if echo cancellation is enabled.
     */
    virtual bool get_echo_cancellation () const = 0;

    /** Enable silence detection
     * @param enabled is true if silence detection should be enabled, false
     * otherwise.
     */
    virtual void set_silence_detection (bool enabled) = 0;

    /** Get silence detection setting
     * @return true if silence detection is enabled.
     */
    virtual bool get_silence_detection () const = 0;

    /** Set maximum jitter 
     * @param max_val is the maximum jitter for calls in seconds.
     */
    virtual void set_maximum_jitter (unsigned max_val) = 0;

    /** Get maximum jitter 
     * @return the maximum jitter for calls in seconds.
     */
    virtual unsigned get_maximum_jitter () const = 0;

    /** Set delay before dropping an incoming call 
     * @param delay is the delay after which the call should be rejected
     * (or forwarded if supported by the CallManager).
     */
    virtual void set_reject_delay (unsigned delay) = 0;

    /** Get delay before dropping an incoming call
     * @return the delay in seconds after which a call should be rejected
     * (or forwarded if supported by the CallManager).
     */
    virtual unsigned get_reject_delay () const = 0;

    /*
     * MISC
     */
    sigc::signal0<void> ready;

    private:
    std::set<gmref_ptr<CallProtocolManager> > managers;
  };

  /**
   * @}
   */
};
#endif
