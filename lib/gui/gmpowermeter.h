
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
 *                         gmpowermeter.h  -  description
 *                         --------------------------------
 *   begin                : Tue Sep 5 2006
 *   copyright            : (C) 2006 by Jan Schampera
 *   description          : header file for a GTK widget that shows different images
 *                          depending on a given float 0.0 .. 1.0 (for example power-bars)
 *   license              : GPL
 *   idea                 : Damien Sandras
 *
 */

/*!\file gmpowermeter.h
 * \brief Header file for the GmPowermeter widget
 * \author Jan Schampera <jan.schampera@unix.net>
 * \version 0.1 pre-ALPHA
 * \date 2006
 */

#ifndef __GM_POWERMETER_H__
#define __GM_POWERMETER_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GM_POWERMETER_TYPE              (gm_powermeter_get_type())
#define GM_POWERMETER(obj)              (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_POWERMETER_TYPE, GmPowermeter))
#define GM_POWERMETER_CLASS(klass)      ((G_TYPE_CHECK_CLASS_CAST ((klass), GM_POWERMETER_TYPE, GmPowermeterClass)))
#define GM_IS_POWERMETER(obj)           (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_POWERMETER_TYPE))
#define GM_IS_POWERMETER_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GM_POWERMETER_TYPE))


typedef struct {
  guint max_index;
  GdkPixbuf** iconv;
} GmPowermeterIconset;


/*!\typedef GmPowermeter
 * \brief type for the _GmPowermeter instance structure
 * \see _GmPowermeter
 */
typedef struct _GmPowermeter GmPowermeter;

/*!\typedef GmPowermeterClass
 * \brief type for the _GmPowermeterClass class structure
 * \see _GmPowermeterClass
 */
typedef struct _GmPowermeterClass GmPowermeterClass;


/*!\struct _GmPowermeter
 * \brief _GmPowermeter instance structure
 * \see GmPowermeter
 */
struct _GmPowermeter
{
  GtkImage actualimage;
  /*!< parent widget, a GtkImage */

  GmPowermeterIconset *iconset;
  /*!< used icons to draw the level, in a NULL terminated vector */

  gfloat level;
  /*!< the level to display, a float between 0.0 and 1.0 */
};


/*!\struct _GmPowermeterClass
 * \brief _GmPowermeter class structure
 * \see GmPowermeterClass
 */
struct _GmPowermeterClass
{
  GtkImageClass parent_class;
  /*!< parent class, a GtkImageClass */
};


/*!\fn gm_powermeter_get_type (void)
 * \brief retrieve the GType of the GmPowermeter
 *
 * Usually used by the macros or by GLib/GTK itself
 * \see GM_POWERMETER_TYPE()
 * \see GM_POWERMETER()
 * \see GM_POWERMETER_CLASS()
 * \see GM_IS_POWERMETER()
 * \see GM_IS_POWERMETER_CLASS()
 */
GType                           gm_powermeter_get_type (void);

/*!\fn gm_powermeter_new (void)
 * \brief return a new instance of a GmPowermeter, with default icon set
 * \see GmPowermeter
 * \see _GmPowermeter
 */
GtkWidget*                      gm_powermeter_new (void);

/*!\fn gm_powermeter_new_with_icon_set (GdkPixbuf**)
 * \brief return a new instance of a GmPowermeter, with a given icon set
 *
 * The GmPowermeter does NOT make a private copy of the vector and its data.
 * It MUST NOT be freed while the stuff is running.
 * \see GmPowermeter
 * \see _GmPowermeter
 */
GtkWidget*                      gm_powermeter_new_with_icon_set (GmPowermeterIconset*);

/*!\fn gm_powermeter_set_level (gfloat)
 * \brief sets the level to display
 *
 * GmPowermeter instance will calculate the proper icon index for the given level
 * \param the level given as float between 0.0 and 1.0
 * \see gm_powermeter_get_level()
 */
void                            gm_powermeter_set_level (GmPowermeter*,
							 gfloat);

/*!\fn gm_powermeter_get_level (void)
 * \brief returns the actual used level
 * \see gm_powermeter_set_level()
 */
gfloat                          gm_powermeter_get_level (GmPowermeter*);

G_END_DECLS

#endif /* __GM_POWERMETER_H__ */

