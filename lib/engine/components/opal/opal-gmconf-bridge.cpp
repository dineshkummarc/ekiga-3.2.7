
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
 *                         opal-gmconf-bridge.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of an object able to do the bridging
 *                          between gmconf and opal
 *
 */

#include <iostream>
#include <sigc++/sigc++.h>

#include "config.h"

#include "gmconf.h"

#include "opal-gmconf-bridge.h"
#include "opal-call-manager.h"

#include "sip-endpoint.h"

#ifdef HAVE_H323
#include "h323-endpoint.h"
#endif

#define AUDIO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/audio/"
#define VIDEO_DEVICES_KEY "/apps/" PACKAGE_NAME "/devices/video/"
#define PERSONAL_DATA_KEY "/apps/" PACKAGE_NAME "/general/personal_data/"
#define CALL_OPTIONS_KEY "/apps/" PACKAGE_NAME "/general/call_options/"
#define NAT_KEY "/apps/" PACKAGE_NAME "/general/nat/"
#define PROTOCOLS_KEY "/apps/" PACKAGE_NAME "/protocols/"
#define H323_KEY "/apps/" PACKAGE_NAME "/protocols/h323/"
#define SIP_KEY "/apps/" PACKAGE_NAME "/protocols/sip/"
#define PORTS_KEY "/apps/" PACKAGE_NAME "/protocols/ports/"
#define CALL_FORWARDING_KEY "/apps/" PACKAGE_NAME "/protocols/call_forwarding/"
#define AUDIO_CODECS_KEY "/apps/" PACKAGE_NAME "/codecs/audio/"
#define VIDEO_CODECS_KEY  "/apps/" PACKAGE_NAME "/codecs/video/"

using namespace Opal;


ConfBridge::ConfBridge (Ekiga::Service & _service)
 : Ekiga::ConfBridge (_service)
{
  Ekiga::ConfKeys keys;
  property_changed.connect (sigc::mem_fun (this, &ConfBridge::on_property_changed));

  keys.push_back (PORTS_KEY "udp_port_range");
  keys.push_back (PORTS_KEY "tcp_port_range");

  keys.push_back (AUDIO_CODECS_KEY "enable_silence_detection");
  keys.push_back (AUDIO_CODECS_KEY "enable_echo_cancelation");

  keys.push_back (AUDIO_CODECS_KEY "media_list");
  keys.push_back (VIDEO_CODECS_KEY "media_list");

  keys.push_back (AUDIO_CODECS_KEY "maximum_jitter_buffer");

  keys.push_back (VIDEO_CODECS_KEY "maximum_video_tx_bitrate");
  keys.push_back (VIDEO_CODECS_KEY "maximum_video_rx_bitrate");
  keys.push_back (VIDEO_CODECS_KEY "temporal_spatial_tradeoff");
  keys.push_back (VIDEO_DEVICES_KEY "size"); 
  keys.push_back (VIDEO_DEVICES_KEY "max_frame_rate");

  keys.push_back (SIP_KEY "forward_host"); 
  keys.push_back (SIP_KEY "outbound_proxy_host");
  keys.push_back (SIP_KEY "dtmf_mode");
  keys.push_back (SIP_KEY "binding_timeout");

  keys.push_back (PERSONAL_DATA_KEY "full_name");

  keys.push_back (CALL_FORWARDING_KEY "forward_on_no_answer");
  keys.push_back (CALL_FORWARDING_KEY "forward_on_busy");
  keys.push_back (CALL_FORWARDING_KEY "always_forward");
  keys.push_back (CALL_OPTIONS_KEY "no_answer_timeout");

  keys.push_back (H323_KEY "enable_h245_tunneling");
  keys.push_back (H323_KEY "enable_early_h245");
  keys.push_back (H323_KEY "enable_fast_start");
  keys.push_back (H323_KEY "dtmf_mode");
  keys.push_back (H323_KEY "forward_host"); 

  keys.push_back (NAT_KEY "stun_server");
  keys.push_back (NAT_KEY "disable_stun");

  load (keys);
}


void ConfBridge::on_property_changed (std::string key, GmConfEntry *entry)
{
  CallManager & manager = (CallManager &) service;

  //
  // Video options
  //
  if (key == VIDEO_CODECS_KEY "maximum_video_tx_bitrate") {

    CallManager::VideoOptions options;
    manager.get_video_options (options);
    options.maximum_transmitted_bitrate = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_CODECS_KEY "temporal_spatial_tradeoff") {

    CallManager::VideoOptions options;
    manager.get_video_options (options);
    options.temporal_spatial_tradeoff = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_DEVICES_KEY "size") {

    CallManager::VideoOptions options;
    manager.get_video_options (options);
    options.size = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }
  else if (key == VIDEO_DEVICES_KEY "max_frame_rate") {

    CallManager::VideoOptions options;
    manager.get_video_options (options);
    options.maximum_frame_rate = gm_conf_entry_get_int (entry);
    if ( (options.maximum_frame_rate < 1) || (options.maximum_frame_rate > 30) ) {
      PTRACE(1, "OpalConfBridge\t" << VIDEO_DEVICES_KEY "max_frame_rate" << " out of range, ajusting to 30");
      options.maximum_frame_rate = 30;
    }
    manager.set_video_options (options);
  }
  else if (key == VIDEO_CODECS_KEY "maximum_video_rx_bitrate") {

    CallManager::VideoOptions options;
    manager.get_video_options (options);
    options.maximum_received_bitrate = gm_conf_entry_get_int (entry);
    manager.set_video_options (options);
  }

  //
  // NAT Key
  //
  else if (key == NAT_KEY "stun_server") {

    const char *stun_server = gm_conf_entry_get_string (entry);
    manager.set_stun_server (stun_server ? stun_server : "stun.ekiga.net");
  }
  else if (key == NAT_KEY "disable_stun") {

    manager.set_stun_enabled (!gm_conf_entry_get_bool (entry));
  }


  // 
  // Jitter buffer configuration
  //
  else if (key == AUDIO_CODECS_KEY "maximum_jitter_buffer") {

    manager.set_maximum_jitter (gm_conf_entry_get_int (entry));
  }


  // 
  // Silence detection
  //
  else if (key == AUDIO_CODECS_KEY "enable_silence_detection") {

    manager.set_silence_detection (gm_conf_entry_get_bool (entry));
  }


  //
  // Echo cancelation
  //
  else if (key == AUDIO_CODECS_KEY "enable_echo_cancelation") {

    manager.set_echo_cancellation (gm_conf_entry_get_bool (entry));
  }
  
  
  // 
  // Audio & video codecs
  //
  else if (key == AUDIO_CODECS_KEY "media_list"
           || key == VIDEO_CODECS_KEY "media_list") {

    // This is a bit longer, we are not sure the list stored in the 
    // configuration is complete, and it could also contain unsupported codecs
    GSList *audio_codecs = NULL;
    GSList *video_codecs = NULL;

    if (key == AUDIO_CODECS_KEY "media_list") {

      audio_codecs = gm_conf_entry_get_list (entry);
      video_codecs = gm_conf_get_string_list (VIDEO_CODECS_KEY "media_list");
    }
    else {

      video_codecs = gm_conf_entry_get_list (entry);
      audio_codecs = gm_conf_get_string_list (AUDIO_CODECS_KEY "media_list");
    }

    Ekiga::CodecList codecs;
    Ekiga::CodecList a_codecs (audio_codecs);
    Ekiga::CodecList v_codecs (video_codecs);

    // Update the manager codecs
    codecs = a_codecs;
    codecs.append (v_codecs);
    manager.set_codecs (codecs);

    g_slist_foreach (audio_codecs, (GFunc) g_free, NULL);
    g_slist_free (audio_codecs);
    g_slist_foreach (video_codecs, (GFunc) g_free, NULL);
    g_slist_free (video_codecs);

    // Update the GmConf keys, in case we would have missed some codecs or
    // used codecs we do not really support
    if (a_codecs != codecs.get_audio_list ()) {

      audio_codecs = codecs.get_audio_list ().gslist ();
      gm_conf_set_string_list (AUDIO_CODECS_KEY "media_list", audio_codecs);
      g_slist_foreach (audio_codecs, (GFunc) g_free, NULL);
      g_slist_free (audio_codecs);
    }

    if (v_codecs != codecs.get_video_list ()) {

      video_codecs = codecs.get_video_list ().gslist ();
      gm_conf_set_string_list (VIDEO_CODECS_KEY "media_list", video_codecs);
      g_slist_foreach (video_codecs, (GFunc) g_free, NULL);
      g_slist_free (video_codecs);
    }
  }

  //
  // SIP related keys
  // 
  else if (key.find (SIP_KEY) != string::npos) {

    gmref_ptr<Opal::Sip::EndPoint> sip_manager = manager.get_protocol_manager ("sip");
    if (sip_manager) {

      if (key == SIP_KEY "outbound_proxy_host") {

        const gchar *str = gm_conf_entry_get_string (entry);
        if (str != NULL)
          sip_manager->set_outbound_proxy (str);
      }
      else if (key == SIP_KEY "dtmf_mode") {

        sip_manager->set_dtmf_mode (gm_conf_entry_get_int (entry));
      }
      else if (key == SIP_KEY "forward_host") {

        const gchar *str = gm_conf_entry_get_string (entry);
	if (str != NULL)
	  sip_manager->set_forward_uri (str);
      }
      else if (key == SIP_KEY "binding_timeout") {

        sip_manager->set_nat_binding_delay (gm_conf_entry_get_int (entry));
      }
    }
  }

  //
  // H.323 keys
  //
#ifdef HAVE_H323
  else if (key.find (SIP_KEY) != string::npos) {

    gmref_ptr<Opal::H323::EndPoint> h323_manager = manager.get_protocol_manager ("h323");
    if (h323_manager) {

      if (key == H323_KEY "enable_h245_tunneling") {

        h323_manager->DisableH245Tunneling (!gm_conf_entry_get_bool (entry));
      }
      else if (key == H323_KEY "enable_early_h245") {

        h323_manager->DisableH245inSetup (!gm_conf_entry_get_bool (entry));
      }
      else if (key == H323_KEY "enable_fast_start") {

        h323_manager->DisableFastStart (!gm_conf_entry_get_bool (entry));
      }
      else if (key == H323_KEY "dtmf_mode") {

        h323_manager->set_dtmf_mode (gm_conf_entry_get_int (entry));
      }
      else if (key == H323_KEY "forward_host") {

        const gchar *str = gm_conf_entry_get_string (entry);
	if (str != NULL)
	  h323_manager->set_forward_uri (str);
      }
    }
  }
#endif
  

  //
  // Personal Data Key
  //
  else if (key == PERSONAL_DATA_KEY "full_name") {

    const gchar *str = gm_conf_entry_get_string (entry);
    if (str != NULL)    
      manager.set_display_name (str);
  }


  //
  // Misc keys
  //
  else if (key == CALL_FORWARDING_KEY "forward_on_no_answer") {

    manager.set_forward_on_no_answer (gm_conf_entry_get_bool (entry));
  }
  else if (key == CALL_FORWARDING_KEY "forward_on_busy") {

    manager.set_forward_on_busy (gm_conf_entry_get_bool (entry));
  }
  else if (key == CALL_FORWARDING_KEY "always_forward") {

    manager.set_unconditional_forward (gm_conf_entry_get_bool (entry));
  }
  else if (key == CALL_OPTIONS_KEY "no_answer_timeout") {

    manager.set_reject_delay (gm_conf_entry_get_int (entry));
  }


  //
  // Ports keys
  //
  else if (key == PORTS_KEY "udp_port_range"
           || key == PORTS_KEY "tcp_port_range") {

    const gchar *ports = gm_conf_entry_get_string (entry);
    gchar **couple = NULL;
    unsigned min_port = 0;
    unsigned max_port = 0;

    if (ports)
      couple = g_strsplit (ports, ":", 2);

    if (couple && couple [0]) 
      min_port = atoi (couple [0]);
    
    if (couple && couple [1]) 
      max_port = atoi (couple [1]);
    
    if (key == PORTS_KEY "udp_port_range") 
      manager.set_udp_ports (min_port, max_port);
    else
      manager.set_tcp_ports (min_port, max_port);

    g_strfreev (couple);
  }
}

