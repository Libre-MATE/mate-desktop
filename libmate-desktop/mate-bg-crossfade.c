/* mate-bg-crossfade.h - fade window background between two surfaces
 *
 * Copyright (C) 2008 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * Author: Ray Strode <rstrode@redhat.com>
 */
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <cairo-xlib.h>
#include <cairo.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>

#define MATE_DESKTOP_USE_UNSTABLE_API
#include <mate-bg.h>

#include "mate-bg-crossfade.h"

typedef struct {
  GdkWindow *window;
  GtkWidget *widget;
  int width;
  int height;
  cairo_surface_t *fading_surface;
  cairo_surface_t *start_surface;
  cairo_surface_t *end_surface;
  gdouble start_time;
  gdouble total_duration;
  guint timeout_id;
  guint is_first_frame : 1;
} MateBGCrossfadePrivate;

enum {
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
};

enum { FINISHED, NUMBER_OF_SIGNALS };

static guint signals[NUMBER_OF_SIGNALS] = {0};

G_DEFINE_TYPE_WITH_PRIVATE(MateBGCrossfade, mate_bg_crossfade, G_TYPE_OBJECT)

static void mate_bg_crossfade_set_property(GObject *object, guint property_id,
                                           const GValue *value,
                                           GParamSpec *pspec) {
  MateBGCrossfadePrivate *priv;

  g_assert(MATE_IS_BG_CROSSFADE(object));

  priv = mate_bg_crossfade_get_instance_private(MATE_BG_CROSSFADE(object));

  switch (property_id) {
    case PROP_WIDTH:
      priv->width = g_value_get_int(value);
      break;
    case PROP_HEIGHT:
      priv->height = g_value_get_int(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void mate_bg_crossfade_get_property(GObject *object, guint property_id,
                                           GValue *value, GParamSpec *pspec) {
  MateBGCrossfadePrivate *priv;

  g_assert(MATE_IS_BG_CROSSFADE(object));

  priv = mate_bg_crossfade_get_instance_private(MATE_BG_CROSSFADE(object));

  switch (property_id) {
    case PROP_WIDTH:
      g_value_set_int(value, priv->width);
      break;
    case PROP_HEIGHT:
      g_value_set_int(value, priv->height);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
      break;
  }
}

static void mate_bg_crossfade_finalize(GObject *object) {
  MateBGCrossfadePrivate *priv;

  g_assert(MATE_IS_BG_CROSSFADE(object));

  priv = mate_bg_crossfade_get_instance_private(MATE_BG_CROSSFADE(object));

  g_clear_pointer(&priv->fading_surface, cairo_surface_destroy);
  g_clear_pointer(&priv->start_surface, cairo_surface_destroy);
  g_clear_pointer(&priv->end_surface, cairo_surface_destroy);
}

static void mate_bg_crossfade_class_init(MateBGCrossfadeClass *fade_class) {
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS(fade_class);

  gobject_class->get_property = mate_bg_crossfade_get_property;
  gobject_class->set_property = mate_bg_crossfade_set_property;
  gobject_class->finalize = mate_bg_crossfade_finalize;

  /**
   * MateBGCrossfade:width:
   *
   * When a crossfade is running, this is width of the fading
   * surface.
   */
  g_object_class_install_property(
      gobject_class, PROP_WIDTH,
      g_param_spec_int("width", "Window Width", "Width of window to fade", 0,
                       G_MAXINT, 0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  /**
   * MateBGCrossfade:height:
   *
   * When a crossfade is running, this is height of the fading
   * surface.
   */
  g_object_class_install_property(
      gobject_class, PROP_HEIGHT,
      g_param_spec_int("height", "Window Height", "Height of window to fade on",
                       0, G_MAXINT, 0,
                       G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE));

  /**
   * MateBGCrossfade::finished:
   * @fade: the #MateBGCrossfade that received the signal
   * @window: the #GdkWindow the crossfade happend on.
   *
   * When a crossfade finishes, @window will have a copy
   * of the end surface as its background, and this signal will
   * get emitted.
   */
  signals[FINISHED] = g_signal_new(
      "finished", G_OBJECT_CLASS_TYPE(gobject_class), G_SIGNAL_RUN_LAST, 0,
      NULL, NULL, g_cclosure_marshal_VOID__OBJECT, G_TYPE_NONE, 1,
      G_TYPE_OBJECT);
}

static void mate_bg_crossfade_init(MateBGCrossfade *fade) {
  MateBGCrossfadePrivate *priv;

  priv = mate_bg_crossfade_get_instance_private(fade);

  priv->window = NULL;
  priv->widget = NULL;
  priv->fading_surface = NULL;
  priv->start_surface = NULL;
  priv->end_surface = NULL;
  priv->timeout_id = 0;
}

/**
 * mate_bg_crossfade_new:
 * @width: The width of the crossfading window
 * @height: The height of the crossfading window
 *
 * Creates a new object to manage crossfading a
 * window background between two #cairo_surface_ts.
 *
 * Return value: the new #MateBGCrossfade
 **/
MateBGCrossfade *mate_bg_crossfade_new(int width, int height) {
  GObject *object;

  object = g_object_new(MATE_TYPE_BG_CROSSFADE, "width", width, "height",
                        height, NULL);

  return (MateBGCrossfade *)object;
}

static cairo_surface_t *tile_surface(cairo_surface_t *surface, int width,
                                     int height) {
  cairo_surface_t *copy;
  cairo_t *cr;

  if (surface == NULL) {
    copy = gdk_window_create_similar_surface(
        gdk_get_default_root_window(), CAIRO_CONTENT_COLOR, width, height);
  } else {
    copy = cairo_surface_create_similar(
        surface, cairo_surface_get_content(surface), width, height);
  }

  cr = cairo_create(copy);

  if (surface != NULL) {
    cairo_pattern_t *pattern;

    cairo_set_source_surface(cr, surface, 0.0, 0.0);
    pattern = cairo_get_source(cr);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
  } else {
    static GtkCssProvider *provider = NULL;
    GtkStyleContext *context;
    GdkRGBA bg;

    if (provider == NULL) provider = gtk_css_provider_new();

    context = gtk_style_context_new();
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_THEME);
    gtk_style_context_get_background_color(context, GTK_STATE_FLAG_NORMAL, &bg);
    gdk_cairo_set_source_rgba(cr, &bg);
    g_object_unref(G_OBJECT(context));
  }

  cairo_paint(cr);

  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS)
    g_clear_pointer(&copy, cairo_surface_destroy);

  cairo_destroy(cr);

  return copy;
}

/**
 * mate_bg_crossfade_set_start_surface:
 * @fade: a #MateBGCrossfade
 * @surface: The cairo surface to fade from
 *
 * Before initiating a crossfade with mate_bg_crossfade_start()
 * a start and end surface have to be set.  This function sets
 * the surface shown at the beginning of the crossfade effect.
 *
 * Return value: %TRUE if successful, or %FALSE if the surface
 * could not be copied.
 **/
gboolean mate_bg_crossfade_set_start_surface(MateBGCrossfade *fade,
                                             cairo_surface_t *surface) {
  MateBGCrossfadePrivate *priv;

  g_return_val_if_fail(MATE_IS_BG_CROSSFADE(fade), FALSE);

  priv = mate_bg_crossfade_get_instance_private(fade);
  g_clear_pointer(&priv->start_surface, cairo_surface_destroy);
  priv->start_surface = tile_surface(surface, priv->width, priv->height);

  return priv->start_surface != NULL;
}

static gdouble get_current_time(void) {
  const double microseconds_per_second = (double)G_USEC_PER_SEC;
  gint64 tv = g_get_real_time();
  return ((double)tv) / microseconds_per_second;
}

/**
 * mate_bg_crossfade_set_end_surface:
 * @fade: a #MateBGCrossfade
 * @surface: The cairo surface to fade to
 *
 * Before initiating a crossfade with mate_bg_crossfade_start()
 * a start and end surface have to be set.  This function sets
 * the surface shown at the end of the crossfade effect.
 *
 * Return value: %TRUE if successful, or %FALSE if the surface
 * could not be copied.
 **/
gboolean mate_bg_crossfade_set_end_surface(MateBGCrossfade *fade,
                                           cairo_surface_t *surface) {
  MateBGCrossfadePrivate *priv;

  g_return_val_if_fail(MATE_IS_BG_CROSSFADE(fade), FALSE);

  priv = mate_bg_crossfade_get_instance_private(fade);
  g_clear_pointer(&priv->end_surface, cairo_surface_destroy);
  priv->end_surface = tile_surface(surface, priv->width, priv->height);

  /* Reset timer in case we're called while animating
   */
  priv->start_time = get_current_time();
  return priv->end_surface != NULL;
}

static gboolean animations_are_disabled(MateBGCrossfade *fade) {
  GtkSettings *settings;
  GdkScreen *screen;
  gboolean are_enabled;

  MateBGCrossfadePrivate *priv;

  priv = mate_bg_crossfade_get_instance_private(fade);

  g_assert(priv->window != NULL);

  screen = gdk_window_get_screen(priv->window);
  settings = gtk_settings_get_for_screen(screen);
  g_object_get(settings, "gtk-enable-animations", &are_enabled, NULL);

  return !are_enabled;
}

static void send_root_property_change_notification(MateBGCrossfade *fade) {
  long zero_length_pixmap = 0;
  MateBGCrossfadePrivate *priv;

  priv = mate_bg_crossfade_get_instance_private(fade);

  /* We do a zero length append to force a change notification,
   * without changing the value */
  XChangeProperty(GDK_WINDOW_XDISPLAY(priv->window),
                  GDK_WINDOW_XID(priv->window),
                  gdk_x11_get_xatom_by_name("_XROOTPMAP_ID"), XA_PIXMAP, 32,
                  PropModeAppend, (unsigned char *)&zero_length_pixmap, 0);
}

static void draw_background(MateBGCrossfade *fade) {
  MateBGCrossfadePrivate *priv;

  priv = mate_bg_crossfade_get_instance_private(fade);

  if (priv->widget != NULL) {
    gtk_widget_queue_draw(priv->widget);
  } else if (gdk_window_get_window_type(priv->window) != GDK_WINDOW_ROOT) {
    cairo_t *cr;
    cairo_region_t *region;
    GdkDrawingContext *draw_context;

    region = gdk_window_get_visible_region(priv->window);
    draw_context = gdk_window_begin_draw_frame(priv->window, region);
    cr = gdk_drawing_context_get_cairo_context(draw_context);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_surface(cr, priv->fading_surface, 0, 0);
    cairo_paint(cr);
    gdk_window_end_draw_frame(priv->window, draw_context);
    cairo_region_destroy(region);
  } else {
    Display *xdisplay = GDK_WINDOW_XDISPLAY(priv->window);
    GdkDisplay *display = gdk_display_get_default();

    gdk_x11_display_error_trap_push(display);
    XGrabServer(xdisplay);
    XClearWindow(xdisplay, GDK_WINDOW_XID(priv->window));
    send_root_property_change_notification(fade);
    XFlush(xdisplay);
    XUngrabServer(xdisplay);
    gdk_x11_display_error_trap_pop_ignored(display);
  }
}

static gboolean on_widget_draw(GtkWidget *widget, cairo_t *cr,
                               MateBGCrossfade *fade) {
  MateBGCrossfadePrivate *priv;

  priv = mate_bg_crossfade_get_instance_private(fade);

  g_assert(priv->fading_surface != NULL);

  cairo_set_source_surface(cr, priv->fading_surface, 0, 0);
  cairo_pattern_set_extend(cairo_get_source(cr), CAIRO_EXTEND_REPEAT);
  cairo_paint(cr);

  return FALSE;
}

static gboolean on_tick(MateBGCrossfade *fade) {
  gdouble now, percent_done;
  cairo_t *cr;
  cairo_status_t status;
  MateBGCrossfadePrivate *priv;

  g_return_val_if_fail(MATE_IS_BG_CROSSFADE(fade), FALSE);

  priv = mate_bg_crossfade_get_instance_private(fade);
  now = get_current_time();

  percent_done = (now - priv->start_time) / priv->total_duration;
  percent_done = CLAMP(percent_done, 0.0, 1.0);

  /* If it's taking a long time to get to the first frame,
   * then lengthen the duration, so the user will get to see
   * the effect.
   */
  if (priv->is_first_frame && percent_done > .33) {
    priv->is_first_frame = FALSE;
    priv->total_duration *= 1.5;
    return on_tick(fade);
  }

  if (priv->fading_surface == NULL || priv->end_surface == NULL)
    return FALSE;

  if (animations_are_disabled(fade))
    return FALSE;

  /* We accumulate the results in place for performance reasons.
   *
   * This means 1) The fade is exponential, not linear (looks good!)
   * 2) The rate of fade is not independent of frame rate. Slower machines
   * will get a slower fade (but never longer than .75 seconds), and
   * even the fastest machines will get *some* fade because the framerate
   * is capped.
   */
  cr = cairo_create(priv->fading_surface);

  cairo_set_source_surface(cr, priv->end_surface, 0.0, 0.0);
  cairo_paint_with_alpha(cr, percent_done);

  status = cairo_status(cr);
  cairo_destroy(cr);

  if (status == CAIRO_STATUS_SUCCESS)
    draw_background(fade);

  return percent_done <= .99;
}

static void on_finished(MateBGCrossfade *fade) {
  cairo_t *cr;
  MateBGCrossfadePrivate *priv;

  priv = mate_bg_crossfade_get_instance_private(fade);

  if (priv->timeout_id == 0) return;

  g_assert(priv->fading_surface != NULL);
  g_assert(priv->end_surface != NULL);

  cr = cairo_create(priv->fading_surface);
  cairo_set_source_surface(cr, priv->end_surface, 0, 0);
  cairo_paint(cr);
  cairo_destroy(cr);
  draw_background(fade);

  g_clear_pointer(&priv->fading_surface, cairo_surface_destroy);
  g_clear_pointer(&priv->end_surface, cairo_surface_destroy);

  g_assert(priv->start_surface != NULL);

  g_clear_pointer(&priv->start_surface, cairo_surface_destroy);

  if (priv->widget != NULL) {
    g_signal_handlers_disconnect_by_func(priv->widget,
                                         (GCallback)on_widget_draw, fade);
  }
  priv->widget = NULL;

  priv->timeout_id = 0;
  g_signal_emit(fade, signals[FINISHED], 0, priv->window);
}

/* This function queries the _XROOTPMAP_ID property from the root window
 * to determine the current root window background pixmap and returns a
 * surface to draw directly to it.
 * If _XROOTPMAP_ID is not set, then NULL returned.
 */
static cairo_surface_t *get_root_pixmap_id_surface(GdkDisplay *display) {
  GdkScreen *screen;
  Display *xdisplay;
  Visual *xvisual;
  Window xroot;
  Atom type;
  int format, result;
  unsigned long nitems, bytes_after;
  unsigned char *data;
  cairo_surface_t *surface = NULL;

  g_return_val_if_fail(display != NULL, NULL);

  screen = gdk_display_get_default_screen(display);
  xdisplay = GDK_DISPLAY_XDISPLAY(display);
  xvisual = GDK_VISUAL_XVISUAL(gdk_screen_get_system_visual(screen));
  xroot = RootWindow(xdisplay, GDK_SCREEN_XNUMBER(screen));

  result = XGetWindowProperty(
      xdisplay, xroot, gdk_x11_get_xatom_by_name("_XROOTPMAP_ID"), 0L, 1L,
      False, XA_PIXMAP, &type, &format, &nitems, &bytes_after, &data);

  if (result != Success || type != XA_PIXMAP || format != 32 || nitems != 1) {
    XFree(data);
    data = NULL;
  }

  if (data != NULL) {
    Pixmap pixmap = *(Pixmap *)data;
    Window root_ret;
    int x_ret, y_ret;
    unsigned int w_ret, h_ret, bw_ret, depth_ret;

    gdk_x11_display_error_trap_push(display);
    if (XGetGeometry(xdisplay, pixmap, &root_ret, &x_ret, &y_ret, &w_ret,
                     &h_ret, &bw_ret, &depth_ret)) {
      surface = cairo_xlib_surface_create(xdisplay, pixmap, xvisual, (int)w_ret,
                                          (int)h_ret);
    }

    gdk_x11_display_error_trap_pop_ignored(display);
    XFree(data);
  }

  gdk_display_flush(display);
  return surface;
}

/**
 * mate_bg_crossfade_start:
 * @fade: a #MateBGCrossfade
 * @window: The #GdkWindow to draw crossfade on
 *
 * This function initiates a quick crossfade between two surfaces on
 * the background of @window. Before initiating the crossfade both
 * mate_bg_crossfade_set_start_surface() and
 * mate_bg_crossfade_set_end_surface() need to be called. If animations
 * are disabled, the crossfade is skipped, and the window background is
 * set immediately to the end surface.
 **/
void mate_bg_crossfade_start(MateBGCrossfade *fade, GdkWindow *window) {
  GSource *source;
  GMainContext *context;
  MateBGCrossfadePrivate *priv;

  g_return_if_fail(MATE_IS_BG_CROSSFADE(fade));

  priv = mate_bg_crossfade_get_instance_private(fade);

  g_return_if_fail(window != NULL);
  g_return_if_fail(priv->start_surface != NULL);
  g_return_if_fail(priv->end_surface != NULL);
  g_return_if_fail(!mate_bg_crossfade_is_started(fade));
  g_return_if_fail(gdk_window_get_window_type(window) != GDK_WINDOW_FOREIGN);

  /* If drawing is done on the root window,
   * it is essential to have the root pixmap.
   */
  if (gdk_window_get_window_type(window) == GDK_WINDOW_ROOT) {
    GdkDisplay *display = gdk_window_get_display(window);
    cairo_surface_t *surface = get_root_pixmap_id_surface(display);

    g_return_if_fail(surface != NULL);
    cairo_surface_destroy(surface);
  }

  if (priv->fading_surface != NULL) {
    cairo_surface_destroy(priv->fading_surface);
    priv->fading_surface = NULL;
  }

  priv->window = window;
  if (gdk_window_get_window_type(priv->window) != GDK_WINDOW_ROOT) {
    priv->fading_surface =
        tile_surface(priv->start_surface, priv->width, priv->height);
    if (priv->widget != NULL) {
      g_signal_connect(priv->widget, "draw", (GCallback)on_widget_draw, fade);
    }
  } else {
    cairo_t *cr;
    GdkDisplay *display = gdk_window_get_display(priv->window);

    priv->fading_surface = get_root_pixmap_id_surface(display);
    cr = cairo_create(priv->fading_surface);
    cairo_set_source_surface(cr, priv->start_surface, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
  }
  draw_background(fade);

  source = g_timeout_source_new(1000 / 60);
  g_source_set_callback(source, (GSourceFunc)on_tick, fade,
                        (GDestroyNotify)on_finished);
  context = g_main_context_default();
  priv->timeout_id = g_source_attach(source, context);
  g_source_unref(source);

  priv->is_first_frame = TRUE;
  priv->total_duration = .75;
  priv->start_time = get_current_time();
}

/**
 * mate_bg_crossfade_start_widget:
 * @fade: a #MateBGCrossfade
 * @widget: The #GtkWidget to draw crossfade on
 *
 * This function initiates a quick crossfade between two surfaces on
 * the background of @widget. Before initiating the crossfade both
 * mate_bg_crossfade_set_start_surface() and
 * mate_bg_crossfade_set_end_surface() need to be called. If animations
 * are disabled, the crossfade is skipped, and the window background is
 * set immediately to the end surface.
 **/
void mate_bg_crossfade_start_widget(MateBGCrossfade *fade, GtkWidget *widget) {
  GdkWindow *window;
  MateBGCrossfadePrivate *priv;

  g_return_if_fail(MATE_IS_BG_CROSSFADE(fade));
  g_return_if_fail(widget != NULL);

  priv = mate_bg_crossfade_get_instance_private(fade);
  priv->widget = widget;
  gtk_widget_realize(priv->widget);
  window = gtk_widget_get_window(priv->widget);

  mate_bg_crossfade_start(fade, window);
}

/**
 * mate_bg_crossfade_is_started:
 * @fade: a #MateBGCrossfade
 *
 * This function reveals whether or not @fade is currently
 * running on a window.  See mate_bg_crossfade_start() for
 * information on how to initiate a crossfade.
 *
 * Return value: %TRUE if fading, or %FALSE if not fading
 **/
gboolean mate_bg_crossfade_is_started(MateBGCrossfade *fade) {
  MateBGCrossfadePrivate *priv;

  g_return_val_if_fail(MATE_IS_BG_CROSSFADE(fade), FALSE);

  priv = mate_bg_crossfade_get_instance_private(fade);
  return priv->timeout_id != 0;
}

/**
 * mate_bg_crossfade_stop:
 * @fade: a #MateBGCrossfade
 *
 * This function stops any in progress crossfades that may be
 * happening.  It's harmless to call this function if @fade is
 * already stopped.
 **/
void mate_bg_crossfade_stop(MateBGCrossfade *fade) {
  MateBGCrossfadePrivate *priv;

  g_return_if_fail(MATE_IS_BG_CROSSFADE(fade));

  priv = mate_bg_crossfade_get_instance_private(fade);

  if (!mate_bg_crossfade_is_started(fade)) return;

  g_assert(priv->timeout_id != 0);
  g_source_remove(priv->timeout_id);
  priv->timeout_id = 0;
}
