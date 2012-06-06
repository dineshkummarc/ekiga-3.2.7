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
 *                         dialpad.h -  description
 *                         ---------------------------
 *   begin                : Thu Jan 3 2008
 *   copyright            : (C) 2008 by Steve Frécinaux
 *   description          : Dial pad widget.
 */

#ifndef __EKIGA_DIALPAD_H__
#define __EKIGA_DIALPAD_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define EKIGA_TYPE_DIALPAD             (ekiga_dialpad_get_type())
#define EKIGA_DIALPAD(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), EKIGA_TYPE_DIALPAD, EkigaDialpad))
#define EKIGA_DIALPAD_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), EKIGA_TYOE_DIALPAD, EkigaDialpadClass))
#define EKIGA_IS_DIALPAD(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), EKIGA_TYPE_DIALPAD))
#define EKIGA_IS_DIALPAD_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), EKIGA_TYPE_DIALPAD))
#define EKIGA_DIALPAD_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), EKIGA_TYPE_DIALPAD, EkigaDialpadClass))

typedef struct _EkigaDialpad        EkigaDialpad;
typedef struct _EkigaDialpadPrivate EkigaDialpadPrivate;
typedef struct _EkigaDialpadClass   EkigaDialpadClass;

struct _EkigaDialpad
{
  GtkTable             parent;
  EkigaDialpadPrivate *priv;
};

struct _EkigaDialpadClass
{
  GtkTableClass parent_class;

  void (* button_clicked) (EkigaDialpad *dialpad, const gchar *button);
};

GType      ekiga_dialpad_get_type         (void) G_GNUC_CONST;
GtkWidget *ekiga_dialpad_new              (GtkAccelGroup *accel_group);

guint      ekiga_dialpad_get_button_code  (EkigaDialpad *dialpad,
                                           char          number);

G_END_DECLS

#endif  /* __EKIGA_DIALPAD_H__ */

/* ex:set ts=2 sw=2 et: */
