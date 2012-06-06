
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
 *                         chat.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of the interface of a chat
 *
 */

#ifndef __CHAT_H__
#define __CHAT_H__

#include <string>
#include <sigc++/sigc++.h>

#include "gmref.h"

#include "chain-of-responsibility.h"
#include "form-request.h"
#include "menu-builder.h"

namespace Ekiga
{
  class ChatObserver: public virtual GmRefCounted
  {
  public:

    virtual ~ChatObserver () {}

    /** Tell the observer about a new user message, like :
     * observer.message ("Damien", "Hi Snark, did you fix bug #314159 ?");
     *
     */
    virtual void message (const std::string to,
			  const std::string msg) = 0;

    /** Tell the observer about a new service message, like :
     * observer.notice ("Snark just disconnected");
     *
     */
    virtual void notice (const std::string msg) = 0;
  };


  class Chat: public virtual GmRefCounted
  {
  public:

    /** The destructor.
     */
    virtual ~Chat ()
    {}

    /** Returns the title of the Chat.
     * @return The chat title.
     */
    virtual const std::string get_title () const = 0;

    /** Connects a new observer to the Chat ; notice that using an
     * observer-as-an-object approch vs a signal based approch makes
     * it possible to detect when the last observer quits, and hence
     * quit the Chat (emit "removed"). The Chat can use that call to
     * send a few previous messages to the new observer.
     * @param The new observer
     */
    virtual void connect (gmref_ptr<ChatObserver> observer) = 0;

    /** Disconnects the new observer from the Chat
     * @param The observer to disconnect
     */
    virtual void disconnect (gmref_ptr<ChatObserver> observer) = 0;

    /** Sends a message through the Chat, or at least attempts to :
     * the two ideas are first that the text entry will get blanked
     * only if true is returned, and second that true doesn't mean the
     * message is really sent, but only that it was valid to try (for
     * example, the Chat will return false if you try to send when not
     * having "voice" on an irc channel, or if you try to send to a
     * disconnected contact with a protocol which doesn't support
     * disconnected operation). The user may still be warned that the
     * actual sending was impossible later through a notice message.
     * @param The message to send
     * @return True if it was valid to send a message
     */
    virtual bool send_message (const std::string msg) = 0;

    /** This signal is emitted when the Chat has been updated.
     */
    sigc::signal0<void> updated;

    /** This signal is emitted when the user requested to see this Chat
     */
    sigc::signal0<void> user_requested;

    /** This signal is emitted when the Chat has been removed.
     */
    sigc::signal0<void> removed;

    /** Feed possible actions on this Chat to the given MenuBuilder
     * @param A MenuBuilder object to populate.
     */
    virtual bool populate_menu (MenuBuilder &) = 0;

    /** This chain allows the Chat to present forms to the user.
     */
    ChainOfResponsibility<FormRequest*> questions;
  };

};

#endif
