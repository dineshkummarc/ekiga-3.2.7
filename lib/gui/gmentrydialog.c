
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
 *                         gmentrydialog.c  -  description
 *                         -------------------------------
 *   begin                : Sat Jan 03 2004
 *   copyright            : (C) 2000-2006 by Damien Sandras 
 *   description          : Contains a gmentrydialog widget permitting to
 *                          quickly build GtkDialogs with a +rw GtkEntry
 *                          field.
 *
 */


#include "gmentrydialog.h"


/* Static functions and declarations */
static void gm_entry_dialog_class_init (GmEntryDialogClass *);

static void gm_entry_dialog_init (GmEntryDialog *);

static GtkDialogClass *parent_class = NULL;


static void
gm_entry_dialog_class_init (GmEntryDialogClass *klass)
{
  GObjectClass *object_class = NULL;
  GmEntryDialogClass *entry_dialog_class = NULL;

  object_class = G_OBJECT_CLASS (klass);
  parent_class = g_type_class_peek_parent (klass);
  entry_dialog_class = GM_ENTRY_DIALOG_CLASS (klass);
}


static void
gm_entry_dialog_init (GmEntryDialog *ed)
{
  ed->field_entry = gtk_entry_new ();
  gtk_entry_set_activates_default (GTK_ENTRY (ed->field_entry), TRUE);
  
  ed->label = gtk_label_new (NULL);
}


/* Global functions */
GType
gm_entry_dialog_get_type (void)
{
  static GType gm_entry_dialog_type = 0;
  
  if (gm_entry_dialog_type == 0)
  {
    static const GTypeInfo entry_dialog_info =
    {
      sizeof (GmEntryDialogClass),
      NULL,
      NULL,
      (GClassInitFunc) gm_entry_dialog_class_init,
      NULL,
      NULL,
      sizeof (GmEntryDialog),
      0,
      (GInstanceInitFunc) gm_entry_dialog_init,
      NULL
    };
    
    gm_entry_dialog_type =
      g_type_register_static (GTK_TYPE_DIALOG,
			      "GmEntryDialog",
			      &entry_dialog_info,
			      (GTypeFlags) 0);
  }
  
  return gm_entry_dialog_type;
}


GtkWidget *
gm_entry_dialog_new (const char *label,
		     const char *button_label)
{
  GtkWidget *hbox = NULL;
  GmEntryDialog *ed = NULL;
  
  ed = 
    GM_ENTRY_DIALOG (g_object_new (GM_ENTRY_DIALOG_TYPE, NULL));

  if (label)
    gtk_label_set_text (GTK_LABEL (GM_ENTRY_DIALOG (ed)->label), label);

  hbox = gtk_hbox_new (FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), ed->label, FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (hbox), ed->field_entry, FALSE, FALSE, 6);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (ed)->vbox), hbox);

  gtk_dialog_add_buttons (GTK_DIALOG (ed),
			  GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			  button_label, GTK_RESPONSE_ACCEPT, NULL);

  gtk_window_set_modal (GTK_WINDOW (ed), TRUE);
  
  gtk_widget_show_all (hbox);
  
  return GTK_WIDGET (ed);
}


void
gm_entry_dialog_set_text (GmEntryDialog *ed,
			  const char *text)
{
  g_return_if_fail (ed != NULL);
  g_return_if_fail (text != NULL);
  
  gtk_entry_set_text (GTK_ENTRY (ed->field_entry), text);
}


const char *
gm_entry_dialog_get_text (GmEntryDialog *ed)
{
  g_return_val_if_fail (ed != NULL, NULL);

  return gtk_entry_get_text (GTK_ENTRY (ed->field_entry));
}
