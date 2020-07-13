/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-carousel-indicator-dots.h"

#include "hdy-animation-private.h"
#include "hdy-swipeable.h"

#include <math.h>

#define DOTS_RADIUS 3
#define DOTS_RADIUS_SELECTED 4
#define DOTS_OPACITY 0.3
#define DOTS_OPACITY_SELECTED 0.9
#define DOTS_SPACING 7
#define DOTS_MARGIN 6

/**
 * SECTION:hdy-carousel-indicator-dots
 * @short_description: A dots indicator for #HdyCarousel
 * @title: HdyCarouselIndicatorDots
 * @See_also: #HdyCarousel, #HdyCarouselIndicatorLines
 *
 * The #HdyCarouselIndicatorDots widget can be used to show a set of dots for each
 * page of a given #HdyCarousel. The dot representing the carousel's active page
 * is larger and more opaque than the others, the transition to the active and
 * inactive state is gradual to match the carousel's position.
 *
 * # CSS nodes
 *
 * #HdyCarouselIndicatorDots has a single CSS node with name carouselindicatordots.
 *
 * Since: 1.0
 */

struct _HdyCarouselIndicatorDots
{
  GtkDrawingArea parent_instance;

  HdyCarousel *carousel;
  GtkOrientation orientation;

  guint tick_cb_id;
  guint64 end_time;
};

G_DEFINE_TYPE_WITH_CODE (HdyCarouselIndicatorDots, hdy_carousel_indicator_dots, GTK_TYPE_DRAWING_AREA,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_CAROUSEL,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_CAROUSEL + 1,
};

static GParamSpec *props[LAST_PROP];

static gboolean
animation_cb (GtkWidget     *widget,
              GdkFrameClock *frame_clock,
              gpointer       user_data)
{
  HdyCarouselIndicatorDots *self = HDY_CAROUSEL_INDICATOR_DOTS (widget);
  gint64 frame_time;

  g_assert (self->tick_cb_id > 0);

  gtk_widget_queue_draw (GTK_WIDGET (self));

  frame_time = gdk_frame_clock_get_frame_time (frame_clock) / 1000;

  if (frame_time >= self->end_time ||
      !hdy_get_enable_animations (GTK_WIDGET (self))) {
    self->tick_cb_id = 0;
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}

static void
stop_animation (HdyCarouselIndicatorDots *self)
{
  if (self->tick_cb_id == 0)
    return;

  gtk_widget_remove_tick_callback (GTK_WIDGET (self), self->tick_cb_id);
  self->tick_cb_id = 0;
}

static void
animate (HdyCarouselIndicatorDots *self,
         gint64                    duration)
{
  GdkFrameClock *frame_clock;
  gint64 frame_time;

  if (duration <= 0 || !hdy_get_enable_animations (GTK_WIDGET (self))) {
    gtk_widget_queue_draw (GTK_WIDGET (self));
    return;
  }

  frame_clock = gtk_widget_get_frame_clock (GTK_WIDGET (self));
  if (!frame_clock) {
    gtk_widget_queue_draw (GTK_WIDGET (self));
    return;
  }

  frame_time = gdk_frame_clock_get_frame_time (frame_clock);

  self->end_time = MAX (self->end_time, frame_time / 1000 + duration);
  if (self->tick_cb_id == 0)
    self->tick_cb_id = gtk_widget_add_tick_callback (GTK_WIDGET (self),
                                                     animation_cb,
                                                     NULL, NULL);
}

static GdkRGBA
get_color (GtkWidget *widget)
{
  GtkStyleContext *context;
  GtkStateFlags flags;
  GdkRGBA color;

  context = gtk_widget_get_style_context (widget);
  flags = gtk_widget_get_state_flags (widget);
  gtk_style_context_get_color (context, flags, &color);

  return color;
}

static void
draw_dots (GtkWidget      *widget,
           cairo_t        *cr,
           GtkOrientation  orientation,
           gdouble         position,
           gdouble        *sizes,
           guint           n_pages)
{
  GdkRGBA color;
  gint i, widget_length, widget_thickness;
  gdouble x, y, indicator_length, dot_size, full_size;
  gdouble current_position, remaining_progress;

  color = get_color (widget);
  dot_size = 2 * DOTS_RADIUS_SELECTED + DOTS_SPACING;

  indicator_length = 0;
  for (i = 0; i < n_pages; i++)
    indicator_length += dot_size * sizes[i];

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    widget_length = gtk_widget_get_allocated_width (widget);
    widget_thickness = gtk_widget_get_allocated_height (widget);
  } else {
    widget_length = gtk_widget_get_allocated_height (widget);
    widget_thickness = gtk_widget_get_allocated_width (widget);
  }

  /* Ensure the indicators are aligned to pixel grid when not animating */
  full_size = round (indicator_length / dot_size) * dot_size;
  if ((widget_length - (gint) full_size) % 2 == 0)
    widget_length--;

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    cairo_translate (cr, (widget_length - indicator_length) / 2.0, widget_thickness / 2);
  else
    cairo_translate (cr, widget_thickness / 2, (widget_length - indicator_length) / 2.0);

  x = 0;
  y = 0;

  current_position = 0;
  remaining_progress = 1;

  for (i = 0; i < n_pages; i++) {
    gdouble progress, radius, opacity;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      x += dot_size * sizes[i] / 2.0;
    else
      y += dot_size * sizes[i] / 2.0;

    current_position += sizes[i];

    progress = CLAMP (current_position - position, 0, remaining_progress);
    remaining_progress -= progress;

    radius = hdy_lerp (DOTS_RADIUS, DOTS_RADIUS_SELECTED, progress) * sizes[i];
    opacity = hdy_lerp (DOTS_OPACITY, DOTS_OPACITY_SELECTED, progress) * sizes[i];

    cairo_set_source_rgba (cr, color.red, color.green, color.blue,
                           color.alpha * opacity);
    cairo_arc (cr, x, y, radius, 0, 2 * G_PI);
    cairo_fill (cr);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      x += dot_size * sizes[i] / 2.0;
    else
      y += dot_size * sizes[i] / 2.0;
  }
}

static void
n_pages_changed_cb (HdyCarouselIndicatorDots *self)
{
  animate (self, hdy_carousel_get_reveal_duration (self->carousel));
}

static void
hdy_carousel_indicator_dots_measure (GtkWidget      *widget,
                                     GtkOrientation  orientation,
                                     gint            for_size,
                                     gint           *minimum,
                                     gint           *natural,
                                     gint           *minimum_baseline,
                                     gint           *natural_baseline)
{
  HdyCarouselIndicatorDots *self = HDY_CAROUSEL_INDICATOR_DOTS (widget);
  gint size = 0;

  if (orientation == self->orientation) {
    gint n_pages = 0;
    if (self->carousel)
      n_pages = hdy_carousel_get_n_pages (self->carousel);

    size = MAX (0, (2 * DOTS_RADIUS_SELECTED + DOTS_SPACING) * n_pages - DOTS_SPACING);
  } else {
    size = 2 * DOTS_RADIUS_SELECTED;
  }

  size += 2 * DOTS_MARGIN;

  if (minimum)
    *minimum = size;

  if (natural)
    *natural = size;

  if (minimum_baseline)
    *minimum_baseline = -1;

  if (natural_baseline)
    *natural_baseline = -1;
}

static void
hdy_carousel_indicator_dots_get_preferred_width (GtkWidget *widget,
                                                 gint      *minimum_width,
                                                 gint      *natural_width)
{
  hdy_carousel_indicator_dots_measure (widget, GTK_ORIENTATION_HORIZONTAL, -1,
                                  minimum_width, natural_width, NULL, NULL);
}

static void
hdy_carousel_indicator_dots_get_preferred_height (GtkWidget *widget,
                                                  gint      *minimum_height,
                                                  gint      *natural_height)
{
  hdy_carousel_indicator_dots_measure (widget, GTK_ORIENTATION_VERTICAL, -1,
                                  minimum_height, natural_height, NULL, NULL);
}

static gboolean
hdy_carousel_indicator_dots_draw (GtkWidget *widget,
                                  cairo_t   *cr)
{
  HdyCarouselIndicatorDots *self = HDY_CAROUSEL_INDICATOR_DOTS (widget);
  gint i, n_points;
  gdouble position;
  g_autofree gdouble *points = NULL;
  g_autofree gdouble *sizes = NULL;

  if (!self->carousel)
    return GDK_EVENT_PROPAGATE;

  points = hdy_swipeable_get_snap_points (HDY_SWIPEABLE (self->carousel), &n_points);
  position = hdy_carousel_get_position (self->carousel);

  if (n_points < 2)
    return GDK_EVENT_PROPAGATE;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
      gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    position = points[n_points - 1] - position;

  sizes = g_new0 (gdouble, n_points);

  sizes[0] = points[0] + 1;
  for (i = 1; i < n_points; i++)
    sizes[i] = points[i] - points[i - 1];

  draw_dots (widget, cr, self->orientation, position, sizes, n_points);

  return GDK_EVENT_PROPAGATE;
}

static void
hdy_carousel_dispose (GObject *object)
{
  HdyCarouselIndicatorDots *self = HDY_CAROUSEL_INDICATOR_DOTS (object);

  hdy_carousel_indicator_dots_set_carousel (self, NULL);

  G_OBJECT_CLASS (hdy_carousel_indicator_dots_parent_class)->dispose (object);
}

static void
hdy_carousel_indicator_dots_get_property (GObject    *object,
                                          guint       prop_id,
                                          GValue     *value,
                                          GParamSpec *pspec)
{
  HdyCarouselIndicatorDots *self = HDY_CAROUSEL_INDICATOR_DOTS (object);

  switch (prop_id) {
  case PROP_CAROUSEL:
    g_value_set_object (value, hdy_carousel_indicator_dots_get_carousel (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_carousel_indicator_dots_set_property (GObject      *object,
                                          guint         prop_id,
                                          const GValue *value,
                                          GParamSpec   *pspec)
{
  HdyCarouselIndicatorDots *self = HDY_CAROUSEL_INDICATOR_DOTS (object);

  switch (prop_id) {
  case PROP_CAROUSEL:
    hdy_carousel_indicator_dots_set_carousel (self, g_value_get_object (value));
    break;

  case PROP_ORIENTATION:
    {
      GtkOrientation orientation = g_value_get_enum (value);
      if (orientation != self->orientation) {
        self->orientation = orientation;
        gtk_widget_queue_resize (GTK_WIDGET (self));
        g_object_notify (G_OBJECT (self), "orientation");
      }
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_carousel_indicator_dots_class_init (HdyCarouselIndicatorDotsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = hdy_carousel_dispose;
  object_class->get_property = hdy_carousel_indicator_dots_get_property;
  object_class->set_property = hdy_carousel_indicator_dots_set_property;

  widget_class->get_preferred_width = hdy_carousel_indicator_dots_get_preferred_width;
  widget_class->get_preferred_height = hdy_carousel_indicator_dots_get_preferred_height;
  widget_class->draw = hdy_carousel_indicator_dots_draw;

  /**
   * HdyCarouselIndicatorDots:carousel:
   *
   * The #HdyCarousel the indicator uses.
   *
   * Since: 1.0
   */
  props[PROP_CAROUSEL] =
    g_param_spec_object ("carousel",
                         _("Carousel"),
                         _("Carousel"),
                         HDY_TYPE_CAROUSEL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "carouselindicatordots");
}

static void
hdy_carousel_indicator_dots_init (HdyCarouselIndicatorDots *self)
{
}

/**
 * hdy_carousel_indicator_dots_new:
 *
 * Create a new #HdyCarouselIndicatorDots widget.
 *
 * Returns: (transfer full): The newly created #HdyCarouselIndicatorDots widget
 *
 * Since: 1.0
 */
GtkWidget *
hdy_carousel_indicator_dots_new (void)
{
  return g_object_new (HDY_TYPE_CAROUSEL_INDICATOR_DOTS, NULL);
}

/**
 * hdy_carousel_indicator_dots_get_carousel:
 * @self: a #HdyCarouselIndicatorDots
 *
 * Get the #HdyCarousel the indicator uses.
 *
 * See: hdy_carousel_indicator_dots_set_carousel()
 *
 * Returns: (nullable) (transfer none): the #HdyCarousel, or %NULL if none has been set
 *
 * Since: 1.0
 */

HdyCarousel *
hdy_carousel_indicator_dots_get_carousel (HdyCarouselIndicatorDots *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL_INDICATOR_DOTS (self), NULL);

  return self->carousel;
}

/**
 * hdy_carousel_indicator_dots_set_carousel:
 * @self: a #HdyCarouselIndicatorDots
 * @carousel: (nullable): a #HdyCarousel
 *
 * Sets the #HdyCarousel to use.
 *
 * Since: 1.0
 */
void
hdy_carousel_indicator_dots_set_carousel (HdyCarouselIndicatorDots *self,
                                          HdyCarousel              *carousel)
{
  g_return_if_fail (HDY_IS_CAROUSEL_INDICATOR_DOTS (self));
  g_return_if_fail (HDY_IS_CAROUSEL (carousel) || carousel == NULL);

  if (self->carousel == carousel)
    return;

  if (self->carousel) {
    stop_animation (self);
    g_signal_handlers_disconnect_by_func (self->carousel, gtk_widget_queue_draw, self);
    g_signal_handlers_disconnect_by_func (self->carousel, n_pages_changed_cb, self);
  }

  g_set_object (&self->carousel, carousel);

  if (self->carousel) {
    g_signal_connect_object (self->carousel, "notify::position",
                             G_CALLBACK (gtk_widget_queue_draw), self,
                             G_CONNECT_SWAPPED);
    g_signal_connect_object (self->carousel, "notify::n-pages",
                             G_CALLBACK (n_pages_changed_cb), self,
                             G_CONNECT_SWAPPED);
  }

  gtk_widget_queue_draw (GTK_WIDGET (self));

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CAROUSEL]);
}
