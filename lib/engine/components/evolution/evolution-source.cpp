
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
 *                         evolution-source.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution source
 *
 */

#include <iostream>

#include "evolution-source.h"

#define GCONF_PATH "/apps/evolution/addressbook/sources"

static void
on_source_list_group_added_c (ESourceList */*source_list*/,
			      ESourceGroup *group,
			      gpointer data)
{
  Evolution::Source *self = NULL;

  self = (Evolution::Source *)data;

  self->add_group (group);
}

void
Evolution::Source::add_group (ESourceGroup *group)
{
  GSList *sources = NULL;

  sources = e_source_group_peek_sources (group);

  for ( ; sources != NULL; sources = g_slist_next (sources)) {

    ESource *source = NULL;
    ESource *s = NULL;
    gchar *uri = NULL;
    EBook *ebook = NULL;

    source = E_SOURCE (sources->data);

    s = e_source_copy (source);

    uri = g_strdup_printf("%s/%s",
			  e_source_group_peek_base_uri (group),
			  e_source_peek_relative_uri (source));
    e_source_set_absolute_uri (s, uri);
    g_free (uri);

    ebook = e_book_new (s, NULL);
    g_object_unref (s);

    BookPtr book (new Evolution::Book (services, ebook));

    g_object_unref (ebook);

    add_book (book);
  }
}

static void
on_source_list_group_removed_c (ESourceList */*source_list*/,
				ESourceGroup *group,
				gpointer data)
{
  Evolution::Source *self = NULL;

  self = (Evolution::Source *)data;

  self->remove_group (group);
}

class remove_helper
{
public :

  remove_helper (ESourceGroup* group_): group(group_)
  { ready (); }

  inline void ready ()
  { found = false; }

  bool test (Evolution::BookPtr book)
  {
    EBook *book_ebook = book->get_ebook ();
    ESource *book_source = e_book_get_source (book_ebook);
    ESourceGroup *book_group = e_source_peek_group (book_source);

    if (book_group == group) {

      book->removed.emit ();
      found = true;
    }
    return !found;
  }

  bool has_found () const
  { return found; }

private:
  ESourceGroup* group;
  bool found;
};

void
Evolution::Source::remove_group (ESourceGroup *group)
{
  remove_helper helper (group);

  do {

    helper.ready ();
    visit_books (sigc::mem_fun (helper, &remove_helper::test));

  } while (helper.has_found ());
}

Evolution::Source::Source (Ekiga::ServiceCore &_services)
  : services(_services)
{
  GSList *groups = NULL;
  ESourceGroup *group = NULL;

  source_list = e_source_list_new_for_gconf_default (GCONF_PATH);

  groups = e_source_list_peek_groups (source_list);

  for ( ; groups != NULL; groups = g_slist_next (groups)) {

    group = E_SOURCE_GROUP (groups->data);
    add_group (group);
  }

  g_signal_connect (source_list, "group-added",
		    G_CALLBACK (on_source_list_group_added_c), this);
  g_signal_connect (source_list, "group-removed",
		    G_CALLBACK (on_source_list_group_removed_c), this);
}

Evolution::Source::~Source ()
{
  g_object_unref (source_list);
}

bool
Evolution::Source::populate_menu (Ekiga::MenuBuilder &/*builder*/)
{
  /* FIXME: add back creating a new addressbook later */
  return false;
}
