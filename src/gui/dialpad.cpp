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
 *                         dialpad.cpp  -  description
 *                         ---------------------------
 *   begin                : Thu Jan 3 2008
 *   copyright            : (C) 2008 by Steve Frécinaux
 *   description          : Dial pad widget.
 */

#include "dialpad.h"

#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>

#include <cstring>
#include <iostream>

struct const_key_info
{
  const char *number;
  const char *letters;
  const unsigned code;
};

/* Translators: the following strings are letters that stand on mobile phone
 * keys.
 */
static const struct const_key_info keys_info[] = {
  { "1", "", GDK_KP_1 },
  { "2", N_("abc"), GDK_KP_2 },
  { "3", N_("def"), GDK_KP_3 },
  { "4", N_("ghi"), GDK_KP_4 },
  { "5", N_("jkl"), GDK_KP_5 },
  { "6", N_("mno"), GDK_KP_6 },
  { "7", N_("pqrs"), GDK_KP_7 },
  { "8", N_("tuv"), GDK_KP_8 },
  { "9", N_("wxyz"), GDK_KP_9 },
  { "*", "", GDK_KP_Multiply },
  { "0", "", GDK_KP_0 },
  { "#", "", GDK_numbersign }
};

struct _EkigaDialpadPrivate
{
  GtkAccelGroup *accel_group;
  GtkWidget *buttons[G_N_ELEMENTS (keys_info)];
};

enum
{
  BUTTON_CLICKED,
  LAST_SIGNAL
};

static guint ekiga_dialpad_signals[LAST_SIGNAL];

enum
{
  PROP_0,
  PROP_ACCEL_GROUP
};

G_DEFINE_TYPE (EkigaDialpad, ekiga_dialpad, GTK_TYPE_TABLE);

static void
ekiga_dialpad_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  EkigaDialpad *dialpad = EKIGA_DIALPAD (object);

  switch (prop_id) {
    case PROP_ACCEL_GROUP:
      g_value_set_object (value, dialpad->priv->accel_group);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
ekiga_dialpad_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  EkigaDialpad *dialpad = EKIGA_DIALPAD (object);

  switch (prop_id) {
    case PROP_ACCEL_GROUP:
      dialpad->priv->accel_group = GTK_ACCEL_GROUP (g_value_get_object (value));
      g_object_ref (dialpad->priv->accel_group);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
on_dialpad_button_clicked (GtkButton    *button,
                           EkigaDialpad *dialpad)
{
  unsigned i;

  for (i = 0; i < G_N_ELEMENTS (keys_info); i++) {
    if (GTK_WIDGET (button) == dialpad->priv->buttons[i]) {
      g_signal_emit (dialpad, ekiga_dialpad_signals[BUTTON_CLICKED], 0,
                     keys_info[i].number);
      return;
    }
  }

  g_return_if_reached ();
}

static void
ekiga_dialpad_init (EkigaDialpad *dialpad)
{
  unsigned i;

  dialpad->priv = G_TYPE_INSTANCE_GET_PRIVATE (dialpad,
                                               EKIGA_TYPE_DIALPAD,
                                               EkigaDialpadPrivate);

  gtk_table_set_col_spacings (GTK_TABLE (dialpad), 2);
  gtk_table_set_row_spacings (GTK_TABLE (dialpad), 2);
  gtk_table_set_homogeneous (GTK_TABLE (dialpad), true);

  /* Create the buttons */
  for (i = 0; i < G_N_ELEMENTS (keys_info); i++) {
    GtkWidget *box;
    GtkWidget *label;
    GtkWidget *button;
    gchar *text;
    GtkWidget *alignment;

    box = gtk_hbox_new (FALSE, 2);

    label = gtk_label_new (keys_info[i].number);
    gtk_misc_set_alignment (GTK_MISC (label), 1.0, 1.0);
    gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);

    label = gtk_label_new (NULL);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 1.0);

    if (strlen (keys_info [i].letters) > 0) {
      text = g_strdup_printf ("<sub><span size=\"small\">%s</span></sub>",
                              _(keys_info [i].letters));
      gtk_label_set_markup (GTK_LABEL (label), text);
      g_free (text);
    }
    gtk_box_pack_start (GTK_BOX (box), label, FALSE, TRUE, 0);

    alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
    gtk_container_add (GTK_CONTAINER (alignment), box);

    button = gtk_button_new ();
    gtk_container_set_border_width (GTK_CONTAINER (button), 0);
    gtk_container_add (GTK_CONTAINER (button), alignment);

    dialpad->priv->buttons[i] = button;

    gtk_table_attach (GTK_TABLE (dialpad),  button,
                      i % 3, i % 3 + 1,
                      i / 3, i / 3 + 1,
                      (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                      (GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                      0, 0);

    g_signal_connect (button, "clicked",
                      G_CALLBACK (on_dialpad_button_clicked), dialpad);
  }
}

static GObject *
ekiga_dialpad_constructor (GType                  type,
                           guint                  n_construct_properties,
                           GObjectConstructParam *construct_properties)
{
  GObjectClass *parent_class;
  EkigaDialpad *dialpad;

  /* Invoke parent constructor. */
  parent_class = G_OBJECT_CLASS (ekiga_dialpad_parent_class);
  dialpad = EKIGA_DIALPAD (parent_class->constructor (type,
                                                      n_construct_properties,
                                                      construct_properties));

  /* The construct properties have been set -> we can use them (similar to
   * object_class->construcor which was not available in gtk 2.10) */
  if (dialpad->priv->accel_group != NULL) {
    unsigned i;
    for (i = 0; i < G_N_ELEMENTS (keys_info); i++) {
      gtk_widget_add_accelerator (dialpad->priv->buttons[i],
                                  "clicked",
                                  dialpad->priv->accel_group,
                                  keys_info[i].code,
                                  (GdkModifierType) 0, (GtkAccelFlags) 0);
    }
  }

  return G_OBJECT (dialpad);
}

static void
ekiga_dialpad_finalize (GObject *object)
{
  EkigaDialpad *dialpad = EKIGA_DIALPAD (object);

  if (dialpad->priv->accel_group != NULL)
    g_object_unref (dialpad->priv->accel_group);

  G_OBJECT_CLASS (ekiga_dialpad_parent_class)->finalize (object);
}

static void
ekiga_dialpad_class_init (EkigaDialpadClass *klass)
{
  GType the_type = G_TYPE_FROM_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructor = ekiga_dialpad_constructor;
  object_class->finalize = ekiga_dialpad_finalize;
  object_class->get_property = ekiga_dialpad_get_property;
  object_class->set_property = ekiga_dialpad_set_property;

  g_object_class_install_property (object_class,
                                   PROP_ACCEL_GROUP,
                                   g_param_spec_object ("accel-group",
                                                        "Accel group",
                                                        "Accel group",
                                                        GTK_TYPE_ACCEL_GROUP,
                                                        (GParamFlags) (G_PARAM_READWRITE |
                                                                       G_PARAM_CONSTRUCT_ONLY |
                                                                       G_PARAM_STATIC_STRINGS)));

  ekiga_dialpad_signals[BUTTON_CLICKED] =
            g_signal_new ("button-clicked", the_type,
                          G_SIGNAL_RUN_LAST,
                          G_STRUCT_OFFSET (EkigaDialpadClass, button_clicked),
                          NULL, NULL,
                          g_cclosure_marshal_VOID__STRING,
                          G_TYPE_NONE,
                          1,
                          G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE);

  g_type_class_add_private (klass, sizeof (EkigaDialpadPrivate));
}

guint
ekiga_dialpad_get_button_code (EkigaDialpad * /* dialpad */,
                               char          number)
{
  unsigned i;
  for (i = 0; i < G_N_ELEMENTS (keys_info); i++)
    if (keys_info[i].number[0] == number)
      return keys_info[i].code;
  return 0;
}

GtkWidget *ekiga_dialpad_new (GtkAccelGroup *accel_group)
{
  gpointer obj;

  if (accel_group == NULL)
    obj = g_object_new (EKIGA_TYPE_DIALPAD, NULL);
  else
    obj = g_object_new (EKIGA_TYPE_DIALPAD, "accel-group", accel_group, NULL);

  return GTK_WIDGET (obj);
}

/* ex:set ts=2 sw=2 et: */
