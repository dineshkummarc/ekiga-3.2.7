
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
 *                         form-dialog.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007 by Julien Puydt
 *   description          : declaration of a gtk+ representation of forms
 *
 */

#ifndef __FORM_DIALOG_GTK_H__
#define __FORM_DIALOG_GTK_H__

#include <gtk/gtk.h>

#include "form-builder.h"
#include "form-request.h"

/* abstract helper class common to all field types */
class Submitter
{
public:

  virtual ~Submitter () {}

  virtual void submit (Ekiga::FormBuilder &builder) = 0;
};

class FormDialog: public Ekiga::FormVisitor
{
public:

  FormDialog (Ekiga::FormRequest &request,
	      GtkWidget *parent);

  ~FormDialog ();

  void run ();

  /* FormVisitor api */

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
		bool advanced);

  void text (const std::string name,
	     const std::string description,
	     const std::string value,
	     bool advanced);

  void private_text (const std::string name,
		     const std::string description,
		     const std::string value,
		     bool advanced);

  void multi_text (const std::string name,
		   const std::string description,
		   const std::string value,
		   bool advanced);

  void single_choice (const std::string name,
		      const std::string description,
		      const std::string value,
		      const std::map<std::string, std::string> choices,
		      bool advanced);

  void multiple_choice (const std::string name,
			const std::string description,
			const std::set<std::string> values,
			const std::map<std::string, std::string> choices,
			bool advanced);

  void editable_set (const std::string name,
		     const std::string description,
		     const std::set<std::string> values,
		     const std::set<std::string> proposed_values,
		     bool advanced);

  /* those are public only to be called from C code */

  void cancel ();

  void submit ();

private:

  void grow_fields (bool advanced);

  Ekiga::FormRequest &request;
  GtkWidget *window;
  GtkWidget *preamble;
  GtkWidget *fields;
  GtkWidget* expander;
  GtkWidget* advanced_fields;
  GtkSizeGroup *labels_group;
  GtkSizeGroup *options_group;
  unsigned int rows;
  unsigned int advanced_rows;
  std::list<Submitter *> submitters;
};

#endif
