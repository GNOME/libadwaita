/*
 * Copyright (C) 2019 Alexander Mikhaylenko <exalm7659@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "hdy-swipeable-private.h"

/**
 * SECTION:hdy-swipeable
 * @short_description: An interface for swipeable widgets.
 * @title: HdySwipeable
 * @See_also: #HdyCarousel, #HdyDeck, #HdyLeaflet, #HdySwipeGroup
 *
 * The #HdySwipeable interface is implemented by all swipeable widgets. They
 * can be synced using #HdySwipeGroup.
 *
 * #HdySwipeable is only meant to be used by libhandy widgets and is currently
 * implemented by #HdyCarousel, #HdyDeck and #HdyLeaflet. It should not be
 * implemented by applications.
 *
 * Since: 0.0.12
 */

G_DEFINE_INTERFACE (HdySwipeable, hdy_swipeable, GTK_TYPE_WIDGET)

enum {
  SIGNAL_CHILD_SWITCHED,
  SIGNAL_SWIPE_BEGAN,
  SIGNAL_UPDATE_SWIPE,
  SIGNAL_END_SWIPE,
  SIGNAL_LAST_SIGNAL,
};

static guint signals[SIGNAL_LAST_SIGNAL];

static void
hdy_swipeable_default_init (HdySwipeableInterface *iface)
{
  /**
   * HdySwipeable::child-switched:
   * @self: The #HdySwipeable instance
   * @index: the index of the child to switch to
   * @duration: Animation duration in milliseconds
   *
   * This signal should be emitted when the widget's visible child is changed.
   *
   * @duration can be 0 if the child is switched without animation.
   *
   * Since: 1.0
   */
  signals[SIGNAL_CHILD_SWITCHED] =
    g_signal_new ("child-switched",
                  G_TYPE_FROM_INTERFACE (iface),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_UINT, G_TYPE_INT64);

  /**
   * HdySwipeable::swipe-began:
   * @self: The #HdySwipeable instance
   * @direction: The direction of the swipe, can be 1 or -1
   *
   * This signal is emitted when a possible swipe is detected. This is used by
   * #HdySwipeGroup, applications should not connect to it.
   * The @direction value can be used to restrict the swipe to a certain
   * direction.
   *
   * Since: 1.0
   */
  signals[SIGNAL_SWIPE_BEGAN] =
    g_signal_new ("swipe-began",
                  G_TYPE_FROM_INTERFACE (iface),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  HDY_TYPE_NAVIGATION_DIRECTION);

  /**
   * HdySwipeable::update-swipe:
   * @self: The #HdySwipeable instance
   * @value: The current animation progress value
   *
   * This signal is emitted every time the progress value changes. This is used
   * by #HdySwipeGroup, applications should not connect to it.
   *
   * Since: 0.0.12
   */
  signals[SIGNAL_UPDATE_SWIPE] =
    g_signal_new ("update-swipe",
                  G_TYPE_FROM_INTERFACE (iface),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  1,
                  G_TYPE_DOUBLE);

  /**
   * HdySwipeable::end-swipe:
   * @self: The #HdySwipeable instance
   * @duration: Snap-back animation duration in milliseconds
   * @to: The progress value to animate to
   *
   * This signal is emitted as soon as the gesture has stopped. This is used by
   * #HdySwipeGroup, applications should not connect to it.
   *
   * Since: 0.0.12
   */
  signals[SIGNAL_END_SWIPE] =
    g_signal_new ("end-swipe",
                  G_TYPE_FROM_INTERFACE (iface),
                  G_SIGNAL_RUN_FIRST,
                  0,
                  NULL, NULL, NULL,
                  G_TYPE_NONE,
                  2,
                  G_TYPE_INT64, G_TYPE_DOUBLE);
}

/**
 * hdy_swipeable_switch_child:
 * @self: a #HdySwipeable
 * @index: the index of the child to switch to
 * @duration: Animation duration in milliseconds
 *
 * See HdySwipeable::child-switched.
 *
 * Since: 0.0.12
 */
void
hdy_swipeable_switch_child (HdySwipeable *self,
                            guint         index,
                            gint64        duration)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_if_fail (iface->switch_child != NULL);

  (* iface->switch_child) (self, index, duration);
}

/**
 * hdy_swipeable_begin_swipe:
 * @self: a #HdySwipeable
 * @direction: The direction of the swipe
 * @direct: %TRUE if the swipe is directly triggered by a gesture,
 *   %FALSE if it's triggered via a #HdySwipeGroup
 *
 * This function is called by #HdySwipeTracker when a possible swipe is detected.
 * The implementation should check whether a swipe is possible, and if it is,
 * it must call hdy_swipe_tracker_confirm_swipe() to provide details about the
 * swipe, see that function for details.
 * The @direction value can be used to restrict the swipe to a certain direction.
*
 * The @direct parameter can be used to have widgets that aren't swipeable, but
 * can still animate in sync with other widgets in a #HdySwipeGroup by only
 * applying restrictions if @direct is %TRUE.
 *
 * Since: 0.0.12
 */
void
hdy_swipeable_begin_swipe (HdySwipeable           *self,
                           HdyNavigationDirection  direction,
                           gboolean                direct)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_if_fail (iface->begin_swipe != NULL);

  (* iface->begin_swipe) (self, direction, direct);

  g_signal_emit (self, signals[SIGNAL_SWIPE_BEGAN], 0, direction);
}

/**
 * hdy_swipeable_update_swipe:
 * @self: a #HdySwipeable
 * @value: The current animation progress value
 *
 * This function is called by #HdySwipeTracker every time the progress value
 * changes. The widget must redraw the widget to reflect the change.
 *
 * Since: 0.0.12
 */
void
hdy_swipeable_update_swipe (HdySwipeable *self,
                            gdouble       value)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_if_fail (iface->update_swipe != NULL);

  (* iface->update_swipe) (self, value);

  g_signal_emit (self, signals[SIGNAL_UPDATE_SWIPE], 0, value);
}

/**
 * hdy_swipeable_end_swipe:
 * @self: a #HdySwipeable
 * @duration: Snap-back animation duration in milliseconds
 * @to: The progress value to animate to
 *
 * This function is called by #HdySwipeTracker as soon as the gesture has
 * stopped. The widget must animate the progress from the current value to the
 * @to value with easeOutCubic interpolation over the next @duration
 * milliseconds.
 *
 * @to will always match either one of the provided snap points if the swipe was
 * completed successfully, or @cancel_progress value passed in
 * hdy_swipe_tracker_confirm_swipe() call if the swipe was cancelled.
 *
 * @duration can be 0, in that case the widget must immediately set the
 * progress value to @to.
 *
 * Since: 0.0.12
 */
void
hdy_swipeable_end_swipe (HdySwipeable *self,
                         gint64        duration,
                         gdouble       to)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_if_fail (iface->end_swipe != NULL);

  (* iface->end_swipe) (self, duration, to);

  g_signal_emit (self, signals[SIGNAL_END_SWIPE], 0, duration, to);
}

/**
 * hdy_swipeable_emit_child_switched:
 * @self: a #HdySwipeable
 * @index: the index of the child to switch to
 * @duration: Animation duration in milliseconds
 *
 * Emits HdySwipeable::child-switched signal. This should be called when the
 * widget switches visible child widget.
 *
 * @duration can be 0 if the child is switched without animation.
 *
 * Since: 1.0
 */
void
hdy_swipeable_emit_child_switched (HdySwipeable *self,
                                   guint         index,
                                   gint64        duration)
{
  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  g_signal_emit (self, signals[SIGNAL_CHILD_SWITCHED], 0, index, duration);
}

/**
 * hdy_swipeable_get_distance:
 * @self: a #HdySwipeable
 *
 * Gets the swipe distance of @self. This corresponds to how many pixels
 * 1 unit represents.
 *
 * Returns: the swipe distance in pixels
 *
 * Since: 1.0
 */
gdouble
hdy_swipeable_get_distance (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), 0);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_distance != NULL, 0);

  return (* iface->get_distance) (self);
}

/**
 * hdy_swipeable_get_range:
 * @self: a #HdySwipeable
 * @lower: (out) (allow-none): location to store the minimum possible value, or %NULL
 * @upper: (out) (allow-none): location to store the maximum possible value, or %NULL
 *
 * Gets the range of possible progress values.
 *
 * Since: 1.0
 */
void
hdy_swipeable_get_range (HdySwipeable *self,
                         gdouble      *lower,
                         gdouble      *upper)
{
  HdySwipeableInterface *iface;

  g_return_if_fail (HDY_IS_SWIPEABLE (self));

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_if_fail (iface->get_range != NULL);

  (* iface->get_range) (self, lower, upper);
}

static gdouble *
get_snap_points_from_range (HdySwipeable *self,
                            gint         *n_snap_points)
{
  gint n;
  gdouble *points, lower, upper;

  hdy_swipeable_get_range (self, &lower, &upper);

  n = (lower != upper) ? 2 : 1;

  points = g_new0 (gdouble, n);
  points[0] = lower;
  points[n - 1] = upper;

  if (n_snap_points)
    *n_snap_points = n;

  return points;
}

/**
 * hdy_swipeable_get_snap_points:
 * @self: a #HdySwipeable
 * @n_snap_points: (out)
 *
 * Gets the snap points of @self. Each snap point represents a progress value
 * that is considered acceptable to end the swipe on.
 *
 * If not implemented, the default implementation returns one snap point for
 * each end of the range, or just one snap point if they are equal.
 *
 * Returns: (array length=n_snap_points) (transfer full): the snap points of @self
 *
 * Since: 1.0
 */
gdouble *
hdy_swipeable_get_snap_points (HdySwipeable *self,
                               gint         *n_snap_points)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), NULL);

  iface = HDY_SWIPEABLE_GET_IFACE (self);

  if (iface->get_snap_points)
    return (* iface->get_snap_points) (self, n_snap_points);

  return get_snap_points_from_range (self, n_snap_points);
}

/**
 * hdy_swipeable_get_progress:
 * @self: a #HdySwipeable
 *
 * Gets the current progress of @self
 *
 * Returns: the current progress, unitless
 *
 * Since: 1.0
 */
gdouble
hdy_swipeable_get_progress (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), 0);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_progress != NULL, 0);

  return (* iface->get_progress) (self);
}

/**
 * hdy_swipeable_get_cancel_progress:
 * @self: a #HdySwipeable
 *
 * Gets the progress @self will snap back to after the gesture is canceled.
 *
 * Returns: the cancel progress, unitless
 *
 * Since: 1.0
 */
gdouble
hdy_swipeable_get_cancel_progress (HdySwipeable *self)
{
  HdySwipeableInterface *iface;

  g_return_val_if_fail (HDY_IS_SWIPEABLE (self), 0);

  iface = HDY_SWIPEABLE_GET_IFACE (self);
  g_return_val_if_fail (iface->get_cancel_progress != NULL, 0);

  return (* iface->get_cancel_progress) (self);
}
