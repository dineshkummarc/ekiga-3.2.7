
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
 *                         gtklevelmeter.h  -  description
 *                         -------------------------------
 *   begin                : Sat Dec 23 2003
 *   copyright            : (C) 2003 by Stefan Brüns <lurch@gmx.li>
 *   description          : This file contains a GTK VU Meter.
 *
 */


#ifndef __GM_LEVEL_METER_H__
#define __GM_LEVEL_METER_H__

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <stdlib.h>


G_BEGIN_DECLS

#define GM_TYPE_LEVEL_METER         (gm_level_meter_get_type ())
#define GM_LEVEL_METER(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), GM_TYPE_LEVEL_METER, GmLevelMeter))
#define GM_LEVEL_METER_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), GM_TYPE_LEVEL_METER, GmLevelMeterClass))
#define GM_IS_LEVEL_METER(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), GM_TYPE_LEVEL_METER))
#define GM_IS_LEVEL_METER_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), GM_TYPE_LEVEL_METER))
#define GM_LEVEL_METER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), GM_TYPE_LEVEL_METER, GmLevelMeterClass))

typedef struct _GmLevelMeter GmLevelMeter;
typedef struct _GmLevelMeterClass GmLevelMeterClass;


struct _GmLevelMeter
{
  GtkWidget widget;

  /* Orientation of the level meter */
  GtkOrientation orientation;

  /* show a peak indicator */
  gboolean showPeak;

  /* show a continous or a segmented (LED like) display */
  gboolean isSegmented;

  /* The ranges of different color of the display */
  GArray* colorEntries;

  /* The pixmap for double buffering */
  GdkPixmap* offscreen_image;

  /* The pixmap with the highlighted bar */
  GdkPixmap* offscreen_image_hl;

  /* The pixmap with the dark bar */
  GdkPixmap* offscreen_image_dark;

  /* The levels */
  gfloat level, peak;
};


struct _GmLevelMeterClass
{
  GtkWidgetClass parent_class;
};


typedef struct _GmLevelMeterColorEntry GmLevelMeterColorEntry;

struct _GmLevelMeterColorEntry
{
  GdkColor color;
  gfloat stopvalue;
  GdkColor darkcolor;
};


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a new VU meter
 * PRE          :  /
 */
GtkWidget *gm_level_meter_new (void);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Get the GType 
 * PRE          :  /
 */
GType gm_level_meter_get_type (void);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set new values for level.
 * PRE          :  Level should be between 0.0 and 1.0,
 *                 lower/higher values are clamped.
 */
void gm_level_meter_set_level (GmLevelMeter *meter, 
                               gfloat level);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Clear the GtkLevelMeter.
 * PRE          :  /
 */
void gm_level_meter_clear (GmLevelMeter *meter);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Set new colors for the different ranges of the meter
 * PRE          :  Each array entry has to be a GtkLevelMeterColorEntry,
 *                 the number of entries is not limited, each range starts
 *                 at the stopvalue of the previous entry (or 0.0 for the
 *                 first), color allocation is done by the widget. A copy
 *                 of the array is stored, so the array given as an argument
 *                 can be deleted after the function call.
 */
void gm_levelmeter_set_colors (GmLevelMeter *meter,
                               GArray *colors);

G_END_DECLS

#endif /* __GM_LEVEL_METER_H__ */
