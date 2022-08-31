/*
 * GTK - The GIMP Toolkit
 * Copyright (C) 1998, 1999 Red Hat, Inc.
 * All rights reserved.
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Mate Library; see the file COPYING.LIB. If not,
 * write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/* Color picker button for GNOME
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 *
 * Modified by the GTK+ Team and others 2003.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __MATE_COLOR_BUTTON_H__
#define __MATE_COLOR_BUTTON_H__

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/* The MateColorButton widget is a simple color picker in a button.
 * The button displays a sample of the currently selected color.  When
 * the user clicks on the button, a color selection dialog pops up.
 * The color picker emits the "color_set" signal when the color is set.
 */

#define MATE_TYPE_COLOR_BUTTON (mate_color_button_get_type())
G_DECLARE_DERIVABLE_TYPE(MateColorButton, mate_color_button, MATE, COLOR_BUTTON,
                         GtkButton)

struct _MateColorButtonClass {
  GtkButtonClass parent_class;

  void (*color_set)(MateColorButton *cp);

  /* Padding for future expansion */
  void (*_gtk_reserved1)(void);
  void (*_gtk_reserved2)(void);
  void (*_gtk_reserved3)(void);
  void (*_gtk_reserved4)(void);
};

GtkWidget *mate_color_button_new(void);
GtkWidget *mate_color_button_new_with_color(const GdkColor *color);
void mate_color_button_set_color(MateColorButton *color_button,
                                 const GdkColor *color);
void mate_color_button_set_rgba(MateColorButton *color_button,
                                const GdkRGBA *color);
void mate_color_button_set_alpha(MateColorButton *color_button, guint alpha);
void mate_color_button_get_color(MateColorButton *color_button,
                                 GdkColor *color);
void mate_color_button_get_rgba(MateColorButton *color_button, GdkRGBA *color);
guint16 mate_color_button_get_alpha(MateColorButton *color_button);
void mate_color_button_set_use_alpha(MateColorButton *color_button,
                                     gboolean use_alpha);
gboolean mate_color_button_get_use_alpha(MateColorButton *color_button);
void mate_color_button_set_title(MateColorButton *color_button,
                                 const gchar *title);
const gchar *mate_color_button_get_title(MateColorButton *color_button);

G_END_DECLS

#endif /* __MATE_COLOR_BUTTON_H__ */
