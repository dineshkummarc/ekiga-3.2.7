
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
 *                         call-protocol-manager.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras 
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : Declaration of the interface of a call protocol manager
 *                          implementation backend. A call manager handles calls
 *                          thanks to various call protocol managers.
 *
 */


#ifndef __CALL_PROTOCOL_MANAGER_H__
#define __CALL_PROTOCOL_MANAGER_H__

#include <string>

#include "gmref.h"

namespace Ekiga
{

/**
 * @addtogroup calls
 * @{:
 */

  class CallProtocolManager: public virtual GmRefCounted
  {

    public:
    class Interface
    {
        public:
        std::string voip_protocol;
        std::string protocol;
        std::string id;
        bool publish;
        unsigned port;
    };


    /* The constructor
     */
    CallProtocolManager () {};

    /* The destructor
     */
    virtual ~CallProtocolManager () {};


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

    /** Return the protocol name
     * @return the protocol name
     */
    virtual const std::string & get_protocol_name () const = 0;


    /*
     * MISC
     */

    /** Return the listen interface
     * @return the interface on which we are accepting calls. Generally,
     * under the form protocol:IP:port.
     */
    virtual const Interface & get_listen_interface () const = 0;

    /** Set the DTMF mode to use to send DTMFs
     * @param mode is the desired DTMF mode
     */
    virtual void set_dtmf_mode (unsigned mode) = 0;

    /** Return the current DTMF mode
     * @return the desired DTMF mode
     */
    virtual unsigned get_dtmf_mode () const = 0;

    /** Set the port to listen to for incoming calls
     * @param port is the port on which we should bind
     */
    virtual bool set_listen_port (unsigned port) = 0;
  };

/**
 * @}
 */

};

#endif
