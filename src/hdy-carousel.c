/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-carousel.h"

#include "hdy-carousel-box-private.h"
#include "hdy-navigation-direction.h"
#include "hdy-swipe-tracker-private.h"
#include "hdy-swipeable-private.h"

#include <math.h>

#define DOTS_RADIUS 3
#define DOTS_RADIUS_SELECTED 4
#define DOTS_OPACITY 0.3
#define DOTS_OPACITY_SELECTED 0.9
#define DOTS_SPACING 7
#define DOTS_MARGIN 6

#define LINE_WIDTH 3
#define LINE_LENGTH 35
#define LINE_SPACING 5
#define LINE_OPACITY 0.3
#define LINE_OPACITY_ACTIVE 0.9
#define LINE_MARGIN 2
#define DEFAULT_DURATION 250

/**
 * SECTION:hdy-carousel
 * @short_description: A paginated scrolling widget.
 * @title: HdyCarousel
 *
 * The #HdyCarousel widget can be used to display a set of pages with
 * swipe-based navigation between them and optional indicators.
 *
 * Since: 1.0
 */

/**
 * HdyCarouselIndicatorStyle
 * @HDY_CAROUSEL_INDICATOR_STYLE_NONE: No indicators
 * @HDY_CAROUSEL_INDICATOR_STYLE_DOTS: Each page is represented by a dot. Active dot gradually becomes larger and more opaque.
 * @HDY_CAROUSEL_INDICATOR_STYLE_LINES: Each page is represented by a thin and long line, and active view is shown with another line that moves between them
 *
 * These enumeration values describe the possible page indicator styles in a
 * #HdyCarousel widget.
 *
 * New values may be added to this enumeration over time.
 */

struct _HdyCarousel
{
  GtkEventBox parent_instance;

  GtkBox *box;
  GtkBox *empty_box;
  HdyCarouselBox *scrolling_box;
  GtkDrawingArea *indicators;

  HdySwipeTracker *tracker;

  HdyCarouselIndicatorStyle indicator_style;
  guint indicator_spacing;
  gboolean center_content;
  GtkOrientation orientation;
  guint animation_duration;

  gulong scroll_timeout_id;
  gboolean can_scroll;
};

static void hdy_carousel_swipeable_init (HdySwipeableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (HdyCarousel, hdy_carousel, GTK_TYPE_EVENT_BOX,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (HDY_TYPE_SWIPEABLE, hdy_carousel_swipeable_init))

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_POSITION,
  PROP_INTERACTIVE,
  PROP_INDICATOR_STYLE,
  PROP_INDICATOR_SPACING,
  PROP_CENTER_CONTENT,
  PROP_SPACING,
  PROP_ANIMATION_DURATION,
  PROP_ALLOW_MOUSE_DRAG,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ALLOW_MOUSE_DRAG + 1,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_PAGE_CHANGED,
  SIGNAL_LAST_SIGNAL,
};
static guint signals[SIGNAL_LAST_SIGNAL];

static void
hdy_carousel_switch_child (HdySwipeable *swipeable,
                           guint         index,
                           gint64        duration)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);
  GtkWidget *child;

  child = hdy_carousel_box_get_nth_child (self->scrolling_box, index);

  hdy_carousel_box_scroll_to (self->scrolling_box, child, duration);
}

static void
hdy_carousel_begin_swipe (HdySwipeable           *swipeable,
                          HdyNavigationDirection  direction,
                          gboolean                direct)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);
  gdouble distance, position, closest_point;
  guint i, n_pages;
  gdouble *points;

  hdy_carousel_box_stop_animation (self->scrolling_box);

  distance = hdy_carousel_box_get_distance (self->scrolling_box);
  g_object_get (self->scrolling_box,
                "position", &position,
                "n-pages", &n_pages,
                NULL);
  closest_point = CLAMP (round (position), 0, n_pages - 1);

  points = g_new (gdouble, n_pages);
  for (i = 0; i < n_pages; i++)
    points[i] = i;

  hdy_swipe_tracker_confirm_swipe (self->tracker, distance, points, n_pages,
                                   position, closest_point);
}

static void
hdy_carousel_update_swipe (HdySwipeable *swipeable,
                           gdouble       value)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  hdy_carousel_box_set_position (self->scrolling_box, value);
}

static void
hdy_carousel_end_swipe (HdySwipeable *swipeable,
                        gint64        duration,
                        gdouble       to)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  if (duration == 0) {
    hdy_carousel_box_set_position (self->scrolling_box, to);
    return;
  }

  hdy_carousel_box_animate (self->scrolling_box, to, duration);
}

static void
notify_n_pages_cb (HdyCarousel *self,
                   GParamSpec  *spec,
                   GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
  gtk_widget_queue_draw (GTK_WIDGET (self->indicators));
}

static void
notify_position_cb (HdyCarousel *self,
                    GParamSpec  *spec,
                    GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POSITION]);
  gtk_widget_queue_draw (GTK_WIDGET (self->indicators));
}

static void
notify_spacing_cb (HdyCarousel *self,
                   GParamSpec  *spec,
                   GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPACING]);
}

static void
animation_stopped_cb (HdyCarousel    *self,
                      HdyCarouselBox *box)
{
  gdouble position;
  gint index;

  position = hdy_carousel_box_get_position (self->scrolling_box);
  index = round (position);

  g_signal_emit (self, signals[SIGNAL_PAGE_CHANGED], 0, index);
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
draw_indicators_lines (GtkWidget      *widget,
                       cairo_t        *cr,
                       GtkOrientation  orientation,
                       gdouble         position,
                       guint           n_pages)
{
  GdkRGBA color;
  gint i, widget_length, indicator_length;
  gdouble length;

  color = get_color (widget);

  length = (gdouble) LINE_LENGTH / (LINE_LENGTH + LINE_SPACING);
  indicator_length = (LINE_LENGTH + LINE_SPACING) * n_pages - LINE_SPACING;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    widget_length = gtk_widget_get_allocated_width (widget);
    cairo_translate (cr, (widget_length - indicator_length) / 2, 0);
    cairo_scale (cr, LINE_LENGTH + LINE_SPACING, LINE_WIDTH);
  } else {
    widget_length = gtk_widget_get_allocated_height (widget);
    cairo_translate (cr, 0, (widget_length - indicator_length) / 2);
    cairo_scale (cr, LINE_WIDTH, LINE_LENGTH + LINE_SPACING);
  }

  cairo_set_source_rgba (cr, color.red, color.green, color.blue,
                         color.alpha * LINE_OPACITY);
  for (i = 0; i < n_pages; i++) {
    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      cairo_rectangle (cr, i, 0, length, 1);
    else
      cairo_rectangle (cr, 0, i, 1, length);
    cairo_fill (cr);
  }

  cairo_set_source_rgba (cr, color.red, color.green, color.blue,
                         color.alpha * LINE_OPACITY_ACTIVE);
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    cairo_rectangle (cr, position, 0, length, 1);
  else
    cairo_rectangle (cr, 0, position, 1, length);
  cairo_fill (cr);
}

#define LERP(a, b, t) ((a) + (((b) - (a)) * (t)))

static void
draw_indicators_dots (GtkWidget      *widget,
                      cairo_t        *cr,
                      GtkOrientation  orientation,
                      gdouble         position,
                      guint           n_pages)
{
  GdkRGBA color;
  gint i, x, y, widget_length, indicator_length;

  color = get_color (widget);

  indicator_length = (DOTS_RADIUS_SELECTED * 2 + DOTS_SPACING) * n_pages - DOTS_SPACING;

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    widget_length = gtk_widget_get_allocated_width (widget);
    cairo_translate (cr, (widget_length - indicator_length) / 2, 0);
  } else {
    widget_length = gtk_widget_get_allocated_height (widget);
    cairo_translate (cr, 0, (widget_length - indicator_length) / 2);
  }

  x = DOTS_RADIUS_SELECTED;
  y = DOTS_RADIUS_SELECTED;

  for (i = 0; i < n_pages; i++) {
    gdouble progress, radius, opacity;

    progress = MAX (1 - ABS (position - i), 0);
    radius = LERP (DOTS_RADIUS, DOTS_RADIUS_SELECTED, progress);
    opacity = LERP (DOTS_OPACITY, DOTS_OPACITY_SELECTED, progress);

    cairo_set_source_rgba (cr, color.red, color.green, color.blue,
                           color.alpha * opacity);
    cairo_arc (cr, x, y, radius, 0, 2 * G_PI);
    cairo_fill (cr);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
      x += 2 * DOTS_RADIUS_SELECTED + DOTS_SPACING;
    else
      y += 2 * DOTS_RADIUS_SELECTED + DOTS_SPACING;
  }
}

static gboolean
draw_indicators_cb (HdyCarousel *self,
                    cairo_t     *cr,
                    GtkWidget   *widget)
{
  guint n_pages;
  gdouble position;

  g_object_get (self->scrolling_box,
                "position", &position,
                "n-pages", &n_pages,
                NULL);

  if (n_pages < 2)
    return GDK_EVENT_PROPAGATE;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL &&
      gtk_widget_get_direction (widget) == GTK_TEXT_DIR_RTL)
    position = n_pages - position - 1;

  switch (self->indicator_style){
  case HDY_CAROUSEL_INDICATOR_STYLE_NONE:
    break;

  case HDY_CAROUSEL_INDICATOR_STYLE_DOTS:
    draw_indicators_dots (widget, cr, self->orientation, position, n_pages);
    break;

  case HDY_CAROUSEL_INDICATOR_STYLE_LINES:
    draw_indicators_lines (widget, cr, self->orientation, position, n_pages);
    break;

  default:
    g_assert_not_reached ();
  }

  return GDK_EVENT_PROPAGATE;
}

static void
update_indicators (HdyCarousel *self)
{
  gboolean show_indicators;
  gint size, margin;

  show_indicators = (self->indicator_style != HDY_CAROUSEL_INDICATOR_STYLE_NONE);
  gtk_widget_set_visible (GTK_WIDGET (self->indicators), show_indicators);
  gtk_widget_set_visible (GTK_WIDGET (self->empty_box),
                          show_indicators && self->center_content);

  if (!show_indicators)
    return;

  switch (self->indicator_style) {
  case HDY_CAROUSEL_INDICATOR_STYLE_DOTS:
    size = 2 * DOTS_RADIUS_SELECTED;
    margin = DOTS_MARGIN;
    break;

  case HDY_CAROUSEL_INDICATOR_STYLE_LINES:
    size = LINE_WIDTH;
    margin = LINE_MARGIN;
    break;

  case HDY_CAROUSEL_INDICATOR_STYLE_NONE:
  default:
    g_assert_not_reached ();
  }

  g_object_set (self->indicators,
                "margin", margin,
                "width-request", size,
                "height-request", size,
                NULL);
}

/* Copied from GtkOrientable. Orientable widgets are supposed
 * to do this manually via a private GTK function. */
static void
set_orientable_style_classes (GtkOrientable *orientable)
{
  GtkStyleContext *context;
  GtkOrientation orientation;

  g_return_if_fail (GTK_IS_ORIENTABLE (orientable));
  g_return_if_fail (GTK_IS_WIDGET (orientable));

  context = gtk_widget_get_style_context (GTK_WIDGET (orientable));
  orientation = gtk_orientable_get_orientation (orientable);

  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      gtk_style_context_add_class (context, GTK_STYLE_CLASS_HORIZONTAL);
      gtk_style_context_remove_class (context, GTK_STYLE_CLASS_VERTICAL);
    }
  else
    {
      gtk_style_context_add_class (context, GTK_STYLE_CLASS_VERTICAL);
      gtk_style_context_remove_class (context, GTK_STYLE_CLASS_HORIZONTAL);
    }
}

static void
update_orientation (HdyCarousel *self)
{
  GtkOrientation opposite;
  gboolean reversed;

  if (!self->scrolling_box)
    return;

  opposite = (self->orientation == GTK_ORIENTATION_HORIZONTAL) ?
    GTK_ORIENTATION_VERTICAL :
    GTK_ORIENTATION_HORIZONTAL;
  reversed = self->orientation == GTK_ORIENTATION_HORIZONTAL &&
    gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  g_object_set (self->scrolling_box, "orientation", self->orientation, NULL);
  g_object_set (self->tracker, "orientation", self->orientation,
                "reversed", reversed, NULL);
  g_object_set (self->box, "orientation", opposite, NULL);

  set_orientable_style_classes (GTK_ORIENTABLE (self));
  set_orientable_style_classes (GTK_ORIENTABLE (self->scrolling_box));

  gtk_widget_queue_draw (GTK_WIDGET (self->indicators));
}

static gboolean
scroll_timeout_cb (HdyCarousel *self)
{
  self->can_scroll = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
handle_discrete_scroll_event (HdyCarousel *self,
                              GdkEvent    *event)
{
  GdkDevice *source_device;
  GdkInputSource input_source;
  GdkScrollDirection direction;
  gdouble dx, dy;
  gint index;
  gboolean allow_vertical;
  GtkOrientation orientation;
  guint duration;

  if (!self->can_scroll)
    return GDK_EVENT_PROPAGATE;

  if (!hdy_carousel_get_interactive (self))
    return GDK_EVENT_PROPAGATE;

  if (event->type != GDK_SCROLL)
    return GDK_EVENT_PROPAGATE;

  source_device = gdk_event_get_source_device (event);
  input_source = gdk_device_get_source (source_device);
  if (input_source == GDK_SOURCE_TOUCHPAD)
    return GDK_EVENT_PROPAGATE;

  /* Mice often don't have easily accessible horizontal scrolling,
   * hence allow vertical mouse scrolling regardless of orientation */
  allow_vertical = (input_source == GDK_SOURCE_MOUSE);

  if (gdk_event_get_scroll_direction (event, &direction)) {
    dx = 0;
    dy = 0;

    switch (direction) {
    case GDK_SCROLL_UP:
      dy = -1;
      break;
    case GDK_SCROLL_DOWN:
      dy = 1;
      break;
    case GDK_SCROLL_LEFT:
      dy = -1;
      break;
    case GDK_SCROLL_RIGHT:
      dy = 1;
      break;
    case GDK_SCROLL_SMOOTH:
      g_assert_not_reached ();
    default:
      return GDK_EVENT_PROPAGATE;
    }
  } else {
    gdk_event_get_scroll_deltas (event, &dx, &dy);
  }

  orientation = gtk_orientable_get_orientation (GTK_ORIENTABLE (self));
  index = 0;

  if (orientation == GTK_ORIENTATION_VERTICAL || allow_vertical) {
    if (dy > 0)
      index++;
    else if (dy < 0)
      index--;
  }

  if (orientation == GTK_ORIENTATION_HORIZONTAL && index == 0) {
    if (dx > 0)
      index++;
    else if (dx < 0)
      index--;
  }

  if (index == 0)
    return GDK_EVENT_PROPAGATE;

  index += (gint) round (hdy_carousel_get_position (self));
  index = CLAMP (index, 0, (gint) hdy_carousel_get_n_pages (self) - 1);

  hdy_carousel_scroll_to (self, hdy_carousel_box_get_nth_child (self->scrolling_box, index));

  /* Don't allow the delay to go lower than 250ms */
  duration = MIN (self->animation_duration, DEFAULT_DURATION);

  self->can_scroll = FALSE;
  g_timeout_add (duration, (GSourceFunc) scroll_timeout_cb, self);

  return GDK_EVENT_STOP;
}

static gboolean
captured_event_cb (HdyCarousel *self,
                   GdkEvent    *event)
{
  if (hdy_swipe_tracker_captured_event (self->tracker, event))
    return GDK_EVENT_STOP;

  return handle_discrete_scroll_event (self, event);
}

static void
hdy_carousel_destroy (GtkWidget *widget)
{
  HdyCarousel *self = HDY_CAROUSEL (widget);

  if (self->box) {
    gtk_widget_destroy (GTK_WIDGET (self->box));
    self->box = NULL;
  }

  GTK_WIDGET_CLASS (hdy_carousel_parent_class)->destroy (widget);
}

static void
hdy_carousel_direction_changed (GtkWidget        *widget,
                                GtkTextDirection  previous_direction)
{
  HdyCarousel *self = HDY_CAROUSEL (widget);

  update_orientation (self);
}

static void
hdy_carousel_add (GtkContainer *container,
                  GtkWidget    *widget)
{
  HdyCarousel *self = HDY_CAROUSEL (container);

  if (self->scrolling_box)
    gtk_container_add (GTK_CONTAINER (self->scrolling_box), widget);
  else
    GTK_CONTAINER_CLASS (hdy_carousel_parent_class)->add (container, widget);
}

static void
hdy_carousel_remove (GtkContainer *container,
                     GtkWidget    *widget)
{
  HdyCarousel *self = HDY_CAROUSEL (container);

  if (self->scrolling_box)
    gtk_container_remove (GTK_CONTAINER (self->scrolling_box), widget);
  else
    GTK_CONTAINER_CLASS (hdy_carousel_parent_class)->remove (container, widget);
}

static void
hdy_carousel_forall (GtkContainer *container,
                     gboolean      include_internals,
                     GtkCallback   callback,
                     gpointer      callback_data)
{
  HdyCarousel *self = HDY_CAROUSEL (container);

  if (include_internals)
    (* callback) (GTK_WIDGET (self->box), callback_data);
  else if (self->scrolling_box)
    gtk_container_foreach (GTK_CONTAINER (self->scrolling_box),
                           callback, callback_data);
}

static void
hdy_carousel_constructed (GObject *object)
{
  HdyCarousel *self = (HdyCarousel *)object;

  update_orientation (self);

  G_OBJECT_CLASS (hdy_carousel_parent_class)->constructed (object);
}

static void
hdy_carousel_dispose (GObject *object)
{
  HdyCarousel *self = (HdyCarousel *)object;

  if (self->tracker) {
    g_clear_object (&self->tracker);

    g_object_set_data (object, "captured-event-handler", NULL);
  }

  if (self->scroll_timeout_id != 0) {
    g_source_remove (self->scroll_timeout_id);
    self->scroll_timeout_id = 0;
  }

  G_OBJECT_CLASS (hdy_carousel_parent_class)->dispose (object);
}

static void
hdy_carousel_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  HdyCarousel *self = HDY_CAROUSEL (object);

  switch (prop_id) {
  case PROP_N_PAGES:
    g_value_set_uint (value, hdy_carousel_get_n_pages (self));
    break;

  case PROP_POSITION:
    g_value_set_double (value, hdy_carousel_get_position (self));
    break;

  case PROP_INTERACTIVE:
    g_value_set_boolean (value, hdy_carousel_get_interactive (self));
    break;

  case PROP_INDICATOR_STYLE:
    g_value_set_enum (value, hdy_carousel_get_indicator_style (self));
    break;

  case PROP_INDICATOR_SPACING:
    g_value_set_uint (value, hdy_carousel_get_indicator_spacing (self));
    break;

  case PROP_CENTER_CONTENT:
    g_value_set_boolean (value, hdy_carousel_get_center_content (self));
    break;

  case PROP_SPACING:
    g_value_set_uint (value, hdy_carousel_get_spacing (self));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    g_value_set_boolean (value, hdy_carousel_get_allow_mouse_drag (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  case PROP_ANIMATION_DURATION:
    g_value_set_uint (value, hdy_carousel_get_animation_duration (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_carousel_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  HdyCarousel *self = HDY_CAROUSEL (object);

  switch (prop_id) {
  case PROP_INTERACTIVE:
    hdy_carousel_set_interactive (self, g_value_get_boolean (value));
    break;

  case PROP_INDICATOR_STYLE:
    hdy_carousel_set_indicator_style (self, g_value_get_enum (value));
    break;

  case PROP_INDICATOR_SPACING:
    hdy_carousel_set_indicator_spacing (self, g_value_get_uint (value));
    break;

  case PROP_CENTER_CONTENT:
    hdy_carousel_set_center_content (self, g_value_get_boolean (value));
    break;

  case PROP_SPACING:
    hdy_carousel_set_spacing (self, g_value_get_uint (value));
    break;

  case PROP_ANIMATION_DURATION:
    hdy_carousel_set_animation_duration (self, g_value_get_uint (value));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    hdy_carousel_set_allow_mouse_drag (self, g_value_get_boolean (value));
    break;

  case PROP_ORIENTATION:
    {
      GtkOrientation orientation = g_value_get_enum (value);
      if (orientation != self->orientation) {
        self->orientation = orientation;
        update_orientation (self);
        g_object_notify (G_OBJECT (self), "orientation");
      }
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_carousel_swipeable_init (HdySwipeableInterface *iface)
{
  iface->switch_child = hdy_carousel_switch_child;
  iface->begin_swipe = hdy_carousel_begin_swipe;
  iface->update_swipe = hdy_carousel_update_swipe;
  iface->end_swipe = hdy_carousel_end_swipe;
}

static void
hdy_carousel_class_init (HdyCarouselClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
  GtkContainerClass *container_class = GTK_CONTAINER_CLASS (klass);

  object_class->constructed = hdy_carousel_constructed;
  object_class->dispose = hdy_carousel_dispose;
  object_class->get_property = hdy_carousel_get_property;
  object_class->set_property = hdy_carousel_set_property;
  widget_class->destroy = hdy_carousel_destroy;
  widget_class->direction_changed = hdy_carousel_direction_changed;
  container_class->add = hdy_carousel_add;
  container_class->remove = hdy_carousel_remove;
  container_class->forall = hdy_carousel_forall;

  /**
   * HdyCarousel:n-pages:
   *
   * The number of pages in a #HdyCarousel
   *
   * Since: 1.0
   */
  props[PROP_N_PAGES] =
    g_param_spec_uint ("n-pages",
                       _("Number of pages"),
                       _("Number of pages"),
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:position:
   *
   * Current scrolling position, unitless. 1 matches 1 page. Use
   * hdy_carousel_scroll_to() for changing it.
   *
   * Since: 1.0
   */
  props[PROP_POSITION] =
    g_param_spec_double ("position",
                         _("Position"),
                         _("Current scrolling position"),
                         0,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:interactive:
   *
   * Whether @self can be navigated. This can be used to temporarily disable
   * a #HdyCarousel to only allow navigating it in a certain state.
   *
   * Since: 1.0
   */
  props[PROP_INTERACTIVE] =
    g_param_spec_boolean ("interactive",
                          _("Interactive"),
                          _("Whether the widget can be swiped"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:indicator-style:
   *
   * The style of page indicators. Depending on orientation, they are displayed
   * below or besides the pages. If the pages are meant to be centered,
   * #HdyCarousel:center-content can be used to compensate for that.
   *
   * Since: 1.0
   */
  props[PROP_INDICATOR_STYLE] =
    g_param_spec_enum ("indicator-style",
                       _("Indicator style"),
                       _("Page indicator style"),
                       HDY_TYPE_CAROUSEL_INDICATOR_STYLE,
                       HDY_CAROUSEL_INDICATOR_STYLE_NONE,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:indicator-spacing:
   *
   * Spacing between content and page indicators. Does nothing if
   * #HdyCarousel:indicator-style is @HDY_CAROUSEL_INDICATOR_STYLE_NONE.
   *
   * Since: 1.0
   */
  props[PROP_INDICATOR_SPACING] =
    g_param_spec_uint ("indicator-spacing",
                       _("Indicator spacing"),
                       _("Spacing between content and indicators"),
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:center-content:
   *
   * Whether the #HdyCarousel is centering pages. If
   * #HdyCarousel:indicator-style is @HDY_CAROUSEL_INDICATOR_STYLE_NONE,
   * centering does nothing, otherwise it adds whitespace to the left or above
   * the pages to compensate for the indicators.
   *
   * Since: 1.0
   */
  props[PROP_CENTER_CONTENT] =
    g_param_spec_boolean ("center-content",
                          _("Center content"),
                          _("Whether to center pages to compensate for indicators"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:spacing:
   *
   * Spacing between pages in pixels.
   *
   * Since: 1.0
   */
  props[PROP_SPACING] =
    g_param_spec_uint ("spacing",
                       _("Spacing"),
                       _("Spacing between pages"),
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:animation-duration:
   *
   * Animation duration in milliseconds, used by hdy_carousel_scroll_to().
   *
   * Since: 1.0
   */
  props[PROP_ANIMATION_DURATION] =
    g_param_spec_uint ("animation-duration",
                       _("Animation duration"),
                       _("Default animation duration"),
                       0, G_MAXUINT, DEFAULT_DURATION,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:allow-mouse-drag:
   *
   * Sets whether the #HdyCarousel can be dragged with mouse pointer. If the
   * value is %FALSE, dragging is only available on touch.
   *
   * This should usually be %FALSE.
   *
   * Since: 1.0
   */
  props[PROP_ALLOW_MOUSE_DRAG] =
    g_param_spec_boolean ("allow-mouse-drag",
                          _("Allow mouse drag"),
                          _("Whether to allow dragging with mouse pointer"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * HdyCarousel::page-changed:
   * @self: The #HdyCarousel instance
   * @index: Current page
   *
   * This signal is emitted after a page has been changed. This can be used to
   * implement "infinite scrolling" by connecting to this signal and amending
   * the pages.
   *
   * Since: 1.0
   */
  signals[SIGNAL_PAGE_CHANGED] =
    g_signal_new ("page-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_UINT);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/sm/puri/handy/ui/hdy-carousel.ui");
  gtk_widget_class_bind_template_child (widget_class, HdyCarousel, box);
  gtk_widget_class_bind_template_child (widget_class, HdyCarousel, empty_box);
  gtk_widget_class_bind_template_child (widget_class, HdyCarousel, scrolling_box);
  gtk_widget_class_bind_template_child (widget_class, HdyCarousel, indicators);
  gtk_widget_class_bind_template_callback (widget_class, draw_indicators_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_n_pages_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_position_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_spacing_cb);
  gtk_widget_class_bind_template_callback (widget_class, animation_stopped_cb);

  gtk_widget_class_set_css_name (widget_class, "carousel");
}

static void
hdy_carousel_init (HdyCarousel *self)
{
  g_type_ensure (HDY_TYPE_CAROUSEL_BOX);
  gtk_widget_init_template (GTK_WIDGET (self));

  self->animation_duration = DEFAULT_DURATION;

  self->tracker = hdy_swipe_tracker_new (HDY_SWIPEABLE (self));
  self->can_scroll = TRUE;

  /*
   * HACK: GTK3 has no other way to get events on capture phase.
   * This is a reimplementation of _gtk_widget_set_captured_event_handler(),
   * which is private. In GTK4 it can be replaced with GtkEventControllerLegacy
   * with capture propagation phase
   */
  g_object_set_data (G_OBJECT (self), "captured-event-handler", captured_event_cb);
}

/**
 * hdy_carousel_new:
 *
 * Create a new #HdyCarousel widget.
 *
 * Returns: The newly created #HdyCarousel widget
 *
 * Since: 1.0
 */
HdyCarousel *
hdy_carousel_new (void)
{
  return g_object_new (HDY_TYPE_CAROUSEL, NULL);
}

/**
 * hdy_carousel_prepend:
 * @self: a #HdyCarousel
 * @child: a widget to add
 *
 * Prepends @child to @self
 *
 * Since: 1.0
 */
void
hdy_carousel_prepend (HdyCarousel *self,
                      GtkWidget   *widget)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  hdy_carousel_box_insert (self->scrolling_box, widget, 0);
}

/**
 * hdy_carousel_insert:
 * @self: a #HdyCarousel
 * @child: a widget to add
 * @position: the position to insert @child in.
 *
 * Inserts @child into @self at position @position.
 *
 * If position is -1, or larger than the number of pages,
 * @child will be appended to the end.
 *
 * Since: 1.0
 */
void
hdy_carousel_insert (HdyCarousel *self,
                     GtkWidget   *widget,
                     gint         position)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  hdy_carousel_box_insert (self->scrolling_box, widget, position);
}
/**
 * hdy_carousel_reorder:
 * @self: a #HdyCarousel
 * @child: a widget to add
 * @position: the position to move @child to.
 *
 * Moves @child into position @position.
 *
 * If position is -1, or larger than the number of pages, @child will be moved
 * to the end.
 *
 * Since: 1.0
 */
void
hdy_carousel_reorder (HdyCarousel *self,
                      GtkWidget   *child,
                      gint         position)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  hdy_carousel_box_reorder (self->scrolling_box, child, position);
}

/**
 * hdy_carousel_scroll_to:
 * @self: a #HdyCarousel
 * @widget: a child of @self
 *
 * Scrolls to @widget position with an animation.
 * #HdyCarousel:animation-duration property can be used for controlling the
 * duration.
 *
 * Since: 1.0
 */
void
hdy_carousel_scroll_to (HdyCarousel *self,
                        GtkWidget   *widget)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  hdy_carousel_scroll_to_full (self, widget, self->animation_duration);
}

/**
 * hdy_carousel_scroll_to_full:
 * @self: a #HdyCarousel
 * @widget: a child of @self
 * @duration: animation duration in milliseconds
 *
 * Scrolls to @widget position with an animation.
 *
 * Since: 1.0
 */
void
hdy_carousel_scroll_to_full (HdyCarousel *self,
                             GtkWidget   *widget,
                             gint64       duration)
{
  GList *children;
  gint n;

  g_return_if_fail (HDY_IS_CAROUSEL (self));

  children = gtk_container_get_children (GTK_CONTAINER (self->scrolling_box));
  n = g_list_index (children, widget);
  g_list_free (children);

  hdy_carousel_box_scroll_to (self->scrolling_box, widget,
                               duration);
  hdy_swipeable_emit_switch_child (HDY_SWIPEABLE (self), n, duration);
}

/**
 * hdy_carousel_get_n_pages:
 * @self: a #HdyCarousel
 *
 * Gets the number of pages in @self.
 *
 * Returns: The number of pages in @self
 *
 * Since: 1.0
 */
guint
hdy_carousel_get_n_pages (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), 0);

  return hdy_carousel_box_get_n_pages (self->scrolling_box);
}

/**
 * hdy_carousel_get_position:
 * @self: a #HdyCarousel
 *
 * Gets current scroll position in @self. It's unitless, 1 matches 1 page.
 *
 * Returns: The scroll position
 *
 * Since: 1.0
 */
gdouble
hdy_carousel_get_position (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), 0);

  return hdy_carousel_box_get_position (self->scrolling_box);
}

/**
 * hdy_carousel_get_interactive
 * @self: a #HdyCarousel
 *
 * Gets whether @self can be navigated.
 *
 * Returns: %TRUE if @self can be swiped
 *
 * Since: 1.0
 */
gboolean
hdy_carousel_get_interactive (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), FALSE);

  return hdy_swipe_tracker_get_enabled (self->tracker);
}

/**
 * hdy_carousel_set_interactive
 * @self: a #HdyCarousel
 * @interactive: whether @self can be swiped.
 *
 * Sets whether @self can be navigated. This can be used to temporarily disable
 * a #HdyCarousel to only allow swiping in a certain state.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_interactive (HdyCarousel *self,
                              gboolean     interactive)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  interactive = !!interactive;

  if (hdy_swipe_tracker_get_enabled (self->tracker) == interactive)
    return;

  hdy_swipe_tracker_set_enabled (self->tracker, interactive);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERACTIVE]);
}

/**
 * hdy_carousel_get_indicator_style
 * @self: a #HdyCarousel
 *
 * Gets the current page indicator style.
 *
 * Returns: the current indicator style
 *
 * Since: 1.0
 */
HdyCarouselIndicatorStyle
hdy_carousel_get_indicator_style (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), FALSE);

  return self->indicator_style;
}

/**
 * hdy_carousel_set_indicator_style
 * @self: a #HdyCarousel
 * @style: indicator style to use
 *
 * Sets style of page indicators. Depending on orientation, they are displayed
 * below or besides the pages. If the pages are meant to be centered,
 * #HdyCarousel:center-content can be used to compensate for that.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_indicator_style (HdyCarousel               *self,
                                  HdyCarouselIndicatorStyle  style)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  if (self->indicator_style == style)
    return;

  self->indicator_style = style;
  update_indicators (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INDICATOR_STYLE]);
}

/**
 * hdy_carousel_get_indicator_spacing:
 * @self: a #HdyCarousel
 *
 * Gets spacing between content and page indicators.
 *
 * Returns: Spacing between content and indicators
 *
 * Since: 1.0
 */
guint
hdy_carousel_get_indicator_spacing (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), 0);

  return self->indicator_spacing;
}

/**
 * hdy_carousel_set_indicator_spacing:
 * @self: a #HdyCarousel
 * @spacing: the new spacing value
 *
 * Sets spacing between content and page indicators. Does nothing if
 * #HdyCarousel:indicator-style is @HDY_CAROUSEL_INDICATOR_STYLE_NONE.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_indicator_spacing (HdyCarousel *self,
                                    guint        spacing)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  if (self->indicator_spacing == spacing)
    return;

  self->indicator_spacing = spacing;
  gtk_box_set_spacing (self->box, spacing);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INDICATOR_SPACING]);
}

/**
 * hdy_carousel_get_center_content
 * @self: a #HdyCarousel
 *
 * Sets whether @self is centering pages.
 *
 * Returns: %TRUE if @self is centering pages
 *
 * Since: 1.0
 */
gboolean
hdy_carousel_get_center_content (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), FALSE);

  return self->center_content;
}

/**
 * hdy_carousel_set_center_content
 * @self: a #HdyCarousel
 * @center_content: whether @self should center contents
 *
 * Sets whether @self is centering content. If #HdyCarousel:indicator-style is
 * @HDY_CAROUSEL_INDICATOR_STYLE_NONE, centering does nothing, otherwise it
 * adds whitespace to the left or above the pages to compensate for the
 * indicators.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_center_content (HdyCarousel *self,
                                 gboolean     center_content)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  center_content = !!center_content;

  if (self->center_content == center_content)
    return;

  self->center_content = center_content;
  update_indicators (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_CENTER_CONTENT]);
}

/**
 * hdy_carousel_get_spacing:
 * @self: a #HdyCarousel
 *
 * Gets spacing between pages in pixels.
 *
 * Returns: Spacing between pages
 *
 * Since: 1.0
 */
guint
hdy_carousel_get_spacing (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), 0);

  return hdy_carousel_box_get_spacing (self->scrolling_box);
}

/**
 * hdy_carousel_set_spacing:
 * @self: a #HdyCarousel
 * @spacing: the new spacing value
 *
 * Sets spacing between pages in pixels.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_spacing (HdyCarousel *self,
                          guint        spacing)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  hdy_carousel_box_set_spacing (self->scrolling_box, spacing);
}

/**
 * hdy_carousel_get_animation_duration:
 * @self: a #HdyCarousel
 *
 * Gets animation duration used by hdy_carousel_scroll_to().
 *
 * Returns: Animation duration in milliseconds
 *
 * Since: 1.0
 */
guint
hdy_carousel_get_animation_duration (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), 0);

  return self->animation_duration;
}

/**
 * hdy_carousel_set_animation_duration:
 * @self: a #HdyCarousel
 * @duration: animation duration in milliseconds
 *
 * Sets animation duration used by hdy_carousel_scroll_to().
 *
 * Since: 1.0
 */
void
hdy_carousel_set_animation_duration (HdyCarousel *self,
                                     guint        duration)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  if (self->animation_duration == duration)
    return;

  self->animation_duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ANIMATION_DURATION]);
}

/**
 * hdy_carousel_get_allow_mouse_drag:
 * @self: a #HdyCarousel
 *
 * Sets whether @self can be dragged with mouse pointer
 *
 * Returns: %TRUE if @self can be dragged with mouse
 *
 * Since: 1.0
 */
gboolean
hdy_carousel_get_allow_mouse_drag (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), FALSE);

  return hdy_swipe_tracker_get_allow_mouse_drag (self->tracker);
}

/**
 * hdy_carousel_set_allow_mouse_drag:
 * @self: a #HdyCarousel
 * @allow_mouse_drag: whether @self can be dragged with mouse pointer
 *
 * Sets whether @self can be dragged with mouse pointer. If @allow_mouse_drag
 * is %FALSE, dragging is only available on touch.
 *
 * This should usually be %FALSE.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_allow_mouse_drag (HdyCarousel *self,
                                   gboolean     allow_mouse_drag)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  allow_mouse_drag = !!allow_mouse_drag;

  if (hdy_carousel_get_allow_mouse_drag (self) == allow_mouse_drag)
    return;

  hdy_swipe_tracker_set_allow_mouse_drag (self->tracker, allow_mouse_drag);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_MOUSE_DRAG]);
}
