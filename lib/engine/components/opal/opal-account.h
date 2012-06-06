
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
 *                         opal-account.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Damien Sandras
 *   copyright            : (c) 2008 by Damien Sandras
 *   description          : declaration of an OPAL account
 *
 */

#ifndef __OPAL_ACCOUNT_H__
#define __OPAL_ACCOUNT_H__

#include "services.h"
#include "account-core.h"
#include "account.h"
#include "form.h"
#include "bank-impl.h"

namespace Opal
{
  /**
   * @addtogroup accounts
   * @internal
   * @{
   */
  class Account: public Ekiga::Account
  {
public:

    typedef enum { SIP, Ekiga, DiamondCard, H323 } Type;

    Account (Ekiga::ServiceCore & core, 
             const std::string & account);

    Account (Ekiga::ServiceCore & core,
             Type t,
             std::string name, 
             std::string host,
             std::string user,
             std::string auth_user,
             std::string password,
             bool enabled,
             unsigned timeout);

    virtual ~Account ();

    const std::string get_name () const;

    const std::string get_aor () const;

    /** Returns the protocol name of the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The protocol name of the Ekiga::Contact.
     */
    const std::string get_protocol_name () const;

    const std::string get_host () const;

    /** Returns the user name for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The user name of the Ekiga::Account.
     */
    const std::string get_username () const;

    /** Returns the authentication user name for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The authentication user name of the Ekiga::Account.
     */
    const std::string get_authentication_username () const;

    /** Returns the password for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The password of the Ekiga::Account.
     */
    const std::string get_password () const;

    void set_authentication_settings (const std::string & username,
                                      const std::string & password);

    /** Returns the registration timeout for the Opal::Account.
     * This function is purely virtual and should be implemented by the
     * Ekiga::Account descendant.
     * @return The timeout of the Ekiga::Account.
     */
    unsigned get_timeout () const;

    void enable ();

    void disable ();

    bool is_enabled () const;

    bool is_limited () const;

    bool is_active () const;

    void remove ();

    void edit ();

    bool populate_menu (Ekiga::MenuBuilder &builder);

    const std::string as_string () const;

    sigc::signal0<void> trigger_saving;
 
private:
    void on_edit_form_submitted (bool submitted,
				 Ekiga::Form &result);
    void on_consult (const std::string url);
    
    // Triggered for our own event
    void on_registration_event (Ekiga::Account::RegistrationState state, std::string info);

    bool dead;
    bool active;
    bool enabled;
    bool limited;
    unsigned timeout;
    std::string aid;
    std::string name;
    std::string protocol_name;
    std::string host;
    std::string username;
    std::string auth_username;
    std::string password;
    Type type;

    Ekiga::ServiceCore & core;
  };

  typedef gmref_ptr<Account> AccountPtr;

  /**
   * @}
   */
};

#endif
