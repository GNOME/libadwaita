/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-cairo-private.h"
#include "hdy-shadow-helper-private.h"

#include <math.h>

/**
 * PRIVATE:hdy-shadow-helper
 * @short_description: Shadow helper used in #HdyLeaflet
 * @title: HdyShadowHelper
 * @See_also: #HdyLeaflet
 * @stability: Private
 *
 * A helper class for drawing #HdyLeaflet transition shadow.
 *
 * Since: 0.0.12
 */

struct _HdyShadowHelper
{
  GObject parent_instance;

  GtkWidget *widget;

  gboolean is_cache_valid;

  cairo_pattern_t *dimming_pattern;
  cairo_pattern_t *shadow_pattern;
  cairo_pattern_t *border_pattern;
  cairo_pattern_t *outline_pattern;
  gint shadow_size;
  gint border_size;
  gint outline_size;

  GtkPanDirection last_direction;
  gint last_width;
  gint last_height;
  gint last_scale;
};

G_DEFINE_TYPE (HdyShadowHelper, hdy_shadow_helper, G_TYPE_OBJECT);

enum {
  PROP_0,
  PROP_WIDGET,
  LAST_PROP,
};

static GParamSpec *props[LAST_PROP];


static GtkStyleContext *
create_context (HdyShadowHelper *self,
                const gchar     *name,
                GtkPanDirection  direction)
{
  g_autoptr(GtkWidgetPath) path = NULL;
  GtkStyleContext *context;
  gint pos;
  const gchar *direction_name;
  GEnumClass *enum_class;

  enum_class = g_type_class_ref (GTK_TYPE_PAN_DIRECTION);
  direction_name = g_enum_get_value (enum_class, direction)->value_nick;

  path = gtk_widget_path_copy (gtk_widget_get_path (self->widget));

  pos = gtk_widget_path_append_type (path, GTK_TYPE_WIDGET);
  gtk_widget_path_iter_set_object_name (path, pos, name);

  gtk_widget_path_iter_add_class (path, pos, direction_name);

  context = gtk_style_context_new ();
  gtk_style_context_set_path (context, path);

  g_type_class_unref (enum_class);

  return context;
}

static gint
get_element_size (GtkStyleContext *context,
                  GtkPanDirection  direction)
{
  gint width, height;

  gtk_style_context_get (context,
                         gtk_style_context_get_state (context),
                         "min-width", &width,
                         "min-height", &height,
                         NULL);

  switch (direction) {
  case GTK_PAN_DIRECTION_LEFT:
  case GTK_PAN_DIRECTION_RIGHT:
    return width;
  case GTK_PAN_DIRECTION_UP:
  case GTK_PAN_DIRECTION_DOWN:
    return height;
  default:
    g_assert_not_reached ();
  }

  return 0;
}

static cairo_pattern_t *
create_element_pattern (GtkStyleContext *context,
                        gint             width,
                        gint             height)
{
  g_autoptr (cairo_surface_t) surface =
    cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  g_autoptr (cairo_t) cr = cairo_create (surface);
  cairo_pattern_t *pattern;

  gtk_render_background (context, cr, 0, 0, width, height);
  gtk_render_frame (context, cr, 0, 0, width, height);

  pattern = cairo_pattern_create_for_surface (surface);

  return pattern;
}

static void
cache_shadow (HdyShadowHelper *self,
              gint             width,
              gint             height,
              GtkPanDirection  direction)
{
  g_autoptr(GtkStyleContext) dim_context = NULL;
  g_autoptr(GtkStyleContext) shadow_context = NULL;
  g_autoptr(GtkStyleContext) border_context = NULL;
  g_autoptr(GtkStyleContext) outline_context = NULL;
  gint shadow_size, border_size, outline_size, scale;

  scale = gtk_widget_get_scale_factor (self->widget);

  if (self->last_direction == direction &&
      self->last_width == width &&
      self->last_height == height &&
      self->last_scale == scale &&
      self->is_cache_valid)
    return;

  hdy_shadow_helper_clear_cache (self);

  dim_context = create_context (self, "dimming", direction);
  shadow_context = create_context (self, "shadow", direction);
  border_context = create_context (self, "border", direction);
  outline_context = create_context (self, "outline", direction);

  shadow_size = get_element_size (shadow_context, direction);
  border_size = get_element_size (border_context, direction);
  outline_size = get_element_size (outline_context, direction);

  self->dimming_pattern = create_element_pattern (dim_context, width, height);
  if (direction == GTK_PAN_DIRECTION_LEFT || direction == GTK_PAN_DIRECTION_RIGHT) {
    self->shadow_pattern = create_element_pattern (shadow_context, shadow_size, height);
    self->border_pattern = create_element_pattern (border_context, border_size, height);
    self->outline_pattern = create_element_pattern (outline_context, outline_size, height);
  } else {
    self->shadow_pattern = create_element_pattern (shadow_context, width, shadow_size);
    self->border_pattern = create_element_pattern (border_context, width, border_size);
    self->outline_pattern = create_element_pattern (outline_context, width, outline_size);
  }

  self->border_size = border_size;
  self->shadow_size = shadow_size;
  self->outline_size = outline_size;

  self->is_cache_valid = TRUE;
  self->last_direction = direction;
  self->last_width = width;
  self->last_height = height;
  self->last_scale = scale;
}

static void
hdy_shadow_helper_dispose (GObject *object)
{
  HdyShadowHelper *self = HDY_SHADOW_HELPER (object);

  hdy_shadow_helper_clear_cache (self);

  if (self->widget)
    g_clear_object (&self->widget);

  G_OBJECT_CLASS (hdy_shadow_helper_parent_class)->dispose (object);
}

static void
hdy_shadow_helper_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdyShadowHelper *self = HDY_SHADOW_HELPER (object);

  switch (prop_id) {
  case PROP_WIDGET:
    g_value_set_object (value, self->widget);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_shadow_helper_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdyShadowHelper *self = HDY_SHADOW_HELPER (object);

  switch (prop_id) {
  case PROP_WIDGET:
    self->widget = GTK_WIDGET (g_object_ref (g_value_get_object (value)));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_shadow_helper_class_init (HdyShadowHelperClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = hdy_shadow_helper_dispose;
  object_class->get_property = hdy_shadow_helper_get_property;
  object_class->set_property = hdy_shadow_helper_set_property;

  /**
   * HdyShadowHelper:widget:
   *
   * The widget the shadow will be drawn for. Must not be %NULL
   *
   * Since: 0.0.11
   */
  props[PROP_WIDGET] =
    g_param_spec_object ("widget",
                         _("Widget"),
                         _("The widget the shadow will be drawn for"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, LAST_PROP, props);
}

static void
hdy_shadow_helper_init (HdyShadowHelper *self)
{
}

/**
 * hdy_shadow_helper_new:
 *
 * Creates a new #HdyShadowHelper object.
 *
 * Returns: The newly created #HdyShadowHelper object
 *
 * Since: 0.0.12
 */
HdyShadowHelper *
hdy_shadow_helper_new (GtkWidget *widget)
{
  return g_object_new (HDY_TYPE_SHADOW_HELPER,
                       "widget", widget,
                       NULL);
}

/**
 * hdy_shadow_helper_clear_cache:
 * @self: a #HdyShadowHelper
 *
 * Clears shadow cache. This should be used after a transition is done.
 *
 * Since: 0.0.12
 */
void
hdy_shadow_helper_clear_cache (HdyShadowHelper *self)
{
  if (!self->is_cache_valid)
    return;

  cairo_pattern_destroy (self->dimming_pattern);
  cairo_pattern_destroy (self->shadow_pattern);
  cairo_pattern_destroy (self->border_pattern);
  cairo_pattern_destroy (self->outline_pattern);
  self->border_size = 0;
  self->shadow_size = 0;
  self->outline_size = 0;

  self->last_direction = 0;
  self->last_width = 0;
  self->last_height = 0;
  self->last_scale = 0;

  self->is_cache_valid = FALSE;
}

/**
 * hdy_shadow_helper_draw_shadow:
 * @self: a #HdyShadowHelper
 * @cr: a Cairo context to draw to
 * @width: the width of the shadow rectangle
 * @height: the height of the shadow rectangle
 * @progress: transition progress, changes from 0 to 1
 * @direction: shadow direction
 *
 * Draws a transition shadow. For caching to work, @width, @height and
 * @direction shouldn't change between calls.
 *
 * Since: 0.0.12
 */
void
hdy_shadow_helper_draw_shadow (HdyShadowHelper *self,
                               cairo_t         *cr,
                               gint             width,
                               gint             height,
                               gdouble          progress,
                               GtkPanDirection  direction)
{
  gdouble remaining_distance, shadow_opacity;
  gint shadow_size, border_size, outline_size, distance;

  if (progress <= 0 || progress >= 1)
    return;

  cache_shadow (self, width, height, direction);

  shadow_size = self->shadow_size;
  border_size = self->border_size;
  outline_size = self->outline_size;

  switch (direction) {
  case GTK_PAN_DIRECTION_LEFT:
  case GTK_PAN_DIRECTION_RIGHT:
    distance = width;
    break;
  case GTK_PAN_DIRECTION_UP:
  case GTK_PAN_DIRECTION_DOWN:
    distance = height;
    break;
  default:
    g_assert_not_reached ();
  }

  remaining_distance = (1 - progress) * (gdouble) distance;
  shadow_opacity = 1;
  if (remaining_distance < shadow_size)
    shadow_opacity = (remaining_distance / shadow_size);

  cairo_save (cr);

  switch (direction) {
  case GTK_PAN_DIRECTION_LEFT:
    cairo_rectangle (cr, -outline_size, 0, width + outline_size, height);
    break;
  case GTK_PAN_DIRECTION_RIGHT:
    cairo_rectangle (cr, 0, 0, width + outline_size, height);
    break;
  case GTK_PAN_DIRECTION_UP:
    cairo_rectangle (cr, 0, -outline_size, width, height + outline_size);
    break;
  case GTK_PAN_DIRECTION_DOWN:
    cairo_rectangle (cr, 0, 0, width, height + outline_size);
    break;
  default:
    g_assert_not_reached ();
  }
  cairo_clip (cr);
  gdk_window_mark_paint_from_clip (gtk_widget_get_window (self->widget), cr);

  cairo_set_source (cr, self->dimming_pattern);
  cairo_paint_with_alpha (cr, 1 - progress);

  switch (direction) {
  case GTK_PAN_DIRECTION_RIGHT:
    cairo_translate (cr, width - shadow_size, 0);
    break;
  case GTK_PAN_DIRECTION_DOWN:
    cairo_translate (cr, 0, height - shadow_size);
    break;
  case GTK_PAN_DIRECTION_LEFT:
  case GTK_PAN_DIRECTION_UP:
    break;
  default:
    g_assert_not_reached ();
  }

  cairo_set_source (cr, self->shadow_pattern);
  cairo_paint_with_alpha (cr, shadow_opacity);

  switch (direction) {
  case GTK_PAN_DIRECTION_RIGHT:
    cairo_translate (cr, shadow_size - border_size, 0);
    break;
  case GTK_PAN_DIRECTION_DOWN:
    cairo_translate (cr, 0, shadow_size - border_size);
    break;
  case GTK_PAN_DIRECTION_LEFT:
  case GTK_PAN_DIRECTION_UP:
    break;
  default:
    g_assert_not_reached ();
  }

  cairo_set_source (cr, self->border_pattern);
  cairo_paint (cr);

  switch (direction) {
  case GTK_PAN_DIRECTION_RIGHT:
    cairo_translate (cr, border_size, 0);
    break;
  case GTK_PAN_DIRECTION_DOWN:
    cairo_translate (cr, 0, border_size);
    break;
  case GTK_PAN_DIRECTION_LEFT:
    cairo_translate (cr, -outline_size, 0);
    break;
  case GTK_PAN_DIRECTION_UP:
    cairo_translate (cr, 0, -outline_size);
    break;
  default:
    g_assert_not_reached ();
  }

  cairo_set_source (cr, self->outline_pattern);
  cairo_paint (cr);

  cairo_restore (cr);
}
