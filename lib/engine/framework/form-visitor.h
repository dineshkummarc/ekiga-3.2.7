
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
 *                         form-visitor.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an object parsing a form
 *
 */

#ifndef __FORM_VISITOR_H__
#define __FORM_VISITOR_H__

#include <map>
#include <set>
#include <string>

namespace Ekiga
{

/**
 * @addtogroup forms
 * @{
 */

  class FormVisitor
  {
  public:

    virtual ~FormVisitor () {}

    virtual void title (const std::string title) = 0;

    virtual void instructions (const std::string instructions) = 0;
    
    virtual void link (const std::string link,
                       const std::string uri) = 0;

    virtual void error (const std::string msg) = 0;

    virtual void hidden (const std::string name,
			 const std::string value) = 0;

    virtual void boolean (const std::string name,
			  const std::string description,
			  bool value,
			  bool advanced) = 0;

    virtual void text (const std::string name,
		       const std::string description,
		       const std::string value,
		       bool advanced) = 0;

    virtual void private_text (const std::string name,
			       const std::string description,
			       const std::string value,
			       bool advanced) = 0;

    virtual void multi_text (const std::string name,
			     const std::string description,
			     const std::string value,
			     bool advanced) = 0;

    virtual void single_choice (const std::string name,
				const std::string description,
				const std::string value,
				/* pairs (value, name),
				 * where value is internal and name shown to
				 * the user (hence translated) */
				const std::map<std::string,std::string> choices,
				bool advanced) = 0;

    virtual void multiple_choice (const std::string name,
				  const std::string description,
				  const std::set<std::string> values,
				  /* pairs (value, name),
				   * where value is internal and name shown to
				   * the user (hence translated) */
				  const std::map<std::string,std::string> choices,
				  bool advanced) = 0;

    virtual void editable_set (const std::string name,
			       const std::string description,
			       const std::set<std::string> values,
			       const std::set<std::string> proposed_values,
			       bool advanced) = 0;
  };

/**
 * @}
 */

};

#endif
