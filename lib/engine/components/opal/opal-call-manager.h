
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
 *                         endpoint.h  -  description
 *                         --------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : This file contains the Endpoint class.
 *
 */


#ifndef _ENDPOINT_H_
#define _ENDPOINT_H_

#include <opal/buildopts.h>
#include <ptbuildopts.h>

#include <ptlib.h>

#include <opal/manager.h>
#include <opal/pcss.h>

#include <h323/h323.h>
#include <sip/sip.h>

#include "gmconf-bridge.h"
#include "runtime.h"
#include "contact-core.h"
#include "presence-core.h"
#include "call-manager.h"
#include "call.h"

#include <sigc++/sigc++.h>
#include <string>


class GMLid;
class GMPCSSEndpoint;

namespace Opal {

  class CallManager : public Ekiga::Service,
                      public Ekiga::CallManager,
                      public OpalManager
  {
    PCLASSINFO(CallManager, OpalManager);

public:

    CallManager (Ekiga::ServiceCore & _core);

    virtual ~CallManager ();

    /**/
    const std::string get_name () const
      { return "opal-component"; }

    const std::string get_description () const
      { return "\tObject bringing in Opal support (calls, text messaging, sip, h323, ...)"; }

    /** Call Manager **/
    bool dial (const std::string & uri); 

    void set_display_name (const std::string & name);
    const std::string & get_display_name () const;

    void set_echo_cancellation (bool enabled);
    bool get_echo_cancellation () const;

    void set_maximum_jitter (unsigned max_val);
    unsigned get_maximum_jitter () const;

    void set_silence_detection (bool enabled);
    bool get_silence_detection () const;

    void set_reject_delay (unsigned delay);
    unsigned get_reject_delay () const;

    void set_codecs (Ekiga::CodecList & codecs); 
    const Ekiga::CodecList & get_codecs () const;

    /* Extended stuff, OPAL CallManager specific */
    void set_forward_on_busy (bool enabled);
    bool get_forward_on_busy ();

    void set_forward_on_no_answer (bool enabled);
    bool get_forward_on_no_answer ();

    void set_unconditional_forward (bool enabled);
    bool get_unconditional_forward ();

    void set_udp_ports (unsigned min_port, 
                        unsigned max_port);

    void get_udp_ports (unsigned & min_port, 
                        unsigned & max_port) const;

    void set_tcp_ports (unsigned min_port, 
                        unsigned max_port);

    void get_tcp_ports (unsigned & min_port, 
                        unsigned & max_port) const;

    void set_stun_server (const std::string & server);
    void set_stun_enabled (bool);

    /**/
    struct VideoOptions 
      {
        VideoOptions () 
          : size (0), 
          maximum_frame_rate (0), 
          temporal_spatial_tradeoff (0), 
          maximum_received_bitrate (0), 
          maximum_transmitted_bitrate (0) {};

        unsigned size;
        unsigned maximum_frame_rate;
        unsigned temporal_spatial_tradeoff;
        unsigned maximum_received_bitrate;
        unsigned maximum_transmitted_bitrate;
      };

    void set_video_options (const VideoOptions & options);
    void get_video_options (VideoOptions & options) const;

private:
    OpalCall *CreateCall (void *uri);
    void DestroyCall (OpalCall *);

    bool OnOpenMediaStream (OpalConnection &,
                            OpalMediaStream &);

    void OnClosedMediaStream (const OpalMediaStream &);

    void GetAllowedFormats (OpalMediaFormatList & full_list);

    void HandleSTUNResult ();

    void ReportSTUNError (const std::string error);

    /* The various related endpoints */
    GMPCSSEndpoint *pcssEP;

    /* Various mutexes to ensure thread safeness around internal
       variables */
    PMutex manager_access_mutex;

    Ekiga::ServiceCore & core;
    Ekiga::CodecList codecs; 

    /* used to get the STUNDetector results */
    GAsyncQueue* queue;
    unsigned int patience;

    std::string display_name;
    std::string stun_server;
    unsigned reject_delay;
    bool forward_on_busy;
    bool unconditional_forward;
    bool forward_on_no_answer;
    bool stun_enabled;
  };
};
#endif
