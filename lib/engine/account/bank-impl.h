
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
 *                         bank.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of a partial implementation
 *                          of a Bank
 *
 */

#ifndef __BANK_IMPL_H__
#define __BANK_IMPL_H__

#include "reflister.h"
#include "bank.h"


namespace Ekiga
{
/**
 * @addtogroup accounts
 * @{
 */

  /** Generic implementation for the Ekiga::BankImpl abstract class.
   *
   * This class is there to make it easy to implement a new type of account
   * bank: it will take care of implementing the external api, you
   * just have to decide when to add and remove accounts.
   *
   * Any deleted BankImpl is automatically removed from the AccountCore.
   * The implementor should not have to take care about that.
   *
   * You can remove a Account from an Ekiga::BankImpl in two ways:
   *  - either by calling the remove_account method,
   *  - or by emission of the account's removed signal.
   *
   * Notice that this class won't take care of removing the Account from a
   * backend -- only from the Ekiga::BankImpl.
   * If you want the Account <b>deleted</b> from the backend, then you
   * probably should have an organization like:
   *  - the account has a 'deleted' signal ;
   *  - the bank listens to this signal ;
   *  - when the signal is received, then do a remove_account followed by
   *    calling the appropriate api function to delete the account in your
   *    backend.
   */
  template<class AccountType = Account>
  class BankImpl:
    public Bank,
    public sigc::trackable,
    protected RefLister<AccountType>
  {

  public:

    typedef typename RefLister<AccountType>::iterator iterator;
    typedef typename RefLister<AccountType>::const_iterator const_iterator;

    /** The constructor
     */
    BankImpl ();

    /** The destructor.
     */
    ~BankImpl ();

    /** Visit all accounts of the bank and trigger the given callback.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_accounts (sigc::slot1<bool, AccountPtr> visitor);

    /** This function is called when a new account has to be added to the Bank.
     */
    void new_account ();

    /** Returns an iterator to the first Account of the collection
     */
    iterator begin ();

    /** Returns an iterator to the last Account of the collection
     */
    iterator end ();

    /** Returns a const iterator to the first Account of the collection
     */
    const_iterator begin () const;

    /** Returns a const iterator to the last Account of the collection
     */
    const_iterator end () const;

  protected:

    /** Adds a account to the Ekiga::BankImpl.
     * @param: The account to be added.
     * @return: The Ekiga::BankImpl 'account_added' signal is emitted when the account
     * has been added. The Ekiga::BankImpl 'account_updated' signal will be emitted
     * when the account has been updated and the Ekiga::BankImpl 'account_removed' signal
     * will be emitted when the account has been removed from the Ekiga::BankImpl.
     */
    void add_account (gmref_ptr<AccountType> account);

    /** Removes a account from the Ekiga::BankImpl.
     * @param: The account to be removed.
     * @return: The Ekiga::BankImpl 'account_removed' signal is emitted when the account
     * has been removed.
     */
    void remove_account (gmref_ptr<AccountType> account);

    using RefLister<AccountType>::add_connection;

  private:
    void on_registration_event (Ekiga::Account::RegistrationState,
				std::string info,
				gmref_ptr<AccountType> account);
  };

/**
 * @}
 */

};


/* here begins the code from the template functions */

template<typename AccountType>
Ekiga::BankImpl<AccountType>::BankImpl ()
{
  /* this is signal forwarding */
  RefLister<AccountType>::object_added.connect (account_added.make_slot ());
  RefLister<AccountType>::object_removed.connect (account_removed.make_slot ());
  RefLister<AccountType>::object_updated.connect (account_updated.make_slot ());
}


template<typename AccountType>
Ekiga::BankImpl<AccountType>::~BankImpl ()
{
}


template<typename AccountType>
void
Ekiga::BankImpl<AccountType>::visit_accounts (sigc::slot1<bool, AccountPtr> visitor)
{
  RefLister<AccountType>::visit_objects (visitor);
}


template<typename AccountType>
typename Ekiga::BankImpl<AccountType>::iterator
Ekiga::BankImpl<AccountType>::begin ()
{
  return RefLister<AccountType>::begin ();
}


template<typename AccountType>
typename Ekiga::BankImpl<AccountType>::iterator
Ekiga::BankImpl<AccountType>::end ()
{
  return RefLister<AccountType>::end ();
}


template<typename AccountType>
typename Ekiga::BankImpl<AccountType>::const_iterator
Ekiga::BankImpl<AccountType>::begin () const
{
  return RefLister<AccountType>::begin ();
}


template<typename AccountType>
typename Ekiga::BankImpl<AccountType>::const_iterator
Ekiga::BankImpl<AccountType>::end () const
{
  return RefLister<AccountType>::end ();
}


template<typename AccountType>
void
Ekiga::BankImpl<AccountType>::add_account (gmref_ptr<AccountType> account)
{
  add_object (account);

  account->questions.add_handler (questions.make_slot ());
  account->registration_event.connect (sigc::bind (sigc::mem_fun (this, &Ekiga::BankImpl<AccountType>::on_registration_event), account));
}


template<typename AccountType>
void
Ekiga::BankImpl<AccountType>::remove_account (gmref_ptr<AccountType> account)
{
  remove_object (account);
}


template<typename AccountType>
void
Ekiga::BankImpl<AccountType>::on_registration_event (Ekiga::Account::RegistrationState state,
						     std::string info,
						     gmref_ptr<AccountType> account)
{
  registration_event.emit (account, state, info);
}

#endif
