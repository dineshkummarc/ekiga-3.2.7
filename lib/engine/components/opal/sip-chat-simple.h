
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
 *                         sip-chat-simple.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Interface of a simple chat in SIP
 *
 */

#ifndef __SIP_CHAT_SIMPLE_H__
#define __SIP_CHAT_SIMPLE_H__

#include "chat-simple.h"
#include "services.h"

namespace SIP
{
  class SimpleChat: public Ekiga::SimpleChat
  {
  public:
    SimpleChat (Ekiga::ServiceCore& core,
		std::string name,
		std::string uri,
		sigc::slot1<bool, std::string> sender_);

    ~SimpleChat ();

    const std::string get_uri () const;

    const std::string get_title () const;

    void connect (gmref_ptr<Ekiga::ChatObserver> observer);

    void disconnect (gmref_ptr<Ekiga::ChatObserver> observer);

    bool send_message (const std::string msg);

    void receive_message (const std::string msg);

    void receive_notice (const std::string msg);

    Ekiga::PresentityPtr get_presentity () const;

    bool populate_menu (Ekiga::MenuBuilder& builder);

  private:

    Ekiga::ServiceCore& core;
    sigc::slot1<bool, std::string> sender;
    std::list<gmref_ptr<Ekiga::ChatObserver> > observers;
    Ekiga::PresentityPtr presentity;
    std::string uri;
  };

  typedef gmref_ptr<SimpleChat> SimpleChatPtr;

};

#endif
