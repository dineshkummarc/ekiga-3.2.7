
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
 *                         ldap-main.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : code to hook LDAP into the main program
 *
 */

#include "services.h"
#include "contact-core.h"

#include "ldap-main.h"
#include "ldap-source.h"

#include <sasl/sasl.h>

struct LDAPSpark: public Ekiga::Spark
{
  LDAPSpark (): result(false)
  {}

  bool try_initialize_more (Ekiga::ServiceCore& core,
			    int* /*argc*/,
			    char** /*argv*/[])
  {
    gmref_ptr<Ekiga::ContactCore> contact_core = core.get ("contact-core");

    if (contact_core) {

      gmref_ptr<OPENLDAP::Source> service (new OPENLDAP::Source (core));
      core.add (service);
      contact_core->add_source (service);
      sasl_client_init (NULL); // FIXME: shouldn't it be done by the source!?
      result = true;
    }

    return result;
  }

  Ekiga::Spark::state get_state () const
  { return result?FULL:BLANK; }

  const std::string get_name () const
  { return "LDAP"; }

  bool result;
};

void
ldap_init (Ekiga::KickStart& kickstart)
{
  gmref_ptr<Ekiga::Spark> spark(new LDAPSpark);
  kickstart.add_spark (spark);
}
