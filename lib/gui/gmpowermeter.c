
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


#include <gtk/gtk.h>
#include <math.h>

#include "gmpowermeter.h"

#include "pixmaps/gm_powermeter_default_00.xpm"
#include "pixmaps/gm_powermeter_default_01.xpm"
#include "pixmaps/gm_powermeter_default_02.xpm"
#include "pixmaps/gm_powermeter_default_03.xpm"
#include "pixmaps/gm_powermeter_default_04.xpm"

static GObjectClass *parent_class = NULL;

static void gm_powermeter_dispose (GObject *obj);

static void gm_powermeter_finalize (GObject *obj);

static void gm_powermeter_class_init (gpointer g_class,
				      gpointer class_data);

static void gm_powermeter_init (GmPowermeter*);

static guint gm_powermeter_get_index_by_level (guint,
					       gfloat);

void gm_powermeter_redraw (GmPowermeter*);


/* Implementation */

static void
gm_powermeter_dispose (GObject *obj)
{
  if (((GmPowermeter*)obj)->iconset->iconv[0])
    g_object_unref (((GmPowermeter*)obj)->iconset->iconv[0]);
  ((GmPowermeter*)obj)->iconset->iconv[0] = NULL;
  if (((GmPowermeter*)obj)->iconset->iconv[1])
    g_object_unref (((GmPowermeter*)obj)->iconset->iconv[1]);
  ((GmPowermeter*)obj)->iconset->iconv[1] = NULL;
  if (((GmPowermeter*)obj)->iconset->iconv[2])
    g_object_unref (((GmPowermeter*)obj)->iconset->iconv[2]);
  ((GmPowermeter*)obj)->iconset->iconv[2] = NULL;
  if (((GmPowermeter*)obj)->iconset->iconv[3])
    g_object_unref (((GmPowermeter*)obj)->iconset->iconv[3]);
  ((GmPowermeter*)obj)->iconset->iconv[3] = NULL;
  if (((GmPowermeter*)obj)->iconset->iconv[4])
    g_object_unref (((GmPowermeter*)obj)->iconset->iconv[4]);
  ((GmPowermeter*)obj)->iconset->iconv[4] = NULL;

  parent_class->dispose (obj);
}

static void
gm_powermeter_finalize (GObject *obj)
{
  g_free (((GmPowermeter*)obj)->iconset->iconv);
  g_free (((GmPowermeter*)obj)->iconset);

  parent_class->finalize (obj);
}

GType
gm_powermeter_get_type (void)
{
  static GType gm_powermeter_type = 0;

  if (!gm_powermeter_type)
    {
      static const GTypeInfo gm_powermeter_info =
	{
	  sizeof (GmPowermeterClass),
	  NULL,
	  NULL,
	  (GClassInitFunc) gm_powermeter_class_init,
	  NULL,
	  NULL,
	  sizeof (GmPowermeter),
	  0,
	  (GInstanceInitFunc) gm_powermeter_init,
	  NULL
	};
      gm_powermeter_type =
	g_type_register_static (GTK_TYPE_IMAGE,
				"GmPowermeter",
				&gm_powermeter_info, (GTypeFlags) 0);
    }
  return gm_powermeter_type;
}


static void
gm_powermeter_class_init (gpointer g_class,
			  G_GNUC_UNUSED gpointer class_data)
{
  GObjectClass *gobject_class = NULL;

  parent_class = (GObjectClass *) g_type_class_peek_parent (g_class);

  gobject_class = (GObjectClass*)g_class;
  gobject_class->dispose = gm_powermeter_dispose;
  gobject_class->finalize = gm_powermeter_finalize;
}


static void
gm_powermeter_init (GmPowermeter* powermeter)
{
  /* adjust that when you change the number of pictures for the default set! */
  const int num_pics = 5;
  char ** tmp_xmp = { NULL };

  g_return_if_fail (powermeter != NULL);
  g_return_if_fail (GM_IS_POWERMETER (powermeter));

  powermeter->level = 0.0;

  /* set the default icon set FIXME isn't that ugly? */
  powermeter->iconset = (GmPowermeterIconset*) g_malloc (sizeof (GmPowermeterIconset));
  powermeter->iconset->max_index = num_pics - 1;

  /* allocate the vector table (plus 1 for NULL) */
  powermeter->iconset->iconv = (GdkPixbuf**) g_malloc (sizeof (GdkPixbuf*) * (num_pics + 1));

  /* populate the vector table and append NULL */
  /* append/remove lines when you change the number of
   * pictures for the default set! (and free them in dispose!)
   * FIXME FIXME
   * the way round char** tmp_xmp for temporary assignment is needed because
   * there seems to be no way to directly do
   *   foo = gdk_pixbuf_new_from_xpm_data ((const char **) xpm_data);
   * without compiler warnings!
   */
  tmp_xmp = (char **) gm_powermeter_default_00_xpm;
  powermeter->iconset->iconv[0] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);
  tmp_xmp = (char **) gm_powermeter_default_01_xpm;
  powermeter->iconset->iconv[1] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);
  tmp_xmp = (char **) gm_powermeter_default_02_xpm;
  powermeter->iconset->iconv[2] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);
  tmp_xmp = (char **) gm_powermeter_default_03_xpm;
  powermeter->iconset->iconv[3] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);
  tmp_xmp = (char **) gm_powermeter_default_04_xpm;
  powermeter->iconset->iconv[4] = gdk_pixbuf_new_from_xpm_data ((const char**) tmp_xmp);
  powermeter->iconset->iconv[num_pics] = NULL;

  gm_powermeter_redraw (powermeter);
}


GtkWidget*
gm_powermeter_new (void)
{
  return GTK_WIDGET (g_object_new (gm_powermeter_get_type (), NULL));
}


GtkWidget*
gm_powermeter_new_with_icon_set (G_GNUC_UNUSED GmPowermeterIconset* iconset)
{
  /* FIXME */
  return gm_powermeter_new ();
}


guint
gm_powermeter_get_index_by_level (guint maxindex,
				  gfloat level)
{
  /* FIXME? */
  gfloat stepvalue = 0.0;
  gfloat stepnumber = 0.0;

  if (level <= 0.0)
    return 0;
  if (level >= 1.0)
    return maxindex;

  stepvalue = 1.0 / maxindex;
  stepnumber = level / stepvalue;

  return (guint) rintf ((float) stepnumber);
}


void
gm_powermeter_redraw (GmPowermeter* powermeter)
{
  guint calculated_index = 0;

  g_return_if_fail (powermeter != NULL);
  g_return_if_fail (GM_IS_POWERMETER (powermeter));

  calculated_index =
    gm_powermeter_get_index_by_level (powermeter->iconset->max_index,
				      powermeter->level);

  gtk_image_set_from_pixbuf (GTK_IMAGE (powermeter),
			     powermeter->iconset->iconv [calculated_index]);
}


void
gm_powermeter_set_level (GmPowermeter* powermeter,
			 gfloat level)
{
  g_return_if_fail (powermeter != NULL);
  g_return_if_fail (GM_IS_POWERMETER (powermeter));

  /* don't bother if we're requested to display the same
   * level we already do */
  if (fabs (level - powermeter->level) <= 0.0001)
    return;

  powermeter->level = level;

  /* limit the level to values between 0 and 1, inclusive */
  if (powermeter->level < 0.0)
    powermeter->level = 0.0;
  if (powermeter->level > 1.0)
    powermeter->level = 1.0;

  gm_powermeter_redraw (powermeter);
}


gfloat
gm_powermeter_get_level (GmPowermeter* powermeter)
{
  g_return_val_if_fail (powermeter != NULL, 0.0);
  g_return_val_if_fail (GM_IS_POWERMETER (powermeter), 0.0);

  return powermeter->level;
}

