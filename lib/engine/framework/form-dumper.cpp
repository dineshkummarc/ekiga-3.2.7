
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
 *                         form-dumper.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : implementation of an object which prints forms
 *
 */

#include "form-dumper.h"

Ekiga::FormDumper::FormDumper (std::ostream &_out): out(_out)
{
}

void
Ekiga::FormDumper::dump (const Ekiga::Form &form)
{
  form.visit (*this);
}

void
Ekiga::FormDumper::title (const std::string _title)
{
  out << "Title: " << _title << std::endl;
}

void
Ekiga::FormDumper::instructions (const std::string _instructions)
{
  out << "Instructions: " << std::endl << _instructions << std::endl;
}

void
Ekiga::FormDumper::link (const std::string _link, const std::string _uri)
{
  out << "Link: " << _link << " , Uri: " << _uri << std::endl;
}

void
Ekiga::FormDumper::error (const std::string _error)
{
  out << "Error: " << _error << std::endl;
}

void
Ekiga::FormDumper::hidden (const std::string name,
			   const std::string value)
{
  out << "Hidden field " << name << ": " << value << std::endl;
}

void
Ekiga::FormDumper::boolean (const std::string name,
			    const std::string description,
			    bool value,
			    bool advanced)
{
  out << "Boolean field " << name
      << " (default value: ";
  if (value)
    out << "true";
  else
    out << "false";
  out << "):" << std::endl
      << description
      << (advanced?"[advanced]":"")
      << std::endl;
}

void
Ekiga::FormDumper::text (const std::string name,
			 const std::string description,
			 const std::string value,
			 bool advanced)
{
  out << "Text field " << name
      << " (default value: " << value << "): " << std::endl
      << description
      << (advanced?"[advanced]":"")
      << std::endl;
}

void
Ekiga::FormDumper::private_text (const std::string name,
				 const std::string description,
				 const std::string value,
				 bool advanced)
{
  out << "Private text field " << name
      << " (default value: " << value << "): " << std::endl
      << description
      << (advanced?"[advanced]":"")
      << std::endl;
}

void
Ekiga::FormDumper::multi_text (const std::string name,
			       const std::string description,
			       const std::string value,
			       bool advanced)
{
  out << "Multiline text field " << name
      << " (default value: " << value << "): " << std::endl
      << description
      << (advanced?"[advanced]":"")
      << std::endl;
}

void
Ekiga::FormDumper::single_choice (const std::string name,
				  const std::string description,
				  const std::string value,
				  const std::map<std::string, std::string> choices,
				  bool advanced)
{
  out << "Single choice list " << name
      << " (default choice: " << value << "): " << std::endl
      << description
      << (advanced?"[advanced]":"")
      << std::endl
      << "where choices are :" << std::endl;
  for (std::map<std::string, std::string>::const_iterator iter = choices.begin ();
       iter != choices.end ();
       iter++)
    out << "(" << iter->first << ", " << iter->second << ")" << std::endl;
}

void
Ekiga::FormDumper::multiple_choice (const std::string name,
				    const std::string description,
				    const std::set<std::string> values,
				    const std::map<std::string, std::string> choices,
				    bool advanced)
{
  out << "Multiple choice list " << name << ":" << std::endl
      << description
      << (advanced?"[advanced]":"")
      << std::endl
      << "where choices are :" << std::endl;
  for (std::map<std::string, std::string>::const_iterator iter = choices.begin ();
       iter != choices.end ();
       iter++) {

    out << "(" << iter->first << ", " << iter->second << ")";

    if (values.find (iter->first) != values.end ())
      out << " (V)" << std::endl;
    else
      out << " (X)" << std::endl;
  }
}

void
Ekiga::FormDumper::editable_set (const std::string name,
				 const std::string description,
				 const std::set<std::string> values,
				 const std::set<std::string> proposed_values,
				 bool advanced)
{
  out << "Editable list " << name << ":" << std::endl
      << description
      << (advanced?"[advanced]":"")
      << "where current set is :" << std::endl;
  for (std::set<std::string>::const_iterator iter = values.begin ();
       iter != values.end ();
       iter++)
    out << *iter << std::endl;
  out << "with proposed set of :" << std::endl;
  for (std::set<std::string>::const_iterator iter = proposed_values.begin ();
       iter != proposed_values.end ();
       iter++)
    out << *iter << std::endl;
}
