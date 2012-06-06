
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
 *                         sip-dialect.h  -  description
 *                         --------------------------------
 *   begin                : written in july 2008 by Julien Puydt
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Interface of the SIP dialect
 *
 */

#ifndef __SIP_DIALECT_H__
#define __SIP_DIALECT_H__

#include "dialect-impl.h"
#include "sip-chat-simple.h"

namespace SIP
{
  class Dialect: public Ekiga::DialectImpl<SimpleChat>
  {
  public:
    Dialect (Ekiga::ServiceCore& core_,
	     /* the strings are : uri then msg */
	     sigc::slot2<bool, std::string, std::string> sender_);

    ~Dialect ();

    void push_message (const std::string uri,
		       const std::string name,
		       const std::string msg);

    void push_notice (const std::string uri,
		      const std::string name,
		      const std::string msg);

    bool populate_menu (Ekiga::MenuBuilder& builder);

    void start_chat_with (std::string uri,
			  std::string name);

  private:
    Ekiga::ServiceCore& core;
    /* the strings are : uri then msg */
    sigc::slot2<bool, std::string, std::string> sender;

    SimpleChatPtr open_chat_with (std::string uri,
					  std::string name,
					  bool user_request);
  };

  typedef gmref_ptr<Dialect> DialectPtr;

};

#endif
