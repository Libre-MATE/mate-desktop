/* mate-bg-crossfade.h - fade window background between two pixmaps

   Copyright 2008, Red Hat, Inc.

   This file is part of the Mate Library.

   The Mate Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Mate Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Mate Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
   Floor, Boston, MA 02110-1301 US.

   Author: Ray Strode <rstrode@redhat.com>
*/

#ifndef __MATE_BG_CROSSFADE_H__
#define __MATE_BG_CROSSFADE_H__

#ifndef MATE_DESKTOP_USE_UNSTABLE_API
#error    MateBGCrossfade is unstable API. You must define MATE_DESKTOP_USE_UNSTABLE_API before including mate-bg-crossfade.h
#endif

#include <gdk/gdk.h>
#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MATE_TYPE_BG_CROSSFADE (mate_bg_crossfade_get_type())
G_DECLARE_DERIVABLE_TYPE(MateBGCrossfade, mate_bg_crossfade, MATE, BG_CROSSFADE,
                         GObject)

struct _MateBGCrossfadeClass {
  GObjectClass parent_class;

  void (*finished)(MateBGCrossfade *fade, GdkWindow *window);
};

MateBGCrossfade *mate_bg_crossfade_new(int width, int height);

gboolean mate_bg_crossfade_set_start_surface(MateBGCrossfade *fade,
                                             cairo_surface_t *surface);
gboolean mate_bg_crossfade_set_end_surface(MateBGCrossfade *fade,
                                           cairo_surface_t *surface);

void mate_bg_crossfade_start(MateBGCrossfade *fade, GdkWindow *window);
void mate_bg_crossfade_start_widget(MateBGCrossfade *fade, GtkWidget *widget);
gboolean mate_bg_crossfade_is_started(MateBGCrossfade *fade);
void mate_bg_crossfade_stop(MateBGCrossfade *fade);

G_END_DECLS

#endif /* __MATE_BG_CROSSFADE_H__ */
