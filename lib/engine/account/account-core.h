
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
 *                         account-core.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : interface of the main account managing object
 *
 */

#ifndef __ACCOUNT_CORE_H__
#define __ACCOUNT_CORE_H__

#include <list>
#include <iostream>

#include "menu-builder.h"
#include "form-request.h"
#include "chain-of-responsibility.h"
#include "services.h"

#include "bank.h"

/* declaration of a few helper classes */
namespace Ekiga
{
  class AccountSubscriber
  {
  public:
    virtual ~AccountSubscriber () {}
  };

  /**
   * @defgroup accounts
   * @{
   */

  /** Core object for address account support.
   *
   * Notice that you give banks to this object as references, so they won't
   * be freed here : it's up to you to free them somehow.
   */
  class AccountCore: public Service
  {
  public:

    /** The constructor.
     */
    AccountCore ();

    /** The destructor.
     */
    ~AccountCore ();


    /*** Service Implementation ***/

    /** Returns the name of the service.
     * @return The service name.
     */
    const std::string get_name () const
    { return "account-core"; }


    /** Returns the description of the service.
     * @return The service description.
     */
    const std::string get_description () const
    { return "\tAccount managing object"; }


    /*** Public API ***/

    /** Adds a bank to the AccountCore service.
     * @param The bank to be added.
     */
    void add_bank (BankPtr bank);


    /** Triggers a callback for all Ekiga::Bank banks of the
     * AccountCore service.
     * @param The callback (the return value means "go on" and allows
     *  stopping the visit)
     */
    void visit_banks (sigc::slot1<bool, BankPtr> visitor);


    /** This signal is emitted when a bank has been added to the core
     */
    sigc::signal1<void, BankPtr> bank_added;

    /** This signal is emitted when a bank has been removed from the core
     */
    sigc::signal1<void, BankPtr> bank_removed;

    /** This signal is emitted when a account has been added to one of
     * the banks
     */
    sigc::signal2<void, BankPtr, AccountPtr> account_added;

    /** This signal is emitted when a account has been removed from one of
     * the banks
     */
    sigc::signal2<void, BankPtr, AccountPtr> account_removed;

    /** This signal is emitted when a account has been updated in one of
     * the banks
     */
    sigc::signal2<void, BankPtr, AccountPtr> account_updated;

  private:

    std::list<BankPtr> banks;
    typedef std::list<BankPtr>::iterator bank_iterator;
    typedef std::list<BankPtr>::const_iterator bank_const_iterator;


    /*** Misc ***/

  public:

    /** Create the menu for the AccountCore and its actions.
     * @param A MenuBuilder object to populate.
     */
    bool populate_menu (MenuBuilder &builder);


    /** This signal is emitted when the AccountCore Service has been
     * updated.
     */
    sigc::signal0<void> updated;


    /** This chain allows the AccountCore to present forms to the user
     */
    ChainOfResponsibility<FormRequest*> questions;


    /** This signal is emitted when there is a new registration event
     * @param: account is the account
     *         state is the state
     *         info contains information about the registration status
     */
    sigc::signal4<void, BankPtr, AccountPtr, Account::RegistrationState, std::string> registration_event;


    /** This signal is emitted when there is a new message waiting event
     * @param: account is the account
     *         info contains information about the indication
     */
    sigc::signal3<void, BankPtr, AccountPtr, std::string> mwi_event;


    /*** Account Subscriber API ***/
  public:
    void add_account_subscriber (AccountSubscriber &subscriber);

    template<class T>
    bool subscribe_account (const T &account);

    template<class T>
    bool unsubscribe_account (const T &account);

    /*** Misc ***/
  private:
    std::set<AccountSubscriber *> account_subscribers;
    typedef std::set<AccountSubscriber *>::iterator subscriber_iterator;
    typedef std::set<AccountSubscriber *>::const_iterator subscriber_const_iterator;
    void on_registration_event (BankPtr bank,
				AccountPtr account,
                                Account::RegistrationState state,
                                const std::string info);
    void on_mwi_event (BankPtr bank,
		       AccountPtr account,
                       const std::string & info);
  };


  template<class T = Account>
  class AccountSubscriberImpl : public AccountSubscriber
  {
  public:
    virtual ~AccountSubscriberImpl () {}

    virtual bool subscribe (const T & /*account*/);
    virtual bool unsubscribe (const T & /*account*/);
  };

  /**
   * @}
   */
};



template<class T>
bool
Ekiga::AccountCore::subscribe_account (const T &account)
{
  for (subscriber_iterator iter = account_subscribers.begin ();
       iter != account_subscribers.end ();
       iter++) {

    Ekiga::AccountSubscriberImpl<T> *subscriber = dynamic_cast<Ekiga::AccountSubscriberImpl<T> *> (*iter);
    if (subscriber)
      if (subscriber->subscribe (account))
        return true;
  }

  return false;
}


template<class T>
bool
Ekiga::AccountCore::unsubscribe_account (const T &account)
{
  for (subscriber_iterator iter = account_subscribers.begin ();
       iter != account_subscribers.end ();
       iter++) {

    Ekiga::AccountSubscriberImpl<T> *subscriber = dynamic_cast<Ekiga::AccountSubscriberImpl<T> *> (*iter);
    if (subscriber)
      if (subscriber->unsubscribe (account))
        return true;
  }

  return false;
}


template<class T>
bool
Ekiga::AccountSubscriberImpl<T>::subscribe (const T & /*account*/)
{
  return false;
}


template<class T>
bool
Ekiga::AccountSubscriberImpl<T>::unsubscribe (const T & /*account*/)
{
  return false;
}
#endif
