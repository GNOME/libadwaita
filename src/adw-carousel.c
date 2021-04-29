/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-carousel.h"

#include "adw-animation-private.h"
#include "adw-carousel-box-private.h"
#include "adw-navigation-direction.h"
#include "adw-swipe-tracker.h"
#include "adw-swipeable.h"

#include <math.h>

#define DEFAULT_DURATION 250

/**
 * SECTION:adw-carousel
 * @short_description: A paginated scrolling widget.
 * @title: AdwCarousel
 * @See_also: #AdwCarouselIndicatorDots, #AdwCarouselIndicatorLines
 *
 * The #AdwCarousel widget can be used to display a set of pages with
 * swipe-based navigation between them.
 *
 * # CSS nodes
 *
 * #AdwCarousel has a single CSS node with name carousel.
 *
 * Since: 1.0
 */

struct _AdwCarousel
{
  GtkWidget parent_instance;

  AdwCarouselBox *scrolling_box;

  AdwSwipeTracker *tracker;

  gboolean allow_scroll_wheel;

  GtkOrientation orientation;
  guint animation_duration;

  gulong scroll_timeout_id;
  gboolean can_scroll;
};

static void adw_carousel_buildable_init (GtkBuildableIface *iface);
static void adw_carousel_swipeable_init (AdwSwipeableInterface *iface);

G_DEFINE_TYPE_WITH_CODE (AdwCarousel, adw_carousel, GTK_TYPE_WIDGET,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL)
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_BUILDABLE, adw_carousel_buildable_init)
                         G_IMPLEMENT_INTERFACE (ADW_TYPE_SWIPEABLE, adw_carousel_swipeable_init))

static GtkBuildableIface *parent_buildable_iface;

enum {
  PROP_0,
  PROP_N_PAGES,
  PROP_POSITION,
  PROP_INTERACTIVE,
  PROP_SPACING,
  PROP_ANIMATION_DURATION,
  PROP_ALLOW_MOUSE_DRAG,
  PROP_ALLOW_SCROLL_WHEEL,
  PROP_ALLOW_LONG_SWIPES,
  PROP_REVEAL_DURATION,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_REVEAL_DURATION + 1,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_PAGE_CHANGED,
  SIGNAL_LAST_SIGNAL,
};
static guint signals[SIGNAL_LAST_SIGNAL];


static void
adw_carousel_switch_child (AdwSwipeable *swipeable,
                           guint         index,
                           gint64        duration)
{
  AdwCarousel *self = ADW_CAROUSEL (swipeable);
  GtkWidget *child;

  child = adw_carousel_box_get_nth_child (self->scrolling_box, index);

  adw_carousel_box_scroll_to (self->scrolling_box, child, duration);
}

static void
begin_swipe_cb (AdwSwipeTracker        *tracker,
                AdwNavigationDirection  direction,
                gboolean                direct,
                AdwCarousel            *self)
{
  adw_carousel_box_stop_animation (self->scrolling_box);
}

static void
update_swipe_cb (AdwSwipeTracker *tracker,
                 double           progress,
                 AdwCarousel     *self)
{
  adw_carousel_box_set_position (self->scrolling_box, progress);
}

static void
end_swipe_cb (AdwSwipeTracker *tracker,
              gint64           duration,
              double           to,
              AdwCarousel     *self)
{
  GtkWidget *child;

  child = adw_carousel_box_get_page_at_position (self->scrolling_box, to);
  adw_carousel_box_scroll_to (self->scrolling_box, child, duration);
}

static AdwSwipeTracker *
adw_carousel_get_swipe_tracker (AdwSwipeable *swipeable)
{
  AdwCarousel *self = ADW_CAROUSEL (swipeable);

  return self->tracker;
}

static double
adw_carousel_get_distance (AdwSwipeable *swipeable)
{
  AdwCarousel *self = ADW_CAROUSEL (swipeable);

  return adw_carousel_box_get_distance (self->scrolling_box);
}

static double *
adw_carousel_get_snap_points (AdwSwipeable *swipeable,
                              int          *n_snap_points)
{
  AdwCarousel *self = ADW_CAROUSEL (swipeable);

  return adw_carousel_box_get_snap_points (self->scrolling_box,
                                           n_snap_points);
}

static double
adw_carousel_get_progress (AdwSwipeable *swipeable)
{
  AdwCarousel *self = ADW_CAROUSEL (swipeable);

  return adw_carousel_get_position (self);
}

static double
adw_carousel_get_cancel_progress (AdwSwipeable *swipeable)
{
  AdwCarousel *self = ADW_CAROUSEL (swipeable);

  return adw_carousel_box_get_closest_snap_point (self->scrolling_box);
}

static void
notify_n_pages_cb (AdwCarousel *self,
                   GParamSpec  *spec,
                   GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static void
notify_position_cb (AdwCarousel *self,
                    GParamSpec  *spec,
                    GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POSITION]);
}

static void
notify_spacing_cb (AdwCarousel *self,
                   GParamSpec  *spec,
                   GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPACING]);
}

static void
notify_reveal_duration_cb (AdwCarousel *self,
                           GParamSpec  *spec,
                           GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_DURATION]);
}

static void
animation_stopped_cb (AdwCarousel    *self,
                      AdwCarouselBox *box)
{
  int index;

  index = adw_carousel_box_get_current_page_index (self->scrolling_box);

  g_signal_emit (self, signals[SIGNAL_PAGE_CHANGED], 0, index);
}

static void
position_shifted_cb (AdwCarousel    *self,
                     double          delta,
                     AdwCarouselBox *box)
{
  adw_swipe_tracker_shift_position (self->tracker, delta);
}

/* Copied from GtkOrientable. Orientable widgets are supposed
 * to do this manually via a private GTK function. */
static void
set_orientable_style_classes (GtkOrientable *orientable)
{
  GtkOrientation orientation = gtk_orientable_get_orientation (orientable);
  GtkWidget *widget = GTK_WIDGET (orientable);

  if (orientation == GTK_ORIENTATION_HORIZONTAL) {
    gtk_widget_add_css_class (widget, "horizontal");
    gtk_widget_remove_css_class (widget, "vertical");
  } else {
    gtk_widget_add_css_class (widget, "vertical");
    gtk_widget_remove_css_class (widget, "horizontal");
  }
}

static void
update_orientation (AdwCarousel *self)
{
  gboolean reversed;

  if (!self->scrolling_box)
    return;

  reversed = self->orientation == GTK_ORIENTATION_HORIZONTAL &&
    gtk_widget_get_direction (GTK_WIDGET (self)) == GTK_TEXT_DIR_RTL;

  g_object_set (self->scrolling_box, "orientation", self->orientation, NULL);
  g_object_set (self->tracker, "orientation", self->orientation,
                "reversed", reversed, NULL);

  set_orientable_style_classes (GTK_ORIENTABLE (self));
  set_orientable_style_classes (GTK_ORIENTABLE (self->scrolling_box));
}

static gboolean
scroll_timeout_cb (AdwCarousel *self)
{
  self->can_scroll = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
scroll_cb (AdwCarousel              *self,
           double                    dx,
           double                    dy,
           GtkEventControllerScroll *controller)
{
  GdkDevice *source_device;
  GdkInputSource input_source;
  int index;
  gboolean allow_vertical;
  GtkOrientation orientation;
  guint duration;

  if (!self->allow_scroll_wheel)
    return GDK_EVENT_PROPAGATE;

  if (!self->can_scroll)
    return GDK_EVENT_PROPAGATE;

  if (!adw_carousel_get_interactive (self))
    return GDK_EVENT_PROPAGATE;

  source_device = gtk_event_controller_get_current_event_device (GTK_EVENT_CONTROLLER (controller));
  input_source = gdk_device_get_source (source_device);
  if (input_source == GDK_SOURCE_TOUCHPAD)
    return GDK_EVENT_PROPAGATE;

  /* Mice often don't have easily accessible horizontal scrolling,
   * hence allow vertical mouse scrolling regardless of orientation */
  allow_vertical = (input_source == GDK_SOURCE_MOUSE);

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

  index += adw_carousel_box_get_current_page_index (self->scrolling_box);
  index = CLAMP (index, 0, (int) adw_carousel_get_n_pages (self) - 1);

  adw_carousel_scroll_to (self, adw_carousel_box_get_nth_child (self->scrolling_box, index));

  /* Don't allow the delay to go lower than 250ms */
  duration = MIN (self->animation_duration, DEFAULT_DURATION);

  self->can_scroll = FALSE;
  g_timeout_add (duration, (GSourceFunc) scroll_timeout_cb, self);

  return GDK_EVENT_STOP;
}

static void
adw_carousel_direction_changed (GtkWidget        *widget,
                                GtkTextDirection  previous_direction)
{
  AdwCarousel *self = ADW_CAROUSEL (widget);

  update_orientation (self);
}

static void
adw_carousel_constructed (GObject *object)
{
  AdwCarousel *self = (AdwCarousel *)object;

  update_orientation (self);

  G_OBJECT_CLASS (adw_carousel_parent_class)->constructed (object);
}

static void
adw_carousel_dispose (GObject *object)
{
  AdwCarousel *self = (AdwCarousel *)object;

  g_clear_object (&self->tracker);

  if (self->scroll_timeout_id != 0) {
    g_source_remove (self->scroll_timeout_id);
    self->scroll_timeout_id = 0;
  }

  gtk_widget_unparent (GTK_WIDGET (self->scrolling_box));

  G_OBJECT_CLASS (adw_carousel_parent_class)->dispose (object);
}

static void
adw_carousel_get_property (GObject    *object,
                           guint       prop_id,
                           GValue     *value,
                           GParamSpec *pspec)
{
  AdwCarousel *self = ADW_CAROUSEL (object);

  switch (prop_id) {
  case PROP_N_PAGES:
    g_value_set_uint (value, adw_carousel_get_n_pages (self));
    break;

  case PROP_POSITION:
    g_value_set_double (value, adw_carousel_get_position (self));
    break;

  case PROP_INTERACTIVE:
    g_value_set_boolean (value, adw_carousel_get_interactive (self));
    break;

  case PROP_SPACING:
    g_value_set_uint (value, adw_carousel_get_spacing (self));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    g_value_set_boolean (value, adw_carousel_get_allow_mouse_drag (self));
    break;

  case PROP_ALLOW_SCROLL_WHEEL:
    g_value_set_boolean (value, adw_carousel_get_allow_scroll_wheel (self));
    break;

  case PROP_ALLOW_LONG_SWIPES:
    g_value_set_boolean (value, adw_carousel_get_allow_long_swipes (self));
    break;

  case PROP_REVEAL_DURATION:
    g_value_set_uint (value, adw_carousel_get_reveal_duration (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  case PROP_ANIMATION_DURATION:
    g_value_set_uint (value, adw_carousel_get_animation_duration (self));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_carousel_set_property (GObject      *object,
                           guint         prop_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  AdwCarousel *self = ADW_CAROUSEL (object);

  switch (prop_id) {
  case PROP_INTERACTIVE:
    adw_carousel_set_interactive (self, g_value_get_boolean (value));
    break;

  case PROP_SPACING:
    adw_carousel_set_spacing (self, g_value_get_uint (value));
    break;

  case PROP_ANIMATION_DURATION:
    adw_carousel_set_animation_duration (self, g_value_get_uint (value));
    break;

  case PROP_REVEAL_DURATION:
    adw_carousel_set_reveal_duration (self, g_value_get_uint (value));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    adw_carousel_set_allow_mouse_drag (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_SCROLL_WHEEL:
    adw_carousel_set_allow_scroll_wheel (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_LONG_SWIPES:
    adw_carousel_set_allow_long_swipes (self, g_value_get_boolean (value));
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
adw_carousel_swipeable_init (AdwSwipeableInterface *iface)
{
  iface->switch_child = adw_carousel_switch_child;
  iface->get_swipe_tracker = adw_carousel_get_swipe_tracker;
  iface->get_distance = adw_carousel_get_distance;
  iface->get_snap_points = adw_carousel_get_snap_points;
  iface->get_progress = adw_carousel_get_progress;
  iface->get_cancel_progress = adw_carousel_get_cancel_progress;
}

static void
adw_carousel_class_init (AdwCarouselClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  object_class->constructed = adw_carousel_constructed;
  object_class->dispose = adw_carousel_dispose;
  object_class->get_property = adw_carousel_get_property;
  object_class->set_property = adw_carousel_set_property;
  widget_class->direction_changed = adw_carousel_direction_changed;

  /**
   * AdwCarousel:n-pages:
   *
   * The number of pages in a #AdwCarousel
   *
   * Since: 1.0
   */
  props[PROP_N_PAGES] =
    g_param_spec_uint ("n-pages",
                       "Number of pages",
                       "Number of pages",
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:position:
   *
   * Current scrolling position, unitless. 1 matches 1 page. Use
   * adw_carousel_scroll_to() for changing it.
   *
   * Since: 1.0
   */
  props[PROP_POSITION] =
    g_param_spec_double ("position",
                         "Position",
                         "Current scrolling position",
                         0,
                         G_MAXDOUBLE,
                         0,
                         G_PARAM_READABLE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:interactive:
   *
   * Whether the carousel can be navigated. This can be used to temporarily
   * disable a #AdwCarousel to only allow navigating it in a certain state.
   *
   * Since: 1.0
   */
  props[PROP_INTERACTIVE] =
    g_param_spec_boolean ("interactive",
                          "Interactive",
                          "Whether the widget can be swiped",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:spacing:
   *
   * Spacing between pages in pixels.
   *
   * Since: 1.0
   */
  props[PROP_SPACING] =
    g_param_spec_uint ("spacing",
                       "Spacing",
                       "Spacing between pages",
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:animation-duration:
   *
   * Animation duration in milliseconds, used by adw_carousel_scroll_to().
   *
   * Since: 1.0
   */
  props[PROP_ANIMATION_DURATION] =
    g_param_spec_uint ("animation-duration",
                       "Animation duration",
                       "Default animation duration",
                       0, G_MAXUINT, DEFAULT_DURATION,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:allow-mouse-drag:
   *
   * Sets whether the #AdwCarousel can be dragged with mouse pointer. If the
   * value is %FALSE, dragging is only available on touch.
   *
   * Since: 1.0
   */
  props[PROP_ALLOW_MOUSE_DRAG] =
    g_param_spec_boolean ("allow-mouse-drag",
                          "Allow mouse drag",
                          "Whether to allow dragging with mouse pointer",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:allow-scroll-wheel:
   *
   * Whether the widget will respond to scroll wheel events. If the value is
   * %FALSE, wheel events will be ignored.
   *
   * Since: 1.0
   */
  props[PROP_ALLOW_SCROLL_WHEEL] =
    g_param_spec_boolean ("allow-scroll-wheel",
                          "Allow scroll wheel",
                          "Whether the widget will respond to scroll wheel events",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:allow-long-swipes:
   *
   * Whether to allow swiping for more than one page at a time. If the value is
   * %FALSE, each swipe can only move to the adjacent pages.
   *
   * Since: 1.0
   */
  props[PROP_ALLOW_LONG_SWIPES] =
    g_param_spec_boolean ("allow-long-swipes",
                          "Allow long swipes",
                          "Whether to allow swiping for more than one page at a time",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwCarousel:reveal-duration:
   *
   * Page reveal duration in milliseconds.
   *
   * Since: 1.0
   */
  props[PROP_REVEAL_DURATION] =
    g_param_spec_uint ("reveal-duration",
                       "Reveal duration",
                       "Page reveal duration",
                       0,
                       G_MAXUINT,
                       0,
                       G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwCarousel::page-changed:
   * @self: The #AdwCarousel instance
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
                                               "/org/gnome/Adwaita/ui/adw-carousel.ui");
  gtk_widget_class_bind_template_child (widget_class, AdwCarousel, scrolling_box);
  gtk_widget_class_bind_template_callback (widget_class, scroll_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_n_pages_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_position_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_spacing_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_reveal_duration_cb);
  gtk_widget_class_bind_template_callback (widget_class, animation_stopped_cb);
  gtk_widget_class_bind_template_callback (widget_class, position_shifted_cb);

  gtk_widget_class_set_layout_manager_type (widget_class, GTK_TYPE_BIN_LAYOUT);
  gtk_widget_class_set_css_name (widget_class, "carousel");
}

static void
adw_carousel_init (AdwCarousel *self)
{
  self->allow_scroll_wheel = TRUE;

  g_type_ensure (ADW_TYPE_CAROUSEL_BOX);
  gtk_widget_init_template (GTK_WIDGET (self));

  gtk_widget_set_overflow (GTK_WIDGET (self), GTK_OVERFLOW_HIDDEN);

  self->animation_duration = DEFAULT_DURATION;

  self->tracker = adw_swipe_tracker_new (ADW_SWIPEABLE (self));
  adw_swipe_tracker_set_allow_mouse_drag (self->tracker, TRUE);

  g_signal_connect_object (self->tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self, 0);

  self->can_scroll = TRUE;
}

static void
adw_carousel_buildable_add_child (GtkBuildable *buildable,
                                  GtkBuilder   *builder,
                                  GObject      *child,
                                  const char   *type)
{
  AdwCarousel *self = ADW_CAROUSEL (buildable);

  if (GTK_IS_WIDGET (child))
    if (!self->scrolling_box)
      gtk_widget_set_parent (GTK_WIDGET (child), GTK_WIDGET (buildable));
    else
      adw_carousel_append (ADW_CAROUSEL (buildable), GTK_WIDGET (child));
  else
    parent_buildable_iface->add_child (buildable, builder, child, type);
}

static void
adw_carousel_buildable_init (GtkBuildableIface *iface)
{
  parent_buildable_iface = g_type_interface_peek_parent (iface);

  iface->add_child = adw_carousel_buildable_add_child;
}

/**
 * adw_carousel_new:
 *
 * Create a new #AdwCarousel widget.
 *
 * Returns: The newly created #AdwCarousel widget
 *
 * Since: 1.0
 */
GtkWidget *
adw_carousel_new (void)
{
  return g_object_new (ADW_TYPE_CAROUSEL, NULL);
}

/**
 * adw_carousel_prepend:
 * @self: a #AdwCarousel
 * @child: a widget to add
 *
 * Prepends @child to @self
 *
 * Since: 1.0
 */
void
adw_carousel_prepend (AdwCarousel *self,
                      GtkWidget   *widget)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  adw_carousel_box_insert (self->scrolling_box, widget, 0);
}

/**
 * adw_carousel_append:
 * @self: a #AdwCarousel
 * @child: a widget to add
 *
 * Appends @child to @self
 *
 * Since: 1.0
 */
void
adw_carousel_append (AdwCarousel *self,
                     GtkWidget   *widget)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  adw_carousel_box_insert (self->scrolling_box, widget, -1);
}

/**
 * adw_carousel_insert:
 * @self: a #AdwCarousel
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
adw_carousel_insert (AdwCarousel *self,
                     GtkWidget   *widget,
                     int          position)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  adw_carousel_box_insert (self->scrolling_box, widget, position);
}
/**
 * adw_carousel_reorder:
 * @self: a #AdwCarousel
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
adw_carousel_reorder (AdwCarousel *self,
                      GtkWidget   *child,
                      int          position)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  adw_carousel_box_reorder (self->scrolling_box, child, position);
}

/**
 * adw_carousel_remove:
 * @self: a #AdwCarousel
 * @child: a widget to remove
 *
 * Removes @child from @self
 *
 * Since: 1.0
 */
void
adw_carousel_remove (AdwCarousel *self,
                     GtkWidget   *child)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));
  g_return_if_fail (GTK_IS_WIDGET (child));

  adw_carousel_box_remove (self->scrolling_box, child);
}

/**
 * adw_carousel_scroll_to:
 * @self: a #AdwCarousel
 * @widget: a child of @self
 *
 * Scrolls to @widget position with an animation.
 * #AdwCarousel:animation-duration property can be used for controlling the
 * duration.
 *
 * Since: 1.0
 */
void
adw_carousel_scroll_to (AdwCarousel *self,
                        GtkWidget   *widget)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  adw_carousel_scroll_to_full (self, widget, self->animation_duration);
}

/**
 * adw_carousel_scroll_to_full:
 * @self: a #AdwCarousel
 * @widget: a child of @self
 * @duration: animation duration in milliseconds
 *
 * Scrolls to @widget position with an animation.
 *
 * Since: 1.0
 */
void
adw_carousel_scroll_to_full (AdwCarousel *self,
                             GtkWidget   *widget,
                             gint64       duration)
{
  int index;

  g_return_if_fail (ADW_IS_CAROUSEL (self));
  g_return_if_fail (GTK_IS_WIDGET (widget));

  index = adw_carousel_box_get_page_index (self->scrolling_box, widget);
  adw_carousel_box_scroll_to (self->scrolling_box, widget, duration);
  adw_swipeable_emit_child_switched (ADW_SWIPEABLE (self), index, duration);
}

/**
 * adw_carousel_get_nth_page:
 * @self: a #AdwCarousel
 * @n: index of the page
 *
 * Gets the page at position @n.
 *
 * Returns: (transfer none): the page
 *
 * Since: 1.0
 */
GtkWidget *
adw_carousel_get_nth_page (AdwCarousel *self,
                           guint        n)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), 0);

  return adw_carousel_box_get_nth_child (self->scrolling_box, n);
}

/**
 * adw_carousel_get_n_pages:
 * @self: a #AdwCarousel
 *
 * Gets the number of pages in @self.
 *
 * Returns: The number of pages in @self
 *
 * Since: 1.0
 */
guint
adw_carousel_get_n_pages (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), 0);

  return adw_carousel_box_get_n_pages (self->scrolling_box);
}

/**
 * adw_carousel_get_position:
 * @self: a #AdwCarousel
 *
 * Gets current scroll position in @self. It's unitless, 1 matches 1 page.
 *
 * Returns: The scroll position
 *
 * Since: 1.0
 */
double
adw_carousel_get_position (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), 0);

  return adw_carousel_box_get_position (self->scrolling_box);
}

/**
 * adw_carousel_get_interactive
 * @self: a #AdwCarousel
 *
 * Gets whether @self can be navigated.
 *
 * Returns: %TRUE if @self can be swiped
 *
 * Since: 1.0
 */
gboolean
adw_carousel_get_interactive (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), FALSE);

  return adw_swipe_tracker_get_enabled (self->tracker);
}

/**
 * adw_carousel_set_interactive
 * @self: a #AdwCarousel
 * @interactive: whether @self can be swiped.
 *
 * Sets whether @self can be navigated. This can be used to temporarily disable
 * a #AdwCarousel to only allow swiping in a certain state.
 *
 * Since: 1.0
 */
void
adw_carousel_set_interactive (AdwCarousel *self,
                              gboolean     interactive)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  interactive = !!interactive;

  if (adw_swipe_tracker_get_enabled (self->tracker) == interactive)
    return;

  adw_swipe_tracker_set_enabled (self->tracker, interactive);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_INTERACTIVE]);
}

/**
 * adw_carousel_get_spacing:
 * @self: a #AdwCarousel
 *
 * Gets spacing between pages in pixels.
 *
 * Returns: Spacing between pages
 *
 * Since: 1.0
 */
guint
adw_carousel_get_spacing (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), 0);

  return adw_carousel_box_get_spacing (self->scrolling_box);
}

/**
 * adw_carousel_set_spacing:
 * @self: a #AdwCarousel
 * @spacing: the new spacing value
 *
 * Sets spacing between pages in pixels.
 *
 * Since: 1.0
 */
void
adw_carousel_set_spacing (AdwCarousel *self,
                          guint        spacing)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  adw_carousel_box_set_spacing (self->scrolling_box, spacing);
}

/**
 * adw_carousel_get_animation_duration:
 * @self: a #AdwCarousel
 *
 * Gets animation duration used by adw_carousel_scroll_to().
 *
 * Returns: Animation duration in milliseconds
 *
 * Since: 1.0
 */
guint
adw_carousel_get_animation_duration (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), 0);

  return self->animation_duration;
}

/**
 * adw_carousel_set_animation_duration:
 * @self: a #AdwCarousel
 * @duration: animation duration in milliseconds
 *
 * Sets animation duration used by adw_carousel_scroll_to().
 *
 * Since: 1.0
 */
void
adw_carousel_set_animation_duration (AdwCarousel *self,
                                     guint        duration)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  if (self->animation_duration == duration)
    return;

  self->animation_duration = duration;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ANIMATION_DURATION]);
}

/**
 * adw_carousel_get_allow_mouse_drag:
 * @self: a #AdwCarousel
 *
 * Sets whether @self can be dragged with mouse pointer
 *
 * Returns: %TRUE if @self can be dragged with mouse
 *
 * Since: 1.0
 */
gboolean
adw_carousel_get_allow_mouse_drag (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), FALSE);

  return adw_swipe_tracker_get_allow_mouse_drag (self->tracker);
}

/**
 * adw_carousel_set_allow_mouse_drag:
 * @self: a #AdwCarousel
 * @allow_mouse_drag: whether @self can be dragged with mouse pointer
 *
 * Sets whether @self can be dragged with mouse pointer. If @allow_mouse_drag
 * is %FALSE, dragging is only available on touch.
 *
 * Since: 1.0
 */
void
adw_carousel_set_allow_mouse_drag (AdwCarousel *self,
                                   gboolean     allow_mouse_drag)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  allow_mouse_drag = !!allow_mouse_drag;

  if (adw_carousel_get_allow_mouse_drag (self) == allow_mouse_drag)
    return;

  adw_swipe_tracker_set_allow_mouse_drag (self->tracker, allow_mouse_drag);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_MOUSE_DRAG]);
}

/**
 * adw_carousel_get_allow_scroll_wheel:
 * @self: a #AdwCarousel
 *
 * Gets whether @self will respond to scroll wheel events.
 *
 * Returns: %TRUE if @self will respond to scroll wheel events
 *
 * Since: 1.0
 */
gboolean
adw_carousel_get_allow_scroll_wheel (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), FALSE);

  return self->allow_scroll_wheel;
}

/**
 * adw_carousel_set_allow_scroll_wheel:
 * @self: a #AdwCarousel
 * @allow_scroll_wheel: whether @self will respond to scroll wheel events.
 *
 * Sets whether @self will respond to scroll wheel events. If the value is
 * %FALSE, wheel events will be ignored.
 *
 * Since: 1.0
 */
void
adw_carousel_set_allow_scroll_wheel (AdwCarousel *self,
                                     gboolean     allow_scroll_wheel)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  allow_scroll_wheel = !!allow_scroll_wheel;

  if (self->allow_scroll_wheel == allow_scroll_wheel)
    return;

  self->allow_scroll_wheel = allow_scroll_wheel;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_SCROLL_WHEEL]);
}

/**
 * adw_carousel_get_allow_long_swipes:
 * @self: a #AdwCarousel
 *
 * Whether to allow swiping for more than one page at a time. If the value is
 * %FALSE, each swipe can only move to the adjacent pages.
 *
 * Returns: %TRUE if long swipes are allowed, %FALSE otherwise
 *
 * Since: 1.0
 */
gboolean
adw_carousel_get_allow_long_swipes (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), FALSE);

  return adw_swipe_tracker_get_allow_long_swipes (self->tracker);
}

/**
 * adw_carousel_set_allow_long_swipes:
 * @self: a #AdwCarousel
 * @allow_long_swipes: whether to allow long swipes
 *
 * Sets whether to allow swiping for more than one page at a time. If the value
 * is %FALSE, each swipe can only move to the adjacent pages.
 *
 * Since: 1.0
 */
void
adw_carousel_set_allow_long_swipes (AdwCarousel *self,
                                    gboolean     allow_long_swipes)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  allow_long_swipes = !!allow_long_swipes;

  if (adw_swipe_tracker_get_allow_long_swipes (self->tracker) == allow_long_swipes)
    return;

  adw_swipe_tracker_set_allow_long_swipes (self->tracker, allow_long_swipes);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_LONG_SWIPES]);
}

/**
 * adw_carousel_get_reveal_duration:
 * @self: a #AdwCarousel
 *
 * Gets duration of the animation used when adding or removing pages in
 * milliseconds.
 *
 * Returns: Page reveal duration
 *
 * Since: 1.0
 */
guint
adw_carousel_get_reveal_duration (AdwCarousel *self)
{
  g_return_val_if_fail (ADW_IS_CAROUSEL (self), 0);

  return adw_carousel_box_get_reveal_duration (self->scrolling_box);
}

/**
 * adw_carousel_set_reveal_duration:
 * @self: a #AdwCarousel
 * @reveal_duration: the new reveal duration value
 *
 * Sets duration of the animation used when adding or removing pages in
 * milliseconds.
 *
 * Since: 1.0
 */
void
adw_carousel_set_reveal_duration (AdwCarousel *self,
                                  guint        reveal_duration)
{
  g_return_if_fail (ADW_IS_CAROUSEL (self));

  adw_carousel_box_set_reveal_duration (self->scrolling_box, reveal_duration);
}
