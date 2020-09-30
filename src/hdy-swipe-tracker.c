/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"
#include <glib/gi18n-lib.h>

#include "hdy-swipe-tracker-private.h"
#include "hdy-navigation-direction.h"

#include <math.h>

#define TOUCHPAD_BASE_DISTANCE_H 400
#define TOUCHPAD_BASE_DISTANCE_V 300
#define SCROLL_MULTIPLIER 10
#define MIN_ANIMATION_DURATION 100
#define MAX_ANIMATION_DURATION 400
#define VELOCITY_THRESHOLD 0.4
#define DURATION_MULTIPLIER 3
#define ANIMATION_BASE_VELOCITY 0.002
#define DRAG_THRESHOLD_DISTANCE 16

/**
 * SECTION:hdy-swipe-tracker
 * @short_description: Swipe tracker used in #HdyCarousel and #HdyLeaflet
 * @title: HdySwipeTracker
 * @See_also: #HdyCarousel, #HdyLeaflet, #HdySwipeable
 *
 * The HdySwipeTracker object can be used for implementing widgets with swipe
 * gestures. It supports touch-based swipes, pointer dragging, and touchpad
 * scrolling.
 *
 * The widgets will probably want to expose #HdySwipeTracker:enabled property.
 * If they expect to use horizontal orientation, #HdySwipeTracker:reversed
 * property can be used for supporting RTL text direction.
 *
 * Since: 1.0
 */

typedef enum {
  HDY_SWIPE_TRACKER_STATE_NONE,
  HDY_SWIPE_TRACKER_STATE_PENDING,
  HDY_SWIPE_TRACKER_STATE_SCROLLING,
  HDY_SWIPE_TRACKER_STATE_FINISHING,
  HDY_SWIPE_TRACKER_STATE_REJECTED,
} HdySwipeTrackerState;

struct _HdySwipeTracker
{
  GObject parent_instance;

  HdySwipeable *swipeable;
  gboolean enabled;
  gboolean reversed;
  gboolean allow_mouse_drag;
  GtkOrientation orientation;

  gdouble pointer_x;
  gdouble pointer_y;

  gdouble start_x;
  gdouble start_y;
  gboolean use_capture_phase;

  guint32 prev_time;
  gdouble velocity;

  gdouble initial_progress;
  gdouble progress;
  gboolean cancelled;

  gdouble prev_offset;

  HdySwipeTrackerState state;

  GtkEventController *motion_controller;
  GtkEventController *scroll_controller;
  GtkGesture *touch_gesture;
  GtkGesture *touch_gesture_capture;
};

G_DEFINE_TYPE_WITH_CODE (HdySwipeTracker, hdy_swipe_tracker, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL));

enum {
  PROP_0,
  PROP_SWIPEABLE,
  PROP_ENABLED,
  PROP_REVERSED,
  PROP_ALLOW_MOUSE_DRAG,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ALLOW_MOUSE_DRAG + 1,
};

static GParamSpec *props[LAST_PROP];

enum {
  SIGNAL_BEGIN_SWIPE,
  SIGNAL_UPDATE_SWIPE,
  SIGNAL_END_SWIPE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
reset (HdySwipeTracker *self)
{
  self->state = HDY_SWIPE_TRACKER_STATE_NONE;

  self->prev_offset = 0;

  self->initial_progress = 0;
  self->progress = 0;

  self->start_x = 0;
  self->start_y = 0;
  self->use_capture_phase = FALSE;

  self->prev_time = 0;
  self->velocity = 0;

  self->cancelled = FALSE;
}

static void
get_range (HdySwipeTracker *self,
           gdouble         *first,
           gdouble         *last)
{
  g_autofree gdouble *points = NULL;
  gint n;

  points = hdy_swipeable_get_snap_points (self->swipeable, &n);

  *first = points[0];
  *last = points[n - 1];
}

static void
gesture_prepare (HdySwipeTracker        *self,
                 HdyNavigationDirection  direction,
                 gboolean                is_drag)
{
  GdkRectangle rect;

  if (self->state != HDY_SWIPE_TRACKER_STATE_NONE)
    return;

  hdy_swipeable_get_swipe_area (self->swipeable, direction, is_drag, &rect);

  if (self->start_x < rect.x ||
      self->start_x >= rect.x + rect.width ||
      self->start_y < rect.y ||
      self->start_y >= rect.y + rect.height) {
    self->state = HDY_SWIPE_TRACKER_STATE_REJECTED;

    return;
  }

  hdy_swipe_tracker_emit_begin_swipe (self, direction, TRUE);

  self->initial_progress = hdy_swipeable_get_progress (self->swipeable);
  self->progress = self->initial_progress;
  self->velocity = 0;
  self->state = HDY_SWIPE_TRACKER_STATE_PENDING;
}

static void
gesture_begin (HdySwipeTracker *self,
               guint32          time)
{
  if (self->state != HDY_SWIPE_TRACKER_STATE_PENDING)
    return;

  self->prev_time = time;
  self->state = HDY_SWIPE_TRACKER_STATE_SCROLLING;
}

static void
gesture_update (HdySwipeTracker *self,
                gdouble          delta,
                guint32          time)
{
  gdouble progress;
  gdouble first_point, last_point;

  if (self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  if (time != self->prev_time)
    self->velocity = delta / (time - self->prev_time);

  get_range (self, &first_point, &last_point);

  progress = self->progress + delta;
  progress = CLAMP (progress, first_point, last_point);

  /* FIXME: this is a hack to prevent swiping more than 1 page at once */
  progress = CLAMP (progress, self->initial_progress - 1, self->initial_progress + 1);

  self->progress = progress;

  hdy_swipe_tracker_emit_update_swipe (self, progress);

  self->prev_time = time;
}

static void
get_closest_snap_points (HdySwipeTracker *self,
                         gdouble         *upper,
                         gdouble         *lower)
{
  gint i, n;
  gdouble *points;

  *upper = 0;
  *lower = 0;

  points = hdy_swipeable_get_snap_points (self->swipeable, &n);

  for (i = 0; i < n; i++) {
    if (points[i] >= self->progress) {
      *upper = points[i];
      break;
    }
  }

  for (i = n - 1; i >= 0; i--) {
    if (points[i] <= self->progress) {
      *lower = points[i];
      break;
    }
  }

  g_free (points);
}

static gdouble
get_end_progress (HdySwipeTracker *self,
                  gdouble          distance)
{
  gdouble upper, lower, middle;

  if (self->cancelled)
    return hdy_swipeable_get_cancel_progress (self->swipeable);

  get_closest_snap_points (self, &upper, &lower);
  middle = (upper + lower) / 2;

  if (self->progress > middle)
    return (self->velocity * distance > -VELOCITY_THRESHOLD ||
            self->initial_progress > upper) ? upper : lower;

  return (self->velocity * distance < VELOCITY_THRESHOLD ||
          self->initial_progress < lower) ? lower : upper;
}

static void
gesture_end (HdySwipeTracker *self,
             gdouble          distance)
{
  gdouble end_progress, velocity;
  gint64 duration;

  if (self->state == HDY_SWIPE_TRACKER_STATE_NONE)
    return;

  end_progress = get_end_progress (self, distance);

  velocity = ANIMATION_BASE_VELOCITY;
  if ((end_progress - self->progress) * self->velocity > 0)
    velocity = self->velocity;

  duration = ABS ((self->progress - end_progress) / velocity * DURATION_MULTIPLIER);
  if (self->progress != end_progress)
    duration = CLAMP (duration, MIN_ANIMATION_DURATION, MAX_ANIMATION_DURATION);

  hdy_swipe_tracker_emit_end_swipe (self, duration, end_progress);

  if (!self->cancelled)
    self->state = HDY_SWIPE_TRACKER_STATE_FINISHING;

  reset (self);
}

static void
gesture_cancel (HdySwipeTracker *self,
                gdouble          distance)
{
  if (self->state != HDY_SWIPE_TRACKER_STATE_PENDING &&
      self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    reset (self);

    return;
  }

  self->cancelled = TRUE;
  gesture_end (self, distance);
}

static gboolean
should_suppress_drag (HdySwipeTracker *self,
                      GtkWidget       *widget)
{
  GtkWidget *parent = widget;
  gboolean found_window_handle = FALSE;

  while (parent && parent != GTK_WIDGET (self->swipeable)) {
    found_window_handle |= GTK_IS_WINDOW_HANDLE (parent);

    parent = gtk_widget_get_parent (parent);
  }

  return found_window_handle;
}

static gboolean
has_conflicts (HdySwipeTracker *self,
               GtkWidget       *widget)
{
  HdySwipeTracker *other;

  if (widget == GTK_WIDGET (self->swipeable))
    return TRUE;

  if (!HDY_IS_SWIPEABLE (widget))
    return FALSE;

  other = hdy_swipeable_get_swipe_tracker (HDY_SWIPEABLE (widget));

  return self->orientation == other->orientation;
}

/* HACK: Since we don't have _gtk_widget_consumes_motion(), we can't do a proper
 * check for whether we can drag from a widget or not. So we trust the widgets
 * to propagate or stop their events. However, GtkButton stops press events,
 * making it impossible to drag from it.
 */
static gboolean
should_force_drag (HdySwipeTracker *self,
                   GtkWidget       *widget)
{
  GtkWidget *parent = widget;
  gboolean found_button = FALSE;

  while (parent && !has_conflicts (self, parent)) {
    found_button |= GTK_IS_BUTTON (parent);

    parent = gtk_widget_get_parent (parent);
  }

  return found_button && parent == GTK_WIDGET (self->swipeable);
}

static void
drag_capture_begin_cb (HdySwipeTracker *self,
                       gdouble          start_x,
                       gdouble          start_y,
                       GtkGestureDrag  *gesture)
{
  GtkWidget *widget;

  if (self->state != HDY_SWIPE_TRACKER_STATE_NONE) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  widget = gtk_widget_pick (GTK_WIDGET (self->swipeable),
                            start_x,
                            start_y,
                            GTK_PICK_DEFAULT);

  if (should_suppress_drag (self, widget) || !should_force_drag (self, widget)) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  self->use_capture_phase = TRUE;

  self->start_x = start_x;
  self->start_y = start_y;

  gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
}

static void
drag_begin_cb (HdySwipeTracker *self,
               gdouble          start_x,
               gdouble          start_y,
               GtkGestureDrag  *gesture)
{
  GtkWidget *widget;

  if (self->state != HDY_SWIPE_TRACKER_STATE_NONE) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  widget = gtk_widget_pick (GTK_WIDGET (self->swipeable),
                          start_x,
                          start_y,
                          GTK_PICK_DEFAULT);

  if (should_suppress_drag (self, widget)) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  self->start_x = start_x;
  self->start_y = start_y;

  gtk_gesture_set_state (self->touch_gesture_capture, GTK_EVENT_SEQUENCE_DENIED);
}

static void
drag_update_cb (HdySwipeTracker *self,
                gdouble          offset_x,
                gdouble          offset_y,
                GtkGestureDrag  *gesture)
{
  gdouble offset, distance;
  gboolean is_vertical, is_offset_vertical;
  guint32 time;

  distance = hdy_swipeable_get_distance (self->swipeable);

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);
  if (is_vertical)
    offset = -offset_y / distance;
  else
    offset = -offset_x / distance;

  if (self->reversed)
    offset = -offset;

  is_offset_vertical = (ABS (offset_y) > ABS (offset_x));

  if (self->state == HDY_SWIPE_TRACKER_STATE_REJECTED) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_NONE) {
    if (is_vertical == is_offset_vertical)
      gesture_prepare (self, offset > 0 ? HDY_NAVIGATION_DIRECTION_FORWARD : HDY_NAVIGATION_DIRECTION_BACK, TRUE);
    else
      gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  if (self->state == HDY_SWIPE_TRACKER_STATE_PENDING) {
    gdouble drag_distance;
    gdouble first_point, last_point;
    gboolean is_overshooting;

    get_range (self, &first_point, &last_point);

    drag_distance = sqrt (offset_x * offset_x + offset_y * offset_y);
    is_overshooting = (offset < 0 && self->progress <= first_point) ||
                      (offset > 0 && self->progress >= last_point);

    if (drag_distance >= DRAG_THRESHOLD_DISTANCE) {
      if ((is_vertical == is_offset_vertical) && !is_overshooting) {
        gesture_begin (self, time);
        self->prev_offset = offset;
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
      } else {
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
      }
    }
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    gesture_update (self, offset - self->prev_offset, time);
    self->prev_offset = offset;
  }
}

static void
drag_end_cb (HdySwipeTracker *self,
             gdouble          offset_x,
             gdouble          offset_y,
             GtkGestureDrag  *gesture)
{
  gdouble distance;

  distance = hdy_swipeable_get_distance (self->swipeable);

  if (self->state == HDY_SWIPE_TRACKER_STATE_REJECTED) {
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);

    reset (self);
    return;
  }

  if (self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    gesture_cancel (self, distance);
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gesture_end (self, distance);
}

static void
drag_cancel_cb (HdySwipeTracker  *self,
                GdkEventSequence *sequence,
                GtkGesture       *gesture)
{
  gdouble distance;

  distance = hdy_swipeable_get_distance (self->swipeable);

  gesture_cancel (self, distance);
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
}

static gboolean
handle_scroll_event (HdySwipeTracker *self,
                     GdkEvent        *event)
{
  GdkDevice *source_device;
  GdkInputSource input_source;
  gdouble dx, dy, delta, distance;
  gboolean is_vertical;
  guint32 time;

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);
  distance = is_vertical ? TOUCHPAD_BASE_DISTANCE_V : TOUCHPAD_BASE_DISTANCE_H;

  if (gdk_scroll_event_get_direction (event) != GDK_SCROLL_SMOOTH)
    return GDK_EVENT_PROPAGATE;

  source_device = gdk_event_get_device (event);
  input_source = gdk_device_get_source (source_device);
  if (input_source != GDK_SOURCE_TOUCHPAD)
    return GDK_EVENT_PROPAGATE;

  gdk_scroll_event_get_deltas (event, &dx, &dy);
  delta = is_vertical ? dy : dx;
  if (self->reversed)
    delta = -delta;

  if (self->state == HDY_SWIPE_TRACKER_STATE_REJECTED) {
    if (gdk_scroll_event_is_stop (event))
      reset (self);

    return GDK_EVENT_PROPAGATE;
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_NONE) {
    if (gdk_scroll_event_is_stop (event))
      return GDK_EVENT_PROPAGATE;

    self->start_x = self->pointer_x;
    self->start_y = self->pointer_y;

    gesture_prepare (self, delta > 0 ? HDY_NAVIGATION_DIRECTION_FORWARD : HDY_NAVIGATION_DIRECTION_BACK, FALSE);
  }

  time = gdk_event_get_time (event);

  if (self->state == HDY_SWIPE_TRACKER_STATE_PENDING) {
    gboolean is_overshooting;
    gdouble first_point, last_point;

    get_range (self, &first_point, &last_point);

    is_overshooting = (delta < 0 && self->progress <= first_point) ||
                      (delta > 0 && self->progress >= last_point);

    if (!is_overshooting)
      gesture_begin (self, time);
    else
      gesture_cancel (self, distance);
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_SCROLLING) {
    if (gdk_scroll_event_is_stop (event)) {
      gesture_end (self, distance);
    } else {
      gesture_update (self, delta / distance * SCROLL_MULTIPLIER, time);
      return GDK_EVENT_STOP;
    }
  }

  if (self->state == HDY_SWIPE_TRACKER_STATE_FINISHING)
    reset (self);

  return GDK_EVENT_PROPAGATE;
}

static void
scroll_begin_cb (HdySwipeTracker          *self,
                 GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  if (self->use_capture_phase)
    return;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  handle_scroll_event (self, event);
}

static gboolean
scroll_cb (HdySwipeTracker          *self,
           gdouble                   dx,
           gdouble                   dy,
           GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  if (self->use_capture_phase)
    return GDK_EVENT_PROPAGATE;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  return handle_scroll_event (self, event);
}

static void
scroll_end_cb (HdySwipeTracker          *self,
               GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  if (self->use_capture_phase)
    return;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  handle_scroll_event (self, event);
}

static void
motion_cb (HdySwipeTracker          *self,
           gdouble                   x,
           gdouble                   y,
           GtkEventControllerMotion *controller)
{
  self->pointer_x = x;
  self->pointer_y = y;
}

static void
update_controllers (HdySwipeTracker *self)
{
  GtkEventControllerScrollFlags flags;

  if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
    flags = GTK_EVENT_CONTROLLER_SCROLL_HORIZONTAL;
  else
    flags = GTK_EVENT_CONTROLLER_SCROLL_VERTICAL;

  if (self->scroll_controller) {
    gtk_event_controller_scroll_set_flags (GTK_EVENT_CONTROLLER_SCROLL (self->scroll_controller), flags);
    gtk_event_controller_set_propagation_phase (self->scroll_controller,
                                                self->enabled ? GTK_PHASE_BUBBLE : GTK_PHASE_NONE);
  }

  if (self->motion_controller)
    gtk_event_controller_set_propagation_phase (self->motion_controller,
                                                self->enabled ? GTK_PHASE_CAPTURE : GTK_PHASE_NONE);

  if (self->touch_gesture)
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (self->touch_gesture),
                                                self->enabled ? GTK_PHASE_BUBBLE : GTK_PHASE_NONE);

  if (self->touch_gesture_capture)
    gtk_event_controller_set_propagation_phase (GTK_EVENT_CONTROLLER (self->touch_gesture_capture),
                                                self->enabled ? GTK_PHASE_CAPTURE : GTK_PHASE_NONE);
}

static void
set_orientation (HdySwipeTracker *self,
                 GtkOrientation   orientation)
{

  if (orientation == self->orientation)
    return;

  self->orientation = orientation;
  update_controllers (self);

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
hdy_swipe_tracker_constructed (GObject *object)
{
  HdySwipeTracker *self = HDY_SWIPE_TRACKER (object);
  GtkEventController *controller;

  g_assert (self->swipeable);

  g_signal_connect_object (self->swipeable, "unrealize", G_CALLBACK (reset), self, G_CONNECT_SWAPPED);

  controller = gtk_event_controller_motion_new ();
  gtk_event_controller_set_propagation_phase (controller, GTK_PHASE_CAPTURE);
  g_signal_connect_object (controller, "motion", G_CALLBACK (motion_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->motion_controller = controller;

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  g_signal_connect_object (controller, "drag-begin", G_CALLBACK (drag_capture_begin_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-update", G_CALLBACK (drag_update_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-end", G_CALLBACK (drag_end_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "cancel", G_CALLBACK (drag_cancel_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->touch_gesture_capture = GTK_GESTURE (controller);

  controller = GTK_EVENT_CONTROLLER (gtk_gesture_drag_new ());
  g_signal_connect_object (controller, "drag-begin", G_CALLBACK (drag_begin_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-update", G_CALLBACK (drag_update_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "drag-end", G_CALLBACK (drag_end_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "cancel", G_CALLBACK (drag_cancel_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->touch_gesture = GTK_GESTURE (controller);

  g_object_bind_property (self, "allow-mouse-drag",
                          self->touch_gesture, "touch-only",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

  g_object_bind_property (self, "allow-mouse-drag",
                          self->touch_gesture_capture, "touch-only",
                          G_BINDING_SYNC_CREATE | G_BINDING_INVERT_BOOLEAN);

  controller = gtk_event_controller_scroll_new (GTK_EVENT_CONTROLLER_SCROLL_NONE);
  g_signal_connect_object (controller, "scroll-begin", G_CALLBACK (scroll_begin_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "scroll", G_CALLBACK (scroll_cb), self, G_CONNECT_SWAPPED);
  g_signal_connect_object (controller, "scroll-end", G_CALLBACK (scroll_end_cb), self, G_CONNECT_SWAPPED);
  gtk_widget_add_controller (GTK_WIDGET (self->swipeable), controller);
  self->scroll_controller = controller;

  update_controllers (self);

  G_OBJECT_CLASS (hdy_swipe_tracker_parent_class)->constructed (object);
}

static void
hdy_swipe_tracker_dispose (GObject *object)
{
  HdySwipeTracker *self = HDY_SWIPE_TRACKER (object);

  if (self->touch_gesture) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable),
                                  GTK_EVENT_CONTROLLER (self->touch_gesture));
    g_clear_object (&self->touch_gesture);
  }

  if (self->motion_controller) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable), self->motion_controller);
    g_clear_object (&self->motion_controller);
  }

  if (self->scroll_controller) {
    gtk_widget_remove_controller (GTK_WIDGET (self->swipeable), self->scroll_controller);
    g_clear_object (&self->scroll_controller);
  }

  g_clear_object (&self->swipeable);

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
  case PROP_SWIPEABLE:
    g_value_set_object (value, hdy_swipe_tracker_get_swipeable (self));
    break;

  case PROP_ENABLED:
    g_value_set_boolean (value, hdy_swipe_tracker_get_enabled (self));
    break;

  case PROP_REVERSED:
    g_value_set_boolean (value, hdy_swipe_tracker_get_reversed (self));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    g_value_set_boolean (value, hdy_swipe_tracker_get_allow_mouse_drag (self));
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
  case PROP_SWIPEABLE:
    self->swipeable = HDY_SWIPEABLE (g_object_ref (g_value_get_object (value)));
    break;

  case PROP_ENABLED:
    hdy_swipe_tracker_set_enabled (self, g_value_get_boolean (value));
    break;

  case PROP_REVERSED:
    hdy_swipe_tracker_set_reversed (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    hdy_swipe_tracker_set_allow_mouse_drag (self, g_value_get_boolean (value));
    break;

  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
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
   * HdySwipeTracker:swipeable:
   *
   * The widget the swipe tracker is attached to. Must not be %NULL.
   *
   * Since: 1.0
   */
  props[PROP_SWIPEABLE] =
    g_param_spec_object ("swipeable",
                         _("Swipeable"),
                         _("The swipeable the swipe tracker is attached to"),
                         HDY_TYPE_SWIPEABLE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * HdySwipeTracker:enabled:
   *
   * Whether the swipe tracker is enabled. When it's not enabled, no events
   * will be processed. Usually widgets will want to expose this via a property.
   *
   * Since: 1.0
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
   * Since: 1.0
   */
  props[PROP_REVERSED] =
    g_param_spec_boolean ("reversed",
                          _("Reversed"),
                          _("Whether swipe direction is reversed"),
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * HdySwipeTracker:allow-mouse-drag:
   *
   * Whether to allow dragging with mouse pointer. This should usually be
   * %FALSE.
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
   * HdySwipeTracker::begin-swipe:
   * @self: The #HdySwipeTracker instance
   * @direction: The direction of the swipe
   * @direct: %TRUE if the swipe is directly triggered by a gesture,
   *   %FALSE if it's triggered via a #HdySwipeGroup
   *
   * This signal is emitted when a possible swipe is detected.
   *
   * The @direction value can be used to restrict the swipe to a certain
   * direction.
   *
   * Since: 1.0
   */
  signals[SIGNAL_BEGIN_SWIPE] =
    g_signal_new ("begin-swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  HDY_TYPE_NAVIGATION_DIRECTION, G_TYPE_BOOLEAN);

  /**
   * HdySwipeTracker::update-swipe:
   * @self: The #HdySwipeTracker instance
   * @progress: The current animation progress value
   *
   * This signal is emitted every time the progress value changes.
   *
   * Since: 1.0
   */
  signals[SIGNAL_UPDATE_SWIPE] =
    g_signal_new ("update-swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_DOUBLE);

  /**
   * HdySwipeTracker::end-swipe:
   * @self: The #HdySwipeTracker instance
   * @duration: Snap-back animation duration in milliseconds
   * @to: The progress value to animate to
   *
   * This signal is emitted as soon as the gesture has stopped.
   *
   * Since: 1.0
   */
  signals[SIGNAL_END_SWIPE] =
    g_signal_new ("end-swipe",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
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
 * @swipeable: a #GtkWidget to add the tracker on
 *
 * Create a new #HdySwipeTracker object on @widget.
 *
 * Returns: the newly created #HdySwipeTracker object
 *
 * Since: 1.0
 */
HdySwipeTracker *
hdy_swipe_tracker_new (HdySwipeable *swipeable)
{
  g_return_val_if_fail (HDY_IS_SWIPEABLE (swipeable), NULL);

  return g_object_new (HDY_TYPE_SWIPE_TRACKER,
                       "swipeable", swipeable,
                       NULL);
}

/**
 * hdy_swipe_tracker_get_swipeable:
 * @self: a #HdySwipeTracker
 *
 * Get @self's swipeable widget.
 *
 * Returns: (transfer none): the swipeable widget
 *
 * Since: 1.0
 */
HdySwipeable *
hdy_swipe_tracker_get_swipeable (HdySwipeTracker *self)
{
  g_return_val_if_fail (HDY_IS_SWIPE_TRACKER (self), NULL);

  return self->swipeable;
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
 * Since: 1.0
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
 * Since: 1.0
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

  update_controllers (self);

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
 * Since: 1.0
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
 * Since: 1.0
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
 * hdy_swipe_tracker_get_allow_mouse_drag:
 * @self: a #HdySwipeTracker
 *
 * Get whether @self can be dragged with mouse pointer.
 *
 * Returns: %TRUE is mouse dragging is allowed
 *
 * Since: 1.0
 */
gboolean
hdy_swipe_tracker_get_allow_mouse_drag (HdySwipeTracker *self)
{
  g_return_val_if_fail (HDY_IS_SWIPE_TRACKER (self), FALSE);

  return self->allow_mouse_drag;
}

/**
 * hdy_swipe_tracker_set_allow_mouse_drag:
 * @self: a #HdySwipeTracker
 * @allow_mouse_drag: whether to allow mouse dragging
 *
 * Set whether @self can be dragged with mouse pointer. This should usually be
 * %FALSE.
 *
 * Since: 1.0
 */
void
hdy_swipe_tracker_set_allow_mouse_drag (HdySwipeTracker *self,
                                        gboolean         allow_mouse_drag)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  allow_mouse_drag = !!allow_mouse_drag;

  if (self->allow_mouse_drag == allow_mouse_drag)
    return;

  self->allow_mouse_drag = allow_mouse_drag;

  update_controllers (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_MOUSE_DRAG]);
}

/**
 * hdy_swipe_tracker_shift_position:
 * @self: a #HdySwipeTracker
 * @delta: the position delta
 *
 * Move the current progress value by @delta. This can be used to adjust the
 * current position if snap points move during the gesture.
 *
 * Since: 1.0
 */
void
hdy_swipe_tracker_shift_position (HdySwipeTracker *self,
                                  gdouble          delta)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  if (self->state != HDY_SWIPE_TRACKER_STATE_PENDING &&
      self->state != HDY_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  self->progress += delta;
  self->initial_progress += delta;
}

void
hdy_swipe_tracker_emit_begin_swipe (HdySwipeTracker        *self,
                                    HdyNavigationDirection  direction,
                                    gboolean                direct)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  g_signal_emit (self, signals[SIGNAL_BEGIN_SWIPE], 0, direction, direct);
}

void
hdy_swipe_tracker_emit_update_swipe (HdySwipeTracker *self,
                                     gdouble          progress)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  g_signal_emit (self, signals[SIGNAL_UPDATE_SWIPE], 0, progress);
}

void
hdy_swipe_tracker_emit_end_swipe (HdySwipeTracker *self,
                                  gint64           duration,
                                  gdouble          to)
{
  g_return_if_fail (HDY_IS_SWIPE_TRACKER (self));

  g_signal_emit (self, signals[SIGNAL_END_SWIPE], 0, duration, to);
}
