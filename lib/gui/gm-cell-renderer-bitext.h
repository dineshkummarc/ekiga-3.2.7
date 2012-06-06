
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
 *                         gm-cell-renderer-bitext.h  -  description
 *                         ------------------------------------------
 *   begin                : written in 2006 by Julien Puydt, but very directly
 *                          inspired by the code of GossipCellRendererText in
 *                          imendio's gossip instant messaging client
 *   copyright            : (c) 2004 by Imendio AB
 *                          (c) 2006-2007 by Julien Puydt
 *   description          : interface of a cell renderer showing two texts
 *
 */

#ifndef __GM_CELL_RENDERER_BITEXT_H__
#define __GM_CELL_RENDERER_BITEXT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS typedef struct _GmCellRendererBitext GmCellRendererBitext;
typedef struct _GmCellRendererBitextClass GmCellRendererBitextClass;
typedef struct _GmCellRendererBitextPrivate GmCellRendererBitextPrivate;

struct _GmCellRendererBitext
{
  GtkCellRendererText parent;

  GmCellRendererBitextPrivate *priv;
};

struct _GmCellRendererBitextClass
{
  GtkCellRendererTextClass parent;
};

/* public api */

GtkCellRenderer *gm_cell_renderer_bitext_new ();

/* GObject thingies */

#define GM_TYPE_CELL_RENDERER_BITEXT (gm_cell_renderer_bitext_get_type ())
#define GM_CELL_RENDERER_BITEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), GM_TYPE_CELL_RENDERER_BITEXT, GmCellRendererBitext))
#define GM_CELL_RENDERER_BITEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), GM_TYPE_CELL_RENDERER_BITEXT, GmCellRendererBitextClass))
#define GM_IS_CELL_RENDERER_BITEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GM_TYPE_CELL_RENDERER_BITEXT))
#define GM_IS_CELL_RENDERER_BITEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GM_TYPE_CELL_RENDERER_BITEXT))
#define GM_CELL_RENDERER_BITEXT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GM_TYPE_CELL_RENDERER_BITEXT, GmCellRendererBitextClass))

GType gm_cell_renderer_bitext_get_type ();

G_END_DECLS
#endif
