
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
 *                         form-builder.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an object able to build a form
 *
 */

#include "form-builder.h"

Ekiga::FormBuilder::FormBuilder ()
{
  // nothing
}

void
Ekiga::FormBuilder::visit (Ekiga::FormVisitor &visitor) const
{
  std::list<struct HiddenField>::const_iterator iter_hidden = hiddens.begin ();
  std::list<struct BooleanField>::const_iterator iter_bool = booleans.begin ();
  std::list<struct TextField>::const_iterator iter_text = texts.begin ();
  std::list<struct TextField>::const_iterator iter_private_text = private_texts.begin ();
  std::list<struct MultiTextField>::const_iterator iter_multi_text = multi_texts.begin ();
  std::list<struct SingleChoiceField>::const_iterator iter_single_choice = single_choices.begin ();
  std::list<struct MultipleChoiceField>::const_iterator iter_multiple_choice = multiple_choices.begin ();
  std::list<struct EditableSetField>::const_iterator iter_editable_set = editable_sets.begin ();

  visitor.title (my_title);
  visitor.instructions (my_instructions);
  if (!my_link.first.empty () && !my_link.second.empty ())
    visitor.link (my_link.first, my_link.second); 
  visitor.error (my_error);

  for (std::list<FieldType>::const_iterator iter = ordering.begin ();
       iter != ordering.end ();
       iter++) {

    switch (*iter) {

    case HIDDEN:

      visitor.hidden (iter_hidden->name, iter_hidden->value);
      iter_hidden++;
      break;

    case BOOLEAN:

      visitor.boolean (iter_bool->name,
		       iter_bool->description,
		       iter_bool->value,
		       iter_bool->advanced);
      iter_bool++;
      break;

    case TEXT:

      visitor.text (iter_text->name, iter_text->description,
		    iter_text->value, iter_text->advanced);
      iter_text++;
      break;

    case PRIVATE_TEXT:

      visitor.private_text (iter_private_text->name,
			    iter_private_text->description,
			    iter_private_text->value,
			    iter_private_text->advanced);
      iter_private_text++;
      break;

    case MULTI_TEXT:

      visitor.multi_text (iter_multi_text->name,
			  iter_multi_text->description,
			  iter_multi_text->value,
			  iter_multi_text->advanced);
      iter_multi_text++;
      break;

    case SINGLE_CHOICE:

      visitor.single_choice (iter_single_choice->name,
			     iter_single_choice->description,
			     iter_single_choice->value,
			     iter_single_choice->choices,
			     iter_single_choice->advanced);
      iter_single_choice++;
      break;

    case MULTIPLE_CHOICE:

      visitor.multiple_choice (iter_multiple_choice->name,
			       iter_multiple_choice->description,
			       iter_multiple_choice->values,
			       iter_multiple_choice->choices,
			       iter_multiple_choice->advanced);
      iter_multiple_choice++;
      break;

    case EDITABLE_SET:

      visitor.editable_set (iter_editable_set->name,
			    iter_editable_set->description,
			    iter_editable_set->values,
			    iter_editable_set->proposed_values,
			    iter_editable_set->advanced);
      iter_editable_set++;
      break;

    default:
      break;
    }
  }
}

const std::string
Ekiga::FormBuilder::hidden (const std::string name) const
{
  for (std::list<struct HiddenField>::const_iterator iter = hiddens.begin ();
       iter != hiddens.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  return ""; // shouldn't happen
}

bool
Ekiga::FormBuilder::boolean (const std::string name) const
{
  for (std::list<struct BooleanField>::const_iterator iter = booleans.begin ();
       iter != booleans.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  return false; // shouldn't happen
}

const std::string
Ekiga::FormBuilder::private_text (const std::string name) const
{
  for (std::list<struct TextField>::const_iterator iter = private_texts.begin ();
       iter != private_texts.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  return ""; // shouldn't happen
}

const std::string
Ekiga::FormBuilder::text (const std::string name) const
{
  for (std::list<struct TextField>::const_iterator iter = texts.begin ();
       iter != texts.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  return ""; // shouldn't happen
}

const std::string
Ekiga::FormBuilder::multi_text (const std::string name) const
{
  for (std::list<struct MultiTextField>::const_iterator iter = multi_texts.begin ();
       iter != multi_texts.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  return ""; // shouldn't happen
}

const std::string
Ekiga::FormBuilder::single_choice (const std::string name) const
{
  for (std::list<struct SingleChoiceField>::const_iterator iter = single_choices.begin ();
       iter != single_choices.end ();
       iter++)
    if (iter->name == name)
      return iter->value;

  return ""; // shouldn't happen
}

const std::set<std::string>
Ekiga::FormBuilder::multiple_choice (const std::string name) const
{
  for (std::list<struct MultipleChoiceField>::const_iterator iter = multiple_choices.begin ();
       iter != multiple_choices.end ();
       iter++)
    if (iter->name == name)
      return iter->values;

  return std::set<std::string>(); // shouldn't happen
}

const std::set<std::string>
Ekiga::FormBuilder::editable_set (const std::string name) const
{
  for (std::list<struct EditableSetField>::const_iterator iter = editable_sets.begin ();
       iter != editable_sets.end ();
       iter++)
    if (iter->name == name)
      return iter->values;

  return std::set<std::string>(); // shouldn't happen
}

void
Ekiga::FormBuilder::title (const std::string _title)
{
  my_title = _title;
}

void
Ekiga::FormBuilder::instructions (const std::string _instructions)
{
  my_instructions = _instructions;
}

void
Ekiga::FormBuilder::link (const std::string _link,
                          const std::string _uri)
{
  my_link = make_pair (_link, _uri);
}

void
Ekiga::FormBuilder::error (const std::string _error)
{
  my_error = _error;
}

void
Ekiga::FormBuilder::hidden (const std::string name,
			    const std::string value)
{
  hiddens.push_back (HiddenField (name, value));
  ordering.push_back (HIDDEN);
}

void
Ekiga::FormBuilder::boolean (const std::string name,
			     const std::string description,
			     bool value,
			     bool advanced)
{
  booleans.push_back (BooleanField (name, description, value, advanced));
  ordering.push_back (BOOLEAN);
}

void
Ekiga::FormBuilder::text (const std::string name,
			  const std::string description,
			  const std::string value,
			  bool advanced)
{
  texts.push_back (TextField (name, description, value, advanced));
  ordering.push_back (TEXT);
}

void
Ekiga::FormBuilder::private_text (const std::string name,
				  const std::string description,
				  const std::string value,
				  bool advanced)
{
  private_texts.push_back (TextField (name, description, value, advanced));
  ordering.push_back (PRIVATE_TEXT);
}

void
Ekiga::FormBuilder::multi_text (const std::string name,
				const std::string description,
				const std::string value,
				bool advanced)
{
  multi_texts.push_back (MultiTextField (name, description, value, advanced));
  ordering.push_back (MULTI_TEXT);
}

void
Ekiga::FormBuilder::single_choice (const std::string name,
				   const std::string description,
				   const std::string value,
				   const std::map<std::string, std::string> choices,
				   bool advanced)
{
  single_choices.push_back (SingleChoiceField (name, description,
					       value, choices, advanced));
  ordering.push_back (SINGLE_CHOICE);
}

void
Ekiga::FormBuilder::multiple_choice (const std::string name,
				     const std::string description,
				     const std::set<std::string> values,
				     const std::map<std::string, std::string> choices,
				     bool advanced)
{
  multiple_choices.push_back (MultipleChoiceField (name, description,
						   values, choices, advanced));
  ordering.push_back (MULTIPLE_CHOICE);
}

void
Ekiga::FormBuilder::editable_set (const std::string name,
				  const std::string description,
				  const std::set<std::string> values,
				  const std::set<std::string> proposed_values,
				  bool advanced)
{
  editable_sets.push_back (EditableSetField (name, description, values,
					     proposed_values, advanced));
  ordering.push_back (EDITABLE_SET);
}
