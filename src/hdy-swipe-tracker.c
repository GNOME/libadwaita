/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-swipe-tracker-private.h"

#include <math.h>

#define TOUCHPAD_BASE_DISTANCE 400
#define SCROLL_MULTIPLIER 10
#define MIN_ANIMATION_DURATION 100
#define MAX_ANIMATION_DURATION 400
#define VELOCITY_THRESHOLD 0.4
#define DURATION_MULTIPLIER 3
#define ANIMATION_BASE_VELOCITY 0.002
#define DRAG_THRESHOLD_DISTANCE 5

/**
 * PRIVATE:hdy-swipe-tracker
 * @short_description: Swipe tracker used in #HdyPaginator and #HdyDrawer
 * @title: HdySwipeTracker
 * @See_also: #HdyPaginator, #HdyDrawer
 * @stability: Private
 *
 * The HdySwipeTracker object can be used for implementing widgets with swipe
 * gestures. It supports touch-based swipes, pointer dragging, and touchpad
 * scrolling.
 *
 * The events must be received as early as possible to defer the events to
 * child widgets when needed. Usually this happens naturally, but
 * GtkScrolledWindow receives events on capture phase via a private function.
 * Because of that, implementing widgets must do the same thing, i.e. receive
 * events on capture phase and call hdy_swipe_tracker_captured_event() for
 * each event. This can be done as follows:
 * |[<!-- language="C" -->
 * g_object_set_data (G_OBJECT (self), "captured-event-handler", captured_event_cb);
 * ]|
 * Where captured_event_cb() is:
 * |[<!-- language="C" -->
 * static gboolean
 * captured_event_cb (MyWidget *self, GdkEvent *event)
 * {
 *   return hdy_swipe_tracker_captured_event (self->tracker, event);
 * }
 * ]|
 *
 * NOTE: In GTK4 this can be replaced by a GtkEventControllerLegacy with capture
 * propagation phase.
 *
 * Aside from that, implementing widgets must connect to #HdySwipeTracker::begin,
 * #HdySwipeTracker::update and #HdySwipeTracker::end signals. See these
 * signals for details.
 *
 * The widgets will probably want to expose #HdySwipeTracker:enabled property.
 * If they expect to use horizontal orientation, #HdySwipeTracker:reversed
 * property can be used for supporting RTL text direction.
 *
 * Since: 0.0.11
 */

typedef enum {
  HDY_SWIPE_TRACKER_STATE_NONE,
  HDY_SWIPE_TRACKER_STATE_PREPARING,
  HDY_SWIPE_TRACKER_STATE_PENDING,
  HDY_SWIPE_TRACKER_STATE_SCROLLING,
  HDY_SWIPE_TRACKER_STATE_FINISHING,
} HdySwipeTrackerState;

struct _HdySwipeTracker
{
  GObject parent_instance;

  GtkWidget *widget;
  gboolean enabled;
  gboolean reversed;
  GtkOrientation orientation;

  guint32 prev_time;
  gdouble velocity;

  gdouble initial_progress;
  gdouble progress;
  gboolean cancelled;
  gdouble cancel_progress;

  gdouble prev_offset;
  gdouble distance;

  gdouble *snap_points;
  gint n_snap_points;

  HdySwipeTrackerState state;
  GtkGesture *touch_gesture;
};

G_DEFINE_TYPE_WITH_CODE (HdySwipeTracker, hdy_swipe_tracker, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL));

enum {
  PROP_0,
  PROP_WIDGET,
  PROP_ENABLED,
  PROP_REVERSED,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_REVERSED + 1,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_BEGIN,
  SIGNAL_UPDATE,
  SIGNAL_END,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
reset (HdySwipeTracker *self)
{
  if (self->snap_points) {
    g_free (self->snap_points);
    self->snap_points = NULL;
  }

  self->state = HDY_SWIPE_TRACKER_STATE_NONE;

  self->prev_offset = 0;
  self->distance = 0;

  self->initial_progress = 0;
  self->progress = 0;

  self->prev_time = 0;
  self->velocity = 0;

  self->cancel_progress = 0;

  self->cancelled = FALSE;
}

static void
gesture_prepare (HdySwipeTracker *self,
                 gdouble          x,
                 gdouble          y)
{
  if (self->state != HDY_SWIPE_TRACKER_STATE_NONE)
    return;

  self->state = HDY_SWIPE_TRACKER_STATE_PREPARING;
  g_signal_emit (self, signals[SIGNAL_BEGIN], 0, x, y);
}

static void
gesture_begin (HdySwipeTracker *self)
{
  GdkEvent *event;

  if (self->state != HDY_SWIPE_TRACKER_STATE_PENDING)
    return;

  event = gtk_get_current_event ();
  self->prev_time = gdk_event_get_time (event);
  self->state = HDY_SWIPE_TRACKER_STATE_SCROLLING;
}

static void
gesture_update (HdySwipeTracker *self,
                gdouble          delta)
{
  GdkEvent *event;
  guint32 time;
  gdouble progress;
  gdouble first_point, last_point;

  if (self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  event = gtk_get_current_event ();
  time = gdk_event_get_time (event);
  if (time != self->prev_time)
    self->velocity = delta / (time - self->prev_time);

  first_point = self->snap_points[0];
  last_point = self->snap_points[self->n_snap_points - 1];

  progress = self->progress + delta;
  progress = CLAMP (progress, first_point, last_point);

  /* FIXME: this is a hack to prevent swiping more than 1 page at once */
  progress = CLAMP (progress, self->initial_progress - 1, self->initial_progress + 1);

  self->progress = progress;

  g_signal_emit (self, signals[SIGNAL_UPDATE], 0, progress);

  self->prev_time = time;
}

static void
get_closest_snap_points (HdySwipeTracker *self,
                         gdouble         *upper,
                         gdouble         *lower)
{
  gint i;

  *upper = 0;
  *lower = 0;

  for (i = 0; i < self->n_snap_points; i++) {
    if (self->snap_points[i] >= self->progress) {
      *upper = self->snap_points[i];
      break;
    }
  }

  for (i = self->n_snap_points - 1; i >= 0; i--) {
    if (self->snap_points[i] <= self->progress) {
      *lower = self->snap_points[i];
      break;
    }
  }
}

static gdouble
get_end_progress (HdySwipeTracker *self)
{
  gdouble upper, lower, middle;

  if (self->cancelled)
    return self->cancel_progress;

  get_closest_snap_points (self, &upper, &lower);
  middle = (upper + lower) / 2;

  if (self->progress > middle)
    return (self->velocity * self->distance > -VELOCITY_THRESHOLD ||
            self->initial_progress > upper) ? upper : lower;

  return (self->velocity * self->distance < VELOCITY_THRESHOLD ||
          self->initial_progress < lower) ? lower : upper;
}

static void
gesture_end (HdySwipeTracker *self)
{
  gdouble end_progress, velocity;
  gint64 duration;

  if (self->state == HDY_SWIPE_TRACKER_STATE_NONE)
    return;

  end_progress = get_end_progress (self);

  velocity = ANIMATION_BASE_VELOCITY;
  if ((end_progress - self->progress) * self->velocity > 0)
    velocity = self->velocity;

  duration = ABS ((self->progress - end_progress) / velocity * DURATION_MULTIPLIER);
  duration = CLAMP (duration, MIN_ANIMATION_DURATION, MAX_ANIMATION_DURATION);

  if (self->cancelled)
    duration = 0;

  g_signal_emit (self, signals[SIGNAL_END], 0, duration, end_progress);

  if (self->cancelled)
    reset (self);
  else
    self->state = HDY_SWIPE_TRACKER_STATE_FINISHING;
}

static void
gesture_cancel (HdySwipeTracker *self)
{
  if (self->state == HDY_SWIPE_TRACKER_STATE_PREPARING) {
    reset (self);
    return;
  }

  if (self->state != HDY_SWIPE_TRACKER_STATE_PENDING &&
      self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  self->cancelled = TRUE;
  gesture_end (self);
}

static void
drag_begin_cb (HdySwipeTracker *self,
               gdouble          start_x,
               gdouble          start_y,
               GtkGestureDrag  *gesture)
{
  if (self->state != HDY_SWIPE_TRACKER_STATE_NONE) {
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gesture_prepare (self, start_x, start_y);
}

static void
drag_update_cb (HdySwipeTracker *self,
                gdouble          offset_x,
                gdouble          offset_y,
                GtkGestureDrag  *gesture)
{
  gdouble offset;
  gboolean is_vertical;

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);
  if (is_vertical)
    offset = -offset_y / self->distance;
  else
    offset = -offset_x / self->distance;

  if (self->reversed)
    offset = -offset;

  if (self->state == HDY_SWIPE_TRACKER_STATE_PENDING) {
    gdouble distance;
    gdouble first_point, last_point;
    gboolean is_offset_vertical, is_overshooting;

    first_point = self->snap_points[0];
    last_point = self->snap_points[self->n_snap_points - 1];

    distance = sqrt (offset_x * offset_x + offset_y * offset_y);
    is_offset_vertical = (ABS (offset_y) > ABS (offset_x));
    is_overshooting = (offset < 0 && self->progress <= first_point) ||
                      (offset > 0 && self->progress >= last_point);

    if (distance >= DRAG_THRESHOLD_DISTANCE) {
      if ((is_vertical == is_offset_vertical) && !is_overshooting) {
        gesture_begin (self);
        gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_CLAIMED);
      } else {
        gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
      }
    }
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    gesture_update (self, offset - self->prev_offset);
    self->prev_offset = offset;
  }
}

static void
drag_end_cb (HdySwipeTracker *self,
             gdouble          offset_x,
             gdouble          offset_y,
             GtkGestureDrag  *gesture)
{
  if (self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    gesture_cancel (self);
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gesture_end (self);
}

static void
drag_cancel_cb (HdySwipeTracker  *self,
                GdkEventSequence *sequence,
                GtkGesture       *gesture)
{
  gesture_cancel (self);
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
}

static gboolean
captured_scroll_event (HdySwipeTracker *self,
                       GdkEvent        *event)
{
  GdkDevice *source_device;
  GdkInputSource input_source;
  gdouble dx, dy, delta;
  gboolean is_vertical;

  if (gdk_event_get_scroll_direction (event, NULL))
    return GDK_EVENT_PROPAGATE;

  source_device = gdk_event_get_source_device (event);
  input_source = gdk_device_get_source (source_device);
  if (input_source != GDK_SOURCE_TOUCHPAD &&
      input_source != GDK_SOURCE_TRACKPOINT)
    return GDK_EVENT_PROPAGATE;

  if (self->state == HDY_SWIPE_TRACKER_STATE_NONE) {
    gdouble root_x, root_y;
    gint x, y;
    GtkWidget *toplevel;

    gdk_event_get_root_coords (event, &root_x, &root_y);

    toplevel = gtk_widget_get_toplevel (self->widget);
    gtk_widget_translate_coordinates (toplevel, self->widget, root_x, root_y, &x, &y);

    gesture_prepare (self, x, y);
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_PREPARING) {
    if (gdk_event_is_scroll_stop_event (event))
      gesture_cancel (self);

    return GDK_EVENT_PROPAGATE;
  }

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);

  gdk_event_get_scroll_deltas (event, &dx, &dy);
  delta = is_vertical ? dy : dx;
  if (self->reversed)
    delta = -delta;

  if (self->state == HDY_SWIPE_TRACKER_STATE_PENDING) {
    gboolean is_delta_vertical, is_overshooting;
    gdouble first_point, last_point;

    first_point = self->snap_points[0];
    last_point = self->snap_points[self->n_snap_points - 1];

    is_delta_vertical = (ABS (dy) > ABS (dx));
    is_overshooting = (delta < 0 && self->progress <= first_point) ||
                      (delta > 0 && self->progress >= last_point);

    if ((is_vertical == is_delta_vertical) && !is_overshooting)
      gesture_begin (self);
    else
      gesture_cancel (self);
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    if (gdk_event_is_scroll_stop_event (event)) {
      gesture_end (self);
    } else {
      self->distance = TOUCHPAD_BASE_DISTANCE;
      gesture_update (self, delta / TOUCHPAD_BASE_DISTANCE * SCROLL_MULTIPLIER);
      return GDK_EVENT_STOP;
    }
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_FINISHING)
    reset (self);

  return GDK_EVENT_PROPAGATE;
}

static void
hdy_swipe_tracker_constructed (GObject *object)
{
  HdySwipeTracker *self = (HdySwipeTracker *)object;

  g_assert (self->widget);

  gtk_widget_add_events (self->widget,
                         GDK_SMOOTH_SCROLL_MASK |
                         GDK_BUTTON_PRESS_MASK |
                         GDK_BUTTON_RELEASE_MASK |
                         GDK_BUTTON_MOTION_MASK |
                         GDK_TOUCH_MASK |
                         GDK_ALL_EVENTS_MASK);

  self->touch_gesture = g_object_new (GTK_TYPE_GESTURE_DRAG,
                                      "widget", self->widget,
                                      "propagation-phase", GTK_PHASE_NONE,
                                      NULL);

  g_signal_connect_swapped (self->touch_gesture, "drag-begin", G_CALLBACK (drag_begin_cb), self);
  g_signal_connect_swapped (self->touch_gesture, "drag-update", G_CALLBACK (drag_update_cb), self);
  g_signal_connect_swapped (self->touch_gesture, "drag-end", G_CALLBACK (drag_end_cb), self);
  g_signal_connect_swapped (self->touch_gesture, "cancel", G_CALLBACK (drag_cancel_cb), self);

  G_OBJECT_CLASS (hdy_swipe_tracker_parent_class)->constructed (object);
}

static void
hdy_swipe_tracker_dispose (GObject *object)
{
  HdySwipeTracker *self = (HdySwipeTracker *)object;

  if (self->touch_gesture)
    g_signal_handlers_disconnect_by_data (self->touch_gesture, self);

  g_clear_pointer (&self->snap_points, g_free);
  g_clear_object (&self->touch_gesture);
  g_clear_object (&self->widget);

  G_OBJECT_CLASS (hdy_swipe_tracker_parent_class)->dispose (object);
}

static void
hdy_swipe_tracker_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  HdySwipeTracker *self = HDY_SWIPE_TRACKER (object);

  switch (prop_id) {
  case PROP_WIDGET:
    g_value_set_object (value, self->widget);
    break;

  case PROP_ENABLED:
    g_value_set_boolean (value, hdy_swipe_tracker_get_enabled (self));
    break;

  case PROP_REVERSED:
    g_value_set_boolean (value, hdy_swipe_tracker_get_reversed (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_swipe_tracker_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  HdySwipeTracker *self = HDY_SWIPE_TRACKER (object);

  switch (prop_id) {
  case PROP_WIDGET:
    self->widget = GTK_WIDGET (g_object_ref (g_value_get_object (value)));
    break;

  case PROP_ENABLED:
    hdy_swipe_tracker_set_enabled (self, g_value_get_boolean (value));
    break;

  case PROP_REVERSED:
    hdy_swipe_tracker_set_reversed (self, g_value_get_boolean (value));
    break;

  case PROP_ORIENTATION:
    {
      GtkOrientation orientation = g_value_get_enum (value);
      if (orientation != self->orientation) {
        self->orientation = g_value_get_enum (value);
        g_object_notify (G_OBJECT (self), "orientation");
      }
    }
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
hdy_swipe_tracker_class_init (HdySwipeTrackerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = hdy_swipe_tracker_constructed;
  object_class->dispose = hdy_swipe_tracker_dispose;
  object_class->get_property = hdy_swipe_tracker_get_property;
  object_class->set_property = hdy_swipe_tracker_set_property;

  /**
   * HdySwipeTracker:widget:
   *
   * The widget the swipe tracker is attached to. Must not be %NULL.
   *
   * Since: 0.0.11
   */
  props[PROP_WIDGET] =
    g_param_spec_object ("widget",
                         _("Widget"),
                         _("The widget the swipe tracker is attached to"),
                         GTK_TYPE_WIDGET,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * HdySwipeTracker:enabled:
   *
   * Whether the swipe tracker is enabled. When it's not enabled, no events
   * will be processed. Usually widgets will want to expose this via a property.
   *
   * Since: 0.0.11
   */
  props[PROP_ENABLED] =
    g_param_spec_boolean ("enabled",
                          _("Enabled"),
                          _("Whether the swipe tracker processes events"),
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdySwipeTracker:reversed:
   *
   * Whether to reverse the swipe direction. If the swipe tracker is horizontal,
   * it can be used for supporting RTL text direction.
   *
   * Since: 0.0.11
   */
  props[PROP_REVERSED] =
    g_param_spec_boolean ("reversed",
                          _("Reversed"),
                          _("Whether swipe direction is reversed"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * HdySwipeTracker::begin:
   * @self: The #HdySwipeTracker instance
   * @x: The X coordinate relative to the widget
   * @y: The Y coordinate relative to the widget
   *
   * This signal is emitted when a possible swipe is detected. The widget should
   * check whether a swipe is possible. The provided coordinates can be used to
   * only allow starting the swipe in certain parts of the widget, for example
   * a drag handle.
   *
   * If a swipe is possible, the widget must call hdy_swipe_tracker_confirm_swipe()
   * to provide details about the swipe, see that function for details.
   *
   * Since: 0.0.11
   */
  signals[SIGNAL_BEGIN] =
    g_signal_new ("begin",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_DOUBLE, G_TYPE_DOUBLE);

  /**
   * HdySwipeTracker::update:
   * @self: The #HdySwipeTracker instance
   * @progress: The current animation progress
   *
   * This signal is emitted every time the progress value changes. The widget
   * must redraw the widget to reflect the change.
   *
   * Since: 0.0.11
   */
  signals[SIGNAL_UPDATE] =
    g_signal_new ("update",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_DOUBLE);

  /**
   * HdySwipeTracker::end:
   * @self: The #HdySwipeTracker instance
   * @duration: Snap-back animation duration in milliseconds
   * @end_progress: The progress value to animate to
   *
   * This signal is emitted as soon as the gesture has stopped. The widget must
   * animate the progress from the current value to @end_progress with
   * easeOutCubic interpolation over the next @duration milliseconds.
   *
   * @end_progress will always match either one of the provided snap points if
   * the swipe was completed successfully, or @cancel_progress value passed in
   * hdy_swipe_tracker_confirm_swipe() call if the swipe was cancelled.
   *
   * @duration can be 0, in that case the widget must immediately set the
   * progress value to @end_progress.
   *
   * Since: 0.0.11
   */
  signals[SIGNAL_END] =
    g_signal_new ("end",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_INT64, G_TYPE_DOUBLE);
}

static void
hdy_swipe_tracker_init (HdySwipeTracker *self)
{
  reset (self);
  self->orientation = GTK_ORIENTATION_HORIZONTAL;
  self->enabled = TRUE;
}

/**
 * hdy_swipe_tracker_new:
 * @widget: a #GtkWidget to add the tracker on
 *
 * Create a new #HdySwipeTracker object on @widget.
 *
 * Returns: the newly created #HdySwipeTracker object
 *
 * Since: 0.0.11
 */
HdySwipeTracker *
hdy_swipe_tracker_new (GtkWidget *widget)
{
  g_return_val_if_fail (widget != NULL, NULL);

  return g_object_new (HDY_TYPE_SWIPE_TRACKER,
                       "widget", widget,
                       NULL);
}

/**
 * hdy_swipe_tracker_get_enabled:
 * @self: a #HdySwipeTracker
 *
 * Get whether @self is enabled. When it's not enabled, no events will be
 * processed. Generally widgets will want to expose this via a property.
 *
 * Returns: %TRUE if @self is enabled
 *
 * Since: 0.0.11
 */
gboolean
hdy_swipe_tracker_get_enabled (HdySwipeTracker *self)
{
  g_return_val_if_fail (HDY_IS_SWIPE_TRACKER (self), FALSE);

  return self->enabled;
}

/**
 * hdy_swipe_tracker_set_enabled:
 * @self: a #HdySwipeTracker
 * @enabled: whether to enable to swipe tracker
 *
 * Set whether @self is enabled. When it's not enabled, no events will be
 * processed. Usually widgets will want to expose this via a property.
 *
 * Since: 0.0.11
 */
void
hdy_swipe_tracker_set_enabled (HdySwipeTracker *self,
                               gboolean         enabled)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  enabled = !!enabled;

  if (self->enabled == enabled)
    return;

  self->enabled = enabled;

  if (!enabled && self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING)
    reset (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLED]);
}

/**
 * hdy_swipe_tracker_get_reversed:
 * @self: a #HdySwipeTracker
 *
 * Get whether @self is reversing the swipe direction.
 *
 * Returns: %TRUE is the direction is reversed
 *
 * Since: 0.0.11
 */
gboolean
hdy_swipe_tracker_get_reversed (HdySwipeTracker *self)
{
  g_return_val_if_fail (HDY_IS_SWIPE_TRACKER (self), FALSE);

  return self->reversed;
}

/**
 * hdy_swipe_tracker_set_reversed:
 * @self: a #HdySwipeTracker
 * @reversed: whether to reverse the swipe direction
 *
 * Set whether to reverse the swipe direction. If @self is horizontal,
 * can be used for supporting RTL text direction.
 *
 * Since: 0.0.11
 */
void
hdy_swipe_tracker_set_reversed (HdySwipeTracker *self,
                                gboolean         reversed)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  reversed = !!reversed;

  if (self->reversed == reversed)
    return;

  self->reversed = reversed;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVERSED]);
}

/**
 * hdy_swipe_tracker_captured_event:
 * @self: a #HdySwipeTracker
 * @event: a captured #GdkEvent
 *
 * Handles an event. This must be called for events received at capture phase
 * only.
 *
 * Returns: %TRUE is the event was handled and must not be propagated
 *
 * Since: 0.0.11
 */
gboolean
hdy_swipe_tracker_captured_event (HdySwipeTracker *self,
                                  GdkEvent        *event)
{
  GdkEventSequence *sequence;
  gboolean retval;
  GtkEventSequenceState state;

  g_return_val_if_fail (HDY_IS_SWIPE_TRACKER (self), GDK_EVENT_PROPAGATE);

  if (!self->enabled && self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING)
    return GDK_EVENT_PROPAGATE;

  if (event->type == GDK_SCROLL)
    return captured_scroll_event (self, event);

  if (event->type != GDK_BUTTON_PRESS &&
      event->type != GDK_BUTTON_RELEASE &&
      event->type != GDK_MOTION_NOTIFY &&
      event->type != GDK_TOUCH_BEGIN &&
      event->type != GDK_TOUCH_END &&
      event->type != GDK_TOUCH_UPDATE &&
      event->type != GDK_TOUCH_CANCEL)
    return GDK_EVENT_PROPAGATE;

  sequence = gdk_event_get_event_sequence (event);
  retval = gtk_event_controller_handle_event (GTK_EVENT_CONTROLLER (self->touch_gesture), event);
  state = gtk_gesture_get_sequence_state (self->touch_gesture, sequence);

  if (state == GTK_EVENT_SEQUENCE_DENIED) {
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (self->touch_gesture));
    return GDK_EVENT_PROPAGATE;
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    return GDK_EVENT_STOP;
  } else if (self->state == HDY_SWIPE_TRACKER_STATE_FINISHING) {
    reset (self);
    return GDK_EVENT_STOP;
  }

  return retval;
}

static gboolean
is_sorted (gdouble *array,
           gint     n)
{
  gint i;

  if (n < 2)
    return TRUE;

  for (i = 0; i < n - 1; i++)
    if (array[i] > array[i + 1])
      return FALSE;

  return TRUE;
}

/**
 * hdy_swipe_tracker_confirm_swipe:
 * @self: a #HdySwipeTracker
 * @distance: swipe distance in pixels
 * @snap_points: (array length=n_snap_points) (transfer full): array of snap
 *   points, must be sorted in ascending order
 * @n_snap_points: length of @snap_points
 * @current_progress: initial progress value
 * @cancel_progress: the value that will be used if the swipe is cancelled
 *
 * Confirms a swipe. User has to call this in #HdySwipeTracker::begin signal
 * handler, otherwise the swipe wouldn't start. If there's an animation running,
 * the user should stop it and pass its calue as @current_progress.
 *
 * The call is not a guarantee that the swipe will be started; child widgets
 * may intercept it, in which case #HdySwipeTracker::end will be emitted with
 * @cancel_progress value and 0 duration.
 *
 * @cancel_progress must always be a snap point, or an otherwise a value
 * matching a non-transient state, otherwise the widget will get stuck
 * mid-animation.
 *
 * If there's no animation running, @current_progress and @cancel_progress
 * must be same.
 *
 * Since: 0.0.11
 */
void
hdy_swipe_tracker_confirm_swipe (HdySwipeTracker *self,
                                 gdouble          distance,
                                 gdouble         *snap_points,
                                 gint             n_snap_points,
                                 gdouble          current_progress,
                                 gdouble          cancel_progress)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));
  g_return_if_fail (distance > 0.0);
  g_return_if_fail (snap_points);
  g_return_if_fail (n_snap_points > 0);
  g_return_if_fail (is_sorted (snap_points, n_snap_points));
  g_return_if_fail (current_progress >= snap_points[0]);
  g_return_if_fail (current_progress <= snap_points[n_snap_points - 1]);
  g_return_if_fail (cancel_progress >= snap_points[0]);
  g_return_if_fail (cancel_progress <= snap_points[n_snap_points - 1]);

  if (self->state != HDY_SWIPE_TRACKER_STATE_PREPARING) {
    gesture_cancel (self);
    return;
  }

  if (self->snap_points)
    g_free (self->snap_points);

  self->distance = distance;
  self->initial_progress = current_progress;
  self->progress = current_progress;
  self->velocity = 0;
  self->snap_points = snap_points;
  self->n_snap_points = n_snap_points;
  self->cancel_progress = cancel_progress;
  self->state = HDY_SWIPE_TRACKER_STATE_PENDING;
}
