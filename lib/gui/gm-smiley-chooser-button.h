
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
 *                         gm-smiley-chooser-button.h  -  description
 *                         ------------------------------------------
 *   begin                : August 2008 by Jan Schampera
 *   copyright            : (C) 2008 by Jan Schampera
 *   description          : Declaration of a popup window to choose a smiley
 *
 */

/*
 * Thanks to (alphabetical):
 * Alphonso, Fabrice
 * Defais, Yannick
 * Puydt, Julien
 * Sandras, Damien
 */

#ifndef __GM_SMILEY_CHOOSER_BUTTON_H__
#define __GM_SMILEY_CHOOSER_BUTTON_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GmSmileyChooserButton GmSmileyChooserButton;
typedef struct _GmSmileyChooserButtonPrivate GmSmileyChooserButtonPrivate;
typedef struct _GmSmileyChooserButtonClass GmSmileyChooserButtonClass;

struct _GmSmileyChooserButton {
  GtkToggleButton parent;

  GmSmileyChooserButtonPrivate* priv;
};

struct _GmSmileyChooserButtonClass {
  GtkToggleButtonClass parent;

  /* the "smiley_selected" signal */
  void (*smiley_selected) (GmSmileyChooserButton* self,
			   gpointer characters);
};

#define GM_SMILEY_CHOOSER_BUTTON_TYPE            \
  (gm_smiley_chooser_button_get_type ())
#define GM_SMILEY_CHOOSER_BUTTON(obj)            \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_SMILEY_CHOOSER_BUTTON_TYPE, GmSmileyChooserButton))
#define GM_IS_SMILEY_CHOOSER_BUTTON(obj)         \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_SMILEY_CHOOSER_BUTTON_TYPE))
#define GM_SMILEY_CHOOSER_BUTTON_CLASS(klass)    \
  (G_TYPE_CHECK_CLASS_CAST ((klass), GM_SMILEY_CHOOSER_BUTTON_TYPE, GmSmileyChooserButtonClass))
#define GM_IS_SMILEY_CHOOSER_BUTTON_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), GM_SMILEY_CHOOSER_BUTTON_TYPE))
#define GM_SMILEY_CHOOSER_BUTTON_GET_CLASS(obj)  \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_SMILEY_CHOOSER_BUTTON_TYPE, GmSmileyChooserButtonClass))


/**
 * The GTyping function for GmSmileyChooserButton
 * @return The GType of the GmSmileyChooserButton
 */
GType gm_smiley_chooser_button_get_type (void);


/**
 * Create a new GmSmileyChooserButton
 * @return a new GmSmileyChooserButton
 */
GtkWidget* gm_smiley_chooser_button_new (void);

G_END_DECLS

#endif /* __GM_SMILEY_CHOOSER_BUTTON_H__ */

