/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "adw-swipe-tracker-private.h"
#include "adw-navigation-direction.h"

#include <math.h>

#define TOUCHPAD_BASE_DISTANCE_H 400
#define TOUCHPAD_BASE_DISTANCE_V 300
#define EVENT_HISTORY_THRESHOLD_MS 150
#define SCROLL_MULTIPLIER 10
#define MIN_ANIMATION_DURATION 100
#define MAX_ANIMATION_DURATION 400
#define VELOCITY_THRESHOLD_TOUCH 0.3
#define VELOCITY_THRESHOLD_TOUCHPAD 0.6
#define DECELERATION_TOUCH 0.998
#define DECELERATION_TOUCHPAD 0.997
#define VELOCITY_CURVE_THRESHOLD 2
#define DECELERATION_PARABOLA_MULTIPLIER 0.35
#define DURATION_MULTIPLIER 3
#define ANIMATION_BASE_VELOCITY 0.002
#define DRAG_THRESHOLD_DISTANCE 16
#define EPSILON 0.005

#define SIGN(x) ((x) > 0.0 ? 1.0 : ((x) < 0.0 ? -1.0 : 0.0))

/**
 * SECTION:adwswipetracker
 * @short_description: Swipe tracker used in #AdwCarousel and #AdwLeaflet
 * @title: AdwSwipeTracker
 * @See_also: #AdwCarousel, #AdwLeaflet, #AdwSwipeable
 *
 * The AdwSwipeTracker object can be used for implementing widgets with swipe
 * gestures. It supports touch-based swipes, pointer dragging, and touchpad
 * scrolling.
 *
 * The widgets will probably want to expose #AdwSwipeTracker:enabled property.
 * If they expect to use horizontal orientation, #AdwSwipeTracker:reversed
 * property can be used for supporting RTL text direction.
 *
 * Since: 1.0
 */

typedef enum {
  ADW_SWIPE_TRACKER_STATE_NONE,
  ADW_SWIPE_TRACKER_STATE_PENDING,
  ADW_SWIPE_TRACKER_STATE_SCROLLING,
  ADW_SWIPE_TRACKER_STATE_FINISHING,
  ADW_SWIPE_TRACKER_STATE_REJECTED,
} AdwSwipeTrackerState;

typedef struct {
  double delta;
  guint32 time;
} EventHistoryRecord;

struct _AdwSwipeTracker
{
  GObject parent_instance;

  AdwSwipeable *swipeable;
  gboolean enabled;
  gboolean reversed;
  gboolean allow_mouse_drag;
  gboolean allow_long_swipes;
  GtkOrientation orientation;

  double pointer_x;
  double pointer_y;

  GArray *event_history;

  double start_x;
  double start_y;

  double initial_progress;
  double progress;
  gboolean cancelled;

  double prev_offset;

  AdwSwipeTrackerState state;

  GtkEventController *motion_controller;
  GtkEventController *scroll_controller;
  GtkGesture *touch_gesture;
  GtkGesture *touch_gesture_capture;
};

G_DEFINE_TYPE_WITH_CODE (AdwSwipeTracker, adw_swipe_tracker, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GTK_TYPE_ORIENTABLE, NULL));

enum {
  PROP_0,
  PROP_SWIPEABLE,
  PROP_ENABLED,
  PROP_REVERSED,
  PROP_ALLOW_MOUSE_DRAG,
  PROP_ALLOW_LONG_SWIPES,

  /* GtkOrientable */
  PROP_ORIENTATION,
  LAST_PROP = PROP_ALLOW_LONG_SWIPES + 1,
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
reset (AdwSwipeTracker *self)
{
  self->state = ADW_SWIPE_TRACKER_STATE_NONE;

  self->prev_offset = 0;

  self->initial_progress = 0;
  self->progress = 0;

  g_array_remove_range (self->event_history, 0, self->event_history->len);

  self->start_x = 0;
  self->start_y = 0;

  self->cancelled = FALSE;
}

static void
get_range (AdwSwipeTracker *self,
           double          *first,
           double          *last)
{
  g_autofree double *points = NULL;
  int n;

  points = adw_swipeable_get_snap_points (self->swipeable, &n);

  *first = points[0];
  *last = points[n - 1];
}

static void
gesture_prepare (AdwSwipeTracker        *self,
                 AdwNavigationDirection  direction,
                 gboolean                is_drag)
{
  GdkRectangle rect;

  if (self->state != ADW_SWIPE_TRACKER_STATE_NONE)
    return;

  adw_swipeable_get_swipe_area (self->swipeable, direction, is_drag, &rect);

  if (self->start_x < rect.x ||
      self->start_x >= rect.x + rect.width ||
      self->start_y < rect.y ||
      self->start_y >= rect.y + rect.height) {
    self->state = ADW_SWIPE_TRACKER_STATE_REJECTED;

    return;
  }

  g_signal_emit (self, signals[SIGNAL_BEGIN_SWIPE], 0, direction);

  self->initial_progress = adw_swipeable_get_progress (self->swipeable);
  self->progress = self->initial_progress;
  self->state = ADW_SWIPE_TRACKER_STATE_PENDING;
}

static void
trim_history (AdwSwipeTracker *self,
              guint32          current_time)
{
  guint32 threshold_time = current_time - EVENT_HISTORY_THRESHOLD_MS;
  guint i;

  for (i = 0; i < self->event_history->len; i++) {
    guint32 time = g_array_index (self->event_history,
                                  EventHistoryRecord, i).time;

    if (time >= threshold_time)
      break;
  }

  if (i > 0)
    g_array_remove_range (self->event_history, 0, i);
}

static void
append_to_history (AdwSwipeTracker *self,
                   double           delta,
                   guint32          time)
{
  EventHistoryRecord record;

  trim_history (self, time);

  record.delta = delta;
  record.time = time;

  g_array_append_val (self->event_history, record);
}

static double
calculate_velocity (AdwSwipeTracker *self)
{
  double total_delta = 0;
  guint32 first_time = 0, last_time = 0;
  guint i;

  for (i = 0; i < self->event_history->len; i++) {
    EventHistoryRecord *r =
      &g_array_index (self->event_history, EventHistoryRecord, i);

    if (i == 0)
      first_time = r->time;
    else
      total_delta += r->delta;

    last_time = r->time;
  }

  if (first_time == last_time)
    return 0;

  return total_delta / (last_time - first_time);
}

static void
gesture_begin (AdwSwipeTracker *self)
{
  if (self->state != ADW_SWIPE_TRACKER_STATE_PENDING)
    return;

  self->state = ADW_SWIPE_TRACKER_STATE_SCROLLING;
}

static int
find_closest_point (double *points,
                    int     n,
                    double  pos)
{
  guint i, min = 0;

  for (i = 1; i < n; i++)
    if (ABS (points[i] - pos) < ABS (points[min] - pos))
      min = i;

  return min;
}

static int
find_next_point (double *points,
                 int     n,
                 double  pos)
{
  guint i;

  for (i = 0; i < n; i++)
    if (points[i] >= pos)
      return i;

  return -1;
}

static int
find_previous_point (double *points,
                     int     n,
                     double  pos)
{
  int i;

  for (i = n - 1; i >= 0; i--)
    if (points[i] <= pos)
      return i;

  return -1;
}

static int
find_point_for_projection (AdwSwipeTracker *self,
                           double          *points,
                           int              n,
                           double           pos,
                           double           velocity)
{
  int initial = find_closest_point (points, n, self->initial_progress);
  int prev = find_previous_point (points, n, pos);
  int next = find_next_point (points, n, pos);

  if ((velocity > 0 ? prev : next) == initial)
    return velocity > 0 ? next : prev;

  return find_closest_point (points, n, pos);
}

static void
get_bounds (AdwSwipeTracker *self,
            double          *points,
            int              n,
            double           pos,
            double          *lower,
            double          *upper)
{
  int prev, next;
  int closest = find_closest_point (points, n, pos);

  if (ABS (points[closest] - pos) < EPSILON) {
    prev = next = closest;
  } else {
    prev = find_previous_point (points, n, pos);
    next = find_next_point (points, n, pos);
  }

  *lower = points[MAX (prev - 1, 0)];
  *upper = points[MIN (next + 1, n - 1)];
}

static void
gesture_update (AdwSwipeTracker *self,
                double           delta,
                guint32          time)
{
  double lower, upper;
  double progress;

  if (self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  if (!self->allow_long_swipes) {
    g_autofree double *points = NULL;
    int n;

    points = adw_swipeable_get_snap_points (self->swipeable, &n);
    get_bounds (self, points, n, self->initial_progress, &lower, &upper);
  } else {
    get_range (self, &lower, &upper);
  }

  progress = self->progress + delta;
  progress = CLAMP (progress, lower, upper);

  self->progress = progress;

  g_signal_emit (self, signals[SIGNAL_UPDATE_SWIPE], 0, progress);
}

static double
get_end_progress (AdwSwipeTracker *self,
                  double           velocity,
                  gboolean         is_touchpad)
{
  double pos, decel, slope;
  g_autofree double *points = NULL;
  int n;
  double lower, upper;

  if (self->cancelled)
    return adw_swipeable_get_cancel_progress (self->swipeable);

  points = adw_swipeable_get_snap_points (self->swipeable, &n);

  if (ABS (velocity) < (is_touchpad ? VELOCITY_THRESHOLD_TOUCHPAD : VELOCITY_THRESHOLD_TOUCH))
    return points[find_closest_point (points, n, self->progress)];

  decel = is_touchpad ? DECELERATION_TOUCHPAD : DECELERATION_TOUCH;
  slope = decel / (1.0 - decel) / 1000.0;

  if (ABS (velocity) > VELOCITY_CURVE_THRESHOLD) {
    const double c = slope / 2 / DECELERATION_PARABOLA_MULTIPLIER;
    const double x = ABS (velocity) - VELOCITY_CURVE_THRESHOLD + c;

    pos = DECELERATION_PARABOLA_MULTIPLIER * x * x
        - DECELERATION_PARABOLA_MULTIPLIER * c * c
        + slope * VELOCITY_CURVE_THRESHOLD;
  } else {
    pos = ABS (velocity) * slope;
  }

  pos = (pos * SIGN (velocity)) + self->progress;

  if (!self->allow_long_swipes) {

    get_bounds (self, points, n, self->initial_progress, &lower, &upper);
  } else {
    get_range (self, &lower, &upper);
  }

  pos = CLAMP (pos, lower, upper);
  pos = points[find_point_for_projection (self, points, n, pos, velocity)];

  return pos;
}

static void
gesture_end (AdwSwipeTracker *self,
             double           distance,
             guint32          time,
             gboolean         is_touchpad)
{
  double end_progress, velocity;
  gint64 duration, max_duration;

  if (self->state == ADW_SWIPE_TRACKER_STATE_NONE)
    return;

  trim_history (self, time);

  velocity = calculate_velocity (self);

  end_progress = get_end_progress (self, velocity, is_touchpad);

  velocity /= distance;

  if ((end_progress - self->progress) * velocity <= 0)
    velocity = ANIMATION_BASE_VELOCITY;

  max_duration = MAX_ANIMATION_DURATION * log2 (1 + MAX (1, ceil (ABS (self->progress - end_progress))));

  duration = ABS ((self->progress - end_progress) / velocity * DURATION_MULTIPLIER);
  if (self->progress != end_progress)
    duration = CLAMP (duration, MIN_ANIMATION_DURATION, max_duration);

  g_signal_emit (self, signals[SIGNAL_END_SWIPE], 0, duration, end_progress);

  if (!self->cancelled)
    self->state = ADW_SWIPE_TRACKER_STATE_FINISHING;

  reset (self);
}

static void
gesture_cancel (AdwSwipeTracker *self,
                double           distance,
                guint32          time,
                gboolean         is_touchpad)
{
  if (self->state != ADW_SWIPE_TRACKER_STATE_PENDING &&
      self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING) {
    reset (self);

    return;
  }

  self->cancelled = TRUE;
  gesture_end (self, distance, time, is_touchpad);
}

static gboolean
should_suppress_drag (AdwSwipeTracker *self,
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

static void
drag_capture_begin_cb (AdwSwipeTracker *self,
                       double           start_x,
                       double           start_y,
                       GtkGestureDrag  *gesture)
{
  gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
}

static void
drag_begin_cb (AdwSwipeTracker *self,
               double           start_x,
               double           start_y,
               GtkGestureDrag  *gesture)
{
  GtkWidget *widget;

  if (self->state != ADW_SWIPE_TRACKER_STATE_NONE) {
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
drag_update_cb (AdwSwipeTracker *self,
                double           offset_x,
                double           offset_y,
                GtkGestureDrag  *gesture)
{
  double offset, distance, delta;
  gboolean is_vertical, is_offset_vertical;
  guint32 time;

  distance = adw_swipeable_get_distance (self->swipeable);

  is_vertical = (self->orientation == GTK_ORIENTATION_VERTICAL);
  offset = is_vertical ? offset_y : offset_x;

  if (!self->reversed)
    offset = -offset;

  delta = offset - self->prev_offset;
  self->prev_offset = offset;

  is_offset_vertical = (ABS (offset_y) > ABS (offset_x));

  if (self->state == ADW_SWIPE_TRACKER_STATE_REJECTED) {
    gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  append_to_history (self, delta, time);

  if (self->state == ADW_SWIPE_TRACKER_STATE_NONE) {
    if (is_vertical == is_offset_vertical)
      gesture_prepare (self, offset > 0 ? ADW_NAVIGATION_DIRECTION_FORWARD : ADW_NAVIGATION_DIRECTION_BACK, TRUE);
    else
      gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_PENDING) {
    double drag_distance;
    double first_point, last_point;
    gboolean is_overshooting;

    get_range (self, &first_point, &last_point);

    drag_distance = sqrt (offset_x * offset_x + offset_y * offset_y);
    is_overshooting = (offset < 0 && self->progress <= first_point) ||
                      (offset > 0 && self->progress >= last_point);

    if (drag_distance >= DRAG_THRESHOLD_DISTANCE) {
      if ((is_vertical == is_offset_vertical) && !is_overshooting) {
        gesture_begin (self);
        self->prev_offset = offset;
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_CLAIMED);
      } else {
        gtk_gesture_set_state (GTK_GESTURE (gesture), GTK_EVENT_SEQUENCE_DENIED);
      }
    }
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_SCROLLING)
    gesture_update (self, delta / distance, time);
}

static void
drag_end_cb (AdwSwipeTracker *self,
             double           offset_x,
             double           offset_y,
             GtkGestureDrag  *gesture)
{
  double distance;
  guint32 time;

  distance = adw_swipeable_get_distance (self->swipeable);

  if (self->state == ADW_SWIPE_TRACKER_STATE_REJECTED) {
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);

    reset (self);
    return;
  }

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  if (self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING) {
    gesture_cancel (self, distance, time, FALSE);
    gtk_gesture_set_state (self->touch_gesture, GTK_EVENT_SEQUENCE_DENIED);
    return;
  }

  gesture_end (self, distance, time, FALSE);
}

static void
drag_cancel_cb (AdwSwipeTracker  *self,
                GdkEventSequence *sequence,
                GtkGesture       *gesture)
{
  guint32 time;
  double distance;

  distance = adw_swipeable_get_distance (self->swipeable);

  time = gtk_event_controller_get_current_event_time (GTK_EVENT_CONTROLLER (gesture));

  gesture_cancel (self, distance, time, FALSE);
  gtk_gesture_set_state (gesture, GTK_EVENT_SEQUENCE_DENIED);
}

static gboolean
handle_scroll_event (AdwSwipeTracker *self,
                     GdkEvent        *event)
{
  GdkDevice *source_device;
  GdkInputSource input_source;
  double dx, dy, delta, distance;
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

  if (self->state == ADW_SWIPE_TRACKER_STATE_REJECTED) {
    if (gdk_scroll_event_is_stop (event))
      reset (self);

    return GDK_EVENT_PROPAGATE;
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_NONE) {
    if (gdk_scroll_event_is_stop (event))
      return GDK_EVENT_PROPAGATE;

    self->start_x = self->pointer_x;
    self->start_y = self->pointer_y;

    gesture_prepare (self, delta > 0 ? ADW_NAVIGATION_DIRECTION_FORWARD : ADW_NAVIGATION_DIRECTION_BACK, FALSE);
  }

  time = gdk_event_get_time (event);

  if (self->state == ADW_SWIPE_TRACKER_STATE_PENDING) {
    gboolean is_overshooting;
    double first_point, last_point;

    get_range (self, &first_point, &last_point);

    is_overshooting = (delta < 0 && self->progress <= first_point) ||
                      (delta > 0 && self->progress >= last_point);

    append_to_history (self, delta * SCROLL_MULTIPLIER, time);

    if (!is_overshooting)
      gesture_begin (self);
    else
      gesture_cancel (self, distance, time, TRUE);
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_SCROLLING) {
    if (gdk_scroll_event_is_stop (event)) {
      gesture_end (self, distance, time, TRUE);
    } else {
      append_to_history (self, delta * SCROLL_MULTIPLIER, time);

      gesture_update (self, delta / distance * SCROLL_MULTIPLIER, time);
      return GDK_EVENT_STOP;
    }
  }

  if (self->state == ADW_SWIPE_TRACKER_STATE_FINISHING)
    reset (self);

  return GDK_EVENT_PROPAGATE;
}

static void
scroll_begin_cb (AdwSwipeTracker          *self,
                 GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  handle_scroll_event (self, event);
}

static gboolean
scroll_cb (AdwSwipeTracker          *self,
           double                    dx,
           double                    dy,
           GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  return handle_scroll_event (self, event);
}

static void
scroll_end_cb (AdwSwipeTracker          *self,
               GtkEventControllerScroll *controller)
{
  GdkEvent *event;

  event = gtk_event_controller_get_current_event (GTK_EVENT_CONTROLLER (controller));

  handle_scroll_event (self, event);
}

static void
motion_cb (AdwSwipeTracker          *self,
           double                    x,
           double                    y,
           GtkEventControllerMotion *controller)
{
  self->pointer_x = x;
  self->pointer_y = y;
}

static void
update_controllers (AdwSwipeTracker *self)
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
set_orientation (AdwSwipeTracker *self,
                 GtkOrientation   orientation)
{

  if (orientation == self->orientation)
    return;

  self->orientation = orientation;
  update_controllers (self);

  g_object_notify (G_OBJECT (self), "orientation");
}

static void
adw_swipe_tracker_constructed (GObject *object)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);
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

  G_OBJECT_CLASS (adw_swipe_tracker_parent_class)->constructed (object);
}

static void
adw_swipe_tracker_dispose (GObject *object)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

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

  G_OBJECT_CLASS (adw_swipe_tracker_parent_class)->dispose (object);
}

static void
adw_swipe_tracker_get_property (GObject    *object,
                                guint       prop_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

  switch (prop_id) {
  case PROP_SWIPEABLE:
    g_value_set_object (value, adw_swipe_tracker_get_swipeable (self));
    break;

  case PROP_ENABLED:
    g_value_set_boolean (value, adw_swipe_tracker_get_enabled (self));
    break;

  case PROP_REVERSED:
    g_value_set_boolean (value, adw_swipe_tracker_get_reversed (self));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    g_value_set_boolean (value, adw_swipe_tracker_get_allow_mouse_drag (self));
    break;

  case PROP_ALLOW_LONG_SWIPES:
    g_value_set_boolean (value, adw_swipe_tracker_get_allow_long_swipes (self));
    break;

  case PROP_ORIENTATION:
    g_value_set_enum (value, self->orientation);
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_swipe_tracker_set_property (GObject      *object,
                                guint         prop_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  AdwSwipeTracker *self = ADW_SWIPE_TRACKER (object);

  switch (prop_id) {
  case PROP_SWIPEABLE:
    self->swipeable = ADW_SWIPEABLE (g_object_ref (g_value_get_object (value)));
    break;

  case PROP_ENABLED:
    adw_swipe_tracker_set_enabled (self, g_value_get_boolean (value));
    break;

  case PROP_REVERSED:
    adw_swipe_tracker_set_reversed (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_MOUSE_DRAG:
    adw_swipe_tracker_set_allow_mouse_drag (self, g_value_get_boolean (value));
    break;

  case PROP_ALLOW_LONG_SWIPES:
    adw_swipe_tracker_set_allow_long_swipes (self, g_value_get_boolean (value));
    break;

  case PROP_ORIENTATION:
    set_orientation (self, g_value_get_enum (value));
    break;

  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
  }
}

static void
adw_swipe_tracker_class_init (AdwSwipeTrackerClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = adw_swipe_tracker_constructed;
  object_class->dispose = adw_swipe_tracker_dispose;
  object_class->get_property = adw_swipe_tracker_get_property;
  object_class->set_property = adw_swipe_tracker_set_property;

  /**
   * AdwSwipeTracker:swipeable:
   *
   * The widget the swipe tracker is attached to. Must not be %NULL.
   *
   * Since: 1.0
   */
  props[PROP_SWIPEABLE] =
    g_param_spec_object ("swipeable",
                         "Swipeable",
                         "The swipeable the swipe tracker is attached to",
                         ADW_TYPE_SWIPEABLE,
                         G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  /**
   * AdwSwipeTracker:enabled:
   *
   * Whether the swipe tracker is enabled. When it's not enabled, no events
   * will be processed. Usually widgets will want to expose this via a property.
   *
   * Since: 1.0
   */
  props[PROP_ENABLED] =
    g_param_spec_boolean ("enabled",
                          "Enabled",
                          "Whether the swipe tracker processes events",
                          TRUE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:reversed:
   *
   * Whether to reverse the swipe direction. If the swipe tracker is horizontal,
   * it can be used for supporting RTL text direction.
   *
   * Since: 1.0
   */
  props[PROP_REVERSED] =
    g_param_spec_boolean ("reversed",
                          "Reversed",
                          "Whether swipe direction is reversed",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:allow-mouse-drag:
   *
   * Whether to allow dragging with mouse pointer. This should usually be
   * %FALSE.
   *
   * Since: 1.0
   */
  props[PROP_ALLOW_MOUSE_DRAG] =
    g_param_spec_boolean ("allow-mouse-drag",
                          "Allow mouse drag",
                          "Whether to allow dragging with mouse pointer",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  /**
   * AdwSwipeTracker:allow-long-swipes:
   *
   * Whether to allow swiping for more than one snap point at a time. If the
   * value is %FALSE, each swipe can only move to the adjacent snap points.
   *
   * Since: 1.0
   */
  props[PROP_ALLOW_LONG_SWIPES] =
    g_param_spec_boolean ("allow-long-swipes",
                          "Allow long swipes",
                          "Whether to allow swiping for more than one snap point at a time",
                          FALSE,
                          G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY);

  g_object_class_override_property (object_class,
                                    PROP_ORIENTATION,
                                    "orientation");

  g_object_class_install_properties (object_class, LAST_PROP, props);

  /**
   * AdwSwipeTracker::begin-swipe:
   * @self: The #AdwSwipeTracker instance
   * @direction: The direction of the swipe
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
                  1,
                  ADW_TYPE_NAVIGATION_DIRECTION);

  /**
   * AdwSwipeTracker::update-swipe:
   * @self: The #AdwSwipeTracker instance
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
   * AdwSwipeTracker::end-swipe:
   * @self: The #AdwSwipeTracker instance
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
adw_swipe_tracker_init (AdwSwipeTracker *self)
{
  self->event_history = g_array_new (FALSE, FALSE, sizeof (EventHistoryRecord));
  reset (self);

  self->orientation = GTK_ORIENTATION_HORIZONTAL;
  self->enabled = TRUE;
}

/**
 * adw_swipe_tracker_new:
 * @swipeable: a #GtkWidget to add the tracker on
 *
 * Create a new #AdwSwipeTracker object on @widget.
 *
 * Returns: the newly created #AdwSwipeTracker object
 *
 * Since: 1.0
 */
AdwSwipeTracker *
adw_swipe_tracker_new (AdwSwipeable *swipeable)
{
  g_return_val_if_fail (ADW_IS_SWIPEABLE (swipeable), NULL);

  return g_object_new (ADW_TYPE_SWIPE_TRACKER,
                       "swipeable", swipeable,
                       NULL);
}

/**
 * adw_swipe_tracker_get_swipeable:
 * @self: a #AdwSwipeTracker
 *
 * Get @self's swipeable widget.
 *
 * Returns: (transfer none): the swipeable widget
 *
 * Since: 1.0
 */
AdwSwipeable *
adw_swipe_tracker_get_swipeable (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), NULL);

  return self->swipeable;
}

/**
 * adw_swipe_tracker_get_enabled:
 * @self: a #AdwSwipeTracker
 *
 * Get whether @self is enabled. When it's not enabled, no events will be
 * processed. Generally widgets will want to expose this via a property.
 *
 * Returns: %TRUE if @self is enabled
 *
 * Since: 1.0
 */
gboolean
adw_swipe_tracker_get_enabled (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->enabled;
}

/**
 * adw_swipe_tracker_set_enabled:
 * @self: a #AdwSwipeTracker
 * @enabled: whether to enable to swipe tracker
 *
 * Set whether @self is enabled. When it's not enabled, no events will be
 * processed. Usually widgets will want to expose this via a property.
 *
 * Since: 1.0
 */
void
adw_swipe_tracker_set_enabled (AdwSwipeTracker *self,
                               gboolean         enabled)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  enabled = !!enabled;

  if (self->enabled == enabled)
    return;

  self->enabled = enabled;

  if (!enabled && self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING)
    reset (self);

  update_controllers (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ENABLED]);
}

/**
 * adw_swipe_tracker_get_reversed:
 * @self: a #AdwSwipeTracker
 *
 * Get whether @self is reversing the swipe direction.
 *
 * Returns: %TRUE is the direction is reversed
 *
 * Since: 1.0
 */
gboolean
adw_swipe_tracker_get_reversed (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->reversed;
}

/**
 * adw_swipe_tracker_set_reversed:
 * @self: a #AdwSwipeTracker
 * @reversed: whether to reverse the swipe direction
 *
 * Set whether to reverse the swipe direction. If @self is horizontal,
 * can be used for supporting RTL text direction.
 *
 * Since: 1.0
 */
void
adw_swipe_tracker_set_reversed (AdwSwipeTracker *self,
                                gboolean         reversed)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  reversed = !!reversed;

  if (self->reversed == reversed)
    return;

  self->reversed = reversed;
  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_REVERSED]);
}

/**
 * adw_swipe_tracker_get_allow_mouse_drag:
 * @self: a #AdwSwipeTracker
 *
 * Get whether @self can be dragged with mouse pointer.
 *
 * Returns: %TRUE is mouse dragging is allowed
 *
 * Since: 1.0
 */
gboolean
adw_swipe_tracker_get_allow_mouse_drag (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->allow_mouse_drag;
}

/**
 * adw_swipe_tracker_set_allow_mouse_drag:
 * @self: a #AdwSwipeTracker
 * @allow_mouse_drag: whether to allow mouse dragging
 *
 * Set whether @self can be dragged with mouse pointer. This should usually be
 * %FALSE.
 *
 * Since: 1.0
 */
void
adw_swipe_tracker_set_allow_mouse_drag (AdwSwipeTracker *self,
                                        gboolean         allow_mouse_drag)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  allow_mouse_drag = !!allow_mouse_drag;

  if (self->allow_mouse_drag == allow_mouse_drag)
    return;

  self->allow_mouse_drag = allow_mouse_drag;

  update_controllers (self);

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_MOUSE_DRAG]);
}

/**
 * adw_swipe_tracker_get_allow_long_swipes:
 * @self: a #AdwSwipeTracker
 *
 * Whether to allow swiping for more than one snap point at a time. If the
 * value is %FALSE, each swipe can only move to the adjacent snap points.
 *
 * Returns: %TRUE if long swipes are allowed, %FALSE otherwise
 *
 * Since: 1.0
 */
gboolean
adw_swipe_tracker_get_allow_long_swipes (AdwSwipeTracker *self)
{
  g_return_val_if_fail (ADW_IS_SWIPE_TRACKER (self), FALSE);

  return self->allow_long_swipes;
}

/**
 * adw_swipe_tracker_set_allow_long_swipes:
 * @self: a #AdwSwipeTracker
 * @allow_long_swipes: whether to allow long swipes
 *
 * Sets whether to allow swiping for more than one snap point at a time. If the
 * value is %FALSE, each swipe can only move to the adjacent snap points.
 *
 * Since: 1.0
 */
void
adw_swipe_tracker_set_allow_long_swipes (AdwSwipeTracker *self,
                                         gboolean         allow_long_swipes)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  allow_long_swipes = !!allow_long_swipes;

  if (self->allow_long_swipes == allow_long_swipes)
    return;

  self->allow_long_swipes = allow_long_swipes;

  g_object_notify_by_pspec (G_OBJECT (self), props[PROP_ALLOW_LONG_SWIPES]);
}

/**
 * adw_swipe_tracker_shift_position:
 * @self: a #AdwSwipeTracker
 * @delta: the position delta
 *
 * Move the current progress value by @delta. This can be used to adjust the
 * current position if snap points move during the gesture.
 *
 * Since: 1.0
 */
void
adw_swipe_tracker_shift_position (AdwSwipeTracker *self,
                                  double           delta)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  if (self->state != ADW_SWIPE_TRACKER_STATE_PENDING &&
      self->state != ADW_SWIPE_TRACKER_STATE_SCROLLING)
    return;

  self->progress += delta;
  self->initial_progress += delta;
}

void
adw_swipe_tracker_reset (AdwSwipeTracker *self)
{
  g_return_if_fail (ADW_IS_SWIPE_TRACKER (self));

  if (self->touch_gesture_capture)
    gtk_event_controller_reset (GTK_EVENT_CONTROLLER (self->touch_gesture_capture));

  if (self->touch_gesture)
    gtk_event_controller_reset GTK_EVENT_CONTROLLER ((self->touch_gesture));

  if (self->scroll_controller)
    gtk_event_controller_reset (self->scroll_controller);
}
