/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-carousel.h"

#include "hdy-animation-private.h"
#include "hdy-carousel-box-private.h"
#include "hdy-navigation-direction.h"
#include "hdy-swipe-tracker.h"
#include "hdy-swipeable.h"

#include <math.h>

#define DEFAULT_DURATION 250

/**
 * SECTION:hdy-carousel
 * @short_description: A paginated scrolling widget.
 * @title: HdyCarousel
 * @See_also: #HdyCarouselIndicatorDots, #HdyCarouselIndicatorLines
 *
 * The #HdyCarousel widget can be used to display a set of pages with
 * swipe-based navigation between them.
 *
 * # CSS nodes
 *
 * #HdyCarousel has a single CSS node with name carousel.
 *
 * Since: 1.0
 */

struct _HdyCarousel
{
  GtkEventBox parent_instance;

  HdyCarouselBox *scrolling_box;

  HdySwipeTracker *tracker;

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
  PROP_SPACING,
  PROP_ANIMATION_DURATION,
  PROP_ALLOW_MOUSE_DRAG,
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
begin_swipe_cb (HdySwipeTracker        *tracker,
                HdyNavigationDirection  direction,
                gboolean                direct,
                HdyCarousel            *self)
{
  hdy_carousel_box_stop_animation (self->scrolling_box);
}

static void
update_swipe_cb (HdySwipeTracker *tracker,
                 gdouble          progress,
                 HdyCarousel     *self)
{
  hdy_carousel_box_set_position (self->scrolling_box, progress);
}

static void
end_swipe_cb (HdySwipeTracker *tracker,
              gint64           duration,
              gdouble          to,
              HdyCarousel     *self)
{
  GtkWidget *child;

  child = hdy_carousel_box_get_page_at_position (self->scrolling_box, to);
  hdy_carousel_box_scroll_to (self->scrolling_box, child, duration);
}

static HdySwipeTracker *
hdy_carousel_get_swipe_tracker (HdySwipeable *swipeable)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  return self->tracker;
}

static gdouble
hdy_carousel_get_distance (HdySwipeable *swipeable)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  return hdy_carousel_box_get_distance (self->scrolling_box);
}

static gdouble *
hdy_carousel_get_snap_points (HdySwipeable *swipeable,
                              gint         *n_snap_points)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  return hdy_carousel_box_get_snap_points (self->scrolling_box,
                                           n_snap_points);
}

static gdouble
hdy_carousel_get_progress (HdySwipeable *swipeable)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  return hdy_carousel_get_position (self);
}

static gdouble
hdy_carousel_get_cancel_progress (HdySwipeable *swipeable)
{
  HdyCarousel *self = HDY_CAROUSEL (swipeable);

  return hdy_carousel_box_get_closest_snap_point (self->scrolling_box);
}

static void
notify_n_pages_cb (HdyCarousel *self,
                   GParamSpec  *spec,
                   GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_N_PAGES]);
}

static void
notify_position_cb (HdyCarousel *self,
                    GParamSpec  *spec,
                    GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_POSITION]);
}

static void
notify_spacing_cb (HdyCarousel *self,
                   GParamSpec  *spec,
                   GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_SPACING]);
}

static void
notify_reveal_duration_cb (HdyCarousel *self,
                           GParamSpec  *spec,
                           GObject     *object)
{
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVEAL_DURATION]);
}

static void
animation_stopped_cb (HdyCarousel    *self,
                      HdyCarouselBox *box)
{
  gint index;

  index = hdy_carousel_box_get_current_page_index (self->scrolling_box);

  g_signal_emit (self, signals[SIGNAL_PAGE_CHANGED], 0, index);
}

static void
position_shifted_cb (HdyCarousel    *self,
                     gdouble         delta,
                     HdyCarouselBox *box)
{
  hdy_swipe_tracker_shift_position (self->tracker, delta);
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
scroll_timeout_cb (HdyCarousel *self)
{
  self->can_scroll = TRUE;
  return G_SOURCE_REMOVE;
}

static gboolean
scroll_event_cb (HdyCarousel *self,
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

  index += hdy_carousel_box_get_current_page_index (self->scrolling_box);
  index = CLAMP (index, 0, (gint) hdy_carousel_get_n_pages (self) - 1);

  hdy_carousel_scroll_to (self, hdy_carousel_box_get_nth_child (self->scrolling_box, index));

  /* Don't allow the delay to go lower than 250ms */
  duration = MIN (self->animation_duration, DEFAULT_DURATION);

  self->can_scroll = FALSE;
  g_timeout_add (duration, (GSourceFunc) scroll_timeout_cb, self);

  return GDK_EVENT_STOP;
}

static void
hdy_carousel_destroy (GtkWidget *widget)
{
  HdyCarousel *self = HDY_CAROUSEL (widget);

  if (self->scrolling_box) {
    gtk_widget_destroy (GTK_WIDGET (self->scrolling_box));
    self->scrolling_box = NULL;
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
    (* callback) (GTK_WIDGET (self->scrolling_box), callback_data);
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

  g_clear_object (&self->tracker);

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

  case PROP_SPACING:
    g_value_set_uint (value, hdy_carousel_get_spacing (self));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    g_value_set_boolean (value, hdy_carousel_get_allow_mouse_drag (self));
    break;

  case PROP_REVEAL_DURATION:
    g_value_set_uint (value, hdy_carousel_get_reveal_duration (self));
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

  case PROP_SPACING:
    hdy_carousel_set_spacing (self, g_value_get_uint (value));
    break;

  case PROP_ANIMATION_DURATION:
    hdy_carousel_set_animation_duration (self, g_value_get_uint (value));
    break;

  case PROP_REVEAL_DURATION:
    hdy_carousel_set_reveal_duration (self, g_value_get_uint (value));
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
  iface->get_swipe_tracker = hdy_carousel_get_swipe_tracker;
  iface->get_distance = hdy_carousel_get_distance;
  iface->get_snap_points = hdy_carousel_get_snap_points;
  iface->get_progress = hdy_carousel_get_progress;
  iface->get_cancel_progress = hdy_carousel_get_cancel_progress;
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
   * Whether the carousel can be navigated. This can be used to temporarily
   * disable a #HdyCarousel to only allow navigating it in a certain state.
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
   * Since: 1.0
   */
  props[PROP_ALLOW_MOUSE_DRAG] =
    g_param_spec_boolean ("allow-mouse-drag",
                          _("Allow mouse drag"),
                          _("Whether to allow dragging with mouse pointer"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdyCarousel:reveal-duration:
   *
   * Page reveal duration in milliseconds.
   *
   * Since: 1.0
   */
  props[PROP_REVEAL_DURATION] =
    g_param_spec_uint ("reveal-duration",
                       _("Reveal duration"),
                       _("Page reveal duration"),
                       0,
                       G_MAXUINT,
                       0,
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
  gtk_widget_class_bind_template_child (widget_class, HdyCarousel, scrolling_box);
  gtk_widget_class_bind_template_callback (widget_class, scroll_event_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_n_pages_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_position_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_spacing_cb);
  gtk_widget_class_bind_template_callback (widget_class, notify_reveal_duration_cb);
  gtk_widget_class_bind_template_callback (widget_class, animation_stopped_cb);
  gtk_widget_class_bind_template_callback (widget_class, position_shifted_cb);

  gtk_widget_class_set_css_name (widget_class, "carousel");
}

static void
hdy_carousel_init (HdyCarousel *self)
{
  g_type_ensure (HDY_TYPE_CAROUSEL_BOX);
  gtk_widget_init_template (GTK_WIDGET (self));

  self->animation_duration = DEFAULT_DURATION;

  self->tracker = hdy_swipe_tracker_new (HDY_SWIPEABLE (self));
  hdy_swipe_tracker_set_allow_mouse_drag (self->tracker, TRUE);

  g_signal_connect_object (self->tracker, "begin-swipe", G_CALLBACK (begin_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "update-swipe", G_CALLBACK (update_swipe_cb), self, 0);
  g_signal_connect_object (self->tracker, "end-swipe", G_CALLBACK (end_swipe_cb), self, 0);

  self->can_scroll = TRUE;
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
GtkWidget *
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
  hdy_swipeable_emit_child_switched (HDY_SWIPEABLE (self), n, duration);
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

/**
 * hdy_carousel_get_reveal_duration:
 * @self: a #HdyCarousel
 *
 * Gets duration of the animation used when adding or removing pages in
 * milliseconds.
 *
 * Returns: Page reveal duration
 *
 * Since: 1.0
 */
guint
hdy_carousel_get_reveal_duration (HdyCarousel *self)
{
  g_return_val_if_fail (HDY_IS_CAROUSEL (self), 0);

  return hdy_carousel_box_get_reveal_duration (self->scrolling_box);
}

/**
 * hdy_carousel_set_reveal_duration:
 * @self: a #HdyCarousel
 * @reveal_duration: the new reveal duration value
 *
 * Sets duration of the animation used when adding or removing pages in
 * milliseconds.
 *
 * Since: 1.0
 */
void
hdy_carousel_set_reveal_duration (HdyCarousel *self,
                                  guint        reveal_duration)
{
  g_return_if_fail (HDY_IS_CAROUSEL (self));

  hdy_carousel_box_set_reveal_duration (self->scrolling_box, reveal_duration);
}
