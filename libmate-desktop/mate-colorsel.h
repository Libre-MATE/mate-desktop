/* GTK - The GIMP Toolkit
 * Copyright (C) 2000 Red Hat, Inc.
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __MATE_COLOR_SELECTION_H__
#define __MATE_COLOR_SELECTION_H__

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MATE_TYPE_COLOR_SELECTION (mate_color_selection_get_type())
G_DECLARE_DERIVABLE_TYPE(MateColorSelection, mate_color_selection, MATE,
                         COLOR_SELECTION, GtkBox)

typedef void (*MateColorSelectionChangePaletteFunc)(const GdkColor *colors,
                                                    gint n_colors);
typedef void (*MateColorSelectionChangePaletteWithScreenFunc)(
    GdkScreen *screen, const GdkColor *colors, gint n_colors);

struct _MateColorSelectionClass {
  GtkBoxClass parent_class;

  void (*color_changed)(MateColorSelection *color_selection);

  /* Padding for future expansion */
  void (*_gtk_reserved1)(void);
  void (*_gtk_reserved2)(void);
  void (*_gtk_reserved3)(void);
  void (*_gtk_reserved4)(void);
};

/* ColorSelection */

GtkWidget *mate_color_selection_new(void);
gboolean mate_color_selection_get_has_opacity_control(
    MateColorSelection *colorsel);
void mate_color_selection_set_has_opacity_control(MateColorSelection *colorsel,
                                                  gboolean has_opacity);
gboolean mate_color_selection_get_has_palette(MateColorSelection *colorsel);
void mate_color_selection_set_has_palette(MateColorSelection *colorsel,
                                          gboolean has_palette);

void mate_color_selection_set_current_color(MateColorSelection *colorsel,
                                            const GdkColor *color);
void mate_color_selection_set_current_alpha(MateColorSelection *colorsel,
                                            guint alpha);
void mate_color_selection_get_current_color(MateColorSelection *colorsel,
                                            GdkColor *color);
guint16 mate_color_selection_get_current_alpha(MateColorSelection *colorsel);
void mate_color_selection_set_previous_color(MateColorSelection *colorsel,
                                             const GdkColor *color);
void mate_color_selection_set_previous_alpha(MateColorSelection *colorsel,
                                             guint16 alpha);
void mate_color_selection_get_previous_color(MateColorSelection *colorsel,
                                             GdkColor *color);
guint16 mate_color_selection_get_previous_alpha(MateColorSelection *colorsel);

gboolean mate_color_selection_is_adjusting(MateColorSelection *colorsel);

gboolean mate_color_selection_palette_from_string(const gchar *str,
                                                  GdkColor **colors,
                                                  gint *n_colors);
gchar *mate_color_selection_palette_to_string(const GdkColor *colors,
                                              gint n_colors);

#ifndef GTK_DISABLE_DEPRECATED
#ifndef GDK_MULTIHEAD_SAFE
MateColorSelectionChangePaletteFunc
mate_color_selection_set_change_palette_hook(
    MateColorSelectionChangePaletteFunc func);
#endif /* ! GDK_MULTIHEAD_SAFE */
#endif /* ! GTK_DISABLE_DEPRECATED */

MateColorSelectionChangePaletteWithScreenFunc
mate_color_selection_set_change_palette_with_screen_hook(
    MateColorSelectionChangePaletteWithScreenFunc func);

#ifndef GTK_DISABLE_DEPRECATED
/* Deprecated calls: */
void mate_color_selection_set_color(MateColorSelection *colorsel,
                                    gdouble *color);
void mate_color_selection_get_color(MateColorSelection *colorsel,
                                    gdouble *color);
#endif /* ! GTK_DISABLE_DEPRECATED */

G_END_DECLS

#endif /* __MATE_COLOR_SELECTION_H__ */
