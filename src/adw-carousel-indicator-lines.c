/*
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "config.h"

#include "adw-carousel-indicator-lines.h"

#include "adw-animation-private.h"
#include "adw-swipeable.h"

#include <math.h>

#define LINE_WIDTH 3
#define LINE_LENGTH 35
#define LINE_SPACING 5
#define LINE_OPACITY 0.3
#define LINE_OPACITY_ACTIVE 0.9
#define LINE_MARGIN 2

/**
 * AdwCarouselIndicatorLines:
 *
 * A lines indicator for [class@Adw.Carousel].
 *
 * The `AdwCarouselIndicatorLines` widget shows a set of lines for each page of
 * a given [class@Adw.Carousel]. The carousel's active page is shown as another
 * line that moves between them to match the carousel's position.
 *
 * See also [class@Adw.CarouselIndicatorDots].
 *
 * ## CSS nodes
 *
 * `AdwCarouselIndicatorLines` has a single CSS node with name
 * `carouselindicatorlines`.
 *
 * Since: 1.0
 */

struct _AdwCarouselIndicatorLines
{
  GtkWidget parent_instance;

  AdwCarousel *carousel;
  GtkOrientation orientation;

  AdwAnimation *animation;
};

G_DEFINE_FINAL_TYPE_WITH_CODE (AdwCarouselIndicatorLines, adw_carousel_indicator_lines, GTK_TYPE_WIDGET,
                               G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL))

enum {
  PROP_0,
  PROP_CAROUSEL,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_CAROUSEL + 1,
};

static GParamSpec *props[LAST_PROP];

static void
value_cb (double     value,
          GtkWidget *widget)
{
  gtk_widget_queue_draw (widget);
}

static void
done_cb (AdwCarouselIndicatorLines *self)
{
  g_clear_object (&self->animation);
}

static void
animate (AdwCarouselIndicatorLines *self,
         gint64                     duration)
{
  if (self->animation)
    adw_animation_stop (self->animation);

  self->animation =
    adw_animation_new (GTK_WIDGET (self), 0, 1, duration,
                       (AdwAnimationTargetFunc) value_cb,
                       self);

  g_signal_connect_swapped (self->animation, "done", G_CALLBACK (done_cb), self);

  adw_animation_start (self->animation);
}

static GdkRGBA
get_color (GtkWidget *widget)
{
  GtkStyleContext *context;
  GdkRGBA color;

  context = gtk_widget_get_style_context (widget);
  gtk_style_context_get_color (context, &color);

  return color;
}

static void
snapshot_lines (GtkWidget      *widget,
                GtkSnapshot    *snapshot,
                GtkOrientation  orientation,
                double          position,
                double         *sizes,
                guint           n_pages)
{
  GdkRGBA color;
  int i, widget_length, widget_thickness;
  double indicator_length, full_size, line_size;
  double x = 0, y = 0, pos;

  color = get_color (widget);
  color.alpha *= LINE_OPACITY;

  line_size = LINE_LENGTH + LINE_SPACING;
  indicator_length = -LINE_SPACING;
  for (i = 0; i < n_pages; i++)
    indicator_length += line_size * sizes[i];

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    widget_length = gtk_widget_get_width (widget);
    widget_thickness = gtk_widget_get_height (widget);
  } else {
    widget_length = gtk_widget_get_height (widget);
    widget_thickness = gtk_widget_get_width (widget);
  }

  /* Ensure the indicators are aligned to pixel grid when not animating */
  full_size = round (indicator_length / line_size) * line_size;
  if ((widget_length - (int) full_size) % 2 == 0)
    widget_length--;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    x = (widget_length - indicator_length) / 2.0;
    y = (widget_thickness - LINE_WIDTH) / 2;
  } else {
    x = (widget_thickness - LINE_WIDTH) / 2;
    y = (widget_length - indicator_length) / 2.0;
  }

  pos = 0;
  for (i = 0; i < n_pages; i++) {
    double length;
    graphene_rect_t rectangle;

    length = (LINE_LENGTH + LINE_SPACING) * sizes[i] - LINE_SPACING;

    if (length > 0) {
      if (orientation == GTK_ORIENTATION_HORIZONTAL)
        graphene_rect_init (&rectangle, x + pos, y, length, LINE_WIDTH);
      else
        graphene_rect_init (&rectangle, x, y + pos, LINE_WIDTH, length);
    }

    gtk_snapshot_append_color (snapshot, &color, &rectangle);

    pos += (LINE_LENGTH + LINE_SPACING) * sizes[i];
  }

  color = get_color (widget);
  color.alpha *= LINE_OPACITY_ACTIVE;

  pos = position * (LINE_LENGTH + LINE_SPACING);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_snapshot_append_color (snapshot, &color,
                               &GRAPHENE_RECT_INIT (x + pos, y, LINE_LENGTH, LINE_WIDTH));
  else
    gtk_snapshot_append_color (snapshot, &color,
                               &GRAPHENE_RECT_INIT (x, y + pos, LINE_WIDTH, LINE_LENGTH));
}

static void
n_pages_changed_cb (AdwCarouselIndicatorLines *self)
{
  animate (self, adw_carousel_get_reveal_duration (self->carousel));
}

static void
adw_carousel_indicator_lines_measure (GtkWidget      *widget,
                                      GtkOrientation  orientation,
                                      int             for_size,
                                      int            *minimum,
                                      int            *natural,
                                      int            *minimum_baseline,
                                      int            *natural_baseline)
{
  AdwCarouselIndicatorLines *self = ADW_CAROUSEL_INDICATOR_LINES (widget);
  int size = 0;

  if (orientation == self->orientation) {
    int n_pages = 0;
    if (self->carousel)
      n_pages = adw_carousel_get_n_pages (self->carousel);

    size = MAX (0, (LINE_LENGTH + LINE_SPACING) * n_pages - LINE_SPACING);
  } else {
    size = LINE_WIDTH;
  }

  size += 2 * LINE_MARGIN;

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
adw_carousel_indicator_lines_snapshot (GtkWidget   *widget,
                                       GtkSnapshot *snapshot)
{
  AdwCarouselIndicatorLines *self = ADW_CAROUSEL_INDICATOR_LINES (widget);
  int i, n_points;
  double position;
  g_autofree double *points = NULL;
  g_autofree double *sizes = NULL;

  if (!self->carousel)
    return;

  points = adw_swipeable_get_snap_points (ADW_SWIPEABLE (self->carousel), &n_points);
  position = adw_carousel_get_position (self->carousel);

  if (n_points < 2)
    return;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
      gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    position = points[n_points - 1] - position;

  sizes = g_new0 (double, n_points);

  sizes[0] = points[0] + 1;
  for (i = 1; i < n_points; i++)
    sizes[i] = points[i] - points[i - 1];

  snapshot_lines (widget, snapshot, self->orientation, position, sizes, n_points);
}

static void
adw_carousel_dispose (GObject *object)
{
  AdwCarouselIndicatorLines *self = ADW_CAROUSEL_INDICATOR_LINES (object);

  adw_carousel_indicator_lines_set_carousel (self, NULL);

  G_OBJECT_CLASS (adw_carousel_indicator_lines_parent_class)->dispose (object);
}

static void
adw_carousel_indicator_lines_get_property (GObject    *object,
                                           guint       prop_id,
                                           GValue     *value,
                                           GParamSpec *pspec)
{
  AdwCarouselIndicatorLines *self = ADW_CAROUSEL_INDICATOR_LINES (object);

  switch (prop_id) {
  case PROP_CAROUSEL:
    g_value_set_object (value, adw_carousel_indicator_lines_get_carousel (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_carousel_indicator_lines_set_property (GObject      *object,
                                           guint         prop_id,
                                           const GValue *value,
                                           GParamSpec   *pspec)
{
  AdwCarouselIndicatorLines *self = ADW_CAROUSEL_INDICATOR_LINES (object);

  switch (prop_id) {
  case PROP_CAROUSEL:
    adw_carousel_indicator_lines_set_carousel (self, g_value_get_object (value));
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
adw_carousel_indicator_lines_class_init (AdwCarouselIndicatorLinesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->dispose = adw_carousel_dispose;
  object_class->get_property = adw_carousel_indicator_lines_get_property;
  object_class->set_property = adw_carousel_indicator_lines_set_property;

  widget_class->measure = adw_carousel_indicator_lines_measure;
  widget_class->snapshot = adw_carousel_indicator_lines_snapshot;

  /**
   * AdwCarouselIndicatorLines:carousel: (attributes org.gtk.Property.get=adw_carousel_indicator_lines_get_carousel org.gtk.Property.set=adw_carousel_indicator_lines_set_carousel)
   *
   * The displayed carousel.
   *
   * Since: 1.0
   */
  props[PROP_CAROUSEL] =
    g_param_spec_object ("carousel",
                         "Carousel",
                         "The displayed carousel",
                         ADW_TYPE_CAROUSEL,
                         G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  gtk_widget_class_set_css_name (widget_class, "carouselindicatorlines");
}

static void
adw_carousel_indicator_lines_init (AdwCarouselIndicatorLines *self)
{
}

/**
 * adw_carousel_indicator_lines_new:
 *
 * Creates a new `AdwCarouselIndicatorLines`.
 *
 * Returns: the newly created `AdwCarouselIndicatorLines`
 *
 * Since: 1.0
 */
GtkWidget *
adw_carousel_indicator_lines_new (void)
{
  return g_object_new (ADW_TYPE_CAROUSEL_INDICATOR_LINES, NULL);
}

/**
 * adw_carousel_indicator_lines_get_carousel: (attributes org.gtk.Method.get_property=carousel)
 * @self: a `AdwCarouselIndicatorLines`
 *
 * Gets the displayed carousel.
 *
 * Returns: (nullable) (transfer none): the displayed carousel
 *
 * Since: 1.0
 */
AdwCarousel *
adw_carousel_indicator_lines_get_carousel (AdwCarouselIndicatorLines *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL_INDICATOR_LINES (self), NULL);

  return self->carousel;
}

/**
 * adw_carousel_indicator_lines_set_carousel: (attributes org.gtk.Method.set_property=carousel)
 * @self: a `AdwCarouselIndicatorLines`
 * @carousel: (nullable): a carousel
 *
 * Sets the displayed carousel.
 *
 * Since: 1.0
 */
void
adw_carousel_indicator_lines_set_carousel (AdwCarouselIndicatorLines *self,
                                           AdwCarousel               *carousel)
{
  g_return_if_fail (ADW_IS_CAROUSEL_INDICATOR_LINES (self));
  g_return_if_fail (carousel == NULL || ADW_IS_CAROUSEL (carousel));

  if (self->carousel == carousel)
    return;

  if (self->animation)
    adw_animation_stop (self->animation);

  if (self->carousel) {
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
