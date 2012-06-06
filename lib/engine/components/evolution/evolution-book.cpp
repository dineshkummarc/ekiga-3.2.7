
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
 *                         evolution-book.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution addressbook
 *
 */

#include <iostream>
#include <string>
#include <glib/gi18n.h>

#include "form-request-simple.h"

#include "evolution-book.h"
#include "evolution-contact.h"

static void
on_view_contacts_added_c (EBook * /*ebook*/,
			  GList *contacts,
			  gpointer data)
{
  ((Evolution::Book *)data)->on_view_contacts_added (contacts);
}

void
Evolution::Book::on_view_contacts_added (GList *econtacts)
{
  EContact *econtact = NULL;
  int nbr = 0;
  gchar* c_status = NULL;

  for (; econtacts != NULL; econtacts = g_list_next (econtacts)) {

    econtact = E_CONTACT (econtacts->data);

    if (e_contact_get_const (econtact, E_CONTACT_FULL_NAME) != NULL) {

      ContactPtr contact(new Evolution::Contact (services, book,
						 econtact));

      add_contact (contact);
      nbr++;
    }
  }

  c_status = g_strdup_printf (ngettext ("%d user found", "%d users found", nbr),
			      nbr);
  status = c_status;
  g_free (c_status);

  updated.emit ();
}

static void
on_view_contacts_removed_c (EBook */*ebook*/,
			    GList *ids,
			    gpointer data)
{
  ((Evolution::Book *)data)->on_view_contacts_removed (ids);
}

class contacts_removed_helper
{
public:

  contacts_removed_helper (const std::string id_): id(id_)
  {}

  bool test (Evolution::ContactPtr contact)
  {
    bool result;

    if (contact->get_id () == id) {

      contact->removed.emit ();
      result = false;
    }

    return result;
  }

private:
  const std::string id;
};

void
Evolution::Book::on_view_contacts_removed (GList *ids)
{
  for (; ids != NULL; ids = g_list_next (ids)) {

    contacts_removed_helper helper((gchar*)ids->data);
    visit_contacts (sigc::mem_fun (helper, &contacts_removed_helper::test));
  }
}

static void
on_view_contacts_changed_c (EBook */*ebook*/,
			    GList *econtacts,
			    gpointer data)
{
  ((Evolution::Book*)data)->on_view_contacts_changed (econtacts);
}

class contact_updated_helper
{
public:

  contact_updated_helper (EContact* econtact_): econtact(econtact_)
  {
    id = (const gchar*)e_contact_get_const (econtact, E_CONTACT_UID);
  }

  bool test (Evolution::ContactPtr contact)
  {
    bool result = true;

    if (contact->get_id () == id) {

      contact->update_econtact (econtact);
      result = false;
    }

    return result;
  }

private:
  EContact* econtact;
  std::string id;
};

void
Evolution::Book::on_view_contacts_changed (GList *econtacts)
{
  for (; econtacts != NULL; econtacts = g_list_next (econtacts)) {

    contact_updated_helper helper (E_CONTACT (econtacts->data));

    visit_contacts (sigc::mem_fun (helper, &contact_updated_helper::test));
  }
}

static void
on_book_view_obtained_c (EBook */*book*/,
			 EBookStatus status,
			 EBookView *view,
			 gpointer data)
{
  ((Evolution::Book*)data)->on_book_view_obtained (status, view);
}

void
Evolution::Book::on_book_view_obtained (EBookStatus _status,
					EBookView *_view)
{
  if (_status == E_BOOK_ERROR_OK) {

    if (view != NULL)
      g_object_unref (view);

    view = _view;

    /* no need for wrappers here : the view will only die with us */
    g_object_ref (view);

    g_signal_connect (view, "contacts-added",
		      G_CALLBACK (on_view_contacts_added_c), this);

    g_signal_connect (view, "contacts-removed",
		      G_CALLBACK (on_view_contacts_removed_c), this);

    g_signal_connect (view, "contacts-changed",
		      G_CALLBACK (on_view_contacts_changed_c), this);

    e_book_view_start (view);
  } else
    removed.emit ();
}

static void
on_book_opened_c (EBook */*book*/,
		  EBookStatus _status,
		  gpointer data)
{
  ((Evolution::Book*)data)->on_book_opened (_status);
}

void
Evolution::Book::on_book_opened (EBookStatus _status)
{
  EBookQuery *query = NULL;

  if (_status == E_BOOK_ERROR_OK) {

    if (search_filter.empty ())
      query = e_book_query_field_exists (E_CONTACT_FULL_NAME);
    else
      query = e_book_query_field_test (E_CONTACT_FULL_NAME,
                                       E_BOOK_QUERY_CONTAINS,
                                       search_filter.c_str ());

    (void)e_book_async_get_book_view (book, query, NULL, 100,
				      on_book_view_obtained_c, this);

    e_book_query_unref (query);

  }
  else {

    book = NULL;
    removed.emit ();
  }
}

Evolution::Book::Book (Ekiga::ServiceCore &_services,
		       EBook *_book)
  : services(_services), book(_book), view(NULL)
{
  g_object_ref (book);

  refresh ();
}

Evolution::Book::~Book ()
{
  if (book != NULL)
    g_object_unref (book);
}

const std::string
Evolution::Book::get_name () const
{
  ESource *source = NULL;
  std::string result;

  source = e_book_get_source (book);
  if (source && E_IS_SOURCE (source))
    result = e_source_peek_name (source);

  return result;
}

EBook *
Evolution::Book::get_ebook () const
{
  return book;
}

bool
Evolution::Book::populate_menu (Ekiga::MenuBuilder &builder)
{
  builder.add_action ("new", _("New _Contact"),
		      sigc::mem_fun (this, &Evolution::Book::new_contact_action));
  return true;
}

void
Evolution::Book::set_search_filter (std::string _search_filter)
{
  search_filter = _search_filter;
  refresh ();
}

const std::string
Evolution::Book::get_status () const
{
  return status;
}

void
Evolution::Book::refresh ()
{
  remove_all_objects ();

  /* we go */
  if (e_book_is_opened (book))
    on_book_opened_c (book, E_BOOK_ERROR_OK, this);
  else
    e_book_async_open (book, TRUE,
                       on_book_opened_c, this);
}

void
Evolution::Book::new_contact_action ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &Evolution::Book::on_new_contact_form_submitted));

  request.title (_("New contact"));

  request.instructions (_("Please update the following fields:"));

  request.text ("name", _("_Name:"), "");
  request.text ("video", _("VoIP _URI:"), "");
  request.text ("home", _("_Home phone:"), "");
  request.text ("work", _("_Office phone:"), "");
  request.text ("cell", _("_Cell phone:"), "");
  request.text ("pager", _("_Pager:"), "");

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
Evolution::Book::set_econtact_attribute_value (EContact *econtact,
					       const std::string subtype,
					       const std::string value) const
{
  EVCardAttribute *attribute = NULL;
  EVCardAttributeParam *param = NULL;

  attribute = e_vcard_attribute_new ("", EVC_TEL);
  param = e_vcard_attribute_param_new (EVC_TYPE);
  e_vcard_attribute_param_add_value (param, subtype.c_str ());
  e_vcard_attribute_add_param (attribute, param);
  e_vcard_attribute_add_value (attribute, value.c_str ());
  e_vcard_add_attribute (E_VCARD (econtact), attribute);
}

void
Evolution::Book::on_new_contact_form_submitted (bool submitted,
						Ekiga::Form &result)
{
  if ( !submitted)
    return;

  EContact *econtact = NULL;


  /* first check we have everything before using */
  std::string name = result.text ("name");
  std::string home = result.text ("home");
  std::string cell = result.text ("cell");
  std::string work = result.text ("work");
  std::string pager = result.text ("pager");
  std::string video = result.text ("video");

  econtact = e_contact_new ();
  e_contact_set (econtact, E_CONTACT_FULL_NAME, (gpointer)name.c_str ());
  if ( !home.empty ())
    set_econtact_attribute_value (econtact, "HOME", home);
  if ( !cell.empty ())
    set_econtact_attribute_value (econtact, "CELL", cell);
  if ( !work.empty ())
    set_econtact_attribute_value (econtact, "WORK", work);
  if ( !pager.empty ())
    set_econtact_attribute_value (econtact, "PAGER", pager);
  if ( !video.empty ())
    set_econtact_attribute_value (econtact, "VIDEO", video);

  e_book_add_contact (book, econtact, NULL);
  g_object_unref (econtact);
}
