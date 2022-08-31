/* HSV color selector for GTK+
 *
 * Copyright (C) 1999 The Free Software Foundation
 * Copyright (C) 2019-2021 MATE Developers
 *
 * Authors: Simon Budig <Simon.Budig@unix-ag.org> (original code)
 *          Federico Mena-Quintero <federico@gimp.org> (cleanup for GTK+)
 *          Jonathan Blandford <jrb@redhat.com> (cleanup for GTK+)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 *
 * Modified to work internally in mate-desktop by Pablo Barciela 2019
 */

#ifndef __MATE_HSV_H__
#define __MATE_HSV_H__

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MATE_TYPE_HSV (mate_hsv_get_type())
G_DECLARE_DERIVABLE_TYPE(MateHSV, mate_hsv, MATE, HSV, GtkWidget);

struct _MateHSVClass {
  GtkWidgetClass parent_class;

  /* Notification signals */
  void (*changed)(MateHSV *hsv);

  /* Keybindings */
  void (*move)(MateHSV *hsv, GtkDirectionType type);

  /* Padding for future expansion */
  void (*_gtk_reserved1)(void);
  void (*_gtk_reserved2)(void);
  void (*_gtk_reserved3)(void);
  void (*_gtk_reserved4)(void);
};

GtkWidget *mate_hsv_new(void);
void mate_hsv_set_color(MateHSV *hsv, double h, double s, double v);
void mate_hsv_get_color(MateHSV *hsv, gdouble *h, gdouble *s, gdouble *v);
void mate_hsv_set_metrics(MateHSV *hsv, gint size, gint ring_width);
void mate_hsv_get_metrics(MateHSV *hsv, gint *size, gint *ring_width);
gboolean mate_hsv_is_adjusting(MateHSV *hsv);

G_END_DECLS

#endif /* __MATE_HSV_H__ */
