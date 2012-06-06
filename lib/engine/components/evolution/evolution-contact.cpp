
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
 *                         evolution-contact.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an evolution contact
 *
 */

#include <iostream>
#include <glib/gi18n.h>

#include "evolution-contact.h"
#include "form-request-simple.h"
#include "menu-builder-tools.h"

Evolution::Contact::Contact (Ekiga::ServiceCore &_services,
			     EBook *ebook,
			     EContact *_econtact) : services(_services),
						    book(ebook),
						    econtact(NULL)
{
  for (unsigned int ii = 0;
       ii < ATTR_NUMBER;
       ii++)
    attributes[ii] = NULL;

  if (E_IS_CONTACT (_econtact))
    update_econtact (_econtact);
}

Evolution::Contact::~Contact ()
{
  if (E_IS_CONTACT (econtact))
    g_object_unref (econtact);
}

const std::string
Evolution::Contact::get_id () const
{
  std::string id;

  id = (gchar *)e_contact_get_const (econtact, E_CONTACT_UID);

  return id;
}

const std::string
Evolution::Contact::get_name () const
{
  std::string name;

  name = (const gchar *)e_contact_get_const (econtact,
					     E_CONTACT_FULL_NAME);

  return name;
}

const std::set<std::string>
Evolution::Contact::get_groups () const
{
  std::set<std::string> groups;
  gchar *categories = NULL;

  categories = (gchar *)e_contact_get_const (econtact, E_CONTACT_CATEGORIES);

  if (categories != NULL) {

    gchar **split = g_strsplit (categories, ",", 0);
    for (gchar **ptr = split; *ptr != NULL; ptr++)
      groups.insert (*ptr);
    g_strfreev (split);
  }

  return groups;
}

bool
Evolution::Contact::is_found (const std::string /*test*/) const
{
  return false;
}

void
Evolution::Contact::update_econtact (EContact *_econtact)
{
  GList *attrs = NULL;

  if (E_IS_CONTACT (econtact))
    g_object_unref (econtact);

  econtact = _econtact;
  g_object_ref (econtact);

  for (unsigned int ii = 0;
       ii < ATTR_NUMBER;
       ii++)
    attributes[ii] = NULL;

  attrs = e_vcard_get_attributes (E_VCARD (econtact));

  for (GList *attribute_ptr = attrs ;
       attribute_ptr != NULL;
       attribute_ptr = g_list_next (attribute_ptr)) {

    EVCardAttribute *attribute = (EVCardAttribute *)attribute_ptr->data;
    std::string attr_name = e_vcard_attribute_get_name (attribute);

    if (attr_name == EVC_TEL) {

      GList *params = e_vcard_attribute_get_params (attribute);
      for (GList *param_ptr = params;
	   param_ptr != NULL;
	   param_ptr = g_list_next (param_ptr)) {

	EVCardAttributeParam *param = (EVCardAttributeParam *)param_ptr->data;
	const gchar *param_name_raw = NULL;
	gchar *param_name_cased = NULL;
	std::string param_name;

	param_name_raw = e_vcard_attribute_param_get_name (param);
	param_name_cased = g_utf8_strup (param_name_raw, -1);
	param_name = param_name_cased;
	g_free (param_name_cased);

	if (param_name == "TYPE") {

	  for (GList *type_ptr = e_vcard_attribute_param_get_values (param);
	       type_ptr != NULL;
	       type_ptr = g_list_next (type_ptr)) {

	    const gchar *type_name_raw = NULL;
	    gchar *type_name_cased = NULL;
	    std::string type_name;

	    type_name_raw = (const gchar *)type_ptr->data;
	    type_name_cased = g_utf8_strup (type_name_raw, -1);
	    type_name = type_name_cased;
	    g_free (type_name_cased);

	    if (type_name == "HOME") {

	      attributes[ATTR_HOME] = attribute;
	      break;
	    } else if (type_name == "CELL") {

	      attributes[ATTR_CELL] = attribute;
	      break;
	    } else if (type_name == "WORK") {

	      attributes[ATTR_WORK] = attribute;
	      break;
	    } else if (type_name == "PAGER") {

	      attributes[ATTR_PAGER] = attribute;
	      break;
	    } else if (type_name == "VIDEO") {

	      attributes[ATTR_VIDEO] = attribute;
	      break;
	    }
	  }
	}
      }
    }
  }
  updated.emit ();
}

void
Evolution::Contact::remove ()
{
  e_book_remove_contact (book, get_id().c_str (), NULL);
}

bool
Evolution::Contact::populate_menu (Ekiga::MenuBuilder &builder)
{
  gmref_ptr<Ekiga::ContactCore> core = services.get ("contact-core");
  bool populated = false;
  std::map<std::string, std::string> uris;

  if (core) {

    Ekiga::TemporaryMenuBuilder tmp_builder;

    for (unsigned int attr_type = 0; attr_type < ATTR_NUMBER; attr_type++) {

      std::string attr_value = get_attribute_value (attr_type);
      if ( !attr_value.empty ()) {

	if (core->populate_contact_menu (ContactPtr(this),
					 attr_value, tmp_builder)) {

	  builder.add_ghost ("", get_attribute_name_from_type (attr_type));
	  tmp_builder.populate_menu (builder);
	  populated = true;
	}
      }
    }
  }

  if (populated)
    builder.add_separator ();

  builder.add_action ("edit", _("_Edit"),
		      sigc::mem_fun (this, &Evolution::Contact::edit_action));
  builder.add_action ("remove", _("_Remove"),
		      sigc::mem_fun (this, &Evolution::Contact::remove_action));
  populated = true;

  return populated;
}


std::string
Evolution::Contact::get_attribute_name_from_type (unsigned int attribute_type) const
{
  std::string result;

  switch (attribute_type) {

  case ATTR_HOME:
    result = "HOME";
    break;
  case ATTR_CELL:
    result = "CELL";
    break;
  case ATTR_WORK:
    result = "WORK";
    break;
  case ATTR_PAGER:
    result = "PAGER";
    break;
  case ATTR_VIDEO:
    result = "VIDEO";
    break;
  default:
    result = "";
    break;
  }

  return result;
}

std::string
Evolution::Contact::get_attribute_value (unsigned int attr_type) const
{
  EVCardAttribute *attribute = attributes[attr_type];

  if (attribute != NULL) {

    GList *values = e_vcard_attribute_get_values_decoded (attribute);
    if (values != NULL)
      return ((GString *)values->data)->str; // only the first
    else
      return "";
  } else
    return "";
}

void
Evolution::Contact::set_attribute_value (unsigned int attr_type,
					 const std::string value)
{
  EVCardAttribute *attribute = attributes[attr_type];

  if ( !value.empty ()) {

    if (attribute == NULL) {

      EVCardAttributeParam *param = NULL;

      attribute = e_vcard_attribute_new ("", EVC_TEL);
      param = e_vcard_attribute_param_new (EVC_TYPE);
      e_vcard_attribute_param_add_value (param,
					 get_attribute_name_from_type (attr_type).c_str ());
      e_vcard_attribute_add_param (attribute, param);
      e_vcard_add_attribute (E_VCARD (econtact), attribute);

      attributes[attr_type]=attribute;
    }
    e_vcard_attribute_remove_values (attribute);
    e_vcard_attribute_add_value (attribute, value.c_str ());
  } else { // empty valued : remove the attribute

    e_vcard_remove_attribute (E_VCARD (econtact), attribute);
    attributes[attr_type] = NULL;
  }
}

void
Evolution::Contact::edit_action ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &Evolution::Contact::on_edit_form_submitted));;

  request.title (_("Edit contact"));

  request.instructions (_("Please update the following fields:"));

  request.text ("name", _("Name:"), get_name ());

  {
    std::string home_uri = get_attribute_value (ATTR_HOME);
    std::string cell_phone_uri = get_attribute_value (ATTR_CELL);
    std::string work_uri = get_attribute_value (ATTR_WORK);
    std::string pager_uri = get_attribute_value (ATTR_PAGER);
    std::string video_uri = get_attribute_value (ATTR_VIDEO);

    request.text ("video", _("VoIP _URI:"), video_uri);
    request.text ("home", _("_Home phone:"), home_uri);
    request.text ("work", _("_Office phone:"), work_uri);
    request.text ("cell", _("_Cell phone:"), cell_phone_uri);
    request.text ("pager", _("_Pager:"), pager_uri);
  }

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
Evolution::Contact::on_edit_form_submitted (bool submitted,
					    Ekiga::Form &result)
{
  if (!submitted)
    return;

  std::string name = result.text ("name");
  std::string home = result.text ("home");
  std::string cell = result.text ("cell");
  std::string work = result.text ("work");
  std::string pager = result.text ("pager");
  std::string video = result.text ("video");

  set_attribute_value (ATTR_HOME, home);
  set_attribute_value (ATTR_CELL, cell);
  set_attribute_value (ATTR_WORK, work);
  set_attribute_value (ATTR_PAGER, pager);
  set_attribute_value (ATTR_VIDEO, video);

  e_contact_set (econtact, E_CONTACT_FULL_NAME, (gpointer)name.c_str ());

  e_book_commit_contact (book, econtact, NULL);
}

void
Evolution::Contact::remove_action ()
{
  Ekiga::FormRequestSimple request(sigc::mem_fun (this, &Evolution::Contact::on_remove_form_submitted));;
  gchar* instructions = NULL;

  request.title (_("Remove contact"));

  instructions = g_strdup_printf (_("Are you sure you want to remove %s from the addressbook?"), get_name ().c_str ());
  request.instructions (instructions);
  g_free (instructions);

  if (!questions.handle_request (&request)) {

    // FIXME: better error reporting
#ifdef __GNUC__
    std::cout << "Unhandled form request in "
	      << __PRETTY_FUNCTION__ << std::endl;
#endif
  }
}

void
Evolution::Contact::on_remove_form_submitted (bool submitted,
					      Ekiga::Form& /*result*/)
{
  if (submitted)
    remove ();
}
