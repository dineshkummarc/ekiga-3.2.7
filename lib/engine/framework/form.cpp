
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
 *                         form.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2008 by Julien Puydt
 *   copyright            : (c) 2008 by Julien Puydt
 *   description          : implementation of the empty form
 *
 */

#include "form.h"


Ekiga::EmptyForm::EmptyForm ()
{
}

Ekiga::EmptyForm::~EmptyForm ()
{
}

void
Ekiga::EmptyForm::visit (FormVisitor &/*visitor*/) const

{
}

const std::string
Ekiga::EmptyForm::hidden (const std::string /*name*/) const
{
  return "";
}

bool Ekiga::EmptyForm::boolean (const std::string /*name*/) const
{
  return false;
}

const std::string
Ekiga::EmptyForm::text (const std::string /*name*/) const
{
  return "";
}

const std::string
Ekiga::EmptyForm::private_text (const std::string /*name*/) const
{
  return "";
}

const std::string
Ekiga::EmptyForm::multi_text (const std::string /*name*/) const
{
  return "";
}

const std::string
Ekiga::EmptyForm::single_choice (const std::string /*name*/) const
{
  return "";
}

const std::set<std::string>
Ekiga::EmptyForm::multiple_choice (const std::string /*name*/) const
{
  return std::set<std::string>();
}

const std::set<std::string>
Ekiga::EmptyForm::editable_set (const std::string /*name*/) const
{
  return std::set<std::string>();
}
