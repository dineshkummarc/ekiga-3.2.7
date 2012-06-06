
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
 *                         opal-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Damien Sandras
 *   copyright            : (c) 2007 by Damien Sandras
 *   description          : code to hook Opal into the main program
 *
 */

#include "config.h"

#include "opal-main.h"
#include "bank.h"
#include "contact-core.h"
#include "presence-core.h"
#include "call-core.h"
#include "chat-core.h"
#include "opal-gmconf-bridge.h"
#include "opal-account.h"
#include "opal-bank.h"
#include "opal-call-manager.h"

#include "sip-endpoint.h"
#define SIP_KEY "/apps/" PACKAGE_NAME "/protocols/sip/"

#ifdef HAVE_H323
#include "h323-endpoint.h"
#define H323_KEY "/apps/" PACKAGE_NAME "/protocols/h323/"
#endif

namespace OpalLinkerHacks {
  extern int loadOpalVideoInput;
  extern int loadOpalVideoOutput;
  extern int loadOpalAudio;
}


static bool
is_supported_address (const std::string uri)
{
#ifdef HAVE_H323
  if (uri.find ("h323:") == 0)
    return true;
#endif

  if (uri.find ("sip:") == 0)
    return true;

  return false;
}

/* FIXME: add here an Ekiga::Service which will add&remove publishers,
 * decorators and fetchers
 */

using namespace Opal;

struct OPALSpark: public Ekiga::Spark
{
  OPALSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    gmref_ptr<Ekiga::ContactCore> contact_core = core.get ("contact-core");
    gmref_ptr<Ekiga::PresenceCore> presence_core = core.get ("presence-core");
    gmref_ptr<Ekiga::CallCore> call_core = core.get ("call-core");
    gmref_ptr<Ekiga::ChatCore> chat_core = core.get ("chat-core");
    gmref_ptr<Ekiga::AccountCore> account_core = core.get ("account-core");

    if (contact_core && presence_core && call_core && chat_core && account_core) {

      gmref_ptr<CallManager> call_manager (new CallManager (core));

      gmref_ptr<Opal::Bank> bank (new Bank (core));

      account_core->add_bank (bank);
      core.add (bank);
      call_manager->ready.connect (sigc::mem_fun (&*bank, &Opal::Bank::call_manager_ready));

      unsigned sip_port = gm_conf_get_int (SIP_KEY "listen_port");
      gmref_ptr<Sip::EndPoint> sip_manager (new Sip::EndPoint (*call_manager, core, sip_port));
      call_manager->add_protocol_manager (sip_manager);
      account_core->add_account_subscriber (*sip_manager);
      contact_core->add_contact_decorator (sip_manager);
      presence_core->add_presentity_decorator (sip_manager);
      presence_core->add_presence_fetcher (sip_manager);
      presence_core->add_presence_publisher (sip_manager);

#ifdef HAVE_H323
      unsigned h323_port = gm_conf_get_int (H323_KEY "listen_port");
      gmref_ptr<H323::EndPoint> h323_manager (new H323::EndPoint (*call_manager, core, h323_port));
      call_manager->add_protocol_manager (h323_manager);
      account_core->add_account_subscriber (*h323_manager);
      contact_core->add_contact_decorator (h323_manager);
      presence_core->add_presentity_decorator (h323_manager);
#endif

      call_core->add_manager (call_manager);

      core.add (call_manager);

      new ConfBridge (*call_manager); // FIXME: isn't that leaked!?

      presence_core->add_supported_uri (sigc::ptr_fun (is_supported_address)); //FIXME

      OpalLinkerHacks::loadOpalVideoInput = 1;
      OpalLinkerHacks::loadOpalVideoOutput = 1;
      OpalLinkerHacks::loadOpalAudio = 1;

      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "OPAL"; }

  bool result;
};

void
opal_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new OPALSpark);
  kickstart.add_spark (spark);
}
