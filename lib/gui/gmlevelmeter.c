
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
 *                         gtklevelmeter.c  -  description
 *                         -------------------------------
 *   begin                : Sat Dec 23 2003
 *   copyright            : (C) 2003 by Stefan Brüns <lurch@gmx.li>
 *   description          : This file contains a GTK VU Meter.
 *
 */


#include "gmlevelmeter.h"

G_DEFINE_TYPE (GmLevelMeter, gm_level_meter, GTK_TYPE_WIDGET);

static void gm_level_meter_finalize (GObject *object);
static void gm_level_meter_free_colors (GArray *colors);
static void gm_level_meter_allocate_colors (GArray *colors);
static void gm_level_meter_rebuild_pixmap (GmLevelMeter *lm);
static void gm_level_meter_realize (GtkWidget *widget);
static void gm_level_meter_create_pixmap (GmLevelMeter *lm);
static void gm_level_meter_paint (GmLevelMeter *lm);
static void gm_level_meter_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gm_level_meter_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gboolean gm_level_meter_expose (GtkWidget *widget, GdkEventExpose *event);



static void
gm_level_meter_class_init (GmLevelMeterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->finalize = gm_level_meter_finalize;

  widget_class->size_request = gm_level_meter_size_request;
  widget_class->size_allocate = gm_level_meter_size_allocate;
  widget_class->expose_event = gm_level_meter_expose;
  widget_class->realize = gm_level_meter_realize;
}


static void
gm_level_meter_init (GmLevelMeter *lm)
{
  lm->orientation = GTK_ORIENTATION_HORIZONTAL;
  lm->showPeak = TRUE;
  lm->isSegmented = FALSE;
  lm->colorEntries = NULL;
  lm->offscreen_image = NULL;
  lm->offscreen_image_hl = NULL;
  lm->offscreen_image_dark = NULL;
  lm->level = .0;
  lm->peak = .0;
}


GtkWidget*
gm_level_meter_new ()
{
  return GTK_WIDGET (g_object_new (GM_TYPE_LEVEL_METER, NULL)); 
}


static void
gm_level_meter_set_default_colors (GArray *colors)
{
  GmLevelMeterColorEntry entry = { {0, 0, 65535, 30000}, 0.8, {0, 0, 0, 0}};

  g_array_append_val (colors, entry);
  entry.color.red = 65535;
  entry.stopvalue = .9;
  g_array_append_val (colors, entry);
  entry.color.green = 0;
  entry.stopvalue = 1.0;
  g_array_append_val (colors, entry);
}
 
 
static void
gm_level_meter_finalize (GObject *object)
{
  GmLevelMeter *lm = NULL;

  g_return_if_fail (GM_IS_LEVEL_METER (object));

  lm = GM_LEVEL_METER (object);

  if (lm->colorEntries) {

    gm_level_meter_free_colors (lm->colorEntries);
    g_array_free (lm->colorEntries, TRUE);
    lm->colorEntries = NULL;
  }

  if (lm->offscreen_image) {

    g_object_unref (lm->offscreen_image);
    lm->offscreen_image = NULL;
  }

  if (lm->offscreen_image_hl) {

    g_object_unref (lm->offscreen_image_hl);
    lm->offscreen_image_hl = NULL;
  }

  if (lm->offscreen_image_dark) {

    g_object_unref (lm->offscreen_image_dark);
    lm->offscreen_image_dark = NULL;
  }


  G_OBJECT_CLASS (gm_level_meter_parent_class)->finalize (object);
}


void
gm_level_meter_set_level (GmLevelMeter *lm,
                          gfloat level)
{
  lm->level = level;

  if (level > lm->peak)
    lm->peak = level;

#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_get_realized (GTK_WIDGET (lm)))
#else
  if (GTK_WIDGET_REALIZED (lm))
#endif
    gm_level_meter_paint (lm);
}


void
gm_level_meter_clear (GmLevelMeter *lm)
{
  lm->level = 0;
  lm->peak = 0;

#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_get_realized (GTK_WIDGET (lm)))
#else
  if (GTK_WIDGET_REALIZED (lm))
#endif
    gm_level_meter_paint (lm);
}


void
gm_level_meter_set_colors (GmLevelMeter* lm,
                           GArray *colors)
{
  unsigned i;

  if (lm->colorEntries) {

    /* free old colors, if they have been allocated; delete old array */
#if GTK_CHECK_VERSION(2,20,0)
    if (gtk_widget_get_realized (GTK_WIDGET (lm)))
#else
    if (GTK_WIDGET_REALIZED (lm))
#endif
      gm_level_meter_free_colors (lm->colorEntries);
    g_array_free (lm->colorEntries, TRUE);
  }

  lm->colorEntries =
    g_array_new (FALSE, FALSE, sizeof (GmLevelMeterColorEntry));

  /* copy array */
  for (i = 0 ; i < colors->len ; i++) {
    GmLevelMeterColorEntry* entry =
      &g_array_index (colors, GmLevelMeterColorEntry, i);
    g_array_append_val (lm->colorEntries, *entry);
  }

#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_get_realized (GTK_WIDGET (lm))) {
#else
  if (GTK_WIDGET_REALIZED (lm)) {
#endif

    gm_level_meter_allocate_colors (lm->colorEntries);

    /* recalc */
    gm_level_meter_rebuild_pixmap (lm);
    gm_level_meter_paint (lm);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Frees the colors allocated for the meter
 * PRE          :  The array should be the same as the one previously
 *                 used with gm_level_meter_allocate_colors
 */
static void
gm_level_meter_free_colors (GArray *colors)
{
  GdkColor *light = NULL;
  GdkColor *dark = NULL;

  unsigned i = 0;

  for (i = 0; i < colors->len; i++) {

    light = &(g_array_index (colors, GmLevelMeterColorEntry, i).color);
    dark = &(g_array_index (colors, GmLevelMeterColorEntry, i).darkcolor);
    gdk_colormap_free_colors (gdk_colormap_get_system (), light, 1);
    gdk_colormap_free_colors (gdk_colormap_get_system (), dark, 1);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Allocate the colors according to the entries of the array
 * PRE          :  Only the light color is used, the dark one is set automatically
 */
static void
gm_level_meter_allocate_colors (GArray *colors)
{
  GdkColor *light = NULL;
  GdkColor *dark = NULL;

  unsigned i = 0;

  if (colors->len == 0)
    gm_level_meter_set_default_colors (colors);

  for (i = 0; i < colors->len; i++) {

    light = &(g_array_index (colors, GmLevelMeterColorEntry, i).color);
    dark = &(g_array_index (colors, GmLevelMeterColorEntry, i).darkcolor);
    dark->red = light->red * .4;
    dark->green = light->green * .4; 
    dark->blue = light->blue * .4; 
    gdk_colormap_alloc_color (gdk_colormap_get_system (), light, FALSE, TRUE);
    gdk_colormap_alloc_color (gdk_colormap_get_system (), dark, FALSE, TRUE);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Rebuilds the light and dark base images which are used
 *                 to compose the offscreen image
 * PRE          :  /
 */
static void
gm_level_meter_rebuild_pixmap (GmLevelMeter *lm)
{
  GtkWidget *widget = GTK_WIDGET (lm);
  GdkGC *gc = NULL;
  gint *borders = NULL;
  gint bar_length = 0;
  gint start_x = 0;
  gint start_y = 0;
  gint width_x = 0;
  gint width_y = 0;
  unsigned i = 0;

  gc = gdk_gc_new (lm->offscreen_image);

  borders = (gint *) g_new (int, lm->colorEntries->len + 1);

  gtk_paint_box (widget->style,
                 lm->offscreen_image_dark,
                 GTK_STATE_PRELIGHT, GTK_SHADOW_IN,
                 NULL, widget, "bar",
                 0, 0,
                 widget->allocation.width, widget->allocation.height);

  switch (lm->orientation) {
  case GTK_ORIENTATION_VERTICAL:
    bar_length = widget->allocation.height - 2 * widget->style->ythickness;
    borders[0] = widget->style->ythickness;
    break;
  case GTK_ORIENTATION_HORIZONTAL:
  default:
    bar_length = widget->allocation.width - 2 * widget->style->xthickness;
    borders[0] = widget->style->xthickness;
  }

  for (i = 0 ; i < lm->colorEntries->len ; i++) {

    /* Calculate position of borders */
    borders[i+1] = borders[0] + bar_length *
      g_array_index (lm->colorEntries, GmLevelMeterColorEntry, i).stopvalue;

    switch (lm->orientation) {
    case GTK_ORIENTATION_VERTICAL:
      start_x = widget->style->xthickness;
      width_x = widget->allocation.width - 2 * widget->style->xthickness;
      width_y = borders[i+1] - borders[i];
      start_y = widget->allocation.height - width_y - borders[i];
      break;
    case GTK_ORIENTATION_HORIZONTAL:
    default:
      start_x = borders[i];
      width_x = borders[i+1] - borders[i];
      start_y = widget->style->ythickness;
      width_y = widget->allocation.height - 2 * widget->style->ythickness;
    }

    gdk_gc_set_foreground (gc, &(g_array_index (lm->colorEntries, GmLevelMeterColorEntry, i).color) );
    gdk_draw_rectangle (lm->offscreen_image_hl,
			gc, TRUE,
			start_x, start_y,
			width_x, width_y);
    gdk_gc_set_foreground (gc, &(g_array_index (lm->colorEntries, GmLevelMeterColorEntry, i).darkcolor) );
    gdk_draw_rectangle (lm->offscreen_image_dark,
			gc,
			TRUE, /* filled */
			start_x, start_y,
			width_x, width_y);
  }

  g_object_unref (gc);
  g_free (borders);
}


static void
gm_level_meter_realize (GtkWidget *widget)
{
  GmLevelMeter *lm = NULL;
  GdkWindowAttr attributes;
  gint attributes_mask;

  g_return_if_fail (GM_IS_LEVEL_METER (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  lm = GM_LEVEL_METER (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask =
    gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;
  widget->window =
    gdk_window_new (widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);
  gdk_window_set_user_data (widget->window, widget);
  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);

  gm_level_meter_create_pixmap (lm);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates the pixmaps in which the light and dark base image
 *                 and the offscreen image are stored
 * PRE          :  /
 */
static void
gm_level_meter_create_pixmap (GmLevelMeter *lm)
{
  GtkWidget *widget = NULL;

  g_return_if_fail (GM_IS_LEVEL_METER (lm));

#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_get_realized (GTK_WIDGET (lm))) {
#else
  if (GTK_WIDGET_REALIZED (lm)) {
#endif
    widget = GTK_WIDGET (lm);

    if (lm->offscreen_image)
      g_object_unref (lm->offscreen_image);
    if (lm->offscreen_image_hl)
      g_object_unref (lm->offscreen_image_hl);
    if (lm->offscreen_image_dark)
      g_object_unref (lm->offscreen_image_dark);

    lm->offscreen_image = gdk_pixmap_new (widget->window,
					  widget->allocation.width,
					  widget->allocation.height,
					  -1);
    lm->offscreen_image_hl = gdk_pixmap_new (widget->window,
					     widget->allocation.width,
					     widget->allocation.height,
					     -1);
    lm->offscreen_image_dark = gdk_pixmap_new (widget->window,
					       widget->allocation.width,
					       widget->allocation.height,
					       -1);
    gdk_draw_rectangle (lm->offscreen_image, widget->style->black_gc, TRUE,
		        0, 0,
			widget->allocation.width, widget->allocation.height);
    gdk_draw_rectangle (lm->offscreen_image_hl, widget->style->black_gc, TRUE,
                        0, 0,
                        widget->allocation.width, widget->allocation.height);
    gdk_draw_rectangle (lm->offscreen_image_dark, widget->style->black_gc, TRUE,
                        0, 0,
                        widget->allocation.width, widget->allocation.height);

    if (lm->colorEntries == NULL)
      lm->colorEntries =
        g_array_new (FALSE, FALSE, sizeof (GmLevelMeterColorEntry));

    gm_level_meter_allocate_colors (lm->colorEntries);
    gm_level_meter_rebuild_pixmap (lm);
    gm_level_meter_paint (lm);
  }
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Composes the offscreen image from the base images
 *                 according to level and peak value
 * PRE          :  /
 */
static void
gm_level_meter_paint (GmLevelMeter *lm)
{
  #define PEAKSTRENGTH 3

  GtkWidget *widget = GTK_WIDGET (lm);

  gint inner_width = 0;
  gint inner_height = 0;
  gint peak_start_x = 0;
  gint peak_start_y = 0;
  gint peak_width = 0;
  gint peak_height = 0;
  gint hl_start_x = 0;
  gint hl_start_y = 0;
  gint hl_width = 0;
  gint hl_height = 0;

  /* widget size minus borders */
  inner_width = widget->allocation.width - 2 * widget->style->xthickness;
  inner_height = widget->allocation.height - 2 * widget->style->ythickness;

  if (lm->peak > 1.0 ) lm->peak = 1.0;
  if (lm->level < 0 ) lm->level = 0;

  switch (lm->orientation) {
  case GTK_ORIENTATION_VERTICAL:
    peak_width = inner_width;
    peak_height = PEAKSTRENGTH;
    peak_start_x = 0;
    hl_width = inner_width;
    hl_start_y = (1.0 - lm->level) * inner_height;
    peak_start_y = (1.0 - lm->peak) * inner_height;

    if (peak_start_y + peak_height > inner_height)
      peak_height = inner_height - peak_start_y;
    if (hl_start_y - PEAKSTRENGTH <= peak_start_y) 
      hl_start_y = peak_start_y + PEAKSTRENGTH + 1;
    hl_height = inner_height - hl_start_y;
    if (hl_height < 0)
      hl_height = 0;
    break;
  case GTK_ORIENTATION_HORIZONTAL:
  default:
    peak_width = PEAKSTRENGTH;
    peak_height = inner_height;
    peak_start_y = 0;
    hl_start_y = 0;
    hl_width = lm->level * inner_width;
    peak_start_x = (lm->peak * inner_width) - PEAKSTRENGTH;

    if (peak_start_x < 0) {
      peak_width += peak_start_x;
      peak_start_x = 0;
    }
    hl_height = inner_height;
    if (hl_width >= peak_start_x) 
      hl_width = peak_start_x-1;
    if (hl_width < 0)
      hl_width = 0;
  }

  /* offset all values with x/ythickness */
  peak_start_x += widget->style->xthickness;
  peak_start_y += widget->style->ythickness;
  hl_start_x = widget->style->xthickness;
  hl_start_y += widget->style->ythickness;

  /* fill with dark and border */
  gdk_draw_drawable (lm->offscreen_image,
		     widget->style->black_gc,
		     lm->offscreen_image_dark,
		     0, 0,
		     0, 0,
		     widget->allocation.width, widget->allocation.height);
  /* paint level bar */
  gdk_draw_drawable (lm->offscreen_image,
		     widget->style->black_gc,
		     lm->offscreen_image_hl,
		     hl_start_x, hl_start_y, 
		     hl_start_x, hl_start_y,
		     hl_width, hl_height);
  /* paint peak */
  gdk_draw_drawable (lm->offscreen_image,
		     widget->style->black_gc,
		     lm->offscreen_image_hl,
		     peak_start_x, peak_start_y,
		     peak_start_x, peak_start_y,
		     peak_width, peak_height);

  /* repaint */
#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_is_drawable (widget))
#else
  if (GTK_WIDGET_DRAWABLE (widget))
#endif
    gdk_draw_drawable (widget->window,
		       widget->style->black_gc,
		       lm->offscreen_image,
		       0, 0,
		       0, 0,
		       widget->allocation.width, widget->allocation.height);

  gtk_widget_queue_draw_area (widget,
                              0, 0,
                              widget->allocation.width, widget->allocation.height);
}


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Sets the requisition to the minimun useful values depending
 *                 on the orientation and the border sizes
 * PRE          :  /
 */
static void 
gm_level_meter_size_request (GtkWidget *widget,
			     GtkRequisition *requisition)
{
  GmLevelMeter *lm = NULL;

  g_return_if_fail (GM_IS_LEVEL_METER (widget));

  lm = GM_LEVEL_METER (widget);

  switch (lm->orientation) {
  case GTK_ORIENTATION_VERTICAL:
    requisition->width = 4;
    requisition->height = 100;
    break;
  case GTK_ORIENTATION_HORIZONTAL:
  default:
    requisition->width = 100;
    requisition->height = 4;
  }
  requisition->width += 2 * widget->style->xthickness;
  requisition->height += 2 * widget->style->ythickness;
}


static void
gm_level_meter_size_allocate (GtkWidget *widget,
			      GtkAllocation *allocation)
{
  g_return_if_fail (GM_IS_LEVEL_METER (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_get_realized (widget)) {
#else
  if (GTK_WIDGET_REALIZED (widget)) {
#endif

    gdk_window_move_resize (widget->window,
                            allocation->x, allocation->y,
                            allocation->width, allocation->height);

    gm_level_meter_create_pixmap (GM_LEVEL_METER (widget));
  }
}


/* DESCRIPTION  :  Get called when the widget has to be redrawn
 * BEHAVIOR     :  The widget gets redrawn from an offscreen image
 * PRE          :  /
 */
static gboolean
gm_level_meter_expose (GtkWidget *widget,
                       GdkEventExpose *event)
{
  g_return_val_if_fail (GM_IS_LEVEL_METER (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  if (event->count > 0)
    return FALSE;

  /* repaint */
#if GTK_CHECK_VERSION(2,20,0)
  if (gtk_widget_is_drawable (widget))
#else
  if (GTK_WIDGET_DRAWABLE (widget))
#endif
    gdk_draw_drawable (widget->window,
		       widget->style->black_gc,
		       GM_LEVEL_METER (widget)->offscreen_image,
		       event->area.x, event->area.y,
		       event->area.x, event->area.y,
		       event->area.width,
		       event->area.height);

  return FALSE;
}
