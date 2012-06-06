
/*
 * Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2009 Damien Sandras <dsandras@seconix.com>
 *
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
 *                         form-dialog.cpp  -  description
 *                         ------------------------------------------
 *   begin                : written in 2007 by Julien Puydt
 *   copyright            : (c) 2007-2008 by Julien Puydt
 *   description          : implementation of a gtk+ representation of forms
 *
 */


#include <algorithm>
#include <cstring>
#include <iostream>
#include <glib/gi18n.h>

#include "toolbox.h"
#include "form-dialog-gtk.h"

/*
 * Declarations : GTK+ Callbacks
 */

/** Called when a choice has been toggled in the
 * GtkListStore.
 *
 * Toggle the choice.
 *
 * @param: data is a pointer to the GtkListStore representing
 * the list of choices.
 */
static void
multiple_choice_choice_toggled_cb (GtkCellRendererToggle *cell,
				   gchar *path_str,
				   gpointer data);


/** Called when the GtkEntry aiming at adding a new
 * value has been activated.
 * Checks if the proposed value is not already in
 * the list, add it to the values if it is not the
 * case.
 *
 * @param: data is a pointer to the GtkListStore presenting
 * the list of values.
 */
static void
editable_set_add_value_activated_cb (GtkWidget *entry,
				     gpointer data);


/** Called when the GtkButton to add a value
 * has been clicked.
 *
 * Emit the 'activated' signal on the GtkEntry
 * to trigger editable_set_add_value_activated_cb.
 *
 * @param: data is a pointer to the GtkEntry containing
 * the new value.
 */
static void
editable_set_add_value_clicked_cb (GtkWidget *button,
				   gpointer data);


/** Called when a choice has been toggled in the
 * GtkListStore.
 *
 * Toggle the choice.
 *
 * @param: data is a pointer to the GtkListStore representing
 * the list of choices.
 */
static void
editable_set_choice_toggled_cb (GtkCellRendererToggle *cell,
				gchar *path_str,
				gpointer data);


/** Called when a link in a Form is clicked.
 * Open the URI.
 *
 * @param: The URI to open.
 */
static void
link_clicked_cb (GtkWidget *widget,
                 gpointer data);


/*
 * Declarations and implementation : the various submitters
 * of a Form
 */
class TitleSubmitter: public Submitter
{
public:

  TitleSubmitter (const std::string _title):
    title (_title)
  { }

  ~TitleSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.title (title);
  }

private:

  const std::string title;
};


class InstructionsSubmitter: public Submitter
{
public:

  InstructionsSubmitter (const std::string _instructions): instructions (_instructions)
  { }

  ~InstructionsSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.instructions (instructions);
  }

private:

  const std::string instructions;
};


class HiddenSubmitter: public Submitter
{
public:

  HiddenSubmitter (const std::string _name,
		   const std::string _value): name(_name), value(_value)
  {}

  ~HiddenSubmitter ()
  {}

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.hidden (name, value);
  }

private:

  const std::string name;
  const std::string value;
};


class BooleanSubmitter: public Submitter
{
public:

  BooleanSubmitter (const std::string _name,
		    const std::string _description,
		    bool _advanced,
		    GtkWidget *_widget): name(_name),
					 description(_description),
					 advanced(_advanced),
					 widget(_widget)
  { }

  ~BooleanSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.boolean (name, description,
		     gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)),
		     advanced);
  }

private:

  const std::string name;
  const std::string description;
  bool advanced;
  GtkWidget *widget;
};


class TextSubmitter: public Submitter
{
public:

  TextSubmitter (const std::string _name,
		 const std::string _description,
		 bool _advanced,
		 GtkWidget *_widget): name(_name),
				      description(_description),
				      advanced(_advanced),
				      widget(_widget)
  { }

  ~TextSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.text (name, description,
		  gtk_entry_get_text (GTK_ENTRY (widget)),
		  advanced);
  }

private:

  const std::string name;
  const std::string description;
  bool advanced;
  GtkWidget *widget;
};


class PrivateTextSubmitter: public Submitter
{
public:

  PrivateTextSubmitter (const std::string _name,
			const std::string _description,
			bool _advanced,
			GtkWidget *_widget): name(_name),
					     description(_description),
					     advanced(_advanced),
					     widget(_widget)
  { }

  ~PrivateTextSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    builder.private_text (name, description,
			  gtk_entry_get_text (GTK_ENTRY (widget)),
			  advanced);
  }

private:

  const std::string name;
  const std::string description;
  bool advanced;
  GtkWidget *widget;
};


class MultiTextSubmitter: public Submitter
{
public:

  MultiTextSubmitter (const std::string _name,
		      const std::string _description,
		      bool _advanced,
		      GtkTextBuffer *_buffer): name(_name),
					       description(_description),
					       advanced(_advanced),
					       buffer(_buffer)
  { }

  ~MultiTextSubmitter ()
  { }

  void submit (Ekiga::FormBuilder &builder)
  {
    GtkTextIter start;
    GtkTextIter end;

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_get_end_iter (buffer, &end);
    builder.multi_text (name, description,
			gtk_text_buffer_get_text (buffer,
						  &start, &end, FALSE),
			advanced);
  }

private:

  const std::string name;
  const std::string description;
  bool advanced;
  GtkTextBuffer *buffer;
};


class SingleChoiceSubmitter: public Submitter
{
public:

  SingleChoiceSubmitter (const std::string _name,
			 const std::string _description,
			 const std::map<std::string, std::string> _choices,
			 bool _advanced,
			 GtkWidget *_combo): name(_name),
					     description(_description),
					     choices(_choices),
					     advanced(_advanced),
					     combo(_combo)
  { }

  ~SingleChoiceSubmitter ()
  { }

  enum {

    COLUMN_VALUE,
    COLUMN_NAME,
    COLUMN_NUMBER
  };

  void submit (Ekiga::FormBuilder &builder)
  {
    gchar *cvalue = NULL;
    GtkTreeModel *model = gtk_combo_box_get_model (GTK_COMBO_BOX (combo));
    GtkTreeIter iter;

    gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combo), &iter);

    gtk_tree_model_get (model, &iter, COLUMN_VALUE, &cvalue, -1);

    builder.single_choice (name, description,
			   std::string (cvalue), choices,
			   advanced);

    g_free (cvalue);
  }

private:

  const std::string name;
  const std::string description;
  const std::map<std::string, std::string> choices;
  bool advanced;
  GtkWidget *combo;
};


class MultipleChoiceSubmitter: public Submitter
{
public:

  MultipleChoiceSubmitter (const std::string _name,
			   const std::string _description,
			   const std::map<std::string, std::string> _choices,
			   bool _advanced,
			   GtkWidget *_tree_view):
    name(_name), description(_description),
    choices(_choices), advanced(_advanced), tree_view (_tree_view)
  { }

  ~MultipleChoiceSubmitter ()
  { }

  enum {

    COLUMN_ACTIVE,
    COLUMN_NAME,
    COLUMN_NUMBER
  };

  void submit (Ekiga::FormBuilder &builder)
  {
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    gboolean active = FALSE;

    std::set<std::string> values;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

      do {

	gchar *gname = NULL;

        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                            COLUMN_ACTIVE, &active,
                            COLUMN_NAME, &gname,
                            -1);

        if (active && gname) {

          values.insert (gname);

          std::map <std::string, std::string>::const_iterator mit;
          mit = choices.find (gname);
          if (mit == choices.end ())
            choices [gname] = gname;
        }

	g_free (gname);
      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }

    builder.multiple_choice (name, description, values, choices, advanced);
  }

private:

  const std::string name;
  const std::string description;
  std::map<std::string, std::string> choices;
  bool advanced;
  GtkWidget *tree_view;
};


class EditableSetSubmitter: public Submitter
{
public:

  EditableSetSubmitter (const std::string _name,
			const std::string _description,
			bool _advanced,
			GtkWidget *_tree_view):
    name(_name), description(_description), advanced(_advanced), tree_view(_tree_view)
  { }

  ~EditableSetSubmitter ()
  { }

  enum {

    COLUMN_ACTIVE,
    COLUMN_VALUE,
    COLUMN_NUMBER
  };

  void submit (Ekiga::FormBuilder &builder)
  {
    GtkTreeModel *model = NULL;
    GtkTreeIter iter;
    std::set<std::string> values;
    std::set<std::string> proposed_values;

    model = gtk_tree_view_get_model (GTK_TREE_VIEW (tree_view));
    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

      do {

	gboolean active = FALSE;
	gchar *value = NULL;

	gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
			    COLUMN_ACTIVE, &active,
			    COLUMN_VALUE, &value,
			    -1);

	if (value) {

	  if (active)
	    values.insert (value);
	  else
	    proposed_values.insert (value);
	  g_free (value);
	}
      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }

    builder.editable_set (name, description, values, proposed_values, advanced);
  }

private:

  const std::string name;
  const std::string description;
  bool advanced;
  GtkWidget *tree_view;
};


/*
 * GTK+ Callbacks
 */

static void
editable_set_add_value_activated_cb (GtkWidget *entry,
				     gpointer data)
{
  GtkTreeModel *model = NULL;

  const char *value = NULL;
  gchar *tree_value = NULL;

  GtkTreeIter iter;

  model = gtk_tree_view_get_model (GTK_TREE_VIEW (data));

  value = gtk_entry_get_text (GTK_ENTRY (entry));
  if (!strcmp (value, ""))
    return;

  if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter)) {

    do {

      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter,
                          EditableSetSubmitter::COLUMN_VALUE, &tree_value,
                          -1);
      if (tree_value && !strcmp (tree_value, value)) {
        g_free (tree_value);
        return;
      }
      g_free (tree_value);

    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
  }

  gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);

  gtk_list_store_prepend (GTK_LIST_STORE (model), &iter);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
		      EditableSetSubmitter::COLUMN_ACTIVE, TRUE,
                      EditableSetSubmitter::COLUMN_VALUE, gtk_entry_get_text (GTK_ENTRY (entry)),
                      -1);

  gtk_entry_set_text (GTK_ENTRY (entry), "");
}


static void
editable_set_add_value_clicked_cb (G_GNUC_UNUSED GtkWidget *button,
				   gpointer data)
{
  if (strcmp (gtk_entry_get_text (GTK_ENTRY (data)), ""))
    gtk_widget_activate (GTK_WIDGET (data));
}


static void
editable_set_choice_toggled_cb (G_GNUC_UNUSED GtkCellRendererToggle *cell,
				gchar *path_str,
				gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  GtkTreeIter iter;
  gboolean fixed = false;

  model = GTK_TREE_MODEL (data);
  path = gtk_tree_path_new_from_string (path_str);

  /* Update the tree model */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter,
		      EditableSetSubmitter::COLUMN_ACTIVE, &fixed,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      EditableSetSubmitter::COLUMN_ACTIVE, fixed^1,
		      -1);
  gtk_tree_path_free (path);
}


static void
multiple_choice_choice_toggled_cb (G_GNUC_UNUSED GtkCellRendererToggle *cell,
				   gchar *path_str,
				   gpointer data)
{
  GtkTreeModel *model = NULL;
  GtkTreePath *path = NULL;

  GtkTreeIter iter;
  gboolean fixed = false;

  model = GTK_TREE_MODEL (data);
  path = gtk_tree_path_new_from_string (path_str);

  /* Update the tree model */
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_model_get (model, &iter,
		      MultipleChoiceSubmitter::COLUMN_ACTIVE, &fixed,
		      -1);
  gtk_list_store_set (GTK_LIST_STORE (model), &iter,
                      MultipleChoiceSubmitter::COLUMN_ACTIVE, fixed^1,
		      -1);
  gtk_tree_path_free (path);
}


static void
link_clicked_cb (GtkWidget * /*widget*/,
                 gpointer data)
{
  gm_open_uri ((gchar *) data);
}


FormDialog::FormDialog (Ekiga::FormRequest &_request,
			GtkWidget *parent): request(_request)
{
  GtkWidget *vbox = NULL;

  rows = 0;
  advanced_rows = 0;

  window = gtk_dialog_new_with_buttons (NULL, GTK_WINDOW (NULL),
                                        GTK_DIALOG_MODAL,
                                        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                        GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
                                        NULL);
  if (GTK_IS_WINDOW (parent))
    gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (parent));

  gtk_dialog_set_default_response (GTK_DIALOG (window),
                                   GTK_RESPONSE_ACCEPT);
  gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), vbox, FALSE, FALSE, 0);
  gtk_widget_show (vbox);

  preamble = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox), preamble, FALSE, FALSE, 0);

  fields = gtk_table_new (0, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (fields), 2);
  gtk_table_set_col_spacings (GTK_TABLE (fields), 2);
  gtk_box_pack_start (GTK_BOX (vbox), fields, FALSE, FALSE, 3);

  advanced_fields = gtk_table_new (0, 2, FALSE);
  gtk_table_set_row_spacings (GTK_TABLE (advanced_fields), 2);
  gtk_table_set_col_spacings (GTK_TABLE (advanced_fields), 2);
  expander = gtk_expander_new (_("Advanced"));
  gtk_container_add (GTK_CONTAINER (expander), advanced_fields);
  gtk_box_pack_start (GTK_BOX (vbox), expander, FALSE, FALSE, 3);

  labels_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);
  options_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

  request.visit (*this);
}


FormDialog::~FormDialog ()
{
  gtk_widget_destroy (GTK_WIDGET (window));
  for (std::list<Submitter *>::iterator iter = submitters.begin ();
       iter != submitters.end ();
       iter++)
    delete (*iter);
  submitters.clear ();
}


void
FormDialog::run ()
{
  gtk_widget_show_all (preamble);
  gtk_widget_show_all (fields);
  if (advanced_rows > 0)
    gtk_widget_show_all (expander);
  gtk_widget_show (window);
  switch (gtk_dialog_run (GTK_DIALOG (window))) {

  case GTK_RESPONSE_ACCEPT:
    submit();
    break;

  case GTK_RESPONSE_CANCEL:
  case GTK_RESPONSE_DELETE_EVENT:
  default:
    cancel();
    break;
  }
}


void
FormDialog::title (const std::string _title)
{
  TitleSubmitter *submitter = NULL;

  gtk_window_set_title (GTK_WINDOW (window), _title.c_str ());
  submitter = new TitleSubmitter (_title);
  submitters.push_back (submitter);
}


void
FormDialog::instructions (const std::string _instructions)
{
  GtkWidget *widget = NULL;
  InstructionsSubmitter *submitter = NULL;
  gchar * label_text = NULL;

  widget = gtk_label_new (NULL);
  label_text = g_strdup_printf ("<i>%s</i>", _instructions.c_str());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (widget), label_text);
  g_free (label_text);

  gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
  gtk_label_set_line_wrap_mode (GTK_LABEL (widget), PANGO_WRAP_WORD);
  gtk_box_pack_start (GTK_BOX (preamble), widget, FALSE, FALSE, 0);

  submitter = new InstructionsSubmitter (_instructions);
  submitters.push_back (submitter);
}


void
FormDialog::link (const std::string _link,
                  const std::string _uri)
{
  GtkWidget *widget = NULL;
  GtkWidget *label = NULL;
  gchar *label_text = NULL;

  widget = gtk_button_new ();
  label = gtk_label_new (NULL);
  label_text = g_strdup_printf ("<span foreground=\"blue\"><u>%s</u></span>",
                                _link.c_str ());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), label_text);
  g_free (label_text);

  gtk_container_add (GTK_CONTAINER (widget), label);

  gtk_button_set_relief (GTK_BUTTON (widget), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (preamble), widget, FALSE, FALSE, 0);

  g_signal_connect_data (G_OBJECT (widget), "clicked",
                         G_CALLBACK (link_clicked_cb), (gpointer) g_strdup (_uri.c_str ()),
                         (GClosureNotify) g_free, (GConnectFlags) 0);
}


void
FormDialog::error (const std::string _error)
{
  GtkWidget *widget = NULL;

  if (!_error.empty ()) {

    widget = gtk_label_new (NULL);
    gtk_label_set_line_wrap (GTK_LABEL (widget), TRUE);
    gtk_label_set_line_wrap_mode (GTK_LABEL (widget), PANGO_WRAP_WORD);
    gtk_label_set_markup_with_mnemonic (GTK_LABEL (widget),
					("<span foreground=\"red\">" + _error + "</span>").c_str ());
    gtk_container_add (GTK_CONTAINER (preamble), widget);
  }
}


void
FormDialog::hidden (const std::string name,
		    const std::string value)
{
  HiddenSubmitter *submitter = NULL;

  submitter = new HiddenSubmitter (name, value);
  submitters.push_back (submitter);
}

void
FormDialog::boolean (const std::string name,
		     const std::string description,
		     bool value,
		     bool advanced)
{
  GtkWidget *widget = NULL;
  BooleanSubmitter *submitter = NULL;

  grow_fields (advanced);

  widget = gtk_check_button_new_with_label (description.c_str ());
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), value);
  if (advanced) {

    gtk_table_attach_defaults (GTK_TABLE (advanced_fields), widget,
			       0, 2, advanced_rows -1, advanced_rows);
  } else {

    gtk_table_attach_defaults (GTK_TABLE (fields), widget,
			       0, 2, rows -1, rows);
  }

  submitter = new BooleanSubmitter (name, description, advanced, widget);
  submitters.push_back (submitter);
}

void
FormDialog::text (const std::string name,
		  const std::string description,
		  const std::string value,
		  bool advanced)
{
  GtkWidget *label = NULL;
  GtkWidget *widget = NULL;
  TextSubmitter *submitter = NULL;

  gchar *label_text = NULL;

  grow_fields (advanced);

  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", description.c_str());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), label_text);
  g_free (label_text);

  widget = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
  gtk_entry_set_activates_default (GTK_ENTRY (widget), true);
  gtk_size_group_add_widget (options_group, widget);
  gtk_entry_set_text (GTK_ENTRY (widget), value.c_str ());

  if (advanced) {

    gtk_table_attach (GTK_TABLE (advanced_fields), label,
		      0, 1, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (advanced_fields), widget,
		      1, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  } else {

    gtk_table_attach (GTK_TABLE (fields), label,
		      0, 1, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (fields), widget,
		      1, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  }

  submitter = new TextSubmitter (name, description, advanced, widget);
  submitters.push_back (submitter);
}


void
FormDialog::private_text (const std::string name,
			  const std::string description,
			  const std::string value,
			  bool advanced)
{
  GtkWidget *label = NULL;
  GtkWidget *widget = NULL;
  PrivateTextSubmitter *submitter = NULL;

  gchar *label_text = NULL;

  grow_fields (advanced);

  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", description.c_str());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), label_text);
  g_free (label_text);

  widget = gtk_entry_new ();
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), widget);
  gtk_entry_set_activates_default (GTK_ENTRY (widget), true);
  gtk_entry_set_visibility (GTK_ENTRY (widget), FALSE);
  gtk_size_group_add_widget (options_group, widget);
  gtk_entry_set_text (GTK_ENTRY (widget), value.c_str ());

  if (advanced) {

    gtk_table_attach (GTK_TABLE (advanced_fields), label,
		      0, 1, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (advanced_fields), widget,
		      1, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  } else {

    gtk_table_attach (GTK_TABLE (fields), label,
		      0, 1, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (fields), widget,
		      1, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  }

  submitter = new PrivateTextSubmitter (name, description, advanced, widget);
  submitters.push_back (submitter);
}


void
FormDialog::multi_text (const std::string name,
			const std::string description,
			const std::string value,
			bool advanced)
{
  GtkWidget *label = NULL;
  GtkWidget *scroller = NULL;
  GtkWidget *widget = NULL;
  GtkTextBuffer *buffer = NULL;
  MultiTextSubmitter *submitter = NULL;

  grow_fields (advanced);

  label = gtk_label_new_with_mnemonic (description.c_str ());
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);
  if (advanced) {

    gtk_table_attach_defaults (GTK_TABLE (advanced_fields), label,
			       0, 2, advanced_rows -1, advanced_rows);
  } else {

    gtk_table_attach_defaults (GTK_TABLE (fields), label,
			       0, 2, rows -1, rows);
  }

  grow_fields (advanced);

  widget = gtk_text_view_new ();
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (widget));
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (widget), GTK_WRAP_WORD);
  gtk_text_buffer_set_text (buffer, value.c_str (), -1);
  scroller = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroller),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scroller), widget);

  if (advanced) {

    gtk_table_attach_defaults (GTK_TABLE (advanced_fields), scroller,
			       0, 2, advanced_rows -1, advanced_rows);
  } else {

    gtk_table_attach_defaults (GTK_TABLE (fields), scroller,
			       0, 2, rows -1, rows);
  }

  submitter = new MultiTextSubmitter (name, description, advanced, buffer);
  submitters.push_back (submitter);
}


void
FormDialog::single_choice (const std::string name,
			   const std::string description,
			   const std::string value,
			   const std::map<std::string, std::string> choices,
			   bool advanced)
{
  GtkWidget *label = NULL;
  gchar* label_text = NULL;
  GtkListStore *model = NULL;
  GtkWidget *widget = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeIter iter;
  SingleChoiceSubmitter *submitter = NULL;

  grow_fields (advanced);

  label = gtk_label_new (NULL);
  gtk_size_group_add_widget (labels_group, label);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", description.c_str ());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), label_text);
  g_free (label_text);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_label_set_line_wrap_mode (GTK_LABEL (label), PANGO_WRAP_WORD);

  model = gtk_list_store_new (SingleChoiceSubmitter::COLUMN_NUMBER,
			      G_TYPE_STRING, G_TYPE_STRING);
  widget = gtk_combo_box_new_with_model (GTK_TREE_MODEL (model));
  renderer = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (widget), renderer, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (widget), renderer,
				  "text", SingleChoiceSubmitter::COLUMN_NAME,
                                  NULL);
  for (std::map<std::string, std::string>::const_iterator map_iter
	 = choices.begin ();
       map_iter != choices.end ();
       map_iter++) {

    gtk_list_store_append (model, &iter);
    gtk_list_store_set (model, &iter,
			SingleChoiceSubmitter::COLUMN_VALUE, map_iter->first.c_str (),
			SingleChoiceSubmitter::COLUMN_NAME, map_iter->second.c_str (),
			-1);
    if (map_iter->first == value)
      gtk_combo_box_set_active_iter (GTK_COMBO_BOX (widget), &iter);
  }

  if (advanced) {

    gtk_table_attach (GTK_TABLE (advanced_fields), label,
		      0, 1, advanced_rows -1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0,0);
    gtk_table_attach (GTK_TABLE (advanced_fields), widget,
		      1, 2, advanced_rows -1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0,0);
  } else {

    gtk_table_attach (GTK_TABLE (fields), label,
		      0, 1, rows -1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0,0);
    gtk_table_attach (GTK_TABLE (fields), widget,
		      1, 2, rows -1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0,0);
  }

  submitter = new SingleChoiceSubmitter (name, description, choices,
					 advanced, widget);
  submitters.push_back (submitter);
}


void
FormDialog::multiple_choice (const std::string name,
			     const std::string description,
			     const std::set<std::string> values,
			     const std::map<std::string, std::string> choices,
			     bool advanced)
{
  GtkWidget *label = NULL;
  GtkWidget *scroll = NULL;
  GtkWidget *tree_view = NULL;
  GtkWidget *frame = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeIter iter;

  gchar *label_text = NULL;

  MultipleChoiceSubmitter *submitter = NULL;

  grow_fields (advanced);

  /* The label */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", description.c_str());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), label_text);
  g_free (label_text);

  /* The GtkListStore containing the choices */
  tree_view = gtk_tree_view_new ();
  list_store = gtk_list_store_new (MultipleChoiceSubmitter::COLUMN_NUMBER,
                                   G_TYPE_BOOLEAN, G_TYPE_STRING);
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 125);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);
  gtk_container_add (GTK_CONTAINER (scroll), tree_view);

  renderer = gtk_cell_renderer_toggle_new ();
  column =
    gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                              "active", MultipleChoiceSubmitter::COLUMN_ACTIVE,
                                              NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (multiple_choice_choice_toggled_cb), list_store);

  renderer = gtk_cell_renderer_text_new ();
  column =
    gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                              "text", MultipleChoiceSubmitter::COLUMN_NAME,
                                              NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);


  for (std::map<std::string, std::string>::const_iterator map_iter
	 = choices.begin ();
       map_iter != choices.end ();
       map_iter++) {

    bool active = (std::find (values.begin (), values.end (), map_iter->first) != values.end ());

    gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
                        MultipleChoiceSubmitter::COLUMN_ACTIVE, active,
                        MultipleChoiceSubmitter::COLUMN_NAME, map_iter->second.c_str (),
                        -1);
  }

  if (advanced) {

    gtk_table_attach (GTK_TABLE (advanced_fields), label,
		      0, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (advanced_fields), frame,
		      0, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  } else {

    gtk_table_attach (GTK_TABLE (fields), label,
		      0, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (fields), frame,
		      0, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  }

  submitter = new MultipleChoiceSubmitter (name, description,
					   choices, advanced, tree_view);
  submitters.push_back (submitter);
}

void
FormDialog::editable_set (const std::string name,
			  const std::string description,
			  const std::set<std::string> values,
			  const std::set<std::string> proposed_values,
			  bool advanced)
{
  GtkWidget *label = NULL;
  GtkWidget *scroll = NULL;
  GtkWidget *button = NULL;
  GtkWidget *tree_view = NULL;
  GtkWidget *frame = NULL;
  GtkWidget *hbox = NULL;
  GtkWidget *entry = NULL;

  GtkListStore *list_store = NULL;
  GtkTreeViewColumn *column = NULL;
  GtkCellRenderer *renderer = NULL;
  GtkTreeIter iter;

  gchar *label_text = NULL;

  EditableSetSubmitter *submitter = NULL;

  grow_fields (advanced);

  /* The label */
  label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  label_text = g_strdup_printf ("<b>%s</b>", description.c_str());
  gtk_label_set_markup_with_mnemonic (GTK_LABEL (label), label_text);
  g_free (label_text);

  /* The GtkListStore containing the values */
  list_store = gtk_list_store_new (EditableSetSubmitter::COLUMN_NUMBER,
				   G_TYPE_BOOLEAN, G_TYPE_STRING);
  tree_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (list_store));
  gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (tree_view), TRUE);
  gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (tree_view), FALSE);

  frame = gtk_frame_new (NULL);
  gtk_widget_set_size_request (GTK_WIDGET (frame), -1, 125);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 0);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (frame), scroll);
  gtk_container_add (GTK_CONTAINER (scroll), tree_view);

  renderer = gtk_cell_renderer_toggle_new ();
  column =
    gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                              "active", EditableSetSubmitter::COLUMN_ACTIVE,
                                              NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);
  g_signal_connect (G_OBJECT (renderer), "toggled",
                    G_CALLBACK (editable_set_choice_toggled_cb), list_store);

  renderer = gtk_cell_renderer_text_new ();
  column =
    gtk_tree_view_column_new_with_attributes (NULL, renderer,
                                              "text", EditableSetSubmitter::COLUMN_VALUE,
                                              NULL);
  gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), column);

  for (std::set<std::string>::const_iterator set_iter = values.begin ();
       set_iter != values.end ();
       set_iter++) {

    gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
    gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
			EditableSetSubmitter::COLUMN_ACTIVE, TRUE,
                        EditableSetSubmitter::COLUMN_VALUE, set_iter->c_str (),
                        -1);
  }
  for (std::set<std::string>::const_iterator set_iter
	 = proposed_values.begin ();
       set_iter != proposed_values.end ();
       set_iter++) {

    if (values.find (*set_iter) == values.end ()) {

      gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
      gtk_list_store_set (GTK_LIST_STORE (list_store), &iter,
			  EditableSetSubmitter::COLUMN_ACTIVE, FALSE,
			  EditableSetSubmitter::COLUMN_VALUE, set_iter->c_str (),
			  -1);
    }
  }

  if (advanced) {

    gtk_table_attach (GTK_TABLE (advanced_fields), label,
		      0, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (advanced_fields), frame,
		      0, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  } else {

    gtk_table_attach (GTK_TABLE (fields), label,
		      0, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
    gtk_table_attach (GTK_TABLE (fields), frame,
		      0, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  }

  hbox = gtk_hbox_new (FALSE, 2);
  entry = gtk_entry_new ();
  button = gtk_button_new_from_stock (GTK_STOCK_ADD);
  gtk_box_pack_start (GTK_BOX (hbox), entry, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 2);

  g_signal_connect (G_OBJECT (entry), "activate",
		    (GCallback) editable_set_add_value_activated_cb,
		    (gpointer) tree_view);

  g_signal_connect (G_OBJECT (button), "clicked",
		    (GCallback) editable_set_add_value_clicked_cb,
		    (gpointer) entry);


  grow_fields (advanced);

  if (advanced) {

    gtk_table_attach (GTK_TABLE (advanced_fields), hbox,
		      0, 2, advanced_rows - 1, advanced_rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  } else {

    gtk_table_attach (GTK_TABLE (fields), hbox,
		      0, 2, rows - 1, rows,
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      (GtkAttachOptions) (GTK_FILL|GTK_EXPAND),
		      0, 0);
  }

  submitter = new EditableSetSubmitter (name, description, advanced, tree_view);
  submitters.push_back (submitter);
}

void
FormDialog::submit ()
{
  Ekiga::FormBuilder builder;

  gtk_widget_hide_all (GTK_WIDGET (window));

  for (std::list<Submitter *>::iterator iter = submitters.begin ();
       iter != submitters.end ();
       iter++)
    (*iter)->submit (builder);

  request.submit (builder);
}

void
FormDialog::cancel ()
{
  gtk_widget_hide_all (GTK_WIDGET (window));
  request.cancel ();
}

void
FormDialog::grow_fields (bool advanced)
{
  if (advanced) {

    advanced_rows++;
    gtk_table_resize (GTK_TABLE (advanced_fields), advanced_rows, 2);
  } else {

    rows++;
    gtk_table_resize (GTK_TABLE (fields), rows, 2);
  }
}
