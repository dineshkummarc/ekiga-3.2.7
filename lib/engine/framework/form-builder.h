
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
 *                         form-builder.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of an object able to build a form
 *
 */

#ifndef __FORM_BUILDER_H__
#define __FORM_BUILDER_H__

#include <list>

#include "form.h"

namespace Ekiga
{

/**
 * @addtogroup forms
 * @{
 */

  class FormBuilder: public virtual Form, public virtual FormVisitor
  {
  public:

    FormBuilder ();

    /* form part */

    void visit (FormVisitor &visitor) const;

    const std::string hidden (const std::string name) const;

    bool boolean (const std::string name) const;

    const std::string text (const std::string name) const;

    const std::string private_text (const std::string name) const;

    const std::string multi_text (const std::string name) const;

    const std::string single_choice (const std::string name) const;

    const std::set<std::string> multiple_choice (const std::string name) const;

    const std::set<std::string> editable_set (const std::string name) const;

    /* builder part */

    void title (const std::string title);

    void instructions (const std::string instructions);

    void link (const std::string link,
               const std::string uri);

    void error (const std::string error);

    void hidden (const std::string name,
		 const std::string value);

    void boolean (const std::string name,
		  const std::string description,
		  bool value,
		  bool advanced = false);

    void text (const std::string text,
	       const std::string description,
	       const std::string value,
	       bool advanced = false);

    void private_text (const std::string text,
		       const std::string description,
		       const std::string value,
		       bool advanced = false);

    void multi_text (const std::string text,
		     const std::string description,
		     const std::string value,
		     bool advanced = false);

    void single_choice (const std::string name,
			const std::string description,
			const std::string value,
			const std::map<std::string, std::string> choices,
			bool advanced = false);

    void multiple_choice (const std::string name,
			  const std::string description,
			  const std::set<std::string> values,
			  const std::map<std::string, std::string> choices,
			  bool advanced = false);

    void editable_set (const std::string name,
		       const std::string description,
		       const std::set<std::string> values,
		       const std::set<std::string> proposed_values,
		       bool advanced = false);
  private:

    struct HiddenField
    {
      HiddenField (const std::string _name,
		   const std::string _value): name(_name), value(_value)
      {}

      const std::string name;
      const std::string value;
      bool advanced;
    };

    struct BooleanField
    {
      BooleanField (const std::string _name,
		    const std::string _description,
		    bool _value,
		    bool _advanced): name(_name), description(_description),
				     value(_value), advanced(_advanced)
      {}

      const std::string name;
      const std::string description;
      bool value;
      bool advanced;
    };

    struct TextField
    {
      TextField (const std::string _name,
		 const std::string _description,
		 const std::string _value,
		 bool _advanced): name(_name),
				  description(_description),
				  value(_value), advanced(_advanced)
      {}

      const std::string name;
      const std::string description;
      const std::string value;
      bool advanced;
    };

    struct MultiTextField
    {
      MultiTextField (const std::string _name,
		      const std::string _description,
		      const std::string _value,
		      bool _advanced): name(_name),
				       description(_description),
				       value(_value), advanced(_advanced)
      {}

      const std::string name;
      const std::string description;
      const std::string value;
      bool advanced;
    };

    struct SingleChoiceField
    {
      SingleChoiceField (const std::string _name,
			 const std::string _description,
			 const std::string _value,
			 const std::map<std::string, std::string> _choices,
			 bool _advanced):
	name(_name), description(_description),
	value(_value), choices(_choices), advanced(_advanced)
      {}

      const std::string name;
      const std::string description;
      const std::string value;
      const std::map<std::string, std::string> choices;
      bool advanced;
    };


    struct MultipleChoiceField
    {
      MultipleChoiceField (const std::string _name,
			   const std::string _description,
			   const std::set<std::string> _values,
			   const std::map<std::string, std::string> _choices,
			   bool _advanced):
	name(_name), description(_description),
	values(_values), choices(_choices), advanced(_advanced)
      {}

      const std::string name;
      const std::string description;
      const std::set<std::string> values;
      const std::map<std::string, std::string> choices;
      bool advanced;
    };

    struct EditableSetField
    {
      EditableSetField (const std::string _name,
			const std::string _description,
			const std::set<std::string> _values,
			const std::set<std::string> _proposed_values,
			bool _advanced):
	name(_name), description(_description),
	values(_values), proposed_values(_proposed_values), advanced(_advanced)
      {}

      const std::string name;
      const std::string description;
      const std::set<std::string> values;
      const std::set<std::string> proposed_values;
      bool advanced;
    };

    typedef enum {

      HIDDEN,
      BOOLEAN,
      TEXT,
      PRIVATE_TEXT,
      MULTI_TEXT,
      SINGLE_CHOICE,
      MULTIPLE_CHOICE,
      EDITABLE_SET
    } FieldType;

    std::string my_title;
    std::string my_instructions;
    std::pair<std::string, std::string> my_link;
    std::string my_error;
    std::list<FieldType> ordering;
    std::list<struct HiddenField> hiddens;
    std::list<struct BooleanField> booleans;
    std::list<struct TextField> texts;
    std::list<struct TextField> private_texts;
    std::list<struct MultiTextField> multi_texts;
    std::list<struct SingleChoiceField> single_choices;
    std::list<struct MultipleChoiceField> multiple_choices;
    std::list<struct EditableSetField> editable_sets;
  };

/**
 * @}
 */

};
#endif
