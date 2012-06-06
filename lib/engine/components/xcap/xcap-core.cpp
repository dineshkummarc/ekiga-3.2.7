
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
 *                         xcap.cpp  -  description
 *                         ------------------------------------
 *   begin                : Mon 29 September 2008
 *   copyright            : (C) 2008 by Julien Puydt
 *   description          : Implementation of the XCAP support code
 *
 */

#include "config.h"

#include "xcap-core.h"

#include <libsoup/soup.h>
#include <iostream>

/* declaration of XCAP::CoreImpl */

class XCAP::CoreImpl
{
public:

  CoreImpl ();

  ~CoreImpl ();

  void read (gmref_ptr<XCAP::Path> path,
	     sigc::slot2<void,bool,std::string> callback);
  void write (gmref_ptr<Path>,
	      const std::string content_type,
	      const std::string content,
	      sigc::slot1<void,std::string> callback);
  void erase (gmref_ptr<Path>,
	      sigc::slot1<void,std::string> callback);

  /* public to be used by C callbacks */

  /* The memory management is a little complex, here is how it works :
   *
   * There are two kinds of live SOUP sessions :
   * - the old sessions : we have to keep them around because we can't unref
   * a SOUP session from result_callback, so we need to keep it
   * around for later ;
   * - the pending ones : we keep them around to be able to cancel them if
   * we destroy the core (and cancelling calls result_callback with an
   * error -- the sessions get pushed from pending to old) ;
   *
   * So the problem of freeing the SOUP sessions becomes one is reduced to
   * that of freeing the old sessions :
   * - in the read method, we check this old_sessions list and unref any old
   * session we find : ok ;
   * - in the destructor, we check the old_sesssions list and unref any old
   * session we find.
   *
   * That means if you launch a thousand reads fast then nothing,
   * we will end up with a thousand SOUP sessions in old_sessions. But any
   * subsequent read will clear them all.
   *
   * If you launch a thousand reads not fast enough, the last reads will
   * already find the first sessions and get rid of them.
   *
   * In short this code guarantees :
   * - the number of old sessions can't really grow if we have reads from time
   * to time  ;
   * - all the sessions will get freed eventually -- no leak!
   */
  std::list<SoupSession*> pending_sessions;
  std::list<SoupSession*> old_sessions;
  void clear_old_sessions ();
};

/* soup callbacks */

struct cb_read_data
{
  XCAP::CoreImpl* core;
  gmref_ptr<XCAP::Path> path;
  sigc::slot2<void, bool, std::string> callback;
};

struct cb_other_data
{
  XCAP::CoreImpl* core;
  gmref_ptr<XCAP::Path> path;
  sigc::slot1<void, std::string> callback;
};

static void
authenticate_read_callback (G_GNUC_UNUSED SoupSession* session,
			    G_GNUC_UNUSED SoupMessage* message,
			    SoupAuth* auth,
			    gboolean retrying,
			    gpointer data)
{
  cb_read_data* cb = (cb_read_data*)data;

  if ( !retrying) {

    soup_auth_authenticate (auth,
			    cb->path->get_username ().c_str (),
			    cb->path->get_password ().c_str ());
  }
}

static void
authenticate_other_callback (G_GNUC_UNUSED SoupSession* session,
			     G_GNUC_UNUSED SoupMessage* message,
			     SoupAuth* auth,
			     gboolean retrying,
			     gpointer data)
{
  cb_other_data* cb = (cb_other_data*)data;

  if ( !retrying) {

    soup_auth_authenticate (auth,
			    cb->path->get_username ().c_str (),
			    cb->path->get_password ().c_str ());
  }
}

static void
result_read_callback (SoupSession* session,
		      SoupMessage* message,
		      gpointer data)
{
  cb_read_data* cb = (cb_read_data*)data;

  if (message->status_code == SOUP_STATUS_OK) {

    cb->callback (false, message->response_body->data);
  } else {

    cb->callback (true, message->reason_phrase);
  }

  cb->core->pending_sessions.remove (session);
  cb->core->old_sessions.push_back (session);

  delete cb;
}

static void
result_other_callback (SoupSession* session,
		       SoupMessage* message,
		       gpointer data)
{
  cb_other_data* cb = (cb_other_data*)data;

  if (message->status_code == SOUP_STATUS_OK) {

    cb->callback ("");
  } else {

    cb->callback (message->reason_phrase);
  }

  cb->core->pending_sessions.remove (session);
  cb->core->old_sessions.push_back (session);

  delete cb;
}

/* implementation of XCAP::CoreImpl */

XCAP::CoreImpl::CoreImpl ()
{
}

XCAP::CoreImpl::~CoreImpl ()
{
  /* we loop like this because aborting calls result_callback, and hence
   * makes the current iterator invalid : it gets pushed to the old sessions
   * list!
   */
  while (pending_sessions.begin () != pending_sessions.end ())
    soup_session_abort (*pending_sessions.begin ());

  /* now all pending sessions have been made old, so we can do that: */
  clear_old_sessions ();
}

void
XCAP::CoreImpl::clear_old_sessions ()
{
  for (std::list<SoupSession*>::iterator iter = old_sessions.begin ();
       iter != old_sessions.end ();
       ++iter) {

    soup_session_abort (*iter); // needed ?
    g_object_unref (*iter);
  }
  old_sessions.clear ();
}

void
XCAP::CoreImpl::read (gmref_ptr<Path> path,
		      sigc::slot2<void, bool, std::string> callback)
{
  SoupSession* session = NULL;
  SoupMessage* message = NULL;
  cb_read_data* data = NULL;

  clear_old_sessions ();

  /* all of this is freed in the result callback */
  session = soup_session_async_new_with_options ("user-agent", "ekiga", NULL);
  message = soup_message_new ("GET", path->to_uri ().c_str ());
  data = new cb_read_data;
  data->core = this;
  data->path = path;
  data->callback = callback;

  g_signal_connect (session, "authenticate",
		    G_CALLBACK (authenticate_read_callback), data);

  soup_session_queue_message (session, message,
			      result_read_callback, data);

  pending_sessions.push_back (session);
}

void
XCAP::CoreImpl::write (gmref_ptr<Path> path,
		       const std::string content_type,
		       const std::string content,
		       sigc::slot1<void,std::string> callback)
{
  SoupSession* session = NULL;
  SoupMessage* message = NULL;
  cb_other_data* data = NULL;

  clear_old_sessions ();

  /* all of this is freed in the result callback */
  session = soup_session_async_new_with_options ("user-agent", "ekiga", NULL);
  message = soup_message_new ("PUT", path->to_uri ().c_str ());
  soup_message_set_request (message, content_type.c_str (),
			    SOUP_MEMORY_COPY,
			    content.c_str (), content.length ());

  data = new cb_other_data;
  data->core = this;
  data->path = path;
  data->callback = callback;

  g_signal_connect (session, "authenticate",
		    G_CALLBACK (authenticate_other_callback), data);

  soup_session_queue_message (session, message,
			      result_other_callback, data);

  pending_sessions.push_back (session);
}

void
XCAP::CoreImpl::erase (gmref_ptr<Path> path,
		       sigc::slot1<void,std::string> callback)
{
  SoupSession* session = NULL;
  SoupMessage* message = NULL;
  cb_other_data* data = NULL;

  clear_old_sessions ();

  /* all of this is freed in the result callback */
  session = soup_session_async_new_with_options ("user-agent", "ekiga", NULL);
  message = soup_message_new ("DELETE", path->to_uri ().c_str ());
  data = new cb_other_data;
  data->core = this;
  data->path = path;
  data->callback = callback;

  g_signal_connect (session, "authenticate",
		    G_CALLBACK (authenticate_other_callback), data);

  soup_session_queue_message (session, message,
			      result_other_callback, data);

  pending_sessions.push_back (session);
}


/* XCAP::Core just pimples : */
XCAP::Core::Core ()
{
  impl = new CoreImpl ();
}

XCAP::Core::~Core ()
{
  delete impl;
}

void
XCAP::Core::read (gmref_ptr<XCAP::Path> path,
		  sigc::slot2<void, bool,std::string> callback)
{
  impl->read (path, callback);
}

void
XCAP::Core::write (gmref_ptr<Path> path,
		   const std::string content_type,
		   const std::string content,
		   sigc::slot1<void,std::string> callback)
{
  impl->write (path, content_type, content, callback);
}

void
XCAP::Core::erase (gmref_ptr<Path> path,
		   sigc::slot1<void,std::string> callback)
{
  impl->erase (path, callback);
}
