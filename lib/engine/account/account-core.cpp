
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
 *                         account-core.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : implementation of the main account managing object
 *
 */

#include <iostream>

#include "config.h"

#include "account-core.h"
#include "bank.h"

Ekiga::AccountCore::AccountCore ()
{
}


Ekiga::AccountCore::~AccountCore ()
{
}


bool
Ekiga::AccountCore::populate_menu (MenuBuilder & builder)
{
  bool populated = false;

  for (bank_const_iterator iter = banks.begin ();
       iter != banks.end ();
       iter++) {

    populated = (*iter)->populate_menu (builder);
  }

  return populated;
}


void
Ekiga::AccountCore::add_bank (BankPtr bank)
{
  banks.push_back (bank);

  bank->account_added.connect (sigc::bind<0>(account_added.make_slot (), bank));
  bank->account_removed.connect (sigc::bind<0>(account_removed.make_slot (), bank));
  bank->account_updated.connect (sigc::bind<0>(account_updated.make_slot (), bank));

  bank_added.emit (bank);

  bank->questions.add_handler (questions.make_slot ());
  bank->registration_event.connect (sigc::bind<0> (sigc::mem_fun (this, &Ekiga::AccountCore::on_registration_event), bank));
}


void
Ekiga::AccountCore::visit_banks (sigc::slot1<bool, BankPtr> visitor)
{
  bool go_on = true;

  for (bank_iterator iter = banks.begin ();
       iter != banks.end () && go_on;
       iter++)
    go_on = visitor (*iter);
}


void
Ekiga::AccountCore::add_account_subscriber (AccountSubscriber &subscriber)
{
  account_subscribers.insert (&subscriber);
}


void
Ekiga::AccountCore::on_registration_event (BankPtr bank,
					   AccountPtr account,
					   Ekiga::Account::RegistrationState state,
					   const std::string info)
{
  registration_event.emit (bank, account, state, info);
}


void Ekiga::AccountCore::on_mwi_event (BankPtr bank,
				       AccountPtr account,
                                       const std::string & info)
{
  mwi_event.emit (bank, account, info);
}
