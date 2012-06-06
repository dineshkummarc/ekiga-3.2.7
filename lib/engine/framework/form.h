
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
 *                         form.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an abstract form
 *
 */

#ifndef __FORM_H__
#define __FORM_H__

#include "form-visitor.h"

namespace Ekiga
{

/**
 * @defgroup forms Forms
 * @{
 */

  class Form
  {
  public:

    virtual ~Form () {}

    virtual void visit (FormVisitor &visitor) const = 0;

    virtual const std::string hidden (const std::string name) const = 0;

    virtual bool boolean (const std::string name) const = 0;

    virtual const std::string text (const std::string name) const = 0;

    virtual const std::string private_text (const std::string name) const = 0;

    virtual const std::string multi_text (const std::string name) const = 0;

    virtual const std::string single_choice (const std::string name) const = 0;

    virtual const std::set<std::string> multiple_choice (const std::string name) const = 0;

    virtual const std::set<std::string> editable_set (const std::string name) const = 0;
  };

/**
 * @}
 */

  class EmptyForm: public Form
  {
  public:

    EmptyForm ();

    ~EmptyForm ();

    void visit (FormVisitor &visitor) const;

    const std::string hidden (const std::string name) const;

    bool boolean (const std::string name) const;

    const std::string text (const std::string name) const;

    const std::string private_text (const std::string name) const;

    const std::string multi_text (const std::string name) const;

    const std::string single_choice (const std::string name) const;

    const std::set<std::string> multiple_choice (const std::string name) const;

    const std::set<std::string> editable_set (const std::string name) const;
  };

};

#endif
