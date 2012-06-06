
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
 *                         form-request-simple.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a simple request form implementation
 *
 */

#ifndef __FORM_REQUEST_SIMPLE_H__
#define __FORM_REQUEST_SIMPLE_H__

#include <sigc++/sigc++.h>

#include "form-builder.h"
#include "form-request.h"

namespace Ekiga
{

/**
 * @addtogroup forms
 * @{
 */

  class FormRequestSimple: public FormRequest,
			   public FormBuilder
  {
  public:

    /* the callbacks gets two informations :
     * - a boolean, which is true if something was submitted, and false if
     * the request was cancelled ;
     * - a form, which contains the submitted answer (or is empty otherwise)
     */
    FormRequestSimple (sigc::slot2<void, bool, Form&> callback_);

    ~FormRequestSimple ();

    void cancel ();

    void submit (Form &);

  private:

    bool answered;
    sigc::slot2<void,bool,Form&> callback;

  };

/**
 * @}
 */

};

#endif
