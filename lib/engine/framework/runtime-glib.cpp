
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
 *                         runtime-glib.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : glib implementation of a runtime helper
 *
 */

#include "runtime.h"

#include <glib.h>

static GAsyncQueue* queue;

/* implementation of the helper functions
 *
 */

struct message
{
  message (sigc::slot0<void> _action,
	   unsigned int _seconds): action(_action),
				   seconds(_seconds)
  {}

  sigc::slot0<void> action;
  unsigned int seconds;
};

static void
free_message (struct message* msg)
{
  delete msg;
}

static gboolean
run_later_or_back_in_main_helper (gpointer data)
{
  struct message *msg = (struct message *)data;

  msg->action ();
  free_message (msg);

  return FALSE;
}

/* Implementation of the GSource
 *
 */

struct source
{
  GSource source;
  GAsyncQueue *queue;
};

static gboolean
check (GSource *source)
{
  return (g_async_queue_length (((struct source *)source)->queue) > 0);
}

static gboolean
prepare (GSource *source,
	 gint *timeout)
{
  *timeout = 100;

  return check (source);
}

static gboolean
dispatch (GSource *source,
	  GSourceFunc /*callback*/,
	  gpointer /*data*/)
{
  struct source *src = (struct source *)source;
  struct message *msg = NULL;

  msg = (struct message *)g_async_queue_pop (src->queue);

  if (msg->seconds == 0)
    (void)run_later_or_back_in_main_helper ((gpointer)msg);
  else
#if GLIB_CHECK_VERSION (2, 14, 0)
    g_timeout_add_seconds (msg->seconds,
			   run_later_or_back_in_main_helper, (gpointer)msg);
#else
    g_timeout_add (1000 * msg->seconds,
		   run_later_or_back_in_main_helper, (gpointer)msg);
#endif
  return TRUE;
}

static void
finalize (GSource *source)
{
  g_async_queue_unref (((struct source *)source)->queue);

  delete (struct source *)source;
}

static GSourceFuncs source_funcs = {
  prepare,
  check,
  dispatch,
  finalize,
  NULL,
  NULL
};

void
Ekiga::Runtime::init ()
{
  // here we get a ref to the queue, which we'll release in quit
  queue = g_async_queue_new_full ((GDestroyNotify)free_message);

  struct source* source = (struct source *)g_source_new (&source_funcs,
					  sizeof (struct source));
  source->queue = queue;
  g_async_queue_ref (queue); // give a ref to the source
  g_source_attach ((GSource *)source, g_main_context_default ());
}

void
Ekiga::Runtime::run ()
{
}

void
Ekiga::Runtime::quit ()
{
  g_async_queue_unref (queue);
  queue = NULL;
}

void
Ekiga::Runtime::run_in_main (sigc::slot0<void> action,
			     unsigned int seconds)
{
  g_async_queue_push (queue, (gpointer)(new struct message (action, seconds)));
}
